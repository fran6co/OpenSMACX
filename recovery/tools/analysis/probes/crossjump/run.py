#!/usr/bin/env python3
"""run.py [name ...]: VC6 cross-jumping (tail merging) ground truth (MATCHING_GUIDE 3.7, "Cross-jumping").
Each variant is one cdecl function; prints a compact listing (labels as Ln:, `use(K)` as uK, `f1(x)` etc. by
name).  For every `jmp` whose target label is reached by fall-through from the previous instruction (a jump into
the middle of straight-line code = a cross-jump or a join), the line is marked `>>`.  Summary: the number of
`call f*` / `uK` occurrences, so a merged statement shows as a missing copy.  SHOW=1 prints the raw listing.
FE_PFX_BASE selects the compiler prefix (default 21)."""
import sys, os, re
H = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(os.path.dirname(H)) + '/inline')
import ib
PFX = os.environ.get('FE_PFX_BASE', '21')
HDR = '''#include "probe.h"
void f1(int); void f2(int, int); int r1(int); void f0(); void q0();
struct D { int v; D(); ~D(); };
'''
V = {
 # Q1: per instruction or per statement? same call, args from different registers/values
 'same2':      'if (a) { use(1); f2(b, c); } else { use(2); f2(b, c); } use(9); return 1;',
 'argdiff':    'if (a) { use(1); f2(b, c); } else { use(2); f2(g, c); } use(9); return 1;',
 'argdiff_k':  'if (a) { use(1); f2(1, c); } else { use(2); f2(2, c); } use(9); return 1;',
 'twostmt':    'if (a) { use(1); f1(b); f1(c); } else { use(2); f1(b); f1(c); } use(9); return 1;',
 'twostmt_d1': 'if (a) { use(1); f1(b); f1(c); } else { use(2); f1(g); f1(c); } use(9); return 1;',
 'store_tail': 'if (a) { use(1); g = b; } else { use(2); g = b; } use(9); return 1;',
 'store_k':    'if (a) { use(1); g = 5; } else { use(2); g = 5; } use(9); return 1;',
 'f0_tail':    'if (a) { use(1); f0(); } else { use(2); f0(); } use(9); return 1;',
 # Q3 a different following block: no merge expected
 'difffollow': 'if (a) { use(1); f1(b); goto x; } use(2); f1(b); use(8); x: use(9); return 1;',
 # Q6/Q7 placements of a common tail X = f1(b); f1(c)
 'X_both':     'if (a) { use(1); f1(b); f1(c); } else { use(2); use(3); f1(b); f1(c); } use(9); return 1;',
 'X_after':    'if (a) { use(1); } else { use(2); use(3); } f1(b); f1(c); use(9); return 1;',
 'X_else':     'if (a) { f1(b); f1(c); } else { use(2); f1(b); f1(c); } use(9); return 1;',
 'X_elseonly': 'if (!a) { use(2); } f1(b); f1(c); use(9); return 1;',
 'X_goto':     'if (a) goto x; use(2); x: f1(b); f1(c); use(9); return 1;',
 # tails ending the function: duplicated join?
 'ret_goto1':  'if (a) { use(1); goto done; } use(2); done: f1(b); return 1;',
 'ret_goto2':  'if (a) { use(1); goto done; } use(2); done: f1(b); f1(c); return 1;',
 'ret_goto4':  'if (a) { use(1); goto done; } use(2); done: f1(b); f1(c); f1(1); f1(2); return 1;',
 'ret_goto8':  'if (a) { use(1); goto done; } use(2); done: f1(b); f1(c); f1(1); f1(2); f1(3); f1(4); f1(5); f1(6); return 1;',
 'ret_dup2':   'if (a) { use(1); f1(b); f1(c); return 1; } use(2); f1(b); f1(c); return 1;',
 'ret_join2':  'if (a) { use(1); } else { use(2); } f1(b); f1(c); return 1;',
 'ret_val':    'if (a) { use(1); return r1(b); } use(2); return r1(c);',
 'ret_valsame': 'if (a) { use(1); return r1(b); } use(2); return r1(b);',
 'ret_var':    'int r; if (a) { use(1); r = r1(b); } else { use(2); r = r1(c); } return r;',
 'ret_k':      'if (a) { use(1); f1(b); return 2; } use(2); f1(b); return 3;',
 # a join that does not end the function
 'mid_goto2':  'if (a) { use(1); goto done; } use(2); done: f1(b); f1(c); if (c) use(7); use(9); return 1;',
 # three arms with the same tail; which copy survives
 'three':      'if (a == 1) { use(1); f1(b); } else if (a == 2) { use(2); f1(b); } else { use(3); f1(b); } use(9); return 1;',
 'three_nofall': 'if (a == 1) { use(1); f1(b); } else if (a == 2) { use(2); f1(b); } else { use(3); f1(c); } use(9); return 1;',
 # loops: continue tails
 'loop_cont':  'int i; for (i = 0; i < a; i++) { if (ga[i]) { use(1); f1(i); continue; } use(2); f1(i); } return 1;',
 'loop_brk':   'int i; for (i = 0; i < a; i++) { if (ga[i] == 1) { use(1); f1(b); break; } if (ga[i] == 2) { use(2); f1(b); break; } } use(9); return 1;',
 # join duplication: small return-ending joins are copied into each non-merging arm (X_both, store_tail)
 'jd1':        'if (a) { use(1); g = b; } else { use(2); g = b; } use(9); return 1;',
 'jd2':        'if (a) { use(1); g = b; } else { use(2); g = b; } use(9); use(8); return 1;',
 'jd3':        'if (a) { use(1); g = b; } else { use(2); g = b; } use(9); use(8); use(7); return 1;',
 'jd4':        'if (a) { use(1); g = b; } else { use(2); g = b; } use(9); use(8); use(7); use(6); return 1;',
 'jd6':        'if (a) { use(1); g = b; } else { use(2); g = b; } use(9); use(8); use(7); use(6); use(5); use(4); return 1;',
 'jd_if':      'if (a) { use(1); g = b; } else { use(2); g = b; } if (c) use(9); return 1;',
 'jd_noret':   'int i; for (i = 0; i < a; i++) { if (ga[i]) { use(1); g = b; } else { use(2); g = b; } use(9); } return 1;',
 'jd_retv':    'if (a) { use(1); g = b; } else { use(2); g = b; } return c;',
 'jd_esi':     'int k = r1(b); if (a) { use(1); f1(k); } else { use(2); f1(k); } use(k); return 1;',
 'jd_esi2':    'int k = r1(b); if (a) { use(1); f1(k); f1(k); } else { use(2); f1(k); f1(k); } use(k); return k;',
 'pop_diff':   'if (a) { use(1); f0(); } else { use(2); use(3); f0(); } use(9); return 1;',
 'pop_flush':  'if (a) { use(1); if (c) use(5); f0(); } else { use(2); use(3); f0(); } use(9); return 1;',
 # scratch register rotation across sites: which identical arm tails merge
 'rot6':       'switch (a) { case 1: use(1); f1(b); break; case 2: use(2); f1(b); break; case 3: use(3); f1(b); break; case 4: use(4); f1(b); break; case 5: use(5); f1(b); break; case 6: use(6); f1(b); break; } use(9); return 1;',
 'rot6r':      'switch (a) { case 1: f1(b); return 1; case 2: f1(b); return 1; case 3: f1(b); return 1; case 4: f1(b); return 1; case 5: f1(b); return 1; case 6: f1(b); return 1; } use(9); return 1;',
 'rot6g':      'if (a == 1) { f1(b); return 1; } if (a == 2) { f1(b); return 1; } if (a == 3) { f1(b); return 1; } if (a == 4) { f1(b); return 1; } if (a == 5) { f1(b); return 1; } if (a == 6) { f1(b); return 1; } use(9); return 1;',
 'rot_gl':     'if (a == 1) { f1(g); return 1; } if (a == 2) { f1(g); return 1; } if (a == 3) { f1(g); return 1; } if (a == 4) { f1(g); return 1; } use(9); return 1;',
 'rot_2stmt':  'if (a == 1) { f1(b); f1(c); return 1; } if (a == 2) { f1(b); f1(c); return 1; } if (a == 3) { f1(b); f1(c); return 1; } if (a == 4) { f1(b); f1(c); return 1; } use(9); return 1;',
 # threshold for cross-jumping whole return blocks in an if-chain
  'thr_f1k': 'if (a == 1) { f1(1); return 1; } if (a == 2) { f1(1); return 1; } if (a == 3) { f1(1); return 1; } if (a == 4) { f1(1); return 1; } use(9); return 1;',
  'thr_f1kk': 'if (a == 1) { f1(1); f1(2); return 1; } if (a == 2) { f1(1); f1(2); return 1; } if (a == 3) { f1(1); f1(2); return 1; } if (a == 4) { f1(1); f1(2); return 1; } use(9); return 1;',
  'thr_f0': 'if (a == 1) { f0(); return 1; } if (a == 2) { f0(); return 1; } if (a == 3) { f0(); return 1; } if (a == 4) { f0(); return 1; } use(9); return 1;',
  'thr_f0f0': 'if (a == 1) { f0(); f0(); return 1; } if (a == 2) { f0(); f0(); return 1; } if (a == 3) { f0(); f0(); return 1; } if (a == 4) { f0(); f0(); return 1; } use(9); return 1;',
  'thr_f0x3': 'if (a == 1) { f0(); f0(); f0(); return 1; } if (a == 2) { f0(); f0(); f0(); return 1; } if (a == 3) { f0(); f0(); f0(); return 1; } if (a == 4) { f0(); f0(); f0(); return 1; } use(9); return 1;',
  'thr_bk': 'if (a == 1) { f1(b); f1(1); return 1; } if (a == 2) { f1(b); f1(1); return 1; } if (a == 3) { f1(b); f1(1); return 1; } if (a == 4) { f1(b); f1(1); return 1; } use(9); return 1;',
  'thr_gk': 'if (a == 1) { g = 1; g2 = 2; return 1; } if (a == 2) { g = 1; g2 = 2; return 1; } if (a == 3) { g = 1; g2 = 2; return 1; } if (a == 4) { g = 1; g2 = 2; return 1; } use(9); return 1;',
  'thr_k3': 'if (a == 1) { g = 1; g2 = 2; g3 = 3; return 1; } if (a == 2) { g = 1; g2 = 2; g3 = 3; return 1; } if (a == 3) { g = 1; g2 = 2; g3 = 3; return 1; } if (a == 4) { g = 1; g2 = 2; g3 = 3; return 1; } use(9); return 1;',
  'thr_f2': 'if (a == 1) { f2(b, 1); return 1; } if (a == 2) { f2(b, 1); return 1; } if (a == 3) { f2(b, 1); return 1; } if (a == 4) { f2(b, 1); return 1; } use(9); return 1;',
  'thr_f2kk': 'if (a == 1) { f2(1, 2); return 1; } if (a == 2) { f2(1, 2); return 1; } if (a == 3) { f2(1, 2); return 1; } if (a == 4) { f2(1, 2); return 1; } use(9); return 1;',
  'thr_f0x4': 'if (a == 1) { f0(); f0(); f0(); f0(); return 1; } if (a == 2) { f0(); f0(); f0(); f0(); return 1; } if (a == 3) { f0(); f0(); f0(); f0(); return 1; } if (a == 4) { f0(); f0(); f0(); f0(); return 1; } use(9); return 1;',
  'thr_f0x3r2': 'if (a == 1) { f0(); f0(); f0(); return 2; } if (a == 2) { f0(); f0(); f0(); return 2; } if (a == 3) { f0(); f0(); f0(); return 2; } if (a == 4) { f0(); f0(); f0(); return 2; } use(9); return 1;',
  'thr_f1f0': 'if (a == 1) { f1(1); f0(); return 1; } if (a == 2) { f1(1); f0(); return 1; } if (a == 3) { f1(1); f0(); return 1; } if (a == 4) { f1(1); f0(); return 1; } use(9); return 1;',
  'thr_f0x2r2': 'if (a == 1) { f0(); f0(); return 2; } if (a == 2) { f0(); f0(); return 2; } if (a == 3) { f0(); f0(); return 2; } if (a == 4) { f0(); f0(); return 2; } use(9); return 1;',
  'thr_f0x8': 'if (a == 1) { f0(); f0(); f0(); f0(); f0(); f0(); f0(); f0(); return 1; } if (a == 2) { f0(); f0(); f0(); f0(); f0(); f0(); f0(); f0(); return 1; } if (a == 3) { f0(); f0(); f0(); f0(); f0(); f0(); f0(); f0(); return 1; } if (a == 4) { f0(); f0(); f0(); f0(); f0(); f0(); f0(); f0(); return 1; } use(9); return 1;',
  'thr_f0g': 'if (a == 1) { f0(); g = 5; f0(); g2 = 5; return 1; } if (a == 2) { f0(); g = 5; f0(); g2 = 5; return 1; } if (a == 3) { f0(); g = 5; f0(); g2 = 5; return 1; } if (a == 4) { f0(); g = 5; f0(); g2 = 5; return 1; } use(9); return 1;',
  'thr_f1x2': 'if (a == 1) { f1(1); f1(1); return 1; } if (a == 2) { f1(1); f1(1); return 1; } if (a == 3) { f1(1); f1(1); return 1; } if (a == 4) { f1(1); f1(1); return 1; } use(9); return 1;',
  'thr_f1f1f0': 'if (a == 1) { f1(1); f1(2); f0(); return 1; } if (a == 2) { f1(1); f1(2); f0(); return 1; } if (a == 3) { f1(1); f1(2); f0(); return 1; } if (a == 4) { f1(1); f1(2); f0(); return 1; } use(9); return 1;',
  'thr_f1x2r': 'if (a == 1) { f1(1); f1(2); return 2; } if (a == 2) { f1(1); f1(2); return 2; } if (a == 3) { f1(1); f1(2); return 2; } if (a == 4) { f1(1); f1(2); return 2; } use(9); return 1;',
  'thr_g4': 'if (a == 1) { g = 5; g2 = 5; g3 = 5; g4 = 5; return 2; } if (a == 2) { g = 5; g2 = 5; g3 = 5; g4 = 5; return 2; } if (a == 3) { g = 5; g2 = 5; g3 = 5; g4 = 5; return 2; } if (a == 4) { g = 5; g2 = 5; g3 = 5; g4 = 5; return 2; } use(9); return 1;',
 # destructor teardown at return sites
 'dtor_ret':   'D x; if (a) { use(1); return 1; } if (b) { use(2); return 1; } use(3); return 1;',
 'dtor_ret2':  'D x; if (a) { use(1); return 1; } { D y; if (b) { use(2); return 1; } use(4); } use(3); return 1;',
 'dtor_retk':  'D x; if (a) { use(1); return 2; } if (b) { use(2); return 3; } use(3); return 1;',
 'dtor_nest':  'D x; { D y; if (a) { use(1); return 1; } if (b) { use(2); return 1; } } use(3); return 1;',
}
def compact(L):
    lab = {}
    out = []
    names = {}
    for l in L:
        if l.endswith(':'):
            out.append(l[:-1].split('$')[-1] + ':')
            continue
        m = re.match(r'(j\w+)\s+\$(\w+)', l)
        if m:
            out.append('%s %s' % (m.group(1), m.group(2).split('$')[-1]))
            continue
        out.append(re.sub(r'\?(\w+)@@\w+', r'\1', l).replace('DWORD PTR ', '').replace('\t', ' '))
    # mark jmps into fall-through labels
    pos = {s[:-1]: k for k, s in enumerate(out) if s.endswith(':')}
    res = []
    for k, s in enumerate(out):
        m = re.match(r'jmp (\S+)', s)
        tag = ''
        if m and m.group(1) in pos:
            p = pos[m.group(1)]
            q = p - 1
            while q >= 0 and out[q].endswith(':'):
                q -= 1
            if q >= 0 and not re.match(r'(jmp|ret)', out[q]):
                tag = '>>'
        res.append(tag + s)
    return res
