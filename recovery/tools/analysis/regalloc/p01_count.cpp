#include "probe.h"
// uses 4,3,2,1 declared a,b,c,d
void c_4321() { D(a); D(b); D(c); D(d); U4(a); U3(b); U2(c); U(d); }
// uses 1,2,3,4 declared a,b,c,d
void c_1234() { D(a); D(b); D(c); D(d); U(a); U2(b); U3(c); U4(d); }
// all equal (2 each)
void c_2222() { D(a); D(b); D(c); D(d); U2(a); U2(b); U2(c); U2(d); }
// all equal, uses in reverse order of definition
void c_2222r() { D(a); D(b); D(c); D(d); U2(d); U2(c); U2(b); U2(a); }
// all equal, definitions in reverse order of declaration
void c_2222d() { int a, b, c, d; S(d); S(c); S(b); S(a); U2(a); U2(b); U2(c); U2(d); }
// 3 candidates only: 1,2,3
void c_123() { D(a); D(b); D(c); U(a); U2(b); U3(c); }
// 2 candidates
void c_12() { D(a); D(b); U(a); U2(b); }
// 1 candidate
void c_1() { D(a); U2(a); }
// five candidates 5,4,3,2,1
void c_54321() { D(a); D(b); D(c); D(d); D(e); U5(a); U4(b); U3(c); U2(d); U(e); }
// five candidates 1,2,3,4,5
void c_12345() { D(a); D(b); D(c); D(d); D(e); U(a); U2(b); U3(c); U4(d); U5(e); }
// six: 3,3,3,1,1,1
void c_333111() { D(a); D(b); D(c); D(d); D(e); D(f); U3(a); U3(b); U3(c); U(d); U(e); U(f); }
// six: 1,1,1,3,3,3
void c_111333() { D(a); D(b); D(c); D(d); D(e); D(f); U(a); U(b); U(c); U3(d); U3(e); U3(f); }
