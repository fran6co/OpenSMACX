#!/usr/bin/env python3
"""grid.py: pairwise grid (c has one more reference at the very end, not in its vector).  Two parameters c and d with reference vectors over the blocks of a fixed skeleton
(B0 [C] B1 [C] B2 by default; --sep '{ C }' for loops); p and q hold esi/edi.  For each d vector and each c pattern
with one free coordinate K, print the first K at which c is the one held in ebx everywhere.
    python3 grid.py [--sep if|loop] [--local]      (--local: d is a local defined at its first reference)"""
import sys, os, re, io, contextlib, itertools
from concurrent.futures import ThreadPoolExecutor
import run
HERE = run.HERE
KS = range(0, 18)


def fn(name, c, d, sep, local, cl=False):
    o = ['void %s(%s) { D(p); D(q); %s %s' % (name, ', '.join((['int c'] if not cl else []) + (['int d'] if not local else [])), 'U(p); ' * 30, 'U(q); ' * 30)]
    cdef = ddef = False
    for i in range(len(c)):
        if i:
            o.append(sep)
        if cl and not cdef and c[i]:
            o.append('D(c);'); cdef = True
            o.append('U(c); ' * (c[i] - 1))
        else:
            o.append('U(c); ' * c[i])
        if local and not ddef and d[i]:
            o.append('D(d);'); ddef = True
            o.append('U(d); ' * (d[i] - 1))
        else:
            o.append('U(d); ' * d[i])
    o.append('U(c); U(p); U(q); }')
    return ' '.join(o)


def main():
    sep = {'if': 'if (cond()) { call(); }', 'loop': 'while (cond()) { call(); }'}[sys.argv[sys.argv.index('--sep') + 1] if '--sep' in sys.argv else 'if']
    local = '--local' in sys.argv
    D = [(8, 0, 0), (0, 8, 0), (0, 0, 8), (4, 4, 0), (4, 0, 4), (0, 4, 4), (1, 8, 1), (1, 0, 8), (8, 0, 1), (0, 16, 0), (4, 0, 0), (0, 4, 0), (0, 0, 4)]
    C = ['K00', '0K0', '00K', 'K10', '1K0', '10K', '01K', 'K40', '4K0', 'K03', '0K3', '40K', '04K', 'K43']
    if local:
        D = [(8, 0, 0), (0, 8, 0), (0, 0, 8), (4, 4, 0), (0, 4, 4)]
    jobs = []
    for di, d in enumerate(D):
        for pat in C:
            for K in KS:
                c = tuple(K if ch == 'K' else int(ch) for ch in pat)
                jobs.append(('g%d_%s_k%d' % (di, pat, K), c, d))
    n = int(os.environ.get('FE_JOBS', '5'))
    base = int(os.environ.get('FE_PFX_BASE', '2'))
    chunks = [jobs[i::n] for i in range(n)]
    res = {}

    def work(i):
        path = '%s/g_grid%d.cpp' % (HERE, i)
        open(path, 'w').write('#include "probe.h"\n' + '\n'.join(fn(nm, c, d, sep, local) for nm, c, d in chunks[i]) + '\n')
        return run.report(path, str(base + i))
    with contextlib.redirect_stdout(io.StringIO()):
        with ThreadPoolExecutor(n) as ex:
            for r in ex.map(work, range(n)):
                res.update(r)
    print('rows: d vector; columns: c pattern; cell: first K where c holds a register at every site / (what d has there)')
    print('%-10s' % 'd' + ''.join('%-8s' % p for p in C))
    for di, d in enumerate(D):
        row = []
        for pat in C:
            first = None
            for K in KS:
                h = res.get('g%d_%s_k%d' % (di, pat, K), {})
                cw = h.get('c') and all(x in run.CS for x in h['c'])
                dw = h.get('d') and all(x in run.CS for x in h['d'])
                if cw and not dw and first is None:
                    first = K
            row.append('-' if first is None else str(first))
        print('%-10s' % ''.join(map(str, d)) + ''.join('%-8s' % x for x in row))
    if '--dump' in sys.argv:
        for k in sorted(res):
            print(k, res[k])


if __name__ == '__main__':
    main()
