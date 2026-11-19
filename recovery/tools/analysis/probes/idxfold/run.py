#!/usr/bin/env python3
"""run.py: when does VC6 keep a constant index offset inside the scaled index
(`lea r,[r+r*2+0xD2]; shl r,4; [r+G]`) instead of folding K*stride into the
displacement (`lea r,[r+r*2]; shl r,4; [r+G+0xD20]`)?  (MATCHING_GUIDE 3.4,
process_message 0x00534400, base_editor_proj 0x0041BBC0)
Usage: uv run python run.py [--show NAME]   (FE_PFX_BASE, default 14)"""
import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))) + '/inline')
import ib
PFX = os.environ.get('FE_PFX_BASE', '14')
HDR = r'''#include "probe.h"
struct R { char *name; int a, b, c, d, e, f, g, h, i, j, k; };   // 0x30 bytes
extern R Fac[];
struct P { int f10, f14, f18; };
static const int K = 70;
enum { EK = 70 };
#define DK 70
extern void say(int, char *);
extern int sel(int);
inline R *rec(int id) { return &Fac[id]; }
inline R *sp(int id) { return &Fac[id + K]; }
inline char *spname(int id) { return Fac[id + K].name; }
extern unsigned uid; extern short sid;
struct RT { R fac[134]; };
extern RT Rules;
extern R FacN[134];
struct Q { char *name; int a, b, c; };   // 16 bytes
extern Q Q16[];
struct SPX { enum { base = 70 }; };
inline char *stname(int fid) { char *n = Fac[fid].name; if (!n) n = 0; return n; }
inline R *strec(int fid) { R *r = &Fac[fid]; return r; }
inline void sayfac(int k, int fid) { if (k) say(k, Fac[fid].name); }
inline char *nameat(int id, int base) { return Fac[base + id].name; }
int GK = 70;
static int SK = 70;
extern const int EXK; const int EXK = 70;
extern const int EXU;
struct SS { static const int k; }; const int SS::k = 70;
const int CK = 70;
extern void take(int *);
struct D { D(); ~D(); int x; };
inline int spid(int id) { return id + K; }
inline int spid2(int id) { int r = id + K; return r; }
inline char *facname(int fid) { return Fac[fid].name; }
'''
V = {
'plain':      'void t_plain(P *p) { int id = p->f10; say(1, Fac[K + id].name); }',
'enum':       'void t_enum(P *p) { int id = p->f10; say(1, Fac[EK + id].name); }',
'define':     'void t_define(P *p) { int id = p->f10; say(1, Fac[DK + id].name); }',
'idfirst':    'void t_idfirst(P *p) { int id = p->f10; say(1, Fac[id + K].name); }',
'ptr':        'void t_ptr(P *p) { int id = p->f10; R *r = &Fac[K + id]; say(1, r->name); say(2, r->name); }',
'var':        'void t_var(P *p) { int i = p->f10 + K; say(1, Fac[i].name); }',
'var_used':   'void t_var_used(P *p) { int i = p->f10 + K; say(i, Fac[i].name); }',
'var_used2':  'void t_var_used2(P *p) { int i = p->f10 + K; say(1, Fac[i].name); sel(i); }',
'unsigned':   'void t_unsigned(P *p) { unsigned id = p->f10; say(1, Fac[K + id].name); }',
'uglob':      'void t_uglob() { say(1, Fac[K + uid].name); }',
'short':      'void t_short() { say(1, Fac[K + sid].name); }',
'rec':        'void t_rec(P *p) { say(1, rec(p->f10 + K)->name); }',
'sp':         'void t_sp(P *p) { say(1, sp(p->f10)->name); }',
'spname':     'void t_spname(P *p) { say(1, spname(p->f10)); }',
'ptrarith':   'void t_ptrarith(P *p) { say(1, (Fac + K)[p->f10].name); }',
'ptrarith2':  'void t_ptrarith2(P *p) { R *base = Fac + K; say(1, base[p->f10].name); }',
'cast':       'void t_cast(P *p) { say(1, Fac[(int)(K + p->f10)].name); }',
'ucast':      'void t_ucast(P *p) { say(1, Fac[(unsigned)(K + p->f10)].name); }',
'call':       'void t_call(P *p) { say(1, Fac[sel(p->f10) + K].name); }',
'twice':      'void t_twice(P *p) { int id = p->f10; say(1, Fac[K + id].name); say(2, Fac[K + id].name); }',
'mul':        'void t_mul(P *p) { int id = p->f10; say(1, ((R *)((char *)Fac + (id + K) * sizeof(R)))->name); }',
'spid':       'void t_spid(P *p) { say(1, Fac[spid(p->f10)].name); }',
'spid2':      'void t_spid2(P *p) { say(1, Fac[spid2(p->f10)].name); }',
'facname':    'void t_facname(P *p) { say(1, facname(p->f10 + K)); }',
'schar':      'void t_schar(P *p) { say(1, Fac[(short)(p->f10 + K)].name); }',
'slot':       'void t_slot(P *p) { int id = sel(0); if (id < 0) return; sel(id); say(1, Fac[K + id].name); sel(id); }',
'cond':       'void t_cond(P *p, int c) { int id = c ? p->f10 : p->f18; say(1, Fac[K + id].name); }',
'member':     'void t_member(P *p) { say(1, Rules.fac[K + p->f10].name); }',
'sized':      'void t_sized(P *p) { say(1, FacN[K + p->f10].name); }',
'q16':        'void t_q16(P *p) { say(1, Q16[(p->f10 + K) * 3].name); }',
'clsk':       'void t_clsk(P *p) { say(1, Fac[SPX::base + p->f10].name); }',
'neg':        'void t_neg(P *p) { say(1, Fac[p->f10 - -K].name); }',
'sub':        'void t_sub(P *p) { int id = p->f10; say(1, Fac[id - (-70)].name); }',
'ref':        'void t_ref(P *p) { R &r = Fac[K + p->f10]; say(1, r.name); }',
'ptrcast':    'void t_ptrcast(P *p) { say(1, *(char **)&Fac[K + p->f10]); }',
'stname':     'void t_stname(P *p) { say(1, stname(p->f10 + K)); }',
'strec':      'void t_strec(P *p) { say(1, strec(p->f10 + K)->name); }',
'sayfac':     'void t_sayfac(P *p) { sayfac(1, p->f10 + K); }',
'loc2':       'void t_loc2(P *p) { int fid = p->f10 + K; R *r = &Fac[fid]; say(1, r->name); say(2, r->name); }',
'lconst':     'void t_lconst(P *p) { const int k = 70; say(1, Fac[k + p->f10].name); }',
'lvar':       'void t_lvar(P *p) { int k = 70; say(1, Fac[k + p->f10].name); }',
'lvar2':      'void t_lvar2(P *p) { int k = 70; int id = p->f10; say(1, Fac[k + id].name); say(2, Fac[k + p->f18].name); }',
'inlk':       'void t_inlk(P *p) { say(1, nameat(p->f10, K)); }',
'sk':         'void t_sk(P *p) { say(1, Fac[SK + p->f10].name); }',
'exk':        'void t_exk(P *p) { say(1, Fac[EXK + p->f10].name); }',
'exu':        'void t_exu(P *p) { say(1, Fac[EXU + p->f10].name); }',
'ssk':        'void t_ssk(P *p) { say(1, Fac[SS::k + p->f10].name); }',
'ck':         'void t_ck(P *p) { say(1, Fac[CK + p->f10].name); }',
'off':        'void t_off(P *p) { int off = (p->f10 + K) * 48; say(1, *(char **)((char *)Fac + off)); }',
'off2':       'void t_off2(P *p) { int off = (p->f10 + K) * (int)sizeof(R); sel(off); say(1, *(char **)((char *)Fac + off)); }',
'twoidx':     'void t_twoidx(P *p) { int id = p->f10; R *r = &Fac[K + id]; say(1, r->name); say(2, Fac[K + p->f18].name); sel(id); }',
'idx3':       'void t_idx3(P *p) { int i = (p->f10 + K) * 3; say(1, Q16[i].name); }',
'intaddr':    'void t_intaddr(P *p) { say(1, ((R *)((int)Fac + (p->f10 + K) * 48))->name); }',
'intaddr2':   'void t_intaddr2(P *p) { int a = (p->f10 + K) * 48 + (int)Fac; say(1, ((R *)a)->name); }',
'divblk':     'void t_divblk(P *p) { say(1, Fac[(p->f10 + K) * 48 / 48].name); }',
'addr':       'void t_addr(P *p) { int id = p->f10; take(&id); say(1, Fac[K + id].name); }',
'vol':        'void t_vol(P *p) { volatile int id = p->f10; say(1, Fac[K + id].name); }',
'many':       'void t_many(P *p, int a, int b, int c, int d) { int id = p->f10; int e = p->f14, f = p->f18; for (int i = 0; i < a; i++) { say(e, 0); say(f, 0); sel(b + c + d + i); } say(1, Fac[K + id].name); sel(e + f + a + b + c + d); }',
'var_eh':     'void t_var_eh(P *p) { D d; int i = p->f10 + K; say(1, Fac[i].name); }',
'var_call':   'void t_var_call(P *p) { int id = sel(0); if (id < 0) return; int i = K + id; say(1, Fac[i].name); sel(2); }',
'var_slot':   'void t_var_slot(P *p) { int id = sel(0); if (id < 0) return; int i = K + id; say(1, Fac[i].name); sel(id); }',
'var_loop':   'void t_var_loop(P *p) { for (int j = 0; j < 64; j++) say(1, Fac[K + j].name); int id = sel(0); int i = K + id; say(1, Fac[i].name); sel(id); }',
'eh_slot':    'void t_eh_slot(P *p) { D d; int id = sel(0); if (id < 0) return; int i = K + id; say(1, Fac[i].name); sel(id); }',
'eh_noval':   'void t_eh_noval(P *p) { D d; int id = sel(0); if (id < 0) return; say(1, Fac[K + id].name); sel(id); }',
}
def shape(lines):
    out = []
    for l in lines:
        if re.match(r'lea\s+\w+, DWORD PTR \[\w+\+\w+\*2(\+\d+)?\]', l) or 'shl' in l or 'imul' in l or re.search(r'\?\w+@@3\S*\[', l):
            out.append(l)
    return out
def main():
    show = sys.argv[sys.argv.index('--show') + 1] if '--show' in sys.argv else None
    src = HDR + '\n'.join(V.values()) + '\n'
    asm, _ = ib.compile_asm(None, PFX, text=src)
    F = {f[0].split('(')[0].strip(): f[2] for f in ib.functions(asm)}
    for k in V:
        lines = F['t_' + k]
        s = shape(lines)
        kept = any(re.search(r'\*2\+(210|0d2H)\]', l, re.I) or re.search(r'\*2\+210', l) for l in s)
        folded = any(re.search(r'@@3\S*\+3360\]', l) for l in s)
        print('%-10s %-7s %s' % (k, 'INDEX' if kept else ('FOLDED' if folded else '?'), ' | '.join(s)))
        if show == k: print('\n'.join(lines))
main()
