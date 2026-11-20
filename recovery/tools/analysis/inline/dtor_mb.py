#!/usr/bin/env python3
"""dtor_mb.py: implicit member and base destructor calls inside a destructor body (MATCHING_GUIDE 2.8
rule 9, 2026-11-20 par-4).

   P : B { M m; ~P() { dm(); } },  ~M() { dm2(); nm ifs },  ~B() { dm3(); nb ifs }
   caller: `P p;` with N early returns (N + 1 teardowns, the closing one last).

Per teardown: P = ~P expanded, M/m = ~M expanded/called, B/b = ~B expanded/called, p = ~P called.

   python3 dtor_mb.py NM NB N...          one (nm, nb) cell
   python3 dtor_mb.py --grid              nm, nb in 0,2,4,8,12 x N in 1,2,3,4,6,8,12, then the rule-8 fit

Fit (1075 teardown decisions): plain rule 8 with ~P's body = [~M, ~B] in destruction order, each an
ordinary site (later_j counts the base), charge a + n_ifs with a = 2.5 for both (no base exemption),
c(~P) = 3.5, S = 54.5: 20 decisions differ, all one-site shifts at the end of N = 4..12. The base
decided first ([~B, ~M]) gives 266, the base lifted to the caller's level 560."""
import sys, os, json
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from deck import run


def ifs(n, base):
    return ' '.join('if (ga[%d] == 1000) use(%d);' % (base + i, base + i) for i in range(n))


def hdr(nm, nb, np_=0):
    return '''
extern void dm2(); extern void dm3();
struct GW { virtual ~GW(); int x[4]; };
struct M : GW { M(); virtual ~M() { dm2(); %s } };
struct B : GW { B(); virtual ~B() { dm3(); %s } };
struct P : B { M m; P(); virtual ~P() { dm(); %s } };
''' % (ifs(nm, 0), ifs(nb, 20), ifs(np_, 40))


def fn(name, N):
    body = ' '.join('if (cond()) { at(%d); return; }' % i for i in range(1, N + 1))
    return 'void %s() { P p; %s at(%d); }' % (name, body, N + 1)


def code(t):
    if 'E' not in t:
        return 'p' if 'cP' in t else '?'
    return ('M' if 'E2' in t else 'm' if 'cM' in t else '?') + ('B' if 'E3' in t else 'b' if 'cB' in t else '?')


def go(nm, nb, Ns):
    r = run([hdr(nm, nb)] + [fn('r%d' % N, N) for N in Ns])
    return {N: [code(r['r%d' % N][1].get(i, [])) for i in range(1, N + 2)] for N in Ns}


def sim(N, cm, cb, S, cP):
    """rule 8: teardown i gets (S - spent) / (1 + later); inside ~P, ~M then ~B greedily."""
    out, spent = [], 0.0
    for i in range(N + 1):
        if cP > S - spent + 1e-9:
            out.append('p')
            continue
        spent += cP
        sp = (S - spent) / (N - i + 1)
        m = cm <= sp + 1e-9
        b = cb <= sp - (cm if m else 0) + 1e-9
        spent += (cm if m else 0) + (cb if b else 0)
        out.append(('M' if m else 'm') + ('B' if b else 'b'))
    return out


def fit(D):
    best = None
    for S in [x * 0.5 for x in range(100, 120)]:
        for a in [x * 0.25 for x in range(6, 14)]:
            for cP in [x * 0.5 for x in range(4, 10)]:
                bad = 0
                for k, v in D.items():
                    nm, nb, N = map(int, k.split(','))
                    bad += sum(x != y for x, y in zip(sim(N, a + nm, a + nb, S, cP), v))
                if best is None or bad < best[0]:
                    best = (bad, S, a, cP)
    return best


if __name__ == '__main__':
    if sys.argv[1:] == ['--grid']:
        Ns, D = [1, 2, 3, 4, 6, 8, 12], {}
        for nm in [0, 2, 4, 8, 12]:
            for nb in [0, 2, 4, 8, 12]:
                for N, v in go(nm, nb, Ns).items():
                    D['%d,%d,%d' % (nm, nb, N)] = v
                print(nm, nb, ' | '.join(''.join(D['%d,%d,%d' % (nm, nb, N)]) for N in Ns), flush=True)
        print('fit (mismatches, S, a, c(~P)) of %d:' % sum(len(v) for v in D.values()), fit(D))
    else:
        nm, nb = int(sys.argv[1]), int(sys.argv[2])
        for N, v in go(nm, nb, [int(x) for x in sys.argv[3:]]).items():
            print('%3d' % N, ' '.join(v))
