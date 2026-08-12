// ORIGINAL: 0x004C5FB0 BYTE_EXACT
// 0x004C5FB0  ?init_sound_timer@@YAHKK@Z  ->  ?init_sound_timer@@YAHKK@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Start the sound timer through the installed driver, or report that
         there is no driver.
Original Offset: 004C5FB0
Return Value: the driver's result, or 4 when no driver is installed
Status: Complete
*/
int __cdecl init_sound_timer(unsigned long a1, unsigned long a2) {
    typedef int(__cdecl * SoundTimerFn)(unsigned long, unsigned long);
    if (*g_0090db78 == 0) {
        return 4;
    }
    return (*reinterpret_cast<SoundTimerFn *>(g_0090db2c))(a1, a2);
}
