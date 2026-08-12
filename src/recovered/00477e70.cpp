// ORIGINAL: 0x00477E70 BYTE_EXACT
// 0x00477E70  ?on_mouse_move@MultiWin@@QAEXHH@Z  ->  ?on_mouse_move@MultiWin@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// BYTE_EXACT. `check`'s two int* out-params are `&a1`/`&a2` - the
// call passes each stack parameter's OWN address back into itself
// (`lea eax,[ebp+0xc]`/`lea ecx,[ebp+8]` are the parameters' own
// slots), so the call overwrites `a1`/`a2` in place with the output
// spot position/type, and everything after the call reads THOSE, not
// the caller's original arguments (matches Ghidra's
// `FUN_005fab00(param_2,param_3,&param_2,&param_3)` exactly). Branch
// polarity again needs the `!= -1` guard written first (`if (result
// != -1) {...} else {...}`, not `if (result == -1) {...} else
// if(...)`) to get the original's `je` (not `jne`) out of the
// bounds-check.
void MultiWin::on_mouse_move(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    int result = reinterpret_cast<Spot *>(self + 0x28ac)->check(a1, a2, &a1, &a2);
    int *field = reinterpret_cast<int *>(self + 0x1168);
    if (result != -1) {
        if (a2 == 1 && *field != a1) {
            *field = a1;
            draw(0);
            return;
        }
    } else {
        if (*field != -1) {
            *field = -1;
            draw(0);
        }
    }
}
