"""costs.py: measure the /Ob2 budget cost of a callee body at level 1.
Caller = n sites of the test callee T, then 80 sites of the reference R (2 one-if statements).
cost(T) in R units = (R expansions without T - with T) / n."""
import sys
sys.path.insert(0, '.')
from deck import run, F
REF = F('ref', 2, mark='mk2(k);')
def measure(tests, n=3):
    P = [REF]
    for i, (name, decl, call) in enumerate(tests):
        P.append(decl)
        P.append('void t_%s() { %s %s }' % (name, ' '.join(call % (900 + q) for q in range(n)), ' '.join('ref(%d);' % s for s in range(1, 81))))
    P.append('void t_base() { %s }' % ' '.join('ref(%d);' % s for s in range(1, 81)))
    r = run(P)
    import ib
    def cnt(fn):
        l = r[fn][2]
        e = sum(1 for k, c, x in ib.sites(l) if c == 'mk2' and k < 900)
        tE = len({k for k, c, x in ib.sites(l) if c == 'mk' and k >= 900})
        tc = len({k for k, c, x in ib.sites(l) if c not in ('mk', 'mk2', 'ref') and k >= 900})
        return e // 2, tE, tc
    base = cnt('t_base')[0]
    for name, decl, call in tests:
        e, tE, tc = cnt('t_' + name)
        print('%-14s expanded %d/%d  ref %d (base %d)  cost/site %.2f ref units' % (name, tE, n, e, base, (base - e) / max(tE, 1)))
if __name__ == '__main__':
    I = lambda n: ' '.join('if (ga[%d]==1000) use(%d);' % (i, i) for i in range(n))
    tests = [
        ('flat1', 'inline void T1(int k) { mk(k); }', 'T1(%d);'),
        ('flat3', 'inline void T3(int k) { mk(k); use(1); use(2); }', 'T3(%d);'),
        ('flat4', 'inline void T4(int k) { mk(k); use(1); use(2); use(3); }', 'T4(%d);'),
        ('flat6', 'inline void T6(int k) { mk(k); use(1); use(2); use(3); use(4); use(5); }', 'T6(%d);'),
        ('if1', 'inline void I1(int k) { mk(k); %s }' % I(1), 'I1(%d);'),
        ('if2', 'inline void I2(int k) { mk(k); %s }' % I(2), 'I2(%d);'),
        ('if4', 'inline void I4(int k) { mk(k); %s }' % I(4), 'I4(%d);'),
        ('force_if4', '__forceinline void FI4(int k) { mk(k); %s }' % I(4), 'FI4(%d);'),
        ('dead_if4', 'inline void D4(int k) { mk(k); if (k == 12345) { %s } }' % I(4), 'D4(%d);'),
        ('expr', 'inline int E1(int k) { return ga[k] + ga[k+1] * 3 - (ga[k+2] >> 2); }', 'mk(%d + E1(5));'),
        ('ret_if', 'inline int RI(int k) { if (ga[k] == 1000) return 1; return 0; }', 'mk(%d + RI(5));'),
        ('ternary', 'inline int RT(int k) { return ga[k] == 1000 ? 1 : 0; }', 'mk(%d + RT(5));'),
        ('cast', 'inline int CA(int k) { return (int)(short)(char)ga[k] + (int)(unsigned char)ga[k+1]; }', 'mk(%d + CA(5));'),
        ('loopfor', 'inline void LF(int k) { mk(k); for (int i = 0; i < 8; i++) ga[i] = 0; }', 'LF(%d);'),
        ('loopdo', 'inline void LD(int k) { mk(k); int i = 0; do { ga[i] = 0; } while (++i < 8); }', 'LD(%d);'),
    ]
    measure(tests)
