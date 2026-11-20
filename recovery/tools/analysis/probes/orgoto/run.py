#!/usr/bin/env python3
"""run.py [name ...]: where VC6 lays a return-ending goto label block among a function's out-of-line exits and
which return copy hosts the shared teardown tail (MATCHING_GUIDE 3.7, "An `||` goto site moves its label block
last"; tech_trade 0x00541100's skeleton).  Prints the block skeleton (uK = use(K), RET).  EH=1 adds an EH object
with an inline destructor; ST=1 then prints the state stores in layout order (S<n>, textual numbering).
SHOW=1 prints the listing.  FE_PFX_BASE selects the prefix.  rl = the label block (u4)."""
import sys, os, re
T = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, T+'/probes/blockorder'); sys.path.insert(0, T+'/inline')
import ib
import run as bo
PFX = os.environ.get('FE_PFX_BASE', '21')
HDR = bo.HDR + '#define U(K) use(K); g9 = K\n'
EH = os.environ.get('EH')
if EH:
    HDR += 'struct Q { Q(); ~Q(); int x; }; struct P { Q q1, q2; P(); ~P() { mk(99); } int x[4]; int exec(); };\n'
TOP = 'again: U(1); if (g) { if (b) return 0; U(2); return 0; } if (g2) { if (b) return 0; U(3); return 0; } '
RL = 'if (b) return 0; if (cond()) return 0; U(4); return 0;'
CH = 'if (g3 < 0) goto rl; if (g4) goto rl; '
WS = 'if (!cond()) return 0; '
LOOP_BODY = 'U(5); if (c == 13) { U(6); if (!cond()) return 0; U(7); return 1; } i = cond(); if (i == 0) { U(8); return 0; } if (i == 3) { U(9); continue; } if (i == 2) { if (g8) { U(10); return 0; } } '
LOOP = 'for (;;) { ' + LOOP_BODY + 'break; } '
END = 'U(11); if (b == 0) return 1; goto again;'
V = {
 'base': TOP + CH + 'if (g5 < 0) { if (g6) goto rl; if (g7 >= a) { rl: ' + RL + ' } } ' + WS + LOOP + END,
 'end':  TOP + CH + 'if (g5 < 0) { if (g6) goto rl; if (g7 >= a) goto rl; } ' + WS + LOOP + END + ' rl: ' + RL,
 'dowh': TOP + CH + 'if (g5 < 0) { if (g6) goto rl; if (g7 >= a) { rl: ' + RL + ' } } ' + WS + 'do { ' + LOOP_BODY + '} while (0); ' + END,
 'gt': TOP + CH + 'if (g5 < 0) { if (g6) goto rl; if (g7 >= a) { rl: ' + RL + ' } } ' + WS + 'top: ' + LOOP_BODY.replace('continue;', 'goto top;') + END,
 'outer': 'for (;;) { ' + TOP.replace('again: ','') + CH + 'if (g5 < 0) { if (g6) goto rl; if (g7 >= a) goto rl; } ' + WS + LOOP + 'U(11); if (b == 0) return 1; } rl: ' + RL,
 'outerbrk': 'for (;;) { ' + TOP.replace('again: ','') + CH.replace('goto rl','break') + 'if (g5 < 0) { if (g6) break; if (g7 >= a) break; } ' + WS + LOOP.replace('break;','goto brk;') + 'brk: U(11); if (b == 0) return 1; } ' + RL,
 'outerwh': 'do { ' + TOP.replace('again: ','') + CH + 'if (g5 < 0) { if (g6) goto rl; if (g7 >= a) goto rl; } ' + WS + LOOP + 'U(11); } while (b); return 1; rl: ' + RL,
 'jar': TOP + CH + 'if (g5 < 0) { if (g6) goto rl; if (g7 >= a) goto rl; } goto sp; rl: ' + RL + ' sp: ' + WS + LOOP + END,
 'ifz': TOP + CH + 'if (g5 < 0) { if (g6) goto rl; if (g7 >= a) goto rl; } if (0) { rl: ' + RL + ' } ' + WS + LOOP + END,
 'els': TOP + CH + 'if (g5 >= 0 || (!g6 && g7 < a)) { ' + WS + LOOP + END + ' } else { rl: ' + RL + ' }',
 'els2': TOP + CH + 'if (g5 < 0 && (g6 || g7 >= a)) { rl: ' + RL + ' } else { ' + WS + LOOP + END + ' }',
 'wsin': TOP + CH + 'if (g5 < 0) { if (g6) goto rl; if (g7 >= a) { rl: ' + RL + ' } } if (cond()) { ' + LOOP + END + ' } return 0;',
 'back': TOP + 'goto chk; rl: ' + RL + ' chk: ' + CH + 'if (g5 < 0) { if (g6) goto rl; if (g7 >= a) goto rl; } ' + WS + LOOP + END,
 'back0': TOP + 'if (0) { rl: ' + RL + ' } ' + CH + 'if (g5 < 0) { if (g6) goto rl; if (g7 >= a) goto rl; } ' + WS + LOOP + END,
 'backi': TOP + 'if (g == 77) { rl: ' + RL + ' } ' + CH + 'if (g5 < 0) { if (g6) goto rl; if (g7 >= a) goto rl; } ' + WS + LOOP + END,
 'tog1': TOP + 'if (g4 == 3 || g5 == 4) return 0; ' + CH + 'if (g5 < 0) { if (g6) goto rl; if (g7 >= a) { rl: ' + RL + ' } } ' + WS + LOOP + END,
 'tog2': TOP + CH + 'if (g5 < 0) { if (g6) goto rl; if (g7 >= a) { rl: ' + RL + ' } } ' + WS + LOOP + 'if (g4 == 3 || g5 == 4) return 0; ' + END,
 'tog3': TOP + CH + 'if (g5 < 0) { if (g6) goto rl; if (g7 >= a) { rl: ' + RL + ' } } ' + 'if (!cond() || g4 == 3) return 0; ' + LOOP + END,
 'orrl': TOP + 'if (g3 < 0 || g4 || (g5 < 0 && (g6 || g7 >= a))) { ' + RL + ' } ' + WS + LOOP + END,
 'orrl2': TOP + CH + 'if (g5 < 0 && (g6 || g7 >= a)) { rl: ' + RL + ' } ' + WS + LOOP + END,
 'orrl3': TOP + CH + 'if (g5 < 0) { if (g6 || g7 >= a) { rl: ' + RL + ' } } ' + WS + LOOP + END,
 'orrl4': TOP + 'if (g3 < 0 || g4) goto rl; if (g5 < 0) { if (g6) goto rl; if (g7 >= a) { rl: ' + RL + ' } } ' + WS + LOOP + END,
}
def states(L):
    out = []
    for l in L:
        m = re.search(r'EHRec\$\[ebp\+8\], (\S+)', l)
        if m and l.startswith('mov'): out.append('S' + m.group(1))
        elif l.startswith('ret'): out.append('RET')
        elif re.match(r'push\s+\d+$', l): out.append('#' + l.split()[1])
        elif l.startswith('call') and 'use' in l and out and out[-1].startswith('#'): out[-1] = 'u' + out[-1][1:]
        elif l.endswith(':'): out.append(l)
    return ' '.join(o for o in out if not o.startswith('#'))
if __name__ == '__main__':
    sel = sys.argv[1:] or list(V)
    pre = 'P p; ' if EH else ''
    src = HDR + ''.join('int %s(int a, int b, int c) {\nint i; %s%s\n}\n' % (n, pre, V[n]) for n in sel)
    asm, out = ib.compile_asm(None, PFX, text=src)
    F = {f[1]: f[2] for f in ib.functions(asm)}
    for n in sel:
        L = next((l for k, l in F.items() if ('?%s@@' % n) in k), None)
        if L is None:
            print(n, 'MISSING', out[-300:] if out else ''); continue
        L = [l for l in L if not l.startswith(';')]
        print('%-8s %s' % (n, ' '.join(bo.skeleton(L))))
        if os.environ.get('ST'): print('   ' + states(L))
        if os.environ.get('SHOW'): print('   ' + ' ; '.join(L))
