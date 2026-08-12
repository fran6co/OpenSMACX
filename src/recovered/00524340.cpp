// ORIGINAL: 0x00524340 BYTE_EXACT
// name      Demo::timer_callback
// size      35 bytes
// spans     0x00524340-0x00524363
// prototype 
// callers   0   call targets   0
// 0x00524340  Demo::timer_callback  ->  _fn_00524340
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Toggle the flag and repaint through virtual slot 62.
Original Offset: 00524340
Return Value: the slot's result
Status: Complete
*/
// The receiver arrives as an EXPLICIT stack argument here - `mov ecx,[ebp+8]`
// at entry - unlike the __thiscall sibling at 0x00524320, which never needs
// that load. Catalogued as taking none.
class Slot { public: int slot062(); };
typedef int (Slot::*Slot062Fn)();

extern "C" int __cdecl fn_00524340(void *self) {
    // ONE statement. Splitting the toggle into a temp and a separate store
    // hoists the vtable load ahead of the `sete`.
    *reinterpret_cast<int *>(reinterpret_cast<char *>(self) + 0xa18) =
        (*reinterpret_cast<int *>(reinterpret_cast<char *>(self) + 0xa18) == 0);
    return (reinterpret_cast<Slot *>(self)->*(*reinterpret_cast<Slot062Fn *>(
        reinterpret_cast<char *>(*reinterpret_cast<void **>(self)) + 0xf8)))();
}
