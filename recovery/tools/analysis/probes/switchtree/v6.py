V.clear()
PP = ''.join(PAN[p] for p in ('26','28','25','27'))
def wrap(name, body): return PRE % name + body + J + '}\n'
H = '        case 0x61:\n        case 0x23:\n            a2 = 4;\n        md:\n' + MD
S_ret = ''.join('        case %s:\n        case %s:\n            mk(%d);\n            return a1;\n' % (a, b, 50 + k) for a, b, k in STUBS)
S_one = S_ret.replace('            mk(57);\n            return a1;\n', '            a2 = 7;\n            goto md;\n')
V['only61'] = wrap('v_only61', '    switch (key) {\n' + H + S_ret + PP + '    }\n')
V['only61_last'] = wrap('v_only61_last', '    switch (key) {\n' + S_ret + PP + H + '    }\n')
V['one_stub'] = wrap('v_one_stub', '    switch (key) {\n' + H + S_one + PP + '    }\n')
V['a1dep'] = wrap('v_a1dep', '    switch (key) {\n' + H.replace('a2 = 4;', 'a2 = a1;') + S_ret + PP + '    }\n')
