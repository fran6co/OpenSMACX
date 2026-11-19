#!/usr/bin/env python3
"""run.py [name ...]: which form does `field & K` (K in bits 8..15 of a 32-bit field) take?  (MATCHING_GUIDE 3.4)
  mov r,[m]; test rh,K>>8      a scratch register is free
  test dword ptr [m],K         none free, not inside a loop
  test byte ptr [m+1],K>>8     none free, inside a loop
Families: plain_/helper/enumk/ptr (spellings, register free), pp_ (register pressure, no loop), q_/l_/m_/a_
(loops: stores, calls, pointer stores, address-taken index), n_ (loop or not: goto, do, while, always-break).
FT=<type> sets the field's type.  Prints the test instructions of each function; SHOW=1 the listing.
Run from the repository root; FE_PFX_BASE selects the compiler prefix (default 21)."""
import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))) + '/inline')
import ib
HDR = "typedef " + os.environ.get("FT","unsigned") + " FT;" + '''
struct P { FT flags; int f2; int trea[8]; char pad[100]; };
struct Q { int d; int e; char pad[8388]; }; extern Q Qs[]; extern P Ps[]; extern unsigned G; extern int GI; extern int ga[];
void use(int); void usep(int *);
#define max(a,b) (((a) > (b)) ? (a) : (b))
inline int hasf(int i, int m) { return Ps[i].flags & m; }
inline int hast(int i, int j, int m) { return Ps[i].trea[j] & m; }
inline int known(int f, int j) { return (Ps[f].flags & 0x200 || Ps[f].f2 & (1 << j)); }
inline void st(int f, int j) { if (f == GI && !(Ps[f].f2 & (1 << j)) && !(G & 0x80) && !(Ps[f].flags & 0x200)) { Ps[f].trea[0] |= 0x400000; GI |= 1; } }
struct M { unsigned char cl, vis; short s; int i4; unsigned bit; unsigned bit2; unsigned bv[7]; }; extern M *MP; extern unsigned WF; extern int ML;
enum { F200 = 0x200 };
'''
V = {
 'pf': 'unsigned f = i / 64; if (Ps[f].flags & 0x200) return 0; if (Qs[f].d <= 3) return 0; return Qs[f].e + 1;',
 'pf_i': 'int f = i / 64; if (Ps[f].flags & 0x200) return 0; if (Qs[f].d <= 3) return 0; return Qs[f].e + 1;',
 'pf_1': 'unsigned f = i / 64; if (Ps[f].flags & 0x200) return 0; return Qs[f].e + 1;',
 'pf_0': 'unsigned f = i / 64; if (Ps[f].flags & 0x200) return 0; return 1;',
 'pp': 'unsigned f = i / 64; if (Ps[f].flags & 0x200) return 0; if (Qs[f].d <= 3) return 0; return Qs[f].e + i;',
 'pp_i': 'int f = i / 64; if (Ps[f].flags & 0x200) return 0; if (Qs[f].d <= 3) return 0; return Qs[f].e + i;',
 'pp_if': 'int f = i / 64; int r = i; if (Ps[f].flags & 0x200) r++; if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'pp_not': 'int f = i / 64; int r = i; if (!(Ps[f].flags & 0x200)) r++; if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'pp_and': 'int f = i / 64; int r = i; if (r == GI && !(Ps[f].flags & 0x200)) r++; if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'pp_and2': 'int f = i / 64; int r = i; if (r == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { r++; use(f); } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'q_or': 'int f = i / 64; int r = i; if ((Ps[f].flags & 0x200) || (Ps[f].f2 & (1 << j))) r++; if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'q_known': 'int f = i / 64; int r = i; if (known(f, j)) r++; if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'q_hasf': 'int f = i / 64; int r = i; if (hasf(f, 0x200)) r++; if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'q_void': 'int f = i / 64; int r = i; st(f, j); if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'q_raw': 'int f = i / 64; int r = i; if (f == GI && !(Ps[f].f2 & (1 << j)) && !(G & 0x80) && !(Ps[f].flags & 0x200)) { Ps[f].trea[0] |= 0x400000; GI |= 1; } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'q_loop': 'int f = i / 64; int r = i; for (int k = 0; k < j; k++) { if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { Ps[k].trea[0] |= 0x400000; GI |= 1; } } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'q_loop2': 'int f = i / 64; int r = i; for (int k = 0; k < j; k++) { if (ga[k] == 3) continue; if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { Ps[k].trea[0] |= 0x400000; GI |= 1; } st(f, k); } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'l_call': 'int f = i / 64; int r = i; for (int k = 0; k < j; k++) { use(k); if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { Ps[k].trea[0] |= 0x400000; GI |= 1; } } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'l_callafter': 'int f = i / 64; int r = i; for (int k = 0; k < j; k++) { if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { Ps[k].trea[0] |= 0x400000; GI |= 1; } use(k); } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'l_pstore': 'int f = i / 64; int r = i; for (int k = 0; k < j; k++) { int *p = &ga[k]; if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { Ps[k].trea[0] |= 0x400000; GI |= 1; } *p |= 4; } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'l_ustore': 'int f = i / 64; int r = i; for (int k = 0; k < j; k++) { unsigned *p = (unsigned *)&ga[k]; if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { Ps[k].trea[0] |= 0x400000; GI |= 1; } *p |= 4; } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'l_ustore_in': 'int f = i / 64; int r = i; for (int k = 0; k < j; k++) { unsigned *p = (unsigned *)&ga[k]; if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { *p |= 0x400000; GI |= 1; } } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'l_ustore_in2': 'int f = i / 64; int r = i; for (int k = 0; k < j; k++) { unsigned *p = (unsigned *)&ga[k]; if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { if (!(p[1] & j)) { *p |= 0x400000; GI |= 1; } } } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'l_twice': 'int f = i / 64; int r = i; for (int k = 0; k < j; k++) { if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { Ps[k].trea[0] |= 0x400000; GI |= 1; } if (ga[k]) continue; if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { Ps[k].trea[0] |= 0x400000; GI |= 1; } } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'l_fstore': 'int f = i / 64; int r = i; for (int k = 0; k < j; k++) { if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { Ps[k].trea[0] |= 0x400000; GI |= 1; } Ps[k].flags |= 4; } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'l_fstore_f': 'int f = i / 64; int r = i; for (int k = 0; k < j; k++) { if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { Ps[k].trea[0] |= 0x400000; GI |= 1; } Ps[f].flags |= 4; } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'l_once_nog': 'int f = i / 64; int r = i; for (int k = 0; k < j; k++) { if (f == GI && !(Ps[f].flags & 0x200)) { Ps[k].trea[0] |= 0x400000; GI |= 1; } } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'l_first': 'int f = i / 64; int r = i; for (int k = 0; k < j; k++) { if (!(Ps[f].flags & 0x200) && f == GI) { Ps[k].trea[0] |= 0x400000; GI |= 1; } } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'l_param': 'int f = i / 64; int r = i; for (int k = 0; k < j; k++) { if (j == GI && !(G & 0x80) && !(Ps[j].flags & 0x200)) { Ps[k].trea[0] |= 0x400000; GI |= 1; } } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'n_noloop': 'int f = i / 64; int r = i; int k = j; if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { Ps[k].trea[0] |= 0x400000; GI |= 1; } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'n_noloop2': 'int f = i / 64; int r = i; int k = j; if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { Ps[k].trea[0] |= 0x400000; GI |= 1; } k++; if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { Ps[k].trea[0] |= 0x400000; GI |= 1; } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'n_goto': 'int f = i / 64; int r = i; int k = 0; top: if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { Ps[k].trea[0] |= 0x400000; GI |= 1; } k++; if (k < j) goto top; if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'n_do': 'int f = i / 64; int r = i; int k = 0; do { if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { Ps[k].trea[0] |= 0x400000; GI |= 1; } k++; } while (k < j); if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'n_while': 'int f = i / 64; int r = i; int k = 0; while (ga[k]) { if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { Ps[k].trea[0] |= 0x400000; GI |= 1; } k++; } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'n_after': 'int f = i / 64; int r = i; int k = 0; for (; k < j; k++) { ga[k] = f; } if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { Ps[k].trea[0] |= 0x400000; GI |= 1; } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'n_before': 'int f = i / 64; int r = i; int k = j; if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { Ps[k].trea[0] |= 0x400000; GI |= 1; } for (k = 0; k < j; k++) { ga[k] = f; }  if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'n_nested_out': 'int f = i / 64; int r = i; for (int k = 0; k < j; k++) { for (int m = 0; m < 8; m++) { ga[m] += k; } if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { Ps[k].trea[0] |= 0x400000; GI |= 1; } } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'n_forbrk': 'int f = i / 64; int r = i; for (int k = 0; k < j; k++) { if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { Ps[k].trea[0] |= 0x400000; GI |= 1; } break; } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'm_base': 'int f = i / 64; int r = i; unsigned char fm = (unsigned char)(1 << f); for (int k = 0; k < j; k++) { M *t = &MP[k]; if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { t->bit2 |= 0x400000; WF |= 1; } } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'm_vis': 'int f = i / 64; int r = i; unsigned char fm = (unsigned char)(1 << f); for (int k = 0; k < j; k++) { M *t = &MP[k]; if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { if (!(t->vis & fm)) { t->bit2 |= 0x400000; WF |= 1; } } } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'm_full': 'int f = i / 64; int r = i; unsigned char fm = (unsigned char)(1 << f); for (int k = 0; k < j; k++) { M *t = &MP[k]; if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { if (!(t->vis & fm)) { t->bit2 |= 0x400000; WF |= 1; } if (((t->bv[f - 1] ^ t->bit) & 0xFFFFFFFC) || !(t->vis & fm)) { r = max(r, k < 9 ? 2 : 3); } } if (f) { t->bv[f - 1] = t->bit; } t->vis |= fm; } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'm_tail': 'int f = i / 64; int r = i; unsigned char fm = (unsigned char)(1 << f); for (int k = 0; k < j; k++) { M *t = &MP[k]; if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { t->bit2 |= 0x400000; WF |= 1; } if (f) { t->bv[f - 1] = t->bit; } t->vis |= fm; } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'm_tailb': 'int f = i / 64; int r = i; unsigned char fm = (unsigned char)(1 << f); for (int k = 0; k < j; k++) { M *t = &MP[k]; if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { t->bit2 |= 0x400000; WF |= 1; } t->vis |= fm; } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'm_tailw': 'int f = i / 64; int r = i; unsigned char fm = (unsigned char)(1 << f); for (int k = 0; k < j; k++) { M *t = &MP[k]; if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { t->bit2 |= 0x400000; WF |= 1; } if (f) { t->bv[f - 1] = t->bit; } } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'm_pre': 'int f = i / 64; int r = i; unsigned char fm = (unsigned char)(1 << f); for (int k = 0; k < j; k++) { M *t = &MP[k]; if ((t->bit2 & 0x80002000) == 0x2000) r++; if (k >= 9 && ML != 2 && t->cl > 3) continue; if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { t->bit2 |= 0x400000; WF |= 1; } } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'a_addr': 'int f = i / 64; int r = i; int k; usep(&k); for (k = 0; k < j; k++) { M *t = &MP[k]; if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { t->bit2 |= 0x400000; WF |= 1; } } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'a_faddr': 'int f = i / 64; int r = i; usep(&f); for (int k = 0; k < j; k++) { M *t = &MP[k]; if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { t->bit2 |= 0x400000; WF |= 1; } } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'a_glob': 'int f = i / 64; int r = i; for (GI = 0; GI < j; GI++) { int k = GI; M *t = &MP[k]; if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { t->bit2 |= 0x400000; WF |= 1; } } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'a_fglob': 'int r = i; for (int k = 0; k < j; k++) { M *t = &MP[k]; if (ML == GI && !(G & 0x80) && !(Ps[ML].flags & 0x200)) { t->bit2 |= 0x400000; WF |= 1; } } int f = ML; if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'a_call': 'int f = i / 64; int r = i; for (int k = 0; k < j; k++) { use(k); M *t = &MP[k]; if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { t->bit2 |= 0x400000; WF |= 1; } } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'a_callp': 'int f = i / 64; int r = i; for (int k = 0; k < j; k++) { M *t = &MP[k]; int z = ga[k]; if (z < 5) { use(k); continue; } if (z == 9 && t->cl > 3) continue; if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { t->bit2 |= 0x400000; WF |= 1; } t->vis |= 3; r += z; } if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'a_many': 'int f = i / 64; int r = i; int a1 = ga[1], a2 = ga[2], a3 = ga[3], a4 = ga[4]; for (int k = 0; k < j; k++) { M *t = &MP[k + a1]; if (k > a2 && a3 != 2 && (t->cl < a4) != (a3 == 1)) continue; if (f == GI && !(G & 0x80) && !(Ps[f].flags & 0x200)) { t->bit2 |= 0x400000; WF |= 1; } t->bv[a2] = a1 + a3; t->vis |= (char)a4; r++; } r += a1 + a2 + a3 + a4; if (Qs[f].d <= 3) r += 5; return Qs[f].e + r;',
 'plain_ret': 'if (Ps[i].flags & 0x200) return 0; use(i); return 1;',
 'plain_ret_u': 'unsigned u = i / 64; if (Ps[u].flags & 0x200) return 0; use(u); return 1;',
 'plain_not': 'if (!(Ps[i].flags & 0x200)) { use(i); } return 1;',
 'plain_if': 'if (Ps[i].flags & 0x200) { use(i); } return 1;',
 'plain_inc': 'int r = j; if (Ps[i].flags & 0x200) { r++; } return r;',
 'chain': 'if (i == GI && !(G & 0x80) && !(Ps[i].flags & 0x200)) { use(i); } return 1;',
 'chain_or': 'if (i == GI || (Ps[i].flags & 0x200)) { use(i); } return 1;',
 'glob': 'if (G & 0x2000) { return j ? 2 : 0; } use(i); return 1;',
 'glob_if': 'if (G & 0x2000) { use(i); } return 1;',
 'glob_not': 'if (!(G & 0x2000)) { use(i); } return 1;',
 'helper': 'if (hasf(i, 0x200)) { use(i); } return 1;',
 'helper_not': 'if (!hasf(i, 0x200)) { use(i); } return 1;',
 'helper_t': 'if (hast(i, j, 0x200)) { use(i); } return 1;',
 'raw_t': 'if (Ps[i].trea[j] & 0x200) { use(i); } return 1;',
 'ne0': 'if ((Ps[i].flags & 0x200) != 0) { use(i); } return 1;',
 'eq0': 'if ((Ps[i].flags & 0x200) == 0) { use(i); } return 1;',
 'enumk': 'if (Ps[i].flags & F200) { use(i); } return 1;',
 'local': 'unsigned f = Ps[i].flags; if (f & 0x200) { use(i); } return 1;',
 'local2': 'unsigned f = Ps[i].flags; if (f & 0x200) { use(i); } if (f & 0x400) use(j); return 1;',
 'intcast': 'if ((int)Ps[i].flags & 0x200) { use(i); } return 1;',
 'ptr': 'P *p = &Ps[i]; if (p->flags & 0x200) { use(i); } return 1;',
 'tern': 'return (Ps[i].flags & 0x200) ? 3 : j;',
 'loop': 'for (int k = 0; k < j; k++) { if (Ps[i].flags & 0x200) continue; use(k); } return 1;',
}
sel = sys.argv[1:] or list(V)
src = HDR + ''.join('int %s(int i, int j) {\n%s\n}\n' % (n, V[n]) for n in sel)
asm, out = ib.compile_asm(None, os.environ.get('FE_PFX_BASE', '21'), text=src)
F = {f[1]: f[2] for f in ib.functions(asm)}
for n in sel:
    L = next((l for k, l in F.items() if ('?%s@@' % n) in k), None)
    if L is None: print(n, 'MISSING'); continue
    t = [l for l in L if re.match(r'test|and\s', l) or '512' in l or '8192' in l or 'ah' in l or 'ch' in l or 'dh' in l]
    print('%-12s %s' % (n, ' ; '.join(re.sub(r'\s+',' ',x) for x in t)))
    if os.environ.get('SHOW'): print('   ' + ' ; '.join(L))
