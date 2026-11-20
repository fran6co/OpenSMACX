#!/usr/bin/env python3
"""fnsweep.py <unit> <addr> [--rules r1,r2,..] [--armwo d,t,c,r] [--steps N] [--fast] [--on-step CMD] [--min-rows K] [--write] [--json OUT]

Single-site rewrite sweep of one function, ranked by the sdeck structural key (EH distance, named call
distance, jcc/jmp/call/ret count distance, control-flow hunks, rows). Every candidate site of the rules
(rules.py RULES; default ifswap,elifswap,stmtswap,ternary,earlyret,declswap,cmpswap) inside the function's
source span is applied alone and evaluated; the best site that beats the base is taken, the function is
re-swept, and so on for up to --steps rounds (greedy until flat). --write writes the unit after every
accepted step, so a caller can commit per step (run with --steps 1 --write, commit, rerun). --armwo adds
armwo.py's whole-function arm spellings of those families (up to ARMWO_MAX) to the same ranking; `--rules ''`
ranks only them.

Comparison (better()): EH distance, then named call distance; when the branch/call count distance of the two
keys differs by at most 1, control-flow hunks decide before that count, then rows; otherwise count, hunks, rows.
--fast: after a step, the next round re-evaluates only the previous round's other winning sites (a full sweep
confirms flat). --on-step CMD runs a shell command after each written step (FNSWEEP_LABEL, FNSWEEP_LINE,
FNSWEEP_KEYS in its environment), e.g. a per-step commit.
--min-rows K: a step that changes nothing but rows must save more than K rows.
Lanes FE_PFX_BASE..FE_PFX_BASE+FE_JOBS-1. Screens only; confirm with `recover measure`."""
import sys, os, re, json, subprocess
from concurrent.futures import ThreadPoolExecutor
S = os.path.dirname(os.path.abspath(__file__)); T = os.path.dirname(S)
sys.path.insert(0, T); sys.path.insert(0, S)
import fe, rules as RL, sdeck

DEFAULT = 'ifswap,elifswap,stmtswap,ternary,earlyret,declswap,cmpswap'
MARK = re.compile(r'// ORIGINAL: (0x[0-9A-F]{8}) \S+ (0x[0-9A-Fa-f]+)-(0x[0-9A-Fa-f]+)')


MIN_ROWS = 0


def better(a, b):
    """True when key a is strictly better than key b (None is worst). A rows-only gain must be over MIN_ROWS."""
    if a is None: return False
    if b is None: return True
    if a[:4] == b[:4]: return a[4] < b[4] - MIN_ROWS
    if a[:2] != b[:2]: return a[:2] < b[:2]
    if abs(a[2] - b[2]) <= 1: return (a[3], a[2], a[4]) < (b[3], b[2], b[4])
    return a[2:5] < b[2:5]


def span(t, addr):
    m = next(m for m in MARK.finditer(t) if int(m.group(1), 16) == addr)
    nx = MARK.search(t, m.end())
    return m.start(), (nx.start() if nx else len(t)), int(m.group(2), 16), int(m.group(3), 16)


def candidates(t, addr, names, arm=None):
    lo, hi, _, _ = span(t, addr); mt = RL.mask(t); out = []
    if arm:
        import armwo
        with open(os.devnull, 'w') as dn:
            deck = armwo.build_deck(t, '0x%08X' % addr, set(arm.split(',')), maxn=int(os.environ.get('ARMWO_MAX', '400'))) or []
        for k, v in deck[1:]:
            d = next((i for i, (x, y) in enumerate(zip(t, v)) if x != y), 0)
            out.append((0, len(t), v, 'armwo-' + k, d))
    for r in names:
        for c in RL.RULES[r](t, mt, lo):
            s, e, rep, label = c[:4]
            if lo <= s and e <= hi: out.append((s, e, rep, label, s))
    return out


def ident(t, c):
    """position-independent identity of a candidate: its rule, the text it replaces and the replacement"""
    s, e, rep, label, at = c
    return (label, t[s:e], rep) if s or e != len(t) else (label, None)


def sweep(unit, addr, t, names, base, jobs, verbose=True, arm=None, only=None):
    _, _, ilo, ihi = span(t, addr)
    cands = candidates(t, addr, [n for n in names if n], arm)
    if only is not None: cands = [c for c in cands if ident(t, c) in only]
    B = sdeck.evaluate(unit, addr, ilo, ihi, t, base)
    if verbose: print('BASE', B, 'sites', len(cands), '(previous winners)' if only is not None else '', flush=True)
    res = [None] * len(cands)

    def lane(k):
        for i in range(k, len(cands), jobs):
            s, e, rep, label, at = cands[i]
            r = sdeck.evaluate(unit, addr, ilo, ihi, t[:s] + rep + t[e:], base + k)
            res[i] = r
            if verbose and better(r, B):
                print('  BETTER', r, label, 'L%d' % (t.count('\n', 0, at) + 1), flush=True)
    with ThreadPoolExecutor(jobs) as ex: list(ex.map(lane, range(jobs)))
    return B, cands, res


def main():
    a = sys.argv[1:]
    unit = a[0]; unit = unit[4:] if unit.startswith('src/') else unit; addr = int(a[1], 16)
    opt = lambda k, d=None: a[a.index(k) + 1] if k in a else d
    names = opt('--rules', DEFAULT).split(','); arm = opt('--armwo')
    steps = int(opt('--steps', '1')); out = opt('--json')
    global MIN_ROWS; MIN_ROWS = int(opt('--min-rows', '0'))
    base = int(os.environ.get('FE_PFX_BASE', '8')); jobs = int(os.environ.get('FE_JOBS', '5'))
    path = fe.R + '/src/' + unit; t = open(path, encoding='latin-1').read()
    hook = opt('--on-step')
    log = []; only = None; step = 0
    while step < steps:
        B, cands, res = sweep(unit, addr, t, names, base, jobs, arm=arm, only=only)
        win = [i for i, r in enumerate(res) if better(r, B)]
        best = None
        for i in win:
            if best is None or better(res[i], res[best]): best = i
        if best is None:
            if only is not None:   # the previous round's winners are spent: confirm with a full sweep
                only = None; continue
            print('FLAT', B); log.append(dict(step=step, base=B, accepted=None)); break
        s, e, rep, label, at = cands[best]; line = t.count('\n', 0, at) + 1
        print('STEP', step, label, 'L%d' % line, B, '->', res[best], flush=True)
        log.append(dict(step=step, base=B, key=res[best], label=label, line=line))
        # --fast: the next round re-evaluates only this round's other winning sites (a full sweep when they are spent)
        only = {ident(t, cands[i]) for i in win if i != best} if '--fast' in a else None
        t = t[:s] + rep + t[e:]
        if '--write' in a: open(path, 'w', encoding='latin-1').write(t)
        if hook: subprocess.run(hook, shell=True, env=dict(os.environ, FNSWEEP_LABEL=label, FNSWEEP_LINE='L%d' % line,
                                                          FNSWEEP_KEYS='%s -> %s' % (B, res[best])))
        step += 1
    if out: json.dump(log, open(out, 'w'))


if __name__ == '__main__':
    main()
