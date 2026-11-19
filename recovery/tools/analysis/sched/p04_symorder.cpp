#include "probe.h"
// Commutative x + y: the operand whose symbol is numbered LATER is loaded first (destination).
// What numbers a member symbol: first direct reference? inline expansions? stores vs reads? this-> vs ptr?
struct M { int x, y, z;
  int a0(); int a1(); int a2(); int a3(); int a4(); int a5(); int a6(); int a7(); int a8(); int a9();
  int gety() { return y; } void sety(int v) { y = v; } int b0(); int b1(); int b2(); int b3(); };
inline int ry(M *m) { return m->y; }
int M::a0() { f1(x); f1(y); f0(); return x + y; }          // x first
int M::a1() { f1(y); f1(x); f0(); return x + y; }          // y first
int M::a2() { f1(gety()); f1(x); f0(); return x + y; }     // y via inline member first
int M::a3() { f1(ry(this)); f1(x); f0(); return x + y; }   // y via inline free fn first
int M::a4() { sety(3); f1(x); f0(); return x + y; }        // y store via inline first
int M::a5() { y = 3; f1(x); f0(); return x + y; }          // y store first
int M::a6() { M *p = this; f1(p->y); f1(x); f0(); return x + y; }  // through local ptr
int M::a7() { if (z) f1(y); f1(x); f0(); return x + y; }   // y first in a branch
int M::a8() { f1(z); f0(); return x + y; }                 // neither
int M::a9() { f0(); return y + x; }                        // neither, reversed
int M::b0() { f1(x); f1(gety()); f0(); return x + y; }     // x direct, then y inline
int M::b1() { f1(gety()); f0(); return x + y; }            // only y inline
int M::b2() { int t = x; f1(y); f1(t); f0(); return x + y; }
int M::b3() { f1(y); f0(); return x + y + z; }
