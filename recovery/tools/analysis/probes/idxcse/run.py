#!/usr/bin/env python3
"""run.py [name ...]: when VC6 CSEs a struct-array element address as a byte offset (`shl r,2; [r+tab]`) or an
address (`lea r,[i*4+tab]; [r]`) against keeping the scaled row index (`lea r,[v+v*2]; lea r,[v+r*4]` = v*13 and
`[r*4+tab]` at each use) (MATCHING_GUIDE 2.7, "Index or address CSE"). Each variant is one function; prints the
per-use addressing (B = byte-offset base [r+tab], X = scaled index [r*4+tab], A = address [r] / [r+k]),
the count of v*13 computations (M) and `shl` (S).
SHOW=name prints that variant's listing.  FE_PFX_BASE selects the compiler prefix (default 21)."""
import sys, os, re
H = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(os.path.dirname(H)) + '/inline')
import ib
PFX = os.environ.get('FE_PFX_BASE', '21')
HDR = '''#include "probe.h"
struct V { int a; int b; short proto; unsigned char fac; char pad[41]; };   // 52 bytes: index *13, scale 4
extern V vs[]; extern V ws[]; extern int ga[], gb[];
struct R { int x[3]; };                     // 12-byte rows: *3, scale 4
extern R rows[]; extern int r2[][3];
struct P { int dip[8]; int integ; int pad[1414]; };   // big stride
extern P pd[];
extern int G; extern int GV;
int hx(int, int);
inline int at3(int *t, int r, int k) { return t[r * 3 + k]; }
inline int vfac(int v) { return vs[v].fac; }
'''
def FILL(n):
    return ''.join('for (int k%d = 0; k%d < b; k%d++) use(ga[k%d] + %d); ' % (i, i, i, i, i) for i in range(n))
def FILLS(n):
    return ''.join('if (ga[%d] == b) use(%d); ' % (i, i) for i in range(n))
