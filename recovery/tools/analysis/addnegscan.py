#!/usr/bin/env python3
"""addnegscan.py --from FEALL.json [--json F] | <unit> <0xADDR[,..]>
`add r, -K` against `sub r, K` census (ReportWin::draw_score 0x004A1140): VC6 emits `x - K` as `sub r, K`,
but when the same `x - K` is written twice and CSE'd (or reaches the code through an inline parameter, as
expand_rect's `right += h`), it emits `add r, -K`. Per function and K, image counts against ours of the two
forms; listed when the image's add -K count differs from ours while the sub K count moves the other way.
Screen only."""
import sys, os, re, argparse, collections, json
from concurrent.futures import ThreadPoolExecutor
H = os.path.dirname(os.path.abspath(__file__))
T = os.path.dirname(H)
sys.path.insert(0, T)
sys.path.insert(0, H)
import fe

ADDN = re.compile(r'add (?:dword ptr \[[^\]]+\]|e[abcd]x|esi|edi), -(0x[0-9a-f]+|\d+)$')
SUBK = re.compile(r'sub (?:dword ptr \[[^\]]+\]|e[abcd]x|esi|edi), (0x[0-9a-f]+|\d+)$')


def census(ins):
    a, s = collections.Counter(), collections.Counter()
    for t in ins:
        m = ADDN.match(t)
        if m:
            a[int(m.group(1), 0)] += 1
        m = SUBK.match(t)
        if m and 'esp' not in t:
            s[int(m.group(1), 0)] += 1
    return a, s


def scan(unit, addr, text, c):
    r = fe.evaluate(c, text, addr)
    if 'error' in r:
        return dict(addr=addr, unit=unit, error=r['error'])
    rows, lo = r['rows'], r['lo']
    iins = [x[1] for k, x, cc in rows if x is not None]
    oins = [x[1] for k, o, x in rows if x is not None]
    ai, si = census(iins)
    ao, so = census(oins)
    hits = []
    for k in sorted(set(ai) | set(ao)):
        da = ai[k] - ao[k]
        ds = si[k] - so[k]
        if da and (ds == -da or ds * da < 0):
            sites = ['%08X' % (lo + x[0]) for kk, x, cc in rows if x is not None and ADDN.match(x[1])
                     and int(ADDN.match(x[1]).group(1), 0) == k]
            hits.append('K=%d add-K img %d ours %d, sub K img %d ours %d  img add sites %s' % (
                k, ai[k], ao[k], si[k], so[k], ' '.join(sites[:6])))
    return dict(addr=addr, unit=unit, sym=r['sym'], diff=r['diff'], hits=hits)


def run_unit(unit, addrs, src, pfx):
    text = open(src, encoding='latin-1').read()
    c, err = fe.compile_unit(unit, src, pfx)
    if c is None:
        return [dict(addr=a, unit=unit, error='COMPILE FAIL') for a in addrs]
    out = []
    for a in addrs:
        try:
            out.append(scan(unit, a, text, c))
        except Exception as e:
            out.append(dict(addr=a, unit=unit, error=repr(e)))
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('unit', nargs='?')
    ap.add_argument('addr', nargs='?')
    ap.add_argument('--pfx', default=os.environ.get('FE_PFX_BASE', '2'))
    ap.add_argument('--from', dest='frm')
    ap.add_argument('--all', action='store_true')
    ap.add_argument('--json')
    a = ap.parse_args()
    if a.frm:
        work = {}
        for r in json.load(open(a.frm)):
            if a.all or not r.get('exact'):
                u = r['unit'] if r['unit'].startswith('src/') else 'src/' + r['unit']
                work.setdefault(u, []).append(int(str(r['addr']), 16))
        work = sorted(work.items())
    else:
        u = a.unit if a.unit.startswith('src/') else 'src/' + a.unit
        work = [(u, [int(v, 16) for v in a.addr.split(',')])]
    jobs = int(os.environ.get('FE_JOBS', '5'))
    base = int(a.pfx)

    def lane(k):
        res = []
        for i in range(k, len(work), jobs):
            u, ad = work[i]
            res += run_unit(u, ad, fe.R + '/' + u, str(base + k))
        return res
    allr = []
    with ThreadPoolExecutor(jobs) as ex:
        for o in ex.map(lane, range(jobs)):
            allr += o
    fl = [x for x in allr if x.get('hits')]
    fl.sort(key=lambda x: x['diff'])
    for x in fl:
        print('%08X %-44s %-22s diff %4d' % (x['addr'], x['sym'][:44], x['unit'][4:], x['diff']))
        for h in x['hits']:
            print('    ' + h)
    print('functions %d, flagged %d, errors %d' % (len(allr), len(fl), sum('error' in x for x in allr)))
    if a.json:
        json.dump(allr, open(a.json, 'w'), indent=0)


if __name__ == '__main__':
    main()
