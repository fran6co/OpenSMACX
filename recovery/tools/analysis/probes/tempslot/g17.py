#!/usr/bin/env python3
"""g17.py: which locals does VC6 pack into a DEAD PARAMETER's home (battle_fight 0x00506AF0: the image keeps the
inlined has_fac's address-taken `mask` in odds_out's dead home +0x20 and the 8-reference nerve_gas flag in a slot
of its own; ours packs nerve_gas into +0x20)? A parameter p read only at the top; after it a heavy local H
(memory-resident by register pressure from p1..p3, or address-taken) and a light local L (address-taken
block-local, or pressure-spilled) whose range lies inside H's. Variants separate H's and L's kinds, their counts,
p's own reference count and whether H's life starts before p's last read."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import t as T
HDR = '''#include "probe.h"
void take(int *);
'''
PRE = '''  int p1 = ga[1], p2 = ga[2], p3 = ga[3], p4 = ga[4];
  for (int G = 0; G < g; ++G) { for (int G2 = 0; G2 < g; ++G2) { call(); use(p1 + G2); use(p2 + G2); use(p3 + G2); use(p4 + G2); } }
'''
POST = '''  for (int G3 = 0; G3 < g; ++G3) { for (int G4 = 0; G4 < g; ++G4) { call(); use(p1 + G4); use(p2 + G4); use(p3 + G4); use(p4 + G4); } }
  use(s);
'''


def u(v, k):
    return ' '.join(['call(); use(%s);' % v] * k)


def P(k):
    return ' '.join(['use(p);'] * k)


def body(h, l, hk, lk, pk=1, hfirst=False):
    """h/l: 'r' = pressure-spilled scalar, 'a' = address-taken; hk/lk counts; pk = p's reads at the top."""
    hdef = 'int H; take(&H);' if h == 'a' else 'int H = ga[s + 7];'
    if l == 'a':
        lb = '{ int L; take(&L); %s }' % u('L', lk - 1)
    elif l == 'n':
        lb = ''
    else:
        lb = '{ int L = ga[s + 9]; %s }' % u('L', lk)
    top = (hdef + ' ' + P(pk)) if hfirst else (P(pk) + ' ' + hdef)
    return '  %s\n%s  %s %s %s\n' % (top, PRE, u('H', hk // 2), lb, u('H', hk - hk // 2))


CASES = {
    # heavy H spilled, light L address-taken (battle_fight's nerve_gas / mask)
    'Hr8_La2': body('r', 'a', 8, 2),
    'Hr8_La2_p4': body('r', 'a', 8, 2, pk=4),
    'Hr8_La2_p12': body('r', 'a', 8, 2, pk=12),
    'Hr2_La8': body('r', 'a', 2, 8),
    'Hr8_Lr2': body('r', 'r', 8, 2),
    'Hr8_Ln': body('r', 'n', 8, 0),
    'Ha8_La2': body('a', 'a', 8, 2),
    'Ha8_Lr2': body('a', 'r', 8, 2),
    'Ha8_Ln': body('a', 'n', 8, 0),
    'Hr8_La2_hfirst': body('r', 'a', 8, 2, hfirst=True),
    'Ha8_La2_hfirst': body('a', 'a', 8, 2, hfirst=True),
}
V = ['void %s(int s, int p) {\n%s%s}\n' % (k, v, POST) for k, v in CASES.items()]
if __name__ == "__main__" and "--part2" not in sys.argv and "--part3" not in sys.argv and "--part4" not in sys.argv:
    T.show(HDR + ''.join(V), asm_of=sys.argv[1] if len(sys.argv) > 1 else None)

# Part 2: address-taken units only (exact counts). A(k) = `int A; take(&A);` then k-1 more `use(A)`.
def A(n, k):
    return '{ int %s; take(&%s); %s' % (n, n, ' '.join(['call(); use(%s);' % n] * (k - 1)))


CASES2 = {
    # X dies before p's last read? no: X lives across p's read (opens own slot), ends; Z light after: X's slot or p's home?
    'x_then_z': '  %s use(p); } %s }\n' % (A('X', 6), A('Z', 2)),
    # same with Z heavier than X
    'x_then_zh': '  %s use(p); } %s }\n' % (A('X', 3), A('Z', 6)),
    # two dead params p, q (both read at the top): which home does a lone unit take?
    'pq_lone': '  use(p); use(q); %s }\n' % A('Z', 4),
    'qp_lone': '  use(q); use(p); %s }\n' % A('Z', 4),
    # p dies first, q later; Z after both
    'p_then_q': '  use(p); call(); use(q); %s }\n' % A('Z', 4),
    # two units after both dead: heavy H and light L overlapping
    'pq_HL': '  use(p); use(q); %s %s } }\n' % (A('H', 8), A('L', 2)),
    # heavy H overlapping p (opens own slot), light L after p dies inside H: p home?
    'H_over_p_L': '  %s use(p); %s } }\n' % (A('H', 8), A('L', 2)),
    # tie: H and M both count 6, both after p, overlapping; which one takes p's home?
    'tie_HM': '  use(p); %s %s } }\n' % (A('H', 6), A('M', 6)),
}
V2 = ['void %s(int s, int p, int q) {\n%s  use(s);\n}\n' % (k, v) for k, v in CASES2.items()]
if __name__ == '__main__' and '--part2' in sys.argv:
    T.show(HDR + ''.join(V2))

# Part 3: param founder count against a local slot's, and two params of different counts.
def Pu(n, k):
    return ' '.join(['use(%s);' % n] * k)


CASES3 = {
    # X(3) local overlapping p's reads, ends; Z(2) after: p read 1x vs 12x
    'x3_p1': '  %s %s } %s }\n' % (A('X', 3), Pu('p', 1), A('Z', 2)),
    'x3_p12': '  %s %s } %s }\n' % (A('X', 3), Pu('p', 12), A('Z', 2)),
    # two params: p read 6x, q read 1x, both before Z (p's last read first / q's last read first)
    'p6q1': '  %s %s %s }\n' % (Pu('p', 6), Pu('q', 1), A('Z', 4)),
    'q1p6': '  %s %s %s }\n' % (Pu('q', 1), Pu('p', 6), A('Z', 4)),
    'q6p1': '  %s %s %s }\n' % (Pu('q', 6), Pu('p', 1), A('Z', 4)),
    'p1q6': '  %s %s %s }\n' % (Pu('p', 1), Pu('q', 6), A('Z', 4)),
    # q read only inside an early-return branch (battle_fight's odds_out), p on the main path
    'qret': '  use(p); if (g) { use(q); return; } %s }\n' % A('Z', 4),
    'qret2': '  if (g) { use(q); return; } use(p); %s }\n' % A('Z', 4),
    # heavy H and light L after: q only in an early-return branch read after p
    'qret_HL': '  use(p); if (g) { use(q); return; } %s %s } }\n' % (A('H', 8), A('L', 2)),
}
V3 = ['void %s(int s, int p, int q) {\n%s  use(s);\n}\n' % (k, v) for k, v in CASES3.items()]
if __name__ == '__main__' and '--part3' in sys.argv:
    T.show(HDR + ''.join(V3))

# Part 4: is a parameter's home live by flow or until its last lexical reference; does a dead initial store
# make a later variable interfere with it (battle_fight: nerve_gas and att_bit avoid odds_out's home)?
CASES4 = {
    # q read lexically after Z's block, on a path Z's block never reaches
    'lex_goto': '  if (g) goto L; %s } return; L: use(q);\n' % A('Z', 4),
    # same, q read before Z lexically in a returning branch (control)
    'lex_ret': '  if (g) { use(q); return; } %s }\n' % A('Z', 4),
    # Z non-address-taken heavy (8 frame accesses needed: a call-heavy body) - dead initial store above q's branch
    'dead_init': '  int N = 0; if (g) { use(q); return; } N = ga[s]; call(); %s %s }\n' % (
        ' '.join(['use(N); call();'] * 4), A('Z', 2)),
    'no_init': '  int N; if (g) { use(q); return; } N = ga[s]; call(); %s %s }\n' % (
        ' '.join(['use(N); call();'] * 4), A('Z', 2)),
}
V4 = ['void %s(int s, int q) {\n%s  use(s);\n}\n' % (k, v) for k, v in CASES4.items()]
if __name__ == '__main__' and '--part4' in sys.argv:
    T.show(HDR + ''.join(V4), asm_of=sys.argv[-1] if sys.argv[-1] in CASES4 else None)
