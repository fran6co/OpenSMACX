#ifndef OPENSMACX_SPAN_MATH_H
#define OPENSMACX_SPAN_MATH_H

#include <stddef.h>
#include "vc6_compat.h"
#if !defined(_MSC_VER) || _MSC_VER > 1200
#include <stdint.h>
#endif

namespace span_math {

// Only +0x10/+0x14 are supported; the prefix is a bounded access view,
// not a complete object layout or named original owner.
struct EndpointPairView {
    uint32_t reserved_00_[4];
    volatile uint32_t low_at_10_;
    volatile uint32_t high_at_14_;
};

typedef char EndpointPairView_low_at_10[
    offsetof(EndpointPairView, low_at_10_) == 0x10 ? 1 : -1];
typedef char EndpointPairView_high_at_14[
    offsetof(EndpointPairView, high_at_14_) == 0x14 ? 1 : -1];

int half_span_plus_addend(const EndpointPairView *source, int addend);

}  // namespace span_math

#endif
