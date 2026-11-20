#!/usr/bin/env python3
"""run.py [name ...]: which source leaves a dead `and r,K` beside a live mask test (spot_all 0x005B69FC
`mov edx,ecx; and ecx,0x80002000; and edx,0x80001000; cmp ecx,0x2000`, edx never read). Prints DEAD when a
second `and` with 0x80001000 survives, and the callee-saved registers. SHOW=name prints the listing."""
import sys, os, re
H = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(os.path.dirname(H)) + '/inline')
import ib
PFX = os.environ.get('FE_PFX_BASE', '21')
HDR = '''#include "probe.h"
extern unsigned tb[]; extern int cnt, cnt2, gk;
'''
V = {
 'base':      'unsigned v = tb[a]; if ((v & 0x80002000) == 0x2000) cnt++; return 0;',
 'else_dead': 'unsigned v = tb[a]; int d = 0; if ((v & 0x80002000) == 0x2000) cnt++; else if ((v & 0x80001000) == 0x1000) d++; return 0;',
 'else_glob': 'unsigned v = tb[a]; if ((v & 0x80002000) == 0x2000) cnt++; else if ((v & 0x80001000) == 0x1000) cnt2++; return 0;',
 'and_glob':  'unsigned v = tb[a]; if ((v & 0x80002000) == 0x2000 && (v & 0x80001000) == 0x1000) cnt++; return 0;',
 'or_glob':   'unsigned v = tb[a]; if ((v & 0x80002000) == 0x2000 || (v & 0x80001000) == 0x1000) cnt++; return 0;',
 'kfold':     'unsigned v = tb[a]; int k = 2; if ((v & 0x80002000) == 0x2000) cnt++; if (k != 2 && (v & 0x80001000) == 0x1000) cnt2++; return 0;',
 'kcase':     'unsigned v = tb[a]; switch (b) { case 2: if ((v & 0x80002000) == 0x2000) cnt++; if (b != 2 && (v & 0x80001000) == 0x1000) cnt2++; } return 0;',
 'loc_m':     'unsigned v = tb[a]; unsigned m = v & 0x80001000; if ((v & 0x80002000) == 0x2000) cnt++; if (b == 3 && b == 4 && m == 0x1000) cnt2++; return 0;',
 'loop_k':    'for (int i = 0; i < b; i++) { unsigned v = tb[i]; if ((v & 0x80002000) == 0x2000) cnt++; if (i < 0 && (v & 0x80001000) == 0x1000) cnt2++; } return 0;',
 'tern':      'unsigned v = tb[a]; cnt += (v & 0x80002000) == 0x2000 ? 1 : ((v & 0x80001000) == 0x1000 ? 0 : 0); return 0;',
'same_arms': 'unsigned v = tb[a]; if ((v & 0x80002000) == 0x2000) cnt++; if ((v & 0x80001000) == 0x1000) use(1); else use(1); return 0;',
 'same_st':   'unsigned v = tb[a]; if ((v & 0x80002000) == 0x2000) cnt++; if ((v & 0x80001000) == 0x1000) cnt2 = 1; else cnt2 = 1; return 0;',
 'goto_nx':   'unsigned v = tb[a]; if ((v & 0x80002000) == 0x2000) cnt++; if ((v & 0x80001000) == 0x1000) goto nx; nx: use(2); return 0;',
 'cont_same': 'for (int i = 0; i < b; i++) { unsigned v = tb[i]; if ((v & 0x80002000) == 0x2000) cnt++; if ((v & 0x80001000) == 0x1000) continue; } return 0;',
 'cont_tail': 'for (int i = 0; i < b; i++) { unsigned v = tb[i]; if ((v & 0x80002000) == 0x2000) { cnt++; } else if ((v & 0x80001000) == 0x1000) { continue; } use(i); } return 0;',
 'ret_same':  'unsigned v = tb[a]; if ((v & 0x80002000) == 0x2000) cnt++; if ((v & 0x80001000) == 0x1000) return 0; return 0;',
 'brk_same':  'for (int i = 0; i < b; i++) { unsigned v = tb[i]; if ((v & 0x80002000) == 0x2000) cnt++; if ((v & 0x80001000) == 0x1000) { use(i); continue; } use(i); } return 0;',
 'sel':       'unsigned v = tb[a]; int t = (v & 0x80001000) == 0x1000; if ((v & 0x80002000) == 0x2000) cnt++; if (t && 0) cnt2++; return 0;',
}


def main(names):
    src = HDR + ''.join('int v_%s(int a, int b) {\n%s\n}\n' % (k, v) for k, v in V.items() if not names or k in names)
    asm, _ = ib.compile_asm(None, PFX, text=src)
    show = os.environ.get('SHOW')
    for title, sym, lines in ib.functions(asm):
        nm = re.sub(r'^\?|@.*$', '', sym).lstrip('_')[2:]
        t = '\n'.join(lines)
        dead = re.search(r'and\s+e\w\w, -2147479552', t) and not re.search(r'cmp\s+e\w\w, 4096', t)
        print('%-10s %s' % (nm, 'DEAD' if dead else ('and+cmp' if re.search(r'and\s+e\w\w, -2147479552', t) else '-')))
        if show == nm or show == 'all':
            print('\n'.join('   ' + l for l in lines))


if __name__ == '__main__':
    main(sys.argv[1:])
