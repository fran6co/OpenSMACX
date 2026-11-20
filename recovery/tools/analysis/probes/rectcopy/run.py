#!/usr/bin/env python3
"""run.py [name ...]: when VC6 keeps the stores of a whole-struct copy into a local RECT whose fields are
overwritten before any read (DiploPop::init 0x004407E0, AlphaSave::init, DiploWin::exec; MATCHING_GUIDE 3.3
"A local RECT copy keeps ...").  Each variant is `r = <source>; r.right = r.left - 3; r.left = x; m = r;`
followed by a later use of r; prints the frame stores (`[ebp-N]`) of the first block and the full count.
SHOW=1 prints the listing.  FE_PFX_BASE selects the compiler prefix (default 21)."""
import sys, os, re
H = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(os.path.dirname(H)) + '/inline')
import ib
PFX = os.environ.get('FE_PFX_BASE', '21')
HDR = '''#include "probe.h"
struct R { int left, top, right, bottom; };
struct G { R a, b, c; const R &ra() const { return a; } R va() const { return a; } };
extern G gg;
void ext(R *);
void extc(const R *);
inline void expand(R *r, int h, int v) { r->left -= h; r->right += h; r->top -= v; r->bottom += v; }
inline void offs(R *r, int dx, int dy) { r->left += dx; r->right += dx; r->top += dy; r->bottom += dy; }
inline int rw(const R *r) { return r->right - r->left; }
extern "C" void *memcpy(void *, const void *, unsigned);
#pragma intrinsic(memcpy)
struct W { R m1, m2, m3; void f(); };
'''
X = 'int x = gg.c.right - gg.b.right + gg.c.left; '
BODY = 'r.right = r.left - 3; r.left = x; m1 = r; '
V = {
 # source of the copy, nothing after
 'mem_none':     'R r; r = gg.a; ' + X + BODY,
 'ref_none':     'R r; r = gg.ra(); ' + X + BODY,
 'val_none':     'R r; r = gg.va(); ' + X + BODY,
 'cpy_none':     'R r; memcpy(&r, &gg.a, sizeof r); ' + X + BODY,
 'init_none':    'R r = gg.a; ' + X + BODY,
 # later uses of r (address taken by an out-of-line call)
 'mem_ext':      'R r; r = gg.a; ' + X + BODY + 'r = gg.b; ext(&r); ',
 'ref_ext':      'R r; r = gg.ra(); ' + X + BODY + 'r = gg.b; ext(&r); ',
 'val_ext':      'R r; r = gg.va(); ' + X + BODY + 'r = gg.b; ext(&r); ',
 'cpy_ext':      'R r; memcpy(&r, &gg.a, sizeof r); ' + X + BODY + 'r = gg.b; ext(&r); ',
 # later inline pointer access (no escape)
 'mem_inl':      'R r; r = gg.a; ' + X + BODY + 'r = gg.b; expand(&r, -4, -4); m2 = r; ',
 'ref_inl':      'R r; r = gg.ra(); ' + X + BODY + 'r = gg.b; expand(&r, -4, -4); m2 = r; ',
 'ref_offs':     'R r; r = gg.ra(); ' + X + BODY + 'r = gg.b; offs(&r, 3, 0); m2 = r; ',
 'ref_rw':       'R r; r = gg.ra(); ' + X + BODY + 'r = gg.b; m2.left = rw(&r); ',
 # later member-form use only
 'ref_member':   'R r; r = gg.ra(); ' + X + BODY + 'r = gg.b; r.left -= 4; m2 = r; ',
 # pointer read in the first block itself
 'ref_ptr1':     'R r; r = gg.ra(); ' + X + 'r.right = rw(&r) - 3; r.left = x; m1 = r; ',
 'ref_ptrw':     'R r; r = gg.ra(); ' + X + 'R *p = &r; p->right = p->left - 3; r.left = x; m1 = r; ',
 # escape only (no second copy)
 'ref_esc':      'R r; r = gg.ra(); ' + X + BODY + 'ext(&r); ',
 # copy with no later overwrite, two copies in a row
 'ref_twice':    'R r; r = gg.ra(); ' + X + BODY + 'r = gg.ra(); r.top = x; m2 = r; ',
 # field overwrite order variants
 'ref_lfirst':   'R r; r = gg.ra(); ' + X + 'int l = r.left; r.left = x; r.right = l - 3; m1 = r; ',
 'ref_noread':   'R r; r = gg.ra(); ' + X + 'r.right = x - 3; r.left = x; m1 = r; ',
 # field-wise copy instead of a struct copy, overwritten unread
 'fld_noread':   'R r; r.left = gg.a.left; r.top = gg.a.top; r.right = gg.a.right; r.bottom = gg.a.bottom; ' + X + 'r.right = x - 3; r.left = x; m1 = r; ',
 # a scalar local overwritten unread
 'int_noread':   'int a = gg.a.left; int b = gg.a.right; ' + X + 'a = x; b = x - 3; m1.left = a; m1.right = b; ',
 # read through a pointer only (forwarding blocked) then overwritten
 'ref_pread':    'R r; r = gg.ra(); ' + X + 'R *p = &r; r.right = p->left - 3; r.left = x; m1 = r; ',
 # overwritten twice, unread
 'ref_twice_w':  'R r; r = gg.ra(); ' + X + 'r.right = x - 3; r.right = x - 5; r.left = x; m1 = r; ',
 # the original field read from the source instead of r
 'ref_srcread':  'R r; r = gg.ra(); ' + X + 'r.right = gg.ra().left - 3; r.left = x; m1 = r; ',
 'ref_noread_e': 'R r; r = gg.ra(); ' + X + 'r.right = x - 3; r.left = x; m1 = r; r = gg.b; ext(&r); ',
}


def frame_stores(L):
    return [l for l in L if re.match(r'mov\s+(DWORD PTR )?_?\w*\$?\w*\[ebp', l) or re.match(r'mov\s+DWORD PTR \w*\$\d*\[ebp', l)]


if __name__ == '__main__':
    sel = sys.argv[1:] or list(V)
    src = HDR + ''.join('void W::f_%s() { %s}\n' % (n, V[n]) for n in sel)
    src = src.replace('void f();', 'void f(); ' + ' '.join('void f_%s();' % n for n in sel))
    asm, _ = ib.compile_asm(None, PFX, text=src)
    F = {f[0]: f[2] for f in ib.functions(asm)}
    for n in sel:
        L = [l for k, l in F.items() if ('f_' + n + '@') in k or k.endswith('f_' + n)]
        if not L:
            print(n, 'not found', list(F)[:4]); continue
        L = L[0]
        st = [l for l in L if l.startswith('mov') and '[ebp' in l.split(',')[0]]
        print('%-13s %2d frame stores: %s' % (n, len(st), ' ; '.join(s.split(',')[0].replace('mov', '').replace('DWORD PTR', '').strip() for s in st)))
        if os.environ.get('SHOW'):
            print('   ' + '\n   '.join(L))
