// Copyright (C) 2019. Huawei Technologies Co., Ltd. All rights reserved.

// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef _H_MVM_COMMON
#define _H_MVM_COMMON

#include "data_type.h"
#include "arm_neon_expand.h"
#include "affinity_policy.h"

inline void mvm_row_tail(U32 N, U32 K, F16 *matrix, F16 *vector, F16 *result)
{
    float16x8_t vec, res, mat;
    U32 KTail = K % 8;
    U32 KInner = K - KTail;

    for (U32 i = 0; i < N; i += 1) {
        res = vdupq_n_f16(0);

        for (U32 j = 0; j < KInner; j += 8) {
            vec = vld1q_f16(&vector[j]);
            mat = vld1q_f16(&matrix[j + K * i]);
            res = vfmaq_f16(res, vec, mat);
        }
        result[i] += vaddvq_f16(res);

        if (KTail != 0) {
            for (U32 p = 0; p < KTail; p += 1) {
                result[i] += vector[p + KInner] * matrix[KInner + p + K * i];
            }
        }
    }
}

inline void mvm_col_tail(U32 N, U32 K, F16 *matrix, F16 *vector, F16 *result)
{
    float16x8_t tmp, res, mat;
    U32 NTail = N % 8;
    U32 NInner = N - NTail;

    for (U32 i = 0; i < K; i += 1) {
        for (U32 j = 0; j < NInner; j += 8) {
            tmp = vld1q_f16(result + j);
            mat = vld1q_f16(&matrix[j + N * i]);
            res = vfmaq_n_f16(tmp, mat, vector[i]);
            vst1q_f16(result + j, res);
        }
        if (NTail != 0) {
            for (U32 p = 0; p < NTail; p += 1) {
                result[NInner + p] += vector[i] * matrix[NInner + N * i + p];
            }
        }
    }
}

inline void mvm_col_kernel(U32 N, U32 K, F16 *matrix, F16 *vector, F16 *result)
{
    float16x8_t mat[4] = {0};

    F16 *w0 = matrix;
    F16 *w1 = matrix + K * N;
    F16 *w2 = matrix + 2 * K * N;
    F16 *w3 = matrix + 3 * K * N;

    U32 N_tail = N % 8;
    U32 N_inner = N - N_tail;

    for (U32 i = 0; i < K; i += 1) {
        for (U32 j = 0; j < N_inner; j += 8) {
            float16x8_t res[4] = {0};

            res[3] = vld1q_f16(result + j);
            mat[0] = vld1q_f16(w0);
            mat[1] = vld1q_f16(w1);
            mat[2] = vld1q_f16(w2);
            mat[3] = vld1q_f16(w3);

            res[0] = vfmaq_n_f16(res[3], mat[0], vector[i]);
            res[1] = vfmaq_n_f16(res[0], mat[1], vector[K + i]);
            res[2] = vfmaq_n_f16(res[1], mat[2], vector[2 * K + i]);
            res[3] = vfmaq_n_f16(res[2], mat[3], vector[3 * K + i]);

            w0 += 8;
            w1 += 8;
            w2 += 8;
            w3 += 8;
            vst1q_f16(result + j, res[3]);
        }
        if (N_tail != 0) {
            for (U32 p = 0; p < N_tail; p += 1) {
                result[N_inner + p] += vector[i] * *w0++;
                result[N_inner + p] += vector[i + K] * *w1++;
                result[N_inner + p] += vector[i + 2 * K] * *w2++;
                result[N_inner + p] += vector[i + 3 * K] * *w3++;
            }
        }
    }
}

