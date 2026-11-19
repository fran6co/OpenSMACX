#!/usr/bin/env python3
"""webflow.py <unit> <0xADDR> [--write] [--show KEY] [--list] | --from FEALL.json [--min N] [--json F]
Value-flow screen (MATCHING_GUIDE 2.7 "value flow"; Buffer::tile 0x005DA860): the image keeps ONE web where
our source splits a value across a parameter and a separate local, or the reverse.

  IP0:L     `T L = P op E;` (P a parameter not read after it)  ->  `P = P op E; T L = P;`
            (the parameter is adjusted in place and copied; Buffer::tile `x -= k; startX = x;`)
  IPALL:L   as IP0 and every use of L reads P: the local is gone
  IPS:L@..  as IP0 and one use of L reads P (the fit test of Buffer::tile)
  CPALL:L   `T L = P;` (a plain copy, P not read after it): every use of L reads P
  SPLIT:P   the reverse: `P op= E;` / `P = f(P);` assigned once, read after:  `T P_w = P op E;` and
            every later read of P reads P_w (the image keeps the parameter untouched in its slot)

Ranking and keep rule as localinplace.py: call multiset, frame, candset count, slot map, rows.
Variants are NOT checked for meaning beyond the textual conditions (P not read after the copy, not
address-taken, single assignment); a definition inside a loop that reads P earlier in the loop body is
another program.  Read every kept variant by hand.
Env: FE_PFX_BASE, FE_JOBS."""
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
import localinplace as li

ASSIGN = li.ASSIGN
INT_T = li.INT_T
OPS = ('+', '-', '*', '/', '%', '|', '&', '^', '<<', '>>')


def params(text, bs):
    """parameter names and declared types of the function whose body starts at bs"""
    q = text.rfind(')', 0, bs)
    d, p = 0, q
    while p >= 0:
        if text[p] == ')':
            d += 1
        elif text[p] == '(':
            d -= 1
            if d == 0:
                break
        p -= 1
    inner = re.sub(r'/\*.*?\*/|//[^\n]*', '', text[p + 1:q], flags=re.S)
    out = {}
    for part in rs.split_top(inner) if inner.strip() else []:
        part = part.split('=')[0].strip()
        m = re.match(r'(.*?)([A-Za-z_]\w*)\s*(\[\s*\w*\s*\])?$', part, re.S)
        if m and m.group(1).strip() and part != 'void':
            out[m.group(2)] = m.group(1).strip()
    return out


def nassign(name, s):
    n = len(re.findall(r'(?<![\w.$>:])%s%s' % (re.escape(name), ASSIGN), s))
    return n + len(re.findall(r'(?:\+\+|--)\s*%s(?!\w)' % re.escape(name), s))


def addr_taken(name, s):
    return bool(re.search(r'(?<![&\w)\]])\s*&\s*%s(?!\w)' % re.escape(name), s))


def enclosing(m, bs, pos):
    """heads of the blocks enclosing pos (outermost first), the function body excluded"""
    st = []
    for i in range(bs + 1, pos):
        c = m[i]
        if c == '{':
            h = max(m.rfind('{', bs, i), m.rfind('}', bs, i))
            st.append(m[h + 1:i])
        elif c == '}' and st:
            st.pop()
    return st


def in_loop(m, bs, pos):
    return any(re.search(r'\b(?:for|while|do)\b', h) for h in enclosing(m, bs, pos))


