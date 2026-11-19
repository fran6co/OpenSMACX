#!/usr/bin/env python3
"""run.py: layout of a sparse VC6 switch (on_key_click 0x005178C0 final switch shape).
Cases 0x61/0x23 (a2 = 4 then the shared direction body MD), seven direction stubs (a2 = K; goto MD)
in two jump tables (0x21..0x28, 0x62..0x69), four pan cases in a byte-indexed table
(0x10026..0x10068) plus the direct 0x10025, and the post-switch join J. Every block carries a
marker call mk(N); the layout is printed as the sequence of markers and tree nodes:
  T=jump table, N:K=compare node with constant K, 4..7=stubs (a2), 10=MD head (arrow test),
  11=arrow_left, 12=arrow_right, 13=MD rest, 14=MD edit path, 15=move_cursor,
  20/21/22/23=pans 0x10026/0x10028/0x10025/0x10027 (s=set_center), 30=J head, 31=veh_key,
  32=reset_turn_view tail, 33=cursor_key.
Image (0x51B081..0x51B66F): N:97 T 11 N:65573 T stubs N:66 T 20 s 30 31 21 22 23 12 10 13 14 15 32 33
Usage: uv run python run.py [--show KEY]   (FE_PFX_BASE, default 14)"""
import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))) + '/inline')
import ib
PFX = os.environ.get('FE_PFX_BASE', '14')
HDR = r'''#include "probe.h"
extern void set_center(int, int, int);
extern int edit_lock();
extern int g_cx, g_cy, g_f1, g_f2, g_f3, g_sel, g_cur, g_lat;
extern int rng(int, int, int);
'''
MD = '''            if (mode != 0 && (raw & 0x10000) != 0) {
                if (a2 == 5) mk(11);
                else if (a2 == 1) mk(12);
                return 1;
            }
            mk(10);
            if (g_cur != 0) {
                if (edit_lock() == 0) {
                    mk(13);
                    if (g_sel < 0) return 1;
                    use(a2);
                    return 1;
                }
                mk(14);
            }
            mk(15); use(a2);
            return 1;
'''
STUBS = [('0x21', '0x69', 0), ('0x27', '0x66', 1), ('0x22', '0x63', 2), ('0x28', '0x62', 3),
         ('0x25', '0x64', 5), ('0x24', '0x67', 6), ('0x26', '0x68', 7)]
def stubs(target='md'):
    return ''.join('        case %s:\n        case %s:\n            a2 = %d;\n            goto %s;\n' % (a, b, k, target) for a, b, k in STUBS)
PAN = {
 '26': '        case 0x10026:\n        case 0x10068:\n            mk(20); y = g_cy - rng(g_f1 / 3, 0, g_f2);\n            if (y != g_cy) set_center(g_cx, y, 1);\n            break;\n',
 '28': '        case 0x10028:\n        case 0x10062:\n            mk(21); y = rng(g_f1 / 3, 0, g_lat - g_f3) + g_cy;\n            if (y != g_cy) set_center(g_cx, y, 1);\n            break;\n',
 '25': '        case 0x10025:\n        case 0x10064:\n            mk(22); x = g_cx - g_f3 / 3;\n            if (x != g_cx) set_center(x, g_cy, 1);\n            break;\n',
 '27': '        case 0x10027:\n        case 0x10066:\n            mk(23); x = g_f3 / 3 + g_cx;\n            if (x != g_cx) set_center(x, g_cy, 1);\n            break;\n',
}
J = '''    mk(30);
    if (g_cur != 0) {
        if (edit_lock() == 0) {
            if (g_sel >= 0) mk(31);
            return 1;
        }
        mk(32);
    }
    mk(33);
    return 1;
'''
PRE = '''int t_%s(int a1, int a2) {
    int x, y;
    const int raw = a2;
    const int mode = raw & 0x20000;
    int key = raw;
    if (mode != 0) key &= ~0x30000;
'''
def fn(name, sw='key', order=('H', 'S', 'P'), pans=('26', '28', '25', '27'), extra_pre='', post=None):
    G = {'H': '        case 0x61:\n        case 0x23:\n            a2 = 4;\n        md:\n' + MD, 'S': stubs(), 'P': ''.join(PAN[p] for p in pans)}
    s = PRE % name + extra_pre + '    switch (%s) {\n' % sw + ''.join(G[g] for g in order) + '    }\n'
    return s + (post if post is not None else J) + '}\n'
def layout(lines):
    out = []
    for i, l in enumerate(lines):
        m = re.match(r'push\s+(\d+)$', l)
        if m and i + 1 < len(lines) and '?mk@@' in lines[i + 1]: out.append(m.group(1))
        elif l.startswith('jmp') and 'DWORD PTR' in l: out.append('T')
        elif re.match(r'cmp\s+e\w+, (\d+)$', l) and int(re.match(r'cmp\s+e\w+, (\d+)$', l).group(1)) > 8:
            out.append('N:%s' % re.match(r'cmp\s+e\w+, (\d+)$', l).group(1))
        elif '?set_center@@' in l: out.append('s')
    return ' '.join(out)
V = {}
def add(key, **kw): V[key] = fn('v_' + re.sub(r'\W', '_', key), **kw)
add('base')
add('HPS', order=('H', 'P', 'S'))
add('SPH', order=('S', 'P', 'H'))
add('unsigned', sw='(unsigned)key')
if __name__ == '__main__':
    show = sys.argv[sys.argv.index('--show') + 1] if '--show' in sys.argv else None
    import importlib
    for extra in sys.argv[1:]:
        if extra.endswith('.py'): exec(open(extra).read())
    src = HDR + ''.join(V.values())
    asm, _ = ib.compile_asm(None, PFX, text=src)
    F = {f[0].split('(')[0].strip(): f[2] for f in ib.functions(asm)}
    for k in V:
        lines = F['t_v_' + re.sub(r'\W', '_', k)]
        print('%-14s %s' % (k, layout(lines)))
        if show == k: print('\n'.join(lines))
