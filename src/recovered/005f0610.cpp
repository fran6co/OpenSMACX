// ORIGINAL: 0x005F0610 BYTE_EXACT
// name      ?set_bottom_border_thickness@Win@@QAEXH@Z
// size      61 bytes
// spans     0x005F0610-0x005F064D
// prototype void (__thiscall ?set_bottom_border_thickness@Win@@QAEXH@Z)(Win* this, int thickness)
// callers   1   call targets   0
// 0x005F0610  ?set_bottom_border_thickness@Win@@QAEXH@Z  ->  ?set_bottom_border_thickness@Win@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

class VCallArg { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003(int, int, int);
};

void Win::set_bottom_border_thickness(int a1) {
    if (iSomeFlag_ & 2) {
        bottom_border_thickness_ = a1;
        int width = client_rect_.right - client_rect_.left;
        int height = client_rect_.bottom - client_rect_.top;
        reinterpret_cast<VCallArg *>(this)->slot003(width, height, 0);
    }
}
