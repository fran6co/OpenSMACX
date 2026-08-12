// ORIGINAL: 0x005F2700 BYTE_EXACT
// 0x005F2700  ?do_caption_buttons@Win@@QAEXXZ  ->  ?do_caption_buttons@Win@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

class VCall2 { public:
    virtual void slot000(int);
};

void Win::do_caption_buttons() {
    char *self = reinterpret_cast<char *>(this);
    VCall2 *obj;

    obj = *reinterpret_cast<VCall2 **>(self + 0xe4);
    if (obj) {
        obj->slot000(1);
        *reinterpret_cast<VCall2 **>(self + 0xe4) = 0;
    }
    obj = *reinterpret_cast<VCall2 **>(self + 0xe8);
    if (obj) {
        obj->slot000(1);
        *reinterpret_cast<VCall2 **>(self + 0xe8) = 0;
    }
    obj = *reinterpret_cast<VCall2 **>(self + 0xec);
    if (obj) {
        obj->slot000(1);
        *reinterpret_cast<VCall2 **>(self + 0xec) = 0;
    }
}
