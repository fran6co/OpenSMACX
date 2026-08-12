// ORIGINAL: 0x00524320 BYTE_EXACT
// 0x00524320  sub_524320  ->  _sub_524320
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Toggle the flag and repaint through virtual slot 62.
Original Offset: 00524320
Return Value: the slot's result
Status: Complete
*/
// Catalogued `extern "C" int __cdecl sub_524320()`, and the disassembly
// disagrees: no stack frame at all, ecx read directly as `this`, and the body
// ends in `jmp [edx+0xf8]` rather than `ret`.
//
// The dispatch goes through a POINTER-TO-MEMBER-FUNCTION rather than a plain
// function pointer: VC6 rejects an explicit `__thiscall` function-pointer
// typedef with C4234, while a pointer-to-member is implicitly thiscall and
// reproduces the tail jump exactly.
class Slot524320 {
 public:
    int slot062();
};
typedef int (Slot524320::*Slot062Fn)();

class Base524320 {
 public:
    void *vtable_;
    char pad[0xa14];
    int flag_;
    int toggle_and_repaint();
};

int Base524320::toggle_and_repaint() {
    flag_ = (flag_ == 0);
    return (reinterpret_cast<Slot524320 *>(this)
            ->*(*reinterpret_cast<Slot062Fn *>(
                reinterpret_cast<char *>(vtable_) + 0xf8)))();
}
