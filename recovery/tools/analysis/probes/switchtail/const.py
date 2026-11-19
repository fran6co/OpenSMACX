#!/usr/bin/env python3
"""const.py: switch tail-merge direction with register-free tails.  Each case is `mk(k);` followed
by a tail; shared tails are `use(1); use(2); ... use(L);` (identical code), unique tails
`use(100+k)`.  After the switch a long join (12 ifs) prevents tail duplication.  Prints the layout
order of the cases and, for each shared case, whether it KEEPs the tail or JUMPs into another case.
Usage: uv run python const.py [--show KEY]   (FE_PFX_BASE, default 21)"""
import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))) + '/inline')
import ib
PFX = os.environ.get('FE_PFX_BASE', '21')
HDR = '#include "probe.h"\n'


def body(k, kind, L):
    s = '  case %d:\n    mk(%d);\n' % (k, k)
    if kind == 'S':
        s += ''.join('    use(%d);\n' % i for i in range(1, L + 1))
    elif kind == 'E':           # short identical body (vehdraw's even cases) without a marker
        s = '  case %d:\n    use(500); use(501);\n' % k
    elif kind == 'R':           # shared tail ended by return instead of break
        s += ''.join('    use(%d);\n' % i for i in range(1, L + 1)) + '    return;\n'
        return s
    else:
        s += '    use(%d);\n' % (100 + k)
    return s + '    break;\n'


def fn(name, cases, L=7, join=12, guard=True, default=False):
    """cases: list of (value, kind) in textual order."""
    s = 'void %s(int c) {\n' % name
    if guard:
        s += '  if (cond()) {\n'
    s += '  switch (c) {\n' + ''.join(body(k, kind, L) for k, kind in cases)
    if default:
        s += '  default:\n    mk(99);\n    break;\n'
    s += '  }\n'
    if guard:
        s += '  } else { g = 0; g2 = 0; g3 = 1; }\n'
    s += ''.join('  if (g == %d) use(%d);\n' % (i + 1000, i + 1000) for i in range(join)) + '}\n'
    return s


def analyse(lines, cases):
    """-> (layout order of case values, {value: '<use calls in own block>[->value it jumps into]'})"""
    labels = {l[:-1]: i for i, l in enumerate(lines) if l.endswith(':')}
    table = [l.split()[1] for l in lines if l.startswith('DD')]
    vals = [k for k, kind in sorted(cases)]
    start = {}
    for v, lab in zip(range(len(table)), table):
        if v in vals: start[v] = labels[lab]
    order = sorted(start, key=start.get)
    def owner(i):
        best = None
        for v, p in start.items():
            if p <= i and (best is None or p > start[best]): best = v
        return best
    res = {}
    for v in order:
        i = start[v] + 1; n = 0
        while i < len(lines) and not lines[i].startswith(('jmp', 'ret')):
            if lines[i].startswith('call') and '?use@@' in lines[i]: n += 1
            i += 1
        tgt = ''
        m = re.match(r'jmp\s+(?:SHORT\s+)?(\$\w+)', lines[i]) if i < len(lines) else None
        if m:
            o = owner(labels[m.group(1)])
            tgt = '->%s' % (o if o is not None and o != v and lines[labels[m.group(1)] + 1].startswith(('call\t?mk', 'call\t?use', 'push')) and labels[m.group(1)] < max(start.values()) + 40 else 'join')
        res[v] = '%d%s' % (n, tgt)
    return order, res


def run(V, show=None):
    src = HDR + ''.join(fn(*a, **kw) for k, a, kw in V)
    asm, _ = ib.compile_asm(None, PFX, text=src)
    F = {f[0].split('(')[0].strip(): f[2] for f in ib.functions(asm)}
    for k, a, kw in V:
        lines = F[a[0]]
        order, res = analyse(lines, a[1])
        print('%-26s layout=%s  %s' % (k, order, ' '.join('%d:%s' % (kk, res[kk]) for kk in order)))
        if show == k:
            print('\n'.join('    ' + l for l in lines))


if __name__ == '__main__':
    show = sys.argv[sys.argv.index('--show') + 1] if '--show' in sys.argv else None
    std = [(0, 'U'), (1, 'U'), (2, 'U'), (3, 'S'), (4, 'U'), (5, 'U'), (6, 'U'), (7, 'S')]
    V = [
        ('3,7 shared', ('t_a', std), {}),
        ('3,7 text 7 first', ('t_b', [(7, 'S'), (0, 'U'), (1, 'U'), (2, 'U'), (3, 'S'), (4, 'U'), (5, 'U'), (6, 'U')]), {}),
        ('3,7 no guard', ('t_c', std), {'guard': False}),
        ('3,7 default', ('t_d', std), {'default': True}),
        ('3,7 evens E', ('t_e', [(0, 'E'), (1, 'U'), (2, 'E'), (3, 'S'), (4, 'E'), (5, 'U'), (6, 'E'), (7, 'S')]), {}),
        ('1,3,5,7 shared', ('t_f', [(k, 'S' if k % 2 else 'U') for k in range(8)]), {}),
        ('3,7 L=2', ('t_g', std), {'L': 2}),
        ('3,7 cases 0..3 only', ('t_h', [(0, 'U'), (1, 'S'), (2, 'U'), (3, 'S')]), {}),
        ('3,7 two cases only', ('t_i', [(3, 'S'), (7, 'S')]), {}),
        ('3,7 return', ('t_j', [(k, 'R' if k in (3, 7) else 'U') for k in range(8)]), {}),
    ]
    run(V, show)
