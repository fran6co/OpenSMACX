#!/usr/bin/env python3
"""run.py [name ...]: what stops VC6 strength-reducing `tab[i]` in a loop into a stepped pointer
(MATCHING_GUIDE 2.7, "SR blocking"). Each variant is one function; prints PTR / OFF / IDX (see cls) and the
callee-saved registers used.
SHOW=name prints that variant's listing.  FE_PFX_BASE selects the compiler prefix (default 21)."""
import sys, os, re
H = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(os.path.dirname(H)) + '/inline')
import ib
PFX = os.environ.get('FE_PFX_BASE', '21')
HDR = '''#include "probe.h"
extern int tab[64]; extern int tab2[64]; extern short stab[64];
extern int gsel; extern int cnt_out; extern int arr8[8];
int hasx(int, int);
struct B { char a, b, fac, d; int pad[7]; }; extern B bases[];
struct SP { int pb[64]; int &operator[](int i) { return pb[i]; } };
struct GD { int x[10]; SP sp; }; extern GD gd;
inline int hasi(int p, int f) { int b = tab[p]; if (b < 0) return 0; return bases[b].fac == f; }
inline int hasg(int p, int f) { int b = gd.sp[p]; if (b < 0) return 0; return bases[b].fac == f; }
inline int at_(int i) { return tab[i]; }
extern int mapx; extern int flg;
inline int wrapx(int x) { if (!(flg & 1)) { if (x < 0) x += mapx; else if (x >= mapx) x -= mapx; } return x; }
int onmap(int, int); int isoc(int, int); void spot(int, int);
'''
V = {
 # tab[i] the only use of i
 'only':        'int c = 0; for (int i = 0; i < 64; i++) { if (tab[i] == gsel) c++; } return c;',
 # i also passed to a call
 'call':        'int c = 0; for (int i = 0; i < 64; i++) { if (tab[i] == gsel) c++; use(i); } return c;',
 # i also as value (i/8 subscript)
 'div8':        'int c = 0; for (int i = 0; i < 64; i++) { if (tab[i] == gsel) c++; arr8[i / 8]++; } return c;',
 # i compared
 'cmp':         'int c = 0; for (int i = 0; i < 64; i++) { if (tab[i] == gsel) c++; if (i == a) c += 2; } return c;',
 # census shape: extern has + i/8
 'census_x':    'int c = 0; for (int i = 0; i < 64; i++) { if (tab[i] == gsel) c++; if (hasx(i, a)) arr8[i / 8]++; } return c;',
 # census shape with inline has reading tab[i] again
 'census_i':    'int c = 0; for (int i = 0; i < 64; i++) { if (tab[i] == gsel) c++; if (hasi(i, a)) arr8[i / 8]++; } return c;',
 # same through struct operator[]
 'census_g':    'int c = 0; for (int i = 0; i < 64; i++) { if (gd.sp[i] == gsel) c++; if (hasg(i, a)) arr8[i / 8]++; } return c;',
 'census_g_c':  'int c = 0; for (int i = 0; i < 64; i++) { if (gd.sp[i] == gsel) c++; if (hasg(i, a)) arr8[i / 8]++; } cnt_out = c; return 0;',
 'census_g_pb': 'int c = 0; for (int i = 0; i < 64; i++) { if (gd.sp.pb[i] == gsel) c++; if (hasg(i, a)) arr8[i / 8]++; } return c;',
 # count unknown
 'cnt_a':       'int c = 0; for (int i = 0; i < a; i++) { if (tab[i] == gsel) c++; use(i); } return c;',
 # i address-taken
 'addr':        'int c = 0; int i; for (i = 0; i < 64; i++) { if (tab[i] == gsel) c++; use(i); } use(*&i); hasx((int)&i, 0); return c;',
 # i modified in body
 'mod':         'int c = 0; for (int i = 0; i < 64; i++) { if (tab[i] == gsel) { c++; i++; } use(i); } return c;',
 # second table
 'two':         'int c = 0; for (int i = 0; i < 64; i++) { if (tab[i] == gsel) c++; if (tab2[i]) c += 3; use(i); } return c;',
 'two_only':    'int c = 0; for (int i = 0; i < 64; i++) { if (tab[i] == gsel) c++; if (tab2[i]) c += 3; } return c;',
 # short table
 'short':       'int c = 0; for (int i = 0; i < 64; i++) { if (stab[i] == gsel) c++; use(i); } return c;',
 # char / short counters
 'ctr_char':    'int c = 0; for (char i = 0; i < 64; i++) { if (tab[i] == gsel) c++; use(i); } return c;',
 'ctr_short':   'int c = 0; for (short i = 0; i < 64; i++) { if (tab[i] == gsel) c++; use(i); } return c;',
 'ctr_uns':     'int c = 0; for (unsigned i = 0; i < 64; i++) { if (tab[i] == gsel) c++; use(i); } return c;',
 # spellings
 'ptrarith':    'int c = 0; for (int i = 0; i < 64; i++) { if (*(tab + i) == gsel) c++; use(i); } return c;',
 'inl':         'int c = 0; for (int i = 0; i < 64; i++) { if (at_(i) == gsel) c++; use(i); } return c;',
 # function-scope i (declared outside, used after)
 'after':       'int c = 0; int i; for (i = 0; i < 64; i++) { if (tab[i] == gsel) c++; use(i); } return c + i;',
 # while loop
 'while':       'int c = 0; int i = 0; while (i < 64) { if (tab[i] == gsel) c++; use(i); i++; } return c;',
 # inner loop reading i
 'inner':       'int c = 0; for (int i = 0; i < 64; i++) { if (tab[i] == gsel) c++; for (int k = 0; k < b; k++) use(i); } return c;',
 # constant-left compare
 'kleft':       'int c = 0; for (int i = 0; 64 > i; i++) { if (tab[i] == gsel) c++; use(i); } return c;',
 'ne':          'int c = 0; for (int i = 0; i != 64; i++) { if (tab[i] == gsel) c++; use(i); } return c;',
 # load of gsel into a local before the loop
 'sel_local':   'int c = 0; int s = gsel; for (int i = 0; i < 64; i++) { if (tab[i] == s) c++; use(i); } return c;',

 # ---- census family (base_build 0x004F8931): no call in the loop, gsel hoistable
 'cs':          'int c = 0; for (int i = 0; i < 64; i++) { if (gd.sp[i] == gsel) c++; if (hasg(i, a)) arr8[i / 8]++; } use(c); return 0;',
 'cs_two':      'int c = 0; for (int i = 0; i < 64; i++) { if (gd.sp[i] == gsel) c++; if (hasg(i, a)) arr8[i / 8]++; tab2[i] = 0; } use(c); return 0;',
 'cs_next':     'int c = 0; for (int i = 0; i < 64; i++) { if (gd.sp[i] == gsel) c++; if (hasg(i, a)) arr8[i / 8]++; if (gd.sp[i+1]) c++; } use(c); return 0;',
 'cs_fs':       'int c = 0; int i; for (i = 0; i < 64; i++) { if (gd.sp[i] == gsel) c++; if (hasg(i, a)) arr8[i / 8]++; } use(c); for (i = 0; i < b; i++) use(i); return 0;',
 'cs_fs_addr':  'int c = 0; int i; for (i = 0; i < 64; i++) { if (gd.sp[i] == gsel) c++; if (hasg(i, a)) arr8[i / 8]++; } use(c); hasx((int)&i, 0); return 0;',
 'cs_glob':     'int c = 0; for (int i = 0; i < 64; i++) { if (gd.sp[i] == gsel) c++; if (hasg(i, a)) arr8[i / 8]++; cnt_out = i; } use(c); return 0;',
 'cs_c_glob':   'cnt_out = 0; for (int i = 0; i < 64; i++) { if (gd.sp[i] == gsel) cnt_out++; if (hasg(i, a)) arr8[i / 8]++; } return 0;',
 'cs_shr':      'int c = 0; for (int i = 0; i < 64; i++) { if (gd.sp[i] == gsel) c++; if (hasg(i, a)) arr8[i >> 3]++; } use(c); return 0;',
 'cs_noinl':    'int c = 0; for (int i = 0; i < 64; i++) { int bb = gd.sp[i]; if (bb == gsel) c++; if (bb >= 0 && bases[bb].fac == a) arr8[i / 8]++; } use(c); return 0;',
 'cs_local8':   'int c = 0; int l8[8]; for (int k = 0; k < 8; k++) l8[k] = 0; for (int i = 0; i < 64; i++) { if (gd.sp[i] == gsel) c++; if (hasg(i, a)) l8[i / 8]++; } use(c); use(l8[a]); return 0;',
 'cs_par':      'int c = 0; for (int i = 0; i < 64; i++) { if (gd.sp[i] == b) c++; if (hasg(i, a)) arr8[i / 8]++; } use(c); return 0;',

 # ---- what counts as a second i-indexed array
 't2_store':    'int c = 0; for (int i = 0; i < 64; i++) { if (tab[i] == gsel) c++; tab2[i] = c; } return c;',
 't2_short':    'int c = 0; for (int i = 0; i < 64; i++) { if (tab[i] == gsel) c++; if (stab[i]) c++; } return c;',
 't2_local':    'int c = 0; int l[64]; for (int i = 0; i < 64; i++) { l[i] = 0; if (tab[i] == gsel) c++; } use(l[a]); return c;',
 't2_local_rd': 'int c = 0; int l[64]; hasx((int)l, 0); for (int i = 0; i < 64; i++) { if (l[i]) c++; if (tab[i] == gsel) c++; } return c;',
 't2_struct':   'int c = 0; for (int i = 0; i < 64; i++) { if (tab[i] == gsel) c++; if (bases[i].fac) c++; } return c;',
 't2_samegd':   'int c = 0; for (int i = 0; i < 64; i++) { if (gd.sp[i] == gsel) c++; if (gd.x[i % 8]) c++; } return c;',
 't2_ptr':      'int c = 0; int *q = tab2; for (int i = 0; i < 64; i++) { if (tab[i] == gsel) c++; if (q[i]) c++; } return c;',
 't2_2d':       'int c = 0; for (int i = 0; i < 16; i++) { if (tab[i * 4] == gsel) c++; if (tab[i * 4 + 1]) c++; } return c;',
 't2_sameoff':  'int c = 0; for (int i = 0; i < 63; i++) { if (tab[i] == gsel) c++; if (tab[i + 1]) c++; } return c;',
 't2_call':     'int c = 0; for (int i = 0; i < 64; i++) { if (tab[i] == gsel) c++; if (tab2[i]) c++; use(c); } return c;',
 't3':          'int c = 0; for (int i = 0; i < 64; i++) { if (tab[i] == gsel) c++; if (tab2[i]) c++; if (stab[i]) c++; } return c;',

 # ---- spot_all radius loop (0x005B6983): two same-stride tables, unknown bound, calls, i compared
 'sp':          'int t = 0; for (int i = 0; i < (b ? 25 : 9); i++) { int x = wrapx(a + tab[i]); int y = b + tab2[i]; if (!onmap(x, y)) continue; if (9 <= i && isoc(x, y)) continue; t = t > (i < 9 ? 2 : 3) ? t : (i < 9 ? 2 : 3); spot(x, y); } return t;',
 'sp_noi':      'int t = 0; for (int i = 0; i < (b ? 25 : 9); i++) { int x = wrapx(a + tab[i]); int y = b + tab2[i]; if (!onmap(x, y)) continue; if (isoc(x, y)) continue; t++; spot(x, y); } return t;',
 'sp_inner':    'int t = 0; for (int i = 0; i < (b ? 25 : 9); i++) { int x = wrapx(a + tab[i]); int y = b + tab2[i]; if (!onmap(x, y)) continue; for (int k = 0; k < i; k++) t++; spot(x, y); } return t;',
 'sp_addr':     'int t = 0; int i; for (i = 0; i < (b ? 25 : 9); i++) { int x = wrapx(a + tab[i]); int y = b + tab2[i]; if (!onmap(x, y)) continue; if (9 <= i && isoc(x, y)) continue; spot(x, y); } hasx((int)&i, 0); return t;',
 'sp_passi':    'int t = 0; for (int i = 0; i < (b ? 25 : 9); i++) { int x = wrapx(a + tab[i]); int y = b + tab2[i]; if (!onmap(x, y)) continue; if (9 <= i && isoc(x, y)) continue; spot(x, i); } return t;',
 'sp_bound':    'int t = 0; int n = b ? 25 : 9; for (int i = 0; i < n; i++) { int x = wrapx(a + tab[i]); int y = b + tab2[i]; if (!onmap(x, y)) continue; if (9 <= i && isoc(x, y)) continue; spot(x, y); } return t;',

 # ---- loops VC6 may not process: backward goto, irreducible entry, huge body
 'lp_goto':     'int c = 0; int i = 0; top: if (tab[i] == gsel) c++; use(i); if (++i < 64) goto top; return c;',
 'lp_irr':      'int c = 0; int i = 0; if (a) goto mid; for (; i < 64; i++) { if (tab[i] == gsel) c++; mid: use(i); } return c;',
 'lp_irr2':     'int c = 0; int i = b; if (a) goto mid; for (; i < 64; i++) { if (tab[i] == gsel) c++; mid: if (tab2[i]) c++; } return c;',
 'lp_switch':   'int c = 0; for (int i = 0; i < 64; i++) { switch (a) { case 1: c++; break; case 2: c--; break; case 3: c += 3; break; case 4: c ^= 1; break; } if (tab[i] == gsel) c++; use(i); } return c;',
 'lp_try':      'int c = 0; for (int i = 0; i < 64; i++) { if (tab[i] == gsel) c++; use(i); } return c;',

 # ---- a constant-left compare of i turns the two-table IDX back into a byte-offset IV (spot_all 0x005B6A0D)
 'two_k9':      'int c = 0; for (int i = 0; i < 64; i++) { if (tab[i] == gsel) c++; if (tab2[i]) c += 3; if (9 <= i) c++; use(i); } return c;',
 'two_i9':      'int c = 0; for (int i = 0; i < 64; i++) { if (tab[i] == gsel) c++; if (tab2[i]) c += 3; if (i >= 9) c++; use(i); } return c;',
 'sp_k9':       'int t = 0; for (int i = 0; i < (b ? 25 : 9); i++) { int x = wrapx(a + tab[i]); int y = b + tab2[i]; if (!onmap(x, y)) continue; if (9 <= i && isoc(x, y)) continue; spot(x, y); } return t;',

 # ---- does VC6 stop processing loops after many loops / inside a goto-built region
 'many8': 'for (int k0 = 0; k0 < b; k0++) use(tab2[k0] + 0); for (int k1 = 0; k1 < b; k1++) use(tab2[k1] + 1); for (int k2 = 0; k2 < b; k2++) use(tab2[k2] + 2); for (int k3 = 0; k3 < b; k3++) use(tab2[k3] + 3); for (int k4 = 0; k4 < b; k4++) use(tab2[k4] + 4); for (int k5 = 0; k5 < b; k5++) use(tab2[k5] + 5); for (int k6 = 0; k6 < b; k6++) use(tab2[k6] + 6); for (int k7 = 0; k7 < b; k7++) use(tab2[k7] + 7);  int c = 0; for (int i = 0; i < 64; i++) { if (tab[i] == gsel) c++; use(c); } return c;',
 'many16': 'for (int k0 = 0; k0 < b; k0++) use(tab2[k0] + 0); for (int k1 = 0; k1 < b; k1++) use(tab2[k1] + 1); for (int k2 = 0; k2 < b; k2++) use(tab2[k2] + 2); for (int k3 = 0; k3 < b; k3++) use(tab2[k3] + 3); for (int k4 = 0; k4 < b; k4++) use(tab2[k4] + 4); for (int k5 = 0; k5 < b; k5++) use(tab2[k5] + 5); for (int k6 = 0; k6 < b; k6++) use(tab2[k6] + 6); for (int k7 = 0; k7 < b; k7++) use(tab2[k7] + 7); for (int k8 = 0; k8 < b; k8++) use(tab2[k8] + 8); for (int k9 = 0; k9 < b; k9++) use(tab2[k9] + 9); for (int k10 = 0; k10 < b; k10++) use(tab2[k10] + 10); for (int k11 = 0; k11 < b; k11++) use(tab2[k11] + 11); for (int k12 = 0; k12 < b; k12++) use(tab2[k12] + 12); for (int k13 = 0; k13 < b; k13++) use(tab2[k13] + 13); for (int k14 = 0; k14 < b; k14++) use(tab2[k14] + 14); for (int k15 = 0; k15 < b; k15++) use(tab2[k15] + 15);  int c = 0; for (int i = 0; i < 64; i++) { if (tab[i] == gsel) c++; use(c); } return c;',
 'many32': 'for (int k0 = 0; k0 < b; k0++) use(tab2[k0] + 0); for (int k1 = 0; k1 < b; k1++) use(tab2[k1] + 1); for (int k2 = 0; k2 < b; k2++) use(tab2[k2] + 2); for (int k3 = 0; k3 < b; k3++) use(tab2[k3] + 3); for (int k4 = 0; k4 < b; k4++) use(tab2[k4] + 4); for (int k5 = 0; k5 < b; k5++) use(tab2[k5] + 5); for (int k6 = 0; k6 < b; k6++) use(tab2[k6] + 6); for (int k7 = 0; k7 < b; k7++) use(tab2[k7] + 7); for (int k8 = 0; k8 < b; k8++) use(tab2[k8] + 8); for (int k9 = 0; k9 < b; k9++) use(tab2[k9] + 9); for (int k10 = 0; k10 < b; k10++) use(tab2[k10] + 10); for (int k11 = 0; k11 < b; k11++) use(tab2[k11] + 11); for (int k12 = 0; k12 < b; k12++) use(tab2[k12] + 12); for (int k13 = 0; k13 < b; k13++) use(tab2[k13] + 13); for (int k14 = 0; k14 < b; k14++) use(tab2[k14] + 14); for (int k15 = 0; k15 < b; k15++) use(tab2[k15] + 15); for (int k16 = 0; k16 < b; k16++) use(tab2[k16] + 16); for (int k17 = 0; k17 < b; k17++) use(tab2[k17] + 17); for (int k18 = 0; k18 < b; k18++) use(tab2[k18] + 18); for (int k19 = 0; k19 < b; k19++) use(tab2[k19] + 19); for (int k20 = 0; k20 < b; k20++) use(tab2[k20] + 20); for (int k21 = 0; k21 < b; k21++) use(tab2[k21] + 21); for (int k22 = 0; k22 < b; k22++) use(tab2[k22] + 22); for (int k23 = 0; k23 < b; k23++) use(tab2[k23] + 23); for (int k24 = 0; k24 < b; k24++) use(tab2[k24] + 24); for (int k25 = 0; k25 < b; k25++) use(tab2[k25] + 25); for (int k26 = 0; k26 < b; k26++) use(tab2[k26] + 26); for (int k27 = 0; k27 < b; k27++) use(tab2[k27] + 27); for (int k28 = 0; k28 < b; k28++) use(tab2[k28] + 28); for (int k29 = 0; k29 < b; k29++) use(tab2[k29] + 29); for (int k30 = 0; k30 < b; k30++) use(tab2[k30] + 30); for (int k31 = 0; k31 < b; k31++) use(tab2[k31] + 31);  int c = 0; for (int i = 0; i < 64; i++) { if (tab[i] == gsel) c++; use(c); } return c;',
 'many48': 'for (int k0 = 0; k0 < b; k0++) use(tab2[k0] + 0); for (int k1 = 0; k1 < b; k1++) use(tab2[k1] + 1); for (int k2 = 0; k2 < b; k2++) use(tab2[k2] + 2); for (int k3 = 0; k3 < b; k3++) use(tab2[k3] + 3); for (int k4 = 0; k4 < b; k4++) use(tab2[k4] + 4); for (int k5 = 0; k5 < b; k5++) use(tab2[k5] + 5); for (int k6 = 0; k6 < b; k6++) use(tab2[k6] + 6); for (int k7 = 0; k7 < b; k7++) use(tab2[k7] + 7); for (int k8 = 0; k8 < b; k8++) use(tab2[k8] + 8); for (int k9 = 0; k9 < b; k9++) use(tab2[k9] + 9); for (int k10 = 0; k10 < b; k10++) use(tab2[k10] + 10); for (int k11 = 0; k11 < b; k11++) use(tab2[k11] + 11); for (int k12 = 0; k12 < b; k12++) use(tab2[k12] + 12); for (int k13 = 0; k13 < b; k13++) use(tab2[k13] + 13); for (int k14 = 0; k14 < b; k14++) use(tab2[k14] + 14); for (int k15 = 0; k15 < b; k15++) use(tab2[k15] + 15); for (int k16 = 0; k16 < b; k16++) use(tab2[k16] + 16); for (int k17 = 0; k17 < b; k17++) use(tab2[k17] + 17); for (int k18 = 0; k18 < b; k18++) use(tab2[k18] + 18); for (int k19 = 0; k19 < b; k19++) use(tab2[k19] + 19); for (int k20 = 0; k20 < b; k20++) use(tab2[k20] + 20); for (int k21 = 0; k21 < b; k21++) use(tab2[k21] + 21); for (int k22 = 0; k22 < b; k22++) use(tab2[k22] + 22); for (int k23 = 0; k23 < b; k23++) use(tab2[k23] + 23); for (int k24 = 0; k24 < b; k24++) use(tab2[k24] + 24); for (int k25 = 0; k25 < b; k25++) use(tab2[k25] + 25); for (int k26 = 0; k26 < b; k26++) use(tab2[k26] + 26); for (int k27 = 0; k27 < b; k27++) use(tab2[k27] + 27); for (int k28 = 0; k28 < b; k28++) use(tab2[k28] + 28); for (int k29 = 0; k29 < b; k29++) use(tab2[k29] + 29); for (int k30 = 0; k30 < b; k30++) use(tab2[k30] + 30); for (int k31 = 0; k31 < b; k31++) use(tab2[k31] + 31); for (int k32 = 0; k32 < b; k32++) use(tab2[k32] + 32); for (int k33 = 0; k33 < b; k33++) use(tab2[k33] + 33); for (int k34 = 0; k34 < b; k34++) use(tab2[k34] + 34); for (int k35 = 0; k35 < b; k35++) use(tab2[k35] + 35); for (int k36 = 0; k36 < b; k36++) use(tab2[k36] + 36); for (int k37 = 0; k37 < b; k37++) use(tab2[k37] + 37); for (int k38 = 0; k38 < b; k38++) use(tab2[k38] + 38); for (int k39 = 0; k39 < b; k39++) use(tab2[k39] + 39); for (int k40 = 0; k40 < b; k40++) use(tab2[k40] + 40); for (int k41 = 0; k41 < b; k41++) use(tab2[k41] + 41); for (int k42 = 0; k42 < b; k42++) use(tab2[k42] + 42); for (int k43 = 0; k43 < b; k43++) use(tab2[k43] + 43); for (int k44 = 0; k44 < b; k44++) use(tab2[k44] + 44); for (int k45 = 0; k45 < b; k45++) use(tab2[k45] + 45); for (int k46 = 0; k46 < b; k46++) use(tab2[k46] + 46); for (int k47 = 0; k47 < b; k47++) use(tab2[k47] + 47);  int c = 0; for (int i = 0; i < 64; i++) { if (tab[i] == gsel) c++; use(c); } return c;',
 'many64': 'for (int k0 = 0; k0 < b; k0++) use(tab2[k0] + 0); for (int k1 = 0; k1 < b; k1++) use(tab2[k1] + 1); for (int k2 = 0; k2 < b; k2++) use(tab2[k2] + 2); for (int k3 = 0; k3 < b; k3++) use(tab2[k3] + 3); for (int k4 = 0; k4 < b; k4++) use(tab2[k4] + 4); for (int k5 = 0; k5 < b; k5++) use(tab2[k5] + 5); for (int k6 = 0; k6 < b; k6++) use(tab2[k6] + 6); for (int k7 = 0; k7 < b; k7++) use(tab2[k7] + 7); for (int k8 = 0; k8 < b; k8++) use(tab2[k8] + 8); for (int k9 = 0; k9 < b; k9++) use(tab2[k9] + 9); for (int k10 = 0; k10 < b; k10++) use(tab2[k10] + 10); for (int k11 = 0; k11 < b; k11++) use(tab2[k11] + 11); for (int k12 = 0; k12 < b; k12++) use(tab2[k12] + 12); for (int k13 = 0; k13 < b; k13++) use(tab2[k13] + 13); for (int k14 = 0; k14 < b; k14++) use(tab2[k14] + 14); for (int k15 = 0; k15 < b; k15++) use(tab2[k15] + 15); for (int k16 = 0; k16 < b; k16++) use(tab2[k16] + 16); for (int k17 = 0; k17 < b; k17++) use(tab2[k17] + 17); for (int k18 = 0; k18 < b; k18++) use(tab2[k18] + 18); for (int k19 = 0; k19 < b; k19++) use(tab2[k19] + 19); for (int k20 = 0; k20 < b; k20++) use(tab2[k20] + 20); for (int k21 = 0; k21 < b; k21++) use(tab2[k21] + 21); for (int k22 = 0; k22 < b; k22++) use(tab2[k22] + 22); for (int k23 = 0; k23 < b; k23++) use(tab2[k23] + 23); for (int k24 = 0; k24 < b; k24++) use(tab2[k24] + 24); for (int k25 = 0; k25 < b; k25++) use(tab2[k25] + 25); for (int k26 = 0; k26 < b; k26++) use(tab2[k26] + 26); for (int k27 = 0; k27 < b; k27++) use(tab2[k27] + 27); for (int k28 = 0; k28 < b; k28++) use(tab2[k28] + 28); for (int k29 = 0; k29 < b; k29++) use(tab2[k29] + 29); for (int k30 = 0; k30 < b; k30++) use(tab2[k30] + 30); for (int k31 = 0; k31 < b; k31++) use(tab2[k31] + 31); for (int k32 = 0; k32 < b; k32++) use(tab2[k32] + 32); for (int k33 = 0; k33 < b; k33++) use(tab2[k33] + 33); for (int k34 = 0; k34 < b; k34++) use(tab2[k34] + 34); for (int k35 = 0; k35 < b; k35++) use(tab2[k35] + 35); for (int k36 = 0; k36 < b; k36++) use(tab2[k36] + 36); for (int k37 = 0; k37 < b; k37++) use(tab2[k37] + 37); for (int k38 = 0; k38 < b; k38++) use(tab2[k38] + 38); for (int k39 = 0; k39 < b; k39++) use(tab2[k39] + 39); for (int k40 = 0; k40 < b; k40++) use(tab2[k40] + 40); for (int k41 = 0; k41 < b; k41++) use(tab2[k41] + 41); for (int k42 = 0; k42 < b; k42++) use(tab2[k42] + 42); for (int k43 = 0; k43 < b; k43++) use(tab2[k43] + 43); for (int k44 = 0; k44 < b; k44++) use(tab2[k44] + 44); for (int k45 = 0; k45 < b; k45++) use(tab2[k45] + 45); for (int k46 = 0; k46 < b; k46++) use(tab2[k46] + 46); for (int k47 = 0; k47 < b; k47++) use(tab2[k47] + 47); for (int k48 = 0; k48 < b; k48++) use(tab2[k48] + 48); for (int k49 = 0; k49 < b; k49++) use(tab2[k49] + 49); for (int k50 = 0; k50 < b; k50++) use(tab2[k50] + 50); for (int k51 = 0; k51 < b; k51++) use(tab2[k51] + 51); for (int k52 = 0; k52 < b; k52++) use(tab2[k52] + 52); for (int k53 = 0; k53 < b; k53++) use(tab2[k53] + 53); for (int k54 = 0; k54 < b; k54++) use(tab2[k54] + 54); for (int k55 = 0; k55 < b; k55++) use(tab2[k55] + 55); for (int k56 = 0; k56 < b; k56++) use(tab2[k56] + 56); for (int k57 = 0; k57 < b; k57++) use(tab2[k57] + 57); for (int k58 = 0; k58 < b; k58++) use(tab2[k58] + 58); for (int k59 = 0; k59 < b; k59++) use(tab2[k59] + 59); for (int k60 = 0; k60 < b; k60++) use(tab2[k60] + 60); for (int k61 = 0; k61 < b; k61++) use(tab2[k61] + 61); for (int k62 = 0; k62 < b; k62++) use(tab2[k62] + 62); for (int k63 = 0; k63 < b; k63++) use(tab2[k63] + 63);  int c = 0; for (int i = 0; i < 64; i++) { if (tab[i] == gsel) c++; use(c); } return c;',
 'lbl_in':      'int c = 0; for (int i = 0; i < 64; i++) { again: if (tab[i] == gsel) c++; use(c); if (c == 7) { c++; goto again; } } return c;',
 'goto_out':    'int c = 0; for (int i = 0; i < 64; i++) { if (tab[i] == gsel) c++; use(c); if (c == 7) goto out; } out: return c;',
 'ret_in':      'int c = 0; for (int i = 0; i < 64; i++) { if (tab[i] == gsel) c++; use(c); if (c == 7) return 3; } return c;',
}


