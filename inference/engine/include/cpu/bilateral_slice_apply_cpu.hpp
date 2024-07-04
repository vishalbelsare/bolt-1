// Copyright (C) 2019. Huawei Technologies Co., Ltd. All rights reserved.

// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef _BILATERAL_SLICE_APPLY_CPU_H
#define _BILATERAL_SLICE_APPLY_CPU_H

#include "bilateral_slice_apply.hpp"

class BilateralSliceApplyCPU : public BilateralSliceApply {
public:
    BilateralSliceApplyCPU(BilateralSliceApplyParamSpec p) : BilateralSliceApply(p)
    {}

    std::shared_ptr<Operator> clone() override
    {
        std::shared_ptr<BilateralSliceApplyCPU> mem =
            std::shared_ptr<BilateralSliceApplyCPU>(new BilateralSliceApplyCPU(this->p));
        *mem = *this;
        return mem;
    }

    void run() override
    {
        Tensor inputTensor = this->inputTensors[0];
        Tensor gridTensor = this->inputTensors[1];
        if (this->p.mode == BILATERAL_SLICE_APPLY_NULL) {
            this->guideTensor = this->inputTensors[2];
        }
        Tensor outputTensor = this->outputTensors[0];
        CHECK_STATUS(bilateral_slice_apply(
            inputTensor, guideTensor, gridTensor, p, this->temp, outputTensor, &this->archInfo));
    }

    EE infer_output_tensors_size(
        std::vector<Tensor *> inTensors, std::vector<Tensor *> outTensors) override
    {
        auto inTensor = inTensors[0];
        auto gridTensor = inTensors[1];
        this->update_guide(inTensors);
        return bilateral_slice_apply_infer_output_size(
            inTensor, &guideTensor, gridTensor, p, outTensors[0], &this->archInfo);
    }
};

#endif  // _BILATERAL_SLICE_APPLYCPU_H
