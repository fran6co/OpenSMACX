#include "probe.h"
// p24 with the cursor one step further (f1(gc + 1) added): PickWin shape: a named value used in an arithmetic argument (count + 14) does not advance the cursor
struct PW { int pad[4]; int count; void s1(); void s2(); void s3(); void s4(); void s5(); void s6(); };
struct MM { void item(char *, int); int exec(int, int, int); };
extern char gstr[];
template <class T> inline T nv(T v) { return v; }
void PW::s1() { MM m; int x, y; f2((int)&x, (int)&y); f1(ga + 1); f1(gc + 1); m.item(gstr, count + 14); m.exec(x, y, 0); }
void PW::s2() { MM m; int x, y; f2((int)&x, (int)&y); f1(ga + 1); f1(gc + 1); int c = count; m.item(gstr, c + 14); m.exec(x, y, 0); }
void PW::s3() { MM m; int x, y; f2((int)&x, (int)&y); f1(ga + 1); f1(gc + 1); m.item(gstr, nv(count) + 14); m.exec(x, y, 0); }
void PW::s4() { MM m; int x, y; f2((int)&x, (int)&y); f1(ga + 1); f1(gc + 1); f1(count + 14); f2(x, y); }
void PW::s5() { MM m; int x, y; f2((int)&x, (int)&y); f1(ga + 1); f1(gc + 1); f1(nv(count) + 14); f2(x, y); }
void PW::s6() { MM m; int x, y; f2((int)&x, (int)&y); f1(ga + 1); f1(gc + 1); f1(nv(count)); f2(x, y); }
void G__s1() { int x, y; f2((int)&x, (int)&y); f1(ga + 1); f1(gc + 1); f1(gb + 14); f2(x, y); }
void G__s2() { int x, y; f2((int)&x, (int)&y); f1(ga + 1); f1(gc + 1); f1(nv(gb) + 14); f2(x, y); }
void G__s3() { int x, y; f2((int)&x, (int)&y); f1(ga + 1); f1(gc + 1); int v = gb; f1(v + 14); f2(x, y); }
void G__s4() { int x, y; f2((int)&x, (int)&y); f1(ga + 1); f1(gc + 1); f1(nv(gb)); f2(x, y); }
void G__s5() { int x, y; f2((int)&x, (int)&y); f1(ga + 1); f1(gc + 1); f1(gb); f2(x, y); }
