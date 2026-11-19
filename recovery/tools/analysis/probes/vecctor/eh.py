"""eh.py: does an EH frame in the caller (a local with a destructor) change the ??_H decision?"""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import run as R
D = 'struct D { D(); ~D(); int v; };\n'
V = [
 ('no_eh', 'void t_a() { R r[7]; sink(r); }\n'),
 ('eh_before', 'void t_b() { D d; R r[7]; sink(r); sink(&d); }\n'),
 ('eh_after', 'void t_c() { R r[7]; sink(r); D d; sink(&d); }\n'),
 ('eh_member_before', 'struct P { int h[4]; R rows[7]; };\nvoid t_d() { D d; P p; sink(&p); sink(&d); }\n'),
 ('eh_member_after', 'struct P { int h[4]; R rows[7]; };\nvoid t_e() { P p; sink(&p); D d; sink(&d); }\n'),
 ('eh_inner_scope', 'void t_f() { { D d; sink(&d); } R r[7]; sink(r); }\n'),
]
for elem in ('extern', 'ool'):
    head = R.EXTERN[0] if elem == 'extern' else R.OOL_BEFORE[0]
    for k, body in V:
        t = R.PRE + D + head + body
        asm, _ = R.ib.compile_asm(None, R.PFX, text=R.HDR + t)
        for name, sym, lines in R.ib.functions(asm):
            if name.startswith('t_'):
                tag, calls = R.classify(lines)
                print('%-8s %-18s %s' % (elem, k, tag))
