// ORIGINAL: 0x006309C0 BYTE_EXACT
// name      sub_6309c0
// size      61 bytes
// spans     0x006309C0-0x006309FD
// prototype 
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006340C0
// 0x006309C0  sub_6309c0  ->  _sub_6309c0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// PROPOSAL: sub_6309c0 is a __thiscall member (nothing here reads it,
// but ecx is live and unread until it is used below), not the free
// __stdcall function the scaffold guessed - hence the shell class.
//
// PROPOSAL: the catalogued `sub_6340c0(int,int,int,int)` is also wrong.
// The disassembly's `add ecx, 0x10c` is not dead: it is materializing
// the receiver of a THISCALL to sub_6340c0 on an object embedded at
// this+0x10c, so its real arity is __thiscall(int,int,int,int) with
// the FIRST of those four now the address argument rather than a
// fifth operand - i.e. one of the catalogued four ints is consumed as
// `this`, not passed on the stack. Confirmed by reproduction: modeling
// the call as thiscall-on-this+0x10c is what makes `add ecx, 0x10c`
// appear at all; modeling sub_6340c0 as a free stdcall taking all 4
// values as explicit stack args never produces it.
//
// The trailing `? &g_bc4bc_target : 0` is the "guard ? obj : 0" shape:
// an `extern` (not the scaffold's fixed-address pointer) keeps the
// `neg/sbb/and` instead of letting /O2 fold the literal and the
// pattern vanish.
extern int g_bc4bc_target;

class Sub6340c0Shim {
public:
    int sub_6340c0(int, int, int, int);
};

class Sub6309c0Owner {
public:
    int sub_6309c0(int a1, int a2);
};

int Sub6309c0Owner::sub_6309c0(int a1, int a2) {
    if (*g_009be600 == 0) return 0;
    if (a1 == 0) return 0;
    Sub6340c0Shim *sub = reinterpret_cast<Sub6340c0Shim *>(
        reinterpret_cast<char *>(this) + 0x10c);
    int r = sub->sub_6340c0(reinterpret_cast<int>(&g_bc4bc_target), a1, 0, a2);
    return r != 0 ? reinterpret_cast<int>(&g_bc4bc_target) : 0;
}
