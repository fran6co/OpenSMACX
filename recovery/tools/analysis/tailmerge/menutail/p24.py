"""Cross-jump blocker screen 5 (par-6 r53): what the switch's end label is (loop back edge, loop exit,
consecutive labels, goto groups).  psave-calls 1 = merged, 4 = kept."""
import sys; from cj import *
import cj
from p1 import INL, INLBN
V = []
AFTER = '  if (gx & 0x40) { switch (c) { case 40: f0(); return; case 41: use(41); return; } }\n'
def cases(n, end='break;', ends=None):
    s = '  case 1: f0(); break;\n'
    for i in range(3):
        s += '  case %d: tg%d_%s(); %s\n' % (10 + i, i, n, ends[i] if ends else end)
    s += '  case 13: { gx ^= 0x40000; dmap(1); } break;\n'
    s += '  case 14: bn_%s(); %s\n' % (n, ends[3] if ends else end)
    return s + '  case 30: use(30); break;\n'
def mk(n, body):
    V.append((n, INL.replace('%s', n) + INLBN % n + 'void t_%s(int c) {\n%s}\n' % (n, body)))
mk('base', '  switch (c) {\n%s  }\n%s' % (cases('base'), AFTER))
mk('while1_back', '  while (1) {\n  switch (c) {\n%s  }\n  c = cond(); if (c == 0) return;\n  }\n' % cases('while1_back'))
mk('while1_backonly', '  while (1) {\n  switch (c) {\n%s  }\n  }\n' % cases('while1_backonly'))
mk('while_cond', '  while (cond()) {\n  switch (c) {\n%s  }\n  }\n' % cases('while_cond'))
mk('while1_cont', '  while (1) {\n  switch (c) {\n%s  }\n  c = cond(); if (c == 0) return;\n  }\n' % cases('while1_cont', 'continue;').replace('break;\n  case 13', 'continue;\n  case 13'))
mk('goto2', '  switch (c) {\n%s  }\nLA: ;\nLB: ;\n%s' % (cases('goto2', ends=['goto LA;', 'goto LA;', 'goto LB;', 'goto LB;']), AFTER))
mk('goto_one', '  switch (c) {\n%s  }\nLA: ;\n%s' % (cases('goto_one', 'goto LA;'), AFTER))
mk('goto_brk', '  switch (c) {\n%s  }\nLA: ;\n%s' % (cases('goto_brk', ends=['goto LA;', 'break;', 'goto LA;', 'break;']), AFTER))
mk('goto_sep', '  switch (c) {\n%s  }\nLA: ;\n  use(77);\nLB: ;\n%s' % (cases('goto_sep', ends=['goto LB;'] * 4), AFTER))
mk('rettail', '  switch (c) {\n%s  }\n%s' % (cases('rettail', 'return;'), AFTER))
run(V, show=sys.argv[1:])
