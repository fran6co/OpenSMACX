#!/usr/bin/env python3
"""run.py [name ...]: which copy of an identical tail hosts a VC6 cross-jump (MATCHING_GUIDE 3.7,
"Cross-jump host"). Each variant writes the same tail T in several arms; the arm markers use(K) show which
arm keeps T (the host: `uK ; <T>`) and which arms end in `jmp` into it. Reuses crossjump/run.py's
listing compaction. FE_PFX_BASE selects the compiler prefix."""
import sys, os
H = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(H) + '/crossjump')
sys.path.insert(0, os.path.dirname(os.path.dirname(H)) + '/inline')
import ib
import run as cj
PFX = os.environ.get('FE_PFX_BASE', '21')
T = 'f1(7); f1(8);'
V = {
 # baseline: three return arms in an if-chain (known: last copy hosts)
 'ret3':      'if (a == 1) { use(1); %s return 1; } if (a == 2) { use(2); %s return 1; } if (a == 3) { use(3); %s return 1; } use(9); return 0;' % (T, T, T),
 # the same three arms joining a common successor instead of returning
 'join3':     'if (a == 1) { use(1); %s } else if (a == 2) { use(2); %s } else if (a == 3) { use(3); %s } use(9); return 0;' % (T, T, T),
 # the last copy is the function's fall-through end (no return statement of its own)
 'fallend':   'if (a == 1) { use(1); %s return 1; } if (a == 2) { use(2); %s return 1; } use(3); %s return 1;' % (T, T, T),
 # suffix length: arm 2 shares a longer suffix with arm 1 (same extra statement) than arm 3 does
 # (f1(6) changes the pending pop, so these only show that unequal pops never merge; see sfx*)
 'suffix12':  'if (a == 1) { use(1); f1(6); %s return 1; } if (a == 2) { use(2); f1(6); %s return 1; } if (a == 3) { use(3); %s return 1; } use(9); return 0;' % (T, T, T),
 'suffix13':  'if (a == 1) { use(1); f1(6); %s return 1; } if (a == 2) { use(2); %s return 1; } if (a == 3) { use(3); f1(6); %s return 1; } use(9); return 0;' % (T, T, T),
 'suffix23':  'if (a == 1) { use(1); %s return 1; } if (a == 2) { use(2); f1(6); %s return 1; } if (a == 3) { use(3); f1(6); %s return 1; } use(9); return 0;' % (T, T, T),
 # suffix length with equal pops: g2 = 6 before T in two of the three arms
 'sfx12':     'if (a == 1) { use(1); g2 = 6; %s return 1; } if (a == 2) { use(2); g2 = 6; %s return 1; } if (a == 3) { use(3); %s return 1; } use(9); return 0;' % (T, T, T),
 'sfx13':     'if (a == 1) { use(1); g2 = 6; %s return 1; } if (a == 2) { use(2); %s return 1; } if (a == 3) { use(3); g2 = 6; %s return 1; } use(9); return 0;' % (T, T, T),
 'sfx23':     'if (a == 1) { use(1); %s return 1; } if (a == 2) { use(2); g2 = 6; %s return 1; } if (a == 3) { use(3); g2 = 6; %s return 1; } use(9); return 0;' % (T, T, T),
 'sfx_only1': 'if (a == 1) { use(1); g2 = 6; %s return 1; } if (a == 2) { use(2); %s return 1; } if (a == 3) { use(3); %s return 1; } use(9); return 0;' % (T, T, T),
 'sfx_only3': 'if (a == 1) { use(1); %s return 1; } if (a == 2) { use(2); %s return 1; } if (a == 3) { use(3); g2 = 6; %s return 1; } use(9); return 0;' % (T, T, T),
 # layout order differs from source order: arm 1's block placed after the function's return (goto)
 'goto_mid':  'if (a == 2) goto two; if (a == 1) { use(1); %s return 1; } if (a == 3) { use(3); %s return 1; } use(9); return 0; two: use(2); %s return 1;' % (T, T, T),
 'goto_early': 'if (a == 3) { use(3); %s return 1; } if (a == 1) goto one; if (a == 2) { use(2); %s return 1; } use(9); return 0; one: use(1); %s return 1;' % (T, T, T),
 'goto_late': 'if (a == 1) goto one; if (a == 2) { use(2); %s return 1; } if (a == 3) { use(3); %s return 1; } use(9); return 0; one: use(1); %s return 1;' % (T, T, T),
 # nested: arm 1 inside a loop (continue), arms 2/3 return
 'loop':      'int i; for (i = 0; i < b; i++) { if (ga[i] == a) { use(1); %s return 1; } } if (a == 2) { use(2); %s return 1; } use(3); %s return 1;' % (T, T, T),
 # switch cases (layout follows case order)
 'switch3':   'switch (a) { case 1: use(1); %s return 1; case 2: use(2); %s return 1; case 3: use(3); %s return 1; } use(9); return 0;' % (T, T, T),
 'switch3r':  'switch (a) { case 3: use(3); %s return 1; case 2: use(2); %s return 1; case 1: use(1); %s return 1; } use(9); return 0;' % (T, T, T),
 # switch with unsorted values: host follows case value, not source order?
 'sw_529':    'switch (a) { case 5: use(5); %s return 1; case 2: use(2); %s return 1; case 9: use(9); %s return 1; } use(1); return 0;' % (T, T, T),
 'sw_952':    'switch (a) { case 9: use(9); %s return 1; case 5: use(5); %s return 1; case 2: use(2); %s return 1; } use(1); return 0;' % (T, T, T),
 'sw_dense':  'switch (a) { case 4: use(4); %s return 1; case 1: use(1); %s return 1; case 3: use(3); %s return 1; case 2: use(2); return 2; case 5: use(6); return 3; } use(7); return 0;' % (T, T, T),
 # else arm written first in source, tested second
 'ifelse':    'if (a) { use(1); %s } else { use(2); %s } use(9); return 0;' % (T, T),
 'ifelse_n':  'if (!a) { use(2); %s } else { use(1); %s } use(9); return 0;' % (T, T),
}
if __name__ == '__main__':
    sel = sys.argv[1:] or list(V)
    src = cj.HDR + 'extern int ga[]; extern int g2;\n' + ''.join('int %s(int a, int b, int c) {\n%s\n}\n' % (n, V[n]) for n in sel)
    asm, out = ib.compile_asm(None, PFX, text=src)
    F = {f[1]: f[2] for f in ib.functions(asm)}
    for n in sel:
        L = next((l for k, l in F.items() if ('?%s@@' % n) in k), None)
        if L is None:
            print(n, 'MISSING', out[-400:] if out else ''); continue
        c = cj.short(cj.compact(L))
        print('== %-10s' % n)
        print('   ' + ' ; '.join(c))
