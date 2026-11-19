#!/usr/bin/env python3
"""jointsolve.py eval [--out F.json] [--base F.json] [--units a,b] [--keep-standins] [--set name=rung,.. --deck DECK.py [--keep]]
   jointsolve.py copies DECK.py            # which body spellings keep the out-of-line copy fe-exact
   jointsolve.py descend DECK.py [--pairs] [--log F] [--start name=rung,...]

Joint solve of the header-inline body spellings (MATCHING_GUIDE 2.8, "Stand-ins are budget
compensation: change bodies together").  The objective is the image's per-function call count of every
header-inline callee, tree-wide, measured WITHOUT the local function-pointer stand-ins
(`T (__cdecl *const x)(..) = callee;` lines are deleted and `x(` spelled `callee(` in a scratch copy
of the unit), so a body is judged against the image's decisions and not against its own compensation.

eval     one pass: every unit compiled once as written (exact set, rows) and the stand-in units a
         second time without them (decision vector).  Prints site-units and, with --base, what moved.
copies   DECK.py defines INLINES = {name: dict(file=..., head=<regex of the definition line>,
         rungs=[(tag, body_text), ...], feasible=[tags], ident=<identifier when the key is not one>,
         sym=<regex of the copy's symbol for an overload>)}; body_text replaces the text between the
         definition line and the closing `\\n}\\n`.  Reports, per rung, whether every copy stays exact.
descend  coordinate descent over the rungs whose copy is exact: change one inline's rung, keep the
         move when tree site-units drop, no (function, callee) pair gets worse and no exact function
         beyond the count canaries breaks (a broken exact caller is shape evidence against the
         spelling); then (--pairs) pair moves, accepted on a strict drop of the total.  The headers are edited in place and left at
         the best configuration; every step is appended to --log.
Prefixes FE_PFX_BASE.. with FE_JOBS lanes.  Screen only (fe rows; `recover measure` is the authority).
"""
import sys, os, re, json, glob, argparse, collections, itertools, time
from concurrent.futures import ProcessPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.dirname(HERE))
import fe  # noqa: E402
import callees  # noqa: E402
import inlineclass  # noqa: E402

R = fe.R
MARK = re.compile(r'// ORIGINAL: 0x([0-9A-F]{8}) \S+ 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+')
STANDIN = re.compile(r'^[ \t]*[\w \t\*]+\(__cdecl \*\s*const\s+(\w+)\)\s*\([^)]*\)\s*=\s*(\w+);[^\n]*\n', re.M)
JOBS = int(os.environ.get('FE_JOBS', '5'))
PFX = int(os.environ.get('FE_PFX_BASE', '21'))


def strip_standins(text, names=None):
    """the unit without its local function-pointer stand-ins for header inlines; None when it has none"""
    hdr = names if names is not None else inlineclass.header_names()
    subs = [(m.group(1), m.group(2)) for m in STANDIN.finditer(text) if m.group(2) in hdr]
    if not subs:
        return None
    out = STANDIN.sub(lambda m: '\n' if m.group(2) in hdr else m.group(0), text)
    for alias, real in set(subs):
        out = re.sub(r'\b%s\(' % re.escape(alias), real + '(', out)
    return out


def unit_list(only=None):
    cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
    units = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/')]
    out = []
    for u in units:
        if only and u not in only and os.path.basename(u) not in only:
            continue
        if os.path.exists(R + '/' + u):
            out.append(u)
    return out


def _measure(c, text, with_rows=True):
    out = {}
    for m in MARK.finditer(text):
        a = int(m.group(1), 16)
        key = '%08X' % a
        try:
            row = {}
            if with_rows:
                r = fe.evaluate(c, text, a)
                if 'error' in r:
                    continue
                row.update(exact=r['exact'], diff=r['diff'], sym=r['sym'])
            d = callees.diff(c, text, a)
            if d is not None:
                ci, co = d
                row['d'] = {k: [ci[k], co[k]] for k in set(ci) | set(co) if ci[k] != co[k]}
            out[key] = row
        except Exception:
            pass
    return out


