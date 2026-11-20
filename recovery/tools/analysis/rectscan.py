#!/usr/bin/env python3
"""rectscan.py <unit> <0xADDR[,..]> [--src ALT.cpp] | --from FEALL.json [--json F] [--sites]
Rect-helper argument-order screen (MATCHING_GUIDE 3.3 "set_rect against per-field stores", PickWin::init
0x004887A0). Within one straight-line block, a group is four dword stores to [R+K], [R+K+4], [R+K+8],
[R+K+0xC] (frame slots included) in that field order. Each group is described by
  shape  `C` the four stores are consecutive (an inline setter's stores after its arguments were evaluated)
         `I` other instructions are interleaved (per-field assignments)
  defs   the order in which the four stored registers were last defined before their store, as field
         letters (l t r b); `-` for an immediate. A set_rect/set_rectangle_dimensions expansion evaluates its
         arguments right to left (`brtl`), a per-field spelling defines each value just before its store.
Image groups are matched with ours by (base kind, K): a function is listed when some image group has a shape
or defs order that ours does not. Screen only: read each site, confirm with fe.py / recover measure."""
import sys, os, re, argparse, collections, json
from concurrent.futures import ThreadPoolExecutor
H = os.path.dirname(os.path.abspath(__file__))
T = os.path.dirname(H)
sys.path.insert(0, T)
sys.path.insert(0, H)
import fe
import tailscan

ST = re.compile(r'mov dword ptr \[(e[abcd]x|esi|edi|ebp)(?: ([+-]) (0x[0-9a-f]+|\d+))?\], (\S+)$')
REG = ('eax', 'ebx', 'ecx', 'edx', 'esi', 'edi')
SUB = {'ax': 'eax', 'al': 'eax', 'ah': 'eax', 'bx': 'ebx', 'bl': 'ebx', 'bh': 'ebx', 'cx': 'ecx', 'cl': 'ecx',
       'ch': 'ecx', 'dx': 'edx', 'dl': 'edx', 'dh': 'edx', 'si': 'esi', 'di': 'edi'}
NOWRITE = ('cmp', 'test', 'push')
BREAK = ('call', 'ret', 'jmp')


def dest(t):
    mn, _, ops = t.partition(' ')
    if mn in NOWRITE or not ops:
        return None
    d = ops.split(', ')[0]
    d = SUB.get(d, d)
    return d if d in REG else None


