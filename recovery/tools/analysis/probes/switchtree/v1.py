# MD after the switch, J before it; direction cases goto md
def _post_after(name, mdfirst):
    G_S = stubs().replace('goto md', 'goto md')
    H = '        case 0x61:\n        case 0x23:\n            a2 = 4;\n            goto md;\n'
    s = PRE % name + '    switch (key) {\n' + H + G_S + ''.join(PAN[p] for p in ('26','28','25','27')) + '    }\n'
    if mdfirst:
        return s + '    goto join;\nmd:\n' + MD + 'join:\n' + J + '}\n'
    return s + J + 'md:\n' + MD + '}\n'
V['md_after_J'] = _post_after('v_md_after_J', False)
V['md_before_J'] = _post_after('v_md_before_J', True)
# 0x61 case textually last with MD falling out of the switch into... (MD returns)
V['H_last_inner'] = fn('v_H_last_inner', order=('S', 'P', 'H'))
# default: break explicit
V['default'] = fn('v_default', post=None).replace('    }\n    mk(30);', '        default: break;\n    }\n    mk(30);')
# pans first, then directions
V['PSH'] = fn('v_PSH', order=('P', 'S', 'H'))
