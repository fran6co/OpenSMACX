#!/usr/bin/env python3
"""chainscan.py [unit ...] [--all] [--json OUT] [--min N]
Chained stores (MATCHING_GUIDE 3.3 "Chained rectangles"): a register value stored to one memory slot
and then offset by a constant (`add r,K`, `lea r2,[r+K]`, through `mov r2,r` copies) before being stored
to another slot. In the image this is the trace of a source that writes one rectangle edge from another
rectangle's edge (`icon.top = lower.top + 3`, `text.left = icon.right + 2`); a side that instead computes
both from one shared value or reloads a member shows no chain for that K.

For each function (non-exact by default), prints the image chains with no same-(K, kind) chain on our
side and our chains the image lacks. kind = f (frame slot -> frame slot), m (other memory), x (mixed).
Screen only."""
import sys, os, re, json, argparse, collections
from concurrent.futures import ThreadPoolExecutor
T = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, T)
sys.path.insert(0, T + '/analysis')
import fe  # noqa: E402
from constfold import MARK, writes, R32  # noqa: E402

NUM = r'(0x[0-9a-f]+|\d+)'


def slotkind(m):
    return 'f' if re.search(r'\[e[bs]p [+-] ', m) or m in ('[ebp]', '[esp]') else 'm'


def chains(stream):
    """[(i_store_src, j_store_dst, K, kind, src_mem, dst_mem)]"""
    labels = {x[3] for x in stream if x[3] is not None}
    stored = {}   # reg -> mem it was stored to (value unchanged since)
    der = {}      # reg -> (src_mem, i_src, K)
    out = []
    for j, (o, t, _, _) in enumerate(stream):
        if o in labels:
            stored, der = {}, {}
        mn, _, op = t.partition(' ')
        m = re.fullmatch(r'mov (dword ptr )?(\[[^\]]+\]), (e[a-z]{2})', t)
        if m and m.group(3) in R32:
            r, mem = m.group(3), m.group(2)
            if r in der and der[r][0] != mem and der[r][2] != 0:
                sm, si, k = der[r]
                kd = slotkind(sm) + slotkind(mem)
                out.append((si, j, k, 'f' if kd == 'ff' else 'm' if kd == 'mm' else 'x', sm, mem))
            if r not in stored:
                stored[r] = (mem, j)
            # any store into a slot invalidates registers mirroring that slot
            for q in list(stored):
                if q != r and stored[q][0] == mem:
                    del stored[q]
            continue
        m = re.fullmatch(r'lea (e[a-z]{2}), \[(e[a-z]{2}) ([+-]) %s\]' % NUM, t)
        if m and m.group(2) != 'ebp' and m.group(2) != 'esp':
            d, s, k = m.group(1), m.group(2), int(m.group(4), 0) * (1 if m.group(3) == '+' else -1)
            src = stored.get(s) and (stored[s][0], stored[s][1], 0) or der.get(s)
            stored.pop(d, None)
            der.pop(d, None)
            if src:
                der[d] = (src[0], src[1], src[2] + k)
            continue
        m = re.fullmatch(r'(add|sub) (e[a-z]{2}), %s' % NUM, t)
        if m:
            r, k = m.group(2), int(m.group(3), 0) * (1 if m.group(1) == 'add' else -1)
            src = stored.get(r) and (stored[r][0], stored[r][1], 0) or der.get(r)
            stored.pop(r, None)
            der.pop(r, None)
            if src:
                der[r] = (src[0], src[1], src[2] + k)
            continue
        m = re.fullmatch(r'(inc|dec) (e[a-z]{2})', t)
        if m:
            r, k = m.group(2), 1 if m.group(1) == 'inc' else -1
            src = stored.get(r) and (stored[r][0], stored[r][1], 0) or der.get(r)
            stored.pop(r, None)
            der.pop(r, None)
            if src:
                der[r] = (src[0], src[1], src[2] + k)
            continue
        m = re.fullmatch(r'mov (e[a-z]{2}), (e[a-z]{2})', t)
        if m:
            d, s = m.group(1), m.group(2)
            stored.pop(d, None)
            der.pop(d, None)
            if s in stored:
                stored[d] = stored[s]
            if s in der:
                der[d] = der[s]
            continue
        if mn.startswith('j') or mn.startswith('ret'):
            stored, der = {}, {}
            continue
        if mn == 'call':
            for r_ in ('eax', 'ecx', 'edx'):
                stored.pop(r_, None)
                der.pop(r_, None)
            continue
        for r_ in list(stored):
            if writes(t, r_):
                del stored[r_]
        for r_ in list(der):
            if writes(t, r_):
                del der[r_]
    return out


