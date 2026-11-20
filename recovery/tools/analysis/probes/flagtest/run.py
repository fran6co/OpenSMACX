#!/usr/bin/env python3
"""run.py [name ...]: when VC6 threads a flag / `||` test away and when it materialises the value and tests it
(MATCHING_GUIDE 3.7, "Flag tests: threaded or materialised").  Prints blockorder/run.py's skeleton plus every
`mov r,1` / `xor r,r` / `test r,r` / `setcc` (the materialised-test fingerprint is `mov r,1; test r,r; je`).
SHOW=1 prints the full listing.  FE_PFX_BASE selects the compiler prefix."""
import sys, os, re
H = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(H) + '/blockorder')
sys.path.insert(0, os.path.dirname(os.path.dirname(H)) + '/inline')
import ib
import run as bo
PFX = os.environ.get('FE_PFX_BASE', '21')
HDR = bo.HDR + '''void pop(int);
inline int fl_inl(int a, int b) { int c = ga[a]; if (c >= 0) { int m = ga[b]; if (m != 0) { use(2); use(m); return 1; } } return 0; }
inline int fl_inl2(int a, int b) { int c = ga[a]; if (c >= 0 && ga[b] != 0) { use(2); use(ga[b]); return 1; } return 0; }
inline int or_inl(int a, int b) { return ga[a] || cond(); }
inline int one() { return 1; }
inline int nz(int x) { return x != 0; }
inline int isp(int f) { return f == g2; }
inline int isp2(int f) { if (f == g2) return 1; return 0; }
struct S { int f; int m() { return f != 0; } };
extern S gs;
'''
PRE = 'use(1); '
IN = 'int k = ga[a]; if (k >= 0) { int m = ga[b]; if (m != 0) { use(2); use(m); SET } } '
TAIL = 'if (shown) { use(3); use(4); } use(5); return 1;'
def inner(s): return IN.replace('SET', s)
V = {
 # draw_intel 0x004A2A50 shape: flag 0, set 1 in a nested arm, tested once
 'f_base':     'int shown = 0; ' + inner('shown = 1;') + TAIL,
 'f_bool':     'bool shown = false; ' + inner('shown = true;') + TAIL,
 'f_char':     'char shown = 0; ' + inner('shown = 1;') + TAIL,
 'f_and':      'int shown = 0; int k = ga[a]; if (k >= 0 && ga[b] != 0) { use(2); use(ga[b]); shown = 1; } ' + TAIL,
 'f_else':     'int shown; ' + 'int k = ga[a]; if (k >= 0) { int m = ga[b]; if (m != 0) { use(2); use(m); shown = 1; } else shown = 0; } else shown = 0; ' + TAIL,
 'f_late0':    'int k = ga[a]; int shown = 0; if (k >= 0) { int m = ga[b]; if (m != 0) { use(2); use(m); shown = 1; } } ' + TAIL,
 'f_inc':      'int shown = 0; ' + inner('shown++;') + TAIL,
 'f_eq1':      'int shown = 0; ' + inner('shown = 1;') + 'if (shown == 1) { use(3); use(4); } use(5); return 1;',
 'f_neq0':     'int shown = 0; ' + inner('shown = 1;') + 'if (shown != 0) { use(3); use(4); } use(5); return 1;',
 'f_neg':      'int shown = 0; ' + inner('shown = 1;') + 'if (!shown) { use(5); return 1; } use(3); use(4); use(5); return 1;',
 # flag read twice / reused
 'f_twice':    'int shown = 0; ' + inner('shown = 1;') + 'if (shown) { use(3); } use(6); if (shown) { use(4); } use(5); return 1;',
 'f_reuse':    'int shown = 0; ' + inner('shown = 1;') + 'if (shown) { use(3); use(4); } for (shown = 0; shown < 4; ++shown) use(shown); return 1;',
 'f_ret':      'int shown = 0; ' + inner('shown = 1;') + 'if (shown) { use(3); use(4); } use(5); return shown;',
 'f_after':    'int shown = 0; ' + inner('shown = 1;') + 'if (shown) { use(3); use(4); } use(shown); return 1;',
 # flag set to 1 then a call before the test
 'f_call':     'int shown = 0; ' + inner('shown = 1;') + 'use(7); ' + TAIL,
 'f_callin':   'int shown = 0; int k = ga[a]; if (k >= 0) { int m = ga[b]; if (m != 0) { use(2); shown = 1; use(m); } } ' + TAIL,
 # flag set before the inner test, cleared in the miss arm
 'f_pre1':     'int shown = 1; int k = ga[a]; if (k >= 0) { int m = ga[b]; if (m != 0) { use(2); use(m); } else shown = 0; } else shown = 0; ' + TAIL,
 # flag through an inline return
 'f_inl':      'int shown = fl_inl(a, b); ' + TAIL,
 'f_inl_if':   'if (fl_inl(a, b)) { use(3); use(4); } use(5); return 1;',
 'f_inl2':     'int shown = fl_inl2(a, b); ' + TAIL,
 # computed bool
 'f_expr':     'int k = ga[a]; int shown = k >= 0 && ga[b] != 0; if (shown) { use(2); use(ga[b]); } ' + TAIL,
 'f_tern':     'int k = ga[a]; int shown = k >= 0 ? (ga[b] != 0) : 0; ' + TAIL,
 # flag in a do/while(0) or loop
 'f_dowh':     'int shown = 0; do { int k = ga[a]; if (k < 0) break; int m = ga[b]; if (m == 0) break; use(2); use(m); shown = 1; } while (0); ' + TAIL,
 'f_for1':     'int shown = 0; for (int i = 0; i < 1; i++) { ' + inner('shown = 1;') + '} ' + TAIL,
 'f_goto':     'int k = ga[a]; if (k < 0) goto skip; { int m = ga[b]; if (m == 0) goto skip; use(2); use(m); } use(3); use(4); skip: use(5); return 1;',
 # flag as a member / global / static / volatile / address taken
 'f_glob':     'g7 = 0; ' + inner('g7 = 1;') + 'if (g7) { use(3); use(4); } use(5); return 1;',
 'f_static':   'static int shown; shown = 0; ' + inner('shown = 1;') + TAIL,
 'f_vol':      'volatile int shown = 0; ' + inner('shown = 1;') + TAIL,
 'f_addr':     'int shown = 0; ' + inner('shown = 1;') + 'call2(&shown); ' + TAIL,
 'f_param':    'c = 0; if (ga[a] >= 0) { int m = ga[b]; if (m != 0) { use(2); use(m); c = 1; } } if (c) { use(3); use(4); } use(5); return 1;',
 'f_setinl':   'int shown = 0; ' + inner('shown = one();') + TAIL,
 'f_setval':   'int shown = 0; int k = ga[a]; if (k >= 0) { int m = ga[b]; shown = m != 0; if (shown) { use(2); use(m); } } ' + TAIL,
 'f_testinl':  'int shown = 0; ' + inner('shown = 1;') + 'if (nz(shown)) { use(3); use(4); } use(5); return 1;',
 'f_for1w':    'int shown = 0; int i = 0; while (i < 1) { ' + inner('shown = 1;') + 'i++; } ' + TAIL,
 'f_for1d':    'int shown = 0; int i = 0; do { ' + inner('shown = 1;') + 'i++; } while (i < 1); ' + TAIL,
 'f_for1c':    'int shown = 0; for (int i = 0; i < 1; i++) { ' + inner('shown = 1;') + '} use(7); ' + TAIL,
 'f_for1nf':   'for (int i = 0; i < 1; i++) { int k = ga[a]; if (k >= 0) { int m = ga[b]; if (m != 0) { use(2); use(m); } } } use(3); use(4); use(5); return 1;',
 # what keeps the image's materialised test (draw_intel): a one-trip loop around the arm, labels, other loops
 'f_for1i':    'int shown = 0; for (int i = 0; i < 1; i++) { int k = ga[a + i]; if (k >= 0) { int m = ga[b + i]; if (m != 0) { use(2); use(m); shown = 1; } } } ' + TAIL,
 'f_for2brk':  'int shown = 0; for (int i = 0; i < 2; i++) { int k = ga[a + i]; if (k >= 0) { int m = ga[b + i]; if (m != 0) { use(2); use(m); shown = 1; break; } } } ' + TAIL,
 'f_forev':    'int shown = 0; for (;;) { ' + inner('shown = 1;') + 'break; } ' + TAIL,
 'f_while1':   'int shown = 0; while (1) { ' + inner('shown = 1;') + 'break; } ' + TAIL,
 'f_wh0':      'int shown = 0; while (!shown) { ' + inner('shown = 1;') + 'break; } ' + TAIL,
 'f_label':    'int shown = 0; ' + inner('shown = 1;') + 'L: ' + TAIL,
 'f_lblused':  'int shown = 0; if (g) goto L; { ' + inner('shown = 1;') + '} L: ' + TAIL,
 'f_loopbef':  'for (int i = 0; i < a; i++) use(i); int shown = 0; ' + inner('shown = 1;') + TAIL,
 'f_loopaft':  'int shown = 0; ' + inner('shown = 1;') + TAIL[:-10] + 'for (int i = 0; i < a; i++) use(i); return 1;',
 'f_forg':     'int shown = 0; for (int i = 0; i < g; i++) { ' + inner('shown = 1;') + '} ' + TAIL,
 'f_sw':       'int shown = 0; switch (a) { case 0: ' + inner('shown = 1;') + 'break; } ' + TAIL,
 'f_scope':    'int shown = 0; { ' + inner('shown = 1;') + '} ' + TAIL,
 # else-if re-test of the same compare (double_cross 0x0050BCE4 / 0x0050BE1A keeps it)
 'e_base':     'if (g) { if (b == g2 && c > 0) { use(1); pop(1); } else if (b == g2 && vis2(a)) { use(2); pop(1); } } use(3); return 1;',
 'e_noouter':  'if (b == g2 && c > 0) { use(1); pop(1); } else if (b == g2 && vis2(a)) { use(2); pop(1); } use(3); return 1;',
 'e_glob':     'if (g) { if (g3 == g2 && c > 0) { use(1); pop(1); } else if (g3 == g2 && vis2(a)) { use(2); pop(1); } } use(3); return 1;',
 'e_struct':   'if (g) { if (b == gs.f && c > 0) { use(1); pop(1); } else if (b == gs.f && vis2(a)) { use(2); pop(1); } } use(3); return 1;',
 'e_calls':    'if (g) { if (b == g2 && c > 0) { use(1); use(4); use(5); pop(1); } else if (b == g2 && vis2(a)) { use(2); pop(1); } } use(3); return 1;',
 'e_ret':      'if (g) { if (b == g2 && c > 0) { use(1); pop(1); } else if (b == g2 && vis2(a)) { use(2); pop(1); } } use(3); return b;',
 'e_inl1':     'if (g) { if (isp(b) && c > 0) { use(1); pop(1); } else if (b == g2 && vis2(a)) { use(2); pop(1); } } use(3); return 1;',
 'e_inl2':     'if (g) { if (b == g2 && c > 0) { use(1); pop(1); } else if (isp(b) && vis2(a)) { use(2); pop(1); } } use(3); return 1;',
 'e_inl12':    'if (g) { if (isp(b) && c > 0) { use(1); pop(1); } else if (isp(b) && vis2(a)) { use(2); pop(1); } } use(3); return 1;',
 'e_inlif':    'if (g) { if (isp2(b) && c > 0) { use(1); pop(1); } else if (isp2(b) && vis2(a)) { use(2); pop(1); } } use(3); return 1;',
 'e_neq':      'if (g) { if (b == g2 && c > 0) { use(1); pop(1); } else if ((b == g2) != 0 && vis2(a)) { use(2); pop(1); } } use(3); return 1;',
 'e_ge1':      'if (g) { if (b == g2 && c >= 1) { use(1); pop(1); } else if (b == g2 && vis2(a)) { use(2); pop(1); } } use(3); return 1;',
 'e_hop':      'if (g && b == g2 && c > 0) { use(1); pop(1); } else if (g && b == g2 && vis2(a)) { use(2); pop(1); } use(3); return 1;',
 # the || value forms of on_key_click
 'o_if':       'if (ga[a] || cond()) { use(3); return 0; } use(4); use(5); return 1;',
 'o_val':      'int go = ga[a] || cond(); if (go) { use(3); return 0; } use(4); use(5); return 1;',
 'o_inl':      'if (or_inl(a, b)) { use(3); return 0; } use(4); use(5); return 1;',
 'o_tern':     'if (ga[a] ? 1 : cond()) { use(3); return 0; } use(4); use(5); return 1;',
 'o_join':     'if (ga[a] || cond()) { use(3); } use(4); use(5); return 1;',
 'o_valjoin':  'int go = ga[a] || cond(); if (go) { use(3); } use(4); use(5); return 1;',
 'o_and':      'if (ga[a] && cond()) { use(3); return 0; } use(4); use(5); return 1;',
 'o_valand':   'int go = ga[a] && cond(); if (go) { use(3); return 0; } use(4); use(5); return 1;',
 'o_mem':      'if (gs.m()) { use(3); return 0; } use(4); return 1;',
}
def skel(L):
    out = []
    for l in L:
        if re.match(r'(mov\s+\w+, 1|xor\s+(\w+), \2|test\s+(\w+), \3|set\w+\s|cmp\s+\w+, 1)$', l) or l.startswith('set'):
            out.append('[' + re.sub(r'\s+', ' ', l) + ']')
            continue
        out.extend(bo.skeleton([l]) if not (l.startswith('call') or l.startswith('push')) else [])
        if l.startswith('call'):
            out.extend(bo.skeleton(pend + [l])); pend.clear()
        elif l.startswith('push'):
            pend.append(l)
    return out
pend = []
if __name__ == '__main__':
    sel = sys.argv[1:] or list(V)
    src = HDR + ''.join('int %s(int a, int b, int c) {\n%s\n}\n' % (n, V[n]) for n in sel)
    asm, out = ib.compile_asm(None, PFX, text=src)
    F = {f[1]: f[2] for f in ib.functions(asm)}
    for n in sel:
        L = next((l for k, l in F.items() if ('?%s@@' % n) in k), None)
        if L is None:
            print(n, 'MISSING'); continue
        L = [l for l in L if not l.startswith(';')]
        pend.clear()
        print('%-10s %s' % (n, ' '.join(skel(L))))
        if os.environ.get('SHOW'):
            print('   ' + ' ; '.join(L))
