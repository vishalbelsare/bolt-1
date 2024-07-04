// Copyright (C) 2019. Huawei Technologies Co., Ltd. All rights reserved.

// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "cpu/arm/fp32/blas_fp32.h"
#include "cpu/arm/fp32/kernels_fp32.h"

EE matrix_matrix_multiply_transform_rhsN_fp32(TensorDesc desc, F32 *src, F32 *dst)
{
    DataType dt;
    DataFormat df;
    U32 N, K;
    CHECK_STATUS(tensor2dGet(desc, &dt, &df, &K, &N));
    int i = 0;
    for (; i < (int)N - 11; i += 12) {
        matrix2_trans(12, K, N, src + i, dst + i * K);
    }
    for (; i < (int)N - 7; i += 8) {
        matrix2_trans(8, K, N, src + i, dst + i * K);
    }
    for (; i < (int)N - 3; i += 4) {
        matrix2_trans(4, K, N, src + i, dst + i * K);
    }
    if ((int)N > i) {
        matrix2_trans(N - i, K, N, src + i, dst + i * K);
    }
    return SUCCESS;
}

EE matrix_matrix_multiply_transform_rhsT_fp32(TensorDesc desc, F32 *src, F32 *dst)
{
    DataType dt;
    DataFormat df;
    U32 N, K;
    CHECK_STATUS(tensor2dGet(desc, &dt, &df, &N, &K));
    int i = 0;
    for (; i < (int)N - 11; i += 12) {
        matrix1_trans(12, K, K, src + i * K, dst + i * K);
    }
    for (; i < (int)N - 7; i += 8) {
        matrix1_trans(8, K, K, src + i * K, dst + i * K);
    }
    for (; i < (int)N - 3; i += 4) {
        matrix1_trans(4, K, K, src + i * K, dst + i * K);
    }
    if ((int)N > i) {
        matrix1_trans(N - i, K, K, src + i * K, dst + i * K);
    }
    return SUCCESS;
}

void mmm_NTail_M12(U32 M, U32 N, U32 K, F32 *matrix1, F32 *matrix2, F32 *result)
{
    float32x4x3_t mat2, res;
    for (U32 i = 0; i < N; i++) {
        res = vld3q_f32(result + i * M);
        for (U32 q = 0; q < K; q++) {
            mat2 = vld3q_f32(matrix2 + q * 12);
            res.val[0] = vfmaq_n_f32(res.val[0], mat2.val[0], matrix1[q * N + i]);
            res.val[1] = vfmaq_n_f32(res.val[1], mat2.val[1], matrix1[q * N + i]);
            res.val[2] = vfmaq_n_f32(res.val[2], mat2.val[2], matrix1[q * N + i]);
        }
        vst3q_f32(result + i * M, res);
    }
}

void mmm_NTail_M8(U32 M, U32 N, U32 K, F32 *matrix1, F32 *matrix2, F32 *result)
{
    float32x4x2_t mat2, res;
    for (U32 i = 0; i < N; i++) {
        res = vld2q_f32(result + i * M);
        for (U32 q = 0; q < K; q++) {
            mat2 = vld2q_f32(matrix2 + q * 8);
            res.val[0] = vfmaq_n_f32(res.val[0], mat2.val[0], matrix1[q * N + i]);
            res.val[1] = vfmaq_n_f32(res.val[1], mat2.val[1], matrix1[q * N + i]);
        }
        vst2q_f32(result + i * M, res);
    }
}

void mmm_NTail_M4(U32 M, U32 N, U32 K, F32 *matrix1, F32 *matrix2, F32 *result)
{
    float32x4_t mat2, res;
    for (U32 i = 0; i < N; i++) {
        res = vld1q_f32(result + i * M);
        for (U32 q = 0; q < K; q++) {
            mat2 = vld1q_f32(matrix2 + q * 4);
            res = vfmaq_n_f32(res, mat2, matrix1[q * N + i]);
        }
        vst1q_f32(result + i * M, res);
    }
}

void mmm_NTail_M(U32 MInner, U32 M, U32 N, U32 K, F32 *matrix1, F32 *matrix2, F32 *result)
{
    for (U32 i = 0; i < N; i++) {
        for (U32 j = 0; j < MInner; j++) {
            for (U32 k = 0; k < K; k++) {
                result[i * M + j] += matrix1[k * N + i] * matrix2[k * MInner + j];
            }
        }
    }
}

