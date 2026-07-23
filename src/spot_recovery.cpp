#include "stdafx.h"
#include "spot.h"
#include "general.h"

Spot::Spot() : spots_(nullptr), max_count_(0), add_count_(0) {
#if defined(__GNUC__) && defined(__i386__)
    __asm__ __volatile__("" : : "a"(this) : "memory");
#endif
}

Spot::~Spot() {
    shutdown();
}

void Spot::clear() {
    spots_ = nullptr;
    max_count_ = 0;
    add_count_ = 0;
}

// Mirrors the recovered Spot::init (src/spot.cpp) for the leaf-test build,
// which links this minimal Spot support rather than the full implementation.
// Buffer::clear_links calls init, so it must resolve here; the real init runs
// in the DLL and hybrid.
void Spot::init(uint32_t count) {
    shutdown();
    spots_ = reinterpret_cast<SpotInternal *>(mem_get(count * sizeof(SpotInternal)));
    if (spots_) {
        max_count_ = count;
        add_count_ = 0;
    }
}
