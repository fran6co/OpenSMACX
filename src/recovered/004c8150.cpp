// 0x004C8150  sub_4c8150  ->  _sub_4c8150
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// sub_4c8150 reads [ecx+N] with no stack access and returns `this` in eax -
// the ABI signature of a non-inline __thiscall constructor. No class in the
// catalogue owns it, so a dummy class expresses the receiver.
class Sub4c8150 {
public:
    Sub4c8150();

private:
    int field_0_;
    uint8_t field_4_[4];
    int field_8_;
    int field_C_;
    int field_10_;
    int field_14_;
    int field_18_;
    int field_1C_;
    int field_20_;
    int field_24_;
};

Sub4c8150::Sub4c8150() {
    memset(field_4_, 0, sizeof(field_4_));
    field_0_ = 0;
    field_8_ = 0;
    field_C_ = 0;
    field_10_ = 0;
    field_14_ = 0;
    field_1C_ = 0;
    field_20_ = 0;
    field_24_ = 0;
    field_18_ = 0xfa;
}