def _lane(arg):
    k, units, keep, hdr = arg
    pfx = str(PFX + k)
    res = {}
    for u in units:
        text = open(R + '/' + u, encoding='latin-1').read()
        if not MARK.search(text):
            continue
        c, err = fe.compile_unit(u, R + '/' + u, pfx)
        if c is None:
            res[u] = dict(fail=(err or '')[:300])
            continue
        r = dict(as_written=_measure(c, text))
        alt = None if keep else strip_standins(text, hdr)
        if alt is not None:
            path = os.path.join(fe.SCR, 'jointsolve_%s.cpp' % pfx)
            open(path, 'w', encoding='latin-1').write(alt)
            c2, err = fe.compile_unit(u, path, pfx)
            if c2 is not None:
                r['bare'] = _measure(c2, alt)
            else:
                r['bare_fail'] = (err or '')[:300]
        res[u] = r
    return res


_HDR = None


def tree(units=None, keep=False):
    """{unit: {as_written: {addr: {exact, diff, sym, d}}, bare: {...}}}"""
    global _HDR
    if _HDR is None:
        _HDR = inlineclass.header_names()
    us = unit_list(units)
    # big units first, spread over the lanes
    us.sort(key=lambda u: -os.path.getsize(R + '/' + u))
    args = [(k, us[k::JOBS], keep, _HDR) for k in range(JOBS)]
    res = {}
    with ProcessPoolExecutor(JOBS) as ex:
        for o in ex.map(_lane, args):
            res.update(o)
    return res


def keepname(n):
    p = inlineclass.plain(n)
    return p.split('::')[-1].replace('dtor', '~') in _HDR or (p.endswith('dtor') and '~' + p.split('::')[0] in _HDR)


def vector(res):
    """{(addr, callee): (image, ours)} for header-inline callees, the bare measurement where there is one"""
    v = {}
    for u, r in res.items():
        src = r.get('bare') or r.get('as_written') or {}
        for a, row in src.items():
            for n, (i, o) in row.get('d', {}).items():
                if keepname(n):
                    v[(a, n)] = (i, o)
    return v


def exact_set(res):
    return {a for r in res.values() for a, row in r.get('as_written', {}).items() if row.get('exact')}


def total(v):
    return sum(abs(i - o) for i, o in v.values())


def merge(base, part):
    out = dict(base)
    out.update(part)
    return out


def compare(vb, vn):
    """(closer, further, lines)"""
    closer = further = 0
    lines = []
    for k in sorted(set(vb) | set(vn)):
        ob = abs(vb[k][0] - vb[k][1]) if k in vb else 0
        on = abs(vn[k][0] - vn[k][1]) if k in vn else 0
        if ob != on:
            if on < ob:
                closer += ob - on
            else:
                further += on - ob
            lines.append('%s %s %-26s %s -> %s' % ('+' if on < ob else '-', k[0], inlineclass.plain(k[1])[:26],
                                                 list(vb[k]) if k in vb else '=', list(vn[k]) if k in vn else '='))
    return closer, further, lines


def syms(res):
    return {a: row.get('sym', '?') for r in res.values() for a, row in r.get('as_written', {}).items()}


# ---- deck handling -------------------------------------------------------------------------------

def load_deck(path):
    ns = {}
    exec(compile(open(path).read(), path, 'exec'), ns)
    return ns['INLINES']


def body_span(text, head):
    m = re.search(head, text, re.M)
    if not m:
        raise SystemExit('head not found: ' + head)
    lo = text.index('{', m.start()) + 1
    hi = text.index('\n}\n', lo)
    return lo, hi


def get_body(spec):
    t = open(R + '/' + spec['file'], encoding='latin-1').read()
    lo, hi = body_span(t, spec['head'])
    return t[lo:hi]


def set_body(spec, body):
    p = R + '/' + spec['file']
    t = open(p, encoding='latin-1').read()
    lo, hi = body_span(t, spec['head'])
    if t[lo:hi] != body:
        open(p, 'w', encoding='latin-1').write(t[:lo] + body + t[hi:])


