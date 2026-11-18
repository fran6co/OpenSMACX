#!/usr/bin/env python3
"""th.py: ad-hoc thresholds.  Reads lines `name | signature-args | spec with {K}` from stdin (spec language of gen.py),
compiles K = 0..N-1 of each and prints the homes of c and d per K as a compact string (C = c in a register at every
site, D = d, B = both, - = neither) plus the first K with C.     python3 th.py [N] [--oy] < specs"""
import sys, os, io, contextlib
from concurrent.futures import ThreadPoolExecutor
import gen, run


def evaluate(lines, N=16, oy=False, vs='cd'):
    jobs = []
    for li, l in enumerate(lines):
        name, args, spec = [x.strip() for x in l.split('|', 2)]
        for K in range(N):
            jobs.append(('t%d_k%d' % (li, K), 'void t%d_k%d(%s) { %s }' % (li, K, args, gen.body(spec.replace('{K}', str(K)).replace('{K+1}', str(K + 1))))))
    n = int(os.environ.get('FE_JOBS', '5')); base = int(os.environ.get('FE_PFX_BASE', '2'))
    res = {}

    def work(i):
        path = '%s/g_th%d.cpp' % (run.HERE, i)
        open(path, 'w').write('#include "probe.h"\nstruct T : Obj { %s };\n' % ' '.join('void %s;' % j[1].split('T::')[1].split(' {')[0] for j in jobs[i::n] if 'T::' in j[1])
                              + '\n'.join(j[1] for j in jobs[i::n]) + '\n')
        return run.report(path, str(base + i), oy)
    with contextlib.redirect_stdout(io.StringIO()):
        with ThreadPoolExecutor(n) as ex:
            for r in ex.map(work, range(n)):
                res.update(r)
    out = []
    for li, l in enumerate(lines):
        s = ''
        for K in range(N):
            h = res.get('t%d_k%d' % (li, K), {})
            a = bool(h.get(vs[0])) and all(x in run.CS for x in h[vs[0]])
            b = bool(h.get(vs[1])) and all(x in run.CS for x in h[vs[1]])
            s += 'B' if a and b else 'C' if a else 'D' if b else '-'
        out.append((l.split('|')[0].strip(), s, res))
    return out


if __name__ == '__main__':
    N = int(sys.argv[1]) if len(sys.argv) > 1 and sys.argv[1].isdigit() else 16
    lines = [l for l in sys.stdin.read().split('\n') if l.strip() and not l.startswith('#')]
    for name, s, _ in evaluate(lines, N, '--oy' in sys.argv):
        print('%-28s %s  first C at %s' % (name, s, s.find('C') if 'C' in s else '-'))
