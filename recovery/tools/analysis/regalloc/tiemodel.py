#!/usr/bin/env python3
"""tiemodel.py: CFG, liveness, rule-4 priority and tie-break features for gen.py specs (if/else/while).

    priority(v) = sum over blocks b referencing v of refs(v,b) * n_b * 2^depth(b)       (guide 2.7 rule 4)
    holes(v)    = blocks where v is live through (live in and out, not referenced) while some other
                  candidate is referenced there                                           (tie-break 1)
    first(v)    = position of v's first reference (definition)                            (tie-break 2)
"""
import re


class Block:
    def __init__(self, i, depth):
        self.i = i
        self.depth = depth
        self.refs = []       # (var, count, is_def) in order
        self.succ = []


def tok_refs(t):
    if re.fullmatch(r'[DS][a-z]', t):
        return [(t[1], 1, True)]
    m = re.fullmatch(r'([a-z])(\d*)', t)
    if m:
        return [(m.group(1), int(m.group(2) or 1), False)]
    return []


def cfg(spec):
    """Supports straight tokens, [ ... ], [ ... | ... ], { ... } (while loop: header block + body)."""
    blocks = []

    def new(depth):
        b = Block(len(blocks), depth)
        blocks.append(b)
        return b
    toks = spec.split()
    cur = new(0)
    stack = []   # ('if', test_block, then_end or None) / ('loop', header)
    depth = 0
    for t in toks:
        if t == '[':
            test = cur
            arm = new(depth)
            test.succ.append(arm)
            stack.append(['if', test, None])
            cur = arm
        elif t == '|':
            fr = stack[-1]
            fr[2] = cur
            arm = new(depth)
            fr[1].succ.append(arm)
            fr.append('else')
            cur = arm
        elif t == ']':
            fr = stack.pop()
            join = new(depth)
            if len(fr) == 4:          # if/else
                fr[2].succ.append(join)
                cur.succ.append(join)
            else:
                fr[1].succ.append(join)
                cur.succ.append(join)
            cur = join
        elif t == '{':
            depth += 1
            head = new(depth)
            cur.succ.append(head)
            body = new(depth)
            head.succ.append(body)
            stack.append(['loop', head])
            cur = body
        elif t == '}':
            fr = stack.pop()
            cur.succ.append(fr[1])
            depth -= 1
            ex = new(depth)
            fr[1].succ.append(ex)
            cur = ex
        else:
            for v, k, d in tok_refs(t):
                cur.refs.append((v, k, d))
    return blocks


def liveness(blocks, cands):
    use = []
    dfn = []
    for b in blocks:
        u, d = set(), set()
        for v, k, isdef in b.refs:
            if v not in cands:
                continue
            if isdef and v not in u:
                d.add(v)
            elif not isdef and v not in d:
                u.add(v)
        use.append(u)
        dfn.append(d)
    lin = [set() for _ in blocks]
    lout = [set() for _ in blocks]
    ch = True
    while ch:
        ch = False
        for b in reversed(blocks):
            o = set().union(*[lin[s.i] for s in b.succ]) if b.succ else set()
            i = use[b.i] | (o - dfn[b.i])
            if o != lout[b.i] or i != lin[b.i]:
                lout[b.i], lin[b.i] = o, i
                ch = True
    return lin, lout


def analyse(spec, cands):
    blocks = cfg(spec)
    lin, lout = liveness(blocks, cands)
    pr = {v: 0 for v in cands}
    holes = {v: 0 for v in cands}
    first = {}
    start = {}
    for b in blocks:
        for v, k, d in b.refs:
            start.setdefault(v, b.i)
    for b in blocks:
        here = [v for v, k, d in b.refs if v in cands]
        nb = len(set(here))
        for v, k, d in b.refs:
            if v in pr:
                pr[v] += k * nb * 2 ** b.depth
        for v in cands:
            if v not in here and v in lin[b.i] and v in lout[b.i] and nb:
                holes[v] += nb * 2 ** b.depth
    # first reference position in source order
    for i, t in enumerate(spec.split()):
        for v, k, d in tok_refs(t):
            first.setdefault(v, i)
    return pr, holes, first, start


def winner(spec, a, b, cands=None):
    cands = cands or (a, b)
    pr, holes, first, start = analyse(spec, cands)
    ka = (-pr[a], holes[a], -start.get(a, -1), first.get(a, 1e9))
    kb = (-pr[b], holes[b], -start.get(b, -1), first.get(b, 1e9))
    return (a if ka < kb else b), pr, holes
