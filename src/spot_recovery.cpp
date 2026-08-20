#include "stdafx.h"
#include "spot.h"
#include "general.h"

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
