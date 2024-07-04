// Copyright (C) 2019. Huawei Technologies Co., Ltd. All rights reserved.

// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "cpu/arm/fp16/tensor_computing_fp16.h"
#include "cpu/cpu_functions.h"

EE eltwise_fp16(std::vector<void *> input,
    std::vector<int> inputSize,
    U32 num,
    U32 len,
    void *output,
    EltwiseMode eltwiseMode)
{
    EE ret = SUCCESS;
    U32 len_tail = len % 8;
    U32 len_main = len - len_tail;
    F16 *output_ptr = (F16 *)output;
#ifdef _USE_OPENMP
#pragma omp parallel num_threads(OMP_NUM_THREADS)
#endif
    {
        F16 buffer[8];
        F16 *tmp = buffer;
#ifdef _USE_OPENMP
#pragma omp for schedule(static)
#endif
        for (U32 i = 0; i < len_main; i += 8) {
            get_vector<F16>((F16 *)input[0], inputSize[0], &tmp, 8, i, 8, buffer);
            float16x8_t tmp_v = vld1q_f16(tmp);
            for (U32 j = 1; j < num; j++) {
                get_vector<F16>((F16 *)input[j], inputSize[j], &tmp, 8, i, 8, buffer);
                float16x8_t value_v = vld1q_f16(tmp);
                switch (eltwiseMode) {
                    case ELTWISE_SUM:
                        tmp_v = vaddq_f16(tmp_v, value_v);
                        break;
                    case ELTWISE_MAX:
                        tmp_v = vmaxq_f16(tmp_v, value_v);
                        break;
                    case ELTWISE_AND:
                    case ELTWISE_PROD:
                        tmp_v = vmulq_f16(tmp_v, value_v);
                        break;
                    case ELTWISE_SUB:
                        tmp_v = vsubq_f16(tmp_v, value_v);
                        break;
                    case ELTWISE_DIV:
                        tmp_v = vdivq_f16(tmp_v, value_v);
                        break;
                    default:
                        ret = NOT_SUPPORTED;
                        break;
                }
            }
            vst1q_f16(output_ptr + i, tmp_v);
        }
#ifdef _USE_OPENMP
#pragma omp for schedule(static)
#endif
        for (U32 i = len_main; i < len; i++) {
            get_vector<F16>((F16 *)input[0], inputSize[0], &tmp, 8, i, 1, buffer);
            F32 tmp_s = tmp[0];
            for (U32 j = 1; j < num; j++) {
                get_vector<F16>((F16 *)input[j], inputSize[j], &tmp, 8, i, 1, buffer);
                F32 value_s = tmp[0];
                switch (eltwiseMode) {
                    case ELTWISE_SUM:
                        tmp_s = value_s + tmp_s;
                        break;
                    case ELTWISE_MAX:
                        tmp_s = (value_s > tmp_s) ? value_s : tmp_s;
                        break;
                    case ELTWISE_AND:
                    case ELTWISE_PROD:
                        tmp_s *= value_s;
                        break;
                    case ELTWISE_SUB:
                        tmp_s -= value_s;
                        break;
                    case ELTWISE_DIV:
                        tmp_s /= value_s;
                        break;
                    default:
                        ret = NOT_SUPPORTED;
                        break;
                }
            }
            output_ptr[i] = tmp_s;
        }
    }
    return ret;
}
