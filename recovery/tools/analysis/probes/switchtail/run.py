#!/usr/bin/env python3
"""run.py: switch tail-merge direction (MATCHING_GUIDE 3.7, vehdraw_scoot_start 0x004BFE00).
A switch on 0..7 whose odd cases 1,3,5,7 end with a long `vector_add`-style chain and whose even cases are
short.  Two cases (A, B) share an identical long tail; the others' tails are made distinct.  For each
spelling we report the layout order of the case bodies (by their marker calls) and which case keeps
the shared tail (the other one ends with a jmp into it).  Knobs: textual order of the cases, the
number of distinct statements before the shared tail in each case, identical short even bodies,
a default, jump table vs compare chain (few cases), break vs return.
Usage: uv run python run.py [--show KEY]   (FE_PFX_BASE, default 21)"""
import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))) + '/inline')
import ib
PFX = os.environ.get('FE_PFX_BASE', '21')
HDR = '''#include "probe.h"
struct V { float v[3]; };
extern V N[4][64];
void vadd(const float *, const float *, float *);
int fi(int a, int b);
void norm(float *);
'''

def tail(n=7):
    s = ''
    for i in range(n):
        s += '    vadd(m.v, t%d.v, m.v);\n' % i
    return s

def case(k, shared, pre=1, uniq=None):
    """case k: `pre` distinct loads, then the shared 7-add tail (or a unique tail)."""
    s = '  case %d:\n    mk(%d);\n' % (k, k)
    for i in range(pre):
        s += '    t%d = N[%d][fi(%d, %d)];\n' % (i % 7, (k + i) % 4, k, i)
    s += '    t6 = N[%d][fi(8, %d)];\n' % (k % 4, k)  # last struct copy before the tail is case-specific
    if shared:
        s += tail()
    else:
        s += '    vadd(m.v, t0.v, m.v);\n    mk2(%d);\n' % k
    return s + '    break;\n'

def even(k, same=True):
    if same:
        return '  case %d:\n    t0 = N[3][fi(c, 0)];\n    t1 = N[1][fi(8, 0)];\n    vadd(t0.v, t1.v, m.v);\n    break;\n' % k
    return '  case %d:\n    mk(%d);\n    t0 = N[3][fi(c, 0)];\n    t1 = N[1][fi(8, 0)];\n    vadd(t0.v, t1.v, m.v);\n    break;\n' % (k, k)

def fn(name, order, shared=(3, 7), evens_same=True, default=False, pre=None):
    pre = pre or {}
    s = 'void %s(int c) {\n  V m, t0, t1, t2, t3, t4, t5, t6;\n  if (cond()) {\n  switch (c) {\n' % name
    for k in order:
        if k % 2 == 0:
            s += even(k, evens_same)
        else:
            s += case(k, k in shared, pre.get(k, 1))
    if default:
        s += '  default:\n    mk(99);\n    break;\n'
    s += '  }\n  } else { m.v[0] = 0; m.v[1] = 0; m.v[2] = 1.0f; }\n  norm(m.v);\n' + ''.join('  if (g == %d) use(%d);\n' % (i, i) for i in range(12)) + '}\n'
    return s

def analyse(lines):
    """-> (layout order of mk/mk2 markers, which shared case keeps the vadd chain)"""
    labels = {l[:-1]: i for i, l in enumerate(lines) if l.endswith(':')}
    order = []
    for i, l in enumerate(lines):
        if l.startswith('call') and ('?mk@@' in l or '?mk2@@' in l):
            m = re.match(r'push\s+(\d+)', lines[i - 1])
            if m: order.append(int(m.group(1)))
    nadd = sum(1 for l in lines if l.startswith('call') and '?vadd@@' in l)
    return order, nadd

def run(V, show=None):
    src = HDR + ''.join(t for k, t in V)
    asm, _ = ib.compile_asm(None, PFX, text=src)
    F = {f[0].split('(')[0].strip(): f[2] for f in ib.functions(asm)}
    for k, t in V:
        name = re.search(r'void (t_\w+)\(', t).group(1)
        lines = F[name]
        order, nadd = analyse(lines)
        # who keeps the shared tail: the marker immediately preceding the long run of vadd calls
        keep = []
        last_mk = None
        run_ = 0
        for i, l in enumerate(lines):
            if l.startswith('call') and '?mk@@' in l:
                m = re.match(r'push\s+(\d+)', lines[i - 1]); last_mk = int(m.group(1)) if m else None; run_ = 0
            if l.startswith('call') and '?vadd@@' in l:
                run_ += 1
                if run_ == 7: keep.append(last_mk)
        tbl = any('DD' in l or 'jmp\tDWORD PTR' in l for l in lines)
        print('%-28s layout=%s vadd=%d keeps7=%s jt=%s' % (k, order, nadd, keep, tbl))
        if show and show == k:
            print('\n'.join('    ' + l for l in lines))

if __name__ == '__main__':
    show = sys.argv[sys.argv.index('--show') + 1] if '--show' in sys.argv else None
    V = []
    V.append(('text0..7', fn('t_a', range(8))))
    V.append(('text0..7_evensdiff', fn('t_b', range(8), evens_same=False)))
    V.append(('text_7_before_3', fn('t_c', [0, 1, 2, 7, 4, 5, 6, 3])))
    V.append(('text_reversed', fn('t_d', list(range(7, -1, -1)))))
    V.append(('pre3_3', fn('t_e', range(8), pre={3: 3})))
    V.append(('pre3_7', fn('t_f', range(8), pre={7: 3})))
    V.append(('default', fn('t_g', range(8), default=True)))
    V.append(('shared1_5', fn('t_h', range(8), shared=(1, 5))))
    V.append(('shared_all_odd', fn('t_i', range(8), shared=(1, 3, 5, 7))))
    V.append(('shared_all_odd_evdiff', fn('t_j', range(8), shared=(1, 3, 5, 7), evens_same=False)))
    run(V, show)
