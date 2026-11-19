import tm, ib, sys, re
from tm3 import src
from concurrent.futures import ThreadPoolExecutor
def one(n, pfx):
    pre = ''.join('void dummy_%d(int c) { if (c) mk(%d); }\n' % (i, i) for i in range(n))
    V = [('T1', pre + src('T1', sys.argv[1] if len(sys.argv) > 1 else 'tl(0); use(1);'))]
    res, asm = tm.run(V, pfx=str(pfx))
    f = [x for x in ib.functions(asm) if 't_T1' in x[0]][0]
    labs = [int(l[2:-1]) for l in f[2] if re.match(r'\$L\d+:', l)]
    return n, min(labs), sum(1 for l in f[2] if 'g3@@' in l and l.startswith('mov') and l.split(',')[0].endswith('A'))
N = list(range(0, 80, 1))
def lane(k):
    return [one(n, 21 + k) for n in N[k::5]]
out = []
with ThreadPoolExecutor(5) as ex:
    for o in ex.map(lane, range(5)):
        out += o
out.sort()
print(' '.join('%d:%s' % (n, 'M' if s > 3 else '.') for n, l, s in out))
print('labels', [(n, l) for n, l, s in out][:6])
