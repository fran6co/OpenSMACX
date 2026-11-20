#!/usr/bin/env python3
"""run.py [name ...]: where VC6 places a multi-predecessor `fail:` return block (MATCHING_GUIDE 3.7, "Fail-block
placement"), on upgrade_prototype 0x004EFE80's skeleton.  Prints blockorder/run.py's skeleton (branches, RET,
use(K) markers as uK, `F` = the fail block's `eax=0`).  SHOW=1 prints the full listing.  FE_PFX_BASE selects
the prefix."""
import sys, os, re
H = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(H) + '/blockorder')
sys.path.insert(0, os.path.dirname(os.path.dirname(H)) + '/inline')
import ib
import run as bo
PFX = os.environ.get('FE_PFX_BASE', '21')
HDR = bo.HDR + 'int pop(int);\n#define U(K) use(K); g9 = K\n'
TOP = 'if ((g & 1) && !(g & 8)) goto fail; if (!b) { if (c) goto fail; if (!(g2 & a)) goto fail; } '
AI = 'if (!(g2 & a)) { U(1); if (g3 > b) goto net; } '
def hum(tail): return 'else { if (a != g4) goto fail; if (b) { U(2); if (b <= g5) { if (!g6) { int r = pop(a); if (!r) goto fail; if (r > 1) g6 = 1; } } else ' + tail + ' } goto net; } '
FAIL = 'fail: return 0; '
NET = 'net: if (g7 & a) { U(3); return 1; } '
END = 'U(5); return 1;'
NE = 'if (c && !(g8 & 4)) goto fail; U(4); return 0;'
V = {
 'base':     TOP + AI + hum('goto ne;') + FAIL + NET + 'if (0) { ne: ' + NE + ' } ' + END,      # ours (v0)
 'inl':      TOP + AI + hum('{ ' + NE + ' }') + FAIL + NET + END,                           # no_energy as the else (v7)
 'gfail':    TOP + AI + hum('goto fail;') + FAIL + NET + END,                               # v2: image placement
 'chain':    TOP + AI + hum('goto ne;') + FAIL + NET + 'if (0) { ne: goto fail; } ' + END,   # v17
 'ret0':     TOP + AI + hum('return 0;') + FAIL + NET + END,
 'neret':    TOP + AI + hum('{ U(4); return 0; }') + FAIL + NET + END,
 'nogo':     TOP + AI + hum('{ U(4); }') + FAIL + NET + END,
 # which goto site claims the fail block
 'h1':       TOP + AI + 'else { if (a != g4) goto fail; if (b) { U(2); if (b <= g5) { if (!g6) { int r = pop(a); if (!r) goto fail; if (r > 1) g6 = 1; } } else goto fail; } if (g8) goto fail; goto net; } ' + FAIL + NET + END,
 'h2':       TOP + AI + hum('{ U(6); goto fail; }') + FAIL + NET + END,
 'h3':       TOP + AI + 'else { if (a != g4) goto fail; if (b) { U(2); if (b <= g5) { if (!g6) { int r = pop(a); if (!r) goto fail; if (r > 1) g6 = 1; } } else return 0; } if (g8) { U(7); goto fail; } goto net; } ' + FAIL + NET + END,
 'h4':       TOP + AI + 'else { if (a != g4) goto fail; if (b) { U(2); if (b <= g5) { if (!g6) { int r = pop(a); if (!r) goto fail; if (r > 1) g6 = 1; } } else return 0; } if (g8) { U(7); } else goto fail; goto net; } ' + FAIL + NET + END,
 'h5':       TOP + AI + 'else { if (a != g4) goto fail; if (b) { U(2); if (b <= g5) { if (!g6) { int r = pop(a); if (!r) goto fail; if (r > 1) g6 = 1; } } else return 0; } if (!g8) goto fail; U(7); goto net; } ' + FAIL + NET + END,
 'h6':       TOP + AI + 'else { if (a != g4) goto fail; goto net; } ' + FAIL + NET + END,
 'h7':       TOP + AI + 'else { U(2); if (a != g4) goto fail; U(7); goto net; } ' + FAIL + NET + END,
 'h8':       TOP + AI + 'else { U(2); if (a == g4) { U(7); goto net; } } ' + FAIL + NET + END,
 'h9':       TOP + AI + 'else { U(2); if (a != g4) goto fail; U(7); } goto net; ' + FAIL + NET + END,
 'b1':       TOP + AI + hum('goto fail;') + FAIL + NET + 'if (g8) goto fail; ' + END,
 'b2':       TOP + AI + hum('goto fail;') + FAIL + NET + 'if (g8) { U(8); goto fail; } ' + END,
 # natural return-0 forms (no fail label): which copy hosts the cross-jumped return 0
 'r1':       'if ((g & 1) && !(g & 8)) return 0; if (!b) { if (c) return 0; if (!(g2 & a)) return 0; } if (!(g2 & a)) { U(1); if (g3 <= b) return 0; } else { if (a != g4) return 0; if (b) { U(2); if (b > g5) { if (c && !(g8 & 4)) return 0; U(4); return 0; } if (!g6) { int r = pop(a); if (!r) return 0; if (r > 1) g6 = 1; } } } ' + NET + END,
 'r2':       'if ((g & 1) && !(g & 8)) return 0; if (!b) { if (c) return 0; if (!(g2 & a)) return 0; } if (!(g2 & a)) { U(1); if (g3 <= b) return 0; } else { if (a != g4) return 0; if (b) { U(2); if (b <= g5) { if (!g6) { int r = pop(a); if (!r) return 0; if (r > 1) g6 = 1; } } else { if (c && !(g8 & 4)) return 0; U(4); return 0; } } } ' + NET + END,
 'r3':       'if ((g & 1) && !(g & 8)) return 0; if (!b) { if (c) return 0; if (!(g2 & a)) return 0; } if (!(g2 & a)) { U(1); if (g3 > b) goto net; return 0; } else { if (a != g4) return 0; if (b) { U(2); if (b <= g5) { if (!g6) { int r = pop(a); if (!r) return 0; if (r > 1) g6 = 1; } } else { if (c && !(g8 & 4)) return 0; U(4); return 0; } } } ' + NET + END,
 'r4':       'if ((g & 1) && !(g & 8)) return 0; if (!b) { if (c) return 0; if (!(g2 & a)) return 0; } if (!(g2 & a)) { U(1); if (g3 <= b) return 0; } else { if (a != g4) return 0; if (b) { U(2); if (b <= g5) { if (!g6) { int r = pop(a); if (!r) return 0; if (r > 1) g6 = 1; } } else goto ne; } } ' + NET + 'if (0) { ne: ' + NE.replace('goto fail', 'return 0') + ' } ' + END,
 # return 0 at the sites, the join's `return 0` (fall-through of the AI arm) as the only label-free copy
 'm1':       TOP.replace('goto fail', 'return 0') + AI + hum('goto ne;').replace('goto fail', 'return 0') + 'return 0; ' + NET + 'if (0) { ne: ' + NE.replace('goto fail', 'return 0') + ' } ' + END,
 'm2':       TOP.replace('goto fail', 'return 0') + AI + hum('goto ne;').replace('goto fail', 'return 0') + FAIL + NET + 'if (0) { ne: ' + NE.replace('goto fail', 'return 0') + ' } ' + END,
 'm3':       TOP + AI + hum('goto ne;') + FAIL + NET + 'if (0) { ne: ' + NE.replace('goto fail', 'return 0') + ' } ' + END,
 'm4':       TOP + AI + hum('goto ne;').replace('if (!r) goto fail', 'if (!r) return 0') + FAIL + NET + 'if (0) { ne: ' + NE.replace('goto fail', 'return 0') + ' } ' + END,
 # backward / forward `else goto fail` sites
 'b3':       TOP + AI + hum('goto fail;') + FAIL + NET + 'if (!g8) { U(8); return 0; } else goto fail; ' + END,
 'b4':       TOP + AI + hum('goto ne;') + FAIL + NET + 'if (0) { ne: if (!c || (g8 & 4)) { U(4); return 0; } else goto fail; } ' + END,
 'f1':       TOP + AI + hum('{ if (!c || (g8 & 4)) { U(4); return 0; } else goto fail; }') + FAIL + NET + END,
 'f2':       TOP + AI + hum('{ if (!c || (g8 & 4)) { U(4); return 0; } goto fail; }') + FAIL + NET + END,
 # the last site in source as a non-claiming `else goto fail` after a non-exiting arm
 'w1':       TOP + AI + 'else { if (a != g4) goto fail; if (b) { U(2); if (b > g5) { ' + NE + ' } if (!g6) { int r = pop(a); if (r) { if (r > 1) g6 = 1; } else goto fail; } } goto net; } ' + FAIL + NET + END,
 'w2':       TOP + AI + 'else { if (a != g4) goto fail; if (b) { U(2); if (b <= g5) { if (!g6) { int r = pop(a); if (r) { if (r > 1) g6 = 1; } else goto fail; } } else goto ne; } goto net; } ' + FAIL + NET + 'if (0) { ne: ' + NE + ' } ' + END,
 'w3':       TOP + AI + 'else { if (a != g4) goto fail; if (b) { U(2); if (b <= g5) { if (!g6) { int r = pop(a); if (r) { if (r > 1) g6 = 1; } else goto fail; } } else { ' + NE + ' } } goto net; } ' + FAIL + NET + END,
 # the human arm after fail: in source (goto-structured), and the arms swapped
 'x1':       TOP + 'if (g2 & a) goto hum; U(1); if (g3 > b) goto net; ' + FAIL + 'hum: if (a != g4) goto fail; if (b) { U(2); if (b <= g5) { if (!g6) { int r = pop(a); if (!r) goto fail; if (r > 1) g6 = 1; } } else { ' + NE + ' } } ' + NET + END,
 'x2':       TOP + 'if (g2 & a) { if (a != g4) goto fail; if (b) { U(2); if (b <= g5) { if (!g6) { int r = pop(a); if (!r) goto fail; if (r > 1) g6 = 1; } } else goto ne; } goto net; } U(1); if (g3 > b) goto net; ' + FAIL + NET + 'if (0) { ne: ' + NE + ' } ' + END,
 'x3':       TOP + 'if (g2 & a) { if (a != g4) goto fail; if (b) { U(2); if (b <= g5) { if (!g6) { int r = pop(a); if (!r) goto fail; if (r > 1) g6 = 1; } } else goto ne; } } else { U(1); if (g3 <= b) goto fail; } ' + NET + 'if (0) { ne: ' + NE + ' } ' + END + ' fail: return 0;',
 # proposal_menu 0x0054DCF0 shape: fail: falls into done:, both reached by later gotos
 'pa':       'U(1); if (a) { U(2); if (b) { if (g7) goto cl; U(3); } fail: g5 = 0; done: g6 = 1; use(10); return g5; } cl: if (c == 13) { U(4); if (g8) goto fail; U(5); if (g4 < 0) goto fail; U(6); } if (!g5 || g5 == 7) goto done; U(8); return 1;',
 'pb':       'U(1); if (a) { U(2); if (b) { if (g7) goto cl; U(3); } fail: g5 = 0; done: g6 = 1; use(10); return g5; } cl: if (c == 13) { U(4); if (g8) goto fail; U(5); if (g4 >= 0) { U(6); } else goto fail; } if (!g5 || g5 == 7) goto done; U(8); return 1;',
 'pc':       'U(1); if (a) { U(2); if (b) { if (g7) goto cl; U(3); } fail: g5 = 0; done: g6 = 1; use(10); return g5; } cl: if (c == 13) { U(4); if (g8) goto fail; U(5); if (g4 >= 0) { U(6); } else goto fail; } if (g5 && g5 != 7) { U(8); return 1; } goto done;',
 'pd':       'U(1); if (a) { U(2); if (b) { if (g7) goto cl; U(3); } fail: g5 = 0; done: g6 = 1; use(10); return g5; } cl: if (c == 13) { U(4); if (g8) goto fail; U(5); if (g4 >= 0) { U(6); } else goto fail; } if (g5 && g5 != 7) { U(8); return 1; } else goto done;',
 'pe':       'U(1); if (a) { U(2); if (b) { if (g7) goto cl; U(3); } fail: g5 = 0; done: g6 = 1; use(10); return g5; } cl: if (c == 13) { U(4); if (g8) goto fail; U(5); if (g4 >= 0) { U(6); } else goto fail; } if (g5 != 7) { U(8); return 1; } else goto done;',
 'pf':       'U(1); if (a) { U(2); if (b) { if (g7) goto cl; U(3); } fail: g5 = 0; done: g6 = 1; use(10); return g5; } cl: if (c == 13) { U(4); if (g8) goto fail; U(5); if (g4 >= 0) { U(6); } else goto fail; } if (g5 != 7) { U(8); } else goto done; return 1;',
 'pg':       'U(1); if (a) { U(2); if (b) { if (g7) goto cl; U(3); } fail: g5 = 0; done: g6 = 1; use(10); return g5; } cl: if (c == 13) { U(4); if (g8) goto fail; U(5); if (g4 >= 0) { U(6); } else goto fail; } if (g5 && g5 != 7) { U(8); } else goto done; return 1;',
 # one label with a fall-through predecessor and one later site
 'q1':       'U(1); if (a) { U(2); done: g6 = 1; use(10); return g5; } U(7); if (g4 || g7) goto done; U(8); return 1;',
 'q2':       'U(1); if (a) { U(2); done: g6 = 1; use(10); return g5; } U(7); if (g4) goto done; U(8); return 1;',
 'q3':       'U(1); if (a) { U(2); done: g6 = 1; use(10); return g5; } U(7); if (g4) { U(8); } else goto done; return 1;',
 'q4':       'U(1); if (a) { U(2); done: g6 = 1; use(10); return g5; } U(7); if (g4 && g7) goto done; U(8); return 1;',
 'q5':       'U(1); if (a) { U(2); done: g6 = 1; use(10); return g5; } U(7); if (g4 || g7) { U(8); } else goto done; return 1;',
 'q6':       'U(1); if (a) { U(2); fail: g5 = 0; done: g6 = 1; use(10); return g5; } U(7); if (g8) goto fail; if (g4 || g7) goto done; U(8); return 1;',
 'q7':       'U(1); if (a) { U(2); fail: g5 = 0; done: g6 = 1; use(10); return g5; } U(7); if (g4 || g7) goto done; if (g8) goto fail; U(8); return 1;',
 'q8':       'U(1); if (a) { U(2); fail: g5 = 0; done: g6 = 1; use(10); return g5; } U(7); if (g4) goto done; if (g8) goto fail; U(8); return 1;',
 'q9':       'U(1); if (a) { U(2); fail: g5 = 0; done: g6 = 1; use(10); return g5; } U(7); if (g8) goto fail; if (g4) goto done; U(8); return 1;',
 # only the last site in source decides
 'q10':      'U(1); if (a) { U(2); done: g6 = 1; use(10); return g5; } U(7); if (g8) goto done; U(9); if (g4 || g7) goto done; U(8); return 1;',
 'q11':      'U(1); if (a) { U(2); done: g6 = 1; use(10); return g5; } U(7); if (g4 || g7) goto done; U(9); if (g8) goto done; U(8); return 1;',
 'q12':      'U(1); if (a) { U(2); done: g6 = 1; use(10); return g5; } U(7); if (g8) goto done; U(9); if (g4) { U(8); } else goto done; return 1;',
 'q13':      'U(1); if (g8) goto done; if (a) { U(2); done: g6 = 1; use(10); return g5; } U(7); if (g4 || g7) goto done; U(8); return 1;',
 'q14':      'U(1); if (a) { U(2); done: g6 = 1; use(10); return g5; } U(7); if (g8) goto done; U(9); if (g4 || g7) goto done; U(8); if (g3) goto done; return 1;',
 # fail: falling into done:, both with non-claiming later sites
 'q15':      'U(1); if (a) { U(2); fail: g5 = 7; done: g6 = 1; use(10); return g5; } U(7); if (g8) { U(9); } else goto fail; if (g4 || g7) goto done; U(8); return 1;',
 'q16':      'U(1); if (a) { U(2); fail: g5 = 7; done: g6 = 1; use(10); return g5; } U(7); if (g4 || g7) goto done; U(8); return 1;',
 'q17':      'U(1); if (a) { U(2); fail: g5 = 7; done: g6 = 1; use(10); return g5; } U(7); if (g8) { U(9); } else goto fail; U(8); return 1;',
 'q18':      'U(1); if (a) { U(2); if (b) goto cl; U(3); fail: g5 = 7; done: g6 = 1; use(10); return g5; } cl: U(7); if (g8) { U(9); } else goto fail; if (g4 || g7) goto done; U(8); return 1;',
 'q19':      'U(1); if (a) { U(2); if (b) goto cl; U(3); done: g6 = 1; use(10); return g5; } cl: U(7); if (g4 || g7) goto done; U(8); return 1;',
 # pb reductions
 'pb1':      'U(1); if (a) { U(2); if (b) { if (g7) goto cl; U(3); } fail: g5 = 7; done: g6 = 1; use(10); return g5; } cl: if (c == 13) { U(4); if (g8) goto fail; U(5); if (g4 >= 0) { U(6); } else goto fail; } if (!g5 || g5 == 7) goto done; U(8); return 1;',
 'pb2':      'U(1); if (a) { U(2); if (b) { if (g7) goto cl; U(3); } fail: g5 = 7; done: g6 = 1; use(10); return g5; } cl: if (c == 13) { U(4); U(5); if (g4 >= 0) { U(6); } else goto fail; } if (!g5 || g5 == 7) goto done; U(8); return 1;',
 'pb3':      'U(1); if (a) { U(2); if (b) { if (g7) goto cl; U(3); } fail: g5 = 7; done: g6 = 1; use(10); return g5; } cl: if (c == 13) { U(4); if (g8) goto fail; U(5); if (g4 >= 0) { U(6); } else goto fail; } if (!g3 || g3 == 7) goto done; U(8); return 1;',
 'pb4':      'U(1); if (a) { U(2); if (b) { if (g7) goto cl; U(3); } fail: g5 = 7; done: g6 = 1; use(10); return g5; } cl: if (c == 13) { U(4); if (g8) goto fail; U(5); if (g4 >= 0) { U(6); } else goto fail; } if (g3 || g2) goto done; U(8); return 1;',
 # q18 (done stays) -> pb4 (done to the end), one difference at a time
 'd1':       'U(1); if (a) { U(2); if (b) { if (g7) goto cl; U(3); } fail: g5 = 7; done: g6 = 1; use(10); return g5; } cl: U(7); if (g8) { U(9); } else goto fail; if (g4 || g7) goto done; U(8); return 1;',
 'd2':       'U(1); if (a) { U(2); if (b) goto cl; U(3); fail: g5 = 7; done: g6 = 1; use(10); return g5; } cl: if (c == 13) { U(7); if (g8) { U(9); } else goto fail; } if (g4 || g7) goto done; U(8); return 1;',
 'd3':       'U(1); if (a) { U(2); if (b) goto cl; U(3); fail: g5 = 7; done: g6 = 1; use(10); return g5; } cl: U(7); if (g3) goto fail; U(6); if (g8) { U(9); } else goto fail; if (g4 || g7) goto done; U(8); return 1;',
 'd4':       'U(1); if (a) { U(2); if (b) goto cl; U(3); fail: g5 = 7; done: g6 = 1; use(10); return g5; } cl: if (c == 13) { U(7); if (g8) { U(9); } else goto fail; U(6); } if (g4 || g7) goto done; U(8); return 1;',
 'd5':       'U(1); if (a) { U(2); if (b) goto cl; U(3); done: g6 = 1; use(10); return g5; } cl: if (c == 13) { U(7); } if (g4 || g7) goto done; U(8); return 1;',
 'd6':       'U(1); if (a) { U(2); if (b) goto cl; U(3); done: g6 = 1; use(10); return g5; } cl: if (c == 13) { U(7); } if (g4) goto done; U(8); return 1;',
 # d2 variants: what keeps done: after fail:
 'e1':  "U(1); if (a) { U(2); if (b) goto cl; U(3); fail: g5 = 7; done: g6 = 1; use(10); return g5; } cl: if (c == 13) { U(7); if (g8) { U(9); } else goto fail; }  if (!g4 && !g7) { U(8); return 1; } goto done;",
 'e2':  "U(1); if (a) { U(2); if (b) goto cl; U(3); fail: g5 = 7; done: g6 = 1; use(10); return g5; } cl: if (c == 13) { U(7); if (g8) { U(9); } else goto fail; }  if (g4 || g7) { } else { U(8); return 1; } goto done;",
 'e3':  "U(1); if (a) { U(2); if (b) goto cl; U(3); fail: g5 = 7; done: g6 = 1; use(10); return g5; } cl: if (c == 13) { U(7); if (g8) { U(9); } else goto fail; }  if (c == 9) { U(11); } if (g4 || g7) goto done; U(8); return 1;",
 'e4':  "U(1); if (a) { U(2); if (b) goto cl; U(3); fail: g5 = 7; done: g6 = 1; use(10); return g5; } cl: if (c == 13) { U(7); if (g8) { U(9); } else goto fail; } else { U(12); } if (g4 || g7) goto done; U(8); return 1;",
 'e5':  "U(1); if (a) { U(2); if (b) goto cl; U(3); fail: g5 = 7; done: g6 = 1; use(10); return g5; } cl: if (c == 13) { U(7); if (g8) { U(9); } else goto fail; } if (g4 || g7) goto done2; U(8); return 1; done2: goto done;",
 'e6':  "U(1); if (a) { U(2); if (b) goto cl; U(3); fail: g5 = 7; done: g6 = 1; use(10); return g5; } cl: if (c == 13) { U(7); if (!g8) goto fail; U(9); } if (g4 || g7) goto done; U(8); return 1;",
 'e7':  "U(1); if (a) { U(2); if (b) goto cl; U(3); fail: g5 = 7; done: g6 = 1; use(10); return g5; } cl: if (c == 13) { U(7); if (g8) { U(9); } else goto fail; U(13); } if (g4 || g7) goto done; U(8); return 1;",
 'k1':  "U(1); if (a) { U(2); if (b) goto cl; U(3); fail: g5 = 7; done: g6 = 1; use(10); return g5; } cl: U(7); if (g8) { U(9); } else goto fail; if (c == 13) { U(13); } if (g4 || g7) goto done; U(8); return 1;",
 'k2':  "U(1); if (a) { U(2); if (b) goto cl; U(3); fail: g5 = 7; done: g6 = 1; use(10); return g5; } cl: if (c == 13) { U(13); } U(7); if (g8) { U(9); } else goto fail; if (g4 || g7) goto done; U(8); return 1;",
 'k3':  "U(1); if (a) { U(2); if (b) goto cl; U(3); done: g6 = 1; use(10); return g5; } cl: if (c == 13) { U(7); if (g8) { U(9); } else goto done; } if (g4 || g7) goto done; U(8); return 1;",
}
if __name__ == '__main__':
    sel = sys.argv[1:] or list(V)
    src = HDR + ''.join('int %s(int a, int b, int c) {\n%s\n}\n' % (n, V[n]) for n in sel)
    asm, out = ib.compile_asm(None, PFX, text=src)
    asm = re.sub(r'^\$(\w+)\$(\d+):', r'$\1_\2:', asm, flags=re.M)
    asm = re.sub(r'\$(fail|net|ne|ok|done|cl)\$(\d+)', r'$\1_\2', asm)
    F = {f[1]: f[2] for f in ib.functions(asm)}
    for n in sel:
        L = next((l for k, l in F.items() if ('?%s@@' % n) in k), None)
        if L is None:
            print(n, 'MISSING', out[-300:] if out else ''); continue
        L = [l for l in L if not l.startswith(';')]
        print('%-10s %s' % (n, ' '.join(bo.skeleton(L))))
        if os.environ.get('SHOW'):
            print('   ' + ' ; '.join(L))
