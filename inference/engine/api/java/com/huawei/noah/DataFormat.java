/**
 * @file
 * @brief Java DataFormat Class Document
 *
 * @copyright
 * @code
 * Copyright (C) 2019. Huawei Technologies Co., Ltd. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * @endcode
 */
package com.huawei.noah;

/** multi-dimensions data format */
public enum DataFormat {
    NCHW,   ///< batch->channel->high->width data order
    NHWC,   ///< batch->high->width->channel data order
    MTK,    ///< batch->time->unit data order
    NORMAL, ///< vectorize input of row major
    NCHWC8, ///< batch->channel/8->width->high->channel eight element data order
    NCHWC4, ///< batch->channel/4->width->high->channel four element data order
    SCALAR  ///< 1-dim data
}
