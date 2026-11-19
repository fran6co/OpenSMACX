#!/usr/bin/env python3
"""okc_groups.py: textual group order of on_key_click's final switch: H (case 0x61/0x23 with the
move_direction body), S (the seven direction stubs), P (the four pan cases); layout + fe rows."""
import sys, os, re, itertools
R = os.path.abspath(os.path.dirname(os.path.abspath(__file__)) + '/../../../../..')
sys.path.insert(0, R + '/recovery/tools'); import perm
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__))); import okc_layout as L
SRC = open(R + '/src/console.cpp', encoding='latin-1').read()
dlo = SRC.index('        case 0x61:\n        case 0x23:')
dhi = SRC.index('        case 0x21:\n        case 0x69:')
plo = SRC.index('        case 0x10026:\n        case 0x10068: {')
phi = SRC.index('        }\n    }\n\n    if (ConsoleGlobal.cursor_active_ != 0) {', plo)
G = {'H': SRC[dlo:dhi], 'S': SRC[dhi:plo], 'P': SRC[plo:phi]}
def build(order): return SRC[:dlo] + ''.join(G[g] for g in order) + SRC[phi:]
DECK = [(''.join(o), build(o)) for o in itertools.permutations('HSP')]
if __name__ == '__main__':
    os.environ.setdefault('FE_PFX_BASE', '14')
    d = R + '/build/recovery/fe/switchtree'; os.makedirs(d, exist_ok=True)
    print('img   ', ' '.join(L.marks(L.image())))
    for k, t in DECK:
        p = '%s/%s.cpp' % (d, k); open(p, 'w', encoding='latin-1').write(t)
        print('%-6s' % k, ' '.join(L.marks(L.ours(p))))
    for w, dd, ex, k in perm.run_deck('src/console.cpp', '0x005178C0', DECK, jobs=5): print(dd, k)
