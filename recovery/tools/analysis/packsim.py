#!/usr/bin/env python3
"""packsim.py <unit> <0xADDR> [--src ALT.cpp] [--sym NAME,..] [--why]
Frame-slot packing model check (MATCHING_GUIDE 3.3 "Frame slots are packed last-fit by raw reference count",
probes in analysis/probes/tempslot/).

Model (measured on probes): every memory-resident symbol (a named local, a front-end temporary `$T..`, or an
optimizer temporary = one web of an anonymous frame operand) has a RAW reference count (frame accesses, no
loop weighting; probe g15: a ?: result spilled per arm and a spilled pointer IV count exactly their frame
accesses, not less). Symbols are taken in descending count; each joins the MOST RECENTLY CREATED slot none of
whose occupants interferes with it, else opens a new slot; slots get frame offsets in creation order (-4
first; dead parameter homes are pre-existing slots tried before every local slot, the parameter read first
first, probe g17). Interference is liveness of the HOME (stores to loads of the slot through the flow graph): a value
cached in a register over a stretch leaves its home dead there.

This tool rebuilds the symbols from our compile (/FAsc names, anonymous operands split into webs), computes
each home's live range on our machine code, runs the model and compares the model's slot of every symbol with
our compiler's. Agreement validates the model on this function. With --why, for every symbol whose actual
slot is not the latest-created compatible one, it prints the later slots and the occupant that interferes
(name, its access lines), i.e. the variable whose home our source keeps alive across that temporary.
--sym prints the same reasoning for the named symbols only.
Screen only."""
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

ap = argparse.ArgumentParser()
ap.add_argument('unit')
ap.add_argument('addr')
ap.add_argument('--src')
ap.add_argument('--sym', default='')
ap.add_argument('--why', action='store_true')
ap.add_argument('--fit', default='last', choices=['last', 'first', 'smallest'], help='alternative models for comparison')
ap.add_argument('--pfx', default=os.environ.get('FE_PFX_BASE', '29'))
ap.add_argument('--defined', action='store_true', help='interference on live AND defined (a home is live only where one of '
                'its own stores reaches), not plain flow liveness')
ap.add_argument('--no-tail', dest='tail', action='store_false', help='plain home liveness for optimizer temporaries '
                '(default: a spilled temporary stays live over its scratch-register reload tail, boom 0x00504AA0)')
ap.add_argument('--no-params', dest='params', action='store_false', help='leave dead parameter homes out (the model '
                'before 2026-11-20); default: parameter homes are pre-existing slots tried before every local slot, '
                'earliest-read parameter first (probe g17)')
ap.add_argument('--param-order', default='first', choices=['first', 'last', 'oldest'],
                help='where parameter homes sit among the candidates: first (before local slots, probe g17), last '
                '(after them), oldest (after them, latest-read parameter first)')
ap.add_argument('--count', default='frame', choices=['frame', 'src', 'max', 'reg1', 'reg2', 'reg1all', 'reg2all', 'reg3', 'reg4', 'reg3all', 'reg4all'],
                help='packing count of a named variable (experiments, 2026-11-20; frame accesses stay the best fit): '
                'src = lexical source references (refcount.py), max = larger of src and frame; reg1 = frame + register '
                'uses after each load, reg2 = also after each store; reg3 = a load counts its register uses (at least '
                '1), reg4 = reg3 + uses after stores; *all = temporaries too. Agreement battle_fight/go_to/boom/'
                'enemy_move units: frame 53/46/25/113, src 51/44/25/95, reg1 49/43/24/84, reg3 49/46/24/95')
ap.add_argument('--tie', default='last', choices=['last', 'first', 'actual'],
                help='order among equal counts: earlier LAST reference first (probe g14, default), or the actual slot (oracle)')
a = ap.parse_args()
addr = int(a.addr, 16)
src = os.path.abspath(a.src) if a.src else R + '/' + a.unit
text = open(src, encoding='latin-1').read()
c, err = fe.compile_unit(a.unit, src, a.pfx)
if c is None:
    sys.exit('COMPILE FAIL ' + str(err))
