#!/usr/bin/env python3
"""inlineclass.py [--out F.json] [--base F.json] [--callee NAME ...] [--top N] [--fe FEALL.json] [-v] [unit ...]

/Ob2 cost-class census of the header inlines (MATCHING_GUIDE 2.8, "A named local takes a small header
inline out of the free class").  One compile per unit (FE_PFX_BASE.., FE_JOBS lanes, about 40 s
tree-wide); for every marker function the image's named call multiset (callees.py) against ours.
For a callee F with an out-of-line copy in the image, `image calls > ours` in a function means our
source expands F at sites where the image calls it (our body is too cheap, or the site sat in an
inline owner), `image calls < ours` the reverse (our body is too dear, or an owner is hand-copied).

Per callee: functions and sites where ours expands more (o>i: `E+`), where ours calls more (`c+`), each
split into exact (x) and non-exact functions (--fe: a feall.py --all report; exact functions cannot
differ, so they only show for claims measured elsewhere), and the functions.  Callees are limited to
names defined with a body in a header (`--any` lifts that).  --base prints what moved against an
earlier --out report: the header-hypothesis screen (sites closer to / further from the image).
Screen only: leaf inlines have no call signature, so the unit is a count per function, not a site;
use callseq.py / inlinemap.py for the sites of one function.
"""
import sys, os, re, json, glob, argparse, collections, threading
from concurrent.futures import ThreadPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.dirname(HERE))
import fe  # noqa: E402
import callees  # noqa: E402

R = fe.R


def plain(sym):
    """undecorated `Class::name` / `name` of a mangled or C symbol (operators and special members keep their code)"""
    if sym.startswith('?'):
        m = re.match(r'\?(\?[0-9A-Z_]+|[A-Za-z_]\w*)@((?:\w+@)*)@', sym)
        if m:
            cls = '::'.join(reversed([p for p in m.group(2).split('@') if p]))
            n = m.group(1)
            n = {'?0': 'ctor', '?1': 'dtor'}.get(n, n)
            return (cls + '::' if cls else '') + n
        return sym
    return sym[1:] if sym.startswith('_') else sym


