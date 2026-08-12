// ORIGINAL: 0x00590CD0 BYTE_EXACT
// name      sub_590cd0
// size      22 bytes
// spans     0x00590CD0-0x00590CE6
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// 0x00590CD0  sub_590cd0  ->  _sub_590cd0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Call the callback stored at the start of the object, when there is
         one, and return its result - or the null pointer itself when not.
Original Offset: 00590CD0
Return Value: the callback's result, or zero
Status: Complete
*/
// A __thiscall member: the first instruction is `mov eax,[ecx]` and ecx is
// never loaded from the stack. Writing `__thiscall` explicitly is C4234.
struct S_00590CD0 {
    int Call(int a1);
};

int S_00590CD0::Call(int a1) {
    typedef int(__cdecl * FnPtr)(int);
    FnPtr callback = *reinterpret_cast<FnPtr *>(this);
    // ONE return site. An explicit `return 0;` on the guarded path
    // materialises a second one and costs six bytes; reusing the already
    // tested pointer's own value leaves eax exactly as the original does.
    int result = reinterpret_cast<int>(callback);
    if (callback) {
        result = callback(a1);
    }
    return result;
}
