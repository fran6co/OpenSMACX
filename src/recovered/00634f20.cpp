// ORIGINAL: 0x00634F20 BYTE_EXACT
// name      ?UNK1@FileBox@@QAEHPAD@Z
// size      78 bytes
// spans     0x00634F20-0x00634F6E
// prototype int (__thiscall ?UNK1@FileBox@@QAEHPAD@Z)(FileBox* this, int8*)
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006453E0 0x00645470
// 0x00634F20  ?UNK1@FileBox@@QAEHPAD@Z  ->  ?UNK1@FileBox@@QAEHPAD@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// `#pragma function` is load-bearing: without it /O2 expands both CRT
// calls inline (`repne scasb` for strlen, a scasb+rep movsd/movsb copy
// for strcat) since the scaffold's `extern "C"` declarations don't
// stop the compiler recognizing them by name. The disassembly shows
// real calls to both, so the intrinsic form is forced back out.
#pragma function(strlen, strcat)
int FileBox::UNK1(char * a1) {
    char *self = reinterpret_cast<char *>(this);
    if (a1 == 0) {
        self[0x418] = 0;
        return 0;
    }
    if (strlen(a1) > 0x104) {
        return 3;
    }
    self[0x418] = 0;
    strcat(self + 0x418, a1);
    return 0;
}
