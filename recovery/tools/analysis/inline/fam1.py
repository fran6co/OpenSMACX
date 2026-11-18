"""fam1.py: nested-inline family. outer(k) = mk(k) + j flat use() + inn(k); inn = mk2 + ni ifs.
For each (j, ni) and N in a list, record the forward accept string of inn and fit
room_i = A - N*Y + i*D - accepted_before (unit = inner cost), accept iff room_i >= 1."""
import sys, itertools
sys.path.insert(0, '.')
from deck import run
import ib

def patterns(j, ni, Ns, K=0):
    P = ['inline void inn(int k) { mk2(k); %s }' % ' '.join('if (ga[%d]==1000) use(%d);' % (i, i) for i in range(ni)),
         'inline void out(int k) { mk(k); %s inn(k); }' % ' '.join('use(%d);' % (100 + i) for i in range(j))]
    for N in Ns:
        P.append('void n%d() { %s %s }' % (N, ' '.join('g3 = %d;' % (5000 + i) for i in range(K)), ' '.join('out(%d);' % i for i in range(1, N + 1))))
    r = run(P)
    out = {}
    for N in Ns:
        l = r['n%d' % N][2]
        o = {}; inn = {}
        for k, c, x in ib.sites(l):
            if c == 'mk': o[k] = 'O'
            elif c == 'out': o[k] = 'o'
            elif c == 'mk2': inn[k] = 'I'
            elif c == 'inn': inn[k] = 'i'
        out[N] = (''.join(o.get(i, '?') for i in range(1, N + 1)), ''.join(inn.get(i, '-') for i in range(1, N + 1)))
    return out

def sim(A, Y, D, N, outer):
    room = A - N * Y; s = []
    for i in range(1, N + 1):
        room += D
        if outer[i - 1] != 'O':
            s.append('-'); room -= D; continue
        if room >= 1 - 1e-9:
            s.append('I'); room -= 1
        else:
            s.append('i')
    return ''.join(s)

def fit(pats):
    sols = []
    for Yi in range(0, 241, 2):
        Y = Yi / 100
        for Di in range(0, 241, 2):
            D = Di / 100
            As = []
            for Ai in range(-200, 4000, 5):
                A = Ai / 100
                if all(sim(A, Y, D, N, o) == p for N, (o, p) in pats.items()):
                    As.append(A)
            if As:
                sols.append((Y, D, min(As), max(As)))
    return sols

if __name__ == '__main__':
    j, ni = int(sys.argv[1]), int(sys.argv[2])
    Ns = [int(x) for x in sys.argv[3:]] or [6, 8, 10, 12, 14, 16, 20]
    pats = patterns(j, ni, Ns)
    for N in Ns:
        print('%3d %s %s' % (N, pats[N][0], pats[N][1]))
    s = fit(pats)
    if s:
        Ys = [x[0] for x in s]; Ds = [x[1] for x in s]
        print('fit: Y %.2f..%.2f  D %.2f..%.2f  A %.2f..%.2f  (%d sols)' % (min(Ys), max(Ys), min(Ds), max(Ds), min(x[2] for x in s), max(x[3] for x in s), len(s)))
    else:
        print('fit: none')
