// ORIGINAL: 0x00644910 BYTE_EXACT
// name      sub_644910
// size      20 bytes
// spans     0x00644910-0x00644924
// prototype 
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// 0x00644910  sub_644910  ->  _sub_644910
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Invoke the callback stored in the object at +0x24, passing the
         object's own field at +0x28 and the caller's argument.
Original Offset: 00644910
Return Value: whatever the callback returns
Status: Complete
*/
extern "C" int __cdecl sub_644910(int a1, int a2) {
    // ONE expression, deliberately. Naming the function pointer and the extra
    // argument as locals costs two `mov`s and takes this to 22 bytes.
    typedef int(__cdecl * FnPtr)(int, int);
    return (*reinterpret_cast<FnPtr *>(reinterpret_cast<char *>(a2) + 0x24))(
        *reinterpret_cast<int *>(reinterpret_cast<char *>(a2) + 0x28), a1);
}