def header_names():
    """names with a body in a header: free/static inline functions and in-class members (text heuristic)"""
    names = set()
    for p in glob.glob(R + '/src/**/*.h', recursive=True):
        t = open(p, encoding='latin-1').read()
        for m in re.finditer(r'\b(~?[A-Za-z_]\w*)\s*\([^;{}()]*(?:\([^()]*\)[^;{}()]*)*\)\s*(?:const\s*)?(?::[^;{}]*)?\{', t):
            n = m.group(1)
            if n in ('if', 'for', 'while', 'switch', 'return', 'sizeof', 'defined'):
                continue
            names.add(n)
    return names


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='*')
    ap.add_argument('--out', default=os.path.join(fe.SCR, 'inlineclass.json'))
    ap.add_argument('--base')
    ap.add_argument('--fe')
    ap.add_argument('--callee', action='append', default=[])
    ap.add_argument('--any', action='store_true')
    ap.add_argument('--top', type=int, default=80)
    ap.add_argument('-v', action='store_true')
    a = ap.parse_args()
    cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
    units = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/')]
    if a.units:
        units = [u for u in units if u in a.units or os.path.basename(u) in a.units]
    work = []
    for u in units:
        try:
            t = open(R + '/' + u, encoding='latin-1').read()
        except OSError:
            continue
        addrs = [int(m.group(1), 16) for m in re.finditer(r'// ORIGINAL: 0x([0-9A-F]{8}) \S+ 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+', t)]
        if addrs:
            work.append((u, t, addrs))
    jobs = int(os.environ.get('FE_JOBS', '5'))
    base = int(os.environ.get('FE_PFX_BASE', '21'))
    callees.addr2sym(), callees.iat()
    fe.compile_cmd(work[0][0] if work else '')
    lock = threading.Lock()

    def lane(k):
        out = []
        for i in range(k, len(work), jobs):
            u, t, addrs = work[i]
            c, err = fe.compile_unit(u, R + '/' + u, str(base + k))
            if c is None:
                out.append(dict(unit=u, addr='-', sym='COMPILE FAIL', d={}))
                continue
            for addr in addrs:
                try:
                    with lock:
                        d = callees.diff(c, t, addr)
                        sym = fe.ident(t, addr)[0]
                except Exception:
                    continue
                if d is None:
                    continue
                ci, co = d
                dd = {k2: [ci[k2], co[k2]] for k2 in set(ci) | set(co) if ci[k2] != co[k2]}
                if dd:
                    out.append(dict(unit=u, addr='%08X' % addr, sym=sym, d=dd))
        return out

    res = []
    with ThreadPoolExecutor(jobs) as ex:
        for o in ex.map(lane, range(jobs)):
            res += o
    json.dump(res, open(a.out, 'w'), indent=0)
    for r in res:
        if r['addr'] == '-':
            print('COMPILE FAIL', r['unit'])
    exact = {}
    if a.fe:
        exact = {x['addr']: x['exact'] for x in json.load(open(a.fe))}
    hdr = header_names()
    copy = {v: k for k, v in callees.addr2sym().items()}

    def keep(n):
        p = plain(n)
        if a.callee:
            return any(k in p or k in n for k in a.callee)
        return a.any or p.split('::')[-1].replace('dtor', '~') in hdr or (p.endswith('dtor') and '~' + p.split('::')[0] in hdr)

    def table(rows):
        agg = collections.defaultdict(lambda: dict(Ef=[], cf=[], E=0, c=0))
        for r in rows:
            for n, (i, o) in r['d'].items():
                if not keep(n):
                    continue
                g = agg[n]
                if i > o:
                    g['E'] += i - o
                    g['Ef'].append((r['addr'], r['sym'], i - o))
                else:
                    g['c'] += o - i
                    g['cf'].append((r['addr'], r['sym'], o - i))
        return agg

    agg = table(res)
    tot = sum(g['E'] + g['c'] for g in agg.values())
    print('header-inline callees whose call counts differ: %d, site-units %d (ours expands more %d, ours calls more %d)' % (
        len(agg), tot, sum(g['E'] for g in agg.values()), sum(g['c'] for g in agg.values())))
    print('%-34s %-8s | %-13s | %-13s' % ('callee', 'copy', 'E+ sites/fn', 'c+ sites/fn'))

    def fl(lst):
        return ' '.join('%s%s:%d' % (ad, 'x' if exact.get(ad) else '', k) for ad, _, k in sorted(lst, key=lambda z: -z[2])[:12])
    for n, g in sorted(agg.items(), key=lambda kv: -(kv[1]['E'] + kv[1]['c']))[:a.top]:
        print('%-34s %-8s | %4d /%3d     | %4d /%3d' % (plain(n)[:34], '%08X' % copy[n] if n in copy else '-', g['E'], len(g['Ef']), g['c'], len(g['cf'])))
        if a.v or a.callee:
            if g['Ef']:
                print('      E+ (ours expands, image calls): ' + fl(g['Ef']))
            if g['cf']:
                print('      c+ (ours calls, image expands): ' + fl(g['cf']))
    if a.base:
        b = json.load(open(a.base))
        B = {(r['addr'], n): v for r in b for n, v in r['d'].items() if keep(n)}
        N = {(r['addr'], n): v for r in res for n, v in r['d'].items() if keep(n)}
        syms = {r['addr']: r['sym'] for r in res + b}
        better = worse = 0
        lines = []
        for k in sorted(set(B) | set(N)):
            ob = abs(B[k][0] - B[k][1]) if k in B else 0
            on = abs(N[k][0] - N[k][1]) if k in N else 0
            if ob != on:
                if on < ob:
                    better += ob - on
                else:
                    worse += on - ob
                lines.append('  %s %s %-28s %-24s %s -> %s' % ('+' if on < ob else '-', k[0], syms[k[0]][:28], plain(k[1])[:24],
                                                          B.get(k, '='), N.get(k, '=')))
        print('AGAINST BASE: site-units closer to the image %d, further %d (image,ours counts)' % (better, worse))
        for ln in lines:
            print(ln)


if __name__ == '__main__':
    sys.exit(main())
