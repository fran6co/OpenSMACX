#!/usr/bin/env python3
"""inl2.py: set_ai's unpack written as calls of a getter inline (the setter is popup_set_write_check_bit
0x0051DCC0, so the reader is plausibly its inline twin).  Probes which getter bodies leave one byte load
in a register with a movsx at each use (the image) rather than one CSE'd movsx.
Usage: uv run python inl2.py   (FE_PFX_BASE, default 21)"""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))) + '/inline')
import ib
PFX = os.environ.get('FE_PFX_BASE', '21')
HDR = '''#include "probe.h"
struct P { int pad[248]; int a, b, c, d; int pad2[400]; };
extern P Pl[];
extern int GI;
int x_pop(const char *, int, int);
void synch(int);
'''
G = {
 'c_once': ('inline int g_c_once(char f, int n) { return (f & (1 << n)) >> n; }', ('GI', '(char)GI')),
 'c_twice': ('inline int g_c_twice(char f, int n) { return f ? (f & (1 << n)) >> n : 0; }', ('GI',)),
 'c_local': ('inline int g_c_local(char f, int n) { int m = 1 << n; return (f & m) >> n; }', ('GI',)),
 'c_ref': ('inline int g_c_ref(const char &f, int n) { return (f & (1 << n)) >> n; }', ('GI',)),
 'c_addr': ('inline int g_c_addr(char f, int n) { char *p = &f; return (*p & (1 << n)) >> n; }', ('GI',)),
 'i_once': ('inline int g_i_once(int f, int n) { return ((char)f & (1 << n)) >> n; }', ('GI',)),
 'p_int': ('inline int g_p_int(char *f, int n) { int v = *f; return (v & (1 << n)) >> n; }', ('(char *)&GI',)),
 'p_once': ('inline int g_p_once(char *f, int n) { return (*f & (1 << n)) >> n; }', ('(char *)&GI',)),
 'a_int': ('inline int g_a_int(char f, int n) { char *p = &f; int v = *p; return (v & (1 << n)) >> n; }', ('GI',)),
 'a_int2': ('inline int i_a_int2(int v, int n) { return (v & (1 << n)) >> n; }\ninline int g_a_int2(char f, int n) { char *p = &f; return i_a_int2(*p, n); }', ('GI',)),
 'c_int2': ('inline int i_c_int2(int v, int n) { return (v & (1 << n)) >> n; }\ninline int g_c_int2(char f, int n) { return i_c_int2(f, n); }', ('GI',)),
}
src = HDR
names = []
for k, (g, args) in G.items():
    src += g + '\n'
    for ai, a in enumerate(args):
        n = '%s_%d' % (k, ai); names.append(n)
        src += 'void f_%s(int id) { if (x_pop("X", 1, 0) >= 0) { Pl[id].a = g_%s(%s, 0); Pl[id].b = g_%s(%s, 1); Pl[id].c = g_%s(%s, 2); Pl[id].d = g_%s(%s, 3); synch(id); } }\n' % (n, k, a, k, a, k, a, k, a)
asm, _ = ib.compile_asm(None, PFX, text=src)
F = {f[0].split('(')[0].strip(): f[2] for f in ib.functions(asm)}
for n in names:
    L = [l for kk, l in F.items() if kk.endswith('f_' + n)][0]
    i = max(k for k, l in enumerate(L) if 'x_pop' in l)
    print('%-12s movsx=%d | %s' % (n, sum('movsx' in l for l in L[i:]), ' ; '.join(L[i + 3:])[:260]))
