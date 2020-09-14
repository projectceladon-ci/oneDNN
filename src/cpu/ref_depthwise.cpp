/*******************************************************************************
* Copyright 2018 Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#include <assert.h>
#include <math.h>

#include "c_types_map.hpp"
#include "type_helpers.hpp"
#include "mkldnn_thread.hpp"

#include "ref_depthwise.hpp"

namespace mkldnn {
namespace impl {
namespace cpu {

using namespace alg_kind;

template <typename T> inline T scale_shift_fwd(T s_val, T w_val, T b_val) {
    return s_val*w_val + b_val;
}

template <typename T> inline T prelu_fwd(T s_val, T w_val) {
    return s_val >= 0 ? s_val : s_val*w_val;
}

union float_raw {
    float f;
    unsigned short i[2];
};

static float bf16tof32(mkldnn_bfloat16_t bf16) {
    union float_raw t = { 0 };
    t.i[1] = bf16;
    t.i[0] = 0;
    return t.f;
}

static mkldnn_bfloat16_t f32tobf16(float f32) {
    union float_raw t = { 0 };
    t.f = f32;
    return t.i[1];
}

inline mkldnn_bfloat16_t bf16_scale_shift_fwd(mkldnn_bfloat16_t s_val, mkldnn_bfloat16_t w_val, mkldnn_bfloat16_t b_val) {
    return f32tobf16(bf16tof32(s_val) * bf16tof32(w_val) + bf16tof32(b_val));
}

inline mkldnn_bfloat16_t bf16_prelu_fwd(mkldnn_bfloat16_t s_val, mkldnn_bfloat16_t w_val) {
    return s_val >= 0 ? s_val : f32tobf16(bf16tof32(s_val) * bf16tof32(w_val));
}

ref_depthwise_scalar_fwd_t::ref_depthwise_scalar_fwd_t(const alg_kind_t alg_)
        : alg(alg_) {
    using namespace alg_kind;

    assert(utils::one_of(alg, depthwise_scale_shift, depthwise_prelu));
}

float ref_depthwise_scalar_fwd_t::compute_scalar(float s, const float* weights, const float* bias) {
    switch (alg) {
        case depthwise_scale_shift: return scale_shift_fwd(s, *weights, *bias);
        case depthwise_prelu: return prelu_fwd(s, *weights);
        default: assert(!"unknown depthwise alg_kind");
    }

    return 0.0f;
}

}
}
}

// vim: et ts=4 sw=4 cindent cino^=l0,\:0,N-s
