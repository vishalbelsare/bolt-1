// Copyright (C) 2019. Huawei Technologies Co., Ltd. All rights reserved.

// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef _GEMV_MALI_FP16
#define _GEMV_MALI_FP16
#include "gpu/mali/fp16/tensor_computing_fp16.h"

EE gemv_build_run_info(GCLHandle_t handle,
    U32 item_c,
    U32 row,
    U32 pitch,
    ActivationParamSpec activeMode,
    bool useBias,
    bool useOutputNchwc4,
    DataType dt,
    U32 *tmpOff,
    GCLMem_t tmpBuf,
    Mem *subTmpBuf,
    char *kernelName,
    KernelOpt *kernelOpt);

EE gemv_run(GCLHandle_t handle,
    U32 item_c,
    U32 row,
    U32 col,
    U32 pitch,
    U32 ow_str,
    U32 oh_str,
    U32 on_str,
    U32 o_off,
    Mem vec,
    Mem mat,
    Mem bias,
    Mem tmp,
    Mem out,
    char *kernelName,
    KernelOpt *kernelOpt);

EE gemv(GCLHandle_t handle,
    TensorDesc vecDesc,
    TensorDesc outputDesc,
    ActivationParamSpec activeMode,
    bool useOutputNchwc4,
    U32 *tmpOff,
    GCLMem_t tmpBuf,
    GCLMem_t vec,
    GCLMem_t bias,
    GCLMem_t mat,
    GCLMem_t out,
    ForwardRunInfoMali_t forwardRunInfo);

TensorDesc gemv_transform_filter_desc(TensorDesc filterDesc, U32 item_h, U32 item_c, U32 item_k);

EE gemv_transform_filter_run(
    GCLHandle_t handle, U32 fc, U32 fn, U32 item_c, DataType dt, Mem filter, Mem filterTran);

EE gemv_transform_filter_mali_fp16(GCLHandle_t handle,
    TensorDesc filterDesc,
    GCLMem_t filter,
    TensorDesc *fltmemDesc,
    GCLMem_t fltmem,
    ForwardRunInfoMali_t forwardRunInfo);

EE gemv_infer_forward_tmp_bytes_mali_fp16(
    TensorDesc inputDesc, TensorDesc outputDesc, U32 *bytes, ForwardRunInfoMali_t forwardRunInfo);
#endif