V = {
 # gen_terrain_poly 0x463888 shape: three uses, call between 2 and 3
 'gtp':        'int v = G; if (vs[v].fac == a && hx(vs[v].proto, 2)) use(vs[v].proto); return 0;',
 'gtp_glob':   'if (vs[G].fac == a && hx(vs[G].proto, 2)) use(vs[G].proto); return 0;',
 'gtp_glob2':  'if (vs[G].fac == a) { int v = G; if (hx(vs[v].proto, 2)) use(vs[v].proto); } return 0;',
 'gtp_vlive':  'int v = G; if (vs[v].fac == a && hx(vs[v].proto, 2)) use(vs[v].proto); use(v); return 0;',
 'gtp_inl':    'int v = G; if (vfac(v) == a && hx(vs[v].proto, 2)) use(vs[v].proto); return 0;',
 'gtp_same':   'int v = G; if (vs[v].proto == a && hx(vs[v].proto, 2)) use(vs[v].proto); return 0;',
 'gtp_nocall': 'int v = G; if (vs[v].fac == a && vs[v].proto == b) use(vs[v].proto); return 0;',
 'gtp_ptr':    'V *p = &vs[G]; if (p->fac == a && hx(p->proto, 2)) use(p->proto); return 0;',
 'gtp_2':      'int v = G; if (vs[v].fac == a) use(vs[v].proto); return 0;',
 'gtp_2b':     'int v = G; if (vs[v].proto == a) use(vs[v].fac); return 0;',
 'gtp_2s':     'int v = G; use(vs[v].fac); use(vs[v].proto); return 0;',
 'gtp_2p':     'int v = a; use(vs[v].fac); use(vs[v].proto); return 0;',
 'gtp_2pp':    'use(vs[a].fac); use(vs[a].proto); return 0;',
 'gtp_2pf':    'use(vs[a].fac); use(vs[a].fac + 1); return 0;',
 # make_gift: int matrix row element, load then store
 'mg_sub':     'if (pd[a].dip[b] < pd[a].integ) pd[a].dip[b]++; return 0;',
 'mg_ref':     'int &c = pd[a].dip[b]; if (c < pd[a].integ) c++; return 0;',
 'mg_sub_call':'if (pd[a].dip[b] < pd[a].integ || hx(a, 0)) pd[a].dip[b]++; return 0;',
 'mg_ref_call':'int &c = pd[a].dip[b]; if (c < pd[a].integ || hx(a, 0)) c++; return 0;',
 # flat int arrays with a row index (base_build a_3c4/a_2ec, 0x4FA13E)
 'fl_g2':      'use(ga[a * 3 + 2]); use(gb[a * 3 + 2]); return 0;',
 'fl_g2s':     'return ga[a * 3 + 2] + gb[a * 3 + 2];',
 'fl_l2':      'int x[54], y[54]; hx((int)x, (int)y); use(x[a * 3 + 2]); use(y[a * 3 + 2]); return 0;',
 'fl_l2s':     'int x[54], y[54]; hx((int)x, (int)y); return x[a * 3 + 2] + y[a * 3 + 2];',
 'fl_l2st':    'int x[54], y[54]; hx((int)x, (int)y); int p = x[a * 3 + 2]; int q = y[a * 3 + 2]; use(p); return p + q;',
 'fl_l1':      'int x[54]; hx((int)x, 0); use(x[a * 3 + 2]); use(x[a * 3 + 1]); return 0;',
 'fl_l1same':  'int x[54]; hx((int)x, 0); use(x[a * 3 + 2]); use(x[a * 3 + 2] + 1); return 0;',
 'fl_2d':      'int x[18][3], y[18][3]; hx((int)x, (int)y); return x[a][2] + y[a][2];',
 'fl_2d_k':    'int x[18][3], y[18][3]; hx((int)x, (int)y); return x[a][2] + y[a][1];',
 'fl_g2k':     'use(ga[a * 3 + 2]); use(gb[a * 3 + 1]); return 0;',
 'fl_gsame':   'use(ga[a * 3 + 2]); use(ga[a * 3 + 2] * 2); return 0;',
 'st_2':       'return vs[a].a + ws[a].a;',
 'st_2k':      'return vs[a].a + ws[a].b;',
 'st_1k':      'return vs[a].a + vs[a].b;',
 'st_1same':   'return vs[a].a + vs[a].a * b;',
 'fl_nv':      'int x[54], y[54]; hx((int)x, (int)y); int r = a * 3; return x[r + 2] + y[r + 2];',
 'fl_nvlive':  'int x[54], y[54]; hx((int)x, (int)y); int r = a * 3; use(x[r + 2] + y[r + 2]); return r;',
 'fl_nvg':     'int r = a * 3; return ga[r + 2] + gb[r + 2];',
 'fl_nv1':     'int r = a * 3; return ga[r + 2] + ga[r + 1];',
 'st_nv':      'int r = a; return vs[r].a + vs[r].b;',
 'fl_par3':    'int x[54], y[54]; hx((int)x, (int)y); a *= 3; return x[a + 2] + y[a + 2];',
 'fl_alive':   'int x[54], y[54]; hx((int)x, (int)y); use(x[a * 3 + 2] + y[a * 3 + 2]); return a;',
 'mx_2dflat':  'int x[18][3], y[54]; hx((int)x, (int)y); return x[a][2] + y[a * 3 + 2];',
 'mx_swap':    'int x[54], y[54]; hx((int)x, (int)y); return x[a * 3 + 2] + y[2 + a * 3];',
 'mx_3a':      'int x[54], y[54]; hx((int)x, (int)y); return x[a * 3 + 2] + y[3 * a + 2];',
 'mx_ptr':     'int x[54], y[54]; hx((int)x, (int)y); return (x + 2)[a * 3] + (y + 2)[a * 3];',
 'mx_short':   'int x[54], y[54]; hx((int)x, (int)y); short s = a; return x[s * 3 + 2] + y[s * 3 + 2];',
 'mx_char':    'int x[54], y[54]; hx((int)x, (int)y); char s = a; return x[s * 3 + 2] + y[s * 3 + 2];',
 'mx_uns':     'int x[54], y[54]; hx((int)x, (int)y); unsigned s = a; return x[s * 3 + 2] + y[s * 3 + 2];',
 'mx_add':     'int x[54], y[54]; hx((int)x, (int)y); return x[a + a + a + 2] + y[a + a + a + 2];',
 'mx_stmt':    'int x[54], y[54]; hx((int)x, (int)y); int p = x[a * 3 + 2]; use(0); int q = y[a * 3 + 2]; return p + q;',
 'mx_inl':     'int x[54], y[54]; hx((int)x, (int)y); return at3(x, a, 2) + at3(y, a, 2);',
 'mx_glob_a':  'int x[54], y[54]; hx((int)x, (int)y); return x[G * 3 + 2] + y[G * 3 + 2];',
 'mx_struct':  'struct { int x[54]; int y[54]; } s; hx((int)&s, 0); return s.x[a * 3 + 2] + s.y[a * 3 + 2];',
 'mx_2d2':     'int x[18][3], y[18][3]; hx((int)x, (int)y); return x[a][2] + y[a][2];',
 'mx_rowst':   'R x[18], y[18]; hx((int)x, (int)y); return x[a].x[2] + y[a].x[2];',
 'big16':  FILL(16) + 'int x[54], y[54]; hx((int)x, (int)y); return x[a * 3 + 2] + y[a * 3 + 2];',
 'big64':  FILL(64) + 'int x[54], y[54]; hx((int)x, (int)y); return x[a * 3 + 2] + y[a * 3 + 2];',
 'big256': FILL(256) + 'int x[54], y[54]; hx((int)x, (int)y); return x[a * 3 + 2] + y[a * 3 + 2];',
 'big1k':  FILL(1024) + 'int x[54], y[54]; hx((int)x, (int)y); return x[a * 3 + 2] + y[a * 3 + 2];',
 'bigs1k': FILLS(1024) + 'int x[54], y[54]; hx((int)x, (int)y); return x[a * 3 + 2] + y[a * 3 + 2];',
 'bigs4k': FILLS(4096) + 'int x[54], y[54]; hx((int)x, (int)y); return x[a * 3 + 2] + y[a * 3 + 2];',
 'cp_k':     'int x[54], y[54]; hx((int)x, (int)y); int k = 2; return x[k + a * 3] + y[k + a * 3];',
 'cp_k2':    'int x[54], y[54]; hx((int)x, (int)y); int k = 2; return x[a * 3 + k] + y[a * 3 + k];',
 'cp_kmix':  'int x[54], y[54]; hx((int)x, (int)y); int k = 2; return x[a * 3 + 2] + y[a * 3 + k];',
 'cp_k1':    'int x[54]; hx((int)x, 0); int k = 2; return x[a * 3 + k] + x[a * 3 + 1];',
 'cp_kloop': 'int x[54], y[54]; hx((int)x, (int)y); int t = 0; for (int k = 0; k < 3; k++) t += x[a * 3 + k] + y[a * 3 + k]; return t;',
 'cp_kg':    'int k = 2; return ga[a * 3 + k] + gb[a * 3 + k];',
 'cp_sw':    'int x[54], y[54]; hx((int)x, (int)y); switch (b) { case 1: use(x[b + a * 3]); break; case 2: return x[b + a * 3] + y[b + a * 3]; } return 0;',
 'cp_if':    'int x[54], y[54]; hx((int)x, (int)y); if (b == 2 && G != 2) return x[b + a * 3] + y[b + a * 3]; return 0;',
 'cp_ifo':   'int x[54], y[54]; hx((int)x, (int)y); if (b == 2 || G != 2) return x[b + a * 3] + y[b + a * 3]; return 0;',
 # 12-byte rows (base_build a_3c4)
 'row_sub':    'rows[a].x[1] += b; use(rows[a].x[2]); return 0;',
 'row_2d':     'r2[a][1] += b; use(r2[a][2]); return 0;',
 'row_ref':    'R &r = rows[a]; r.x[1] += b; use(r.x[2]); return 0;',
 'row_ld3':    'use(rows[a].x[0]); use(rows[a].x[1]); use(rows[a].x[2]); return 0;',
 'row_ld3c':   'use(rows[a].x[0] + rows[a].x[1] + rows[a].x[2]); return 0;',
 'row_st3':    'rows[a].x[0] = 0; rows[a].x[1] = 0; rows[a].x[2] = b; return 0;',
 'row_st3d':   'r2[a][0] = 0; r2[a][1] = 0; r2[a][2] = b; return 0;',
}


