#!/usr/bin/env python3
"""reusesweep.py <unit> <0xADDR> [--write] [--iter N] [--show KEY] [--screen-only] | --from FEALL.json [--min 100] [--skip A,B] [--write]
Variable-reuse screen (MATCHING_GUIDE 3.3 "Reused function-scope variable"): renames each block-local
int/BOOL of a function into each existing function-scope int/BOOL (a depth-1 declaration before it, or a
parameter) that is DEAD over the block-local's whole scope, and fingerprints every rename.

Liveness is proved by the compiler, not guessed: a target T qualifies for block-local B only when
  (1) T is not named anywhere in B's scope (declaration to the end of the enclosing block; VC6's for-init
      variables leak to the enclosing block too), no label inside the scope is a goto target from outside, and
  (2) the kill probe `T = 0x2468ACF;` inserted at B's declaration compiles to exactly the base function
      (same instructions): /Og removed the store, so T is dead there; with no reference to T inside the scope
      it is dead throughout, and writing B's values into T changes nothing T's later readers see.
The rename variants of qualifying pairs are fe'd with their frame and slot maps (framescan), the best ones
also with candset.py (one-side roles + disputed pairs). Ranking key: frame difference, slot-map difference
(sum over frame offsets of |image refs - our refs|), candset count, rows. --write keeps the best rename when
its key beats the base and repeats (--iter rounds), so chained renames are found one at a time.
--screen-only skips the pairs the volatile screen cannot judge (packed slots, address-taken targets): on a
giant function they cost hours (enemy_move: 1,405 such pairs; each variant's 17,440-row alignment takes about
15 s and is serialised, so even the 450 screened pairs need about two hours).
Env: FE_PFX_BASE (first lane prefix, default 21), FE_JOBS (lanes, default 5)."""
import sys, os, re, io, json, argparse, collections, contextlib, threading, queue
H = os.path.dirname(os.path.abspath(__file__))
T_ = os.path.dirname(H)
R = os.path.dirname(os.path.dirname(T_))
sys.path.insert(0, T_)
sys.path.insert(0, H)
os.chdir(R)
import fe
import framescan
import armwo

PB = int(os.environ.get('FE_PFX_BASE', '21'))
JOBS = int(os.environ.get('FE_JOBS', '5'))
KILL = '0x2468ACF'
TYPES = ('int', 'BOOL')
SCR = R + '/build/recovery/fe/reuse'
LOCK = threading.Lock()
NEAR = 40  # a scope's code is entered from a marker at most NEAR lines above it
SCREEN_ONLY = False  # --screen-only: only the pairs the volatile screen proves (giant functions)
LBASE = 20000  # above every header (win.h 1,413 lines), below the 65,535 CodeView line limit
FDECL = []  # keyword offsets of the depth-1 declarations of the last analyse()


def mask(t):
    """t with comments, string and char literals blanked (newlines kept)"""
    o = list(t)
    i, n = 0, len(t)
    while i < n:
        if t.startswith('//', i):
            j = t.find('\n', i)
            j = n if j < 0 else j
            for k in range(i, j):
                o[k] = ' '
            i = j
        elif t.startswith('/*', i):
            j = t.find('*/', i + 2)
            j = n if j < 0 else j + 2
            for k in range(i, j):
                if o[k] != '\n':
                    o[k] = ' '
            i = j
        elif t[i] in '"\'':
            q = t[i]
            j = i + 1
            while j < n and t[j] != q:
                j += 2 if t[j] == '\\' else 1
            for k in range(i + 1, min(j, n)):
                o[k] = ' '
            i = j + 1
        else:
            i += 1
    return ''.join(o)


def ident_re(name):
    return re.compile(r'(?<![\w.$])(?<!->)(?<!::)%s(?!\w)' % re.escape(name))


def split_top(s):
    out, d, cur = [], 0, ''
    for c in s:
        if c in '([{':
            d += 1
        elif c in ')]}':
            d -= 1
        if c == ',' and d == 0:
            out.append(cur)
            cur = ''
        else:
            cur += c
    out.append(cur)
    return out


