// ORIGINAL: 0x00524370 BYTE_EXACT
// name      sub_524370
// size      38 bytes
// spans     0x00524370-0x00524396
// prototype 
// callers   0   call targets   0
// 0x00524370  sub_524370  ->  _sub_524370
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Activate on Enter, Escape or Space, through virtual slot 58.
Original Offset: 00524370
Return Value: one
Status: Complete
*/
// A __thiscall member, not the catalogued `extern "C" int __stdcall`: ecx is
// used directly with no stack load, and `ret 8` pops the two explicit
// arguments a thiscall callee cleans up itself. The first is dead.
class Slot { public: int slot058(); };
typedef int (Slot::*Slot058Fn)();

class Widget {
 public:
    int on_key(int a1, int a2);
};

int Widget::on_key(int, int a2) {
    // 0xd, 0x1b, 0x20 are VK_RETURN, VK_ESCAPE and VK_SPACE.
    if (a2 == 0xd || a2 == 0x1b || a2 == 0x20) {
        (reinterpret_cast<Slot *>(this)->*(*reinterpret_cast<Slot058Fn *>(
            reinterpret_cast<char *>(*reinterpret_cast<void **>(this))
            + 0xe8)))();
    }
    return 1;
}
