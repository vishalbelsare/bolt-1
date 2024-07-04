// Copyright (C) 2019. Huawei Technologies Co., Ltd. All rights reserved.

// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef _SQUEEZE_H
#define _SQUEEZE_H

#include "operator.hpp"

class Squeeze : public Operator {
public:
    Squeeze(DataType dt, SqueezeParamSpec p)
    {
        this->dt = dt;
        this->p = p;
    }
    ~Squeeze(){}
    OperatorType get_type() override
    {
        return OT_Squeeze;
    }

    SqueezeParamSpec get_param(const TensorDesc &desc)
    {
        SqueezeParamSpec ps = this->p;
        if (ps.num_axes == 0) {
            for (I32 i = desc.nDims - 1; i >= 0; i--) {
                if (desc.dims[i] == 1) {
                    ps.axes[ps.num_axes++] = desc.nDims - 1 - i;
                }
            }
        }
        return ps;
    }

protected:
    SqueezeParamSpec p;
};

#endif  // _SQUEEZE_H
