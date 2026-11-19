#include "probe.h"
extern int ge, gf, gg, gh;
extern int *gp;
struct ST { int get(int); };
extern ST gst;
void a__expr() { f1(ga + 1); f2(gp[5], gd); f1(gf); }
void a__named() { f1(ga + 1); int v = gp[5]; f2(v, gd); f1(gf); }
void b__expr() { f1(ga + 1); f1(gp[5]); f1(gf); f1(gg); }
void b__named() { f1(ga + 1); int v = gp[5]; f1(v); f1(gf); f1(gg); }
void b__namedg() { f1(ga + 1); int v = gc; f1(v); f1(gf); f1(gg); }
void b__namedadd() { f1(ga + 1); int v = gc + 3; f1(v); f1(gf); f1(gg); }
void c__expr() { f1(ga + 1); f1(gst.get(gp[5])); f1(gf); f1(gg); }
void c__named() { f1(ga + 1); int v = gp[5]; f1(gst.get(v)); f1(gf); f1(gg); }
void c__namedg() { f1(ga + 1); int v = gc; f1(gst.get(v)); f1(gf); f1(gg); }
void d__expr() { f1(ga + 1); f1(gb + 1); f1(gp[5]); f1(gf); f1(gg); }
void d__named() { f1(ga + 1); f1(gb + 1); int v = gp[5]; f1(v); f1(gf); f1(gg); }
