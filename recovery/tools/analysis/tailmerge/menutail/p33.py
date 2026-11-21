"""par-6 r54: jump table against compare chain for the JJ toggle-tail merge (p28 harness, ii17 tail).
`table` = dense case values (one jump table), `chain` = sparse values (compare tree), `mixed` = toggles in
a table, base_names reached by compare, `bytetab` = two-level (byte-indexed) table as menu_proc's 0x213 range.
psave-calls 1 = merged (ours), 4 = kept (image)."""
import sys; from cj import *
import cj
from p1 import T as TG, toggle
V = []
TAIL = 'psave(0); dmap(1);'
def mk(n, vals, extra=''):
    s = ''.join('static inline void tg%d_%s(int c) %s\n' % (i, n, toggle(i, *t, tail=TAIL)) for i, t in enumerate(TG))
    s += 'static inline void bn_%s(int c) { int f = gmp; int n; if (f & 0x1000) n = f & 0xFFFFCFFF; else if (f & 0x2000) n = f | 0x1000; else n = f | 0x2000; gmp = n; imp = n; %s }\n' % (n, TAIL)
    a, b, c_, d, e, f1, g = vals
    s += 'void t_%s(int c) {\n switch (c) {\n  case %d: f0(); break;\n' % (n, a)
    s += ''.join('  case %d: tg%d_%s(c); break;\n' % (v, i, n) for i, v in enumerate((b, c_, d)))
    s += '  case %d: { gx ^= 0x40000; dmap(1); } break;\n  case %d: bn_%s(c); break;\n  case %d: use(30); break;\n%s  }\n' % (e, f1, n, g, extra)
    s += '  if (gx & 0x40) { switch (c) { case 40: f0(); return; case 41: use(41); return; } }\n}\n'
    V.append((n, s))
mk('table', (1, 10, 11, 12, 13, 14, 30))
mk('chain', (1, 0x20047, 0x30058, 0x40001, 0x50000, 0x60000, 0x70000))
mk('chain2', (0x10, 0x200, 0x3000, 0x40000, 0x500000, 0x6000000, 0x7))
mk('mixed', (1, 10, 11, 12, 13, 0x60000, 30))
mk('mixed2', (1, 0x20047, 11, 0x40001, 13, 14, 30))
by = ''.join('  case %d: use(%d); break;\n' % (100 + 9 * i, i) for i in range(30))
mk('bytetab', (1, 10, 11, 12, 13, 14, 30), extra=by)
run(V, show=sys.argv[1:])
