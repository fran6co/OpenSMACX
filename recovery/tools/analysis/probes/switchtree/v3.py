V.clear()
V['base'] = fn('v_base')
H = '        case 0x61:\n        case 0x23:\n            a2 = 4;\n        md:\n' + MD
PP = ''.join(PAN[p] for p in ('26','28','25','27'))
def wrap(name, body): return PRE % name + body + J + '}\n'
V['nested_def'] = wrap('v_nested_def', '    switch (key) {\n' + H + stubs() + '        default:\n            switch (key) {\n' + PP + '            }\n    }\n')
V['nested_pans_first'] = wrap('v_nested_pans_first', '    switch (key) {\n' + PP + '        default:\n            switch (key) {\n' + H + stubs() + '            }\n    }\n')
V['split_if'] = wrap('v_split_if', '    if (key >= 0x10000) {\n    switch (key) {\n' + PP + '    }\n    } else {\n    switch (key) {\n' + H + stubs() + '    }\n    }\n')
V['split_if_dirs_first'] = wrap('v_split_if_dirs_first', '    if (key < 0x10000) {\n    switch (key) {\n' + H + stubs() + '    }\n    } else {\n    switch (key) {\n' + PP + '    }\n    }\n')
V['two_switches'] = wrap('v_two_switches', '    switch (key) {\n' + H + stubs() + '    }\n    switch (key) {\n' + PP + '    }\n')
V['two_switches_pf'] = wrap('v_two_switches_pf', '    switch (key) {\n' + PP + '    }\n    switch (key) {\n' + H + stubs() + '    }\n')