def analyse(text, addr):
    """-> (bs, be, fvars {name: decl offset}, locals [dict(name, at, kind, s, e, decl)])"""
    bs, be, _ = armwo.locate(text, '0x%08X' % addr)
    m = mask(text)
    depth = [0] * (be - bs + 1)
    d = 0
    for i in range(bs, be):
        c = m[i]
        if c == '{':
            d += 1
        elif c == '}':
            d -= 1
        depth[i - bs] = d  # depth after this char
    def dep(i):  # depth before char i
        return depth[i - bs - 1] if i > bs else 0
    # parameters: the parenthesised list before bs
    p = m.rfind(')', 0, bs)
    q, dd = p, 0
    while q > 0:
        if m[q] == ')':
            dd += 1
        elif m[q] == '(':
            dd -= 1
            if dd == 0:
                break
        q -= 1
    fvars = {}
    del FDECL[:]
    for prm in split_top(m[q + 1:p]):
        mm = re.fullmatch(r'\s*(?:const\s+)?(int|BOOL)\s+(\w+)\s*', prm)
        if mm:
            fvars[mm.group(2)] = -1
    locs = []
    for mm in re.finditer(r'(?<![\w.])(int|BOOL)\s+(?=[A-Za-z_])', m[bs:be]):
        at = bs + mm.start()
        dl = dep(at)
        pre = m[:at].rstrip()
        prevc = pre[-1] if pre else ''
        isfor = re.search(r'\bfor\s*\($', pre) is not None
        if not isfor and prevc not in ';{}:':
            continue  # a cast, a parameter, `if (c) int x` ...
        if prevc == ':' and re.search(r'\?[^;{}]*:$', pre):
            continue
        end = m.find(';', at)
        decl = m[at:end]
        parts = split_top(decl[mm.end() - mm.start():])
        names = []
        ok = True
        for part in parts:
            pm = re.fullmatch(r'\s*(\w+)\s*(=.*)?', part, re.S)
            if not pm:
                ok = False
                break
            names.append(pm.group(1))
        if not ok:
            continue
        if dl == 1 and not isfor:
            for nm in names:
                fvars.setdefault(nm, at)
            FDECL.append((at, names))
            continue
        if len(names) != 1:
            continue
        # scope: to the end of the enclosing block (depth dl)
        e = at
        while e < be and not (m[e] == '}' and dep(e) == dl):
            e += 1
        locs.append(dict(name=names[0], at=at, s=at, e=e, isfor=isfor, line=text.count('\n', 0, at) + 1,
                         kwlen=mm.end() - mm.start(), init='=' in parts[0], end=end))
    return bs, be, m, fvars, locs


def gotos_into(m, bs, be, s, e):
    for lm in re.finditer(r'(?m)^\s*([A-Za-z_]\w*)\s*:(?!:)', m[s:e]):
        lab = lm.group(1)
        if lab in ('default', 'public', 'private', 'protected'):
            continue
        for g in re.finditer(r'\bgoto\s+%s\b' % lab, m[bs:be]):
            if not (s <= bs + g.start() < e):
                return True
    return False


def read_next(m, e, be, T):
    """the first reference to T after the scope reads it (screen only: an else-arm read can look later)"""
    mm = ident_re(T).search(m, e, be)
    if not mm:
        return False
    a = mm.start()
    st = max(m.rfind(';', 0, a), m.rfind('{', 0, a), m.rfind('}', 0, a))
    if m[st + 1:a].strip():
        return True  # not at a statement start: part of an expression
    rest = m[mm.end():m.find(';', mm.end())]
    am = re.match(r'\s*=(?!=)(.*)$', rest, re.S)
    return not am or bool(ident_re(T).search(am.group(1)))


def pairs(text, addr):
    bs, be, m, fvars, locs = analyse(text, addr)
    out = []
    for L in locs:
        if gotos_into(m, bs, be, L['s'], L['e']):
            continue
        scope = m[L['s']:L['e']]
        for T, at in fvars.items():
            if T == L['name'] or at >= L['s']:
                continue
            if ident_re(T).search(scope):
                continue
            if read_next(m, L['e'], be, T):
                continue
            out.append((L, T))
    return out, m


def kill_pos(text, L):
    ins = L['at']
    if L['isfor']:
        ins = text.rfind('for', 0, ins)
    return ins


