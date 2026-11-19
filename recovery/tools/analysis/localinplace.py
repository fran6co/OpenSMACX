#!/usr/bin/env python3
"""localinplace.py <unit> <0xADDR> [--write] [--iter N] [--max-sites K] [--only NAME] [--show KEY] [--list]
                   | --from FEALL.json [--min 100] [--skip A,B] [--only-addr A,B] [--write] [--json F]
Named-copy screen (MATCHING_GUIDE 2.7 "Named copies of a field"): for every single-assignment local of a
function that is initialised from a pure field/global read (`int proto = Vehs[id].proto_id;`,
`faction = v->faction_id;`, `int turn = TurnCurrentNum;`) it measures both spellings of every use:

  ALL:name     every use reads the field in place and the local is gone (declaration/assignment removed)
  S:name@Lnnn  one use reads the field in place, the local stays
  R:name@Lnnn  the reverse: one later textual occurrence of the same field read is spelled through the local
  RALL:name    every later occurrence of the field read is spelled through the local

A local qualifies when it is assigned exactly once, its address is never taken, the read has no call and no
side effect, and no local named in the read (an index, a pointer) is assigned after the copy. Whether the
field itself changes between the copy and a use is NOT proved: an in-place read after a store or a call
reloads, so a variant is only kept when the image agrees. Ranking key, as the structure ladder: call
multiset (analysis/callees.py counts), frame difference, candset.py count (one-side roles + disputed
pairs), slot-map difference, rows. --write keeps the best variant when the key says it is better:
a lower call or frame difference, or the same with fewer candset disagreements, never with more rows;
at an unchanged key, MIN_GAIN fewer rows; it then repeats (--iter rounds), so the sites of
several locals are found one at a time. The `home` column is the image's evidence for the local: `slot`
(the aligned image row stores a frame slot at the copy), `reg` (the image's aligned row defines a
callee-saved register), `none` (neither: no image home at the copy).
Env: FE_PFX_BASE (first lane prefix, default 21), FE_JOBS (lanes, default 5)."""
import sys, os, re, json, argparse
H = os.path.dirname(os.path.abspath(__file__))
T_ = os.path.dirname(H)
R = os.path.dirname(os.path.dirname(T_))
sys.path.insert(0, T_)
sys.path.insert(0, H)
os.chdir(R)
import fe
import armwo
import reusesweep as rs

PB = rs.PB
ROWS_SLACK = 0
MIN_GAIN = 3  # a keep on rows alone needs this many rows
INT_T = (r'(?:const\s+)?(?:unsigned\s+|signed\s+)?(?:int|BOOL|short|char|long|uint32_t|int32_t|uint16_t|int16_t|'
         r'uint8_t|int8_t|unsigned|size_t|DWORD|WORD|BYTE|LONG|UINT)')
PATH = r'[A-Za-z_]\w*(?:::\w+)*(?:\s*(?:\.|->)\s*\w+|\s*\[[^\[\]();=?]*(?:\[[^\[\]();=?]*\][^\[\]();=?]*)*\])*'
KW = {'return', 'sizeof', 'true', 'false', 'NULL', 'this', 'if', 'else', 'while', 'for', 'int', 'char', 'short',
      'unsigned', 'signed', 'const', 'long'}
ASSIGN = r'\s*(?:=(?!=)|\+=|-=|\*=|/=|%%=|\|=|&=|\^=|<<=|>>=|\+\+|--)'


def norm(e):
    return re.sub(r'\s+', '', e)


