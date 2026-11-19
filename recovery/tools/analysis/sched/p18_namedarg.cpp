#include "probe.h"
// Named single-use local as a call argument: when is it a cursor allocation
// (forward-substituted into the push) and when a lowest-free "variable"?
// Cursor before the probe site: eax (f1(ga+1)) or ecx (two of them).
extern int *gp;
extern void f2s(int, int);
// left arg named, another arg evaluated between def and use
void L1__expr() { f1(ga + 1); f2(gp[5], gd); f1(gc); }
void L1__named() { f1(ga + 1); int v = gp[5]; f2(v, gd); f1(gc); }
void L2__expr() { f1(ga + 1); f1(gb + 1); f2(gp[5], gd); f1(gc); }
void L2__named() { f1(ga + 1); f1(gb + 1); int v = gp[5]; f2(v, gd); f1(gc); }
void L3__expr() { f1(ga + 1); f1(gb + 1); f2(gc, gd); f1(gc); }
void L3__named() { f1(ga + 1); f1(gb + 1); int v = gc; f2(v, gd); f1(gc); }
void L4__expr() { f1(ga + 1); f2(gc, gd); f1(gb); }
void L4__named() { f1(ga + 1); int v = gc; f2(v, gd); f1(gb); }
// right arg named (adjacent to its def in evaluation order)
void R1__expr() { f1(ga + 1); f2(gd, gp[5]); f1(gc); }
void R1__named() { f1(ga + 1); int v = gp[5]; f2(gd, v); f1(gc); }
void R2__expr() { f1(ga + 1); f1(gb + 1); f2(gd, gp[5]); f1(gc); }
void R2__named() { f1(ga + 1); f1(gb + 1); int v = gp[5]; f2(gd, v); f1(gc); }
void R4__expr() { f1(ga + 1); f2(gd, gc); f1(gb); }
void R4__named() { f1(ga + 1); int v = gc; f2(gd, v); f1(gb); }
// middle of three
void M4__expr() { f1(ga + 1); f3(gd, gc, gb); f1(gb); }
void M4__named() { f1(ga + 1); int v = gc; f3(gd, v, gb); f1(gb); }
// named param of an inline helper
inline void cat2(int d, int s) { f2(d, s); }
void I4__expr() { f1(ga + 1); f2(gc, gd); f1(gb); }
void I4__inl() { f1(ga + 1); cat2(gc, gd); f1(gb); }
void I2__expr() { f1(ga + 1); f1(gb + 1); f2(gc, gd); f1(gb); }
void I2__inl() { f1(ga + 1); f1(gb + 1); cat2(gc, gd); f1(gb); }
