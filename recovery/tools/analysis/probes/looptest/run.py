#!/usr/bin/env python3
"""run.py [name ...]: when VC6 duplicates a loop's test (guard before the loop + test at the bottom) and when it
enters the loop by a jump to the one test at the bottom (MATCHING_GUIDE 3.7, "Loop tests").  Each variant is one
cdecl function; prints its block skeleton (branches, `ret`, `use(K)` as uK, other calls) and the entry class:
DUP = a conditional guard before the loop, JMP = `jmp` to the test, NONE = the body is entered unconditionally
(do-while, or a first test the compiler proved), TOP = test at the top with a `jmp` back.
SHOW=1 prints the full listing.  FE_PFX_BASE selects the compiler prefix (default 21)."""
import sys, os, re
H = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(os.path.dirname(H)) + '/inline')
sys.path.insert(0, os.path.dirname(H) + '/blockorder')
import ib
import run as bo
PFX = os.environ.get('FE_PFX_BASE', '21')
HDR = '''#include "probe.h"
int vis2(int);
struct S { int a, b, c; S *next; };
extern S *gs; extern S gt[];
inline int get_n() { return g3; }
inline int in_two(int i) { return i < g && ga[i] != 0; }
'''
B = 'use(i);'
V = {
 # --- for loops, test of growing size
 'for_k':       'int i; for (i = 0; i < 8; i++) { use(i); } return 1;',
 'for_par':     'int i; for (i = 0; i < a; i++) { use(i); } return 1;',
 'for_glob':    'int i; for (i = 0; i < g; i++) { use(i); } return 1;',
 'for_start':   'int i; for (i = b; i < a; i++) { use(i); } return 1;',
 'for_mem':     'int i; for (i = 0; i < gs->a; i++) { use(i); } return 1;',
 'for_arr':     'int i; for (i = 0; i < ga[a]; i++) { use(i); } return 1;',
 'for_expr':    'int i; for (i = 0; i < ga[a] * 3 + g; i++) { use(i); } return 1;',
 'for_expr2':   'int i; for (i = 0; i < (ga[a] * 3 + g) / (b + 1); i++) { use(i); } return 1;',
 'for_call':    'int i; for (i = 0; i < cond(); i++) { use(i); } return 1;',
 'for_inl':     'int i; for (i = 0; i < get_n(); i++) { use(i); } return 1;',
 'for_and':     'int i; for (i = 0; i < a && ga[i]; i++) { use(i); } return 1;',
 'for_and_g':   'int i; for (i = 0; i < g && ga[i]; i++) { use(i); } return 1;',
 'for_and3':    'int i; for (i = 0; i < g && ga[i] && g2 != i; i++) { use(i); } return 1;',
 'for_or':      'int i; for (i = 0; i < a || ga[i]; i++) { use(i); } return 1;',
 'for_inl2':    'int i; for (i = 0; in_two(i); i++) { use(i); } return 1;',
 'for_andcall': 'int i; for (i = 0; i < a && cond(); i++) { use(i); } return 1;',
 'for_callfirst': 'int i; for (i = 0; cond() && i < a; i++) { use(i); } return 1;',
 'for_ptr':     'S *p; for (p = gs; p; p = p->next) { use(p->a); } return 1;',
 'for_ptr2':    'S *p; for (p = gs; p && p->a; p = p->next) { use(p->a); } return 1;',
 'for_ptrk':    'S *p; for (p = gt; p < &gt[8]; p++) { use(p->a); } return 1;',
 'for_ptrg':    'S *p; for (p = gt; p < &gt[g]; p++) { use(p->a); } return 1;',
 'for_big':     'int i; for (i = 0; i < a; i++) { use(i); use(i + b); if (ga[i]) { use(3); } use(c); } return 1;',
 'for_nostep':  'int i; for (i = 0; i < a;) { use(i); i += ga[i]; } return 1;',
 'for_noinit':  'int i = b; use(i); for (; i < a; i++) { use(i); } return 1;',
 'for_after':   'int i; use(a); for (i = 0; i < a; i++) { use(i); } use(b); return 1;',
 'for_inif':    'int i; if (b) { for (i = 0; i < a; i++) { use(i); } } use(b); return 1;',
 'for_inelse':  'int i; if (b) { use(1); } else { for (i = 0; i < a; i++) { use(i); } } use(b); return 1;',
 'for_nest':    'int i, j; for (i = 0; i < a; i++) { for (j = 0; j < b; j++) { use(i + j); } } return 1;',
 'for_nest_g':  'int i, j; for (i = 0; i < g; i++) { for (j = 0; j < g2; j++) { use(i + j); } } return 1;',
 'for_nest_dep': 'int i, j; for (i = 0; i < a; i++) { for (j = i; j < b; j++) { use(i + j); } } return 1;',
 'for_nest_call': 'int i, j; for (i = 0; i < a; i++) { for (j = 0; j < cond(); j++) { use(i + j); } } return 1;',
 # --- while loops
 'wh_par':      'int i = 0; while (i < a) { use(i); i++; } return 1;',
 'wh_glob':     'while (g) { use(1); } return 1;',
 'wh_call':     'while (cond()) { use(1); } return 1;',
 'wh_and':      'int i = 0; while (i < a && ga[i]) { use(i); i++; } return 1;',
 'wh_andcall':  'int i = 0; while (i < a && cond()) { use(i); i++; } return 1;',
 'wh_dec':      'while (a--) { use(a); } return 1;',
 'wh_predec':   'while (--a >= 0) { use(a); } return 1;',
 'wh_ptr':      'S *p = gs; while (p) { use(p->a); p = p->next; } return 1;',
 'wh_expr2':    'int i = 0; while (i < (ga[a] * 3 + g) / (b + 1)) { use(i); i++; } return 1;',
 'wh_1':        'int i = 0; while (1) { if (i >= a) break; use(i); i++; } return 1;',
 'wh_1b':       'int i = 0; while (1) { use(i); i++; if (i >= a) break; } return 1;',
 'for_ever':    'int i; for (i = 0; ; i++) { if (ga[i] == a) break; use(i); } return 1;',
 'for_ever_ret': 'int i; for (i = 0; ; i++) { if (ga[i] == a) return i; use(i); }',
 # --- do-while and guarded do-while
 'do_par':      'int i = 0; do { use(i); i++; } while (i < a); return 1;',
 'do_guard':    'int i = 0; if (i < a) { do { use(i); i++; } while (i < a); } return 1;',
 'do_guard_s':  'int i = b; if (i < a) { do { use(i); i++; } while (i < a); } return 1;',
 'goto_test':   'int i = b; goto test; body: use(i); i++; test: if (i < a) goto body; return 1;',
 # --- continue / break inside
 'for_cont':    'int i; for (i = 0; i < a; i++) { if (ga[i]) continue; use(i); } return 1;',
 'for_brk':     'int i; for (i = 0; i < a; i++) { if (ga[i]) break; use(i); } return 1;',
 'for_g_cont':  'int i; for (i = 0; i < g; i++) { if (ga[i]) continue; use(i); } return 1;',
 'for_gwrite':  'int i; for (i = 0; i < g; i++) { g2 = i; use(i); } return 1;',
 'for_first':   'int i; for (i = 0; i < g; i++) { use(i); } for (i = 0; i < g; i++) { use(i + 1); } return 1;',
 # a re-test after a do-while with break (drone_riot 0x004F4FB0): which block follows the latch
 'rt_do':      'int i = 0; do { use(1); if (ga[i]) break; } while (++i <= a); if (i <= a) { use(2); return 0; } use(3); return 1;',
 'rt_do_v':    'int i = 0; do { use(1); if (ga[i]) break; } while (++i <= a); if (i <= a) { use(2); return; } if (b) return; use(3); use(4);',
 'rt_do_rv':   'int i = 0; do { use(1); if (ga[i]) break; } while (++i <= a); if (i > a) { if (b) return; use(3); use(4); return; } use(2);',
 'rt_for':     'int i; for (i = 0; i <= a; i++) { use(1); if (ga[i]) break; } if (i <= a) { use(2); return; } if (b) return; use(3); use(4);',
 'rt_do_mid':  'int i = 0; do { use(1); if (ga[i] && ga[i+1]) break; } while (++i <= a); if (i <= a) { use(2); return; } if (b) return; use(3); use(4);',
 'rt_do_f':    'int i = 0; int f; do { f = cond(); int ok = 1; for (int k = 0; k < b; k++) { if (ga[k] == f) { ok = 0; break; } } if (f > 1 && cond() && ok) break; } while (++i <= a); if (i <= a) { use(f); return; } if (c) return; use(3); use(4);',
  'rd_and': 'int i = 0; int f; do { f = cond(); int ok = 1; for (int k = 0; k < b; k++) { if (ga[k] == f) { ok = 0; break; } } if (f > 1 && cond() && ok) break; } while (++i <= a); if (i <= a) { use(f); return; } if (c) return; use(3); use(4);',
  'rd_nest': 'int i = 0; int f; do { f = cond(); int ok = 1; for (int k = 0; k < b; k++) { if (ga[k] == f) { ok = 0; break; } } if (f > 1) { if (cond()) { if (ok) break; } } } while (++i <= a); if (i <= a) { use(f); return; } if (c) return; use(3); use(4);',
  'rd_cont': 'int i = 0; int f; do { f = cond(); int ok = 1; for (int k = 0; k < b; k++) { if (ga[k] == f) { ok = 0; break; } } if (f <= 1) continue; if (!cond()) continue; if (ok) break; } while (++i <= a); if (i <= a) { use(f); return; } if (c) return; use(3); use(4);',
  'rd_one': 'int i = 0; int f; do { f = cond(); int ok = 1; for (int k = 0; k < b; k++) { if (ga[k] == f) { ok = 0; break; } } if (ok) break; } while (++i <= a); if (i <= a) { use(f); return; } if (c) return; use(3); use(4);',
  'rd_two': 'int i = 0; int f; do { f = cond(); int ok = 1; for (int k = 0; k < b; k++) { if (ga[k] == f) { ok = 0; break; } } if (f > 1 && ok) break; } while (++i <= a); if (i <= a) { use(f); return; } if (c) return; use(3); use(4);',
  'rd_callfirst': 'int i = 0; int f; do { f = cond(); int ok = 1; for (int k = 0; k < b; k++) { if (ga[k] == f) { ok = 0; break; } } if (cond() && ok) break; } while (++i <= a); if (i <= a) { use(f); return; } if (c) return; use(3); use(4);',
  'rd_noinner': 'int i = 0; int f; do { f = cond(); if (f > 1 && cond() && ga[f]) break; } while (++i <= a); if (i <= a) { use(f); return; } if (c) return; use(3); use(4);',
  'rd_for': 'int i; int f; for (i = 0; i <= a; i++) { f = cond(); int ok = 1; for (int k = 0; k < b; k++) { if (ga[k] == f) { ok = 0; break; } } if (f > 1 && cond() && ok) break; } if (i <= a) { use(f); return; } if (c) return; use(3); use(4);',
  'rd_forever': 'int i = 0; int f; for (;;) { f = cond(); int ok = 1; for (int k = 0; k < b; k++) { if (ga[k] == f) { ok = 0; break; } } if (f > 1 && cond() && ok) break; if (++i > a) break; } if (i <= a) { use(f); return; } if (c) return; use(3); use(4);',
  'rd_goto': 'int i = 0; int f; top: { f = cond(); int ok = 1; for (int k = 0; k < b; k++) { if (ga[k] == f) { ok = 0; break; } } if (f > 1 && cond() && ok) goto out; } if (++i <= a) goto top; out: if (i <= a) { use(f); return; } if (c) return; use(3); use(4);',
  'rd_inv': 'int i = 0; int f; do { f = cond(); int ok = 1; for (int k = 0; k < b; k++) { if (ga[k] == f) { ok = 0; break; } } if (f > 1 && cond() && ok) break; } while (++i <= a); if (i > a) { if (c) return; use(3); use(4); return; } use(f);',
  'rd_for1': 'int i; int f; for (i = 0; i <= a || i == 0; i++) { f = cond(); int ok = 1; for (int k = 0; k < b; k++) { if (ga[k] == f) { ok = 0; break; } } if (f > 1 && cond() && ok) break; } if (i <= a) { use(f); return; } if (c) return; use(3); use(4);',
  'rd_whi': 'int i = 0; int f; while (i <= a) { f = cond(); int ok = 1; for (int k = 0; k < b; k++) { if (ga[k] == f) { ok = 0; break; } } if (f > 1 && cond() && ok) break; i++; } if (i <= a) { use(f); return; } if (c) return; use(3); use(4);',
  'rd_dobrk': 'int i = 0; int f; do { f = cond(); int ok = 1; for (int k = 0; k < b; k++) { if (ga[k] == f) { ok = 0; break; } } if (f > 1 && cond() && ok) break; i++; } while (i <= a); if (i <= a) { use(f); return; } if (c) return; use(3); use(4);',
}
def klass(sk):
    """skeleton -> entry class of the first loop"""
    lab = {}
    for k, s in enumerate(sk):
        if s.endswith(':'):
            lab[s[:-1]] = k
    for k, s in enumerate(sk):
        m = re.match(r'(j\w+) (\S+)', s)
        if m and m.group(2) in lab and lab[m.group(2)] < k:      # first back edge
            h = lab[m.group(2)]
            if m.group(1) == 'jmp':
                return 'TOP'
            pre = [x for x in sk[:h] if re.match(r'j\w+ ', x)]
            if any(re.match(r'jmp (\S+)', x) and h < lab.get(x.split()[1], -1) <= k for x in pre):
                return 'JMP'
            if any(not x.startswith('jmp') and lab.get(x.split()[1], -1) > k for x in pre):
                return 'DUP'
            return 'NONE'
    return 'NOLOOP'
if __name__ == '__main__':
    sel = sys.argv[1:] or list(V)
    src = HDR + ''.join(('void' if 'return;' in V[n] or V[n].rstrip().endswith(');') else 'int') + ' %s(int a, int b, int c) {\n%s\n}\n' % (n, V[n]) for n in sel)
    asm, out = ib.compile_asm(None, PFX, text=src)
    F = {f[1]: f[2] for f in ib.functions(asm)}
    for n in sel:
        L = next((l for k, l in F.items() if ('?%s@@' % n) in k), None)
        if L is None:
            print(n, 'MISSING', out[-300:] if out else ''); continue
        L = [l for l in L if not l.startswith(';')]
        sk = bo.skeleton(L)
        print('%-14s %-5s %s' % (n, klass(sk), ' '.join(sk)))
        if os.environ.get('SHOW'):
            print('   ' + ' ; '.join(L))
