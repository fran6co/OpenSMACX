#!/usr/bin/env python3
"""packrev.py <unit> <0xADDR> [--src ALT.cpp] [--pfx N] [--max-moves K] [--units] [--slots S,..]
Reverse frame-slot packing (MATCHING_GUIDE 3.3 "Frame slots are packed last-fit ..."): packsim.py's model run on
the IMAGE, to infer the original's variable set from its slot occupancy.

Image units: every web of every image frame slot (live.image webs) is labelled with our variable through the fe
alignment (same instruction shape, /FAsc operand name).  Hypothesis H0 groups the webs of one image slot that
carry the same named variable of ours into one unit; temporaries (`$T`, `tv`, unnamed), unaligned webs and
webs of different names are separate units.  Each unit has its image raw count (frame accesses) and its home
liveness on the image's flow graph.  The model (descending count, last-fit, interference = home liveness)
is run on these units and compared with the image's actual slots.

A unit the model places elsewhere than the image means the grouping is wrong: some variable is heavier or
lighter in the original than in ours.  The tool then searches groupings greedily (each move re-runs the model):
  split V@S     the webs our variable V has in image slot S are several variables in the original (roles);
                printed with the web split that explains the image (lighter units pack later)
  merge V+W@S   our variables V and W in image slot S are one variable in the original (one name reused)
and prints every accepted move with the agreement it buys (units and accesses whose model slot = image slot).
Also printed:
  SPLIT ACROSS SLOTS  our variable whose webs sit in several image slots: one name per image slot (packing can
                      never put one variable in two slots, so these are certain)
  BLOCKERS            per disagreeing unit: the image slot the model wanted, and the occupant whose (heavier)
                      count makes that slot exist too early / the interference that keeps it out
  OFFSET ORDER        image slot totals against offsets (the model's step 4 must hold on the image)
Screen only.  Register-resident stretches have no frame access, so counts are frame accesses as in packsim."""
import sys, os, re, argparse, collections
H = os.path.dirname(os.path.abspath(__file__))
T_ = os.path.dirname(H)
R = os.path.dirname(os.path.dirname(T_))
sys.path.insert(0, T_)
sys.path.insert(0, H)
os.chdir(R)
import fe
import regprio
import varset
import live

TEMP = re.compile(r'^(\$T\d+|tv\d+|T-?0x)')


def build(unit, src, addr, pfx):
    text = open(src, encoding='latin-1').read()
    c, err = fe.compile_unit(unit, src, pfx)
    if c is None:
        sys.exit('COMPILE FAIL ' + str(err))
    r = fe.evaluate(c, text, addr)
    if 'error' in r:
        sys.exit(r['error'])
    items, frame = regprio.cod_listing(unit, src, pfx, r['sym'])
    name_at, line_of, cur = {}, {}, 0
    for it in items:
        if it[0] == 'line':
            cur = it[1]
        elif it[0] == 'ins':
            line_of[it[1]] = cur
            m = varset.SYM.search(it[2])
            if m:
                name_at[it[1]] = varset.varname(m.group(1))
    sym, lo, hi = fe.ident(text, addr)
    I = live.image(lo, hi)
    # our name per image instruction (aligned, same shape) and our slots per name
    shape = lambda s: varset.SLOT.sub('[S]', s)
    lab = {}
    our_slots = collections.defaultdict(set)
    our_cnt = collections.Counter()
    oline = {}
    for k, o, c_ in r['rows']:
        if c_ is not None:
            so = varset.slot(c_[1])
            if so is not None and so < 0:
                nm = name_at.get(c_[0], 'T%s' % hex(so))
                our_slots[nm].add(so)
                our_cnt[nm] += 1
        if o is None or c_ is None:
            continue
        si, so = varset.slot(o[1]), varset.slot(c_[1])
        if si is None or so is None or shape(o[1]) != shape(c_[1]):
            continue
        nm = name_at.get(c_[0], 'T%s' % hex(so))
        lab[lo + o[0]] = nm
        oline[lo + o[0]] = line_of.get(c_[0], 0)
    return r, I, lab, our_slots, our_cnt, oline


