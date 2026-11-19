import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import deck as D, deck2 as D2
S = D2.V['popp3_01']
F = S.index('int Console::upgrade(int veh_id) {')
PROBE = 'static inline int upg_probe(int x) {\n    if (x == 0) x = 7;\n    return x;\n}\n'
POS = {
    'pre_cost': '    int cost = upgrade_cost(faction_id, choice, Vehs[veh_id].proto_id) * 10;\n',
    'pre_old': "    StringTemp[0] = '\\0';\n    strcat(StringTemp, VehPrototypes[Vehs[veh_id].proto_id].veh_name);\n    strcat(StringTemp, BufferJoinSpace);\n    strcat(StringTemp, SocialWinOpenParenText);\n    say_stats_2(StringTemp, Vehs[veh_id].proto_id);\n    if (VehPrototypes[Vehs[veh_id].proto_id].ability_flags) {\n        strcat(StringTemp, BufferJoinSpace);\n        for (int i = 0; i < MaxAbilityNum; i++) {\n            if (has_abil(Vehs[veh_id].proto_id, 1 << i)) {\n                strcat(StringTemp, table_text(Ability[i].abbreviation));\n            }\n        }\n    }\n    strcat(StringTemp, SocialWinCloseParenText);\n    parse_says(0, StringTemp, -1, -1);\n",
    'pre_ret1': '        // A declined confirmation still unlocks and reports success.\n',
}
V = {'base': S}
for pos, anchor in POS.items():
    assert S[F:].count(anchor) == 1, pos
    for n in (1, 2, 3, 4, 6):
        t = S[:F] + PROBE + S[F:].replace(anchor, ''.join('    upg_probe(%d);\n' % (1000 + k) for k in range(n)) + anchor)
        V['%s_%d' % (pos, n)] = t
D.run(V, sys.argv[1:] or None)
