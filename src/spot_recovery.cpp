#include "stdafx.h"
#include "spot.h"

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
