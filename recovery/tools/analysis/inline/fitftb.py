"""fitftb.py: fit a forward token bucket (room_i = R0 + d*i - c*accepted_before, accept iff room >= c, c = 1)
to accept/refuse strings; prints the feasible (R0, d) box per pattern."""
import sys
def sim(R0, d, n):
    room = R0; out = []
    for i in range(1, n + 1):
        room += d
        if room >= 1 - 1e-9:
            out.append('I'); room -= 1
        else:
            out.append('i')
    return ''.join(out)
def fit(p):
    ok = []
    for di in range(0, 301):
        d = di / 100
        for ri in range(-4000, 1501):
            R0 = ri / 100
            if sim(R0, d, len(p)) == p:
                ok.append((R0, d))
    if not ok:
        return None
    ds = sorted(set(x[1] for x in ok))
    return (min(x[0] for x in ok), max(x[0] for x in ok), ds[0], ds[-1])
if __name__ == '__main__':
    for p in sys.argv[1:]:
        print(p, fit(p))
