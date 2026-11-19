V.clear()
V['base'] = fn('v_base')
PP = ''.join(PAN[p] for p in ('26','28','25','27'))
def wrap(name, body): return PRE % name + body + J + '}\n'
H_nolabel = '        case 0x61:\n        case 0x23:\n            a2 = 4;\n' + MD
S_ret = ''.join('        case %s:\n        case %s:\n            mk(%d);\n            return 1;\n' % (a, b, 50 + k) for a, b, k in STUBS)
V['nolabel'] = wrap('v_nolabel', '    switch (key) {\n' + H_nolabel + S_ret + PP + '    }\n')
V['nolabel_last'] = wrap('v_nolabel_last', '    switch (key) {\n' + S_ret + PP + H_nolabel + '    }\n')
S_brk = ''.join('        case %s:\n        case %s:\n            mk(%d);\n            break;\n' % (a, b, 50 + k) for a, b, k in STUBS)
V['nolabel_brk'] = wrap('v_nolabel_brk', '    switch (key) {\n' + H_nolabel + S_brk + PP + '    }\n')
