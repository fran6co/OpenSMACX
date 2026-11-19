# (h) function size: filler blocks before / after the chain.
import tm7, sys
V = []
def add(k, A, **kw): V.append((k, tm7.fn('t_' + k, A, **kw)))
LAST = ['mk(99); if (g7) return; goto OUT;']
def P(j):
    return 'mk(%d); g3 = %d; if (cond()) { f2(%d, %d); }' % (j * 7 + 3, j, j, j + 100)
F = lambda j: 'mk(%d); g2 = %d; f3(%d, 2, 3);' % (j * 7 + 500, j, j)
A = [P(1), F(1), P(2), P(3), F(2), P(4)] + LAST
fill = lambda n, b: ''.join('  if (g%d == %d) use(%d);\n' % (2 + i % 7, i, i + b) for i in range(n))
for n in [int(x) for x in (sys.argv[1].split(',') if len(sys.argv) > 1 else '0,100,300,600,1000,1500,2500'.split(','))]:
    add('pre%d' % n, A, pro=fill(n, 10000), epi='  call();\nOUT:\n  use(777);\n')
    add('post%d' % n, A, epi='  call();\n' + fill(n, 20000) + 'OUT:\n  use(777);\n')
if __name__ == '__main__':
    res = tm7.run(V)
    tm7.report(res, V)
