#include "probe.h"
extern int ge, gf, gg, gh;
// does a multi-use value advance the cursor? which register does it take?
void m__s_s_s() { f1(ga + 1); f0(); gb = gc; ge = gf; gg = gh; }
void m__m_s_s() { f1(ga + 1); f0(); gb = gc; gd = gc; ge = gf; gg = gh; }
void m__s_m_s() { f1(ga + 1); f0(); ge = gf; gb = gc; gd = gc; gg = gh; }
void m__s_s_m() { f1(ga + 1); f0(); ge = gf; gg = gh; gb = gc; gd = gc; }
void m2__m_s() { f1(ga + 1); f1(gb + 1); f0(); gb = gc; gd = gc; ge = gf; }
void m2__s_m() { f1(ga + 1); f1(gb + 1); f0(); ge = gf; gb = gc; gd = gc; }
void m3__m_s() { f0(); gb = gc; gd = gc; ge = gf; }
void m3__s_m() { f0(); ge = gf; gb = gc; gd = gc; }
// multi-use as arguments
void ma__a() { f1(ga + 1); f0(); f2(gc, gc); f1(gf); }
void ma__b() { f1(ga + 1); f0(); f2(gc + 1, gc + 2); f1(gf); }