void mmm_N8_MTail(U32 MInner, U32 M, U32 K, F32 *matrix1, F32 *matrix2, F32 *result)
{
    CHECK_REQUIREMENT(MInner < 4);
    float32x4_t mat1[2] = {0}, res[4][2] = {{0}};
    F32 tmp[8] = {0};
    for (U32 i = 0; i < K; i++) {
        mat1[0] = vld1q_f32(matrix1 + i * 8);
        mat1[1] = vld1q_f32(matrix1 + i * 8 + 4);
        for (U32 j = 0; j < MInner; j++) {
            res[j][0] = vfmaq_n_f32(res[j][0], mat1[0], matrix2[j + i * MInner]);
            res[j][1] = vfmaq_n_f32(res[j][1], mat1[1], matrix2[j + i * MInner]);
        }
    }
    for (U32 p = 0; p < MInner; p++) {
        vst1q_f32(tmp, res[p][0]);
        vst1q_f32(tmp + 4, res[p][1]);
        for (U32 q = 0; q < 8; q++) {
            result[q * M + p] += tmp[q];
        }
    }
}

void mmm_N4_MTail(U32 MInner, U32 M, U32 K, F32 *matrix1, F32 *matrix2, F32 *result)
{
    CHECK_REQUIREMENT(MInner < 4);
    float32x4_t mat1 = {0}, res[4] = {0};
    F32 tmp[4] = {0};
    for (U32 i = 0; i < K; i++) {
        mat1 = vld1q_f32(matrix1 + i * 4);
        for (U32 j = 0; j < MInner; j++) {
            res[j] = vfmaq_n_f32(res[j], mat1, matrix2[j + i * MInner]);
        }
    }
    for (U32 p = 0; p < MInner; p++) {
        vst1q_f32(tmp, res[p]);
        for (U32 q = 0; q < 4; q++) {
            result[q * M + p] += tmp[q];
        }
    }
}

void mmm_N2_MTail(U32 MInner, U32 M, U32 K, F32 *matrix1, F32 *matrix2, F32 *result)
{
    CHECK_REQUIREMENT(MInner < 4);
    float32x2_t mat1 = {0}, res[4] = {0};
    F32 tmp[2] = {0};
    for (U32 i = 0; i < K; i++) {
        mat1 = vld1_f32(matrix1 + i * 2);
        for (U32 j = 0; j < MInner; j++) {
            res[j] = vfma_n_f32(res[j], mat1, matrix2[j + i * MInner]);
        }
    }
    for (U32 p = 0; p < MInner; p++) {
        vst1_f32(tmp, res[p]);
        for (U32 q = 0; q < 2; q++) {
            result[q * M + p] += tmp[q];
        }
    }
}

void mmm_2x4(U32 offset, U32 K, F32 *in, F32 *w, F32 *out)
{
    asm volatile(
        // init in- > v1, w- > v0
        "ldr d1, [%0]\n"

        "ldr q0, [%1]\n"

        // give in address to x3
        "mov x3, %0\n"

        // give w address to x0
        "mov x0, %1\n"

        // K- > x2
        "mov x2, %3\n"

        // give out address to x26
        "mov x26, %2\n"

        // load in bias
        "ldr q5, [x26]\n"
        "add x26, x26, %4\n"

        "ldr q7, [x26]\n"

        // Computation loop
        "0:\n"

        "ldr d3, [x3, 8]!\n"
        "ldr q29, [x0, 16]!\n"
        "fmla v5.4s, v0.4s, v1.s[0]\n"
        "fmla v7.4s, v0.4s, v1.s[1]\n"
        "subs x2, x2, #1\n"
        "mov v1.8b, v3.8b\n"
        "mov v0.16b, v29.16b\n"
        "bne 0b\n"

        "1:\n"

        // give out address to x26
        "mov x26, %2\n"

        "str q5, [x26]\n"
        "add x26, x26, %4\n"

        "str q7, [x26]\n"

        : "+r"(in), "+r"(w), "+r"(out)
        : "r"((I64)K), "r"((I64)offset)
        : "memory", "cc", "v30", "v29", "v11", "v9", "v7", "v5", "v3", "v1", "v0", "x26", "x3",
        "x2", "x0");
}

