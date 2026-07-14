#include "stdafx.h"
#include "spot.h"

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
