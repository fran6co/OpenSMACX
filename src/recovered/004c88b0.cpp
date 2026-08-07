// 0x004C88B0  sub_4c88b0  ->  _sub_4c88b0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Forward to the delegate at +0x3C through its virtual slot 8.
Original Offset: 004C88B0
Return Value: zero
Status: Complete
*/
// A __thiscall member, not the catalogued `extern "C" int __cdecl`. The
// scaffolding's own VCall shim is used as-is here: slot 8 is nullary and the
// call pushes nothing, so its default arity happens to be right.
struct S_004C88B0 {
    char pad[0x3c];
    VCall *other;
    int Call();
};

int S_004C88B0::Call() {
    if (other) {
        other->slot008();
    }
    return 0;
}
