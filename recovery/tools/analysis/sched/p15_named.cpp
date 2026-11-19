#include "probe.h"
extern int ge, gf, gg, gh;
extern int *gp;
inline int ident(int x) { return x; }
inline int at(int k) { return gp[k]; }
// a single-use named local vs the inline expression
void n__expr() { f1(ga + 1); f0(); f2(gc, gd); f1(gf); }
void n__named1() { f1(ga + 1); f0(); int t = gc; f2(t, gd); f1(gf); }
void n__named2() { f1(ga + 1); f0(); int t = gd; f2(gc, t); f1(gf); }
void n__inl1() { f1(ga + 1); f0(); f2(ident(gc), gd); f1(gf); }
void n__inl2() { f1(ga + 1); f0(); f2(gc, ident(gd)); f1(gf); }
// pointer-indexed through an inline vs written out
void p__expr() { f1(ga + 1); f0(); f2(gp[5], gd); f1(gf); }
void p__inl() { f1(ga + 1); f0(); f2(at(5), gd); f1(gf); }
void p__named() { f1(ga + 1); f0(); int t = gp[5]; f2(t, gd); f1(gf); }
// a cast/derived named local
void d__expr() { f1(ga + 1); f0(); f2(gc + 4, gd); f1(gf); }
void d__named() { f1(ga + 1); f0(); int t = gc + 4; f2(t, gd); f1(gf); }
