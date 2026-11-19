#!/usr/bin/env python3
"""foldk.py --from FEALL.json [--min 1] [--max 100000] [--only ADDR,..] [--skip ADDR,..] [--json OUT]
Inverse of sitek.py (MATCHING_GUIDE 3.2, "A literal argument of a header inline is not the same as a local holding
that value"): finds the plain locals of a function body that hold an integer literal and are never written again
(`int lo = 1;`, not `const`), and compiles one variant per local with every use replaced by the literal and the
declaration removed.  A variant that lowers the rows means OUR source names a value the image folded.  Prints the
variants that move the rows.  FE_PFX_BASE / FE_JOBS lanes.  Screen only."""
import sys, os, re, json, argparse
R = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
sys.path.insert(0, R + '/recovery/tools'); sys.path.insert(0, R + '/recovery/tools/analysis')
import fe
import outside
from concurrent.futures import ThreadPoolExecutor

DECL = re.compile(r'^([ \t]*)(?:unsigned |signed )?(?:int|long|short|char|BOOL|uint32_t|int32_t|DWORD|UINT)\s+(\w+)\s*=\s*'
                  r'(-?(?:0x[0-9A-Fa-f]+|\d+)[uUlL]*)\s*;[^\n]*\n', re.M)


def locals_of(text, d):
    """[(decl_start, decl_end, name, literal)] for single-assignment literal locals in the body"""
    b, e = d[1], d[2]; body = text[b:e]; out = []
    for m in DECL.finditer(body):
        name = m.group(2)
        line = text.rfind('\n', 0, b + m.start(2)) + 1
        if re.search(r'\b(const|static|volatile)\b', text[line:b + m.start(2)]):
            continue
        rest = body[:m.start()] + body[m.end():]
        if re.search(r'(\+\+|--|&)\s*%s\b|\b%s\s*(\+\+|--|[-+*/%%&|^]?=(?!=)|<<=|>>=|\[)' % (name, name), rest):
            continue
        if len(re.findall(r'\b(?:int|long|short|char|BOOL|unsigned)\s+%s\b' % name, body)) != 1:
            continue
        if not re.search(r'\b%s\b' % name, rest):
            continue
        out.append((b + m.start(), b + m.end(), name, m.group(3)))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--from', dest='frm', required=True); ap.add_argument('--max', type=int, default=100000); ap.add_argument('--min', type=int, default=1)
    ap.add_argument('--only'); ap.add_argument('--skip'); ap.add_argument('--json')
    a = ap.parse_args()
    base = int(os.environ.get('FE_PFX_BASE', '21')); jobs = int(os.environ.get('FE_JOBS', '5'))
    norm = lambda s: {x.upper().replace('0X', '') for x in s.split(',')} if s else None
    only, skip = norm(a.only), norm(a.skip) or set()
    todo = [x for x in json.load(open(a.frm)) if not x['exact'] and a.min <= x['diff'] <= a.max and (not only or x['addr'] in only) and x['addr'] not in skip]
    todo.sort(key=lambda x: (x['unit'], x['addr']))
    fe.compile_cmd('src/main.cpp')
    out = open(a.json, 'a') if a.json else None

    def one(x, k):
        unit = x['unit']; addr = int(x['addr'], 16); pfx = str(base + k)
        text = open(R + '/' + unit, encoding='latin-1').read()
        d = outside.definition(text, addr)
        if not d or re.search(r'// ORIGINAL: 0x%08X [^\n]*\n(?://(?! ORIGINAL)[^\n]*\n)*// compile_context' % addr, text):
            return None
        ls = locals_of(text, d)
        if not ls:
            return None
        p = '%s/build/recovery/fe/fk%d.cpp' % (R, k)

        def rows(t):
            open(p, 'w', encoding='latin-1').write(t)
            c, err = fe.compile_unit(unit, p, pfx)
            if c is None:
                return None
            r = fe.evaluate(c, t, addr)
            return None if 'error' in r else (0 if r['exact'] else max(r['diff'], 1))
        b0 = rows(text); res = []
        for s, e, name, lit in ls:
            body = text[e:d[2]]
            t = text[:s] + re.sub(r'\b%s\b' % name, '(' + lit + ')' if lit.startswith('-') else lit, body) + text[d[2]:]
            res.append((text.count('\n', 0, s) + 1, name, lit, rows(t)))
        return dict(addr=x['addr'], unit=unit, sym=x['sym'], base=b0, locals=res)

    def lane(k):
        for i in range(k, len(todo), jobs):
            try:
                r = one(todo[i], k)
            except Exception as ex:
                r = None
                print('ERR', todo[i]['addr'], repr(ex), flush=True)
            if not r:
                continue
            mv = [s for s in r['locals'] if s[3] != r['base']]
            print('%s %s %s base %s locals %d%s' % (r['addr'], r['unit'], r['sym'][:40], r['base'], len(r['locals']),
                                                   ''.join('\n    %4s  L%d %s = %s' % (v, ln, n, lit) for ln, n, lit, v in mv)), flush=True)
            if out:
                out.write(json.dumps(r) + '\n'); out.flush()
    with ThreadPoolExecutor(jobs) as ex:
        list(ex.map(lane, range(jobs)))


if __name__ == '__main__':
    main()
