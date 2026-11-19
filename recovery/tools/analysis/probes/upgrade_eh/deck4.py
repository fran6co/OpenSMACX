import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import deck as D, deck2 as D2
S = D2.V['popp3_01']
F = S.index('int Console::upgrade(int veh_id) {'); E = S.index('\n}\n', F) + 3
def edit(f):
    return S[:F] + f(S[F:E]) + S[E:]
V = {'base': S}
TT = 'strcat(StringTemp, table_text(Ability[i].abbreviation));'
def tt_after(t, n):
    # replace the last n table_text sites
    idx = [m.start() for m in re.finditer(r'table_text\(Ability\[[ij]\]\.abbreviation\)', t)]
    for s in reversed(idx[len(idx) - n:]):
        e = t.index(')', t.index('abbreviation', s)) + 1
        inner = t[s + len('table_text('):e - 1]
        t = t[:s] + 'StringTable.get(' + inner + ')' + t[e:]
    return t
V['sget_last2'] = edit(lambda t: tt_after(t, 2))
V['sget_all4'] = edit(lambda t: tt_after(t, 4))
MOR8 = '''                    {
                        uint8_t morale = Vehs[veh_id].morale;
                        Vehs[veh_id].morale = static_cast<uint8_t>(range(morale - 1, 0, 6));
                    }
'''
MOR32 = MOR8.replace('uint8_t morale', 'uint32_t morale')
MORP = '                    Vehs[veh_id].morale = static_cast<uint8_t>(range(Vehs[veh_id].morale - 1, 0, 6));\n'
V['morale_plain'] = edit(lambda t: t.replace(MOR8, MORP).replace(MOR32, MORP))
V['triad_open'] = edit(lambda t: t.replace('get_proto_triad(choice)', 'Chassis[VehPrototypes[choice].chassis_id].triad'))
D.run(V, sys.argv[1:] or None)