def _norm(body):
    """a body without comment lines and indentation, for comparing with the deck"""
    return '\n'.join(l.strip() for l in body.split('\n') if l.strip() and not l.strip().startswith('//'))


def rung_body(spec, tag):
    for t, b in spec['rungs']:
        if t == tag:
            return b if b.startswith('\n') else '\n' + b.rstrip('\n')
    raise KeyError(tag)


_COPIES = None


def copies_of(spec, name):
    """[(unit, addr)] of every marker whose symbol is this function (`sym` regex in the spec, else ?name@@Y)"""
    global _COPIES
    if _COPIES is None:
        _COPIES = []
        for u in unit_list():
            t = open(R + '/' + u, encoding='latin-1').read()
            for m in re.finditer(r'// ORIGINAL: 0x([0-9A-F]{8}) (\S+) 0x', t):
                _COPIES.append((u, int(m.group(1), 16), m.group(2)))
    pat = re.compile(spec.get('sym') or r'^\?%s@@Y' % re.escape(name.split('/')[0]))
    return [(u, a) for u, a, s in _COPIES if pat.search(s)]


def copy_exact(spec, name):
    """-> [(addr, exact, rows)] over every copy"""
    out = []
    for unit, addr in copies_of(spec, name):
        host = unit
        text = open(R + '/' + unit, encoding='latin-1').read()
        m = re.search(r'// ORIGINAL: 0x%08X [^\n]*\n((?://[^\n]*\n)*)' % addr, text)
        m2 = m and re.search(r'// compile_context (\S+)', m.group(1))
        if m2:
            host = m2.group(1)
        c, err = fe.compile_unit(host, R + '/' + host, str(PFX))
        if c is None:
            out.append((addr, None, 'COMPILE FAIL ' + (err or '')[:200]))
            continue
        r = fe.evaluate(c, text, addr)
        if 'error' in r:
            out.append((addr, None, r['error']))
        else:
            out.append((addr, r['exact'], r['diff']))
    return out


def cmd_copies(a):
    deck = load_deck(a.deck)
    for name, spec in deck.items():
        if a.only and name not in a.only.split(','):
            continue
        cur = get_body(spec)
        try:
            for tag, _ in spec['rungs']:
                b = rung_body(spec, tag)
                set_body(spec, b)
                cs = copy_exact(spec, name)
                ok = bool(cs) and all(e is True for _, e, _ in cs)
                print('%-18s %-30s %s %s%s' % (name, tag, 'EXACT' if ok else ('nocopy' if not cs else 'no   '),
                                              ' '.join('%08X:%s' % (ad, d) for ad, e, d in cs),
                                              '   <- current' if _norm(b) == _norm(cur) else ''), flush=True)
        finally:
            set_body(spec, cur)


def users(deck, name, names_closure):
    """units whose text names the inline or a header inline that (transitively) names it"""
    pat = re.compile(r'\b(%s)\b' % '|'.join(re.escape(n) for n in names_closure[name]))
    return [u for u in unit_list() if pat.search(open(R + '/' + u, encoding='latin-1').read())]


def closure(deck):
    """name -> names of header functions whose body (transitively) uses it, by header text"""
    hdrs = ''.join(open(p, encoding='latin-1').read() for p in glob.glob(R + '/src/**/*.h', recursive=True))
    defs = {}
    for m in re.finditer(r'\b(~?[A-Za-z_]\w*)\s*\([^;{}()]*(?:\([^()]*\)[^;{}()]*)*\)\s*(?:const\s*)?(?::[^;{}]*)?\{', hdrs):
        n = m.group(1)
        if n in ('if', 'for', 'while', 'switch', 'return', 'sizeof', 'defined'):
            continue
        # body up to the matching brace
        depth, i = 1, m.end()
        while depth and i < len(hdrs):
            depth += {'{': 1, '}': -1}.get(hdrs[i], 0)
            i += 1
        defs.setdefault(n, '')
        defs[n] += hdrs[m.end():i]
    out = {}
    for name in deck:
        short = deck[name].get('ident', name.split('/')[0])
        seen = {short}
        grew = True
        while grew:
            grew = False
            pat = re.compile(r'\b(%s)\s*\(' % '|'.join(re.escape(s) for s in seen))
            for n, body in defs.items():
                if n not in seen and pat.search(body):
                    seen.add(n)
                    grew = True
        out[name] = seen
    return out


