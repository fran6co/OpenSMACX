"""deck5.py: reservation probes (free outer inline around a charged inner inline) after return 9,
on the committed source (popp3_01). Diagnostic only."""
import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import deck as D
S = D.SRC; F = D.FN
PROBE = '''static inline int upg_in(int x) {
    if (x == 1) x = 3;
    if (x == 2) x = 5;
    if (x == 3) x = 9;
    if (x == 4) x = 11;
    return x;
}
static inline int upg_out(int x) { return upg_in(x); }
static inline int upg_in1(int x) {
    if (x == 1) x = 3;
    return x;
}
static inline int upg_out1(int x) { return upg_in1(x); }
'''
A = {'pre_cost': '    int cost = upgrade_cost(faction_id, choice, Vehs[veh_id].proto_id) * 10;\n',
     'pre_ret1': '        // A declined confirmation still unlocks and reports success.\n'}
V = {'base': S}
NS = [int(x) for x in os.environ.get('NS', '1,2,4,8').split(',')]
for pos, anchor in A.items():
    for fn in ('upg_out', 'upg_out1'):
        for n in NS:
            V['%s_%s_%d' % (pos, fn, n)] = S[:F] + PROBE + S[F:].replace(anchor, ''.join('    %s(%d);\n' % (fn, 1000 + k) for k in range(n)) + anchor, 1)
D.run(V, sys.argv[1:] or None)
