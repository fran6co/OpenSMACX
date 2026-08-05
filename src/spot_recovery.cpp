#include "stdafx.h"
#include "spot.h"
#include "general.h"

Spot::Spot() : spots_(nullptr), max_count_(0), add_count_(0) {
}

Spot::~Spot() {
    shutdown();
}

void Spot::clear() {
    spots_ = nullptr;
    max_count_ = 0;
    add_count_ = 0;
}

// Mirrors the recovered Spot::init (src/spot.cpp) for builds that link this
// minimal Spot support rather than the full implementation - recovery-leaf-tests
// needs it because Buffer::clear_links calls init.
//
// Guarded, because the annotated recovery lives in spot.cpp and every build that
// links both had two strong definitions of it. The DLL only tolerated that
// through -Wl,--allow-multiple-definition, which left the choice of body to link
// order rather than to anything deliberate.
#if defined(OPENSMACX_SPOT_MINIMAL_INIT)
void Spot::init(uint32_t count) {
    shutdown();
    spots_ = reinterpret_cast<SpotInternal *>(mem_get(count * sizeof(SpotInternal)));
    if (spots_) {
        max_count_ = count;
        add_count_ = 0;
    }
}
#endif
