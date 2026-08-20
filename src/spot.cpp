/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */
#include "stdafx.h"
#include "spot.h"
#include "general.h"

/*
Purpose: Shutdown the class instance.
// ORIGINAL: 0x005FA830 ?shutdown@Spot@@QAEXXZ 0x005FA830-0x005FA85E
// size      46 bytes
// prototype void (__thiscall ?shutdown@Spot@@QAEXXZ)(Spot* this)
// callers   9   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
Return Value: n/a
Status: Complete
*/
void Spot::shutdown() {
    if (spots_) {
        free(spots_);
    }
    clear();
}

/*
Purpose: Initialize the class instance with count.
// ORIGINAL: 0x005FA8A0 ?init@Spot@@QAEXH@Z 0x005FA8A0-0x005FA8F5
// size      85 bytes
// prototype void (__thiscall ?init@Spot@@QAEXH@Z)(Spot* this, int count)
// callers   20   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2
Return Value: n/a
Status: Complete
*/
void Spot::init(int count) {
    shutdown();
    spots_ = (SpotInternal *)mem_get(count * sizeof(SpotInternal));
    if (spots_) {
        max_count_ = count;
        add_count_ = 0;
    }
}

/*
Purpose: Search for a specific spot and replace the RECT value.
// ORIGINAL: 0x005FA900 ?replace@Spot@@QAEXHHHHHH@Z 0x005FA900-0x005FA956
// size      86 bytes
// prototype void (__thiscall ?replace@Spot@@QAEXHHHHHH@Z)(Spot* this, int position, int type, int left, int top, int length, int width)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Spot::replace(int position, int type, int left, int top, int length, int width) {
    for (uint32_t i = 0; i < add_count_; i++) {
        if (spots_[i].position == position && spots_[i].type == type) {
            spots_[i].rect.left = left;
            spots_[i].rect.right = left + length;
            spots_[i].rect.top = top;
            spots_[i].rect.bottom = top + width;
        }
    }
}

/*
Purpose: Add a new spot from the specified values.
// ORIGINAL: 0x005FA960 ?add@Spot@@QAEHHHHHHH@Z 0x005FA960-0x005FA9C0
// size      96 bytes
// prototype int (__thiscall ?add@Spot@@QAEHHHHHHH@Z)(Spot* this, int position, int type, int left, int top, int length, int width)
// callers   37   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: Spot position on success otherwise -1 on error
Status: Complete
*/
int Spot::add(int position, int type, int left, int top, int length, int width) {
    if (add_count_ >= max_count_) {
        return -1;
    }
    spots_[add_count_].rect.left = left;
    spots_[add_count_].rect.right = left + length;
    spots_[add_count_].rect.top = top;
    spots_[add_count_].rect.bottom = top + width;
    spots_[add_count_].position = position;
    spots_[add_count_].type = type;
    return add_count_++;
}

/*
Purpose: Add a new spot from a RECT.
Original Offset: n/a (Optimized out but available in PPC version)
Return Value: Spot position on success otherwise -1 on error
Status: Complete
*/
int Spot::add(int position, int type, RECT *rect) {
    return add(position, type, rect->left, rect->top, rect->right - rect->left, 
        rect->bottom - rect->top);
}

/*
Purpose: Remove all spots at a specific position.
// ORIGINAL: 0x005FA9C0 ?kill_pos@Spot@@QAEXH@Z 0x005FA9C0-0x005FAA04
// size      68 bytes
// prototype void (__thiscall ?kill_pos@Spot@@QAEXH@Z)(Spot* this, int position)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Spot::kill_pos(int position) {
    if (position < 0 || (uint32_t)position >= add_count_) {
        return;
    }
    add_count_--;
    if ((uint32_t)position < add_count_) {
        size_t size = sizeof(SpotInternal) * (add_count_ - position);
        memcpy_s(&spots_[position], size, &spots_[position + 1], size);
    }
}

/*
Purpose: Remove a specific spot.
// ORIGINAL: 0x005FAA10 ?kill_specific@Spot@@QAEXHH@Z 0x005FAA10-0x005FAA89
// size      121 bytes
// prototype void (__thiscall ?kill_specific@Spot@@QAEXHH@Z)(Spot* this, int position, int type)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Spot::kill_specific(int position, int type) {
    for (int i = add_count_ - 1; i >= 0; i--) {
        if (spots_[i].position == position && spots_[i].type == type) {
            kill_pos(i);
        }
    }
}

/*
Purpose: Remove all spots of a specific type.
// ORIGINAL: 0x005FAA90 ?kill_type@Spot@@QAEXH@Z 0x005FAA90-0x005FAAFD
// size      109 bytes
// prototype void (__thiscall ?kill_type@Spot@@QAEXH@Z)(Spot* this, int type)
// callers   25   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Spot::kill_type(int type) {
    for (int i = add_count_ - 1; i >= 0; i--) {
        if (spots_[i].type == type) {
            kill_pos(i);
        }
    }
}

/*
Purpose: Check if the coordinates fall inside a spot. If so, return information about the spot.
// ORIGINAL: 0x005FAB00 ?check@Spot@@QAEHHHPAH0@Z 0x005FAB00-0x005FAB69
// size      105 bytes
// prototype int (__thiscall ?check@Spot@@QAEHHHPAH0@Z)(Spot* this, int xCoord, int yCoord, int* spotPos, int* spotType)
// callers   49   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Spot position on success otherwise -1 on error
Status: Complete
*/
int Spot::check(int x, int y, int *spot_pos, int *spot_type) {
    int offset = add_count_ - 1;
    if (offset < 0) {
        return -1;
    }
    for (int i = offset; i >= 0; i--) {
        if (x >= spots_[i].rect.left && x < spots_[i].rect.right && y >= spots_[i].rect.top 
            && y < spots_[i].rect.bottom) {
            if (spot_pos) {
                *spot_pos = spots_[i].position;
            }
            if (spot_type) {
                *spot_type = spots_[i].type;
            }
            return i;
        }
    }
    return -1;
}

