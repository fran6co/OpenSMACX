#!/usr/bin/env python3
r"""What splits a constant-zero web (MATCHING_GUIDE 2.7 Z10): two groups of zero stores separated by a gap.
    python3 gen_zsplit.py OUT.spec && python3 cand.py OUT.spec --grep '^xor\s+(e..), \1|^mov\s+DWORD PTR \?g[0-7]@@3HA, '"""
import sys
HDR = 'extern int g0, g1, g2, g3, g4, g5, g6, g7, gm, garr[64];\n'
GAPS = {
    'call1': 'call();',
    'call3': 'call(); call(); call();',
    'if3': 'if (cond()) call(); if (cond()) call(); if (cond()) call();',
    'loopc': 'for (int i = 0; i < gm; i++) call();',
    'loopnc': 'for (int i = 1; i < gm; i++) garr[i] = i;',
    'pq': 'D(p); D(q); U(p); U(q); U(p); U(q); U(p); U(q);',
    'pqr': 'D(p); D(q); D(r); U(p); U(q); U(r); U(p); U(q); U(r); U(p); U(q); U(r);',
    'pqr_loop': 'D(p); D(q); D(r); for (int i = 1; i < gm; i++) { U(p); U(q); U(r); }',
    'pqr_weak': 'D(p); D(q); D(r); U(p); U(q); U(r);',
    'pqr_w2': 'D(p); D(q); D(r); call(); U(p); U(q); U(r);',
    'pqrs': 'D(p); D(q); D(r); D(s); U(p); U(q); U(r); U(s); U(p); U(q); U(r); U(s);',
    'pqr_z1': 'D(p); D(q); D(r); U(p); U(q); U(r); g2 = 0; U(p); U(q); U(r);',
    'pq_then_r': 'D(p); D(q); U(p); U(q); U(p); U(q); D(r); U(r); U(r); U(r);',
    'ret': 'if (cond()) return;',
}
FIRST = {'f1': 'g0 = 0;', 'f2': 'g0 = 0; call(); g1 = 0;', 'f4': 'g0 = 0; call(); g1 = 0; call(); g2 = 0; call(); g3 = 0;'}
LAST = {'l1': 'g4 = 0;', 'l2': 'g4 = 0; call(); g5 = 0;', 'l4': 'g4 = 0; call(); g5 = 0; call(); g6 = 0; call(); g7 = 0;'}
P = []
for fk, f in FIRST.items():
    for lk, l in LAST.items():
        for gk, g in GAPS.items():
            P.append('== %s_%s_%s\n%svoid probe() {\n    call();\n    %s\n    %s\n    %s\n    call();\n}' % (fk, gk, lk, HDR, f, g, l))
open(sys.argv[1], 'w').write('\n'.join(P) + '\n')