def scan_unit(unit, pfx, mode, only):
    text = open(fe.R + '/' + unit, encoding='latin-1').read()
    c, err = fe.compile_unit(unit, fe.R + '/' + unit, pfx)
    if c is None:
        return [('ERR', unit, err)]
    out = []
    for m in MARK.finditer(text):
        addr = int(m.group(1), 16)
        if only and addr not in only:
            continue
        r = fe.evaluate(c, text, addr)
        if 'error' in r or (mode == 'nonexact' and r['exact']):
            continue
        rows = r['rows']
        O = [x[1] for x in rows if x[1] is not None]
        C = [x[2] for x in rows if x[2] is not None]
        co, cc = chains(O), chains(C)
        so = collections.Counter((k, kd) for _, _, k, kd, _, _ in co)
        sc = collections.Counter((k, kd) for _, _, k, kd, _, _ in cc)
        io = []
        left = sc.copy()
        for i, j, k, kd, sm, dm in co:
            if left[(k, kd)] > 0:
                left[(k, kd)] -= 1
                continue
            io.append(dict(at='%08X' % (r['lo'] + O[j][0]), k=k, kind=kd, src=sm, dst=dm,
                           txt='%s .. %s' % (O[i][1], O[j][1])))
        left = so.copy()
        oo = []
        for i, j, k, kd, sm, dm in cc:
            if left[(k, kd)] > 0:
                left[(k, kd)] -= 1
                continue
            oo.append(dict(at='+%X' % C[j][0], k=k, kind=kd, src=sm, dst=dm, txt='%s .. %s' % (C[i][1], C[j][1])))
        if io or oo:
            out.append(dict(unit=unit, addr='%08X' % addr, fn=r['sym'], exact=r['exact'], diff=r['diff'],
                            img=io, ours=oo))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='*')
    ap.add_argument('--all', action='store_true')
    ap.add_argument('--json')
    ap.add_argument('--addr', default='')
    ap.add_argument('--min', type=int, default=1, help='minimum unmatched image chains to list a function')
    ap.add_argument('--kind', default='fmx')
    ap.add_argument('--jobs', type=int, default=int(os.environ.get('FE_JOBS', '5')))
    ap.add_argument('--pfx-base', type=int, default=int(os.environ.get('FE_PFX_BASE', '21')))
    a = ap.parse_args()
    mode = 'all' if a.all else 'nonexact'
    only = {int(x, 16) for x in a.addr.split(',') if x}
    cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
    units = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/')]
    units = [u for u in units if '/vendor/' not in u and u.endswith('.cpp')]
    if a.units:
        units = [u for u in units if u in a.units or os.path.basename(u) in a.units]
    fe.compile_cmd(units[0])

    def lane(k):
        r = []
        for i in range(k, len(units), a.jobs):
            r += scan_unit(units[i], a.pfx_base + k, mode, only)
        return r
    res = []
    with ThreadPoolExecutor(a.jobs) as ex:
        for o in ex.map(lane, range(a.jobs)):
            res += o
    for e in res:
        if isinstance(e, tuple):
            print('ERROR', e[1], (e[2] or '')[:200])
    res = [x for x in res if isinstance(x, dict)]
    if a.json:
        json.dump(res, open(a.json, 'w'), indent=0)
    n = 0
    for x in sorted(res, key=lambda x: -len(x['img'])):
        io = [p for p in x['img'] if p['kind'] in a.kind]
        if len(io) < a.min:
            continue
        n += 1
        print('%s %s %-40s %s diff=%d img-only=%d ours-only=%d' % (
            'EXACT' if x['exact'] else 'NONEX', x['addr'], x['fn'][:40], x['unit'], x['diff'], len(io),
            len(x['ours'])))
        for p in io:
            print('   img  %s K=%-5d %s %s -> %s   %s' % (p['at'], p['k'], p['kind'], p['src'], p['dst'], p['txt']))
        for p in x['ours']:
            if p['kind'] in a.kind:
                print('   ours %s K=%-5d %s %s -> %s   %s' % (p['at'], p['k'], p['kind'], p['src'], p['dst'],
                                                             p['txt']))
    print('functions listed=%d' % n)


if __name__ == '__main__':
    main()
