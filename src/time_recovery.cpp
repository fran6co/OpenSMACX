#include "stdafx.h"
#include "time.h"

/*
// ORIGINAL: 0x006161D0 ??0Time@@QAE@XZ 0x006161D0-0x006161F6 BYTE_EXACT
// size      38 bytes
// prototype void (__thiscall ??0Time@@QAE@XZ)(Time* this)
// callers   19   call targets   0
// kind      
// flags     
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned constructor
*/
Time::Time() {
    // IMAGE ORDER. An initialiser list runs in DECLARATION order,
    // and the image does not write the fields in that order.
    id_event_ = 0;
    callback1_ = nullptr;
    callback2_ = nullptr;
    cb_param2_ = 0;
    cb_param1_ = 0;
    count_ = 0;
    tick_posted_ = 0;
    resolution_ = 5;
    unk_2_ = 0;
    oneshot_state_ = 0;
}

/*
// ORIGINAL: 0x00616200 ??1Time@@QAE@XZ 0x00616200-0x00616260
// size      96 bytes
// prototype void (__thiscall ??1Time@@QAE@XZ)(Time* this)
// callers   30   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FD370
// indirect  0x0061621E 0x0061622C
// notes     Staged hybrid export redirect calls the source-owned destructor
*/
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
