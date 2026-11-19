import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import run as R
E = R.EXTERN
V = []
base = R.PRE + E[0]
V.append(('new_var', base + 'void t_nv(int n) { R *r = new R[n]; sink(r); }\n'))
V.append(('new_const', base + 'void t_nc() { R *r = new R[7]; sink(r); }\n'))
V.append(('virtual', R.PRE + 'struct R { int a[95]; R(); virtual void f(); };\nvoid t_v() { R r[7]; sink(r); }\n'))
V.append(('2d', base + 'void t_2d() { R r[2][7]; sink(r); }\n'))
V.append(('Q_ctor', base + 'struct Q { int h[4]; R rows[7]; Q(); };\nQ::Q() { h[0] = 0; }\nvoid t_q() { Q q; sink(&q); }\n'))
V.append(('Q_ctor_ext', base + 'struct Q { int h[4]; R rows[7]; Q(); };\nQ::Q() { h[0] = 0; }\n'))
V.append(('Q_gen_func', base + 'struct Q { int h[4]; R rows[7]; };\nvoid t_qg() { Q q; sink(&q); }\n'))
V.append(('defarg', R.PRE + 'struct R { int a[95]; R(int x = 0); };\nvoid t_da() { R r[7]; sink(r); }\n'))
V.append(('static_local', base + 'void t_sl() { static R r[7]; sink(r); }\n'))
V.append(('global', base + 'R gr[7];\nvoid t_g() { sink(gr); }\n'))
for k, t in V:
    print('== ' + k)
    asm, _ = R.ib.compile_asm(None, R.PFX, text=R.HDR + t)
    for name, sym, lines in R.ib.functions(asm):
        tag, calls = R.classify(lines)
        print('  %-40s %-12s %s' % (name[:40], tag, ' '.join(calls)))
