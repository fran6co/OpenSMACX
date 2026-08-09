// ORIGINAL: 0x005242C0
// 0x005242C0  sub_5242c0  ->  _sub_5242c0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

class GraphicWinSub : public GraphicWin { public:
    void sub_5242c0();
};

void GraphicWinSub::sub_5242c0() {
    char *self = reinterpret_cast<char *>(this);
    Buffer *buf;
    if (this) {
        buf = reinterpret_cast<Buffer *>(self + 0x444);
    } else {
        buf = 0;
    }
    int flag = (*reinterpret_cast<int *>(self + 0xa18) == 0);
    char *rect = self + 0xa44;
    Sprite *spr = reinterpret_cast<Sprite *>(self + 0xa74 + flag * 0x2c);
    int p4 = *reinterpret_cast<int *>(self + 0xa48);
    int p3 = *reinterpret_cast<int *>(rect);
    int p2 = *reinterpret_cast<unsigned char *>(reinterpret_cast<char *>(spr) + 8);
    spr->draw(buf, p2, p3, p4, 1, 1);
    this->soft_update(reinterpret_cast<RECT *>(rect));
}
