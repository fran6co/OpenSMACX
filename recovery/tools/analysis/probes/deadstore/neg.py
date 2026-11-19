#!/usr/bin/env python3
"""neg.py: `neg esi; mov eax, esi; push eax` (SocialWin::draw_info 0x004B0289): which spelling of
`_itoa(-u, buf, 10)` negates the variable in place and then copies it to a scratch register?
u lives in a callee-saved register across calls (loaded from a call result before a branch).
Usage: uv run python neg.py [name ...]   (FE_PFX_BASE, default 21)"""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))) + '/inline')
import ib
PFX = os.environ.get('FE_PFX_BASE', '21')
HDR = '''#include "probe.h"
char *it(int, char *, int);
void sc(char *);
inline void put_num(int v, char *b) { it(v, b, 10); sc(b); }
'''
PRE = 'char buf[16]; int u = cond(); call(); if (u < 0) { call(); '
POST = ' } else if (u > 0) { it(u, buf, 10); sc(buf); } call(); '
V = {
 'expr': 'it(-u, buf, 10); sc(buf);',
 'assign': 'u = -u; it(u, buf, 10); sc(buf);',
 'local': 'int v = -u; it(v, buf, 10); sc(buf);',
 'inline': 'put_num(-u, buf);',
 'neg_then_inline': 'u = -u; put_num(u, buf);',
 'fnscope': 'int v; v = -u; it(v, buf, 10); sc(buf);',
}
FUNCS = dict((n, 'void f_%s() { %s%s%s }\n' % (n, PRE, b, POST)) for n, b in V.items())
FUNCS['fs_shared'] = 'void f_fs_shared() { char buf[16]; int v; int u = cond(); call(); if (u < 0) { call(); v = -u; it(v, buf, 10); sc(buf); } else if (u > 0) { v = u; it(v, buf, 10); sc(buf); } call(); }\n'
if __name__ == '__main__':
    sel = sys.argv[1:] or list(FUNCS)
    asm, _ = ib.compile_asm(None, PFX, text=HDR + ''.join(FUNCS[n] for n in sel))
    F = {f[0].split('(')[0].strip(): f[2] for f in ib.functions(asm)}
    for n in sel:
        L = [l for k, l in F.items() if k.endswith('f_' + n)][0]
        i = next(k for k, l in enumerate(L) if l.startswith('neg'))
        print('%-16s | %s' % (n, ' ; '.join(L[i - 1:i + 6])))
