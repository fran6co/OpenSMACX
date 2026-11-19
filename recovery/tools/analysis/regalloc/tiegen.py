#!/usr/bin/env python3
"""tiegen.py: random exact-tie probes for the tie-break study (MATCHING_GUIDE 2.7 "Tie-breaking").

Builds random straight/if/else programs over candidates in gen.py's spec language, computes rule-4
priorities (sum over references of n_b, n_b = distinct candidates referenced in the block), adjusts one
reference count so that two candidates tie exactly, compiles each probe alone (ties.py) and prints, per
probe, the winner of the tied pair and the features of both candidates.

    python3 tiegen.py --count 200 --seed 1 [--others 1] [--loops] > ties.tsv
"""
import sys, os, random, argparse, json
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ties


def blocks_of(spec):
    """-> list of (block index, depth, token) for each token; tokens [ | ] { } start new blocks."""
    out = []
    b = 0
    depth = 0
    for t in spec.split():
        if t == '[':
            out.append((b, depth, 'cond'))
            b += 1
        elif t == '|':
            b += 1
        elif t == ']':
            b += 1
        elif t == '{':
            b += 1            # loop header/body block
            depth += 1
            out.append((b, depth, 'cond'))
        elif t == '}':
            depth -= 1
            b += 1
        else:
            out.append((b, depth, t))
    return out


def refs(tok):
    """token -> [(var, count)]"""
    if len(tok) >= 2 and tok[0] in 'DS' and tok[1].islower() and len(tok) == 2:
        return [(tok[1], 1)]
    if tok[0].islower() and (len(tok) == 1 or tok[1:].isdigit()):
        return [(tok[0], int(tok[1:] or 1))]
    return []


def priorities(spec, cands):
    import tiemodel
    pr, holes, first, start = tiemodel.analyse(spec, cands)
    _, feat = priorities_old(spec, cands)
    return pr, feat


def priorities_old(spec, cands):
    bl = blocks_of(spec)
    used = {}
    for b, dep, t in bl:
        for v, k in refs(t):
            used.setdefault(b, set()).add(v)
    pr = {v: 0 for v in cands}
    feat = {v: {} for v in cands}
    for i, (b, dep, t) in enumerate(bl):
        for v, k in refs(t):
            if v not in pr:
                continue
            nb = len([x for x in used[b] if x in cands])
            pr[v] += k * nb * (2 ** dep)
            f = feat[v]
            f.setdefault('first_pos', i)
            f.setdefault('first_blk', b)
            f['last_pos'] = i
            f['last_blk'] = b
            f.setdefault('blocks', set()).add(b)
            f['nrefs'] = f.get('nrefs', 0) + k
    for v in feat:
        if 'blocks' in feat[v]:
            feat[v]['nblk_ref'] = len(feat[v]['blocks'])
            del feat[v]['blocks']
    return pr, feat


def rand_prog(rng, cands, loops=False):
    """random program: list of segments; each segment = list of tokens; definitions placed at first use."""
    segs = []
    n = rng.randint(2, 5)
    for _ in range(n):
        kind = rng.choice(['s', 'i', 'e'] + (['w'] if loops else []))
        def blk():
            vs = [v for v in cands if rng.random() < 0.5]
            toks = []
            for v in vs:
                toks.append('%s%d' % (v, rng.randint(1, 4)))
            rng.shuffle(toks)
            return toks or ['C']
        if kind == 's':
            segs.append(blk())
        elif kind == 'i':
            segs.append(['['] + blk() + [']'])
        elif kind == 'e':
            segs.append(['['] + blk() + ['|'] + blk() + [']'])
        else:
            segs.append(['{'] + blk() + ['}'])
    toks = [t for s in segs for t in s]
    # definitions: each candidate defined right before its first use (if used only in an arm, define up front)
    out = []
    defined = set()
    for t in toks:
        r = refs(t)
        for v, k in r:
            if v not in defined:
                defined.add(v)
                out.append('D' + v)
                if k > 1:
                    t = '%s%d' % (v, k - 1) if k > 2 else v
                else:
                    t = None
        if t:
            out.append(t)
    # guarantee final uses so ranges reach the end sometimes
    if rng.random() < 0.5:
        out += [v for v in cands if v in defined]
    # definitions inside an arm are illegal for later use: hoist any D that sits after '[' / '|' / '{' at depth>0
    depth = 0
    fixed = []
    hoist = []
    for t in out:
        if t in ('[', '{'):
            depth += 1
        elif t in (']', '}'):
            depth -= 1
        if t.startswith('D') and len(t) == 2 and depth > 0:
            hoist.append(t)
            fixed.append(t[1])  # keep the reference
        else:
            fixed.append(t)
    return ' '.join(hoist + fixed)


def make_tie(rng, spec, a, b, cands):
    """add references of a or b to one of its blocks to make pr[a] == pr[b]; None if impossible"""
    pr, feat = priorities(spec, cands)
    if a not in feat or b not in feat or not feat[a] or not feat[b]:
        return None
    toks = spec.split()
    for _ in range(20):
        diff = pr[b] - pr[a]
        if diff == 0:
            return ' '.join(toks)
        lo = a if diff > 0 else b
        cand = [i for i, t in enumerate(toks) if refs(t) and refs(t)[0][0] == lo and not t.startswith('D')]
        if not cand:
            return None
        i = rng.choice(cand)
        k = refs(toks[i])[0][1]
        toks[i] = '%s%d' % (lo, k + 1)
        spec2 = ' '.join(toks)
        pr2, _ = priorities(spec2, cands)
        if abs(pr2[b] - pr2[a]) > abs(diff):
            return None
        pr = pr2
    pr, _ = priorities(' '.join(toks), cands)
    return ' '.join(toks) if pr[a] == pr[b] else None


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('--count', type=int, default=100)
    ap.add_argument('--seed', type=int, default=1)
    ap.add_argument('--others', type=int, default=0)
    ap.add_argument('--loops', action='store_true')
    a = ap.parse_args()
    rng = random.Random(a.seed)
    cands = 'cd' + 'pq'[:a.others]
    probes = []
    while len(probes) < a.count:
        spec = rand_prog(rng, cands, a.loops)
        t = make_tie(rng, spec, 'c', 'd', cands)
        if not t:
            continue
        pr, feat = priorities(t, cands)
        if pr['c'] < 4:
            continue
        # others must not tie with c/d
        if any(pr[o] == pr['c'] for o in cands[2:]):
            continue
        probes.append((t, pr, feat))
    jobs = [((i, 0), ties.source('', '', s, 0)) for i, (s, pr, feat) in enumerate(probes)]
    res = ties.evaluate(jobs)
    for i, (s, pr, feat) in enumerate(probes):
        h = res[(i, 0)]
        rc = ties.cell(h, 'c'); rd = ties.cell(h, 'd')
        rank = 'SDBPm'
        win = 'c' if rank.index(rc if rc in rank else 'm') < rank.index(rd if rd in rank else 'm') else 'd' if rc != rd else '='
        print(json.dumps({'spec': s, 'pr': pr, 'reg': {v: ties.cell(h, v) for v in cands}, 'win': win, 'feat': feat}))
