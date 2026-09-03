// ORIGINAL: 0x00634FC0 ?UNK1@TexHeap@@QAEHH@Z 0x00634FC0-0x00634FD3 BYTE_EXACT
// size      19 bytes
// prototype int (__thiscall ?UNK1@TexHeap@@QAEHH@Z)(TexHeap* this, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00635050
// 0x00634FC0  ?UNK1@TexHeap@@QAEHH@Z  ->  ?UNK1@TexHeap@@QAEHH@Z
//
// BLOCKED on a census ceiling, not on a spelling (2026-09-03, P1 calibration).
//
// The body is `UNK2(a1); return this;` with the mangled return type H - the
// image hands the receiver back as an int handle (`mov esi, ecx` on entry,
// `mov eax, esi` on the way out). Every spelling of that in C++ is a pun of
// `this`, and both the shapes class_debt.py counts are at their ceilings:
//
//   pointer-as-int  0/0  - `return reinterpret_cast<int>(this)`
//   raw self-access 164/164 - any other reinterpret_cast or C-style cast of
//                             `this`, including `(int)this`
//
// The census cannot fall for a body whose whole point is a pointer travelling
// as an int, so raising pointer-as-int 0 -> 1 with this reason attached is the
// coordinator's call. Do NOT launder it through a local (`TexHeap *self =
// this; return reinterpret_cast<int>(self);`) - that is the member-pun trade
// the census prose already refuses, and it reads as a clean tree while saying
// the same lie.
//
// Its callee is unmodelled too: `UNK2` is ?UNK2@TexHeap@@QAEHH@Z at
// 0x00635050 (src/unrecovered/00635050.cpp, 800 bytes), so a forwarder is
// needed either way. TexHeap's declaration now lives in sprite.h.

int TexHeap::UNK1(int a1) {
    UNK2(a1);
    return reinterpret_cast<int>(this);
}
