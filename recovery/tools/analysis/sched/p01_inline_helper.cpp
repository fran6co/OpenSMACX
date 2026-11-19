#include "probe.h"
// see README.md; run: python3 run.py p01_inline_helper.cpp --cut 'call Buf::tile@1' --upto cmp --brief
struct Buf { char pad[0x588]; void tile(Buf *s, int x, int y, int sx, int sy, int w, int h); void box(RECT *, int, int); void soft(RECT *); void restore(); void boxs(RECT *, int *); void wr(char *, RECT *, int); };
struct Spr { int pad[4]; int w_, h_; int pad2[5]; int width() const { return w_; } int height() const { return h_; } };
extern Buf gb2;
extern int styles[];
inline RECT *expand(RECT *rect, int h, int v) { rect->left -= h; rect->right += h; rect->top -= v; rect->bottom += v; return rect; }
struct PD { int a, psych, labs, b; };
extern PD pdata[];
struct SW { char pad0[0x3d90]; Spr lock[2]; char pad1[0x3ef0 - 0x3d90 - 2*sizeof(Spr)]; Buf slider[6]; char pad2[0x100]; RECT tip; int fac; void k0(); void k1(); void k2(); void k3(); void k4(); void k5(); void k6(); void k7(); void k8(); void k9(); };
void sound();
extern char stemp[];
#define T2 if (title.left < title.right) { gb2.tile(&slider[1], title.left, title.top, title.left, title.top, title.right - title.left, title.bottom - title.top); } gb2.soft(&tip); sound();
#define MID2 panel.top = panel.bottom - lock[0].height(); panel.left += lock[0].width(); title = panel; \
    panel.left += (panel.right - panel.left) * remaining / 10; \
    if (panel.left < panel.right) { title.right = (remaining != 0) + panel.left; \
        gb2.tile(&slider[0], panel.left, panel.top, panel.left, panel.top, panel.right - panel.left, panel.bottom - panel.top); }
#define MID3 panel.top = panel.bottom - lock[0].height(); panel.left += lock[0].width(); title = panel; \
    panel.left += (panel.right - panel.left) * remaining / 10; \
    if (panel.left < panel.right) { title.right = panel.left + (remaining != 0); \
        gb2.tile(&slider[0], panel.left, panel.top, panel.left, panel.top, panel.right - panel.left, panel.bottom - panel.top); } else { title.right = title.right; }
#define T2b if (title.right > title.left) { gb2.tile(&slider[1], title.left, title.top, title.left, title.top, title.right - title.left, title.bottom - title.top); } gb2.soft(&tip); sound();
#define MID panel.top = panel.bottom - lock[0].height(); panel.left += lock[0].width(); title = panel; \
    panel.left += (panel.right - panel.left) * remaining / 10; \
    if (panel.left < panel.right) { title.right = panel.left + (remaining != 0); \
        gb2.tile(&slider[0], panel.left, panel.top, panel.left, panel.top, panel.right - panel.left, panel.bottom - panel.top); }
struct RB { RECT r; }; inline void adj(RECT &t) { t.left += 0x17; t.right -= 0x17; t.bottom = t.top + 0x14; }
void SW::k0() {
    RECT panel = tip; gb2.restore(); gb2.boxs(&panel, &styles[6]); expand(&panel, -3, -3);
    RECT title = panel; title.left += 0x17; title.right -= 0x17; title.bottom = title.top + 0x14;
    gb2.box(&title, 0xbc, 0xbc);
    const int remaining = 10 - pdata[fac].psych - pdata[fac].labs;
    f1(remaining * 10);
    MID T2
}
void SW::k1() {
    RECT panel = tip; gb2.restore(); gb2.boxs(&panel, &styles[6]); expand(&panel, -3, -3);
    RECT title = panel; title.left = title.left + 0x17; title.right = title.right - 0x17; title.bottom = title.top + 0x14;
    gb2.box(&title, 0xbc, 0xbc);
    const int remaining = 10 - pdata[fac].psych - pdata[fac].labs;
    f1(remaining * 10);
    MID T2
}
void SW::k2() {
    RECT panel = tip; gb2.restore(); gb2.boxs(&panel, &styles[6]); expand(&panel, -3, -3);
    RECT title = panel; adj(title);
    gb2.box(&title, 0xbc, 0xbc);
    const int remaining = 10 - pdata[fac].psych - pdata[fac].labs;
    f1(remaining * 10);
    MID T2
}
void SW::k3() {
    RECT panel = tip; gb2.restore(); gb2.boxs(&panel, &styles[6]); expand(&panel, -3, -3);
    RECT title; title = panel; title.left += 0x17; title.right -= 0x17; title.bottom = title.top + 0x14;
    gb2.box(&title, 0xbc, 0xbc);
    const int remaining = 10 - pdata[fac].psych - pdata[fac].labs;
    f1(remaining * 10);
    MID T2
}
void SW::k4() {
    RECT panel = tip; gb2.restore(); gb2.boxs(&panel, &styles[6]); expand(&panel, -3, -3);
    RECT title = panel; title.left += 0x17; title.bottom = title.top + 0x14; title.right -= 0x17;
    gb2.box(&title, 0xbc, 0xbc);
    const int remaining = 10 - pdata[fac].psych - pdata[fac].labs;
    f1(remaining * 10);
    MID T2
}
void SW::k5() {
    RECT panel = tip; gb2.restore(); gb2.boxs(&panel, &styles[6]); panel.left += 3; panel.right -= 3; panel.top += 3; panel.bottom -= 3;
    RECT title = panel; title.left += 0x17; title.right -= 0x17; title.bottom = title.top + 0x14;
    gb2.box(&title, 0xbc, 0xbc);
    const int remaining = 10 - pdata[fac].psych - pdata[fac].labs;
    f1(remaining * 10);
    MID T2
}
void SW::k6() {
    RECT panel = tip; gb2.restore(); gb2.boxs(&panel, &styles[6]); panel.right -= 3; panel.left += 3; panel.top += 3; panel.bottom -= 3;
    RECT title = panel; title.left += 0x17; title.right -= 0x17; title.bottom = title.top + 0x14;
    gb2.box(&title, 0xbc, 0xbc);
    const int remaining = 10 - pdata[fac].psych - pdata[fac].labs;
    f1(remaining * 10);
    MID T2
}
void SW::k7() {
    RECT panel = tip; gb2.restore(); gb2.boxs(&panel, &styles[6]); expand(&panel, -3, -3);
    RECT title = panel; title.left += 0x17; title.right -= 0x17; title.bottom = title.top + 0x14;
    gb2.box(&title, 0xbc, 0xbc);
    const int remaining = 10 - pdata[fac].psych - pdata[fac].labs;
    f1(remaining * 10);
    MID2 T2
}
void SW::k8() {
    RECT panel = tip; gb2.restore(); gb2.boxs(&panel, &styles[6]); expand(&panel, -3, -3);
    RECT title = panel; title.left += 0x17; title.right -= 0x17; title.bottom = title.top + 0x14;
    gb2.box(&title, 0xbc, 0xbc);
    const int remaining = 10 - pdata[fac].psych - pdata[fac].labs;
    f1(remaining * 10);
    MID T2b
}
void SW::k9() {
    RECT panel = tip; gb2.restore(); gb2.boxs(&panel, &styles[6]); expand(&panel, -3, -3);
    RECT title = panel; title.left += 0x17; title.right -= 0x17; title.bottom = title.top + 0x14;
    gb2.box(&title, 0xbc, 0xbc);
    const int remaining = 10 - pdata[fac].psych - pdata[fac].labs;
    f1(remaining * 10);
    MID3 T2
}