def webs_labelled(F, slot, lab, A):
    """def-use webs split where the aligned labels disagree: a use reached by stores of differently named
    variables is a maybe-uninitialised read (live-and-defined keeps such variables apart, probe g13), so a web
    links only def/use pairs whose labels agree (an unlabelled access joins a neighbour when all its labelled
    neighbours agree)."""
    defs = [n for n, i in enumerate(F.ins) if slot in i['wr']]
    uses = [n for n, i in enumerate(F.ins) if slot in i['rd']]
    L = lambda n: lab.get(A[n])
    edges = []
    from_entry = set()
    for d in defs + ['entry']:
        seen = set()
        work = list(F.ins[d]['s']) if d != 'entry' else [0]
        while work:
            n = work.pop()
            if n in seen:
                continue
            seen.add(n)
            i = F.ins[n]
            if slot in i['rd']:
                if d == 'entry':
                    from_entry.add(n)
                elif n != d:
                    edges.append((d, n))
            if slot in i['wr']:
                continue
            work.extend(i['s'])
    parent = {}

    def find(x):
        while parent.setdefault(x, x) != x:
            parent[x] = parent[parent[x]]
            x = parent[x]
        return x
    nb = collections.defaultdict(set)
    for d, u in edges:
        nb[d].add(u)
        nb[u].add(d)
        if L(d) is not None and L(d) == L(u) or L(d) is None and L(u) is None:
            parent[find(d)] = find(u)
    for n in defs + uses:
        if L(n) is None:
            ls = {L(m) for m in nb[n] if L(m) is not None}
            if len(ls) == 1:
                for m in nb[n]:
                    if L(m) is not None:
                        parent[find(n)] = find(m)
                        break
    groups = collections.defaultdict(lambda: (set(), set()))
    for d in defs:
        groups[find(d)][0].add(d)
    for u in uses:
        g = groups[find(u)]
        g[1].add(u)
        if u in from_entry:
            g[0].add('entry')
    return list(groups.values())


def webs_noentry(F, slot):
    """live.Func.webs without the entry pseudo-definition as a union point (an out-parameter written through a
    lea'd address looks uninitialised and would chain unrelated webs); a web reached from entry gets 'entry'."""
    defs = [n for n, i in enumerate(F.ins) if slot in i['wr']]
    uses = [n for n, i in enumerate(F.ins) if slot in i['rd']]
    parent = {}

    def find(x):
        while parent.setdefault(x, x) != x:
            parent[x] = parent[parent[x]]
            x = parent[x]
        return x
    from_entry = set()
    for d in defs + ['entry']:
        seen = set()
        work = list(F.ins[d]['s']) if d != 'entry' else [0]
        while work:
            n = work.pop()
            if n in seen:
                continue
            seen.add(n)
            i = F.ins[n]
            if slot in i['rd']:
                if d == 'entry':
                    from_entry.add(n)
                else:
                    parent[find(('d', d))] = find(('u', n))
            if slot in i['wr']:
                continue
            work.extend(i['s'])
    groups = collections.defaultdict(lambda: (set(), set()))
    for d in defs:
        groups[find(('d', d))][0].add(d)
    for u in uses:
        g = groups[find(('u', u))]
        g[1].add(u)
        if u in from_entry:
            g[0].add('entry')
    return list(groups.values())