def cls(lines):
    t = '\n'.join(lines)
    uses = []
    for l in lines:
        m = re.search(r'\?(vs|ws|rows|r2|pd|ga|gb)@@[\w@]+(\+\d+)?\[(\w+)(\*4)?(\+\d+)?\]', l)
        if m:
            uses.append('X' if m.group(4) else 'B')
        elif re.search(r'_[xy]\$\[ebp\+e\w\w\*4', l):
            uses.append('X')
        elif re.search(r'_[xy]\$\[ebp\+e\w\w[+\]]', l):
            uses.append('B')
        elif re.search(r'PTR \[e\w\w(\+\d+|-\d+)?\]', l) and not re.search(r'\[e[bs]p', l):
            uses.append('A')
    mul = len(re.findall(r'lea\s+e\w\w, DWORD PTR \[e\w\w\+e\w\w\*[248]\]', t))
    shl = len(re.findall(r'\bshl\s', t))
    return ''.join(uses), mul, shl


def main(names):
    src = HDR + ''.join('int v_%s(int a, int b) {\n%s\n}\n' % (k, v) for k, v in V.items() if not names or k in names)
    asm, _ = ib.compile_asm(None, PFX, text=src)
    show = os.environ.get('SHOW')
    for title, sym, lines in ib.functions(asm):
        nm = re.sub(r'^\?|@.*$', '', sym).lstrip('_')[2:]
        u, mul, shl = cls(lines)
        regs = sorted(set(re.findall(r'\b(ebx|esi|edi)\b', ' '.join(lines))))
        print('%-12s uses=%-10s lea=%d shl=%d %s' % (nm, u, mul, shl, ','.join(regs)))
        if show == nm or show == 'all':
            print('\n'.join('   ' + l for l in lines))


if __name__ == '__main__':
    main(sys.argv[1:])
