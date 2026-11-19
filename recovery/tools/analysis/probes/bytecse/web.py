#!/usr/bin/env python3
"""web.py: set_ai's unpack with a char variable whose web cannot be copy-propagated into a load
(function-scope char reused for the flag build, address taken, defined twice).
Usage: uv run python web.py"""
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
void takec(char *);
inline int bi(int v, int m, int sh) { return (v & m) >> sh; }
inline unsigned bu(unsigned v, unsigned m, int sh) { return (v & m) >> sh; }
inline int bn(int v, int m, int sh) { return ((v & m) >> sh) != 0; }
'''
F = {
 'perv': '''int fl = (*a != 0) ? 1 : 0; if (*b) fl |= 2; else fl &= ~2; GI = fl;
  if (x_pop("X", 1, 0) >= 0) { char s = (char)GI; { int v = s; *a = v & 1u; } { int v = s; *b = (v & 2u) >> 1; } { int v = s; *c = (v & 4u) >> 2; } { int v = s; *d = (v & 8u) >> 3; } synch(id); }''',
 'perv_if': '''int fl = (*a != 0) ? 1 : 0; if (*b) fl |= 2; else fl &= ~2; GI = fl;
  if (x_pop("X", 1, 0) >= 0) { char s = (char)GI; if (id >= -100) { int v = s; *a = v & 1u; } if (id >= -101) { int v = s; *b = (v & 2u) >> 1; } { int v = s; *c = (v & 4u) >> 2; } { int v = s; *d = (v & 8u) >> 3; } synch(id); }''',
 'sw4': '''int fl = (*a != 0) ? 1 : 0; if (*b) fl |= 2; else fl &= ~2; GI = fl;
  if (x_pop("X", 1, 0) >= 0) { char s = (char)GI; switch (id) { case 0: *a = s & 1u; break; case 1: *b = (s & 2u) >> 1; break; case 2: *c = (s & 4u) >> 2; break; default: *d = (s & 8u) >> 3; } synch(id); }''',

 'inl_int': '''int fl = (*a != 0) ? 1 : 0; if (*b) fl |= 2; else fl &= ~2; GI = fl;
  if (x_pop("X", 1, 0) >= 0) { char s = (char)GI; *a = bi(s, 1, 0); *b = bi(s, 2, 1); *c = bi(s, 4, 2); *d = bi(s, 8, 3); synch(id); }''',
 'inl_uns': '''int fl = (*a != 0) ? 1 : 0; if (*b) fl |= 2; else fl &= ~2; GI = fl;
  if (x_pop("X", 1, 0) >= 0) { char s = (char)GI; *a = bu(s, 1, 0); *b = bu(s, 2, 1); *c = bu(s, 4, 2); *d = bu(s, 8, 3); synch(id); }''',
 'inl_g': '''int fl = (*a != 0) ? 1 : 0; if (*b) fl |= 2; else fl &= ~2; GI = fl;
  if (x_pop("X", 1, 0) >= 0) { *a = bu((char)GI, 1, 0); *b = bu((char)GI, 2, 1); *c = bu((char)GI, 4, 2); *d = bu((char)GI, 8, 3); synch(id); }''',
 'inl_ne': '''int fl = (*a != 0) ? 1 : 0; if (*b) fl |= 2; else fl &= ~2; GI = fl;
  if (x_pop("X", 1, 0) >= 0) { char s = (char)GI; *a = bn(s, 1, 0); *b = bn(s, 2, 1); *c = bn(s, 4, 2); *d = bn(s, 8, 3); synch(id); }''',

 'ifs_s': '''int fl = (*a != 0) ? 1 : 0; if (*b) fl |= 2; else fl &= ~2; GI = fl;
  if (x_pop("X", 1, 0) >= 0) { char s = (char)GI; if (id >= -100) *a = s & 1; if (id >= -101) *b = (s & 2) >> 1; if (id >= -102) *c = (s & 4) >> 2; if (id >= -103) *d = (s & 8) >> 3; synch(id); }''',
 'ifs_gs': '''int fl = (*a != 0) ? 1 : 0; if (*b) fl |= 2; else fl &= ~2; GI = fl;
  if (x_pop("X", 1, 0) >= 0) { if (id >= -100) Pl[id].a = (char)GI & 1; if (id >= -101) Pl[id].b = ((char)GI & 2) >> 1; if (id >= -102) Pl[id].c = ((char)GI & 4) >> 2; if (id >= -103) Pl[id].d = ((char)GI & 8) >> 3; synch(id); }''',
 'ifs_ne': '''int fl = (*a != 0) ? 1 : 0; if (*b) fl |= 2; else fl &= ~2; GI = fl;
  if (x_pop("X", 1, 0) >= 0) { char s = (char)GI; if (id >= -100) *a = (s & 1) != 0; if (id >= -101) *b = ((s & 2) >> 1) != 0; if (id >= -102) *c = ((s & 4) >> 2) != 0; if (id >= -103) *d = ((s & 8) >> 3) != 0; synch(id); }''',

 'dowhile': '''int fl = (*a != 0) ? 1 : 0; if (*b) fl |= 2; else fl &= ~2; GI = fl;
  if (x_pop("X", 1, 0) >= 0) { char s = (char)GI; do { *a = s & 1u; } while (0); do { *b = (s & 2u) >> 1; } while (0); do { *c = (s & 4u) >> 2; } while (0); do { *d = (s & 8u) >> 3; } while (0); synch(id); }''',
 'labels': '''int fl = (*a != 0) ? 1 : 0; if (*b) fl |= 2; else fl &= ~2; GI = fl;
  if (x_pop("X", 1, 0) >= 0) { char s = (char)GI; L1: *a = s & 1u; L2: *b = (s & 2u) >> 1; L3: *c = (s & 4u) >> 2; L4: *d = (s & 8u) >> 3; synch(id); if (id == -9) goto L1; if (id == -8) goto L2; if (id == -7) goto L3; if (id == -6) goto L4; }''',
 'ifs': '''int fl = (*a != 0) ? 1 : 0; if (*b) fl |= 2; else fl &= ~2; GI = fl;
  if (x_pop("X", 1, 0) >= 0) { char s = (char)GI; if (id >= -100) *a = s & 1u; if (id >= -101) *b = (s & 2u) >> 1; if (id >= -102) *c = (s & 4u) >> 2; if (id >= -103) *d = (s & 8u) >> 3; synch(id); }''',
 'ifs_g': '''int fl = (*a != 0) ? 1 : 0; if (*b) fl |= 2; else fl &= ~2; GI = fl;
  if (x_pop("X", 1, 0) >= 0) { if (id >= -100) Pl[id].a = (char)GI & 1u; if (id >= -101) Pl[id].b = ((char)GI & 2u) >> 1; if (id >= -102) Pl[id].c = ((char)GI & 4u) >> 2; if (id >= -103) Pl[id].d = ((char)GI & 8u) >> 3; synch(id); }''',

 'twodef': '''char s = (*a != 0) ? 1 : 0;
  if (*b) s |= 2; else s &= ~2;
  GI = s;
  if (x_pop("X", 1, 0) >= 0) { s = (char)GI; *a = s & 1u; *b = (s & 2u) >> 1; *c = (s & 4u) >> 2; *d = (s & 8u) >> 3; synch(id); }''',
 'twodef_int': '''int fl = (*a != 0) ? 1 : 0;
  if (*b) fl |= 2; else fl &= ~2;
  GI = fl;
  if (x_pop("X", 1, 0) >= 0) { fl = GI; *a = (char)fl & 1u; *b = ((char)fl & 2u) >> 1; *c = ((char)fl & 4u) >> 2; *d = ((char)fl & 8u) >> 3; synch(id); }''',
 'addr': '''int fl = (*a != 0) ? 1 : 0;
  if (*b) fl |= 2; else fl &= ~2;
  GI = fl;
  if (x_pop("X", 1, 0) >= 0) { char s = (char)GI; if (id < -5) takec(&s); *a = s & 1u; *b = (s & 2u) >> 1; *c = (s & 4u) >> 2; *d = (s & 8u) >> 3; synch(id); }''',
 'cfl': '''char fl = (*a != 0) ? 1 : 0;
  if (*b) fl |= 2; else fl &= ~2;
  GI = fl;
  if (x_pop("X", 1, 0) >= 0) { *a = (char)GI & 1u; *b = ((char)GI & 2u) >> 1; *c = ((char)GI & 4u) >> 2; *d = ((char)GI & 8u) >> 3; synch(id); }''',
}
src = HDR
for n, b in F.items():
    src += 'void f_%s(int id) {\n  int *a = &Pl[id].a; int *b = &Pl[id].b; int *c = &Pl[id].c; int *d = &Pl[id].d;\n  %s\n}\n' % (n, b)
asm, _ = ib.compile_asm(None, PFX, text=src)
FN = {f[0].split('(')[0].strip(): f[2] for f in ib.functions(asm)}
for n in F:
    L = [l for k, l in FN.items() if k.endswith('f_' + n)][0]
    print('%-12s movsx=%d | %s' % (n, sum('movsx' in l for l in L), ' ; '.join(L)[:900]))