void mmm_4x4(U32 offset, U32 K, F32 *in, F32 *w, F32 *out)
{
    asm volatile(
        // init in- > v1, w- > v0
        "ldr q1, [%0]\n"

        "ldr q0, [%1]\n"

        // give in address to x3
        "mov x3, %0\n"

        // give w address to x0
        "mov x0, %1\n"

        // K- > x2
        "mov x2, %3\n"

        // give out address to x26
        "mov x26, %2\n"

        // load in bias
        "ldr q5, [x26]\n"
        "add x26, x26, %4\n"

        "ldr q7, [x26]\n"
        "add x26, x26, %4\n"

        "ldr q9, [x26]\n"
        "add x26, x26, %4\n"

        "ldr q11, [x26]\n"

        // Computation loop
        "0:\n"

        "ldr q3, [x3, 16]!\n"
        "ldr q29, [x0, 16]!\n"
        "fmla v5.4s, v0.4s, v1.s[0]\n"
        "fmla v7.4s, v0.4s, v1.s[1]\n"
        "subs x2, x2, #1\n"
        "fmla v9.4s, v0.4s, v1.s[2]\n"
        "fmla v11.4s, v0.4s, v1.s[3]\n"
        "mov v1.16b, v3.16b\n"
        "mov v0.16b, v29.16b\n"
        "bne 0b\n"

        "1:\n"

        // give out address to x26
        "mov x26, %2\n"

        "str q5, [x26]\n"
        "add x26, x26, %4\n"

        "str q7, [x26]\n"
        "add x26, x26, %4\n"

        "str q9, [x26]\n"
        "add x26, x26, %4\n"

        "str q11, [x26]\n"

        : "+r"(in), "+r"(w), "+r"(out)
        : "r"((I64)K), "r"((I64)offset)
        : "memory", "cc", "v30", "v29", "v11", "v9", "v7", "v5", "v3", "v1", "v0", "x26", "x3",
        "x2", "x0");
}

void mmm_8x4(U32 offset, U32 K, F32 *in, F32 *w, F32 *out)
{
    asm volatile(
        // init in- > v1, w- > v0
        "ldr q1, [%0]\n"

        "ldr q0, [%1]\n"

        // give in address to x3
        "mov x3, %0\n"

        // give w address to x0
        "mov x0, %1\n"

        // K- > x2
        "mov x2, %3\n"

        // give out address to x26
        "mov x26, %2\n"

        // load in bias
        "ldr q5, [x26]\n"
        "add x26, x26, %4\n"

        "ldr q7, [x26]\n"
        "add x26, x26, %4\n"

        "ldr q9, [x26]\n"
        "add x26, x26, %4\n"

        "ldr q11, [x26]\n"
        "add x26, x26, %4\n"

        "ldr q13, [x26]\n"
        "add x26, x26, %4\n"

        "ldr q15, [x26]\n"
        "add x26, x26, %4\n"

        "ldr q17, [x26]\n"
        "add x26, x26, %4\n"

        "ldr q19, [x26]\n"

        // Computation loop
        "0:\n"

        "ldr q3, [x3, 16]\n"
        "ldr q29, [x0, 16]!\n"
        "fmla v5.4s, v0.4s, v1.s[0]\n"
        "fmla v7.4s, v0.4s, v1.s[1]\n"
        "fmla v9.4s, v0.4s, v1.s[2]\n"
        "fmla v11.4s, v0.4s, v1.s[3]\n"

        "subs x2, x2, #1\n"
        "fmla v13.4s, v0.4s, v3.s[0]\n"
        "fmla v15.4s, v0.4s, v3.s[1]\n"
        "ldr q1, [x3, 32]!\n"
        "fmla v17.4s, v0.4s, v3.s[2]\n"
        "fmla v19.4s, v0.4s, v3.s[3]\n"
        "mov v0.16b, v29.16b\n"
        "bne 0b\n"

        // give out address to x26
        "mov x26, %2\n"

        "str q5, [x26]\n"
        "add x26, x26, %4\n"

        "str q7, [x26]\n"
        "add x26, x26, %4\n"

        "str q9, [x26]\n"
        "add x26, x26, %4\n"

        "str q11, [x26]\n"
        "add x26, x26, %4\n"

        "str q13, [x26]\n"
        "add x26, x26, %4\n"

        "str q15, [x26]\n"
        "add x26, x26, %4\n"

        "str q17, [x26]\n"
        "add x26, x26, %4\n"

        "str q19, [x26]\n"

        : "+r"(in), "+r"(w), "+r"(out)
        : "r"((I64)K), "r"((I64)offset)
        : "memory", "cc", "v30", "v29", "v19", "v17", "v15", "v13", "v11", "v9", "v7", "v5", "v3",
        "v1", "v0", "x26", "x3", "x2", "x0");
}

