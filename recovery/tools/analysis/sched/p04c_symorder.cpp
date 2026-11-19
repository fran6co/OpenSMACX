#include "probe.h"
// commutative operand order for memory-homed locals (address taken), params, globals, struct fields
void take(int *);
int L__xy() { int x, y; take(&x); take(&y); f0(); return x + y; }
int L__yx() { int y, x; take(&y); take(&x); f0(); return x + y; }
int L__decl_yx() { int y, x; take(&x); take(&y); f0(); return x + y; }
int P__ab(int a, int b) { take(&a); take(&b); f0(); return a + b; }
int P__ba(int a, int b) { take(&b); take(&a); f0(); return a + b; }
int G__xy() { f1(ga); f1(gb); f0(); return ga + gb; }
int G__yx() { f1(gb); f1(ga); f0(); return ga + gb; }
int G__none() { f0(); return gb + ga; }
int R__lr(RECT *src) { RECT r = *src; take((int *)&r); r.left += 1; r.right += 2; f0(); return r.left + r.right; }
int R__rl(RECT *src) { RECT r = *src; take((int *)&r); r.right += 2; r.left += 1; f0(); return r.left + r.right; }
// other commutative ops: *, &, |, ==
int X__mul_xy() { int x, y; take(&x); take(&y); f0(); return x * y; }
int X__mul_yx() { int x, y; take(&y); take(&x); f0(); return x * y; }
int X__and_xy() { int x, y; take(&x); take(&y); f0(); return x & y; }
int X__and_yx() { int x, y; take(&y); take(&x); f0(); return x & y; }
// non-commutative (control): x - y
int X__sub_xy() { int x, y; take(&x); take(&y); f0(); return x - y; }
int X__sub_yx() { int x, y; take(&y); take(&x); f0(); return x - y; }
// comparison
int X__lt_xy() { int x, y; take(&x); take(&y); f0(); return x < y ? 3 : 4; }
int X__lt_yx() { int x, y; take(&y); take(&x); f0(); return x < y ? 3 : 4; }
