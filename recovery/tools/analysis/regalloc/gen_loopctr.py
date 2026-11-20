#!/usr/bin/env python3
r"""Bisect a call-free loop body (TutWin::tour dir loop, MATCHING_GUIDE 2.7 G1): when does the counter leave
the scratch registers?  python3 gen_loopctr.py OUT.spec && python3 cand.py OUT.spec --grep '^add\s+e.., 4$'"""
import sys, itertools
HDR = r'''
struct Mp { unsigned char climate; unsigned char a, b, c; unsigned char d; unsigned char val3; unsigned char e, f; int bit; int rest[8]; };
extern Mp *gMT; extern int gML, gMB, gMLB, gMF; extern int DX[8], DY[8];
extern int s1, s2, s3, s4, s5, s6, gfac;
inline int wrapx(int x) { return (gMF & 1) ? x : x < 0 ? x + gMB : x >= gMB ? x - gMB : x; }
inline Mp *mloc(int x, int y) { return &gMT[(x >> 1) + y * gML]; }
'''
PARTS = {
    'W': ('int xa = wrapx(x + DX[dir]);', 'int xa = x + DX[dir];'),
    'B': ('if (ya < 0 || ya >= gMLB || xa < 0 || xa >= gMB) continue;', 'if (ya < 0) continue;'),
    'M': ('int alt = mloc(xa, ya)->climate & ~0x1F;', 'int alt = (xa + ya) & ~0x1F;'),
    'T': ('if (alt > (tile->climate & ~0x1F)) { DT1 }', 'if (alt > 7) { DT1 }'),
    'D': ('if (!dir || dir >= 6) hi = 1;', 'hi = 1;'),
    'I': ('if (idx >= 2 && idx <= 4) lo = 1;', 'lo = 1;'),
}
P = []
keys = list(PARTS)
for bits in itertools.product((0, 1), repeat=len(keys)):
    on = {k: b for k, b in zip(keys, bits)}
    sel = {k: PARTS[k][0 if on[k] else 1] for k in keys}
    t = sel['T'].replace('DT1', sel['D'] + ' ' + sel['I'])
    body = '\n        '.join([sel['W'], 'int ya = DY[dir] + y;', sel['B'], sel['M'], t])
    name = ''.join(k if on[k] else '-' for k in keys)
    P.append('''== %s
%s
void probe(int x, int y, int idx) {
    call();
    Mp *tile = mloc(x, y);
    int hi = 0, lo = 0;
    for (int dir = 0; dir < 8; dir++) {
        %s
    }
    s1 = hi; s2 = lo; s3 = tile->val3;
    call();
}''' % (name, HDR, body))
open(sys.argv[1], 'w').write('\n'.join(P) + '\n')
