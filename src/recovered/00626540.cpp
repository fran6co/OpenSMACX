// ORIGINAL: 0x00626540 BYTE_EXACT
// name      sub_626540
// size      87 bytes
// spans     0x00626540-0x00626597
// prototype 
// callers   0   call targets   1
// 0x00626540  sub_626540  ->  _sub_626540
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// The contract head takes no arguments, but [esp+4] is read as a base
// pointer throughout - a real parameter the emitter didn't know about
// since this row had never been recovered. Proposed: sub_626540(int a1).
//
// sub_626470 is declared here with 0 args (its own signature is also
// unrecovered), but the call needs 2. Redeclaring the same extern "C"
// name with a different arity is a hard conflict (C2733); a function
// pointer reinterpret_cast from its address is the same symbol/
// relocation with the arity this call actually needs.
typedef int (__cdecl *Sub626470Fn)(int, int);

extern "C" int __cdecl sub_626540(int a1) {
    int *p = reinterpret_cast<int *>(a1);
    p[1] = p[1] - 1;
    if (p[1] == 0) {
        Sub626470Fn fn = reinterpret_cast<Sub626470Fn>(&sub_626470);
        return fn(a1, p[2]) != 0;
    }
    p[3] = p[3] + p[4];
    p[6] = p[6] + p[7];
    if (p[6] > 0) {
        p[3] = p[5] + p[3];
        p[6] = p[6] - p[8];
    }
    return 1;
}
