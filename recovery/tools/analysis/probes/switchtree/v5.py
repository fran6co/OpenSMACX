V.clear()
PP = ''.join(PAN[p] for p in ('26','28','25','27'))
def wrap(name, body): return PRE % name + body + J + '}\n'
Hg = '        case 0x61:\n        case 0x23:\n            a2 = 4;\n            goto md;\n'
V['SHgP_md'] = wrap('v_SHgP_md', '    switch (key) {\n' + stubs() + Hg + PP + '        md:\n' + MD + '    }\n')
V['HgSP_md'] = wrap('v_HgSP_md', '    switch (key) {\n' + Hg + stubs() + PP + '        md:\n' + MD + '    }\n')
V['SHgP_mdout'] = PRE % 'v_SHgP_mdout' + '    switch (key) {\n' + stubs() + Hg + PP + '    }\n' + J + 'md:\n' + MD + '}\n'