# count-sensitive canaries (AGENTS.md): they flip with a unit's front-end counter, not with a decision
CANARIES = {'004A6330', '00524870', '004E2000', '0053EB50', '004CEDE0', '00527290', '004C3090', '0042B630',
            '0055BC80', '00435400', '0048D840', '005C6DB0', '00453A70', '004CC360', '004A0260', '00500B60'}


def cmd_eval(a):
    t0 = time.time()
    moves = dict(kv.split('=') for kv in a.set.split(',')) if a.set else {}
    orig = {}
    if moves:
        deck = load_deck(a.deck)
        orig = {n: get_body(deck[n]) for n in moves}
        for n, tag in moves.items():
            set_body(deck[n], rung_body(deck[n], tag))
    try:
        res = tree(a.units.split(',') if a.units else None, keep=a.keep_standins)
    finally:
        if not a.keep:
            for n in orig:
                set_body(deck[n], orig[n])
    json.dump(res, open(a.out, 'w'))
    v = vector(res)
    ex = exact_set(res)
    fails = [u for u, r in res.items() if 'fail' in r or 'bare_fail' in r]
    print('site-units %d in %d (function, callee) pairs; exact-by-fe %d; %.0f s%s' % (
        total(v), len(v), len(ex), time.time() - t0, ('; FAIL ' + ' '.join(fails)) if fails else ''))
    if a.base:
        b = json.load(open(a.base))
        gone = sorted(set(b) - set(res))
        if gone and not a.units:
            print('units not measured now (left out of the comparison): ' + ' '.join(gone))
        b = {u: r for u, r in b.items() if u in res}
        vb = vector(b)
        v = vector({u: r for u, r in res.items() if u in b})
        ex = exact_set({u: r for u, r in res.items() if u in b})
        cl, fu, lines = compare(vb, v)
        print('against base: closer %d, further %d' % (cl, fu))
        for ln in lines:
            print('  ' + ln)
        eb = exact_set(b)
        sy = syms(res)
        print('exact lost %d (%d beyond the count canaries): %s' % (len(eb - ex), len(eb - ex - CANARIES), ' '.join('%s(%s)' % (x, sy.get(x, '?')[:24]) for x in sorted(eb - ex))))
        print('exact gained %d: %s' % (len(ex - eb), ' '.join('%s(%s)' % (x, sy.get(x, '?')[:24]) for x in sorted(ex - eb))))
    elif a.v:
        for k in sorted(v):
            print('  %s %-28s %s' % (k[0], inlineclass.plain(k[1])[:28], list(v[k])))


