#include "probe.h"
struct M { int x, y, z; int c0(); int c1(); int c2(); int c3(); int c4(); int c5(); int c6(); int c7(); int c8(); int c9(); };
int M::c0() { if (z) f1(y); f1(x); f0(); return x + y; }
int M::c1() { if (z) f1(x); f1(y); f0(); return x + y; }
int M::c2() { if (z) f1(y); else f1(x); f0(); return x + y; }
int M::c3() { if (z) f1(y); f1(z); f0(); return x + y; }
int M::c4() { f1(y); if (z) f1(x); f0(); return x + y; }
int M::c5() { if (z) f1(x); f0(); return x + y; }
int M::c6() { if (z) f1(y); f0(); return x + y; }
int M::c7() { if (z) { f1(y); f1(x); } f0(); return x + y; }
int M::c8() { if (z) { f1(x); f1(y); } f0(); return x + y; }
int M::c9() { if (z) f0(); f1(y); f1(x); f0(); return x + y; }
