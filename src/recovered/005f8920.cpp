// ORIGINAL: 0x005F8920 ?init@PullDown@@QAEXPAUMenu@@@Z 0x005F8920-0x005F8967 BYTE_EXACT
// size      71 bytes
// prototype void (__thiscall ?init@PullDown@@QAEXPAUMenu@@@Z)(PullDown* this, Menu*)
// callers   12   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4EF0
// 0x005F8920  ?init@PullDown@@QAEXPAUMenu@@@Z  ->  ?init@PullDown@@QAEXPAUMenu@@@Z
//
// Purpose: reset the pull-down to a clean, empty menu bar owned by `menu`,
//          resolving the class-shared default font on the first use.
//
// BLOCKED (2026-09-03, P1 calibration) - error C2248: 'markup_enabled_' :
// cannot access private member declared in class 'Buffer'.
//
// The image's last store is `mov dword ptr [esi+0x964], 0`, and 0x964 is
// Buffer + 0x520 = `markup_enabled_`: the window body is reaching straight
// into its own surface subobject, which is how the image reaches it. Buffer's
// run from 0x4AC to the end of the class is PRIVATE, so a derived window body
// cannot name it. buffer.h's own comment on the 0x4.. block already states the
// remedy and the reason - "PROTECTED, NOT PRIVATE: GraphicWin derives from
// Buffer and the homed window bodies read these directly - access control is
// compile-time only, so no offset and no emitted byte changes" - it just was
// never applied to this second half of the class. Widening that one `private:`
// to `protected:` is the whole fix; this file is pulldown.cpp's, so it was
// left to the owner of buffer.h rather than poked from here.
//
// Everything else in the body is settled: PullDownDefaultFont (0x009B7B68) and
// FontDefault (0x009BB484) are the two globals the scaffold bound as
// g_009b7b68/g_009bb484, menu_ is 0xf30, and the qualified GraphicWin::init
// call is 0x005D4EF0.

void PullDown::init(Menu *menu) {
    if (PullDownDefaultFont == nullptr) {
        PullDownDefaultFont = FontDefault;
    }
    menu_ = menu;
    GraphicWin::init(0, 0, 0, 0, nullptr, 0, nullptr, nullptr, nullptr);
    markup_enabled_ = 0;
}
