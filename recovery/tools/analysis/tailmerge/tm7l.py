# Sibling merge (inner if/else-if both ending in a pop) with invisible differences:
# inline wrappers around the call, empty inline after it, an inline with its own return label, goto.
import tm7, sys
tm7.HDR += ('extern int gt, gu, gv;\ninline void w2(int a, int b) { f2(a, b); }\ninline void nop_i() {}\n'
            'inline int wr(int a, int b) { if (gt) return 0; f2(a, b); return 1; }\n'
            'inline int wr2(int a, int b) { f2(a, b); return 1; }\n'
            'struct W { void pop(int a, int b) { f2(a, b); } static void spop(int a, int b) { f2(a, b); } }; extern W wobj;\n')
V = []
def add(k, A, **kw): V.append((k, tm7.fn('t_' + k, A, **kw)))
LAST = ['mk(99); if (g7) return; goto OUT;']
EPI = '  call();\nOUT:\n  use(777);\n'
Q = lambda j: 'mk(%d); g4 = %d; use(%d); g5 = %d;' % (j * 7 + 900, j, j, j)
def sib(t1, t2): return 'mk(1700); if (g4 == 3) { use(1); %s } else if (g5 == 3) { use(2); %s }' % (t1, t2)
C = {
 'plain': ('f2(1, 1);', 'f2(2, 2);'),
 'w2_both': ('w2(1, 1);', 'w2(2, 2);'),
 'w2_one': ('w2(1, 1);', 'f2(2, 2);'),
 'nop_after': ('f2(1, 1); nop_i();', 'f2(2, 2); nop_i();'),
 'wr_both': ('wr(1, 1);', 'wr(2, 2);'),
 'wr2_both': ('wr2(1, 1);', 'wr2(2, 2);'),
 'member_both': ('wobj.pop(1, 1);', 'wobj.pop(2, 2);'),
 'spop_both': ('W::spop(1, 1);', 'W::spop(2, 2);'),
 'goto_after': ('f2(1, 1); goto J;', 'f2(2, 2); goto J;'),
 'semi': ('f2(1, 1); ;', 'f2(2, 2); {}'),
}
for k, (a, b) in C.items():
    epi = EPI if 'goto' not in k else EPI
    join = 'J: use(g); call();' if 'goto' in k else 'use(g); call();'
    add(k, [Q(1), sib(a, b), Q(2)] + LAST, epi=epi, join=join)
if __name__ == '__main__':
    res = tm7.run(V, show=sys.argv[1:]) if 'v2' not in sys.argv else []
    tm7.report(res, V)
V2 = []
def add2(k, A, **kw): V2.append((k, tm7.fn('t_b' + k, A, **kw)))
AFT = {
 'none': '',
 'nop_inline': 'nop_i();',
 'deadlocal': 'int dead = g4; dead = 0;',
 'label': 'LB: ;',
 'empty_if': 'if (g6) {}',
 'expr': 'g4;',
 'inline_ret': 'wr2(7, 7);',
 'do0': 'do { } while (0);',
}
for k, a in AFT.items():
    add2(k, [Q(1), sib('f2(1, 1);', 'f2(2, 2);') + ' ' + a, Q(2)] + LAST, epi=EPI)
    add2(k + '_wrapped', [Q(1), '{ ' + sib('f2(1, 1);', 'f2(2, 2);') + ' } ' + a, Q(2)] + LAST, epi=EPI)
if __name__ == '__main__' and 'v2' in sys.argv:
    res = tm7.run(V2)
    tm7.report(res, V2)
