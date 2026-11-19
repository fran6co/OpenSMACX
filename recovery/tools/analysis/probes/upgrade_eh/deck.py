#!/usr/bin/env python3
"""deck.py: Console::upgrade 0x004D06C0 teardown depths per return site (ehsites runs) and fe rows for
helper/scope variants. Image runs: [1, 5,5,5,5, 5,5,5,5,5, 7,13, 7,18, 7,18]."""
import sys, os, re, subprocess
from concurrent.futures import ThreadPoolExecutor
R = os.path.abspath(os.path.dirname(os.path.abspath(__file__)) + '/../../../../..')
T = R + '/recovery/tools'
U = 'src/console_commands.cpp'
SRC = open(R + '/' + U, encoding='latin-1').read()
FN = SRC.index('int Console::upgrade(int veh_id) {')
def sub(t, a, b, n=1):
    assert t.count(a) >= n, (a[:60], t.count(a))
    return t.replace(a, b)
def block(var, indent):
    i = ' ' * indent
    return (i + "StringTemp[0] = '\\0';\n" + i + "strcat(StringTemp, VehPrototypes[%s].veh_name);\n" % var +
            i + "strcat(StringTemp, BufferJoinSpace);\n" + i + "strcat(StringTemp, SocialWinOpenParenText);\n" +
            i + "say_stats_2(StringTemp, %s);\n" % var + i + "if (VehPrototypes[%s].ability_flags) {\n" % var +
            i + "    strcat(StringTemp, BufferJoinSpace);\n")
def helper_variants():
    V = {'base': SRC}
    H = '''static inline void say_design(int proto_id) {
    StringTemp[0] = '\\0';
    strcat(StringTemp, VehPrototypes[proto_id].veh_name);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, SocialWinOpenParenText);
    say_stats_2(StringTemp, proto_id);
    if (VehPrototypes[proto_id].ability_flags) {
        strcat(StringTemp, BufferJoinSpace);
        for (int i = 0; i < MaxAbilityNum; i++) {
            if (has_abil(proto_id, 1 << i)) {
                strcat(StringTemp, table_text(Ability[i].abbreviation));
            }
        }
    }
    strcat(StringTemp, SocialWinCloseParenText);
}
'''
    body = SRC[FN:]
    end = body.index('\n}\n') + 3
    fb = body[:end]
    # the four written-out copies
    import itertools
    pat = re.compile(r"( +)StringTemp\[0\] = '\\0';\n\1strcat\(StringTemp, VehPrototypes\[(.+?)\]\.veh_name\);\n.*?\1strcat\(StringTemp, SocialWinCloseParenText\);\n", re.S)
    sites = list(pat.finditer(fb))
    def make(which):
        out = fb; off = 0
        for k, m in enumerate(sites):
            if k not in which: continue
            rep = '%ssay_design(%s);\n' % (m.group(1), m.group(2))
            s, e = m.start() + off, m.end() + off
            out = out[:s] + rep + out[e:]; off += len(rep) - (m.end() - m.start())
        return SRC[:FN] + H + out + SRC[FN + end:]
    names = ['top', 'loop', 'old', 'new']
    for which in [(2, 3), (0, 2, 3), (0, 1, 2, 3), (1, 2, 3), (0,), (2,), (3,)]:
        V['helper_' + '+'.join(names[w] for w in which)] = make(which)
    return V, len(sites)
def run(V, keys=None, base=14):
    d = R + '/build/recovery/fe/upgrade_eh'; os.makedirs(d, exist_ok=True)
    keys = keys or list(V)
    def one(k):
        i = keys.index(k); lane = base + i % 5
        p = '%s/%s.cpp' % (d, k); open(p, 'w', encoding='latin-1').write(V[k])
        eh = subprocess.run(['uv', 'run', 'python', T + '/analysis/ehsites.py', U, '0x004D06C0', '--src', p, '--pfx', str(lane)], cwd=R, capture_output=True, text=True).stdout
        fe = subprocess.run(['uv', 'run', 'python', T + '/fe.py', U, '0x004D06C0', '--src', p, '--pfx', str(lane), '--quiet'], cwd=R, capture_output=True, text=True).stdout
        r = re.search(r'OURS runs (\[.*\])', eh); m = re.search(r'diff=(\d+)', fe)
        return k, r.group(1) if r else eh[-300:], m.group(1) if m else fe[-200:]
    import threading
    locks = [threading.Lock() for _ in range(5)]
    def locked(k):
        with locks[keys.index(k) % 5]: return one(k)
    with ThreadPoolExecutor(5) as ex:
        for k, r, rows in ex.map(locked, keys): print('%-28s %-5s %s' % (k, rows, r))
if __name__ == '__main__':
    V, n = helper_variants(); print('copies found', n)
    run(V, sys.argv[1:] or None)