void mmm_2x8(U32 offset, U32 K, F32 *in, F32 *w, F32 *out)
{
    asm volatile(
        // init in- > v1, w- > v0
        "ldr d1, [%0]\n"

        "ldr q0, [%1]\n"

        // give in address to x3
        "mov x3, %0\n"

        // give w address to x0
        "mov x0, %1\n"

        // K- > x2
        "mov x2, %3\n"

        // give out address to x26
        "mov x26, %2\n"

        "ld1 {v5.4s, v6.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "ld1 {v7.4s, v8.4s}, [x26]\n"

        /* Layout
         * 5   6
         * 7   8
         */

        // Computation loop
        "0:\n"

        "ldr q29, [x0, 16]\n"
        "fmla v5.4s, v0.4s, v1.s[0]\n"
        "ldr d3, [x3, 8]!\n"
        "fmla v7.4s, v0.4s, v1.s[1]\n"
        "subs x2, x2, #1\n"

        "fmla v6.4s, v29.4s, v1.s[0]\n"
        "ldr q0, [x0, 32]!\n"
        "fmla v8.4s, v29.4s, v1.s[1]\n"
        "mov v1.8b, v3.8b\n"
        "bne 0b\n"

        // give out address to x26
        "mov x26, %2\n"

        "st1 {v5.4s, v6.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "st1 {v7.4s, v8.4s}, [x26]\n"

        : "+r"(in), "+r"(w), "+r"(out)
        : "r"((I64)K), "r"((I64)offset)
        : "memory", "cc", "v29", "v12", "v11", "v10", "v9", "v8", "v7", "v6", "v5", "v3", "v1",
        "v0", "x26", "x3", "x2", "x0");
}

