V.clear()
PP = ''.join(PAN[p] for p in ('26','28','25','27'))
def wrap(name, body): return PRE % name + body + J + '}\n'
HEAD = '''            if (mode != 0 && (raw & 0x10000) != 0) {
                if (a2 == 5) mk(11);
                else if (a2 == 1) mk(12);
                return 1;
            }
            goto rest;
'''
REST = MD[MD.index('            mk(10);'):]
H = '        case 0x61:\n        case 0x23:\n            a2 = 4;\n        md:\n' + HEAD
V['split'] = wrap('v_split', '    switch (key) {\n' + H + stubs() + PP + '        rest:\n' + REST + '    }\n')
V['split_after'] = PRE % 'v_split_after' + '    switch (key) {\n' + H + stubs() + PP + '    }\n' + J + 'rest:\n' + REST + '}\n'
