#!/usr/bin/env python3
"""run.py [name ...]: how VC6 lowers a `switch` by case count and density (MATCHING_GUIDE 3.7, "Switch
lowering").  Each variant is one cdecl function `switch (a) { case v: use(v); break; ... } use(99); return 1;`
(suffix d = with `default: use(98)`); prints TABLE (`jmp [reg*4+T]`), BYTE (`mov dl,[reg+T]` index table +
jump table), the number of `cmp`/`sub`/`dec` tests of a compare chain or tree, and the test mnemonics in order.
SHOW=1 prints the listing.  FE_PFX_BASE selects the compiler prefix (default 21)."""
import sys, os, re
H = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(os.path.dirname(H)) + '/inline')
import ib
PFX = os.environ.get('FE_PFX_BASE', '21')
HDR = '#include "probe.h"\n'
SETS = {
 'seq2': [1, 2], 'seq3': [1, 2, 3], 'seq4': [1, 2, 3, 4], 'seq5': [1, 2, 3, 4, 5], 'seq6': [1, 2, 3, 4, 5, 6],
 'z3': [0, 1, 2], 'z4': [0, 1, 2, 3],
 'gap4_5': [1, 2, 4, 5], 'gap4_6': [1, 2, 4, 6], 'gap4_7': [1, 3, 5, 7], 'gap4_8': [1, 3, 5, 8], 'gap4_9': [1, 3, 6, 9],
 'gap4_10': [1, 4, 7, 10], 'gap4_13': [1, 5, 9, 13], 'gap4_20': [1, 7, 13, 20],
 'gap5_9': [1, 3, 5, 7, 9], 'gap5_13': [1, 4, 7, 10, 13], 'gap5_17': [1, 5, 9, 13, 17], 'gap5_30': [1, 8, 15, 22, 30],
 'gap6_16': [1, 4, 7, 10, 13, 16], 'gap6_21': [1, 5, 9, 13, 17, 21], 'gap6_40': [1, 9, 17, 25, 33, 40],
 'gap8_30': [1, 5, 9, 13, 17, 21, 25, 30], 'gap8_60': [1, 9, 17, 25, 33, 41, 49, 60], 'gap8_200': [1, 30, 60, 90, 120, 150, 180, 200],
 'two_runs': [1, 2, 3, 4, 101, 102, 103, 104], 'run_far': [1, 2, 3, 4, 500], 'run_far2': [1, 2, 3, 4, 500, 900],
 'sparse3': [1, 20, 300], 'sparse4': [1, 20, 300, 4000], 'sparse5': [1, 20, 300, 4000, 50000],
 'sparse7': [1, 20, 300, 4000, 50000, 600000, 7000000],
}
def body(vals, dflt, same=False):
    s = 'switch (a) { '
    for v in vals:
        s += 'case %d: use(%d); break; ' % (v, 7 if same else v)
    if dflt:
        s += 'default: use(98); break; '
    return s + '} use(99); return 1;'
V = {}
for k, vals in SETS.items():
    V[k] = body(vals, False)
    V[k + '_d'] = body(vals, True)
V['same6_30'] = 'switch (a) { case 1: case 7: case 13: case 19: case 25: case 30: use(7); break; default: use(98); break; } use(99); return 1;'
V['same12_60'] = 'switch (a) { ' + ' '.join('case %d:' % v for v in range(1, 61, 5)) + ' use(7); break; case 2: use(2); break; default: use(98); break; } use(99); return 1;'
def klass(L):
    t = [l for l in L if re.match(r'(cmp|sub|dec|test|add)\s', l) and 'esp' not in l]
    tb = any(re.match(r'jmp\s+DWORD PTR \$L\d+\[', l) for l in L)
    by = any(re.search(r'BYTE PTR \$L\d+\[', l) for l in L)
    return ('BYTE ' if by else 'TABLE' if tb else 'TREE '), len(t), ' '.join(re.sub(r'\s+', ' ', x) for x in t)
if __name__ == '__main__':
    sel = sys.argv[1:] or list(V)
    src = HDR + ''.join('int %s(int a, int b, int c) {\n%s\n}\n' % (n, V[n]) for n in sel)
    asm, out = ib.compile_asm(None, PFX, text=src)
    F = {f[1]: f[2] for f in ib.functions(asm)}
    for n in sel:
        L = next((l for k, l in F.items() if ('?%s@@' % n) in k), None)
        if L is None:
            print(n, 'MISSING', out[-300:] if out else ''); continue
        L = [l for l in L if not l.startswith(';')]
        k, nt, tests = klass(L)
        print('%-12s %s tests=%d  %s' % (n, k, nt, tests[:150]))
        if os.environ.get('SHOW'):
            print('   ' + ' ; '.join(L))
