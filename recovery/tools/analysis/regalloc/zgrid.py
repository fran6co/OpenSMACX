#!/usr/bin/env python3
"""zgrid.py [family ...]: constant formation / materialisation / rank grids (MATCHING_GUIDE 2.7 Z9).

Per probe: the callee-saved register holding the zero (`xor r,r`), its register uses, the immediate zero
references left and the `xor eax,eax` pieces.  Families: stores (S stores split by calls, compares, pushes),
loop (stores in a loop, 2^depth), arms (if/else/switch/&&), rank (one rival with 3 references against S stores
plus compares, pushes or rival compares).  Formation itself is c_zform.spec (`python3 cand.py c_zform.spec`)."""
import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import cand
CS = ('esi', 'edi', 'ebx')
H = 'extern int gz[32], gl[32], gm; extern int *gpa[512];\n'


def classify(ins):
    z = None
    for i in ins:
        m = re.match(r'xor\s+(e[sd]i|ebx), \1$', i)
        if m:
            z = m.group(1)
            break
    imm = sum(1 for i in ins if re.search(r'(, 0$|^push 0$)', i))
    regu = sum(1 for i in ins if z and re.search(r'(, %s$|^push %s$|^cmp .*, %s$)' % (z, z, z), i))
    piece = sum(1 for i in ins if re.match(r'xor\s+eax, eax', i))
    return '%s use=%d imm=%d xeax=%d' % (z or '-', regu, imm, piece)


def st(S, off=0):
    return ' '.join('call(); gz[%d] = 0;' % (k + off) for k in range(S))


def fam_stores():
    J = []
    for S in range(1, 7):
        J.append(('st_sep/%d' % S, H + 'void probe() { %s call(); }' % st(S)))
    for C in (0, 3, 7):
        for S in (3, 4):
            J.append(('cmp%d_st%d' % (C, S), H + 'void probe() { %s %s call(); }' % (
                ' '.join('if (gl[%d] == 0) call();' % k for k in range(C)), st(S))))
            J.append(('push%d_st%d' % (C, S), H + 'void probe() { %s %s call(); }' % (' '.join('use_p(0);' for _ in range(C)), st(S))))
    return J


def fam_loop():
    J = []
    for L in (1, 2):
        for A in (0, 1, 2):
            loop = 'for (int **p = gpa; p < gpa + gm; p++) { %s }' % st(L)
            J.append(('loop%d_after%d' % (L, A), H + 'void probe() { %s %s call(); }' % (loop, st(A, 10))))
    return J


def fam_arms():
    J = []
    for S in (3, 4):
        J.append(('ifs/%d' % S, H + 'void probe() { %s call(); }' % ' '.join('if (cond()) gz[%d] = 0;' % k for k in range(S))))
        J.append(('else/%d' % S, H + 'void probe() { %s call(); }' % ' '.join('if (cond()) call(); else gz[%d] = 0;' % k for k in range(S))))
        J.append(('andand/%d' % S, H + 'void probe() { %s call(); }' % ' '.join('if (cond() && cond()) gz[%d] = 0;' % k for k in range(S))))
        J.append(('cases/%d' % S, H + 'void probe(int v) { switch (v) { %s } call(); }' % ' '.join(
            'case %d: call(); gz[%d] = 0; break;' % (k, k) for k in range(S))))
    for S in (2, 3):
        J.append(('ifelse_pairs/%d' % S, H + 'void probe() { %s call(); }' % ' '.join(
            'if (cond()) gz[%d] = 0; else gl[%d] = 0;' % (k, k) for k in range(S))))
    J.append(('adj4_nocall', H + 'void probe() { call(); gz[0]=0; gz[1]=0; gz[2]=0; gz[3]=0; call(); }'))
    return J


def fam_rank():
    J = []
    pq = 'D(p); D(q); U8(p); U8(q); U8(p); U8(q); '
    end = ' U8(p); U8(q); U8(p); U8(q);'
    for S in (4, 5, 7):
        for kind in ('cmp', 'push', 'rcmp'):
            for C in (0, 2, 4, 8):
                x = ' '.join({'cmp': 'if (gl[%d] == 0) call();' % k, 'push': 'use_p(0);', 'rcmp': 'if (c == 0) call();'}[kind]
                             for k in range(C))
                J.append(('S%d_%s%d' % (S, kind, C), H + 'void probe() { %s D(c); %s %s U(c); U(c);%s }' % (pq, st(S), x, end)))
    return J


FAM = dict(stores=fam_stores, loop=fam_loop, arms=fam_arms, rank=fam_rank)
if __name__ == '__main__':
    for f in sys.argv[1:] or list(FAM):
        J = FAM[f]()
        res = cand.evaluate(J)
        for k, _ in J:
            print('%-6s %-18s %s' % (f, k, classify(res[k])))
