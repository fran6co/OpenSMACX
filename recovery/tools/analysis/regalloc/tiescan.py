#!/usr/bin/env python3
"""tiescan.py <feall.json> [--pfx N]: classify non-exact functions whose difference is only a permutation of
callee-saved registers (a register tie or swapped pair).  For each function with equal lengths and the same
mnemonics, tries every bijection of {esi, edi, ebx, ebp} on our side and reports the best: PERM (all rows
equal under one renaming), PERM~k (k rows still differ), or nothing.  Run under `uv run python`."""
import sys, os, re, json, itertools, collections
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
import fe
CS = ['esi', 'edi', 'ebx', 'ebp']
SUB = {'esi': ['si'], 'edi': ['di'], 'ebx': ['bx', 'bl', 'bh'], 'ebp': ['bp']}


def rename(text, perm):
    m = {}
    for a, b in perm.items():
        m[a] = b
        for x, y in zip(SUB[a], SUB[b]):
            m[x] = y
    return re.sub(r'\b(esi|edi|ebx|ebp|si|di|bx|bl|bh|bp)\b', lambda k: m.get(k.group(1), k.group(1)), text)


def classify(r):
    rows = r['rows']
    pairs = [(o[1], c[1]) for k, o, c in rows if o is not None and c is not None]
    if any(o is None or c is None for k, o, c in rows):
        return None
    if any(a.split()[0] != b.split()[0] for a, b in pairs):
        return None
    diff = [(a, b) for a, b in pairs if a != b]
    best = None
    for p in itertools.permutations(CS):
        perm = dict(zip(CS, p))
        if all(perm[x] == x for x in CS):
            continue
        n = sum(1 for a, b in diff if rename(b, perm) != a)
        if best is None or n < best[0]:
            best = (n, {k: v for k, v in perm.items() if k != v})
    return best, len(diff)


if __name__ == '__main__':
    data = json.load(open(sys.argv[1]))
    pfx = sys.argv[sys.argv.index('--pfx') + 1] if '--pfx' in sys.argv else '8'
    byunit = collections.defaultdict(list)
    for d in data:
        if not d.get('exact') and d.get('diff', -1) > 0 and d['img'] == d['ours']:
            byunit[d['unit']].append(d)
    for u, ds in sorted(byunit.items()):
        text = open(fe.R + '/' + u, encoding='latin-1').read()
        c, err = fe.compile_unit(u, fe.R + '/' + u, pfx)
        if c is None:
            continue
        for d in ds:
            r = fe.evaluate(c, text, int(d['addr'], 16))
            if 'error' in r:
                continue
            res = classify(r)
            if res is None:
                continue
            (n, perm), nd = res
            tag = 'PERM' if n == 0 else 'PERM~%d' % n
            if n < nd:
                print('%-8s %s %-30s %s diff=%d rows %s' % (tag, d['addr'], u, d['sym'][:40], r['diff'], perm), flush=True)
