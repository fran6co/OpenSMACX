// ORIGINAL: 0x004C5FE0 BYTE_EXACT
// name      ?get_sound_version@@YAHXZ
// size      18 bytes
// spans     0x004C5FE0-0x004C5FF2
// prototype 
// callers   1   call targets   0
// 0x004C5FE0  ?get_sound_version@@YAHXZ  ->  ?get_sound_version@@YAHXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Ask the installed sound driver for its version, or zero when there
         is no driver.
Original Offset: 004C5FE0
Return Value: the driver version, or zero
Status: Complete
*/
void __cdecl clear_placeholder_never_used();

int __cdecl get_sound_version() {
    typedef int(__cdecl * SoundVersionFn)();
    if (*g_0090db78 == 0) {
        return 0;
    }
    // `g_0090db4c` addresses the CELL holding the driver's function pointer,
    // not the code: casting the cell itself emits a `mov` where the original
    // has `jmp dword ptr [addr]`.
    return (*reinterpret_cast<SoundVersionFn *>(g_0090db4c))();
}
