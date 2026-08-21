// ORIGINAL: 0x004042B0 sub_4042b0 0x004042B0-0x004042B3 BYTE_EXACT
// symbol    ?method@Sub4042b0@@QAEPAXXZ
// size      3 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// 0x004042B0  sub_4042b0  ->  _sub_4042b0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// mov eax, ecx; ret -- ecx is read as input with no prior definition,
// so this is a thiscall method that returns `this`, not a true 0-arg
// __cdecl (which would have nothing live in ecx to return).
class Sub4042b0 { public:
    void *method();
};

void *Sub4042b0::method() {
    return this;
}
