// Copyright (C) 2019. Huawei Technologies Co., Ltd. All rights reserved.

// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef H_OCL_ENGINE
#define H_OCL_ENGINE

#include "sys.h"
#include "ocl_context.h"

#define REGISTER_OCL_OPERATOR_RUN                                                               \
    virtual void run() override                                                                 \
    {                                                                                           \
        GCLHandle_t handle = OCLContext::getInstance().handle.get();                            \
        handle->kernelVec = &this->opKernelVec;                                                 \
        if (this->needSetKernelVec) {                                                           \
            CHECK_STATUS(gcl_clean_kernels(&(this->opKernelVec)));                              \
            run_prepare();                                                                      \
            this->needSetKernelVec = false;                                                     \
            if (this->needSelectKernelLS) {                                                     \
                CHECK_STATUS(gcl_infer_best_kernelVec_ls_with_map(handle, this->algorithmMap)); \
                this->needSelectKernelLS = false;                                               \
            }                                                                                   \
        }                                                                                       \
        CHECK_STATUS(gcl_run_kernelVec(handle));                                                \
    }                                                                                           \
                                                                                                \
    virtual void update_kernel() override                                                       \
    {                                                                                           \
        this->needSetKernelVec = true;                                                          \
    }                                                                                           \
                                                                                                \
protected:                                                                                      \
    bool needSetKernelVec;                                                                      \
    bool needSelectKernelLS;                                                                    \
    std::vector<GCLKernelInfo> opKernelVec;                                                     \
    MaliPara maliPara;

#define DESTROY_OCL_KERNEL CHECK_STATUS(gcl_clean_kernels(&(this->opKernelVec)));

#define INIT_GPU_INFO(runInfoPtr)                                 \
    {                                                             \
        maliPara.handle = OCLContext::getInstance().handle.get(); \
        maliPara.forwardRunInfo = runInfoPtr;                     \
        archInfo.archPara = &maliPara;                            \
        needSetKernelVec = true;                                  \
        needSelectKernelLS = true;                                \
    }
#endif  // H_OCL_ENGINE