/*
Purpose: Check if the coordinates fall within a spot. If so, return information about the spot.
// ORIGINAL: 0x005FAB70 ?check@Spot@@QAEHHHPAH0PAURECT@@@Z 0x005FAB70-0x005FABFF
// symbol    ?check@Spot@@QAEHHHPAH0PAUtagRECT@@@Z
// size      143 bytes
// prototype int (__thiscall ?check@Spot@@QAEHHHPAH0PAURECT@@@Z)(Spot* this, int xCoord, int yCoord, int* spotPos, int* spotType, RECT* spotRect)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Spot position on success otherwise -1 on error
Status: Complete
*/
int Spot::check(int x, int y, int *spot_pos, int *spot_type, RECT *spot_rect) {
    int offset = add_count_ - 1;
    if (offset < 0) {
        return -1;
    }
    for (int i = offset; i >= 0; i--) {
        if (x >= spots_[i].rect.left && x < spots_[i].rect.right && y >= spots_[i].rect.top 
            && y < spots_[i].rect.bottom) {
            if (spot_pos) {
                *spot_pos = spots_[i].position;
            }
            if (spot_type) {
                *spot_type = spots_[i].type;
            }
            if (spot_rect) {
                *spot_rect = spots_[i].rect;
            }
            return i;
        }
    }
    return -1;
}

/*
Purpose: Get the RECT for a specific spot.
// ORIGINAL: 0x005FAC00 ?get_rect@Spot@@QAEHHHPAURECT@@@Z 0x005FAC00-0x005FAC5B BYTE_EXACT
// symbol    ?get_rect@Spot@@QAEHHHPAUtagRECT@@@Z
// size      91 bytes
// prototype int (__thiscall ?get_rect@Spot@@QAEHHHPAURECT@@@Z)(Spot* this, int position, int type, RECT* spotRect)
// callers   2   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: Spot position on success otherwise -1 on error
Status: Complete
*/
int Spot::get_rect(int position, int type, RECT *spot_rect) {
    int offset = add_count_ - 1;
    if (offset < 0) {
        return -1;
    }
    for (int i = offset; i >= 0; i--) {
        if (spots_[i].position == position && spots_[i].type == type) {
            if (spot_rect) {
                *spot_rect = spots_[i].rect;
            }
            return i;
        }
    }
    return -1;
}

// ---------------------------------------------------------------------------
// Merged from the retired src/spot_recovery.cpp on 2026-08-20, following
// src/text_recovery.cpp, which went into text.cpp on 2026-08-15. A class's
// bodies in two translation units is not a neutral choice: which unit a
// piece compiles in decides what can be inlined into it.
// ---------------------------------------------------------------------------

/*
// ORIGINAL: 0x005FA860 ??0Spot@@QAE@XZ 0x005FA860-0x005FA86D BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ??0Spot@@QAE@XZ)(Spot* this)
// callers   19   call targets   0
// kind      
// flags     
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned constructor
*/
Spot::Spot() {
    // IMAGE ORDER, not declaration order - an initialiser list runs
    // in the order the members are declared, and the image writes
    // the two counts before the pointer.
    max_count_ = 0;
    add_count_ = 0;
    spots_ = nullptr;
}

/*
// ORIGINAL: 0x005FA870 ??1Spot@@QAE@XZ 0x005FA870-0x005FA89E
// size      46 bytes
// prototype void (__thiscall ??1Spot@@QAE@XZ)(Spot* this)
// callers   110   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
// notes     Staged hybrid export redirect calls the source-owned destructor
*/
Spot::~Spot() {
    shutdown();
}

/*
// ORIGINAL: 0x005FA820 ?clear@Spot@@QAEXXZ 0x005FA820-0x005FA82B BYTE_EXACT
// size      11 bytes
// prototype void (__thiscall ?clear@Spot@@QAEXXZ)(Spot* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned method
*/
void Spot::clear() {
    // IMAGE ORDER, as in the constructor above.
    max_count_ = 0;
    add_count_ = 0;
    spots_ = nullptr;
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
