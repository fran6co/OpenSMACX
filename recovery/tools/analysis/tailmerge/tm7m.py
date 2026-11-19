# Persistent empty blocks between a tail and the join: user label, switch end, loop, goto-target.
import tm7, sys
tm7.HDR += 'extern int gt, gu, gv;\n'
V = []
def add(k, A, **kw): V.append((k, tm7.fn('t_' + k, A, **kw)))
LAST = ['mk(99); if (g7) return; goto OUT;']
EPI = '  call();\nOUT:\n  use(777);\n'
Q = lambda j: 'mk(%d); g4 = %d; use(%d); g5 = %d;' % (j * 7 + 900, j, j, j)
SIB = 'if (g4 == 3) { use(1); f2(1, 1); } else if (g5 == 3) { use(2); f2(2, 2); }'
S = {
 'none': SIB,
 'label_after': SIB + ' LB: ;',
 'label_after_used': 'if (gv) goto LB; ' + SIB + ' LB: ;',
 'label_before': 'LB: ; ' + SIB,
 'label_in_then': 'if (g4 == 3) { use(1); f2(1, 1); LB: ; } else if (g5 == 3) { use(2); f2(2, 2); }',
 'switch_end': 'switch (gv) { case 1: use(5); break; case 2: ' + SIB + ' break; case 3: use(6); break; }',
 'while_end': 'while (gv) { gv--; ' + SIB + ' }',
 'brk_loop': 'for (;;) { ' + SIB + ' break; }',
}
for k, s in S.items():
    add(k, [Q(1), 'mk(1700); ' + s, Q(2)] + LAST, epi=EPI)
# label at the end of an arm whose tail would be the host's partner (arm-level)
P = lambda j, lab='': 'mk(%d); g3 = %d; f2(%d, 1); %s' % (j * 7 + 3, j, j, lab)
add('arms_plain', [Q(1), P(1), P(2), P(3)] + LAST, epi=EPI)
add('arms_label1', [Q(1), P(1, 'LA: ;'), P(2), P(3)] + LAST, epi=EPI)
add('arms_label_all', [Q(1), P(1, 'LA: ;'), P(2, 'LB: ;'), P(3, 'LC: ;')] + LAST, epi=EPI)
if __name__ == '__main__':
    res = tm7.run(V, show=sys.argv[1:])
    tm7.report(res, V)
