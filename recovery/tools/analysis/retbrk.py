#!/usr/bin/env python3
"""retbrk.py --from FEALL.json [--min N] [--max N] [--write] | <unit> <0xADDR> [--write]
Switch-arm exit screen (MATCHING_GUIDE 3.7 "Return-tail copies"; Net::drop_player 0x00632350): a switch that is
directly followed by `return X;` can leave its arms by `break;` or by writing `return X;` in the arm.  Both give
every arm its own epilogue copy, but they differ in how the function's OTHER conditional `return X` are folded
(written in the arms, VC6 folds one more bare conditional return into the shared copy).  For each such switch of
a non-exact function this tries, per switch and for all switches together:
  A  arms' `return X;` (the same X as the statement after the switch) -> `break;`
  B  arms' `break;` -> `return X;`
(only statements at the switch body's own brace level; nested switches are handled as their own sites) and
reports the fe rows; --write keeps the best variant that lowers the rows.  FE_PFX_BASE/FE_JOBS as usual."""
import sys, os, re, json, argparse
T = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, T)
import fe, perm  # noqa: E402


def body_span(text, addr):
    m = re.search(r'// ORIGINAL: 0x%08X ' % addr, text)
    if not m:
        return None
    i = text.find('{', text.find('\n', m.end()))
    # the function body: first `{` at line end after the marker block that is not in a comment line
    for mm in re.finditer(r'^[^/\n][^\n]*\)\s*(?:const\s*)?\{\s*$', text[m.end():], re.M):
        i = m.end() + mm.end() - 1
        while text[i] != '{':
            i -= 1
        break
    d = 0
    for j in range(i, len(text)):
        c = text[j]
        if c == '{':
            d += 1
        elif c == '}':
            d -= 1
            if d == 0:
                return i, j + 1
    return None


def match_brace(t, i):
    d = 0
    for j in range(i, len(t)):
        if t[j] == '{':
            d += 1
        elif t[j] == '}':
            d -= 1
            if d == 0:
                return j
    return -1


def sites(body):
    """-> one entry per `switch`: (open, close, X) when it is followed by `return X;`, else None"""
    out = []
    for m in re.finditer(r'\bswitch\s*\(', body):
        i = body.find('{', m.end())
        j = match_brace(body, i) if i >= 0 else -1
        mm = re.match(r'\s*return\s*([^;{}]*);', body[j + 1:]) if j >= 0 else None
        out.append((i, j, mm.group(1).strip()) if mm else None)
    return out


def apply(body, ords, mode):
    n = 0
    for k in ords:
        s = sites(body)[k]
        r = rewrite(body, s, mode) if s else None
        if r:
            body = r
            n += 1
    return body if n == len(ords) else None


def level0(seg):
    """offsets in seg (a switch body without its outer braces) that are at brace level 0 and not inside a nested switch"""
    d = 0
    ok = []
    for k, c in enumerate(seg):
        if c == '{':
            d += 1
        elif c == '}':
            d -= 1
        ok.append(d == 0)
    return ok


def rewrite(body, site, mode):
    i, j, x = site
    seg = body[i + 1:j]
    ok = level0(seg)
    pat = (r'\breturn\s*%s\s*;' % re.escape(x)) if mode == 'A' else r'\bbreak\s*;'
    new = 'break;' if mode == 'A' else ('return %s;' % x if x else 'return;')
    out = []
    last = 0
    n = 0
    for m in re.finditer(pat, seg):
        if not ok[m.start()]:
            continue
        out.append(seg[last:m.start()] + new)
        last = m.end()
        n += 1
    out.append(seg[last:])
    return (body[:i + 1] + ''.join(out) + body[j:]) if n else None


def run(unit, addr, write=False, quiet=False):
    text = open(fe.R + '/' + unit, encoding='latin-1').read()
    sp = body_span(text, addr)
    if not sp:
        return None
    a, b = sp
    body = text[a:b]
    ss = [k for k, s in enumerate(sites(body)) if s]
    if not ss:
        return None
    deck = [('base', text)]
    import itertools
    for mode in 'AB':
        good = [k for k in ss if apply(body, [k], mode)]
        subsets = [c for n in range(1, len(good) + 1) for c in itertools.combinations(good, n)] if len(good) <= 5 else [(k,) for k in good] + [tuple(good)]
        for c in subsets:
            r = apply(body, list(c), mode)
            if r:
                deck.append((mode + ''.join(str(k) for k in c), text[:a] + r + text[b:]))
    if len(deck) == 1:
        return None
    res = perm.run_deck(unit, '0x%08X' % addr, deck)
    base = next(d for w, d, ex, k in res if k == 'base')
    best = min(res, key=lambda r: r[1])
    if not quiet or best[1] < base:
        print('%s 0x%08X base %d best %s %d  (%s)' % (unit, addr, base, best[3], best[1], ' '.join('%s=%d' % (k, d) for w, d, ex, k in res[:8] if k != 'base')))
    if write and best[1] < base:
        open(fe.R + '/' + unit, 'w', encoding='latin-1').write(dict(deck)[best[3]])
    return base, best


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('unit', nargs='?')
    ap.add_argument('addr', nargs='?')
    ap.add_argument('--from', dest='src')
    ap.add_argument('--min', type=int, default=1)
    ap.add_argument('--max', type=int, default=100000)
    ap.add_argument('--write', action='store_true')
    a = ap.parse_args()
    if a.src:
        for r in sorted(json.load(open(a.src)), key=lambda r: r['diff']):
            if r['exact'] or not (a.min <= r['diff'] <= a.max):
                continue
            addr = int(r['addr'], 16) if isinstance(r['addr'], str) else r['addr']
            try:
                run(r['unit'], addr, a.write, quiet=True)
            except Exception as e:  # noqa: BLE001
                print('ERR', r['unit'], hex(addr), e)
    else:
        run(a.unit, int(a.addr, 16), a.write)


if __name__ == '__main__':
    main()
