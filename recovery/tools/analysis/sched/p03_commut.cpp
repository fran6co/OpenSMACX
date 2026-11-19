#include "probe.h"
// Which operand of a commutative + is loaded first (into the destination register)?
extern int gx, gy;
struct M { int x, y, z; int sum(); int sum_r(); int pre_x(); int pre_y(); int pre_yx(); int dec_only(); };
int M::sum() { f0(); return x + y; }
int M::sum_r() { f0(); return y + x; }
int M::pre_x() { f1(x); f0(); return x + y; }
int M::pre_y() { f1(y); f0(); return x + y; }
int M::pre_yx() { f1(y); f1(x); f0(); return x + y; }
int G__ab() { f0(); return gx + gy; }
int G__ba() { f0(); return gy + gx; }
int G__prex() { f1(gx); f0(); return gx + gy; }
int G__prey() { f1(gy); f0(); return gx + gy; }
int L__ab(int a, int b) { f0(); return a + b; }
int L__ba(int a, int b) { f0(); return b + a; }
int L__preb(int a, int b) { f1(b); f0(); return a + b; }
// push argument of a + b into a call
void C__xy(M *m) { f0(); f1(m->x + m->y); }
void C__yx(M *m) { f0(); f1(m->y + m->x); }
void C__prey(M *m) { f1(m->y); f0(); f1(m->x + m->y); }