r = fe.evaluate(c, text, addr)
items, frame = regprio.cod_listing(a.unit, src, a.pfx, r['sym'])
name_at, line_of, cur = {}, {}, 0
for it in items:
    if it[0] == 'line':
        cur = it[1]
    elif it[0] == 'ins':
        line_of[it[1]] = cur
        m = varset.SYM.search(it[2])
        if m:
            name_at[it[1]] = varset.varname(m.group(1))
F = live.ours(a.unit, src, addr, a.pfx)
addr_of = [i['a'] for i in F.ins]

# relabel every frame access with its symbol
lab = {}
for n, i in enumerate(F.ins):
    for s in list(i['rd']) + list(i['wr']):
        if isinstance(s, tuple):
            s = s[1]
        if s >= 0 and not (a.params and s >= 8):
            continue
        nm = name_at.get(i['a'])
        lab.setdefault(n, {})[s] = nm
slots_used = sorted({s for d in lab.values() for s in d}, reverse=True)

# symbols: named -> set of instruction indices; anonymous -> webs per slot
sym_ins = collections.defaultdict(set)
sym_slot = {}
addr_taken = set()
for n, d in lab.items():
    for s, nm in d.items():
        if nm is not None:
            sym_ins[nm].add(n)
            sym_slot.setdefault(nm, collections.Counter())[s] += 1
            if ('addr', s) in F.ins[n]['rd']:
                addr_taken.add(nm)
for s in slots_used:
    for defs, uses in F.webs(s):
        idx = {x for x in defs | uses if x != 'entry'}
        anon = {x for x in idx if lab.get(x, {}).get(s, 'X') is None}
        if not anon:
            continue
        nm = 'T%s@%x' % (hex(s), addr_of[min(anon)])
        sym_ins[nm] |= anon
        sym_slot.setdefault(nm, collections.Counter())[s] += len(anon)


def relabel(sym):
    """live set of sym's home: instruction indices where it is live on entry, plus its writes."""
    s = sym_slot[sym].most_common(1)[0][0]
    mine = sym_ins[sym]
    rd = [n for n in mine if s in F.ins[n]['rd'] or ('addr', s) in F.ins[n]['rd']]
    wr = {n for n in mine if s in F.ins[n]['wr']}
    liv = set(rd)
    work = list(rd)
    while work:
        n = work.pop()
        for p in F.pred[n]:
            if p in liv:
                continue
            if p in wr and p not in rd:
                continue
            if p in mine and s in F.ins[p]['wr'] and s not in F.ins[p]['rd']:
                continue
            liv.add(p)
            work.append(p)
    if a.defined and wr:
        reach = set()
        work = [x for w in wr for x in F.ins[w]['s']]
        while work:
            n = work.pop()
            if n in reach:
                continue
            reach.add(n)
            work.extend(F.ins[n]['s'])
        liv &= reach | wr
    if a.tail and sym.startswith('T'):
        liv |= F.reload_tail(rd)
    return liv, wr


LIVE = {}
for sym in sym_ins:
    if (sym in addr_taken and sym_slot[sym].most_common(1)[0][0] < 0) or sym.startswith('coast_'):
        LIVE[sym] = (set(range(len(F.ins))), set())
    else:
        LIVE[sym] = relabel(sym)


def interferes(x, y):
    lx, wx = LIVE[x]
    ly, wy = LIVE[y]
    # x's home is live at a point where y writes or is live (live-out of a write counts through succ)
    return bool((lx & (ly | wy)) or (ly & wx))


