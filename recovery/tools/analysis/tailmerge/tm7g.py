# Host choice: most-shared tail vs first arm (source order) whose tail has a partner.
import tm7, sys
V = []
def add(k, A, **kw): V.append((k, tm7.fn('t_' + k, A, **kw)))
LAST = ['mk(99); if (g7) return; goto OUT;']
EPI = '  call();\nOUT:\n  use(777);\n'
P = lambda j: 'mk(%d); g3 = %d; if (cond()) { f2(%d, %d); }' % (j * 7 + 3, j, j, j + 100)
F = lambda j: 'mk(%d); g2 = %d; f3(%d, 2, 3);' % (j * 7 + 500, j, j)
Q = lambda j: 'mk(%d); g4 = %d; use(%d); g5 = 1;' % (j * 7 + 900, j, j)   # unshared tail
pat = {'F': F, 'P': P, 'Q': Q}
for spec in (sys.argv[1].split(',') if len(sys.argv) > 1 else ['FFPPP', 'FFPPPPP', 'QFFPPPPP', 'PFFPPP', 'FPFPPP', 'FFFPP', 'PPFFF', 'QPPQFFF']):
    cnt = {}
    A = []
    for c in spec:
        cnt[c] = cnt.get(c, 0) + 1
        A.append(pat[c](cnt[c]))
    add(spec, A + LAST, epi=EPI)
if __name__ == '__main__':
    res = tm7.run(V)
    tm7.report(res, V)