def candidates(text, addr):
    """-> (bs, be, masked, [dict(name, expr, ds, de, decl, uses [offsets], revs [(s, e)], line)])"""
    bs, be, _ = armwo.locate(text, '0x%08X' % addr)
    m = rs.mask(text)
    body = m[bs:be]
    out = []
    seen = set()
    for mm in re.finditer(r'(?<=[;{}])(\s*)((?:%s)\s+)?([A-Za-z_]\w*)\s*=\s*(%s)\s*;' % (INT_T, PATH), body):
        name, expr = mm.group(3), mm.group(4)
        if name in KW or name in seen:
            continue
        ne = norm(expr)
        if not ('.' in ne or '->' in ne or '[' in ne or ne[0].isupper()):
            continue
        if re.search(r'(?<![\w.>])%s(?!\w)' % re.escape(name), expr):
            continue
        ir = rs.ident_re(name)
        # assigned once, never address-taken, never a reference/pointer target
        nas = len(re.findall(r'(?<![\w.$>:])%s%s' % (re.escape(name), ASSIGN), body))
        nas += len(re.findall(r'(?:\+\+|--)\s*%s(?!\w)' % re.escape(name), body))
        if nas != 1 or re.search(r'(?<![&\w)\]])\s*&\s*%s(?!\w)' % re.escape(name), body):
            continue
        if not mm.group(2):
            # a separate assignment: the name must be a declared integer local or parameter
            dm = re.search(r'(?<![\w.])%s\s+(?:[\w\[\]\s,=*.-]*,\s*)?%s\s*[,;\[]' % (INT_T, re.escape(name)), m[max(0, bs - 400):be])
            if not dm or re.search(r'%s\s*\[' % re.escape(name), body[:mm.start()]):
                continue
        ds, de = bs + mm.start() + len(mm.group(1)), bs + mm.end()
        # locals named in the read must not be assigned after the copy
        bad = False
        for idn in set(re.findall(r'(?<![\w.>])([a-z_]\w*)', re.sub(r'(?:\.|->)\s*\w+', '', expr))):
            if idn in KW:
                continue
            if re.search(r'(?<![\w.$>:])%s%s' % (re.escape(idn), ASSIGN), m[de:be]) or \
                    re.search(r'(?:\+\+|--)\s*%s(?!\w)' % re.escape(idn), m[de:be]) or \
                    re.search(r'(?<![&\w)\]])\s*&\s*%s(?!\w)' % re.escape(idn), body):
                bad = True
        if bad:
            continue
        uses = [de + u.start() for u in ir.finditer(m[de:be])]
        if not uses or ir.search(m[bs:ds]) and mm.group(2):
            continue
        # a use before the copy on a loop back edge or through a goto would read another value: only
        # declarations with initialisers (scope starts at the copy) or names unread before it
        if not mm.group(2) and [u for u in ir.finditer(m[bs:ds]) if not re.match(r'\s*[,;]', m[bs + u.end():bs + u.end() + 3])]:
            continue
        # reverse sites: the same read spelled again after the copy
        revs = []
        pat = re.compile(r'(?<![\w.>])' + r'\s*'.join(re.escape(tok) for tok in re.findall(r'\w+|->|\S', expr)) + r'(?![\w\[]|\s*(?:\.|->))')
        for u in pat.finditer(m[de:be]):
            s, e = de + u.start(), de + u.end()
            if re.match(ASSIGN, m[e:e + 4]) or m[:s].rstrip().endswith('&'):
                continue
            revs.append((s, e))
        # a statement that names both the copy and the read compares the saved value with the live one
        # (`if (Count != count_at_entry)`): neither spelling may be changed there
        def stmt(o):
            a = max(m.rfind(';', 0, o), m.rfind('{', 0, o), m.rfind('}', 0, o))
            b = m.find(';', o)
            return a, b
        both = set()
        for s, e in revs:
            a, b = stmt(s)
            if any(a < u < b for u in uses):
                both.add((a, b))
        clean = lambda o: stmt(o) not in both
        seen.add(name)
        out.append(dict(name=name, expr=expr.strip(), ds=ds, de=de, decl=bool(mm.group(2)),
                        uses=[u for u in uses if clean(u)], revs=[r for r in revs if clean(r[0])],
                        mixed=len(both), line=text.count('\n', 0, ds) + 1))
    return bs, be, m, out


def apply(text, edits):
    """edits [(s, e, new)] non-overlapping"""
    for s, e, new in sorted(edits, reverse=True):
        text = text[:s] + new + text[e:]
    return text