void mmm_4x8(U32 offset, U32 K, F32 *in, F32 *w, F32 *out)
{
    asm volatile(
        // init in- > v1, w- > v0
        "ldr q1, [%0]\n"

        "ldr q0, [%1]\n"

        // give in address to x3
        "mov x3, %0\n"

        // give w address to x0
        "mov x0, %1\n"

        // K- > x2
        "mov x2, %3\n"

        // give out address to x26
        "mov x26, %2\n"

        "ld1 {v5.4s, v6.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "ld1 {v7.4s, v8.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "ld1 {v9.4s, v10.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "ld1 {v11.4s, v12.4s}, [x26]\n"

        /* Layout
         * 5   6
         * 7   8
         * 9   10
         * 11  12
         */

        // Computation loop
        "0:\n"

        "ldr q29, [x0, 16]\n"
        "fmla v5.4s, v0.4s, v1.s[0]\n"
        "ldr q3, [x3, 16]!\n"
        "fmla v7.4s, v0.4s, v1.s[1]\n"
        "subs x2, x2, #1\n"
        "fmla v9.4s, v0.4s, v1.s[2]\n"
        "fmla v11.4s, v0.4s, v1.s[3]\n"

        "fmla v6.4s, v29.4s, v1.s[0]\n"
        "ldr q0, [x0, 32]!\n"
        "fmla v8.4s, v29.4s, v1.s[1]\n"
        "fmla v10.4s, v29.4s, v1.s[2]\n"
        "fmla v12.4s, v29.4s, v1.s[3]\n"
        "mov v1.16b, v3.16b\n"
        "bne 0b\n"

        // give out address to x26
        "mov x26, %2\n"

        "st1 {v5.4s, v6.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "st1 {v7.4s, v8.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "st1 {v9.4s, v10.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "st1 {v11.4s, v12.4s}, [x26]\n"

        : "+r"(in), "+r"(w), "+r"(out)
        : "r"((I64)K), "r"((I64)offset)
        : "memory", "cc", "v29", "v12", "v11", "v10", "v9", "v8", "v7", "v6", "v5", "v3", "v1",
        "v0", "x26", "x3", "x2", "x0");
}

void mmm_8x8(U32 offset, U32 K, F32 *in, F32 *w, F32 *out)
{
    asm volatile(
        // init in- > v1, w- > v0
        "ldr q1, [%0]\n"

        "ldr q0, [%1]\n"

        // give in address to x3
        "mov x3, %0\n"

        // give w address to x0
        "mov x0, %1\n"

        // K- > x2
        "mov x2, %3\n"

        // give out address to x26
        "mov x26, %2\n"

        // load in bias
        "ld1 {v5.4s, v6.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "ld1 {v7.4s, v8.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "ld1 {v9.4s, v10.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "ld1 {v11.4s, v12.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "ld1 {v13.4s, v14.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "ld1 {v15.4s, v16.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "ld1 {v17.4s, v18.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "ld1 {v19.4s, v20.4s}, [x26]\n"

        /* Layout
           5   6
           7   8
           9   10
           11  12

           13  14
           15  16
           17  18
           19  20
         */

        // Computation loop
        "0:\n"

        "fmla v5.4s, v0.4s, v1.s[0]\n"
        "ldr q3, [x3, 16]!\n"
        "fmla v7.4s, v0.4s, v1.s[1]\n"
        "ldr q29, [x0, 16]\n"
        "fmla v9.4s, v0.4s, v1.s[2]\n"
        "fmla v11.4s, v0.4s, v1.s[3]\n"

        "fmla v13.4s, v0.4s, v3.s[0]\n"
        "subs x2, x2, #1\n"
        "fmla v15.4s, v0.4s, v3.s[1]\n"
        "fmla v17.4s, v0.4s, v3.s[2]\n"
        "fmla v19.4s, v0.4s, v3.s[3]\n"

        "fmla v6.4s, v29.4s, v1.s[0]\n"
        "fmla v8.4s, v29.4s, v1.s[1]\n"
        "ldr  q0, [x0, 32]!\n"
        "fmla v10.4s, v29.4s, v1.s[2]\n"
        "fmla v12.4s, v29.4s, v1.s[3]\n"

        "fmla v14.4s, v29.4s, v3.s[0]\n"
        "fmla v16.4s, v29.4s, v3.s[1]\n"
        "ldr q1, [x3, 16]!\n"
        "fmla v18.4s, v29.4s, v3.s[2]\n"
        "fmla v20.4s, v29.4s, v3.s[3]\n"

        "bne 0b\n"

        // give out address to x26
        "mov x26, %2\n"

        "st1 {v5.4s, v6.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "st1 {v7.4s, v8.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "st1 {v9.4s, v10.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "st1 {v11.4s, v12.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "st1 {v13.4s, v14.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "st1 {v15.4s, v16.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "st1 {v17.4s, v18.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "st1 {v19.4s, v20.4s}, [x26]\n"

        : "+r"(in), "+r"(w), "+r"(out)
        : "r"((I64)K), "r"((I64)offset)
        : "memory", "cc", "v29", "v20", "v19", "v18", "v17", "v16", "v15", "v14", "v13", "v12",
        "v11", "v10", "v9", "v8", "v7", "v6", "v5", "v3", "v1", "v0", "x26", "x3", "x2", "x0");
}

void mmm_2x12(U32 offset, U32 K, F32 *in, F32 *w, F32 *out)
{
    //return mmm_NTail_M(12, offset / 4, 2, K, in, w, out);
    asm volatile(
        // init in->v1, w->v0
        "ldr d1, [%0]\n"

        "ldr q0, [%1]\n"

        "ldr q29, [%1, 16]\n"  // prefetch one more w

        // give in address to x3
        "mov x3, %0\n"

        // give w address to x0
        "mov x0, %1\n"

        // K->x2
        "mov x2, %3\n"

        // give out address to x26
        "mov x26, %2\n"

        // load in bias
        "ld1 {v5.4s, v6.4s, v7.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "ld1 {v8.4s, v9.4s, v10.4s}, [x26]\n"

        /* Layout
           5   6   7
           8   9   10
         */

        // Computation loop
        "0:\n"
        // in(x3): v1
        // w(x0):  v0 v29 v30

        "fmla v5.4s, v0.4s, v1.s[0]\n"
        "ldr q30, [x0, 32]\n"
        "fmla v8.4s, v0.4s, v1.s[1]\n"
        "ldr d2, [x3, 8]!\n"  // input of next round

        "fmla v6.4s, v29.4s, v1.s[0]\n"
        "fmla v9.4s, v29.4s, v1.s[1]\n"
        "ldr q0, [x0, 48]!\n"  // first w of next round

        "fmla v7.4s, v30.4s, v1.s[0]\n"
        "ldr q29, [x0, 16]\n"
        "fmla v10.4s, v30.4s, v1.s[1]\n"
        "subs x2, x2, #1\n"

        "mov v1.8b, v2.8b\n"
        "bne 0b\n"

        // give out address to x26
        "mov x26, %2\n"

        "st1 {v5.4s, v6.4s, v7.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "st1 {v8.4s, v9.4s, v10.4s}, [x26]\n"

        : "+r"(in), "+r"(w), "+r"(out)
        : "r"((I64)K), "r"((I64)offset)
        : "memory", "cc", "v30", "v29", "v16", "v15", "v14", "v13", "v12", "v11", "v10", "v9", "v8",
        "v7", "v6", "v5", "v3", "v2", "v1", "v0", "x26", "x19", "x3", "x2", "x0");
}

void mmm_4x12(U32 offset, U32 K, F32 *in, F32 *w, F32 *out)
{
    asm volatile(
        // init in->v1, w->v0
        "ldr q1, [%0]\n"

        "ldr q0, [%1]\n"

        "ldr q29, [%1, 16]\n"  // prefetch one more w

        // give in address to x3
        "mov x3, %0\n"

        // give w address to x0
        "mov x0, %1\n"

        // K->x2
        "mov x2, %3\n"

        // give out address to x26
        "mov x26, %2\n"

        // load in bias
        "ld1 {v5.4s, v6.4s, v7.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "ld1 {v8.4s, v9.4s, v10.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "ld1 {v11.4s, v12.4s, v13.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "ld1 {v14.4s, v15.4s, v16.4s}, [x26]\n"

        /* Layout
           5   6   7
           8   9   10
           11  12  13
           14  15  16
         */

        // Computation loop
        "0:\n"
        // in(x3): v1
        // w(x0):  v0 v29 v30

        "fmla v5.4s, v0.4s, v1.s[0]\n"
        "ldr q30, [x0, 32]\n"
        "fmla v8.4s, v0.4s, v1.s[1]\n"
        "fmla v11.4s, v0.4s, v1.s[2]\n"
        "ldr q2, [x3, 16]!\n"  // input of next round
        "fmla v14.4s, v0.4s, v1.s[3]\n"

        "fmla v6.4s, v29.4s, v1.s[0]\n"
        "fmla v9.4s, v29.4s, v1.s[1]\n"
        "ldr q0, [x0, 48]!\n"  // first w of next round
        "fmla v12.4s, v29.4s, v1.s[2]\n"
        "fmla v15.4s, v29.4s, v1.s[3]\n"

        "fmla v7.4s, v30.4s, v1.s[0]\n"
        "ldr q29, [x0, 16]\n"
        "fmla v10.4s, v30.4s, v1.s[1]\n"
        "fmla v13.4s, v30.4s, v1.s[2]\n"
        "subs x2, x2, #1\n"
        "fmla v16.4s, v30.4s, v1.s[3]\n"

        "mov v1.16b, v2.16b\n"
        "bne 0b\n"

        // give out address to x26
        "mov x26, %2\n"

        "st1 {v5.4s, v6.4s, v7.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "st1 {v8.4s, v9.4s, v10.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "st1 {v11.4s, v12.4s, v13.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "st1 {v14.4s, v15.4s, v16.4s}, [x26]\n"

        : "+r"(in), "+r"(w), "+r"(out)
        : "r"((I64)K), "r"((I64)offset)
        : "memory", "cc", "v30", "v29", "v16", "v15", "v14", "v13", "v12", "v11", "v10", "v9", "v8",
        "v7", "v6", "v5", "v3", "v2", "v1", "v0", "x26", "x19", "x3", "x2", "x0");
}

void mmm_8x12(U32 offset, U32 K, F32 *in, F32 *w, F32 *out)
{
    asm volatile(
        // init in->v1, w->v0
        "ldr q1, [%0]\n"

        "ldr q0, [%1]\n"

        "ldr q29, [%1, 16]\n"  // prefetch one more w

        // give in address to x3
        "mov x3, %0\n"

        // give w address to x0
        "mov x0, %1\n"

        // K->x2
        "mov x2, %3\n"

        // give out address to x26
        "mov x26, %2\n"

        // load in bias
        "ld1 {v5.4s, v6.4s, v7.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "ld1 {v8.4s, v9.4s, v10.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "ld1 {v11.4s, v12.4s, v13.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "ld1 {v14.4s, v15.4s, v16.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "ld1 {v17.4s, v18.4s, v19.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "ld1 {v20.4s, v21.4s, v22.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "ld1 {v23.4s, v24.4s, v25.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "ld1 {v26.4s, v27.4s, v28.4s}, [x26]\n"

        /* Layout
           5   6   7
           8   9   10
           11  12  13
           14  15  16

           17  18  19
           20  21  22
           23  24  25
           26  27  28
         */

        // Computation loop
        "0:\n"
        // in(x3): v1 v2
        // w(x0):  v0 v29 v30

        "fmla v5.4s, v0.4s, v1.s[0]\n"
        "ldr q30, [x0, 32]\n"
        "fmla v8.4s, v0.4s, v1.s[1]\n"
        "fmla v11.4s, v0.4s, v1.s[2]\n"
        "ldr q2, [x3, 16]\n"
        "fmla v14.4s, v0.4s, v1.s[3]\n"

        "fmla v6.4s, v29.4s, v1.s[0]\n"
        "fmla v9.4s, v29.4s, v1.s[1]\n"
        "ldr q3, [x0, 48]!\n"  // first w of next round
        "fmla v12.4s, v29.4s, v1.s[2]\n"
        "fmla v15.4s, v29.4s, v1.s[3]\n"

        "fmla v7.4s, v30.4s, v1.s[0]\n"
        "subs x2, x2, #1\n"
        "fmla v10.4s, v30.4s, v1.s[1]\n"
        "fmla v13.4s, v30.4s, v1.s[2]\n"
        "fmla v16.4s, v30.4s, v1.s[3]\n"

        "fmla v17.4s, v0.4s, v2.s[0]\n"
        "ldr q1, [x3, 32]!\n"
        "fmla v20.4s, v0.4s, v2.s[1]\n"
        "fmla v23.4s, v0.4s, v2.s[2]\n"
        "fmla v26.4s, v0.4s, v2.s[3]\n"

        "fmla v18.4s, v29.4s, v2.s[0]\n"
        "mov v0.16b, v3.16b\n"
        "fmla v21.4s, v29.4s, v2.s[1]\n"
        "fmla v24.4s, v29.4s, v2.s[2]\n"
        "fmla v27.4s, v29.4s, v2.s[3]\n"

        "fmla v19.4s, v30.4s, v2.s[0]\n"
        "ldr q29, [x0, 16]\n"
        "fmla v22.4s, v30.4s, v2.s[1]\n"
        "fmla v25.4s, v30.4s, v2.s[2]\n"
        "fmla v28.4s, v30.4s, v2.s[3]\n"

        "bne 0b\n"

        // give out address to x26
        "mov x26, %2\n"

        "st1 {v5.4s, v6.4s, v7.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "st1 {v8.4s, v9.4s, v10.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "st1 {v11.4s, v12.4s, v13.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "st1 {v14.4s, v15.4s, v16.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "st1 {v17.4s, v18.4s, v19.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "st1 {v20.4s, v21.4s, v22.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "st1 {v23.4s, v24.4s, v25.4s}, [x26]\n"
        "add x26, x26, %4\n"
        "st1 {v26.4s, v27.4s, v28.4s}, [x26]\n"

        : "+r"(in), "+r"(w), "+r"(out)
        : "r"((I64)K), "r"((I64)offset)
        : "memory", "cc", "v30", "v29", "v28", "v27", "v26", "v25", "v24", "v23", "v22", "v21",
        "v20", "v19", "v18", "v17", "v16", "v15", "v14", "v13", "v12", "v11", "v10", "v9", "v8",
        "v7", "v6", "v5", "v3", "v2", "v1", "v0", "x26", "x3", "x2", "x0");
}

EE mmm_fp32(int M, int N, int K, bool transposeA, F32 *matrix1, F32 *matrix2, F32 *tmp, F32 *result)
{
    int blockK = K;
    int blockM = 96;
    for (int k = 0; k < K; k += blockK) {
        int KInner = UNI_MIN(blockK, K - k);
#ifdef _USE_OPENMP
#pragma omp parallel for num_threads(OMP_NUM_THREADS)
#endif
        for (int n = 0; n <= N - 8; n += 8) {
            F32 *matrix1Trans = tmp + n * KInner;
            if (transposeA) {
                matrix2_trans(8, KInner, N, matrix1 + n, matrix1Trans);
            } else {
                matrix1_trans(8, KInner, K, matrix1 + n * K + k, matrix1Trans);
            }
        }
        int n = N / 8 * 8;
        for (; n <= N - 4; n += 4) {
            F32 *matrix1Trans = tmp + n * KInner;
            if (transposeA) {
                matrix2_trans(4, KInner, N, matrix1 + n, matrix1Trans);
            } else {
                matrix1_trans(4, KInner, K, matrix1 + n * K + k, matrix1Trans);
            }
        }
        n = N / 4 * 4;
        for (; n <= N - 2; n += 2) {
            F32 *matrix1Trans = tmp + n * KInner;
            if (transposeA) {
                matrix2_trans(2, KInner, N, matrix1 + n, matrix1Trans);
            } else {
                matrix1_trans(2, KInner, K, matrix1 + n * K + k, matrix1Trans);
            }
        }
        if (N - n > 0) {
            F32 *matrix1Trans = tmp + n * KInner;
            if (transposeA) {
                matrix2_trans(N - n, KInner, N, matrix1 + n, matrix1Trans);
            } else {
                matrix1_trans(N - n, KInner, K, matrix1 + n * K + k, matrix1Trans);
            }
        }

#ifdef _USE_OPENMP
#pragma omp parallel for num_threads(OMP_NUM_THREADS)
#endif
        for (int i = 0; i < M; i += blockM) {
            int MInner = UNI_MIN(blockM, M - i);
            F32 *resultCurrent;
            int m, n;
            for (n = 0; n <= N - 8; n += 8) {
                F32 *matrix1Trans = tmp + n * KInner;
                //if (i == 0) {
                //    if (transposeA) {
                //        matrix2_trans(8, KInner, N, matrix1 + n, matrix1Trans);
                //    } else {
                //        matrix1_trans(8, KInner, K, matrix1 + n * K + k, matrix1Trans);
                //    }
                //}
                for (int m = 0; m <= (MInner - 12); m += 12) {
                    resultCurrent = result + n * M + m + i;
                    mmm_8x12(M * 4, KInner, matrix1Trans, matrix2 + (i + m) * KInner, resultCurrent);
                }
                m = MInner / 12 * 12;
                for (; m <= (MInner - 8); m += 8) {
                    resultCurrent = result + n * M + m + i;
                    mmm_8x8(M * 4, KInner, matrix1Trans, matrix2 + (i + m) * KInner, resultCurrent);
                }
                if ((MInner - m) >= 4) {
                    resultCurrent = result + n * M + m + i;
                    mmm_8x4(M * 4, KInner, matrix1Trans, matrix2 + (i + m) * KInner, resultCurrent);
                    m += 4;
                }
                if (MInner - m) {
                    resultCurrent = result + n * M + m + i;
                    mmm_N8_MTail(MInner - m, M, KInner, matrix1Trans, matrix2 + (i + m) * KInner,
                        resultCurrent);
                }
            }

            if ((N - n) >= 4) {
                F32 *matrix1Trans = tmp + n * KInner;
                //if (i == 0) {
                //    if (transposeA) {
                //        matrix2_trans(4, KInner, N, matrix1 + n, matrix1Trans);
                //    } else {
                //        matrix1_trans(4, KInner, K, matrix1 + n * K + k, matrix1Trans);
                //    }
                //}
                for (m = 0; m <= (MInner - 12); m += 12) {
                    resultCurrent = result + n * M + m + i;
                    mmm_4x12(M * 4, KInner, matrix1Trans, matrix2 + (i + m) * KInner, resultCurrent);
                }
                for (; m <= (MInner - 8); m += 8) {
                    resultCurrent = result + n * M + m + i;
                    mmm_4x8(M * 4, KInner, matrix1Trans, matrix2 + (i + m) * KInner, resultCurrent);
                }
                if ((MInner - m) >= 4) {
                    resultCurrent = result + n * M + m + i;
                    mmm_4x4(M * 4, KInner, matrix1Trans, matrix2 + (i + m) * KInner, resultCurrent);
                    m += 4;
                }
                if (MInner - m) {
                    resultCurrent = result + n * M + m + i;
                    mmm_N4_MTail(MInner - m, M, KInner, matrix1Trans, matrix2 + (i + m) * KInner,
                        resultCurrent);
                }
                n += 4;
            }

            if ((N - n) >= 2) {
                F32 *matrix1Trans = tmp + n * KInner;
                //if (i == 0) {
                //    if (transposeA) {
                //        matrix2_trans(4, KInner, N, matrix1 + n, matrix1Trans);
                //    } else {
                //        matrix1_trans(4, KInner, K, matrix1 + n * K + k, matrix1Trans);
                //    }
                //}
                for (m = 0; m <= (MInner - 12); m += 12) {
                    resultCurrent = result + n * M + m + i;
                    mmm_2x12(M * 4, KInner, matrix1Trans, matrix2 + (i + m) * KInner, resultCurrent);
                }
                for (; m <= (MInner - 8); m += 8) {
                    resultCurrent = result + n * M + m + i;
                    mmm_2x8(M * 4, KInner, matrix1Trans, matrix2 + (i + m) * KInner, resultCurrent);
                }
                if ((MInner - m) >= 4) {
                    resultCurrent = result + n * M + m + i;
                    mmm_2x4(M * 4, KInner, matrix1Trans, matrix2 + (i + m) * KInner, resultCurrent);
                    m += 4;
                }
                if (MInner - m) {
                    resultCurrent = result + n * M + m + i;
                    mmm_N2_MTail(MInner - m, M, KInner, matrix1Trans, matrix2 + (i + m) * KInner,
                        resultCurrent);
                }
                n += 2;
            }

            if (N - n) {
                F32 *matrix1Trans = tmp + n * KInner;
                //if (i == 0) {
                //    if (transposeA) {
                //        matrix2_trans(N - n, KInner, N, matrix1 + n, matrix1Trans);
                //    } else {
                //        matrix1_trans(
                //            N - n, KInner, K, matrix1 + n * K + k, matrix1Trans);
                //    }
                //}
                for (m = 0; m <= (MInner - 12); m += 12) {
                    resultCurrent = result + n * M + m + i;
                    mmm_NTail_M12(
                        M, N - n, KInner, matrix1Trans, matrix2 + (i + m) * KInner, resultCurrent);
                }
                for (; m <= (MInner - 8); m += 8) {
                    resultCurrent = result + n * M + m + i;
                    mmm_NTail_M8(
                        M, N - n, KInner, matrix1Trans, matrix2 + (i + m) * KInner, resultCurrent);
                }
                if ((MInner - m) >= 4) {
                    resultCurrent = result + n * M + m + i;
                    mmm_NTail_M4(
                        M, N - n, KInner, matrix1Trans, matrix2 + (i + m) * KInner, resultCurrent);
                    m += 4;
                }
                if (MInner - m) {
                    resultCurrent = result + n * M + m + i;
                    mmm_NTail_M(MInner - m, M, N - n, KInner, matrix1Trans,
                        matrix2 + (i + m) * KInner, resultCurrent);
                }
            }
        }
    }
    return SUCCESS;
}
