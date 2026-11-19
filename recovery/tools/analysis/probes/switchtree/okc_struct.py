#!/usr/bin/env python3
"""okc_struct.py: control-structure variants of on_key_click's final switch in the real unit;
prints the landmark layout (okc_layout) and fe rows per variant."""
import sys, os, re
R = os.path.abspath(os.path.dirname(os.path.abspath(__file__)) + '/../../../../..')
sys.path.insert(0, R + '/recovery/tools'); import perm
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__))); import okc_layout as L
SRC = open(R + '/src/console.cpp', encoding='latin-1').read()
dlo = SRC.index('        case 0x61:\n        case 0x23:')
dhi = SRC.index('        case 0x21:\n        case 0x69:')
plo = SRC.index('        case 0x10026:\n        case 0x10068: {')
phi = SRC.index('        }\n    }\n\n    if (ConsoleGlobal.cursor_active_ != 0) {', plo)
jlo = phi + len('        }\n    }\n\n')
jhi = SRC.index('close_messages:', jlo)
H, S, P, J = SRC[dlo:dhi], SRC[dhi:plo], SRC[plo:phi], SRC[jlo:jhi]
MD = H[H.index('        move_direction:\n') + len('        move_direction:\n'):]
def dedent(t, n=4): return ''.join(l[n:] if l.startswith(' ' * n) else l for l in t.splitlines(True))
V = {}
# (1) directions break into MD after the switch; pans/default goto the join
s_brk = re.sub(r'goto move_direction;', 'break;', S)
P_goto = P.replace('            break;\n        }', '            goto keys_done;\n        }')
V['md_join'] = SRC[:dlo] + '        case 0x61:\n        case 0x23:\n            a2 = 4;\n            break;\n' + s_brk + P_goto + \
    '        default:\n            goto keys_done;\n        }\n' + dedent(MD) + '    }\n\nkeys_done:\n' + J + SRC[jhi:]
H_goto = '        case 0x61:\n        case 0x23:\n            a2 = 4;\n            goto move_direction;\n'
H_brk = H_goto.replace('goto move_direction;', 'break;')
MDL = '        move_direction:\n' + MD
# (2) 0x61 a goto stub written after the other stubs; the label body after the pans, inside the switch
V['SHgP_md'] = SRC[:dlo] + S + H_goto + P + MDL + SRC[phi:]
V['HgSP_md'] = SRC[:dlo] + H_goto + S + P + MDL + SRC[phi:]
# (3) same, the label body first in the switch
V['MD_SHgP'] = SRC[:dlo] + MDL.replace('        move_direction:\n', '        move_direction:\n', 1) + S + H_goto + P + SRC[phi:]
# (4) break form with 0x61 last of the directions
V['SHbP_join'] = SRC[:dlo] + s_brk + H_brk + P_goto + '        default:\n            goto keys_done;\n        }\n' + dedent(MD) + '    }\n\nkeys_done:\n' + J + SRC[jhi:]
if __name__ == '__main__':
    os.environ.setdefault('FE_PFX_BASE', '14')
    d = R + '/build/recovery/fe/switchtree'; os.makedirs(d, exist_ok=True)
    print('img        ', ' '.join(L.marks(L.image())))
    keys = sys.argv[1:] or list(V)
    for k in keys:
        p = '%s/%s.cpp' % (d, k); open(p, 'w', encoding='latin-1').write(V[k])
        print('%-11s' % k, ' '.join(L.marks(L.ours(p))))
    for w, dd, ex, k in perm.run_deck('src/console.cpp', '0x005178C0', [(k, V[k]) for k in keys], jobs=5): print(dd, k)
