#!/usr/bin/env python3
"""run.py [name ...]: which arm VC6 places in line and which it moves after the `ret` (MATCHING_GUIDE 3.7,
"Block order").  Each variant is one cdecl function; prints its block skeleton: every instruction is dropped
except branches, `ret` and the `use(K)` markers (as uK), so the emitted order of the source arms can be read
off.  SHOW=1 prints the full listing.  FE_PFX_BASE selects the compiler prefix (default 21)."""
import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))) + '/inline')
import ib
PFX = os.environ.get('FE_PFX_BASE', '21')
HDR = '''#include "probe.h"
int vis2(int);
void call2(int *);
'''
BODY = 'int k = ga[a]; use(k); use(k + b); use(k); '
BIG = ' int k = ga[a]; int m = ga[b]; int n = ga[c]; int loc[4]; loc[0] = k; call2(loc); use(k); use(k + m); use(n + loc[1]); use(m); use(n); use(k); '
REG = ' int loc[4]; loc[0] = a; call2(loc); use(a + b); use(c + loc[1]); if (cond()) { use(a); } use(b + c); use(a); '
V = {
 # --- early returns at the head of a function with a frame of saved registers
 'ret1':        'if (!a) return 0; ' + BODY + 'return 1;',
 'ret2':        'if (!a) return 0; if (a == 10 && g <= 1) return 0; ' + BODY + 'return 1;',
 'ret2_block':  'if (!a) { return 0; } if (a == 10 && g <= 1) { return 0; } ' + BODY + 'return 1;',
 'ret2_else':   'if (!a) return 0; else if (a == 10 && g <= 1) return 0; ' + BODY + 'return 1;',
 'ret2_or':     'if (!a || (a == 10 && g <= 1)) return 0; ' + BODY + 'return 1;',
 'ret2_simple': 'if (!a) return 0; if (a == 10) return 0; ' + BODY + 'return 1;',
 'ret2_diff':   'if (!a) return 0; if (a == 10 && g <= 1) return 2; ' + BODY + 'return 1;',
 'ret3':        'if (!a) return 0; if (a == 10 && g <= 1) return 0; if (a >= 70 && g2 & 2) return 0; ' + BODY + 'return 1;',
 'ret2_call':   'if (!a) return 0; if (a == 10 && cond()) return 0; ' + BODY + 'return 1;',
 'ret2_res':    'int r = 0; if (!a) return r; if (a == 10 && g <= 1) return 0; ' + BODY + 'return 1;',
 'ret2_false':  'if (!a) return false; if (a == 10 && g <= 1) return false; ' + BODY + 'return true;',
 'ret2_mid':    'if (!a) return 0; use(7); if (a == 10 && g <= 1) return 0; ' + BODY + 'return 1;',
 'ret2_many':   'if (!a) return 0; if (a == 10 && g <= 1) return 0; ' + BODY + 'if (k == 4) return 0; use(9); if (k == 5) return 1; use(10); if (g3 < k) return 0; use(11); return 1;',
 # --- the function's last statement returns the same value as the early return
 'end0':        'if (!a) return 0; ' + BODY + 'if (k == 4) return 1; use(9); return 0;',
 'end0_two':    'if (!a) return 0; if (a == 10 && g <= 1) return 0; ' + BODY + 'if (k == 4) return 1; use(9); return 0;',
 'end0_two1':   'if (!a) return 0; if (a == 10) return 0; ' + BODY + 'if (k == 4) return 1; use(9); return 0;',
 'end0_a':      'if (!a) return a; if (a == 10 && g <= 1) return 0; ' + BODY + 'if (k == 4) return 1; use(9); return 0;',
 'end0_else':   'if (!a) return 0; else if (a == 10 && g <= 1) return 0; ' + BODY + 'if (k == 4) return 1; use(9); return 0;',
 'end0_wrap':   'if (a) { if (a == 10 && g <= 1) return 0; ' + BODY + 'if (k == 4) return 1; use(9); } return 0;',
 'end0_pos':    'if (a == 0) return 0; if (a == 10 && g <= 1) return 0; ' + BODY + 'if (k == 4) return 1; use(9); return 0;',
 'end0_false':  'if (!a) return false; if (a == 10 && g <= 1) return false; ' + BODY + 'if (k == 4) return true; use(9); return false;',
 'end0_endif':  'if (!a) return 0; if (a == 10 && g <= 1) return 0; ' + BODY + 'if (k == 4) return 1; use(9); if (cond()) return 1; return 0;',
 'end0_endifz': 'if (!a) return 0; if (a == 10 && g <= 1) return 0; ' + BODY + 'if (k == 4) return 1; use(9); if (!cond()) return 0; return 1;',
 'end0_tern':   'if (!a) return 0; if (a == 10 && g <= 1) return 0; ' + BODY + 'if (k == 4) return 1; use(9); return cond() ? 1 : 0;',
 'end0_ne':     'if (!a) return 0; if (a == 10 && g <= 1) return 0; ' + BODY + 'if (k == 4) return 1; use(9); return cond() != 0;',
 'end0_ifelse': 'if (!a) return 0; if (a == 10 && g <= 1) return 0; ' + BODY + 'if (k == 4) return 1; use(9); if (cond()) { return 1; } else { return 0; }',
 'end0_and':    'if (!a) return 0; if (a == 10 && g <= 1) return 0; ' + BODY + 'if (k == 4) return 1; use(9); if (a == 32 && !cond()) return 0; return 1;',
 'end0_and1':   'if (!a) return 0; if (a == 10 && g <= 1) return 0; ' + BODY + 'if (k == 4) return 1; use(9); if (a == 32 && cond()) return 1; return 0;',
 'end0_r':      'int r = 0; if (!a) return 0; if (a == 10 && g <= 1) return 0; ' + BODY + 'if (k == 4) return 1; use(9); if (a == 32 && cond()) r = 1; return r;',
 # --- the last statement is a conditional `return 0` (its block is the last one emitted), then `return 1`
 'last0':       'if (!a) return 0; if (a == 10 && g <= 1) return 0; ' + BODY + 'if (k == 4) return 1; use(9); if (a == 32 && g4 < 3) return 0; return 1;',
 'last0_3':     'if (!a) return 0; if (a == 10 && g <= 1) return 0; if (a == 11) return 0; ' + BODY + 'if (k == 4) return 1; use(9); if (k == 7) return 0; use(10); if (a == 32 && g4 < 3) return 0; return 1;',
 'last0_nest':  'if (!a) return 0; if (a == 10 && g <= 1) return 0; ' + BODY + 'if (k == 4) return 1; use(9); if (g5) { if (k == 7) return 0; use(10); if (a == 32 && g4 < 3) return 0; } return 1;',
 'last0_else':  'if (!a) return 0; if (a == 10 && g <= 1) return 0; ' + BODY + 'if (k == 4) return 1; use(9); if (g5) { use(12); if (k == 9) return 0; } else { if (k == 7) return 0; use(10); if (a == 32 && g4 < 3) return 0; } return 1;',
 'last0_blk':   'if (!a) { return 0; } if (a == 10 && g <= 1) return 0; ' + BODY + 'if (k == 4) return 1; use(9); if (g5) { if (k == 7) return 0; use(10); if (a == 32 && g4 < 3) return 0; } return 1;',
 'last0_and':   'if (!a && 1) return 0; if (a == 10 && g <= 1) return 0; ' + BODY + 'if (k == 4) return 1; use(9); if (g5) { if (k == 7) return 0; use(10); if (a == 32 && g4 < 3) return 0; } return 1;',
 'last0_neg':   'if (a) { if (a == 10 && g <= 1) return 0; ' + BODY + 'if (k == 4) return 1; use(9); if (g5) { if (k == 7) return 0; use(10); if (a == 32 && g4 < 3) return 0; } return 1; } return 0;',
 'last0_negelse': 'if (a) { if (a == 10 && g <= 1) return 0; ' + BODY + 'if (k == 4) return 1; use(9); if (g5) { if (k == 7) return 0; use(10); if (a == 32 && g4 < 3) return 0; } return 1; } else return 0;',
 'last0_ifelse': 'if (!a) return 0; else { if (a == 10 && g <= 1) return 0; ' + BODY + 'if (k == 4) return 1; use(9); if (g5) { if (k == 7) return 0; use(10); if (a == 32 && g4 < 3) return 0; } return 1; }',
 # --- same, with a frame and three saved registers (a 7-instruction epilogue), as facility_avail 0x005BA0E0
 'big0':        'if (!a) return 0; if (a == 10 && g <= 1) return 0; if (a >= 70 && g2 & 2) return 0;' + BIG + 'if (k == 4) return 1; use(9); if (k == 7) return 0; use(10); if (a == 32 && g4 < 3) return 0; return 1;',
 'big0_else':   'if (!a) return 0; else if (a == 10 && g <= 1) return 0; if (a >= 70 && g2 & 2) return 0;' + BIG + 'if (k == 4) return 1; use(9); if (k == 7) return 0; use(10); if (a == 32 && g4 < 3) return 0; return 1;',
 'big0_c':      'if (!c) return 0; if (a == 10 && g <= 1) return 0; if (a >= 70 && g2 & 2) return 0;' + BIG + 'if (k == 4) return 1; use(9); if (k == 7) return 0; use(10); if (a == 32 && g4 < 3) return 0; return 1;',
 'big0_g':      'if (!g6) return 0; if (a == 10 && g <= 1) return 0; if (a >= 70 && g2 & 2) return 0;' + BIG + 'if (k == 4) return 1; use(9); if (k == 7) return 0; use(10); if (a == 32 && g4 < 3) return 0; return 1;',
 'big0_swap':   'if (a == 10 && g <= 1) return 0; if (!a) return 0; if (a >= 70 && g2 & 2) return 0;' + BIG + 'if (k == 4) return 1; use(9); if (k == 7) return 0; use(10); if (a == 32 && g4 < 3) return 0; return 1;',
 'big0_ret1':   'if (!a) return 0; if (a == 10 && g <= 1) return 0; if (a >= 70 && g2 & 2) return 0;' + BIG + 'if (k == 4) return 1; use(9); if (k == 7) return 0; use(10); if (a == 32 && g4 < 3) return 0; use(11); return 1;',
 # --- a, b, c live in ebx/esi/edi from the entry: every push is up front and the epilogue has seven instructions
 'reg0':        'if (!a) return 0; if (a == 10 && g <= 1) return 0; if (a >= 70 && g2 & 2) return 0;' + REG + 'if (a == 4) return 1; use(9); if (b == 7) return 0; use(10); if (a == 32 && g4 < 3) return 0; return 1;',
 'reg0_tail1':  'if (!a) return 0; if (a == 10 && g <= 1) return 0; if (a >= 70 && g2 & 2) return 0;' + REG + 'if (a == 4) return 1; use(9); if (b == 7) return 0; use(10); if (a == 32 && g4 < 3) return 0; use(11); return 1;',
 'reg0_end0':   'if (!a) return 0; if (a == 10 && g <= 1) return 0; if (a >= 70 && g2 & 2) return 0;' + REG + 'if (a == 4) return 1; use(9); if (b == 7) return 0; use(10); if (a == 32 && g4 < 3) return 1; return 0;',
 'reg0_bool':   'if (!a) return false; if (a == 10 && g <= 1) return false; if (a >= 70 && g2 & 2) return false;' + REG + 'if (a == 4) return true; use(9); if (b == 7) return false; use(10); if (a == 32 && g4 < 3) return false; return true;',
 'reg0_uncond': 'if (!a) return 0; if (a == 10 && g <= 1) return 0; if (a >= 70 && g2 & 2) return 0; if (a == 34) { if (g5 && g6) return 1; return 0; } if (a == 77) return 0;' + REG + 'if (a == 4) return 1; use(9); if (b == 7) return 0; use(10); if (a == 32 && g4 < 3) return 0; return 1;',
 'reg0_expr':   'if (!a) return 0; if (a == 10 && g <= 1) return 0; if (a >= 70 && g2 & 2) return 0; if (a == 34) { return g5 == 3; } if (a == 77) return 0;' + REG + 'if (a == 4) return 1; use(9); if (b == 7) return 0; use(10); if (a == 32 && g4 < 3) return 0; return 1;',
 'reg0_blk':    'if (!a) return 0; if (a == 10 && g <= 1) return 0; if (a >= 70 && g2 & 2) return 0; if (a == 77) return 0;' + REG + 'if (a > 4) { use(9); if (b == 7) return 0; use(10); if (a == 32 && g4 < 3) return 0; } return 1;',
 'reg0_ifelse': 'if (!a) return 0; if (a == 10 && g <= 1) return 0; if (a >= 70 && g2 & 2) return 0; if (a == 77) return 0;' + REG + 'if (a > 40) { use(12); if (c == 9) return 0; use(13); return 1; } else { use(9); if (b == 7) return 0; use(10); if (a == 32 && g4 < 3) return 0; } return 1;',
 # --- if/else arm order: which arm is emitted first, which is moved after the ret
 'ie':          'use(1); if (a) { use(2); use(3); } else { use(4); use(5); } use(6); return 1;',
 'ie_neg':      'use(1); if (!a) { use(4); use(5); } else { use(2); use(3); } use(6); return 1;',
 'ie_ret_then': 'use(1); if (a) { use(2); return 0; } else { use(4); use(5); } use(6); return 1;',
 'ie_ret_else': 'use(1); if (a) { use(2); use(3); } else { use(4); return 0; } use(6); return 1;',
 'ie_ret_noelse': 'use(1); if (a) { use(2); return 0; } use(4); use(5); use(6); return 1;',
 'ie_last':     'use(1); if (a) { use(2); use(3); } else { use(4); use(5); } return 1;',
 'ie_last_ret': 'use(1); if (a) { use(2); use(3); return 1; } else { use(4); use(5); return 2; }',
 'ie_last_ret2': 'use(1); if (a) { use(2); use(3); return 1; } use(4); use(5); return 2;',
 'ie_goto':     'use(1); if (a) { use(2); goto done; } use(4); use(5); done: use(6); return 1;',
 'if_goto_far': 'use(1); if (a) goto away; use(4); use(5); back: use(6); return 1; away: use(2); use(3); goto back;',
 'if_empty_else': 'use(1); if (a) { } else { use(4); use(5); } use(6); return 1;',
 'if_big_then': 'use(1); if (a) { use(2); use(3); use(7); use(8); use(9); use(10); } use(6); return 1;',
 # --- loops: continue / break / exit arms
 'lp_cont':     'for (int i = 0; i < a; i++) { if (ga[i] == b) continue; use(2); use(3); } use(6); return 1;',
 'lp_wrap':     'for (int i = 0; i < a; i++) { if (ga[i] != b) { use(2); use(3); } } use(6); return 1;',
 'lp_break':    'for (int i = 0; i < a; i++) { if (ga[i] == b) break; use(2); use(3); } use(6); return 1;',
 'lp_brk_body': 'for (int i = 0; i < a; i++) { if (ga[i] == b) { use(7); break; } use(2); use(3); } use(6); return 1;',
 'lp_ret_body': 'for (int i = 0; i < a; i++) { if (ga[i] == b) { use(7); return 0; } use(2); use(3); } use(6); return 1;',
 'lp_ret':      'for (int i = 0; i < a; i++) { if (ga[i] == b) return 0; use(2); use(3); } use(6); return 1;',
 'lp_cont_body': 'for (int i = 0; i < a; i++) { if (ga[i] == b) { use(7); continue; } use(2); use(3); } use(6); return 1;',
 'lp_ie':       'for (int i = 0; i < a; i++) { if (ga[i] == b) { use(7); } else { use(2); use(3); } } use(6); return 1;',
 'lp_while':    'int i = 0; while (i < a) { if (ga[i] == b) { use(7); break; } use(2); i++; } use(6); return 1;',
 'lp_do':       'int i = 0; do { if (ga[i] == b) { use(7); break; } use(2); i++; } while (i < a); use(6); return 1;',
 # --- if-chain against switch
 'chain':       'if (a == 1) { use(1); } else if (a == 2) { use(2); } else if (a == 3) { use(3); } else { use(4); } use(6); return 1;',
 'chain_ret':   'if (a == 1) { use(1); return 0; } if (a == 2) { use(2); return 0; } if (a == 3) { use(3); return 0; } use(4); use(6); return 1;',
 'sw':          'switch (a) { case 1: use(1); break; case 2: use(2); break; case 3: use(3); break; default: use(4); break; } use(6); return 1;',
 'sw_ret':      'switch (a) { case 1: use(1); return 0; case 2: use(2); return 0; case 3: use(3); return 0; } use(4); use(6); return 1;',
 'sw_sparse':   'switch (a) { case 1: use(1); break; case 20: use(2); break; case 300: use(3); break; default: use(4); break; } use(6); return 1;',
}
def skeleton(L):
    out = []
    for l in L:
        if l.endswith(':'):
            out.append(l.split('$')[-1][:-1] + ':' if l.startswith('$L') else l)
        elif re.match(r'j\w+\s', l):
            out.append(l.split()[0] + ' ' + l.split('$')[-1])
        elif l.startswith('ret'):
            out.append('RET')
        elif l.startswith('call') and 'use' in l:
            out.append('u' + (out.pop()[1:] if out and out[-1].startswith('#') else '?'))
        elif l.startswith('call'):
            if out and out[-1].startswith('#'): out.pop()
            out.append('call')
        else:
            m = re.match(r'push\s+(\d+)', l)
            if out and out[-1].startswith('#'): out.pop()
            if m: out.append('#' + m.group(1))
            elif re.match(r'(xor\s+eax, eax|mov\s+eax, \d+)$', l): out.append('eax=' + (l.split()[-1] if l.startswith('mov') else '0'))
    return [o for o in out if not o.startswith('#')]
if __name__ == '__main__':
    sel = sys.argv[1:] or list(V)
    src = HDR + ''.join('int %s(int a, int b, int c) {\n%s\n}\n' % (n, V[n]) for n in sel)
    asm, out = ib.compile_asm(None, PFX, text=src)
    F = {f[1]: f[2] for f in ib.functions(asm)}
    for n in sel:
        L = next((l for k, l in F.items() if ('?%s@@' % n) in k), None)
        if L is None:
            print(n, 'MISSING', out[-300:] if out else ''); continue
        L = [l for l in L if not l.startswith(';')]
        print('%-14s %s' % (n, ' '.join(skeleton(L))))
        if os.environ.get('SHOW'):
            print('   ' + ' ; '.join(L))