def candidates(text, addr):
    bs, be, _ = armwo.locate(text, '0x%08X' % addr)
    m = rs.mask(text)
    body = m[bs:be]
    ps = params(text, bs)
    out = []
    for P, ptype in ps.items():
        if '*' in ptype or '[' in ptype or '&' in ptype or not re.search(INT_T, ptype):
            continue
        ir = rs.ident_re(P)
        if addr_taken(P, body):
            continue
        # forward: T L = <expr naming P>;   P not read or written after
        for mm in re.finditer(r'(?<=[;{}])(\s*)((?:%s)\s+)?([A-Za-z_]\w*)\s*=\s*([^;=]*?)\s*;' % INT_T, body):
            L, expr = mm.group(3), mm.group(4)
            if L == P or L in li.KW or not ir.search(expr) or '?' in expr or L.endswith('_') or '\n' in expr:
                continue
            flat = re.sub(r'\[[^\[\]]*\]', '', expr)
            if li.norm(expr) != P and not re.search(r'(?<![\w.>])%s\s*(?:[-+*/%%|&^]|<<|>>)(?!=)|(?:[-+*/%%|&^]|<<|>>)\s*%s(?!\w)'
                                                   % (re.escape(P), re.escape(P)), flat):
                continue
            if not mm.group(2) and not re.search(r'(?<![\w.])%s\s+(?:[\w\[\]\s,=*.-]*,\s*)?%s\s*[,;\[=]'
                                                 % (INT_T, re.escape(L)), body):
                continue
            ds, de = bs + mm.start() + len(mm.group(1)), bs + mm.end()
            nxt = ir.search(m[de:be])
            # P not read after the copy: no later reference, or the first one overwrites it (`x = startX;`)
            if nxt and not re.match(r'\s*=(?!=)', m[de + nxt.end():de + nxt.end() + 3]):
                continue
            if nxt and in_loop(m, bs, de + nxt.start()) and not in_loop(m, bs, ds):
                continue
            if nassign(L, body) != 1 or addr_taken(L, body) or in_loop(m, bs, ds):
                continue
            if not mm.group(2) and rs.ident_re(L).search(m[bs:ds]):
                continue
            uses = [de + u.start() for u in rs.ident_re(L).finditer(m[de:be])]
            if not uses:
                continue
            plain = li.norm(expr) == P
            out.append(dict(kind='CP' if plain else 'IP', P=P, L=L, ptype=ptype, expr=expr.strip(), ds=ds, de=de,
                            decl=mm.group(2), uses=uses, line=text.count('\n', 0, ds) + 1))
        # reverse: P assigned once in place and read after
        if nassign(P, body) == 1:
            am = re.search(r'(?<=[;{}])(\s*)%s\s*(=(?!=)|\+=|-=|\*=|/=|%%=|\|=|&=|\^=|<<=|>>=)\s*([^;]*?)\s*;'
                           % re.escape(P), body)
            if am:
                ds, de = bs + am.start() + len(am.group(1)), bs + am.end()
                if enclosing(m, bs, ds):
                    continue
                rhs = am.group(3)
                if am.group(2) == '=' and not ir.search(rhs):
                    continue
                later = [de + u.start() for u in ir.finditer(m[de:be])]
                if later:
                    out.append(dict(kind='SPLIT', P=P, L=P + '_w', ptype=ptype, op=am.group(2), expr=rhs.strip(),
                                    ds=ds, de=de, uses=later, line=text.count('\n', 0, ds) + 1))
    return bs, be, m, out


def variants(text, c, max_sites=6):
    ln = lambda o: text.count('\n', 0, o) + 1
    v = {}
    P, L, e = c['P'], c['L'], c['expr']
    nL = len(L)
    if c['kind'] == 'IP':
        decl = c['decl'] or ''
        new = '%s = %s;\n%s%s%s = %s;' % (P, e, ' ' * 0, decl, L, P)
        # keep the indentation of the original statement for the second line
        ls = text.rfind('\n', 0, c['ds']) + 1
        ind = text[ls:c['ds']] if not text[ls:c['ds']].strip() else ''
        new = '%s = %s;\n%s%s%s = %s;' % (P, e, ind, decl, L, P)
        v['IP0:%s' % L] = [(c['ds'], c['de'], new)]
        v['IPALL:%s' % L] = [(c['ds'], c['de'], '%s = %s;' % (P, e))] + [(u, u + nL, P) for u in c['uses']]
        if 1 < len(c['uses']) <= max_sites:
            for u in c['uses']:
                v['IPS:%s@L%d' % (L, ln(u))] = [(c['ds'], c['de'], new), (u, u + nL, P)]
    elif c['kind'] == 'CP':
        ls = text.rfind('\n', 0, c['ds']) + 1
        le = text.find('\n', c['de'])
        drop = (ls, le + 1, '') if not text[ls:c['ds']].strip() and not text[c['de']:le].strip() else (c['ds'], c['de'], '')
        v['CPALL:%s' % L] = [drop] + [(u, u + nL, P) for u in c['uses']]
    else:
        op = c['op']
        rhs = e if op == '=' else '%s %s %s' % (P, op[:-1], e if re.match(r'^[\w.\->\[\]]+$', e) else '(%s)' % e)
        ty = re.sub(r'\bconst\b', '', c['ptype']).strip()
        v['SPLIT:%s' % P] = [(c['ds'], c['de'], '%s %s = %s;' % (ty, L, rhs))] + [(u, u + len(P), L) for u in c['uses']]
    return v


