# Host choice vs shared-tail length. A = 3-arg tails sharing only `add esp,0xc` (distinct callees),
# P = f2 tails sharing `call f2; add esp,8`, R = tails sharing 4 instr (`push 2; push 1; call f2; add esp,8` after distinct stores).
import tm7, sys
tm7.HDR += 'extern void f2s(const char *, const char *); extern void f2t(const char *, const char *); extern int gt, gu;\nextern void f3b(int, int, int); extern void f3c(int, int, int);\n'
V = []
def add(k, A, **kw): V.append((k, tm7.fn('t_' + k, A, **kw)))
LAST = ['mk(99); if (g7) return; goto OUT;']
EPI = '  call();\nOUT:\n  use(777);\n'
P = lambda j: 'mk(%d); g3 = %d; if (cond()) { f2(%d, %d); }' % (j * 7 + 3, j, j, j + 100)
A = lambda j: 'mk(%d); g2 = %d; %s(%d, 2, 3);' % (j * 7 + 500, j, ('f3', 'f3b', 'f3c')[j % 3], j)
R = lambda j: 'mk(%d); g6 = %d; if (cond()) { g8 = %d; f2(1, 2); }' % (j * 7 + 800, j, j)
Q = lambda j: 'mk(%d); g4 = %d; use(%d); g5 = %d;' % (j * 7 + 900, j, j, j)
G = lambda j: 'mk(%d); g6 = %d; if (cond()) { g8 = %d; f2(g4, g5); }' % (j * 7 + 1200, j, j)
D2 = lambda j: 'mk(%d); g6 = %d; if (g4 == 3) { use(%d); f2(%d, 1); } else if (g5 == 3) { use(%d); f2(%d, 2); }' % (j * 7 + 1500, j, j, j, j + 9, j)
D3 = lambda j: 'mk(%d); if (g4 == 3) { if (cond()) f2(%d, 1); } else { if (cond()) f2(%d, 2); }' % (j * 7 + 1800, j, j)
B = lambda j: 'mk(%d); g2 = %d; if (cond()) { %s(%d, 2, 3); }' % (j * 7 + 2500, j, ('f3', 'f3b', 'f3c')[j % 3], j)
C = lambda j: 'mk(%d); g3 = %d; f2(%d, %d);' % (j * 7 + 3000, j, j, j + 100)
S3 = lambda j: 'mk(%d); if (g4 == 1) { use(11); } else if (g4 == 2) { use(12); } else { use(13); } %s(%d, 2, 3);' % (j * 7 + 3500, ('f3', 'f3b', 'f3c')[j % 3], j)
M3 = lambda j: 'mk(%d); if (g4 == 1) { use(11); %s(%d, 2, 3); } else if (g4 == 2) { use(12); %s(%d, 2, 3); } else { use(13); %s(%d, 2, 3); }' % ((j * 7 + 3700,) + (('f3', 'f3b', 'f3c')[j % 3], j) * 3)
W = lambda j: 'mk(%d); g3 = %d; if (cond()) { use(%d); f2s(gt ? "a%d" : "b%d", gu ? "c" : "d"); }' % (j * 7 + 4000, j, j, j, j)
X = lambda j: 'mk(%d); g3 = %d; if (g4 == 3) { use(%d); f2s(gt ? "a%d" : "b%d", gu ? "c" : "d"); } else if (g5 == 3) { use(%d); f2s(gt ? "e%d" : "f%d", gu ? "c" : "d"); }' % (j * 7 + 4300, j, j, j, j, j + 50, j, j)
SR = lambda j: 'mk(%d); if (g4 == 99) { use(98); return; } use(97); if (g4 == 1) { use(11); } else if (g4 == 2) { if (g6 < 0) return; do { if (ga[g6] == 5) break; g6 = cond(); } while (g6 >= 0); use(12); } else { use(13); } %s(%d, 2, 3);' % (j * 7 + 3900, ('f3', 'f3b', 'f3c')[j % 3], j)
Y = lambda j: 'mk(%d); g3 = %d; if (g4 == 3) { use(%d); f2s(gt ? "a%d" : "b%d", gu ? "c" : "d"); } else if (g5 == 3) { use(%d); f2s(gt ? "e%d" : "f%d", gu ? "c" : "d"); g9 = 1; }' % (j * 7 + 4600, j, j, j, j, j + 50, j, j)
Yd = lambda j: 'mk(%d); g3 = %d; if (g4 == 3) { use(%d); f2s(gt ? "a%d" : "b%d", gu ? "c" : "d"); } else if (g5 == 3) { use(%d); f2t(gt ? "e%d" : "f%d", gu ? "c" : "d"); }' % (j * 7 + 4900, j, j, j, j, j + 50, j, j)
Y1 = lambda j: 'mk(%d); g3 = %d; if (g4 == 3) { use(%d); f2s(gt ? "a%d" : "b%d", gu ? "c" : "d"); g9 = 2; } else if (g5 == 3) { use(%d); f2s(gt ? "e%d" : "f%d", gu ? "c" : "d"); g9 = 1; }' % (j * 7 + 5200, j, j, j, j, j + 50, j, j)
Yl = lambda j: Y(j) + ' LY%d: ;' % j
Yn = lambda j: 'mk(%d); g3 = %d; if (g4 == 3) { use(%d); f2s(gt ? "a%d" : "b%d", gu ? "c" : "d"); } else if (g5 == 3) { use(%d); f2s(gt ? "e%d" : "f%d", gu ? "c" : "d"); }' % (j * 7 + 5500, j, j, j, j, j + 50, j, j)
Ynl = lambda j: Yn(j) + ' LN%d: ;' % j
WS = lambda j: 'mk(%d); g3 = %d; if (g4 == 3) { use(%d); f2s(gt ? "a%d" : "b%d", gu ? "c" : "d"); } else { use(%d); g9 = 1; }' % (j * 7 + 5800, j, j, j, j, j + 50)
WE = lambda j: 'mk(%d); g3 = %d; if (g4 == 3) { use(%d); g9 = 1; } else { use(%d); f2s(gt ? "a%d" : "b%d", gu ? "c" : "d"); }' % (j * 7 + 6100, j, j, j + 50, j, j)
WN = lambda j: 'mk(%d); g3 = %d; if (g4 == 3) { if (g5 == 3) { use(%d); f2s(gt ? "a%d" : "b%d", gu ? "c" : "d"); } }' % (j * 7 + 6400, j, j, j, j)
pat = {'s': WS, 'e': WE, 'n': WN, 'L': Yl, 'N': Yn, 'K': Ynl, 'V': Yd, 'U': Y1, 'Y': Y, 'Z': SR, 'W': W, 'X': X, 'S': S3, 'M': M3, 'B': B, 'C': C, 'D': D2, 'E': D3, 'A': A, 'P': P, 'R': R, 'Q': Q, 'G': G}
for spec in ([x for x in sys.argv[1].split(',') if not x.startswith('D_')] if len(sys.argv) > 1 else ['AAPPP', 'PPAA', 'APAP', 'PPRR', 'RRPP', 'PRPR', 'AARR', 'AAPPPPPPP']):
    cnt = {}
    L = []
    for c in spec:
        cnt[c] = cnt.get(c, 0) + 1
        L.append(pat[c](cnt[c]))
    add(spec, L + LAST, epi=EPI)
    if 'eh' in sys.argv: add('D_' + spec, L + LAST, epi=EPI, pro='  D d;\n')
if __name__ == '__main__':
    res = tm7.run(V, show=sys.argv[2:])
    for (k, r), _ in zip(res, V):
        print(k, r)
