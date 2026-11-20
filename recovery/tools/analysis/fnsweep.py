#!/usr/bin/env python3
"""fnsweep.py <unit> <addr> [--rules r1,r2,..] [--steps N] [--write] [--json OUT] [--min-hunks K]

Single-site rewrite sweep of one function, ranked by the sdeck structural key (EH distance, named call
distance, jcc/jmp/call/ret count distance, control-flow hunks, rows). Every candidate site of the rules
(rules.py RULES; default ifswap,elifswap,stmtswap,ternary,earlyret,declswap,cmpswap) inside the function's
source span is applied alone and evaluated; the best site that beats the base is taken, the function is
re-swept, and so on for up to --steps rounds (greedy until flat). --write writes the unit after every
accepted step (so a caller can commit per step: run with --steps 1 --write, commit, rerun).

Comparison (better()): EH distance, then named call distance; when the branch/call count distance of the two
keys differs by at most 1, control-flow hunks decide before that count, then rows; otherwise count, hunks, rows.
Lanes FE_PFX_BASE..FE_PFX_BASE+FE_JOBS-1. Screens only; confirm with `recover measure`."""
import sys, os, re, json
from concurrent.futures import ThreadPoolExecutor
S = os.path.dirname(os.path.abspath(__file__)); T = os.path.dirname(S)
sys.path.insert(0, T); sys.path.insert(0, S)
import fe, rules as RL, sdeck

DEFAULT = 'ifswap,elifswap,stmtswap,ternary,earlyret,declswap,cmpswap'
MARK = re.compile(r'// ORIGINAL: (0x[0-9A-F]{8}) \S+ (0x[0-9A-Fa-f]+)-(0x[0-9A-Fa-f]+)')


def better(a, b):
    """True when key a is strictly better than key b (None is worst)."""
    if a is None: return False
    if b is None: return True
    if a[:2] != b[:2]: return a[:2] < b[:2]
    if abs(a[2] - b[2]) <= 1: return (a[3], a[2], a[4]) < (b[3], b[2], b[4])
    return a[2:5] < b[2:5]


def span(t, addr):
    m = next(m for m in MARK.finditer(t) if int(m.group(1), 16) == addr)
    nx = MARK.search(t, m.end())
    return m.start(), (nx.start() if nx else len(t)), int(m.group(2), 16), int(m.group(3), 16)


def candidates(t, addr, names):
    lo, hi, _, _ = span(t, addr); mt = RL.mask(t); out = []
    for r in names:
        for c in RL.RULES[r](t, mt, lo):
            s, e, rep, label = c[:4]
            if lo <= s and e <= hi: out.append((s, e, rep, label))
    return out


def sweep(unit, addr, t, names, base, jobs, verbose=True):
    _, _, ilo, ihi = span(t, addr)
    cands = candidates(t, addr, names)
    B = sdeck.evaluate(unit, addr, ilo, ihi, t, base)
    if verbose: print('BASE', B, 'sites', len(cands), flush=True)
    res = [None] * len(cands)

    def lane(k):
        for i in range(k, len(cands), jobs):
            s, e, rep, label = cands[i]
            r = sdeck.evaluate(unit, addr, ilo, ihi, t[:s] + rep + t[e:], base + k)
            res[i] = r
            if verbose and better(r, B):
                print('  BETTER', r, label, 'L%d' % (t.count('\n', 0, s) + 1), flush=True)
    with ThreadPoolExecutor(jobs) as ex: list(ex.map(lane, range(jobs)))
    return B, cands, res


def main():
    a = sys.argv[1:]
    unit = a[0]; unit = unit[4:] if unit.startswith('src/') else unit; addr = int(a[1], 16)
    opt = lambda k, d=None: a[a.index(k) + 1] if k in a else d
    names = opt('--rules', DEFAULT).split(',')
    steps = int(opt('--steps', '1')); out = opt('--json')
    base = int(os.environ.get('FE_PFX_BASE', '8')); jobs = int(os.environ.get('FE_JOBS', '5'))
    path = fe.R + '/src/' + unit; t = open(path, encoding='latin-1').read()
    log = []
    for step in range(steps):
        B, cands, res = sweep(unit, addr, t, names, base, jobs)
        best = None
        for i, r in enumerate(res):
            if better(r, B) and (best is None or better(r, res[best])): best = i
        if best is None:
            print('FLAT', B); log.append(dict(step=step, base=B, accepted=None)); break
        s, e, rep, label = cands[best]; line = t.count('\n', 0, s) + 1
        print('STEP', step, label, 'L%d' % line, B, '->', res[best], flush=True)
        log.append(dict(step=step, base=B, key=res[best], label=label, line=line))
        t = t[:s] + rep + t[e:]
        if '--write' in a: open(path, 'w', encoding='latin-1').write(t)
    if out: json.dump(log, open(out, 'w'))


if __name__ == '__main__':
    main()
