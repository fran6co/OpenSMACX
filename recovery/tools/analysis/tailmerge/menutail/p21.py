"""Cross-jump blocker screen 2 (par-6 r53): the jump TARGET of the toggle tails.  Same inline toggles as p1,
reached through nested switches, separate labels at the join, an if-chain, etc.  psave-calls 1 = merged."""
import sys; from cj import *
import cj
from p1 import INL, INLBN
V = []
def mk(name, body, pre=''):
    s = INL.replace('%s', name) + INLBN % name + pre
    s += 'void t_%s(int c) {\n%s\n  if (gx & 0x40) { switch (c) { case 40: f0(); return; case 41: use(41); return; } }\n}\n' % (name, body)
    V.append((name, s))
T = lambda n, i: 'tg%d_%s();' % (i, n)
B = lambda n: 'bn_%s();' % n
def flat(n, wrap=lambda n, cs: cs):
    return '  switch (c) {\n  case 1: f0(); break;\n' + wrap(n, ''.join('  case %d: %s break;\n' % (10 + i, T(n, i)) for i in range(3))
        + '  case 13: { gx ^= 0x40000; dmap(1); } break;\n  case 14: %s break;\n' % B(n)) + '  case 30: use(30); break;\n  }\n'
mk('base', flat('base'))
# toggles inside an inner switch in the default of the outer switch
mk('inner_default', '  switch (c) {\n  case 1: f0(); break;\n  case 30: use(30); break;\n  default:\n    switch (c) {\n'
   + ''.join('    case %d: %s break;\n' % (10 + i, T('inner_default', i)) for i in range(3))
   + '    case 14: %s break;\n    }\n    break;\n  }\n' % B('inner_default'))
# inner switch on the command group
mk('inner_group', '  switch (c >> 8) {\n  case 1: f0(); break;\n  case 2:\n    switch (c) {\n'
   + ''.join('    case 0x%x: %s break;\n' % (0x209 + i, T('inner_group', i)) for i in range(3))
   + '    case 0x210: %s break;\n    }\n    break;\n  case 3: use(30); break;\n  }\n' % B('inner_group'))
# half the toggles in an inner switch, half in the outer
mk('split', '  switch (c) {\n  case 1: f0(); break;\n  case 10: %s break;\n  case 14: %s break;\n  default:\n    switch (c) {\n'
   % (T('split', 0), B('split')) + ''.join('    case %d: %s break;\n' % (10 + i, T('split', i)) for i in (1, 2)) + '    }\n    break;\n  }\n')
# if/else-if chain
mk('ifchain', '  if (c == 1) f0();\n' + ''.join('  else if (c == %d) %s\n' % (10 + i, T('ifchain', i)) for i in range(3))
   + '  else if (c == 14) %s\n  else if (c == 30) use(30);\n' % B('ifchain'))
# separate goto labels at the join
mk('labels', '  switch (c) {\n  case 1: f0(); break;\n' + ''.join('  case %d: %s goto L%d;\n' % (10 + i, T('labels', i), i) for i in range(3))
   + '  case 14: %s goto L3;\n  case 30: use(30); break;\n  }\nL0: ;\nL1: ;\nL2: ;\nL3: ;\n' % B('labels'))
# tails inside a do/while(0) per case ending in break of the loop
mk('dowhile', '  switch (c) {\n  case 1: f0(); break;\n' + ''.join('  case %d: do { %s } while (0); break;\n' % (10 + i, T('dowhile', i)) for i in range(3))
   + '  case 14: %s break;\n  case 30: use(30); break;\n  }\n' % B('dowhile'))
# switch inside a for(;;) with break out after
mk('loop', '  for (;;) {\n  switch (c) {\n  case 1: f0(); break;\n' + ''.join('  case %d: %s break;\n' % (10 + i, T('loop', i)) for i in range(3))
   + '  case 14: %s break;\n  case 30: use(30); continue;\n  }\n  break;\n  }\n' % B('loop'))
run(V, show=sys.argv[1:])