def cmd_descend(a):
    deck = load_deck(a.deck)
    clo = closure(deck)
    log = open(a.log, 'a')

    def say(s):
        print(s, flush=True)
        log.write(s + '\n')
        log.flush()
    # feasible rungs
    feas = {}
    cur = {}
    for name, spec in deck.items():
        body = get_body(spec)
        cur[name] = next((t for t, _ in spec['rungs'] if _norm(rung_body(spec, t)) == _norm(body)), None)
        feas[name] = [t for t, _ in spec['rungs'] if t in spec.get('feasible', [t for t, _ in spec['rungs']])]
        if cur[name] is None:
            say('!! %s: current body is not a deck rung' % name)
    origtag = dict(cur)
    for kv in (a.start.split(',') if a.start else []):
        n, t = kv.split('=')
        set_body(deck[n], rung_body(deck[n], t))
        cur[n] = t
    full = tree()
    v0 = vector(full)
    e0 = exact_set(full)
    say('START %s' % ' '.join('%s=%s' % kv for kv in sorted(cur.items())))
    say('START site-units %d exact %d' % (total(v0), len(e0)))
    ucache = {}

    def trial(moves):
        """moves: {name: tag}; -> (res_full, vector, exact)"""
        us = set()
        for n in moves:
            if n not in ucache:
                ucache[n] = users(deck, n, clo)
            us.update(ucache[n])
        for n, t in moves.items():
            set_body(deck[n], rung_body(deck[n], t))
        part = tree(sorted(us))
        res = merge(full, part)
        return res, vector(res), exact_set(res), len(us)

    orig = {n: get_body(s) for n, s in deck.items()}

    def undo(moves):
        for n in moves:
            set_body(deck[n], orig[n] if cur[n] == origtag[n] else rung_body(deck[n], cur[n]))

    def consider(moves, strict):
        nonlocal full, v0, e0
        res, v, e, nu = trial(moves)
        cl, fu, lines = compare(v0, v)
        tag = ' '.join('%s:%s->%s' % (n, cur[n], t) for n, t in moves.items())
        # an exact caller that breaks (beyond the count canaries) is shape evidence against the spelling
        ok = total(v) < total(v0) and (fu == 0 or not strict) and not (e0 - e - CANARIES)
        say('%s %-60s units %3d site-units %d -> %d (closer %d further %d) exact %+d/-%d' % (
            'KEEP' if ok else 'drop', tag, nu, total(v0), total(v), cl, fu, len(e - e0), len(e0 - e)))
        for ln in lines:
            say('      ' + ln)
        if e0 - e:
            sy = syms(res)
            say('      exact lost: ' + ' '.join('%s(%s)' % (x, sy.get(x, '?')[:22]) for x in sorted(e0 - e)))
        if e - e0:
            sy = syms(res)
            say('      exact gained: ' + ' '.join('%s(%s)' % (x, sy.get(x, '?')[:22]) for x in sorted(e - e0)))
        if ok:
            full, v0, e0 = res, v, e
            cur.update(moves)
        else:
            undo(moves)
        return ok

    changed = True
    rnd = 0
    while changed:
        changed = False
        rnd += 1
        say('ROUND %d single moves' % rnd)
        for name in deck:
            for t in feas[name]:
                if t != cur[name]:
                    if consider({name: t}, strict=True):
                        changed = True
        if a.pairs and not changed:
            say('ROUND %d pair moves' % rnd)
            names = [n for n in deck if len(feas[n]) > 1]
            for n1, n2 in itertools.combinations(names, 2):
                for t1 in feas[n1]:
                    for t2 in feas[n2]:
                        if t1 != cur[n1] and t2 != cur[n2]:
                            if consider({n1: t1, n2: t2}, strict=False):
                                changed = True
    say('FIXED POINT %s' % ' '.join('%s=%s' % kv for kv in sorted(cur.items())))
    say('FINAL site-units %d exact %d' % (total(v0), len(e0)))
    json.dump(full, open(a.out, 'w'))


def main():
    global _HDR
    _HDR = inlineclass.header_names()
    ap = argparse.ArgumentParser()
    sub = ap.add_subparsers(dest='cmd', required=True)
    p = sub.add_parser('eval')
    p.add_argument('--out', default=os.path.join(fe.SCR, 'jointsolve.json'))
    p.add_argument('--base')
    p.add_argument('--units')
    p.add_argument('--keep-standins', action='store_true')
    p.add_argument('--set', help='name=rung,... from --deck, applied for this evaluation')
    p.add_argument('--deck')
    p.add_argument('--keep', action='store_true', help='leave the --set bodies in the headers')
    p.add_argument('-v', action='store_true')
    p = sub.add_parser('copies')
    p.add_argument('deck')
    p.add_argument('--only')
    p = sub.add_parser('descend')
    p.add_argument('deck')
    p.add_argument('--pairs', action='store_true')
    p.add_argument('--log', default=os.path.join(fe.SCR, 'jointsolve.log'))
    p.add_argument('--out', default=os.path.join(fe.SCR, 'jointsolve_final.json'))
    p.add_argument('--start')
    a = ap.parse_args()
    {'eval': cmd_eval, 'copies': cmd_copies, 'descend': cmd_descend}[a.cmd](a)


if __name__ == '__main__':
    sys.exit(main())
