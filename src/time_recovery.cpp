#include "stdafx.h"
#include "time.h"

Time::Time()
    : oneshot_state_(0),
      id_event_(0),
      callback1_(nullptr),
      callback2_(nullptr),
      cb_param2_(0),
      cb_param1_(0),
      count_(0),
      tick_posted_(0),
      resolution_(5),
      unk_2_(0) {
}

Time::~Time() {
    close();
}

void Time::set_modal() {
    TimeModal = this;
}

void Time::release_modal() {
    TimeModal = nullptr;
}

int __cdecl Time::init_class() {
    TimeInitCount = static_cast<int>(static_cast<uint32_t>(TimeInitCount) + 1U);
    return 0;
}

void __cdecl Time::close_class() {
    TimeInitCount = static_cast<int>(static_cast<uint32_t>(TimeInitCount) - 1U);
}
