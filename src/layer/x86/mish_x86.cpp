// Tencent is pleased to support the open source community by making ncnn available.
//
// Copyright (C) 2020 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// https://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.
#if __AVX__
#include "avx_activation.h"
#endif // __AVX__

#include "mish_x86.h"

#include <math.h>

namespace ncnn {

DEFINE_LAYER_CREATOR(Mish_x86)

Mish_x86::Mish_x86()
{
#if __AVX__
    support_packing = true;
#endif // __AVX__
}

int Mish_x86::forward_inplace(Mat& bottom_top_blob, const Option& opt) const
{
    int w = bottom_top_blob.w;
    int h = bottom_top_blob.h;
    int channels = bottom_top_blob.c;
    int size = w * h;
    int elempack = bottom_top_blob.elempack;
#if __AVX__
    if (elempack == 8)
    {
        #pragma omp parallel for num_threads(opt.num_threads)
        for (int q = 0; q < channels; q++)
        {
            float* ptr = bottom_top_blob.channel(q);

            for (int i = 0; i < size; i++)
            {
                __m256 _p = _mm256_loadu_ps(ptr);
                _p = mish_avx(_p);
                _mm256_storeu_ps(ptr, _p);
                ptr += 8;
            }
        }

        return 0;
    }
#endif // __AVX__

    #pragma omp parallel for num_threads(opt.num_threads)
    for (int q = 0; q < channels; q++)
    {
        float* ptr = bottom_top_blob.channel(q);

        int remain = size;
        for (; remain > 0; remain--)
        {
            *ptr = *ptr * tanh(log(exp(*ptr) + 1.f));
            ptr++;
        }
    }

    return 0;
}

} // namespace ncnn
