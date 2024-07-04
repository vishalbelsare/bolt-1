// Copyright (C) 2019. Huawei Technologies Co., Ltd. All rights reserved.

// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "cpu/x86/tensor_computing_x86.h"
#include "cpu/x86/int32/tensor_computing_int32.h"
#ifdef _USE_FP32
#include "cpu/x86/fp32/tensor_computing_fp32.h"
#endif
#include <float.h>

EE clip_x86(TensorDesc inputDesc, void *input, ClipParamSpec p, TensorDesc outputDesc, void *output)
{
    UNUSED(outputDesc);
    EE ret = NOT_SUPPORTED;
    switch (inputDesc.dt) {
#ifdef _USE_FP32
        case DT_F32: {
            ret = clip_fp32((F32 *)input, (F32 *)output, tensorNumElements(inputDesc), p.min, p.max);
            break;
        }
#endif
        case DT_I32: {
	        I32 max_i = INT_MAX;
	        I32 min_i = INT_MIN;
	        if (p.max < max_i) {
	            max_i = (I32)p.max;
	        } 
	        if (p.min > min_i) {
	            min_i = (I32)p.min;
	        }
            ret =
                clip_int32((I32 *)input, (I32 *)output, tensorNumElements(inputDesc), min_i, max_i);
            break;
        }
        default:
            break;
    }
    return ret;
}
