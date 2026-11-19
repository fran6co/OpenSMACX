#!/usr/bin/env python3
"""sitek.py --from FEALL.json [--min 1] [--max 60] [--only ADDR,..] [--write] [--json OUT]
Site screen for the cost_factor rule (MATCHING_GUIDE 3.2, "Residue census"): an integer literal passed to a header
inline is folded into the expansion by the front end; the same value in a plain (non-const) local reaches the
expansion as a variable and is propagated later, which changes how the caller's other expansions are materialized.
For every call of a header inline in the function's body, each integer-literal argument becomes a local declared at
the top of the body (`int fe_kN = <literal>;`), one site per variant; prints the variants that move the rows.
`--write` keeps a variant that makes the function exact by fe (tag it WORKAROUND(decl) and name the local).
FE_PFX_BASE / FE_JOBS lanes. Screen only."""
import sys, os, re, json, argparse
R = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
sys.path.insert(0, R + '/recovery/tools'); sys.path.insert(0, R + '/recovery/tools/analysis')
import fe
import outside
from concurrent.futures import ThreadPoolExecutor

LIT = re.compile(r'^\s*-?(?:0x[0-9A-Fa-f]+|\d+)\s*$')


def sites(text, d, names):
    """[(start, end, literal)] of integer-literal arguments of calls to the named inlines inside the body"""
    b, e = d[1], d[2]; out = []
    for m in re.finditer(r'\b(\w+)\s*\(', text[b:e]):
        if m.group(1) not in names:
            continue
        j = b + m.end(); depth = 1; start = j; k = j
        while k < e and depth:
            ch = text[k]
            if ch in '([':
                depth += 1
            elif ch in ')]':
                depth -= 1
                if depth == 0:
                    if LIT.match(text[start:k]):
                        out.append((start, k, text[start:k].strip()))
                    break
            elif ch == ',' and depth == 1:
                if LIT.match(text[start:k]):
                    out.append((start, k, text[start:k].strip()))
                start = k + 1
            elif ch == '"':
                k += 1
                while text[k] != '"':
                    k += 2 if text[k] == '\\' else 1
            k += 1
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--from', dest='frm', required=True); ap.add_argument('--max', type=int, default=60); ap.add_argument('--min', type=int, default=1)
    ap.add_argument('--only'); ap.add_argument('--write', action='store_true'); ap.add_argument('--json')
    a = ap.parse_args()
    base = int(os.environ.get('FE_PFX_BASE', '21')); jobs = int(os.environ.get('FE_JOBS', '5'))
    only = {x.upper().replace('0X', '') for x in a.only.split(',')} if a.only else None
    todo = [x for x in json.load(open(a.frm)) if not x['exact'] and a.min <= x['diff'] <= a.max and (not only or x['addr'] in only)]
    todo.sort(key=lambda x: (x['unit'], x['addr']))
    headers = outside.load_headers()
    fe.compile_cmd('src/main.cpp')
    out = open(a.json, 'a') if a.json else None

    def one(x, k):
        unit = x['unit']; addr = int(x['addr'], 16); pfx = str(base + k)
        text = open(R + '/' + unit, encoding='latin-1').read()
        d = outside.definition(text, addr)
        if not d or re.search(r'// ORIGINAL: 0x%08X [^\n]*\n(?://(?! ORIGINAL)[^\n]*\n)*// compile_context' % addr, text):
            return None
        inc = outside.closure(unit, text, headers)
        names = {n for rel in inc for n, ret, params, b, e in outside.inline_defs(headers[rel])}
        ss = sites(text, d, names)
        res = []
        p = '%s/build/recovery/fe/sk%d.cpp' % (R, k)

        def rows(t):
            open(p, 'w', encoding='latin-1').write(t)
            c, err = fe.compile_unit(unit, p, pfx)
            if c is None:
                return None
            r = fe.evaluate(c, t, addr)
            return None if 'error' in r else (0 if r['exact'] else max(r['diff'], 1))
        b0 = rows(text)
        for n, (s, e, lit) in enumerate(ss):
            t = text[:d[1] + 1] + '\n    int fe_k%d = %s;' % (n, lit) + text[d[1] + 1:s] + ' fe_k%d' % n + text[e:]
            r = rows(t)
            ln = text.count('\n', 0, s) + 1
            res.append((ln, lit, r, t))
        best = [v for v in res if v[2] == 0]
        if a.write and best and b0:
            open(R + '/' + unit, 'w', encoding='latin-1').write(best[0][3])
        return dict(addr=x['addr'], unit=unit, sym=x['sym'], base=b0, sites=[(ln, lit, r) for ln, lit, r, t in res])

    def lane(k):
        for i in range(k, len(todo), jobs):
            try:
                r = one(todo[i], k)
            except Exception as ex:
                r = None
                print('ERR', todo[i]['addr'], repr(ex), flush=True)
            if not r:
                continue
            mv = [s for s in r['sites'] if s[2] is not None and s[2] != r['base']]
            print('%s %s %s base %s sites %d%s' % (r['addr'], r['unit'], r['sym'][:40], r['base'], len(r['sites']),
                                                  ''.join('\n    %4d  L%d literal %s' % (v, ln, lit) for ln, lit, v in mv)), flush=True)
            if out:
                out.write(json.dumps(r) + '\n'); out.flush()
    with ThreadPoolExecutor(jobs) as ex:
        list(ex.map(lane, range(jobs)))


if __name__ == '__main__':
    main()
