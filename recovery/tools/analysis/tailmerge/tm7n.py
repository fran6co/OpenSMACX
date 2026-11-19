# label-after-siblings with ternary-argument pops (probe's GOT/TOOK shape), and with an A-partner arm earlier.
import tm7, sys
tm7.HDR += 'extern int gt, gu, gv; extern void f3b(int, int, int);\n'
V = []
def add(k, A, **kw): V.append((k, tm7.fn('t_' + k, A, **kw)))
LAST = ['mk(99); if (g7) return; goto OUT;']
EPI = '  call();\nOUT:\n  use(777);\n'
Q = lambda j: 'mk(%d); g4 = %d; use(%d); g5 = %d;' % (j * 7 + 900, j, j, j)
A = lambda j: 'mk(%d); g2 = %d; %s(%d, 2, 3);' % (j * 7 + 500, j, ('f3', 'f3b')[j % 2], j)
T1 = 'if (g4 == 3) { use(1); f2(gt ? 1 : 2, gu ? 3 : 4); } else if (g5 == 3) { use(2); f2(gt ? 1 : 2, gu ? 5 : 6); }'
P1 = 'if (g4 == 3) { use(1); f2(1, 1); } else if (g5 == 3) { use(2); f2(2, 2); }'
for nm, sib in (('tern', T1), ('plain', P1)):
    for lab in ('', ' LB: ;'):
        k = nm + ('_lab' if lab else '')
        add(k + '_Q', [Q(1), 'mk(1700); ' + sib + lab, Q(2)] + LAST, epi=EPI)
        add(k + '_A', [Q(1), A(1), 'mk(1700); ' + sib + lab, Q(2), A(2)] + LAST, epi=EPI)
        add(k + '_Ad', [Q(1), A(1), 'mk(1700); ' + sib + lab, Q(2), A(2)] + LAST, epi=EPI, pro='  D d;\n')
if __name__ == '__main__' and 'v2' not in sys.argv:
    res = tm7.run(V, show=sys.argv[1:])
    tm7.report(res, V)
V2 = []
def add2(k, A, **kw): V2.append((k, tm7.fn('t_c' + k, A, **kw)))
T2 = 'int e = cond(); g9 -= e; if (g4 == 3) { use(e); f2(e ? 1 : 2, gu ? 3 : 4); } else if (g5 == 3) { use(2); f2(e ? 1 : 2, gu ? 5 : 6); }'
for lab in ('', ' LB: ;'):
    add2('local' + lab[1:3], [Q(1), A(1), 'mk(1700); ' + T2 + lab, Q(2), A(2)] + LAST, epi=EPI)
    add2('local_after_join' + lab[1:3], [Q(1), A(1), 'mk(1700); ' + T2 + lab, Q(2), A(2)], join='use(g); call(); use(g9);')
    add2('local_nolast' + lab[1:3], [Q(1), A(1), 'mk(1700); ' + T2 + lab, Q(2), A(2), 'mk(99); return;'])
if 'v2' in sys.argv:
    res = tm7.run(V2, show=sys.argv[2:]); tm7.report(res, V2)