def kill_variant(text, group):
    """every (L, T) of group probed at once: if the function is unchanged, every store was dead (a store
    that some path reads before a redefinition stays, and another probe on that path would be live too)"""
    at = collections.defaultdict(list)
    for L, T in group:
        at[kill_pos(text, L)].append(T)
    for p in sorted(at, reverse=True):
        text = text[:p] + ''.join('%s = %s; ' % (T, KILL) for T in at[p]) + text[p:]
    return text


def dead_pairs(unit, addr, text, keyed, bsig, log):
    """group testing over the kill probes: halves of a live group are retested"""
    groups = [sorted(keyed)]
    dead, ncomp = [], 0
    while groups:
        res = run(unit, addr, [('%d' % i, kill_variant(text, [keyed[k] for k in g])) for i, g in enumerate(groups)])
        ncomp += len(groups)
        nxt = []
        for i, g in enumerate(groups):
            r = res.get('%d' % i)
            if r is not None and ours_sig(r) == bsig:
                dead += g
            elif len(g) > 1:
                nxt += [g[:len(g) // 2], g[len(g) // 2:]]
        groups = nxt
    log('  kill probes: %d compiles' % ncomp)
    return dead


def rename_variant(text, m, L, T):
    s, e = L['s'], L['e']
    seg = text[s:e]
    mseg = m[s:e]
    out = []
    last = 0
    for mm in ident_re(L['name']).finditer(mseg):
        out.append(seg[last:mm.start()])
        out.append(T)
        last = mm.end()
    out.append(seg[last:])
    seg = ''.join(out)
    # drop the declaration keyword (`int x = e;` -> `T = e;`, `int x;` -> ``)
    kw = L['kwlen']
    if L['init']:
        seg = seg[kw:]
    else:
        semi = seg.find(';')
        seg = seg[semi + 1:]
        ls = text.rfind('\n', 0, s) + 1
        if not text[ls:s].strip() and seg[:seg.find('\n') + 1].strip() == '':
            return text[:ls] + seg[seg.find('\n') + 1:] + text[e:]  # the declaration was the whole line
    return text[:s] + seg + text[e:]


def ours_sig(r):
    return [(c[1], c[2]) for k, o, c in r['rows'] if c is not None]


def frame_slots(r):
    res = []
    for side in (0, 1):
        ins = sorted((x[0], x[1].split(' ', 1)[0], x[1].split(' ', 1)[1] if ' ' in x[1] else '', x[2])
                     for row in r['rows'] for x in (row[1 + side],) if x is not None)
        fr = framescan.frame(ins)
        sl = framescan.slots(ins, fr[0], fr[2], fr[3])
        res.append((fr, sl))
    (fi, si), (fo, so) = res
    fd = abs(fi[0] - fo[0]) + (8 if fi[1] != fo[1] else 0)
    sd = sum(abs((si[k][2] if k in si else 0) - (so[k][2] if k in so else 0)) for k in set(si) | set(so) if k < 0)
    return fd, sd, fo[0]


def run(unit, addr, deck):
    """deck [(key, text)] -> {key: evaluate dict or None}"""
    os.makedirs(SCR, exist_ok=True)
    fe.compile_cmd(unit)  # fill fe's compile_commands cache before the lanes race on it
    qu = queue.Queue()
    for item in deck:
        qu.put(item)
    res = {}
    def lane(k):
        pfx = PB + k
        path = '%s/v%d.cpp' % (SCR, pfx)
        while True:
            try:
                key, text = qu.get_nowait()
            except queue.Empty:
                return
            open(path, 'w', encoding='latin-1').write(text)
            c, err = fe.compile_unit(unit, path, pfx)
            with LOCK:  # capstone's decoder is shared: evaluate one at a time
                r = fe.evaluate(c, text, addr) if c is not None else None
            res[key] = None if (r is None or 'error' in r) else r
    th = [threading.Thread(target=lane, args=(k,)) for k in range(JOBS)]
    [t.start() for t in th]
    [t.join() for t in th]
    return res


def cand_count(unit, text, addr, r):
    import candset, regprio
    path = '%s/cand.cpp' % SCR
    open(path, 'w', encoding='latin-1').write(text)
    try:
        items, _ = regprio.cod_listing(unit, path, PB, r['sym'])
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            candset.report(r, text, items, addr, sections=('only', 'order'))
    except Exception:
        return 999
    n, sec = 0, None
    for ln in buf.getvalue().split('\n'):
        if ln.startswith('---'):
            sec = ln
        elif ln.startswith('  ') and sec:
            n += 1
    return n


def volatile_dead(unit, addr, text, ps, log):
    """liveness screen: every depth-1 int/BOOL made volatile (each source read is a slot read, no cached
    copy, no dead-store removal), then T is dead over L's scope when its slot is live-in at none of the
    instructions of the scope's source lines.  -> set of (id(L), T) proved dead, set of T screened"""
    import live, regprio
    bs, be, m, _, _ = analyse(text, addr)
    taken = lambda nm: re.search(r'[(,=?:]\s*&\s*%s\b' % re.escape(nm), m[bs:be])
    v = text
    for at, names in sorted(FDECL, reverse=True):
        if not any(taken(nm) for nm in names):  # `volatile int *` does not convert to `int *`
            v = v[:at] + 'volatile ' + v[at:]
    # the unit's own lines moved down by LBASE blank lines: /FAsc prints an inlined header function's
    # lines as bare numbers, which would otherwise collide with the unit's (`#line` drops the listing's
    # source lines altogether)
    v = '\n' * LBASE + v
    path = '%s/vol_%d.cpp' % (SCR, PB)
    open(path, 'w', encoding='latin-1').write(v)
    sym = fe.ident(text, addr)[0]
    try:
        items, frame = regprio.cod_listing(unit, path, PB, sym)
        F = live.ours(unit, path, addr, str(PB))
    except BaseException as e:
        log('  volatile screen failed: %r' % (e,))
        return set(), set()
    # the volatile variant shifts no lines (the keyword is inserted inside a line)
    # each instruction's line and the line of the marker before it: VC6 also labels stray code (an
    # inlined callee hoisted to the top) with a line of the unit, but such a run is entered from a far line
    line_of, cur, prev = {}, 0, 0
    for it in items:
        if it[0] == 'line':
            prev, cur = cur, it[1]
        elif it[0] == 'ins':
            line_of[it[1]] = (cur, prev)
    slots = collections.Counter(frame.values())
    lv = {}
    dead, screened = set(), set()
    for L, T in ps:
        if T not in frame or not any(i['rd'] or i['wr'] for i in F.ins):
            continue
        s = frame[T]
        if slots[s] > 1:
            continue  # a packed slot: its liveness is another variable's too (left to the kill probe)
        if taken(T):
            continue  # address taken: reads through pointers are invisible
        screened.add(T)
        if s not in lv:
            lv[s] = F.livein(s)
        l0, l1 = LBASE + text.count('\n', 0, L['s']) + 1, LBASE + text.count('\n', 0, L['e']) + 1
        idx = [n for n, i in enumerate(F.ins)
               if l0 <= line_of.get(i['a'], (-1, 0))[0] <= l1 and l0 - NEAR <= line_of[i['a']][1] <= l1]
        if idx and not any(n in lv[s] for n in idx):
            dead.add((id(L), T))
    return dead, screened


def sweep(unit, addr, write=False, top=6, log=print):
    path = R + '/' + unit
    text = open(path, encoding='latin-1').read()
    base = run(unit, addr, [('BASE', text)])['BASE']
    if base is None:
        log('  base fails')
        return None
    ps, m = pairs(text, addr)
    if not ps:
        log('  no pairs')
        return dict(base=base['diff'], best=None)
    keyed = {'%s@L%d->%s' % (L['name'], L['line'], T): (L, T) for L, T in ps}
    bsig = ours_sig(base)
    vd, screened = volatile_dead(unit, addr, text, ps, log)
    rest = {k: p for k, p in keyed.items() if p[1] not in screened}
    vdead = {k: p for k, p in keyed.items() if (id(p[0]), p[1]) in vd}
    log('  volatile screen: %d targets, %d pairs dead; %d pairs to kill-probe' % (len(screened), len(vdead), len(rest)))
    # the kill probe confirms the screen (a sound proof when the store disappears)
    dead = dead_pairs(unit, addr, text, vdead, bsig, log) if vdead else []
    if len(dead) != len(vdead):
        log('  kill probe keeps %d of the screen\'s dead pairs: %s' % (len(vdead) - len(dead), sorted(set(vdead) - set(dead))[:8]))
    if rest and not SCREEN_ONLY:
        dead += dead_pairs(unit, addr, text, rest, bsig, log)
    log('  pairs %d dead %d' % (len(keyed), len(dead)))
    if not dead:
        return dict(base=base['diff'], best=None, pairs=len(keyed), dead=0)
    vt = {k: rename_variant(text, m, *keyed[k]) for k in dead}
    rres = run(unit, addr, list(vt.items()))
    bf, bsl, bfr = frame_slots(base)
    scored = []
    for k in dead:
        r = rres.get(k)
        if r is None:
            continue
        fd, sd, fr = frame_slots(r)
        scored.append([fd, sd, None, r['diff'], k, r['exact'], fr])
    scored.sort(key=lambda x: (x[0], x[1], x[3]))
    bc = cand_count(unit, text, addr, base)
    bkey = (bf, bsl, bc, base['diff'])
    for x in scored[:top]:
        if (x[0], x[1]) <= (bf, bsl):
            x[2] = cand_count(unit, vt[x[4]], addr, rres[x[4]])
    full = sorted([x for x in scored if x[2] is not None], key=lambda x: (x[0], x[1], x[2], x[3]))
    log('  BASE frame %d slots %d cand %d rows %d (frame 0x%x)' % (bkey + (bfr,)))
    for x in scored[:12]:
        log('  %s frame %d slots %d cand %s rows %d%s' % (x[4], x[0], x[1], x[2], x[3], ' EXACT' if x[5] else ''))
    best = full[0] if full else None
    out = dict(base=base['diff'], bkey=bkey, pairs=len(keyed), dead=len(dead),
               variants=[(x[4], x[0], x[1], x[2], x[3]) for x in scored])
    def better(x):
        # a better frame or slot map wins; at the same frame and slots, fewer rows with no worse candset
        if (x[0], x[1]) != (bf, bsl):
            return (x[0], x[1]) < (bf, bsl)
        return x[3] < base['diff'] and x[2] <= bc
    full = [x for x in full if better(x)]
    best = full[0] if full else None
    if best:
        out['best'] = (best[4], best[0], best[1], best[2], best[3])
        if write:
            open(path, 'w', encoding='latin-1').write(vt[best[4]])
            log('  WROTE %s' % best[4])
    else:
        out['best'] = None
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('unit', nargs='?')
    ap.add_argument('addr', nargs='?')
    ap.add_argument('--from', dest='frm')
    ap.add_argument('--min', type=int, default=100)
    ap.add_argument('--skip', default='')
    ap.add_argument('--write', action='store_true')
    ap.add_argument('--iter', type=int, default=3)
    ap.add_argument('--json')
    ap.add_argument('--show')
    ap.add_argument('--screen-only', action='store_true')
    a = ap.parse_args()
    global SCREEN_ONLY
    SCREEN_ONLY = a.screen_only
    todo = []
    if a.frm:
        skip = {s.upper().replace('0X', '').zfill(8) for s in a.skip.split(',') if s}
        for x in json.load(open(a.frm)):
            if not x['exact'] and x['diff'] >= a.min and x['addr'] not in skip:
                todo.append((x['unit'], '0x' + x['addr']))
    else:
        todo.append((a.unit if a.unit.startswith('src/') else 'src/' + a.unit, a.addr))
    if a.show:
        unit, addr = todo[0]
        text = open(R + '/' + unit, encoding='latin-1').read()
        ps, m = pairs(text, int(addr, 16))
        for L, T in ps:
            if '%s@L%d->%s' % (L['name'], L['line'], T) == a.show:
                import difflib
                v = rename_variant(text, m, L, T)
                sys.stdout.writelines(difflib.unified_diff(text.splitlines(1), v.splitlines(1), n=1))
        return
    allres = {}
    for unit, addr in todo:
        print('== %s %s' % (unit, addr), flush=True)
        hist = []
        for it in range(a.iter if a.write else 1):
            o = sweep(unit, int(addr, 16), a.write, log=lambda s: print(s, flush=True))
            if o is None:
                break
            hist.append(o)
            if not (a.write and o.get('best')):
                break
        allres['%s %s' % (unit, addr)] = hist
        if a.json:
            json.dump(allres, open(a.json, 'w'), indent=1)


if __name__ == '__main__':
    main()