def sweep(unit, addr, write=False, top=5, log=print, only=None):
    path = R + '/' + unit
    text = open(path, encoding='latin-1').read()
    bs, be, m, cands = candidates(text, addr)
    if only:
        cands = [c for c in cands if c['L'] in only or c['P'] in only]
    if not cands:
        log('  no value-flow candidates')
        return dict(best=None, cands=0)
    base = rs.run(unit, addr, [('BASE', text)])['BASE']
    if base is None:
        log('  base fails')
        return None
    vt = {}
    for c in cands:
        log('  cand %-5s %-14s P=%-12s L%-5d uses %-2d %s' % (c['kind'], c['L'], c['P'], c['line'], len(c['uses']), c['expr'][:50]))
        for k, ed in variants(text, c).items():
            vt[k] = li.apply(text, ed)
    res = rs.run(unit, addr, list(vt.items()))
    bf, bsl, bfr = rs.frame_slots(base)
    scored = []
    for k in vt:
        r = res.get(k)
        if r is None:
            continue
        fd, sd, fr = rs.frame_slots(r)
        scored.append(dict(key=k, fd=fd, sd=sd, rows=r['diff'], exact=r['exact'], cand=None, calls=None))
    bc = rs.cand_count(unit, text, addr, base)
    bcl = li.call_diff(unit, text, addr)
    for x in sorted([x for x in scored if x['fd'] <= bf], key=lambda x: x['rows'])[:top]:
        x['cand'] = rs.cand_count(unit, vt[x['key']], addr, res[x['key']])
        x['calls'] = li.call_diff(unit, vt[x['key']], addr)
    log('  BASE calls %d frame %d cand %d slots %d rows %d' % (bcl, bf, bc, bsl, base['diff']))
    for x in sorted(scored, key=lambda x: x['rows'])[:10]:
        log('  %-30s calls %s frame %d cand %s slots %d rows %d%s' % (
            x['key'], x['calls'], x['fd'], x['cand'], x['sd'], x['rows'], ' EXACT' if x['exact'] else ''))

    def better(x):
        if x['cand'] is None:
            return False
        if x['exact']:
            return True
        k0 = (bcl, bf, bc)
        k1 = (x['calls'], x['fd'], x['cand'])
        if k1 != k0:
            return k1 < k0 and x['rows'] <= base['diff'] + 10
        return x['rows'] <= base['diff'] - 3
    good = sorted([x for x in scored if better(x)], key=lambda x: (not x['exact'], x['calls'], x['fd'], x['cand'], x['rows']))
    out = dict(base=base['diff'], bkey=(bcl, bf, bc, bsl, base['diff']), cands=len(cands),
               variants=[(x['key'], x['calls'], x['fd'], x['cand'], x['sd'], x['rows']) for x in scored], best=None)
    if good:
        b = good[0]
        out['best'] = (b['key'], b['calls'], b['fd'], b['cand'], b['sd'], b['rows'], b['exact'])
        log('  BEST %s' % (out['best'],))
        if write:
            open(path, 'w', encoding='latin-1').write(vt[b['key']])
            log('  WROTE %s (%d -> %d rows)' % (b['key'], base['diff'], b['rows']))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('unit', nargs='?')
    ap.add_argument('addr', nargs='?')
    ap.add_argument('--from', dest='frm')
    ap.add_argument('--min', type=int, default=1)
    ap.add_argument('--skip', default='')
    ap.add_argument('--write', action='store_true')
    ap.add_argument('--only', default='')
    ap.add_argument('--json')
    ap.add_argument('--show')
    ap.add_argument('--list', action='store_true')
    a = ap.parse_args()
    todo = []
    if a.frm:
        skip = {s.upper().replace('0X', '').zfill(8) for s in a.skip.split(',') if s}
        for x in sorted(json.load(open(a.frm)), key=lambda x: x['diff']):
            if not x['exact'] and x['diff'] >= a.min and x['addr'] not in skip:
                todo.append((x['unit'], '0x' + x['addr']))
    else:
        todo.append((a.unit if a.unit.startswith('src/') else 'src/' + a.unit, a.addr))
    if a.list or a.show:
        for unit, addr in todo:
            text = open(R + '/' + unit, encoding='latin-1').read()
            try:
                bs, be, m, cands = candidates(text, int(addr, 16))
            except SystemExit:
                continue
            for c in cands:
                if a.list:
                    print('%s %s %-5s %-14s P=%-12s L%-5d uses %-2d %s' % (addr, unit, c['kind'], c['L'], c['P'], c['line'],
                                                                     len(c['uses']), c['expr'][:60]))
                for k, ed in variants(text, c, 99).items():
                    if k == a.show:
                        import difflib
                        sys.stdout.writelines(difflib.unified_diff(text.splitlines(1), li.apply(text, ed).splitlines(1), n=1))
        return
    allres = {}
    for unit, addr in todo:
        try:
            text = open(R + '/' + unit, encoding='latin-1').read()
            if not candidates(text, int(addr, 16))[3]:
                continue
        except SystemExit:
            continue
        print('== %s %s' % (unit, addr), flush=True)
        o = sweep(unit, int(addr, 16), a.write, log=lambda s: print(s, flush=True),
                  only=set(a.only.split(',')) if a.only else None)
        allres['%s %s' % (unit, addr)] = o
        if a.json:
            json.dump(allres, open(a.json, 'w'), indent=1)


if __name__ == '__main__':
    main()
