#include "probe.h"
extern int ge, gf;
// single-use vs multi-use value after one cursor step
void use__single() { f1(ga + 1); f0(); gb = gc; }
void use__multi() { f1(ga + 1); f0(); gb = gc; gd = gc; }
void use__multi2() { f1(ga + 1); f0(); gb = gc; gd = gc; ge = gf; }
void use__single2() { f1(ga + 1); f0(); gb = gc; ge = gf; }
// deferred pop pending vs flushed
void pop__pending() { f1(ga + 1); f2(gb, gc); }
void pop__pending2() { f1(ga + 1); gd = 1; f2(gb, gc); }
void pop__flushed() { f1(ga + 1); f0(); f2(gb, gc); }
void pop__store() { f1(ga + 1); gd = ge; f2(gb, gc); }
void pop__store2() { f1(ga + 1); f0(); gd = ge; f2(gb, gc); }
// args after a call result arg
void cr__a() { f2(ga, g0()); }
void cr__b() { f2(g0(), ga); }
void cr__c() { f1(ga + 1); f2(ga, g0()); }
void cr__d() { f1(ga + 1); f2(g0(), ga); }