def groups(ins):
    """ins: [(off, text, target)] -> [(base, K, shape, defs, idx_first)]"""
    targets = set(t for _, _, t in ins if t is not None)
    out = []
    blocks, cur = [], []
    for i, (off, t, tg) in enumerate(ins):
        if off in targets and cur:
            blocks.append(cur)
            cur = []
        cur.append(i)
        mn = t.split(' ')[0]
        if mn in BREAK or mn.startswith('j'):
            blocks.append(cur)
            cur = []
    if cur:
        blocks.append(cur)
    for b in blocks:
        st = []  # (pos in block, base, K, src)
        for p, i in enumerate(b):
            m = ST.match(ins[i][1])
            if m:
                k = int(m.group(3) or '0', 0) * (-1 if m.group(2) == '-' else 1)
                st.append((p, m.group(1), k, m.group(4)))
        used = set()
        for a, (p0, base, k0, s0) in enumerate(st):
            if a in used:
                continue
            # a group: the first store of K followed (any order, within 40 instructions) by K+4, K+8, K+0xC;
            # only a K with no K-4 store nearby starts a group
            seq = [a]
            for f in (1, 2, 3):
                nxt = sorted((abs(st[c][0] - p0), c) for c in range(len(st)) if c not in used and c != a
                             and st[c][1] == base and st[c][2] == k0 + 4 * f and abs(st[c][0] - p0) <= 40)
                nxt = [c for _, c in nxt]
                if not nxt:
                    break
                seq.append(nxt[0])
            if len(seq) < 4:
                continue
            seq.sort()
            sorder = ''.join('ltrb'[(st[c][2] - k0) // 4] for c in seq)
            ps = [st[c][0] for c in seq]
            if ps[-1] - ps[0] > 40:
                continue
            ok = all(dest(ins[b[q]][1]) != base for q in range(ps[0] + 1, ps[-1]))
            if not ok:
                continue
            used.update(seq)
            shape = ('C' if ps[-1] - ps[0] == 3 else 'I') + sorder
            # last definition of each source before its store
            dpos = []
            for c in sorted(seq, key=lambda c: st[c][2]):
                f = (st[c][2] - k0) // 4
                src = st[c][3]
                if src not in REG:
                    dpos.append(None)
                    continue
                q = st[c][0] - 1
                while q >= 0 and dest(ins[b[q]][1]) != src:
                    q -= 1
                dpos.append(q if q >= 0 else -1)
            order = sorted([(d, 'ltrb'[f]) for f, d in enumerate(dpos) if d is not None and d >= 0])
            defs = ''.join(x for _, x in order) or '-'
            kind = 'f' if base == 'ebp' else 'm'
            out.append([kind, k0, shape, defs, b[ps[0]], b, ps[0], ps[-1]])
    # loads in each group's window: from the previous group's last store (same block) to its last store
    for n, g in enumerate(out):
        b, p0, p1 = g[5], g[6], g[7]
        start = 0
        for h in out[:n]:
            if h[5] is b and h[7] < p1:
                start = max(start, h[7] + 1)
        loads = []
        for q in range(start, p1 + 1):
            t = ins[b[q]][1]
            mn, _, ops = t.partition(' ')
            if mn == 'lea' or not ops:
                continue
            parts = ops.split(', ')
            srcs = parts[1:] if not (mn in NOWRITE) else parts
            if mn not in ('mov', 'movsx', 'movzx', 'lea') and mn not in NOWRITE:
                srcs = parts  # read-modify-write reads its destination too
            for o in srcs:
                m = re.search(r'\[(e[abcd]x|esi|edi|ebp)(?: ([+-]) (0x[0-9a-f]+|\d+))?\]', o)
                if m:
                    k = int(m.group(3) or '0', 0) * (-1 if m.group(2) == '-' else 1)
                    loads.append(('F' if m.group(1) == 'ebp' else 'M') + '%x' % k if m.group(1) != 'ebp' else 'F')
                elif '<r>' in o and '[' in o:
                    loads.append('G')
        g.append(loads)
    out = [tuple(g[:5]) + (g[8],) for g in out]
    return out


def scan(unit, addr, text, c, cod):
    r = fe.evaluate(c, text, addr)
    if 'error' in r:
        return dict(addr=addr, unit=unit, error=r['error'])
    rows = r['rows']
    lo = r['lo']
    oins = sorted((x[0], x[1], x[3]) for k, o, x in rows if x is not None)
    iins = sorted((x[0], x[1], x[3]) for k, x, cc in rows if x is not None)
    lmap = tailscan.lines_of(cod, r['sym'], os.path.basename(unit).lower()) if cod else {}
    gi, go = groups(iins), groups(oins)
    # pair image and our groups by (kind, K) in code order (frame groups: kind only, slots differ)
    def bykey(gs):
        d = collections.defaultdict(list)
        for g in gs:
            d[(g[0], g[1] if g[0] == 'm' else 0)].append(g)
        return d
    di, do = bykey(gi), bykey(go)
    flags = []
    for k in di:
        for gi_, go_ in zip(di[k], do.get(k, [])):
            li, lo2 = gi_[5], go_[5]
            why = []
            if gi_[2] != go_[2]:
                why.append('SHAPE %s/%s' % (gi_[2], go_[2]))
            if li != lo2 and sorted(li) == sorted(lo2):
                why.append('ORDER')
            elif sorted(li) != sorted(lo2):
                why.append('LOADS')
            if why:
                flags.append('%s %+6x %-14s img %08X %s | ours %4x %s %s' % (
                    k[0], k[1], ','.join(why), lo + iins[gi_[4]][0], ' '.join(li), oins[go_[4]][0],
                    lmap.get(oins[go_[4]][0], '?'), ' '.join(lo2)))
    unpaired = sum(max(0, len(v) - len(do.get(k, []))) for k, v in di.items()) + \
        sum(max(0, len(v) - len(di.get(k, []))) for k, v in do.items())
    norder = sum('ORDER' in f for f in flags)
    nshape = sum('SHAPE' in f for f in flags)
    return dict(addr=addr, unit=unit, sym=r['sym'], diff=r['diff'], exact=r['exact'],
                extra=norder, less=nshape, missing=unpaired,
                img_sites=flags, our_sites=[], ngi=len(gi), ngo=len(go))


def run_unit(unit, addrs, src, pfx):
    try:
        text = open(src, encoding='latin-1').read()
    except OSError:
        return [dict(addr=a, unit=unit, error='NO SOURCE') for a in addrs]
    c, cod = tailscan.compile_listing(unit, src, pfx)
    if c is None:
        return [dict(addr=a, unit=unit, error='COMPILE FAIL') for a in addrs]
    out = []
    for a in addrs:
        try:
            out.append(scan(unit, a, text, c, cod))
        except Exception as e:
            out.append(dict(addr=a, unit=unit, error=repr(e)))
    return out


def show(x, sites=True):
    if 'error' in x:
        return ['ERR %08X %s %s' % (x['addr'], x['unit'], x['error'])]
    l = ['%08X %-40s %-24s diff %4d  groups img %d ours %d  order %d shape %d unpaired %d' % (
        x['addr'], x['sym'][:40], x['unit'][4:], x['diff'], x['ngi'], x['ngo'], x['extra'], x['less'],
        x['missing'])]
    if sites:
        l += ['    ' + s for s in x['img_sites']]
        l += ['    OUR ' + s for s in x['our_sites']]
    return l


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('unit', nargs='?')
    ap.add_argument('addr', nargs='?')
    ap.add_argument('--src')
    ap.add_argument('--pfx', default=os.environ.get('FE_PFX_BASE', '2'))
    ap.add_argument('--from', dest='frm')
    ap.add_argument('--all', action='store_true', help='with --from: include exact functions (validation)')
    ap.add_argument('--sites', action='store_true')
    ap.add_argument('--json')
    a = ap.parse_args()
    if a.frm:
        work = {}
        for r in json.load(open(a.frm)):
            if a.all or not r.get('exact'):
                u = r['unit'] if r['unit'].startswith('src/') else 'src/' + r['unit']
                work.setdefault(u, []).append(int(str(r['addr']), 16))
        work = sorted(work.items())
        jobs = int(os.environ.get('FE_JOBS', '5'))
        fe.compile_cmd(work[0][0])
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
        fl = [x for x in allr if 'error' not in x and (x['extra'] or x['less'])]
        fl.sort(key=lambda x: (-(x['extra'] + x['less']), x['diff']))
        for x in fl:
            print('\n'.join(show(x, sites=a.sites)))
        print('functions %d, flagged %d, errors %d' % (len(allr), len(fl), sum('error' in x for x in allr)))
        if a.json:
            json.dump(allr, open(a.json, 'w'), indent=0)
        return
    unit = a.unit if a.unit.startswith('src/') else 'src/' + a.unit
    src = os.path.abspath(a.src) if a.src else fe.R + '/' + unit
    for x in run_unit(unit, [int(v, 16) for v in a.addr.split(',')], src, a.pfx):
        print('\n'.join(show(x)))


if __name__ == '__main__':
    main()