inline void mvm_col_kernel_4x8(U32 N, U32 K, F16 *matrix, F16 *vector, F16 *result)
{
    F16 *result_end8 = result + N / 8 * 8;
    F16 *result_end = result + N;
    asm volatile("mov x20, %0\n"
                 "add x21, x20, %5\n"
                 "add x22, x21, %5\n"
                 "add x23, x22, %5\n"
                 "mov x24, %1\n"
                 "add x25, x24, %6\n"
                 "add x26, x25, %6\n"
                 "add x27, x26, %6\n"
                 "mov x2, x21\n"

                 "00:\n"
                 "cmp x20, x2\n"
                 "bge 01f\n"
                 "ldr h0, [x20], 2\n"
                 "dup v0.8h, v0.h[0]\n"
                 "ldr h1, [x21], 2\n"
                 "dup v1.8h, v1.h[0]\n"
                 "ldr h2, [x22], 2\n"
                 "dup v2.8h, v2.h[0]\n"
                 "ldr h3, [x23], 2\n"
                 "dup v3.8h, v3.h[0]\n"

                 "mov x28, %2\n"

                 "10:\n"
                 "cmp x28, %3\n"
                 "bge 11f\n"
                 "ldr  q4, [x28]\n"
                 "ldr  q8, [x24], 16\n"
                 "ldr  q9, [x25], 16\n"
                 "ldr q10, [x26], 16\n"
                 "fmla v4.8h,  v8.8h, v0.8h\n"
                 "ldr q11, [x27], 16\n"
                 "fmla v4.8h,  v9.8h, v1.8h\n"
                 "fmla v4.8h, v10.8h, v2.8h\n"
                 "fmla v4.8h, v11.8h, v3.8h\n"
                 "str  q4, [x28], 16\n"
                 "b 10b\n"

                 "11:\n"
                 "cmp x28, %4\n"
                 "bge 12f\n"
                 "ldr  h4, [x28]\n"
                 "ldr  h8, [x24], 2\n"
                 "ldr  h9, [x25], 2\n"
                 "ldr h10, [x26], 2\n"
                 "fmla h4,  h8, v0.h[0]\n"
                 "ldr h11, [x27], 2\n"
                 "fmla h4,  h9, v1.h[0]\n"
                 "fmla h4, h10, v2.h[0]\n"
                 "fmla h4, h11, v3.h[0]\n"
                 "str  h4, [x28], 2\n"
                 "b 11b\n"

                 "12:\n"
                 "b 00b\n"
                 "01:\n"
                 : "+r"(vector), "+r"(matrix), "+r"(result), "+r"(result_end8), "+r"(result_end)
                 : "r"((I64)K * 2), "r"((I64)K * N * 2)
                 : "memory", "cc", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28",
                 "x2", "v0", "v1", "v2", "v3", "v4", "v8", "v9", "v10", "v11");
}

inline void mvm_row_kernel(U32 N, U32 K, F16 *matrix, F16 *vector, F16 *result)
{
    float16x8_t res[4] = {0}, mat[4] = {0}, vec;
    float16x8_t tmp[6] = {0};

    F16 *w0 = matrix;
    F16 *w1 = matrix + K * N;
    F16 *w2 = matrix + 2 * K * N;
    F16 *w3 = matrix + 3 * K * N;

    U32 K_tail = K % 8;
    U32 K_inner = K - K_tail;

    for (U32 i = 0; i < N; i += 1) {
        for (U32 j = 0; j < K_inner; j += 8) {
            vec = vld1q_f16(&vector[j]);

            mat[0] = vld1q_f16(w0);
            mat[1] = vld1q_f16(w1);
            mat[2] = vld1q_f16(w2);
            mat[3] = vld1q_f16(w3);
            for (U32 k = 0; k < 4; k++) {
                res[k] = vfmaq_f16(res[k], vec, mat[k]);
            }
            w0 += 8;
            w1 += 8;
            w2 += 8;
            w3 += 8;
        }

        for (U32 m = 0; m < 2; m++) {
            tmp[m] = vpaddq_f16(res[m * 2], res[m * 2 + 1]);
        }
        tmp[4] = vpaddq_f16(tmp[0], tmp[1]);
        tmp[5] = vpaddq_f16(tmp[4], tmp[3]);
        F16 addbias;
        for (U32 n = 0; n < 4; n++) {
            vst1q_lane_f16_builtin(&addbias, tmp[5], n);
            result[i + N * n] += addbias;
            res[n] = vdupq_n_f16(0);
        }

        if (K_tail != 0) {
            for (U32 p = 0; p < K_tail; p += 1) {
                *(result + i) += vector[p + K_inner] * *w0++;
                *(result + N + i) += vector[p + K_inner] * *w1++;
                *(result + 2 * N + i) += vector[p + K_inner] * *w2++;
                *(result + 3 * N + i) += vector[p + K_inner] * *w3++;
            }
        }
    }
}

inline void mvm_col(U32 numRows, U32 numColumns, F16 *matrix, F16 *vector, F16 *result)
{
    // Actual layout is KN, and vector is K
    U32 N = numRows;
    U32 K = numColumns;
    U32 KInner = K / 4;
    U32 KTail = K % 4;
    mvm_col_kernel_4x8(N, KInner, matrix, vector, result);
    if (KTail != 0) {
        mvm_col_tail(N, KTail, matrix + (K - KTail) * N, vector + (K - KTail), result);
    }
}

// N is number of rows, K for columns
inline void mvm_row(U32 N, U32 K, F16 *matrix, F16 *vector, F16 *result)
{
    U32 NInner = (N / 4);
    U32 NTail = N % 4;
    mvm_row_kernel(NInner, K, matrix, vector, result);
    if (NTail != 0) {
        mvm_row_tail(NTail, K, matrix + (N - NTail) * K, vector, result + N - NTail);
    }
}
#endif
