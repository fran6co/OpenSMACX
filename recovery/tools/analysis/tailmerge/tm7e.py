# EH-region factor without destructor code: objects whose inline destructor is empty.
import tm7, sys
tm7.HDR += ('struct E { ~E() {} int v; };\nstruct E2 { E2(); ~E2() {} int v; };\nstruct E3 { E3() { v = 0; } ~E3() {} int v; };\n'
            'struct E4 { ~E4() { g9 = 0; } };\n')
V = []
def add(k, A, **kw): V.append((k, tm7.fn('t_' + k, A, **kw)))
LAST = ['mk(99); if (g7) return; goto OUT;']
EPI = '  call();\nOUT:\n  use(777);\n'
def P(j, tail=None, pre=''):
    return '%smk(%d); g3 = %d; if (cond()) { %s }' % (pre, j * 7 + 3, j, tail or 'f2(%d, %d);' % (j, j + 100))
F = lambda j: 'mk(%d); g2 = %d; f3(%d, 2, 3);' % (j * 7 + 500, j, j)
def std(Pf, key, **kw):
    add(key, [Pf(1), F(1), Pf(2), Pf(3), F(2), Pf(4)] + LAST, epi=EPI, **kw)
std(P, 'base')
for T in ('E', 'E2', 'E3'):
    std(lambda j, T=T: P(j, tail='%s e; e.v = %d; f2(%d, 1);' % (T, j, j)), 'in_' + T)
    std(lambda j, T=T: P(j, pre='%s e; e.v = %d; ' % (T, j)), 'arm_' + T)
    std(lambda j, T=T: P(j, tail='%s e; e.v = %d; f2(%d, 1);' % (T, j, j)), 'in_%s_fnD' % T, pro='  D d;\n')
std(lambda j: P(j, tail='E2 e; f2(e.v, 1);'), 'in_E2_samearg', pro='  D d;\n')
if __name__ == '__main__':
    res = tm7.run(V, show=sys.argv[1:])
    tm7.report(res, V)
