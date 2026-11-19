# Join with no source fallthrough predecessor: the last arm leaves (return / goto past the join).
import tm7, sys
V = []
def add(k, A, **kw): V.append((k, tm7.fn('t_' + k, A, **kw)))
LAST = ['mk(99); if (g7) return; goto OUT;']
EPI = '  call();\nOUT:\n  use(777);\n'
for n in (1, 2, 3, 6):
    add('ret_last_n%d' % n, tm7.arms(n=n) + LAST, epi=EPI)
add('ret_last_ret', tm7.arms() + ['mk(99); return;'])
# f3 arms more numerous than pop arms
add('f3x4_pop2', tm7.arms(n=2) + ['mk(61); use(61); f3(1, 2, 3);', 'mk(62); use(62); f3(1, 2, 3);'] + LAST, epi=EPI)
# probe-like: the pops nested in second-level ifs
add('nested', [ 'mk(%d); if (cond()) { mk(%d); if (g4) { f2(%d, 1); } } else { if (g5) f2(%d, 2); }' % (j, j+20, j, j) for j in range(3)] + ['mk(50); use(50); f3(1, 2, 3);'] + LAST, epi=EPI)
if __name__ == '__main__':
    res = tm7.run(V, show=sys.argv[1:])
    tm7.report(res, V)
