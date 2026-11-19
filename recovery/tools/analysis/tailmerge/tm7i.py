# Host choice F F P P P with the first F arm made more complex: loop, early return, nested if/else, EH.
import tm7, sys
V = []
def add(k, A, **kw): V.append((k, tm7.fn('t_' + k, A, **kw)))
LAST = ['mk(99); if (g7) return; goto OUT;']
EPI = '  call();\nOUT:\n  use(777);\n'
P = lambda j: 'mk(%d); g3 = %d; if (cond()) { f2(%d, %d); }' % (j * 7 + 3, j, j, j + 100)
F = lambda j, pre='': 'mk(%d); %s g2 = %d; f3(%d, 2, 3);' % (j * 7 + 500, pre, j, j)
Fp = lambda j: 'mk(%d); g2 = %d; if (cond()) { f2(%d, %d); } f3(%d, 2, 3);' % (j * 7 + 500, j, j, j + 300, j)
PRE = {
    'plain': '',
    'loop': 'int i = g4; do { if (ga[i] == 5) break; i = cond(); } while (i >= 0);',
    'ret': 'if (g4 == 99) { use(1); return; }',
    'nest': 'if (g4 == 0) { use(1); } else if (g4 == 98) { use(2); } else { use(3); }',
    'loopret': 'int i = g4; if (i < 0) return; do { if (ga[i] == 5) break; i = cond(); } while (i >= 0); if (i >= 0) use(i);',
    'innerpop': 'if (cond()) f2(55, 56);',
}
for k, pre in PRE.items():
    add('FFPPP_' + k, [F(1, pre), F(2), P(1), P(2), P(3)] + LAST, epi=EPI)
    add('FFPPP_' + k + '_D', [F(1, pre), F(2), P(1), P(2), P(3)] + LAST, epi=EPI, pro='  D d;\n')
add('FpFPPP', [Fp(1), F(2), P(1), P(2), P(3)] + LAST, epi=EPI)
if __name__ == '__main__':
    res = tm7.run(V, show=sys.argv[1:])
    tm7.report(res, V)
V2 = []
def add2(k, A, **kw): V2.append((k, tm7.fn('t_v2' + k, A, **kw)))
for n in (0, 2, 5, 10, 20):
    st = ' '.join('ga[%d] = %d;' % (i, i) for i in range(n))
    add2('F1long%d' % n, [F(1, st), F(2), P(1), P(2), P(3)] + LAST, epi=EPI)
    add2('F2long%d' % n, [F(1), F(2, st), P(1), P(2), P(3)] + LAST, epi=EPI)
    add2('Plong%d' % n, [F(1), F(2)] + ['mk(%d); g3 = %d; if (cond()) { %s f2(%d, %d); }' % (j * 7 + 3, j, st, j, j + 100) for j in (1, 2, 3)] + LAST, epi=EPI)
# F arms whose tail block begins at a label (inner if before it) but F1 alone
add2('F1if_F2if', [F(1, 'if (g4) use(4);'), F(2, 'if (g5) use(5);'), P(1), P(2), P(3)] + LAST, epi=EPI)
add2('F2if', [F(1), F(2, 'if (g5) use(5);'), P(1), P(2), P(3)] + LAST, epi=EPI)
add2('Pplain', [F(1), F(2)] + ['mk(%d); g3 = %d; f2(%d, %d);' % (j * 7 + 3, j, j, j + 100) for j in (1, 2, 3)] + LAST, epi=EPI)
if __name__ == '__main__' and 'v2' in sys.argv:
    res = tm7.run(V2)
    tm7.report(res, V2)
