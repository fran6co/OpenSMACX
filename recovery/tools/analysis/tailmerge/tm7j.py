# Sibling tails inside one arm (inner if/else-if both ending in f2(...)): which shapes merge?
# Q arms are unshared filler; host=?f2 and f2-copies=1 means the siblings merged.
import tm7, sys
V = []
def add(k, A, **kw): V.append((k, tm7.fn('t_' + k, A, **kw)))
LAST = ['mk(99); if (g7) return; goto OUT;']
EPI = '  call();\nOUT:\n  use(777);\n'
Q = lambda j: 'mk(%d); g4 = %d; use(%d); g5 = %d;' % (j * 7 + 900, j, j, j)
A = lambda j: 'mk(%d); g2 = %d; %s(%d, 2, 3);' % (j * 7 + 500, j, ('f3', 'f3b', 'f3c')[j % 3], j)
tm7.HDR += 'extern void f3b(int, int, int); extern void f3c(int, int, int); extern int gt, gu, gv;\n'
SIB = {
 'plain': 'if (g4 == 3) { use(1); f2(1, 1); } else if (g5 == 3) { use(2); f2(2, 2); }',
 'tern': 'if (g4 == 3) { use(1); f2(gt ? 1 : 2, gu ? 3 : 4); } else if (g5 == 3) { use(2); f2(gt ? 5 : 6, gu ? 3 : 4); }',
 'tern_same': 'if (g4 == 3) { use(1); f2(gt ? 1 : 2, gu ? 3 : 4); } else if (g5 == 3) { use(2); f2(gt ? 1 : 2, gu ? 3 : 4); }',
 'or_cond': 'if (gv & 1) { if (g4 == 3 || g5 == 3) f2(1, 1); } else { if (g4 == 3 || g5 == 3) f2(2, 2); }',
 'or_cond2': 'if (gv) { use(5); if (gv & 1) { if (g4 == 3 || g5 == 3) f2(1, 1); } else { if (g4 == 3 || g5 == 3) f2(2, 2); } } else { if (gv & 1) { if (g4 == 3 || g5 == 3) f2(3, 1); } else { if (g4 == 3 || g5 == 3) f2(4, 2); } }',
 'two_ifs': 'if (g4 == 3) { use(1); f2(1, 1); } if (g5 == 3) { use(2); f2(2, 2); }',
 'sw': 'switch (g4) { case 3: use(1); f2(1, 1); break; case 5: use(2); f2(2, 2); break; }',
}
for k, s in SIB.items():
    add(k, [Q(1), A(1), 'mk(1700); ' + s, A(2)] + LAST, epi=EPI)
    add(k + '_D', [Q(1), A(1), 'mk(1700); ' + s, A(2)] + LAST, epi=EPI, pro='  D d;\n')
if __name__ == '__main__':
    res = tm7.run(V, show=sys.argv[1:])
    tm7.report(res, V)
