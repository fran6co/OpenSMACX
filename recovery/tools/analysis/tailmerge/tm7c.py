# Who hosts a join with no source fallthrough predecessor: first arm, most-shared tail, or largest?
import tm7, sys
V = []
def add(k, A, **kw): V.append((k, tm7.fn('t_' + k, A, **kw)))
LAST = ['mk(99); if (g7) return; goto OUT;']
EPI = '  call();\nOUT:\n  use(777);\n'
P = lambda j: 'mk(%d); g3 = %d; if (cond()) { f2(%d, %d); }' % (j * 7 + 3, j, j, j + 100)
F = lambda j: 'mk(%d); g2 = %d; f3(%d, 2, 3);' % (j * 7 + 500, j, j)
add('F1_P3', [F(1), P(1), P(2), P(3)] + LAST, epi=EPI)
add('P3_F1', [P(1), P(2), P(3), F(1)] + LAST, epi=EPI)
add('F3_P1', [F(1), F(2), F(3), P(1)] + LAST, epi=EPI)
add('P1_F3', [P(1), F(1), F(2), F(3)] + LAST, epi=EPI)
add('F2_P2', [F(1), F(2), P(1), P(2)] + LAST, epi=EPI)
add('P2_F2', [P(1), P(2), F(1), F(2)] + LAST, epi=EPI)
add('F1_P1', [F(1), P(1)] + LAST, epi=EPI)
add('P1_F1', [P(1), F(1)] + LAST, epi=EPI)
if __name__ == '__main__':
    res = tm7.run(V, show=sys.argv[1:])
    tm7.report(res, V)
