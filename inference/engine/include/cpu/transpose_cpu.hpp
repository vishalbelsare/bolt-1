// Copyright (C) 2019. Huawei Technologies Co., Ltd. All rights reserved.

// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef _TRANSPOSE_CPU_H
#define _TRANSPOSE_CPU_H

#include "transpose.hpp"

class TransposeCPU : public Transpose {
public:
    TransposeCPU(DataType dt, TransposeParamSpec p) : Transpose(dt, p)
    {}

    std::shared_ptr<Operator> clone() override
    {
        std::shared_ptr<TransposeCPU> mem =
            std::shared_ptr<TransposeCPU>(new TransposeCPU(this->dt, this->p));
        *mem = *this;
        return mem;
    }

    void run() override
    {
        Tensor inputTensor = this->inputTensors[0];
        Tensor outputTensor = this->outputTensors[0];
        CHECK_STATUS(transpose(inputTensor, this->p, this->temp, outputTensor, &this->archInfo));
        outputTensor.set_scale(inputTensor.get_scale());
    }

    void update_param(TensorDesc desc)
    {
        if (this->p.num_axes == 0 && desc.nDims == 2) {
            p.num_axes = 2;
            p.axes[0] = 1;
            p.axes[1] = 0;
        }
    }

    EE infer_output_tensors_size(
        std::vector<Tensor *> inTensors, std::vector<Tensor *> outTensors) override
    {
        if (inTensors.size() == 1) {
            this->update_param(inTensors[0]->get_desc());
        }
        return transpose_infer_output_size(inTensors[0], this->p, outTensors[0], &this->archInfo);
    }
};

#endif  // _TRANSPOSE_CPU_H
