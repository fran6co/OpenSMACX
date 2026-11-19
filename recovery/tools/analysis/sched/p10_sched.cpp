#include "probe.h"
struct O2 { int get(int); };
extern O2 st;
extern int *lab;
void g2(int *, int *);
// store then return 0
int R__a(int *p) { f0(); ga = *p; return 0; }
int R__b(int *p) { f0(); int v = *p; ga = v; return 0; }
int R__c(int *p, int c) { f0(); if (c) { ga = *p; return 0; } gb = 5; return 0; }
int R__d(int *p) { int buf[4]; f1((int)buf); ga = buf[2]; return 0; }
int R__e() { int buf[5]; f1((int)buf); ga = buf[2]; return 0; }
// two stores of loaded values
void S__ab() { f0(); ga = gc; gb = gd; }
void S__ba() { f0(); gb = gd; ga = gc; }
// receiver of a later thiscall vs local stores
void T__a(int a, int b) { f0(); int y = b; int x = a; int r = st.get(lab[0x33A]); g2(&x, &y); f1(r); }
void T__b(int a, int b) { f0(); int r = st.get(lab[0x33A]); int y = b; int x = a; g2(&x, &y); f1(r); }
void T__c(int a, int b) { f0(); int x = a; int y = b; int r = st.get(lab[0x33A]); g2(&x, &y); f1(r); }
