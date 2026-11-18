#!/usr/bin/env python3
"""homes.py N [vars] [--oy] < specs: like th.py (lines `name | args | spec with {K}`), but prints for every K the
register of each listed variable (S=esi D=edi B=ebx P=ebp m=memory *=changes along the function)."""
import sys, os, io, contextlib, re
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from concurrent.futures import ThreadPoolExecutor
import gen, run
def evaluate(lines, N=16, oy=False, pick=None):
    jobs = []
    for li, l in enumerate(lines):
        name, args, spec = [x.strip() for x in l.split('|', 2)]
        for K in range(N):
            jobs.append(('t%d_k%d' % (li, K), 'void t%d_k%d(%s) { %s }' % (li, K, args, gen.body(spec.replace('{K}', str(K))))))
    n = int(os.environ.get('FE_JOBS', '5')); base = int(os.environ.get('FE_PFX_BASE', '2'))
    res = {}
    def work(i):
        path = '%s/g_th%d.cpp' % (run.HERE, i)
        open(path, 'w').write('#include "probe.h"\nstruct T : Obj { %s };\n' % ' '.join('void %s;' % j[1].split('T::')[1].split(' {')[0] for j in jobs[i::n] if 'T::' in j[1]) + '\n'.join(j[1] for j in jobs[i::n]) + '\n')
        return run.report(path, str(base + i), oy)
    with contextlib.redirect_stdout(io.StringIO()):
        with ThreadPoolExecutor(n) as ex:
            for r in ex.map(work, range(n)):
                res.update(r)
    for li, l in enumerate(lines):
        name = l.split('|')[0].strip()
        vs = pick or 'cd'
        cells = []
        for K in range(N):
            h = res.get('t%d_k%d' % (li, K), {})
            cell = ''
            for v in vs:
                x = h.get(v, ['?'])
                cell += {'esi':'S','edi':'D','ebx':'B','ebp':'P'}.get(x[0], 'm') if len(x)==1 else '*'
            cells.append(cell)
        print('%-16s %s' % (name, ' '.join(cells)))
if __name__ == '__main__':
    N = int(sys.argv[1]); pick = sys.argv[2] if len(sys.argv) > 2 and sys.argv[2] != '--oy' else None
    lines = [l for l in sys.stdin.read().split('\n') if l.strip() and not l.startswith('#')]
    evaluate(lines, N, '--oy' in sys.argv, pick)