def variants(text, c, max_sites):
    n = len(c['name'])
    ln = lambda o: text.count('\n', 0, o) + 1
    v = {}
    e = c['expr']
    # the whole copy statement goes (with its line when it stands alone)
    ls = text.rfind('\n', 0, c['ds']) + 1
    le = text.find('\n', c['de'])
    if not text[ls:c['ds']].strip() and not text[c['de']:le].strip():
        drop = (ls, le + 1, '')
    else:
        drop = (c['ds'], c['de'], '')
    if len(c['uses']) <= 4 * max_sites and not c['mixed'] and c['uses']:
        v['ALL:%s' % c['name']] = ([drop] + [(u, u + n, e) for u in c['uses']])
    if (c['mixed'] or 1 < len(c['uses'])) and len(c['uses']) <= max_sites:
        for u in c['uses']:
            v['S:%s@L%d.%d' % (c['name'], ln(u), u - text.rfind('\n', 0, u))] = [(u, u + n, e)]
    if c['revs']:
        if len(c['revs']) > 1:
            v['RALL:%s' % c['name']] = [(s, t, c['name']) for s, t in c['revs']]
        if len(c['revs']) <= max_sites:
            for s, t in c['revs']:
                v['R:%s@L%d.%d' % (c['name'], ln(s), s - text.rfind('\n', 0, s))] = [(s, t, c['name'])]
    return v


def call_diff(unit, text, addr):
    """number of callee count differences against the image (analysis/callees.py), 0 on failure"""
    try:
        import callees
        path = '%s/calls_%d.cpp' % (rs.SCR, PB)
        open(path, 'w', encoding='latin-1').write(text)
        c, err = fe.compile_unit(unit, path, PB)
        return callees.named_diff(c, text, addr) if c is not None else 0
    except Exception:
        return 0


def homes(unit, text, addr, base, cands):
    """image evidence per candidate: what the image's aligned row does where ours stores/defines the copy"""
    import regprio
    path = '%s/home_%d.cpp' % (rs.SCR, PB)
    open(path, 'w', encoding='latin-1').write(text)
    try:
        items, frame = regprio.cod_listing(unit, path, PB, base['sym'])
    except BaseException:
        return {}
    line_of, cur = {}, 0
    for it in items:
        if it[0] == 'line':
            cur = it[1]
        elif it[0] == 'ins':
            line_of[it[1]] = cur
    res = {}
    for c in cands:
        l0 = c['line']
        l1 = text.count('\n', 0, c['de']) + 1
        kinds = set()
        ours_home = 'slot' if c['name'] in frame else 'reg/none'
        for k, o, w in base['rows']:
            if w is None or not (l0 <= line_of.get(w[0], -1) <= l1):
                continue
            ot = o[1] if o is not None else ''
            if re.match(r'mov (?:dword|word|byte) ptr \[ebp [-+] \w+\], ', ot):
                kinds.add('slot')
            elif re.match(r'(?:mov|movsx|movzx|lea) (?:esi|edi|ebx),', ot):
                kinds.add('reg')
        res[c['name']] = (ours_home, '+'.join(sorted(kinds)) or 'none')
    return res


