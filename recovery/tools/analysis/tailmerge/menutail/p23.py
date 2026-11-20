"""Cross-jump blocker screen 4 (par-6 r53): the join's predecessor count.  N extra cases each calling a
distinct no-argument callee (no common suffix, so none cross-jumps away and every one stays a `jmp` to
the join).  psave-calls 1 = merged, 4 = kept."""
import sys; from cj import *
import cj
from p1 import fn
NS = [int(a) for a in sys.argv[1:] if a.isdigit()] or [10, 30, 50, 60, 64, 70, 100, 128, 150]
cj.HDR += ''.join('extern void x%d();\n' % j for j in range(max(NS) + 1))
V = []
for n in NS:
    ex = ''.join('  case %d: x%d(); break;\n' % (1000 + j, j) for j in range(n))
    V.append(('distinct%d' % n, fn('dn%d' % n, 1, 1).replace('  case 1: f0(); break;\n', '  case 1: f0(); break;\n' + ex)))
run(V, show=[a for a in sys.argv[1:] if not a.isdigit()])
if True:  # the same cases after the toggles
    V = []
    for n in NS:
        ex = ''.join('  case %d: x%d(); break;\n' % (1000 + j, j) for j in range(n))
        V.append(('after%d' % n, fn('da%d' % n, 1, 1, extra=ex)))
    run(V)