actual = {sym: sym_slot[sym].most_common(1)[0][0] for sym in sym_ins}
count = {sym: len(sym_ins[sym]) for sym in sym_ins}
if a.count.startswith('reg'):
    LD = re.compile(r'^mov (e[a-ds][xi]), dword ptr \[ebp')
    ST = re.compile(r'^mov dword ptr \[ebp [-+] \w+\], (e[a-ds][xi])$')
    for sym in sym_ins:
        if not a.count.endswith('all') and (sym.startswith('T') and '@' in sym or sym.startswith('$T') or sym.startswith('tv')):
            continue
        if a.count[:4] in ('reg3', 'reg4'):
            k = 0
            for n in sym_ins[sym]:
                tx = F.ins[n]['txt']
                m = LD.match(tx)
                if m:
                    k += max(1, len(live.reg_uses(F, n, m.group(1))))
                    continue
                k += 1
                m = a.count.startswith('reg4') and ST.match(tx)
                if m:
                    k += len(live.reg_uses(F, n, m.group(1)))
            count[sym] = k
            continue
        extra = set()
        for n in sym_ins[sym]:
            tx = F.ins[n]['txt']
            m = LD.match(tx) or (a.count.startswith('reg2') and ST.match(tx))
            if m:
                extra |= live.reg_uses(F, n, m.group(1))
        count[sym] += len(extra - sym_ins[sym])
elif a.count != 'frame':
    import refcount
    RC = refcount.Counts(text, addr)
    for sym in sym_ins:
        if sym.startswith('T') and '@' in sym or sym.startswith('$T') or sym.startswith('tv'):
            continue
        k = RC.count(sym, [line_of.get(addr_of[n], 0) for n in sym_ins[sym]])
        if k:
            count[sym] = k if a.count == 'src' else max(k, count[sym])
if a.tie == 'last':  # probe g14: of two equal counts, the one whose last reference comes first is created first
    order = sorted(sym_ins, key=lambda x: (-count[x], max(addr_of[n] for n in sym_ins[x])))
elif a.tie == 'first':  # control: earlier first reference first
    order = sorted(sym_ins, key=lambda x: (-count[x], min(addr_of[n] for n in sym_ins[x])))
else:
    order = sorted(sym_ins, key=lambda x: (-count[x], actual[x] if actual[x] < 0 else -9999))


def sym_slot_all():
    return {s for c in sym_slot.values() for s in c}


def _reach(starts, s):
    seen, work = set(), list(starts)
    while work:
        n = work.pop()
        if n in seen:
            continue
        seen.add(n)
        if s in F.ins[n]['wr']:
            continue
        work.extend(F.ins[n]['s'])
    return seen


_ER = {}


def entry_web(sym, s):
    """number of sym's reads of slot s that only the parameter's entry value reaches (no store of s reaches them;
    a maybe-uninitialised local packed into the home has none)."""
    if s not in _ER:
        stores = [n for n in range(len(F.ins)) if s in F.ins[n]['wr']]
        _ER[s] = (_reach([0], s), _reach([x for w in stores for x in F.ins[w]['s']], s))
    er, sr = _ER[s]
    return sum(1 for n in sym_ins[sym] if s in F.ins[n]['rd'] and n in er and n not in sr)


# model run
groups = []  # list of [members]
model = {}
# parameter homes (probe g17): pre-existing slots, founded by the parameter itself (the web its entry value
# reaches), tried before every local slot, the parameter read first first
pgroups = []
pname = {}
if a.params:
    for s in sorted({x for x in sym_slot_all() if x >= 8}):
        par = sorted((sym for sym in sym_ins if actual[sym] == s and entry_web(sym, s)), key=lambda x: -entry_web(x, s))
        if par:
            pname[s] = par[0]
            g = [par[0]]
            pgroups.append((min(addr_of[n] for n in sym_ins[par[0]]), s, g))
            model[par[0]] = g
    pgroups.sort()
    if os.environ.get('PACKSIM_DEBUG'):
        print('PARAMS', [(hex(s), g[0], hex(f)) for f, s, g in pgroups], 'slots', sorted(hex(x) for x in sym_slot_all() if x >= 8))
        for sym in sym_ins:
            if actual[sym] >= 8:
                print('  ', sym, hex(actual[sym]), sorted(hex(addr_of[n]) for n in sym_ins[sym])[:8])
    pgroups = [g for _, _, g in pgroups]