def short(c):
    """use(K) as uK: the push before call use"""
    o = []
    for s in c:
        if s.startswith('call') and 'use' in s and o and o[-1].startswith('push'):
            o[-1] = 'u' + o[-1].split()[-1]
            continue
        if re.match(r'(push|mov|add|xor|lea|test|cmp|sub|pop|inc|dec)', s) and not os.environ.get('FULL'):
            if s.startswith('add esp') or s.startswith('push') or s.startswith('call') or s.startswith('mov') or s.startswith('cmp') or s.startswith('test') or s.startswith('pop') or s.startswith('lea') or s.startswith('xor'):
                o.append(s.replace(' ', '', 0)); continue
        o.append(s)
    return o
if __name__ == '__main__':
    sel = sys.argv[1:] or list(V)
    src = HDR + ''.join('int %s(int a, int b, int c) {\n%s\n}\n' % (n, V[n]) for n in sel)
    asm, out = ib.compile_asm(None, PFX, text=src)
    F = {f[1]: f[2] for f in ib.functions(asm)}
    for n in sel:
        L = next((l for k, l in F.items() if ('?%s@@' % n) in k), None)
        if L is None:
            print(n, 'MISSING', out[-300:] if out else ''); continue
        c = short(compact(L))
        calls = {}
        for s in c:
            m = re.match(r'call\s+(\w+)', s)
            if m: calls[m.group(1)] = calls.get(m.group(1), 0) + 1
        print('== %-12s %s | %s' % (n, V[n], ' '.join('%s*%d' % kv for kv in sorted(calls.items()))))
        print('   ' + ' ; '.join(c))
        if os.environ.get('SHOW'):
            print('   RAW ' + ' ; '.join(L))
