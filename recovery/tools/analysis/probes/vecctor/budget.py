"""budget.py: is the ??_H expansion an ordinary /Ob2 budget decision?  The array is constructed
at the top of a caller followed (after) or preceded (before) by N branchy inline calls."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import run as R
INL = 'inline int rng(int a, int b) { if (a < b) { use(a); return b; } if (a > g) { use(b); return a; } return 0; }\n'
def caller(name, pre, post, size=0):
    s = 'void %s(int x) {\n' % name
    for i in range(pre): s += '  g%d = rng(x, %d);\n' % (2 + i % 8, i)
    s += '  R r[7]; sink(r);\n'
    for i in range(post): s += '  g%d = rng(x, %d);\n' % (2 + i % 8, i + 500)
    for i in range(size): s += '  if (g == %d) use(%d);\n' % (i, i)
    return s + '}\n'
V = []
for e in ('extern', 'ool'):
    head = R.EXTERN[0] if e == 'extern' else R.OOL_BEFORE[0]
    for pre in (0, 4, 8, 16, 32, 64):
        V.append(('%s pre=%d' % (e, pre), R.PRE + head + INL + caller('t_pre%d' % pre, pre, 0)))
    for size in (0, 100, 400):
        V.append(('%s pre=16 size=%d' % (e, size), R.PRE + head + INL + caller('t_sz%d' % size, 16, 0, size)))
for k, t in V:
    asm, _ = R.ib.compile_asm(None, R.PFX, text=R.HDR + t)
    for name, sym, lines in R.ib.functions(asm):
        if not name.startswith('t_'): continue
        tag, calls = R.classify(lines)
        n_rng = sum(1 for c in calls if c.startswith('?rng'))
        print('%-24s %-12s rng-calls=%d' % (k, tag, n_rng))
print('-- sites after the array')
for post in (8, 16, 32, 64, 128):
    t = R.PRE + R.EXTERN[0] + INL + caller('t_post%d' % post, 0, post)
    asm, _ = R.ib.compile_asm(None, R.PFX, text=R.HDR + t)
    for name, sym, lines in R.ib.functions(asm):
        if not name.startswith('t_'): continue
        tag, calls = R.classify(lines)
        print('post=%-4d %-12s rng-calls=%d' % (post, tag, sum(1 for c in calls if c.startswith('?rng'))))