def sweep(unit, addr, write=False, max_sites=6, only=None, top=5, log=print, giant=4000):
    path = R + '/' + unit
    text = open(path, encoding='latin-1').read()
    bs, be, m, cands = candidates(text, addr)
    if only:
        cands = [c for c in cands if c['name'] in only]
    if not cands:
        log('  no single-assignment field copies')
        return dict(best=None, cands=0)
    base = rs.run(unit, addr, [('BASE', text)])['BASE']
    if base is None:
        log('  base fails')
        return None
    if base['img'] > giant:
        max_sites = min(max_sites, 4)
    vt = {}
    owner = {}
    for c in cands:
        for k, ed in variants(text, c, max_sites).items():
            vt[k] = apply(text, ed)
            owner[k] = c
    hm = homes(unit, text, addr, base, cands)
    for c in cands:
        log('  cand %-18s L%-5d uses %-2d respelled %-2d ours %-8s image %-9s = %s' % (
            c['name'], c['line'], len(c['uses']), len(c['revs']), hm.get(c['name'], ('?', '?'))[0],
            hm.get(c['name'], ('?', '?'))[1], c['expr'][:50]))
    res = rs.run(unit, addr, list(vt.items()))
    bf, bsl, bfr = rs.frame_slots(base)
    scored = []
    for k in vt:
        r = res.get(k)
        if r is None:
            continue
        fd, sd, fr = rs.frame_slots(r)
        scored.append(dict(key=k, fd=fd, sd=sd, rows=r['diff'], exact=r['exact'], cand=None, calls=None))
    scored.sort(key=lambda x: (x['fd'], x['rows']))
    bc = rs.cand_count(unit, text, addr, base)
    bcl = call_diff(unit, text, addr)
    # candset for the variants that could be kept: frame not worse and (rows lower, or among the best few)
    pool = [x for x in scored if x['fd'] <= bf]
    for x in sorted(pool, key=lambda x: x['rows'])[:top]:
        x['cand'] = rs.cand_count(unit, vt[x['key']], addr, res[x['key']])
        x['calls'] = call_diff(unit, vt[x['key']], addr)
    log('  BASE calls %d frame %d cand %d slots %d rows %d' % (bcl, bf, bc, bsl, base['diff']))
    for x in sorted(scored, key=lambda x: x['rows'])[:14]:
        log('  %-34s calls %s frame %d cand %s slots %d rows %d%s' % (
            x['key'], x['calls'], x['fd'], x['cand'], x['sd'], x['rows'], ' EXACT' if x['exact'] else ''))

    def better(x):
        # rows never rise in an automatic keep: an in-place read after the field changed is another
        # program, and only the image's rows at the site can tell
        if x['cand'] is None or x['rows'] > base['diff'] + ROWS_SLACK:
            return False
        if x['exact']:
            return True
        if x['calls'] != bcl:
            return x['calls'] < bcl
        if x['fd'] != bf:
            return x['fd'] < bf
        if x['cand'] != bc:
            return x['cand'] < bc
        return x['rows'] <= base['diff'] - MIN_GAIN
    good = sorted([x for x in scored if better(x)], key=lambda x: (not x['exact'], x['calls'], x['fd'], x['cand'], x['rows']))
    out = dict(base=base['diff'], bkey=(bcl, bf, bc, bsl, base['diff']), cands=len(cands),
               homes={k: v for k, v in hm.items()},
               variants=[(x['key'], x['calls'], x['fd'], x['cand'], x['sd'], x['rows']) for x in scored], best=None)
    if good:
        b = good[0]
        out['best'] = (b['key'], b['calls'], b['fd'], b['cand'], b['sd'], b['rows'])
        if write:
            open(path, 'w', encoding='latin-1').write(vt[b['key']])
            log('  WROTE %s (%d -> %d rows)' % (b['key'], base['diff'], b['rows']))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('unit', nargs='?')
    ap.add_argument('addr', nargs='?')
    ap.add_argument('--from', dest='frm')
    ap.add_argument('--min', type=int, default=100)
    ap.add_argument('--skip', default='')
    ap.add_argument('--only-addr', default='')
    ap.add_argument('--only', default='')
    ap.add_argument('--write', action='store_true')
    ap.add_argument('--iter', type=int, default=4)
    ap.add_argument('--max-sites', type=int, default=6)
    ap.add_argument('--json')
    ap.add_argument('--show')
    ap.add_argument('--list', action='store_true')
    a = ap.parse_args()
    todo = []
    if a.frm:
        skip = {s.upper().replace('0X', '').zfill(8) for s in a.skip.split(',') if s}
        only = {s.upper().replace('0X', '').zfill(8) for s in a.only_addr.split(',') if s}
        for x in sorted(json.load(open(a.frm)), key=lambda x: -x['diff']):
            if not x['exact'] and x['diff'] >= a.min and x['addr'] not in skip and (not only or x['addr'] in only):
                todo.append((x['unit'], '0x' + x['addr']))
    else:
        todo.append((a.unit if a.unit.startswith('src/') else 'src/' + a.unit, a.addr))
    if a.show or a.list:
        unit, addr = todo[0]
        text = open(R + '/' + unit, encoding='latin-1').read()
        bs, be, m, cands = candidates(text, int(addr, 16))
        for c in cands:
            if a.list:
                print('%-18s L%-5d uses %-2d respelled %-2d %s = %s' % (c['name'], c['line'], len(c['uses']), len(c['revs']),
                                                                     'decl' if c['decl'] else 'asgn', c['expr']))
            for k, ed in variants(text, c, 99).items():
                if k == a.show:
                    import difflib
                    sys.stdout.writelines(difflib.unified_diff(text.splitlines(1), apply(text, ed).splitlines(1), n=1))
        return
    allres = {}
    for unit, addr in todo:
        print('== %s %s' % (unit, addr), flush=True)
        hist = []
        for it in range(a.iter if a.write else 1):
            o = sweep(unit, int(addr, 16), a.write, a.max_sites, set(a.only.split(',')) if a.only else None,
                      log=lambda s: print(s, flush=True))
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
