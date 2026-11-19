#!/usr/bin/env python3
"""run.py [name ...]: when does VC6 defer a callee-saved push past an early-out? (MATCHING_GUIDE 3.7,
"Prologue saves and return tails").  Each variant is one member function with an early guard and a body that
needs one or two more registers; prints the pushes before the first branch, the later saves and the number
of `ret`.  FE_PFX_BASE selects the compiler prefix (default 21)."""
import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))) + '/inline')
import ib
PFX = os.environ.get('FE_PFX_BASE', '21')
HDR = '''#include "probe.h"
struct V { short x, y, home; char pad[46]; };
struct B { short x, y; char pad[304]; };
extern V Vs[]; extern B Bs[];
int vis2(int);
struct W { int a; int b; int c; int vis(); void redraw(); %s };
'''
BODY2 = 'int k = ga[i]; use(k); use(k + a); use(k);'
V = {
 # guard spelling, single exit
 'er_var':    ('void', 'if (!vis()) return; ' + BODY2),
 'wrap_var':  ('void', 'if (vis()) { ' + BODY2 + ' }'),
 'er_trail':  ('void', 'if (!vis()) return; ' + BODY2 + ' return;'),
 'goto_var':  ('void', 'int k; if (!vis()) goto end; k = ga[i]; use(k); use(k + a); use(k); end:;'),
 'er_val':    ('int', 'if (!vis()) return 0; ' + BODY2 + ' return k;'),
 'er_val1':   ('int', 'if (!vis()) return 0; ' + BODY2 + ' return 1;'),
 'er_val0':   ('int', 'if (!vis()) return 0; ' + BODY2 + ' return 0;'),
 'res_val':   ('int', 'int r = 0; if (vis()) { ' + BODY2 + ' r = k; } return r;'),
 # a second exit inside the body
 'er_inret':  ('void', 'if (!vis()) return; int k = ga[i]; use(k); if (k == 3) { use(k + a); return; } use(k); use(k + 1);'),
 'wrap_inret': ('void', 'if (vis()) { int k = ga[i]; use(k); if (k == 3) { use(k + a); return; } use(k); use(k + 1); }'),
 'er_inret_last': ('void', 'if (!vis()) return; int k = ga[i]; use(k); if (k == 3) { use(k + a); return; } use(k);'),
 'er_er':     ('void', 'if (!vis()) return; if (!b) return; ' + BODY2),
 'er_er_reg': ('void', 'if (!vis()) return; int k = ga[i]; if (!k) return; use(k); use(k + a); use(k);'),
 'er_and':    ('void', 'if (!vis() || !b) return; ' + BODY2),
 'er_val_in': ('int', 'if (!vis()) return 0; int k = ga[i]; use(k); if (k == 3) { use(k + a); return 2; } use(k); return k;'),
 'er_val_in0': ('int', 'if (!vis()) return 0; int k = ga[i]; use(k); if (k == 3) { use(k + a); return 0; } use(k); return k;'),
 # loops / do-while with a return inside
 'er_loop':   ('void', 'if (!vis()) return; for (int k = 0; k < i; k++) { use(k + a); }'),
 'er_do_ret': ('void', 'if (!vis()) return; int k = 0; do { use(k + a); k++; } while (k < i); use(1); return;'),
 'er_for_ret': ('void', 'if (!vis()) return; for (int k = 0;; k++) { use(k + a); if (ga[k] == 0) { use(1); return; } }'),
 'er_for_brk': ('void', 'if (!vis()) return; for (int k = 0;; k++) { use(k + a); if (ga[k] == 0) break; } use(1);'),
 # a compiler temporary (16-bit compare through a register), no named value: check_veh 0x0041A8B0
 'er_tmp':    ('void', 'if (!vis()) return; if ((Vs[i].x == Bs[b].x && Vs[i].y == Bs[b].y) || Vs[i].home == b) { use(1); redraw(); }'),
 'wrap_tmp':  ('void', 'if (vis()) { if ((Vs[i].x == Bs[b].x && Vs[i].y == Bs[b].y) || Vs[i].home == b) { use(1); redraw(); } }'),
 'er_tmp1':   ('void', 'if (!vis()) return; if (Vs[i].x == Bs[b].x) { use(1); redraw(); }'),
 'er_tmp_and': ('void', 'if (!vis()) return; if (Vs[i].x == Bs[b].x && Vs[i].y == Bs[b].y) { use(1); redraw(); }'),
 'er_var_or': ('void', 'if (!vis()) return; int k = ga[i]; if ((k == g && k == g2) || k == g3) { use(1); redraw(); }'),
 'er_var_if': ('void', 'if (!vis()) return; int k = ga[i]; if (k == g) { use(k); redraw(); }'),
 'er_var_ifelse': ('void', 'if (!vis()) return; int k = ga[i]; if (k == g) { use(k); redraw(); } else { use(k + 1); }'),
 # the edges into the epilogue: one (fallthrough) or more
 'er_if_then':  ('void', 'if (!vis()) return; int k = ga[i]; if (k == g) { use(k); redraw(); } use(k + 1);'),
 'er_loop_then': ('void', 'if (!vis()) return; for (int k = 0; k < i; k++) { use(k + a); } use(1);'),
 'er_loop_do':  ('void', 'if (!vis()) return; int k = 0; do { use(k + a); k++; } while (k < i);'),
 'er_ifelse_then': ('void', 'if (!vis()) return; int k = ga[i]; if (k == g) { use(k); redraw(); } else { use(k + 1); } use(2);'),
 'er_sw':     ('void', 'if (!vis()) return; int k = ga[i]; switch (k) { case 1: use(k); break; case 2: use(k + a); break; case 5: redraw(); break; }'),
 'er_sw_then': ('void', 'if (!vis()) return; int k = ga[i]; switch (k) { case 1: use(k); break; case 2: use(k + a); break; case 5: redraw(); break; } use(k);'),
 'er_call_er': ('void', 'if (!vis()) return; use(1); if (!b) return; ' + BODY2),
 'er_call_er_this': ('void', 'if (!cond()) return; use(1); if (!g) return; int k = ga[i]; use(k); use(k + 1); use(k);'),
 'er_two':    ('void', 'if (!vis()) return; int k = ga[i]; use(k); int j = ga[k]; use(j + k); use(j); if (j == 3) { use(k); } use(k + a);'),
 'er_two_if': ('void', 'if (!vis()) return; int k = ga[i]; use(k); if (k == 3) { int j = ga[k]; use(j + k); use(j); } use(k + a);'),
 'er_inret_goto': ('void', 'int k; if (!vis()) return; k = ga[i]; use(k); if (k == 3) { use(k + a); goto end; } use(k); use(k + 1); end:;'),
 'er_inret_else': ('void', 'if (!vis()) return; int k = ga[i]; use(k); if (k == 3) { use(k + a); } else { use(k); use(k + 1); }'),
 'er_inret_same': ('void', 'if (!vis()) return; int k = ga[i]; use(k); if (k == 3) { use(k + a); use(2); return; } use(k); use(2);'),
 'er_val_res': ('int', 'if (!vis()) return 0; int k = ga[i]; int r = 1; use(k); if (k == 3) { use(k + a); r = 2; } use(k); return r;'),
 'noguard_if': ('void', 'if (b) { int k = ga[i]; use(k); use(k + a); use(k); }'),
 'noguard_if_g': ('void', 'if (g) { int k = ga[i]; use(k); use(k + 1); use(k); }'),
 'noguard_if_g_else': ('void', 'if (g) { int k = ga[i]; use(k); use(k + 1); use(k); } else { use(3); }'),
 'noguard_if_g_then': ('void', 'if (g) { int k = ga[i]; use(k); use(k + 1); use(k); } use(3);'),
 'pre_then_er': ('void', 'use(3); if (!g) return; int k = ga[i]; use(k); use(k + 1); use(k);'),
 # this not needed after the guard
 'er_nothis': ('void', 'if (!cond()) return; int k = ga[i]; use(k); use(k + 1); use(k);'),
}
if __name__ == '__main__':
    sel = sys.argv[1:] or list(V)
    decl = ' '.join('%s %s(int i);' % (V[n][0], n) for n in V)
    src = HDR % decl + ''.join('%s W::%s(int i) {\n%s\n}\n' % (V[n][0], n, V[n][1]) for n in sel)
    asm, out = ib.compile_asm(None, PFX, text=src)
    F = {f[1]: f[2] for f in ib.functions(asm)}
    for n in sel:
        L = next((l for k, l in F.items() if ('?%s@W@@' % n) in k), None)
        if L is None:
            print(n, 'MISSING', out[-300:] if out else ''); continue
        L = [l for l in L if not l.endswith(':') and not l.startswith(';')]
        first = next((i for i, l in enumerate(L) if re.match(r'j\w+\s|ret', l)), len(L))
        cs = r'push\s+(ebx|esi|edi|ebp)$'
        up = [re.match(cs, l).group(1) for l in L[:first] if re.match(cs, l)]
        seen = set(up); de = []
        for l in L[first:]:
            m = re.match(cs, l)
            if m and m.group(1) not in seen:
                # a save, not an argument: the register has no earlier write
                de.append(m.group(1)); seen.add(m.group(1))
        pops = [re.match(r'pop\s+(\w+)', l).group(1) for l in L if re.match(r'pop\s+(ebx|esi|edi|ebp)$', l)]
        print('%-14s up=%-12s def=%-10s ret=%d pops=%s' % (n, ','.join(up) or '-', ','.join(de) or '-', sum(1 for l in L if l.startswith('ret')), ','.join(pops)))
        if os.environ.get('SHOW'):
            print('   ' + ' ; '.join(L))
