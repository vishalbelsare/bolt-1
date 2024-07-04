// Copyright (C) 2019. Huawei Technologies Co., Ltd. All rights reserved.

// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef _H_TENSOR_TRANSPOSE
#define _H_TENSOR_TRANSPOSE

#include "tensor_desc.h"

void array_transpose_naive(U32 elementSize,
    U32 *inputDims,
    const void *input,
    U32 *outputDims,
    void *output,
    U32 *transposeDims,
    I32 dimsNum);

void array_transpose(U32 elementSize,
    U32 *inputDims,
    const void *input,
    U32 *outputDims,
    void *output,
    U32 *transposeDims,
    I32 inputDimsNum,
    I32 outputDimsNum);

EE transformToNCHW(
    TensorDesc inputDesc, const void *input, TensorDesc outputDesc, void *output);

EE transformToNHWC(
    TensorDesc inputDesc, const void *input, TensorDesc outputDesc, void *output);

EE transformNCHWC16ToNCHWC8(
    TensorDesc inputDesc, const void *input, TensorDesc outputDesc, void *output);

EE transformNCHWToNCHWC8(
    TensorDesc inputDesc, const void *input, TensorDesc outputDesc, void *output);

EE transformNHWCToNCHWC8(
    TensorDesc inputDesc, const void *input, TensorDesc outputDesc, void *output);

EE transformNCHWC8ToNCHWC8ByGroup(
    TensorDesc inputDesc, const void *input, int group, TensorDesc outputDesc, void *output);

EE transformToNCHWC16(
    TensorDesc inputDesc, const void *input, TensorDesc outputDesc, void *output);

EE transformFormat(
    TensorDesc inputDesc, const void *input, TensorDesc outputDesc, void *output);
#endif
