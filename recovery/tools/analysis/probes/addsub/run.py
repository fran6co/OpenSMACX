#!/usr/bin/env python3
"""run.py: when does VC6 keep `(b - t) + t - 1` unsimplified (`sub r,t; lea r,[r+t-1]`, GraphicWin::fill
0x005D5350) instead of folding it to `b - 1` (`dec`)?  Each variant passes left/top/right-1/bottom-1 of a
RECT parameter to an external call, after an earlier use of the same quantities.
Usage: uv run python run.py [name ...]   (FE_PFX_BASE, default 21)"""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))) + '/inline')
import ib
PFX = os.environ.get('FE_PFX_BASE', '21')
HDR = '''#include "probe.h"
struct R { int left, top, right, bottom; };
void cp(int, int, int, int);
void mp(int, int, int, int, int);
extern int TBL;
inline void mp_wh(int x, int y, int w, int h, int t) { mp(x, y, x + w - 1, y + h - 1, t); }
inline void mp_r(R *r, int t) { mp(r->left, r->top, r->right - 1, r->bottom - 1, t); }
inline int rw(R *r) { return r->right - r->left; }
inline int rh(R *r) { return r->bottom - r->top; }
'''
V = {
 'rect_lt': 'cp(a->left, a->top, a->right - a->left, a->bottom - a->top); if (TBL) { R r; r.left = a->left; r.top = a->top; mp(r.left, r.top, r.left + (a->right - a->left) - 1, r.top + (a->bottom - a->top) - 1, TBL); }',
 'rect_rb': 'cp(a->left, a->top, a->right - a->left, a->bottom - a->top); if (TBL) { R r; r.left = a->left; r.top = a->top; r.right = r.left + (a->right - a->left); r.bottom = r.top + (a->bottom - a->top); mp(r.left, r.top, r.right - 1, r.bottom - 1, TBL); }',

 'rcopy': 'cp(a->left, a->top, a->right - a->left, a->bottom - a->top); if (TBL) { R r = *a; mp(r.left, r.top, r.left + (r.right - r.left) - 1, r.top + (r.bottom - r.top) - 1, TBL); }',
 'rdims': 'cp(a->left, a->top, a->right - a->left, a->bottom - a->top); if (TBL) { R r; r.left = a->left; r.top = a->top; r.right = a->right - a->left; r.bottom = a->bottom - a->top; mp(r.left, r.top, r.left + r.right - 1, r.top + r.bottom - 1, TBL); }',
 'arr': 'cp(a->left, a->top, a->right - a->left, a->bottom - a->top); if (TBL) { int v[4]; v[0] = a->left; v[1] = a->top; v[2] = a->right - a->left; v[3] = a->bottom - a->top; mp(v[0], v[1], v[0] + v[2] - 1, v[1] + v[3] - 1, TBL); }',

 'form_a_lw': 'cp(a->left, a->top, a->right - a->left, a->bottom - a->top); if (TBL) { int t = a->top; int h = a->bottom - t; int l = a->left; int w = a->right - l; mp(l, t, l + w - 1, t + h - 1, TBL); }',
 'form_a_wl': 'cp(a->left, a->top, a->right - a->left, a->bottom - a->top); if (TBL) { int t = a->top; int h = a->bottom - t; int l = a->left; int w = a->right - l; mp(l, t, w + l - 1, h + t - 1, TBL); }',
 'form_b_lw': 'cp(a->left, a->top, a->right - a->left, a->bottom - a->top); if (TBL) { int t = a->top; int h = a->bottom - t; int l = a->left; int w = a->right - l; mp(l, t, l + (w - 1), t + (h - 1), TBL); }',
 'form_b_wl': 'cp(a->left, a->top, a->right - a->left, a->bottom - a->top); if (TBL) { int t = a->top; int h = a->bottom - t; int l = a->left; int w = a->right - l; mp(l, t, w + (l - 1), h + (t - 1), TBL); }',
 'form_c_lw': 'cp(a->left, a->top, a->right - a->left, a->bottom - a->top); if (TBL) { int t = a->top; int h = a->bottom - t; int l = a->left; int w = a->right - l; mp(l, t, (l - 1) + w, (t - 1) + h, TBL); }',
 'form_c_wl': 'cp(a->left, a->top, a->right - a->left, a->bottom - a->top); if (TBL) { int t = a->top; int h = a->bottom - t; int l = a->left; int w = a->right - l; mp(l, t, (w - 1) + l, (h - 1) + t, TBL); }',
 'form_d_lw': 'cp(a->left, a->top, a->right - a->left, a->bottom - a->top); if (TBL) { int t = a->top; int h = a->bottom - t; int l = a->left; int w = a->right - l; mp(l, t, l - 1 + w, t - 1 + h, TBL); }',
 'form_d_wl': 'cp(a->left, a->top, a->right - a->left, a->bottom - a->top); if (TBL) { int t = a->top; int h = a->bottom - t; int l = a->left; int w = a->right - l; mp(l, t, w - 1 + l, h - 1 + t, TBL); }',
 'form_e_lw': 'cp(a->left, a->top, a->right - a->left, a->bottom - a->top); if (TBL) { int t = a->top; int h = a->bottom - t; int l = a->left; int w = a->right - l; mp(l, t, l + w + -1, t + h + -1, TBL); }',
 'form_e_wl': 'cp(a->left, a->top, a->right - a->left, a->bottom - a->top); if (TBL) { int t = a->top; int h = a->bottom - t; int l = a->left; int w = a->right - l; mp(l, t, w + l + -1, h + t + -1, TBL); }',

 'defs_before_if': 'cp(a->left, a->top, a->right - a->left, a->bottom - a->top); int t = a->top; int h = a->bottom - t; int l = a->left; int w = a->right - l; if (TBL) { mp(l, t, l + w - 1, t + h - 1, TBL); }',
 'defs_before_tbl': 'cp(a->left, a->top, a->right - a->left, a->bottom - a->top); int t = a->top; int h = a->bottom - t; int l = a->left; int w = a->right - l; int tb = TBL; if (tb) { mp(l, t, l + w - 1, t + h - 1, tb); }',
 'loop': 'cp(a->left, a->top, a->right - a->left, a->bottom - a->top); int t = a->top; int h = a->bottom - t; for (int i = 0; i < 4; i++) mp(i, t, i, t + h - 1, TBL);',

 'single': 'cp(a->left, a->top, a->right - a->left, a->bottom - a->top); if (TBL) { int t = a->top; int h = a->bottom - t; int l = a->left; int w = a->right - l; mp(l, t, l + w - 1, t + h - 1, TBL); }',
 'reuse': 'int l, t, w, h; t = a->top; l = a->left; w = a->right - l; h = a->bottom - t; cp(l, t, w, h); if (TBL) { t = a->top; h = a->bottom - t; l = a->left; w = a->right - l; mp(l, t, l + w - 1, t + h - 1, TBL); }',
 'reuse_noreload': 'int l, t, w, h; t = a->top; l = a->left; w = a->right - l; h = a->bottom - t; cp(l, t, w, h); if (TBL) { mp(l, t, l + w - 1, t + h - 1, TBL); }',
 'inl_wh': 'cp(a->left, a->top, a->right - a->left, a->bottom - a->top); if (TBL) mp_wh(a->left, a->top, a->right - a->left, a->bottom - a->top, TBL);',
 'inl_rwrh': 'cp(a->left, a->top, rw(a), rh(a)); if (TBL) mp(a->left, a->top, a->left + rw(a) - 1, a->top + rh(a) - 1, TBL);',
 'inline_expr': 'cp(a->left, a->top, a->right - a->left, a->bottom - a->top); if (TBL) mp(a->left, a->top, a->left + (a->right - a->left) - 1, a->top + (a->bottom - a->top) - 1, TBL);',
 'twouse': 'cp(a->left, a->top, a->right - a->left, a->bottom - a->top); if (TBL) { int t = a->top; int h = a->bottom - t; int l = a->left; int w = a->right - l; mp(l, t, l + w - 1, t + h - 1, TBL); use(w); use(h); }',
 'unsigned': 'cp(a->left, a->top, a->right - a->left, a->bottom - a->top); if (TBL) { int t = a->top; unsigned h = a->bottom - t; int l = a->left; unsigned w = a->right - l; mp(l, t, l + w - 1, t + h - 1, TBL); }',
}
if __name__ == '__main__':
    sel = sys.argv[1:] or list(V)
    src = HDR + ''.join('void f_%s(R *a) { %s }\n' % (n, V[n]) for n in sel)
    asm, _ = ib.compile_asm(None, PFX, text=src)
    F = {f[0].split('(')[0].strip(): f[2] for f in ib.functions(asm)}
    for n in sel:
        L = [l for k, l in F.items() if k.endswith('f_' + n)][0]
        i = next(k for k, l in enumerate(L) if 'cp@' in l)
        seq = ' ; '.join(l for l in L[i + 1:] if not l.startswith(('pop', 'ret')))
        kept = any(l.startswith('lea') and '-1]' in l.replace(' ', '') for l in L)
        print('%-15s kept=%d | %s' % (n, kept, seq[:300]))
