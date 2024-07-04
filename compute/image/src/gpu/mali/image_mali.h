// Copyright (C) 2019. Huawei Technologies Co., Ltd. All rights reserved.

// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef _H_IMAGE_MALI
#define _H_IMAGE_MALI

#include "image.h"

EE resize_mali(GCLHandle_t handle,
    TensorDesc inputDesc,
    GCLMem_t input,
    ResizeParamSpec p,
    TensorDesc outputDesc,
    GCLMem_t tmpbuf,
    GCLMem_t output);

EE convert_color_mali(GCLHandle_t handle,
    TensorDesc inputDesc,
    GCLMem_t input,
    ConvertColorParamSpec p,
    TensorDesc outputDesc,
    GCLMem_t output);

EE lut_mali(GCLHandle_t handle,
    TensorDesc inputDesc,
    GCLMem_t input,
    TensorDesc lutDesc,
    GCLMem_t lut,
    LutParamSpec p,
    TensorDesc outputDesc,
    GCLMem_t output);

EE lut_preprocess_mali(
    GCLHandle_t handle, TensorDesc inputDesc, GCLMem_t input, TensorDesc outputDesc, GCLMem_t output);
#endif
