#include "probe.h"
struct RW { int pad[3]; RECT r; int flag; int out[4]; void s1(); void s2(); void s3(); void s4(); void s5(); void s6(); void s7(); void s8(); };
inline int rw(RECT *r) { return r->right - r->left; }
inline int rwc(const RECT &r) { return r.right - r.left; }
void RW::s1() { f1(0); RECT *p = &r; f1((int)p); unsigned in = flag ? 0x4C : 0; out[0] = p->left + in; out[1] = p->top; out[2] = rw(p) - in; out[3] = 0; }
void RW::s2() { f1(0); RECT *p = &r; f1((int)p); int in = flag ? 0x4C : 0; out[0] = p->left + in; out[1] = p->top; out[2] = (unsigned)rw(p) - in; out[3] = 0; }
void RW::s3() { f1(0); RECT *p = &r; f1((int)p); int in = flag ? 0x4C : 0; out[0] = p->left + in; out[1] = p->top; out[2] = -in + rw(p); out[3] = 0; }
void RW::s4() { f1(0); RECT *p = &r; f1((int)p); int in = flag ? 0x4C : 0; out[0] = p->left + in; out[1] = p->top; out[2] = rw(p); out[2] -= in; out[3] = 0; }
void RW::s5() { f1(0); RECT *p = &r; f1((int)p); int in = flag ? 0x4C : 0; out[0] = p->left + in; out[1] = p->top; out[2] = p->right - in; out[2] -= p->left; out[3] = 0; }
void RW::s6() { f1(0); RECT *p = &r; f1((int)p); int in = flag ? 0x4C : 0; out[0] = p->left + in; out[1] = p->top; int w = p->right - in; out[2] = w - p->left; out[3] = 0; }
void RW::s7() { f1(0); f1((int)&r); int in = flag ? 0x4C : 0; out[0] = r.left + in; out[1] = r.top; out[2] = rwc(r) - in; out[3] = 0; }
void RW::s8() { f1(0); RECT *p = &r; f1((int)p); int in = flag ? 0x4C : 0; out[0] = p->left + in; out[1] = p->top; out[2] = rw(p) + -in; out[3] = 0; }
