// Copyright (C) 2019. Huawei Technologies Co., Ltd. All rights reserved.

// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef _POOLING_CPU_H
#define _POOLING_CPU_H

#include "pooling.hpp"

class PoolingCPU : public Pooling {
public:
    PoolingCPU(PoolingParamSpec p) : Pooling(p)
    {}

    std::shared_ptr<Operator> clone() override
    {
        std::shared_ptr<PoolingCPU> mem = std::shared_ptr<PoolingCPU>(new PoolingCPU(this->p));
        *mem = *this;
        return mem;
    }

    void run() override
    {
        CHECK_STATUS(pooling(
            this->inputTensors[0], this->p, this->temp, this->outputTensors, &this->archInfo));
    }

    EE infer_output_tensors_size(
        std::vector<Tensor *> inTensors, std::vector<Tensor *> outTensors) override
    {
        if (this->p.kernel_h == 0 && this->p.kernel_w == 0) {
            Pooling::set_stride(1, 1);
        }
        EE ret = pooling_infer_output_size(inTensors[0], this->p, outTensors[0], &this->archInfo);
        if (outTensors.size() == 2) {
            TensorDesc desc = outTensors[0]->get_desc();
            desc.dt = DT_I32;
            outTensors[1]->resize(desc);
        }
        return ret;
    }

    U32 infer_tmp_memory_size() override
    {
        U32 size = 0;
        CHECK_STATUS(pooling_infer_forward_tmp_bytes(
            this->inputTensors[0], this->outputTensors[0], &size, &this->archInfo));
        return size;
    }
};

#endif  // _POOLINGCPU_H
