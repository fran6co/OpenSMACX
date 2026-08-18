// ORIGINAL: 0x00635840 sub_635840 0x00635840-0x00635864 BYTE_EXACT
// size      36 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x00635855
// 0x00635840  sub_635840  ->  _sub_635840
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Hand the pending value to the attached object, then clear it.
Original Offset: 00635840
Return Value: n/a
Status: Complete
*/
// The callee is NOT thiscall: `push ecx; push eax; call [edx+0x80]` with no
// `mov ecx,eax` first means the object pointer is an explicit first argument,
// COM-style. A plain __stdcall function-pointer typedef is both correct and
// simpler here - no bare `__thiscall` type, so no C4234.
typedef void(__stdcall * DispatchFn)(void *, int);

class Handler {
 public:
    void flush();
};

void Handler::flush() {
    void *const target =
        *reinterpret_cast<void **>(reinterpret_cast<char *>(this) + 0xc);
    int *const pending =
        reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x10);
    if (target && *pending) {
        void *const vtable = *reinterpret_cast<void **>(target);
        (*reinterpret_cast<DispatchFn *>(
            reinterpret_cast<char *>(vtable) + 0x80))(target, *pending);
        *pending = 0;
    }
}
