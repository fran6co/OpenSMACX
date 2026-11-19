#include "probe.h"
// see README.md; run: python3 run.py p01_fieldorder.cpp --cut 'call Buf::tile@1' --upto cmp --brief
struct Buf { char pad[0x588]; void tile(Buf *s, int x, int y, int sx, int sy, int w, int h); void box(RECT *, int, int); void soft(RECT *); void restore(); void boxs(RECT *, int *); void wr(char *, RECT *, int); };
struct Spr { int pad[4]; int w_, h_; int pad2[5]; int width() const { return w_; } int height() const { return h_; } };
extern Buf gb2;
extern int styles[];
void expand(RECT *, int, int);
struct PD { int a, psych, labs, b; };
extern PD pdata[];
struct SW { char pad0[0x3d90]; Spr lock[2]; char pad1[0x3ef0 - 0x3d90 - 2*sizeof(Spr)]; Buf slider[6]; char pad2[0x100]; RECT tip; int fac; void v(); void a(); void b(); void c(); void d(); void e(); void f(); void g(); void h(); };
void sound();
extern char stemp[];
#define T2 if (title.left < title.right) { gb2.tile(&slider[1], title.left, title.top, title.left, title.top, title.right - title.left, title.bottom - title.top); } gb2.soft(&tip); sound();
#define MID panel.top = panel.bottom - lock[0].height(); panel.left += lock[0].width(); title = panel; \
    panel.left += (panel.right - panel.left) * remaining / 10; \
    if (panel.left < panel.right) { title.right = panel.left + (remaining != 0); \
        gb2.tile(&slider[0], panel.left, panel.top, panel.left, panel.top, panel.right - panel.left, panel.bottom - panel.top); }
void SW::v() {
    RECT panel = tip; gb2.box(&panel, 1, 1);
    RECT title = panel; 
    gb2.box(&title, 0xbc, 0xbc);
    const int remaining = 10 - pdata[fac].psych - pdata[fac].labs;
    f1(remaining * 10);
    MID T2
}
void SW::a() {
    RECT panel = tip; gb2.restore(); gb2.boxs(&panel, &styles[6]); expand(&panel, -3, -3);
    RECT title = panel; 
    gb2.box(&title, 0xbc, 0xbc);
    const int remaining = 10 - pdata[fac].psych - pdata[fac].labs;
    f1(remaining * 10);
    MID T2
}
void SW::b() {
    RECT panel = tip; gb2.box(&panel, 1, 1);
    RECT title = panel; title.left += 0x17; title.right -= 0x17; title.bottom = title.top + 0x14;
    gb2.box(&title, 0xbc, 0xbc);
    const int remaining = 10 - pdata[fac].psych - pdata[fac].labs;
    f1(remaining * 10);
    MID T2
}
void SW::c() {
    RECT panel = tip; gb2.box(&panel, 1, 1);
    RECT title = panel; title.left += 0x17;
    gb2.box(&title, 0xbc, 0xbc);
    const int remaining = 10 - pdata[fac].psych - pdata[fac].labs;
    f1(remaining * 10);
    MID T2
}
void SW::d() {
    RECT panel = tip; gb2.box(&panel, 1, 1);
    RECT title = panel; title.right -= 0x17;
    gb2.box(&title, 0xbc, 0xbc);
    const int remaining = 10 - pdata[fac].psych - pdata[fac].labs;
    f1(remaining * 10);
    MID T2
}
void SW::e() {
    RECT panel = tip; gb2.box(&panel, 1, 1);
    RECT title = panel; title.bottom = title.top + 0x14;
    gb2.box(&title, 0xbc, 0xbc);
    const int remaining = 10 - pdata[fac].psych - pdata[fac].labs;
    f1(remaining * 10);
    MID T2
}
void SW::f() {
    RECT panel = tip; gb2.box(&panel, 1, 1);
    RECT title = panel; title.left += 0x17; title.right -= 0x17;
    gb2.box(&title, 0xbc, 0xbc);
    const int remaining = 10 - pdata[fac].psych - pdata[fac].labs;
    f1(remaining * 10);
    MID T2
}
void SW::g() {
    RECT panel = tip; gb2.box(&panel, 1, 1);
    RECT title = panel; title.right -= 0x17; title.left += 0x17;
    gb2.box(&title, 0xbc, 0xbc);
    const int remaining = 10 - pdata[fac].psych - pdata[fac].labs;
    f1(remaining * 10);
    MID T2
}
void SW::h() {
    RECT panel = tip; gb2.box(&panel, 1, 1);
    RECT title = panel; title.bottom = title.top + 0x14; title.left += 0x17; title.right -= 0x17;
    gb2.box(&title, 0xbc, 0xbc);
    const int remaining = 10 - pdata[fac].psych - pdata[fac].labs;
    f1(remaining * 10);
    MID T2
}
