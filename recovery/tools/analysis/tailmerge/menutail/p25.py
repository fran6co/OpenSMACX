"""Cross-jump blocker screen 6 (par-6 r53): the join laid out in the middle of the switch (after an early
case whose `if` falls into it, as menu_proc's case 0x106) so the toggle tails jump BACKWARD.
psave-calls 1 = merged, 4 = kept.  Prints whether the join label precedes the toggles."""
import sys; from cj import *
import cj
from p1 import fn
cj.HDR += 'extern void big(int, int, int);\n'
V = []
EARLY = {
    'if_call': '  case 5: if (gx & 0x200) big(1, 2, 3); break;\n',
    'if_loop': '  case 5: if (gx & 0x200) { for (int i = 0; i < gx; i++) big(i, gx, 3); } break;\n',
    'if_call_ret': '  case 5: if (gx & 0x200) { big(1, 2, 3); c = gmp; } break;\n',
    'none': '',
}
for k, e in EARLY.items():
    V.append((k, fn('j' + k, 1, 1).replace('  case 1: f0(); break;\n', e + '  case 1: f0(); break;\n')))
run(V, show=sys.argv[1:])
