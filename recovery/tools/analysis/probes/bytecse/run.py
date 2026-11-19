#!/usr/bin/env python3
"""run.py: what keeps VC6 from CSE-ing the sign extension of a byte held in a register
(MATCHING_GUIDE 3.7 "Byte held once").  Image shape (Console::set_ai 0x00514F50):
    mov cl,[G]; movsx eax,cl; and eax,1; mov [d0],eax; movsx eax,cl; and eax,2; shr eax,1; ...
Each variant writes the four-flag unpack after a call; prints the body.
Usage: uv run python run.py [name ...]   (FE_PFX_BASE, default 21)"""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))) + '/inline')
import ib
PFX = os.environ.get('FE_PFX_BASE', '21')

HDR = '''#include "probe.h"
struct P { int a, b, c, d; };
extern P Pl[];
extern int GI;
extern char GC;
extern signed char GS;
extern char GA[];
union U { int i; char c; }; extern U GU;
struct BF { signed char a:1; signed char b:1; signed char c:1; signed char d:1; }; extern BF GB;
struct CS { char c; }; extern CS GCS; extern CS *GCP;
inline int bit(char c, unsigned m, int s) { return (c & m) >> s; }
inline int bits(char c, int s) { return (c >> s) & 1; }
'''
V = {}
def body(expr_fn, pre=''):
    return pre + ''.join('d->%s = %s;\n' % (f, expr_fn(i)) for i, f in enumerate('abcd'))
def mkd(name, pre, e):
    V[name] = 'void f_%s(int id) { if (cond() >= 0) { %s%s call(); } }\n' % (name, pre, ''.join('Pl[id].%s = %s;\n' % (f, e(i)) for i, f in enumerate('abcd')))
def mkt(name, pre, e, post='if (s & 0x20) g = 1; if (s & 0x100) g2 = 1;'):
    V[name] = 'void f_%s(int id) { if (cond() >= 0) { %s%s %s call(); } }\n' % (name, pre, ''.join('Pl[id].%s = %s;\n' % (f, e(i)) for i, f in enumerate('abcd')), post)
def mk(name, pre, e):
    V[name] = 'void f_%s(int id) { P *d = &Pl[id]; if (cond() >= 0) { %s call(); } }\n' % (name, body(e, pre))

masks = lambda v: (lambda i: '(%s & %du) >> %d' % (v, 1 << i, i) if i else '%s & 1u' % v)
masks_s = lambda v: (lambda i: '(%s & %d) >> %d' % (v, 1 << i, i) if i else '%s & 1' % v)
mk('castG', '', masks('(char)GI'))
mk('localchar', 'char c = (char)GI;', masks('c'))
mk('localchar_s', 'char c = (char)GI;', masks_s('c'))
mk('localschar', 'signed char c = (signed char)GI;', masks('c'))
mk('globchar', '', masks('GC'))
mk('globchar_loc', 'char c = GC;', masks('c'))
mk('arr0', '', masks('GA[0]'))
mk('union', '', masks('GU.c'))
mk('union_loc', 'char c = GU.c;', masks('c'))
mk('structptr', '', masks('GCP->c'))
mk('structptr_loc', 'CS *p = GCP;', masks('p->c'))
mk('inl', 'char c = (char)GI;', lambda i: 'bit(c, %du, %d)' % (1 << i, i))
mk('inl_g', '', lambda i: 'bit((char)GI, %du, %d)' % (1 << i, i))
mk('inl2', 'char c = (char)GI;', lambda i: 'bits(c, %d)' % i)
mk('bitf', '', lambda i: 'GB.%s' % 'abcd'[i])
mk('int_loc', 'int c = (char)GI;', masks('c'))
mk('intcast', 'char c = (char)GI;', masks('(int)c'))
mk('char_ref', 'char &c = GC;', masks('c'))
mk('intloc_cast', 'int v = GI;', masks('(char)v'))
mk('intloc_cast_s', 'int v = GI;', masks_s('(char)v'))
mk('uintloc_cast', 'unsigned v = GI;', masks('(char)v'))
mk('intloc_inl', 'int v = GI;', lambda i: 'bit((char)v, %du, %d)' % (1 << i, i))
mk('param_cast', '', masks('(char)id'))
mkd('D_castG', '', masks('(char)GI'))
mkd('D_castG_s', '', masks_s('(char)GI'))
mkd('D_globchar', '', masks('GC'))
mkd('D_ne', '', lambda i: '((char)GI & %d) >> %d != 0' % (1 << i, i) if i else '((char)GI & 1) != 0')
mkd('D_addrcast', '', masks('*(char *)&GI'))
mkd('D_addrcast_loc', 'char *p = (char *)&GI;', masks('*p'))
mkd('D_charref', 'char &c = (char &)GI;', masks('c'))
mkd('D_union', '', masks('GU.c'))
mkd('D_arr', '', masks('GA[0]'))
mkd('D_bitf', '', lambda i: 'GB.%s' % 'abcd'[i])
mkd('L_call', 'char c = (char)cond();', masks('c'))
mkd('L_merge', 'char c = g ? GC : (char)GI;', masks('c'))
mkd('L_store', 'char c = (char)GI; GC = c;', masks('c'))
mkd('L_storeafter', 'char c = (char)GI;', masks('c'))
mkd('L_uchar', 'unsigned char c = (unsigned char)GI;', masks('c'))
mkd('L_ccall', 'char c = (char)GI; use(c);', masks('c'))
mkt('T_intcast', 'int s = GI;', masks('(char)s'))
mkt('T_intcast_ne', 'int s = GI;', lambda i: '(((char)s & %d) >> %d) != 0' % (1 << i, i))
mkt('T_uintcast', 'unsigned s = GI;', masks('(char)s'))
mkt('T_charloc', 'int s = GI; char c = (char)s;', masks('c'))
mkt('T_glob', '', masks('(char)GI'), 'if (GI & 0x20) g = 1; if (GI & 0x100) g2 = 1;')
mkd('D_inl_g', '', lambda i: 'bit((char)GI, %du, %d)' % (1 << i, i))
mkd('D_inl_c', '', lambda i: 'bit(GC, %du, %d)' % (1 << i, i))
mkd('D_inl_loc', 'char c = (char)GI;', lambda i: 'bit(c, %du, %d)' % (1 << i, i))
mk('vol', 'volatile char c = (char)GI;', masks('c'))

if __name__ == '__main__':
    sel = sys.argv[1:] or list(V)
    src = HDR + ''.join(V[n] for n in sel)
    asm, _ = ib.compile_asm(None, PFX, text=src)
    F = {f[0].split('(')[0].strip(): f[2] for f in ib.functions(asm)}
    for n in sel:
        L = [l for k, l in F.items() if k.endswith('f_' + n)]
        L = L[0] if L else []
        i = next((k for k, l in enumerate(L) if 'cond' in l), 0)
        seq = ' ; '.join(l for l in L[i + 1:] if not l.startswith(('pop', 'ret', 'push e', 'mov esp')))
        print('%-14s movsx=%d | %s' % (n, sum('movsx' in l for l in L), seq[:400]))
