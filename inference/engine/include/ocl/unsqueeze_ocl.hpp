// Copyright (C) 2019. Huawei Technologies Co., Ltd. All rights reserved.

// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef _UNSQUEEZE_OCL_H
#define _UNSQUEEZE_OCL_H

#include "unsqueeze.hpp"

class UnsqueezeOCL : public Unsqueeze {
public:
    UnsqueezeOCL(DataType dt, UnsqueezeParamSpec p) : Unsqueeze(dt, p)
    {
        INIT_GPU_INFO(nullptr)
    }

    ~UnsqueezeOCL(){DESTROY_OCL_KERNEL}

    std::shared_ptr<Operator> clone() override
    {
        std::shared_ptr<UnsqueezeOCL> mem =
            std::shared_ptr<UnsqueezeOCL>(new UnsqueezeOCL(this->dt, this->p));
        *mem = *this;
        return mem;
    }

    inline void run_prepare()
    {
        OCLContext::getInstance().handle.get()->curOpName = this->get_name();
        CHECK_STATUS(
            unsqueeze(this->inputTensors[0], this->temp, this->outputTensors[0], &this->archInfo));
    }

    EE infer_output_tensors_size(
        std::vector<Tensor *> inTensors, std::vector<Tensor *> outTensors) override
    {
        this->needSetKernelVec = true;
        return unsqueeze_infer_output_size(inTensors[0], this->p, outTensors[0], &this->archInfo);
    }

    U32 infer_tmp_memory_size() override
    {
        Tensor inputTensor = this->inputTensors[0];
        Tensor outputTensor = this->outputTensors[0];
        U32 bytes = 0;
        CHECK_STATUS(
            unsqueeze_infer_forward_tmp_bytes(inputTensor, outputTensor, &bytes, &this->archInfo));
        return bytes;
    }
    REGISTER_OCL_OPERATOR_RUN
};

#endif  // _UNSQUEEZE_OCL_H
