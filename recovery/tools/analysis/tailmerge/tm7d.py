# Host-choice probe with 4 pop arms (P) vs 2 f3 arms (F): base host = f2 with all pops merged.
# A factor that blocks the pop merge shows as host=f3, f2-copies=4.
import tm7, sys
V = []
def add(k, A, **kw): V.append((k, tm7.fn('t_' + k, A, **kw)))
LAST = ['mk(99); if (g7) return; goto OUT;']
EPI = '  call();\nOUT:\n  use(777);\n'
def P(j, tail=None, pre=''):
    return '%smk(%d); g3 = %d; if (cond()) { %s }' % (pre, j * 7 + 3, j, tail or 'f2(%d, %d);' % (j, j + 100))
F = lambda j: 'mk(%d); g2 = %d; f3(%d, 2, 3);' % (j * 7 + 500, j, j)
def std(Pf, **kw):
    add(kw.pop('key'), [Pf(1), F(1), Pf(2), Pf(3), F(2), Pf(4)] + LAST, epi=EPI, **kw)
std(P, key='base')
std(lambda j: P(j, tail=('f2x(%d, 1);' if j % 2 else 'f2(%d, 1);') % j), key='alt_callee')
std(P, key='eh_fn', pro='  D d;\n')
std(P, key='eh_fn_big', pro='  D d; int big[3000]; big[g] = 1; use(big[g2]);\n')
std(P, key='big', pro='  int big[3000]; big[g] = 1; use(big[g2]);\n')
std(lambda j: P(j, pre='{ D d; use(d.v); } '), key='eh_before')
std(lambda j: P(j, tail='{ D d; f2(%d, 1); }' % j), key='eh_around_dtor')
std(lambda j: P(j, tail='f2(%d, 1); { D d; }' % j), key='eh_after')
std(lambda j: P(j, tail='f2(g%d, 1);' % (j + 3)), key='glob_arg')
std(lambda j: P(j, tail='f2(%d, 1); g = g;' % j), key='selfstore')
std(lambda j: P(j, tail='f2(%d, 1); call();' % j), key='call_after')
std(lambda j: P(j, tail='f2((g4 ? %d : 7), (g5 ? 9 : 8));' % j), key='ternary_args')
if __name__ == '__main__':
    res = tm7.run(V, show=sys.argv[1:])
    tm7.report(res, V)