def cls(lines):
    """PTR: a pointer to the table is formed (`OFFSET FLAT:` table address loaded); OFF: an unscaled byte offset
    `tab[reg]` stepped by the stride; IDX: the counter itself scaled, `tab[reg*4]`."""
    t = ' '.join(lines)
    out = []
    for m in re.finditer(r'(?:mov|lea)\s+(e\w\w), (?:DWORD PTR )?(?:OFFSET FLAT:)?\?(s?tab2?|gd|bases)@@', t):
        if re.search(r'add\s+%s, \d+' % m.group(1), t[m.end():]):
            out.append('PTR')
            break
    if re.search(r'\?(s?tab2?|gd)@@[\w@]+(\+\d+)?\[\w+(\+\d+)?\]', t):
        out.append('OFF')
    if re.search(r'\?(s?tab2?|gd)@@[\w@]+(\+\d+)?\[\w+\*[248](\+\d+)?\]', t):
        out.append('IDX')
    return '+'.join(out) or '?'


def main(names):
    src = HDR + ''.join('int v_%s(int a, int b) {\n%s\n}\n' % (k, v) for k, v in V.items() if not names or k in names)
    asm, _ = ib.compile_asm(None, PFX, text=src)
    show = os.environ.get('SHOW')
    for title, sym, lines in ib.functions(asm):
        nm = re.sub(r'^\?|@.*$', '', sym).lstrip('_')[2:]
        regs = sorted(set(re.findall(r'\b(ebx|esi|edi)\b', ' '.join(lines))))
        print('%-12s %-8s %s' % (nm, cls(lines), ','.join(regs)))
        if show == nm or show == 'all':
            print('\n'.join('   ' + l for l in lines))


if __name__ == '__main__':
    main(sys.argv[1:])