for sym in order:
    if sym in addr_taken and actual[sym] < 0 or sym in model:
        continue
    if not a.params and actual[sym] > 0:
        continue
    loc = list(reversed(groups)) if a.fit == 'last' else list(groups)
    cands = (list(pgroups) + loc if a.param_order == 'first' else loc + list(pgroups) if a.param_order == 'last'
             else loc + list(reversed(pgroups)))
    if a.fit == 'smallest':
        cands.sort(key=lambda g: sum(count[m] for m in g))
    for g in cands:
        if not any(interferes(sym, m) for m in g):
            g.append(sym)
            model[sym] = g
            break
    else:
        g = [sym]
        groups.append(g)
        model[sym] = g
# compare: map model groups to actual slots by majority
gslot = {}
groups = pgroups + groups
for g in groups:
    cnt = collections.Counter(actual[m] for m in g)
    gslot[id(g)] = cnt.most_common(1)[0][0]
ok = sum(1 for s in model if gslot[id(model[s])] == actual[s])
w_ok = sum(count[s] for s in model if gslot[id(model[s])] == actual[s])
print('symbols %d  model slot = actual for %d (%d of %d accesses); model slots %d, actual slots %d' % (
    len(model), ok, w_ok, sum(count[s] for s in model), len(groups), len({actual[s] for s in model})))
if os.environ.get('PACKSIM_LIST'):
    for s in model:
        print('SYM %s n=%d frame=%d ok=%d actual=%s' % (s, count[s], len(sym_ins[s]), gslot[id(model[s])] == actual[s], hex(actual[s])))
# actual slot creation order: by offset (frame offsets in creation order)
act_order = sorted({actual[s] for s in model}, reverse=True)
for g in groups:
    print('  model %s: %s' % (hex(gslot[id(g)]), ' '.join('%s(%d)%s' % (m, count[m], '' if actual[m] == gslot[id(g)] else '[%s]' % hex(actual[m])) for m in g[:14]) + (' ...' if len(g) > 14 else '')))


def lines(sym):
    return sorted({line_of.get(addr_of[n], 0) for n in sym_ins[sym]})


if a.why or a.sym:
    want = set(a.sym.split(',')) if a.sym else None
    occ = collections.defaultdict(list)
    for s in sym_ins:
        occ[actual[s]].append(s)
    print('\nWHY (actual layout: slots deeper than the symbol\'s own that it could not join, with the blocker)')
    for sym in order:
        if sym not in model:
            continue
        if want and sym not in want and not any(sym.startswith(w) for w in want):
            continue
        if not want and not sym.startswith('T'):
            continue
        mine = actual[sym]
        later = [s for s in act_order if s < mine]
        bl = []
        for s in later:
            blk = [m for m in occ[s] if m != sym and count[m] >= count[sym] and interferes(sym, m)]
            free = not blk
            bl.append('%s:%s' % (hex(s), 'FREE' if free else ','.join(blk[:2])))
        print('%s n=%d L%s in %s; deeper: %s' % (sym, count[sym], lines(sym)[:6], hex(mine), ' '.join(bl[:12])))

# offset order check: actual slots sorted by offset against founder count and group total
occ2 = collections.defaultdict(list)
for s in model:
    occ2[actual[s]].append(s)
offs = sorted(occ2, reverse=True)
def inversions(key):
    v = [key(o) for o in offs]
    return sum(1 for i in range(len(v)) for j in range(i + 1, len(v)) if v[i] < v[j])
print('\nOFFSET ORDER inversions (lower = explains the layout): founder %d, total %d, of %d pairs' % (
    inversions(lambda o: max(count[m] for m in occ2[o])), inversions(lambda o: sum(count[m] for m in occ2[o])),
    len(offs) * (len(offs) - 1) // 2))
for o in offs:
    print('  %6s founder %4d total %4d  %s' % (hex(o), max(count[m] for m in occ2[o]), sum(count[m] for m in occ2[o]),
                                              ' '.join(sorted(occ2[o], key=lambda m: -count[m])[:5])))
