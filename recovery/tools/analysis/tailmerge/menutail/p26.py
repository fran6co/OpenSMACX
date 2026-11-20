"""Cross-jump: physical register or pre-allocation value? (par-6 r53).  Four cases each end
`psave(v); dmap(1);` where v is a DIFFERENT local per case (all allocated to the same callee-saved
register, live across the case's calls), against the same local in every case.  psave-calls 1 = merged."""
import sys; from cj import *
import cj
cj.HDR += 'extern int cond2(); extern void mid(int);\n'
V = []
def mk(n, same):
    s = 'void t_%s(int c) {\n  int v = 0;\n  switch (c) {\n  case 1: f0(); break;\n' % n
    for i in range(4):
        var = 'v' if same else 'v%d' % i
        decl = 'v = cond();' if same else 'int v%d = cond();' % i
        s += '  case %d: { %s mid(%d); use(%s + %d); psave(%s); dmap(1); } break;\n' % (10 + i, decl, i, var, i, var)
    s += '  case 30: use(30); break;\n  }\n  if (gx & 0x40) use(c);\n}\n'
    V.append((n, s))
mk('same_var', 1)
mk('distinct_vars', 0)
run(V, show=sys.argv[1:])
