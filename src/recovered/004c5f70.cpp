// ORIGINAL: 0x004C5F70 BYTE_EXACT
// name      ?unload_sound_dll@@YAXXZ
// size      53 bytes
// spans     0x004C5F70-0x004C5FA5
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C5F84
// 0x004C5F70  ?unload_sound_dll@@YAXXZ  ->  ?unload_sound_dll@@YAXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

typedef int (__stdcall *FreeLibraryProc)(int);

void __cdecl unload_sound_dll() {
    int handle = *g_0090db78;
    *g_0090db7c = 0;
    if (handle != 0) {
        FreeLibraryProc pFreeLibrary = *reinterpret_cast<FreeLibraryProc *>(g_00669128);
        pFreeLibrary(handle);
        *g_0090db78 = 0;
    }
    for (int index = 0; index < 0xb; ++index) {
        g_0090db24[index] = 0;
    }
}
