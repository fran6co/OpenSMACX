#!/usr/bin/env python3
"""okc_dup.py: direction cases of on_key_click's final switch written out (each case its own copy
of the move_direction body) vs the goto form; layout landmarks + fe rows."""
import sys, os, re, subprocess
R = os.path.abspath(os.path.dirname(os.path.abspath(__file__)) + '/../../../../..')
sys.path.insert(0, R + '/recovery/tools'); import perm
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__))); import okc_layout as L
SRC = open(R + '/src/console.cpp', encoding='latin-1').read()
dlo = SRC.index('        case 0x61:\n        case 0x23:')
plo = SRC.index('        case 0x10026:\n        case 0x10068: {')
region = SRC[dlo:plo]
body = region[region.index('        move_direction:\n') + len('        move_direction:\n'):region.index('        case 0x21:\n')]
stubs = re.findall(r'        case (0x\w+):\n        case (0x\w+):\n            a2 = (\d);\n            goto move_direction;\n', region)
cases = [('0x61', '0x23', '4')] + stubs
def build(order, label=False):
    out = ''
    for i, (a, b, k) in enumerate(order):
        out += '        case %s:\n        case %s:\n            a2 = %s;\n' % (a, b, k) + body
    return SRC[:dlo] + out + SRC[plo:]
DECK = [('61first', build(cases)), ('61last', build(cases[1:] + cases[:1]))]
if __name__ == '__main__':
    os.environ.setdefault('FE_PFX_BASE', '14')
    d = R + '/build/recovery/fe/switchtree'; os.makedirs(d, exist_ok=True)
    for k, t in DECK:
        p = '%s/%s.cpp' % (d, k); open(p, 'w', encoding='latin-1').write(t)
        print(k, ' '.join(L.marks(L.ours(p))))
    for w, dd, ex, k in perm.run_deck('src/console.cpp', '0x005178C0', DECK, jobs=2): print(dd, k)
