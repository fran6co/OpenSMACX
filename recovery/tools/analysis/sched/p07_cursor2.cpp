#include "probe.h"
void aftercall__p0() {  f0(); f0(); f1(ga + 1); f1(gb + 1); f1(gc + 1); }
void aftercall__p1() { f1(ga + 1); f0(); f0(); f1(ga + 1); f1(gb + 1); f1(gc + 1); }
void aftercall__p2() { f1(ga + 1); f1(gb + 1); f0(); f0(); f1(ga + 1); f1(gb + 1); f1(gc + 1); }
void seq__p0() {  f0(); ga = gb; gc = gd; ga = gc; gb = gd; }
void seq__p1() { f1(ga + 1); f0(); ga = gb; gc = gd; ga = gc; gb = gd; }
void seq__p2() { f1(ga + 1); f1(gb + 1); f0(); ga = gb; gc = gd; ga = gc; gb = gd; }
void ifelse__p0() {  f0(); if (g0()) { ga = gb; } else { gc = gd; gb = ga; } gd = gc; }
void ifelse__p1() { f1(ga + 1); f0(); if (g0()) { ga = gb; } else { gc = gd; gb = ga; } gd = gc; }
void ifelse__p2() { f1(ga + 1); f1(gb + 1); f0(); if (g0()) { ga = gb; } else { gc = gd; gb = ga; } gd = gc; }
void loop__p0() {  f0(); for (int i = 0; i < 10; ++i) { garr[i] = gb; } gd = gc; }
void loop__p1() { f1(ga + 1); f0(); for (int i = 0; i < 10; ++i) { garr[i] = gb; } gd = gc; }
void loop__p2() { f1(ga + 1); f1(gb + 1); f0(); for (int i = 0; i < 10; ++i) { garr[i] = gb; } gd = gc; }
void two_live__p0() {  f0(); f2(ga + 1, gb + 2); gc = gd; }
void two_live__p1() { f1(ga + 1); f0(); f2(ga + 1, gb + 2); gc = gd; }
void two_live__p2() { f1(ga + 1); f1(gb + 1); f0(); f2(ga + 1, gb + 2); gc = gd; }
void three_live__p0() {  f0(); f3(ga + 1, gb + 2, gc + 3); gd = ga; }
void three_live__p1() { f1(ga + 1); f0(); f3(ga + 1, gb + 2, gc + 3); gd = ga; }
void three_live__p2() { f1(ga + 1); f1(gb + 1); f0(); f3(ga + 1, gb + 2, gc + 3); gd = ga; }