def hx(s):
    return 'new' if s == 1 << 20 else hex(s)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('unit')
    ap.add_argument('addr')
    ap.add_argument('--src')
    ap.add_argument('--pfx', default=os.environ.get('FE_PFX_BASE', '29'))
    ap.add_argument('--max-moves', type=int, default=12)
    ap.add_argument('--units', action='store_true', help='print every image unit')
    ap.add_argument('--raw-webs', action='store_true', help='plain def-use webs (no split at label changes)')
    ap.add_argument('--slots', default='', help='print the web timeline of these image slots')
    a = ap.parse_args()
    unit = a.unit if a.unit.startswith('src/') else 'src/' + a.unit
    src = os.path.abspath(a.src) if a.src else R + '/' + unit
    addr = int(a.addr, 16)
    r, I, lab, our_slots, our_cnt, oline = build(unit, src, addr, a.pfx)
    A = [i['a'] for i in I.ins]
    aggregate = {n for n, s in our_slots.items() if len(s) > 1 and not TEMP.match(n)}
    slots = sorted({s for i in I.ins for s in i['rd'] | i['wr'] if isinstance(s, int) and s < 0}, reverse=True)
    fixed = set()
    # C++ EH registration: [ebp-4] state, [ebp-8] handler, [ebp-0xc] previous record (push -1; push handler)
    head = ' '.join(i['txt'] for i in I.ins[:6])
    if 'fs:[0]' in head or 'push -1' in head or 'push 0xffffffff' in head:
        fixed |= {-4, -8, -0xc}
    # address-taken slots: a scalar of ours (one slot, named) packs like any other unit, its liveness being the
    # region between its accesses (block scope); arrays, aggregates and unlabelled ones stay fixed
    leas = collections.defaultdict(list)
    for n, i in enumerate(I.ins):
        for s in i['rd']:
            if isinstance(s, tuple) and s[0] == 'addr':
                leas[s[1]].append(n)
    for s, ns in leas.items():
        for n in ns:
            nm = lab.get(A[n])
            if nm is None or nm in aggregate or TEMP.match(nm):
                fixed.add(s)
    # webs -> labelled pieces
    webs = []  # (slot, idxset, label)
    multi = []
    for s in slots:
        for defs, uses in (webs_noentry(I, s) if a.raw_webs else webs_labelled(I, s, lab, A)):
            idx = {x for x in defs | uses if x != 'entry'}
            if not idx:
                continue
            names = collections.Counter(lab[A[x]] for x in idx if A[x] in lab)
            nm = names.most_common(1)[0][0] if names else None
            named = {k: v for k, v in names.items() if not TEMP.match(k)}
            if len(named) > 1:
                multi.append((s, idx, named))
            if nm in aggregate:
                fixed.add(s)
            webs.append(dict(slot=s, idx=idx, name=nm, entry='entry' in defs, addr=False,
                             lines=sorted({oline[A[x]] for x in idx if A[x] in oline})))
        for n in leas.get(s, ()):
            webs.append(dict(slot=s, idx={n}, name=lab.get(A[n]), entry=False, addr=True,
                             lines=[oline[A[n]]] if A[n] in oline else []))
    webs = [w for w in webs if w['slot'] not in fixed]
    for n, w in enumerate(webs):
        w['id'] = n

    def is_named(nm):
        return nm is not None and not TEMP.match(nm)

    # liveness of a set of instruction indices in one slot (packsim.relabel on the image)
    LV = {}

    succ = [i['s'] for i in I.ins]

    def reach(starts, nxt):
        seen = set()
        work = [x for st in starts for x in nxt[st]]
        while work:
            n = work.pop()
            for p in nxt[n]:
                if p not in seen:
                    seen.add(p)
                    work.append(p)
        return seen

    def liveness(key, s, mine, addr=False):
        if key in LV:
            return LV[key]
        if addr:
            liv = (reach(mine, succ) | set(mine)) & (reach(mine, I.pred) | set(mine))
            LV[key] = (liv, set())
            return LV[key]
        rd = [n for n in mine if s in I.ins[n]['rd']]
        wr = {n for n in mine if s in I.ins[n]['wr']}
        liv = set(rd)
        work = list(rd)
        while work:
            n = work.pop()
            for p in I.pred[n]:
                if p in liv:
                    continue
                if p in mine and s in I.ins[p]['wr'] and s not in I.ins[p]['rd']:
                    continue
                liv.add(p)
                work.append(p)
        # live AND defined (probe tempslot/g13): the home is free where none of the unit's own stores reaches,
        # so a maybe-uninitialised read does not hold the slot over the other arm
        if wr:
            liv &= reach(wr, succ) | wr
        LV[key] = (liv, wr)
        return LV[key]

    def units_of(groups):
        """groups: list of frozenset(web ids) -> list of unit dicts"""
        out = []
        for g in groups:
            ws = [webs[i] for i in g]
            s = ws[0]['slot']
            mine = set().union(*(w['idx'] for w in ws))
            out.append(dict(webs=g, slot=s, n=len(mine), live=liveness(g, s, mine, any(w['addr'] for w in ws)),
                            name='+'.join(sorted({w['name'] or '?' for w in ws}))))
        return out

    def interferes(x, y):
        lx, wx = x['live']
        ly, wy = y['live']
        return bool((lx & (ly | wy)) or (ly & wx))

    IF = {}

    def intf(x, y):
        k = frozenset((x['webs'], y['webs']))
        if k not in IF:
            IF[k] = interferes(x, y)
        return IF[k]

    def run(groups):
        U = units_of(groups)
        order = sorted(U, key=lambda u: (-u['n'], -u['slot']))
        G = []
        where = {}
        for u in order:
            for g in reversed(G):
                if not any(intf(u, m) for m in g):
                    g.append(u)
                    where[u['webs']] = g
                    break
            else:
                g = [u]
                G.append(g)
                where[u['webs']] = g
        # one-to-one: groups claim image slots by shared accesses, heaviest overlap first
        ov = []
        for g in G:
            cnt = collections.Counter()
            for m in g:
                cnt[m['slot']] += m['n']
            for s_, k in cnt.items():
                ov.append((-k, id(g), s_))
        ov.sort()
        gs, taken = {}, set()
        for k, gi, s_ in ov:
            if gi in gs or s_ in taken:
                continue
            gs[gi] = s_
            taken.add(s_)
        for g in G:
            gs.setdefault(id(g), 1 << 20)
        ok = [u for u in U if gs[id(where[u['webs']])] == u['slot']]
        return U, G, where, gs, len(ok), sum(u['n'] for u in ok), sum(u['n'] for u in U)

    # H0
    by = collections.defaultdict(list)
    for w in webs:
        key = (w['slot'], w['name']) if is_named(w['name']) else (w['slot'], 'w%d' % w['id'])
        by[key].append(w['id'])
    H0 = [frozenset(v) for v in by.values()]
    U, G, where, gs, ok, wok, tot = run(H0)
    print('FE %08X %s diff=%d   image slots %d (address-taken/aggregate %d), webs %d, units %d' % (
        addr, r['sym'], r['diff'], len(slots), len(fixed), len(webs), len(U)))
    print('H0 (our names): model slot = image slot for %d of %d units (%d of %d accesses); model slots %d, image %d' % (
        ok, len(U), wok, tot, len(G), len({u['slot'] for u in U})))

    print('\nMULTI-NAME WEBS (one image def-use web carrying several of our names; NOT proof of one variable: two '
          'variables on exclusive paths share a web by flow and still pack, probe g13 `exclusive`)')
    for s, idx, named in multi:
        print('  %6s img %x-%x n=%d: %s' % (hex(s), A[min(idx)], A[max(idx)], len(idx), ' '.join(
            '%s:%d' % kv for kv in sorted(named.items(), key=lambda kv: -kv[1]))))
    print('\nMUST-MERGE (two of our names in one image slot whose homes interfere by live-and-defined flow: '
          'separate variables could not share it)')
    byslot = collections.defaultdict(list)
    for u in U:
        if u['name'] != '?' and not TEMP.match(u['name']):
            byslot[u['slot']].append(u)
    for s_, us in sorted(byslot.items(), reverse=True):
        for i in range(len(us)):
            for j in range(i + 1, len(us)):
                if us[i]['name'] != us[j]['name'] and intf(us[i], us[j]):
                    print('  %6s %s(%d) + %s(%d)' % (hex(s_), us[i]['name'], us[i]['n'], us[j]['name'], us[j]['n']))
    # split across slots
    nslots = collections.defaultdict(collections.Counter)
    for w in webs:
        if is_named(w['name']):
            nslots[w['name']][w['slot']] += len(w['idx'])
    print('\nSPLIT ACROSS SLOTS (our variable -> image slots: accesses)')
    for nm, cn in sorted(nslots.items(), key=lambda x: -sum(x[1].values())):
        if len(cn) > 1:
            print('  %-18s %s' % (nm, ' '.join('%s:%d' % (hex(s), k) for s, k in cn.most_common())))

    # image slot table
    occ = collections.defaultdict(list)
    for u in U:
        occ[u['slot']].append(u)
    print('\nIMAGE SLOTS (offset total | units by count: name(n) [model slot if different])')
    for s in sorted(occ, reverse=True):
        us = sorted(occ[s], key=lambda u: -u['n'])
        print('  %6s %4d | %s' % (hex(s), sum(u['n'] for u in us), ' '.join(
            '%s(%d)%s' % (u['name'], u['n'], '' if gs[id(where[u['webs']])] == s else '[->%s]' % hx(gs[id(where[u['webs']])]))
            for u in us[:10]) + (' ...' if len(us) > 10 else '')))
    offs = sorted(occ, reverse=True)
    tots = [sum(u['n'] for u in occ[o]) for o in offs]
    inv = sum(1 for i in range(len(tots)) for j in range(i + 1, len(tots)) if tots[i] < tots[j])
    print('OFFSET ORDER: %d inversions of slot totals against offsets (%d pairs)' % (inv, len(offs) * (len(offs) - 1) // 2))

    # blockers: per disagreeing unit, why
    print('\nBLOCKERS (H0; image unit in S, model put it in M)')
    founder = {}
    for s in occ:
        founder[s] = max(occ[s], key=lambda u: u['n'])
    for u in sorted(U, key=lambda u: -u['n']):
        m = gs[id(where[u['webs']])]
        if m == u['slot']:
            continue
        S = u['slot']
        why = []
        blk = [x for x in occ[S] if x is not u and x['n'] >= u['n'] and intf(u, x)]
        if blk:
            why.append('own slot %s holds interfering %s' % (hex(S), ','.join('%s(%d)' % (x['name'], x['n']) for x in blk[:3])))
        if m in occ:
            f = founder[m]
            fS = founder[S]
            if not any(intf(u, x) for x in occ[m] if x['n'] >= u['n']):
                why.append('%s free over it, founder %s(%d) vs %s founder %s(%d): needs %s lighter than %d or heavier than %d' % (
                    hex(m), f['name'], f['n'], hex(S), fS['name'], fS['n'], f['name'], u['n'], fS['n']))
        print('  %-24s n=%-3d %s -> %s  L%s  %s' % (u['name'], u['n'], hex(S), hx(m), ','.join(map(str, sorted(
            set().union(*(webs[i]['lines'] for i in u['webs'])))[:5])), '; '.join(why)))

    # greedy search over groupings
    print('\nSEARCH (greedy moves on H0; score = accesses whose model slot = image slot)')
    cur = list(H0)
    best = (wok, ok - len(U))
    for step in range(a.max_moves):
        cands = []
        for g in cur:
            if len(g) > 1:
                # split this unit into its webs
                nm = webs[next(iter(g))]['name']
                cands.append(('split %s@%s into %d webs' % (nm, hex(webs[next(iter(g))]['slot']), len(g)),
                              [x for x in cur if x != g] + [frozenset([i]) for i in g]))
                # split off each single web
                if len(g) > 2:
                    for i in g:
                        w = webs[i]
                        cands.append(('split %s@%s: web L%s (n=%d) is its own variable' % (
                            w['name'], hex(w['slot']), ','.join(map(str, w['lines'][:4])), len(w['idx'])),
                            [x for x in cur if x != g] + [frozenset([i]), g - {i}]))
        bys = collections.defaultdict(list)
        for g in cur:
            bys[webs[next(iter(g))]['slot']].append(g)
        for s, gl in bys.items():
            for i in range(len(gl)):
                for j in range(i + 1, len(gl)):
                    n1 = '+'.join(sorted({webs[x]['name'] or '?' for x in gl[i]}))
                    n2 = '+'.join(sorted({webs[x]['name'] or '?' for x in gl[j]}))
                    cands.append(('merge %s + %s @%s' % (n1, n2, hex(s)),
                                  [x for x in cur if x is not gl[i] and x is not gl[j]] + [gl[i] | gl[j]]))
        scored = []
        for desc, h in cands:
            res = run(h)
            scored.append(((res[5], res[4] - len(res[0])), desc, h))
        scored.sort(key=lambda x: (-x[0][0], -x[0][1]))
        if not scored or scored[0][0] <= best:
            break
        sc, desc, h = scored[0]
        print('  %2d %-70s %d -> %d accesses (%d units off)' % (step + 1, desc, best[0], sc[0], -sc[1]))
        best = sc
        cur = h
    U2, G2, where2, gs2, ok2, wok2, tot2 = run(cur)
    print('final: %d of %d units, %d of %d accesses' % (ok2, len(U2), wok2, tot2))

    if a.units or a.slots:
        want = {int(x, 16) for x in a.slots.split(',') if x}
        print('\nWEBS')
        for w in sorted(webs, key=lambda w: (-w['slot'], min(w['idx']))):
            if want and w['slot'] not in want:
                continue
            print('  %6s %-18s n=%-3d img %x-%x L%s' % (hex(w['slot']), w['name'], len(w['idx']), A[min(w['idx'])],
                                                     A[max(w['idx'])], ','.join(map(str, w['lines'][:6]))))


if __name__ == '__main__':
    main()
