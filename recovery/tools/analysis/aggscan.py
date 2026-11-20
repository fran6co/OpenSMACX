#!/usr/bin/env python3
"""aggscan.py <unit> <0xADDR[,..]> [--src ALT.cpp] | --from FEALL.json [--json F] [--min N]
Mixed member/pointer aggregate screen (MATCHING_GUIDE 3.3 "VC6 never forwards a stored value between a
member access and an access through a pointer to the same aggregate", PickWin::init 0x004887A0).
Within one straight-line block (no call, jump, ret or jump target between), on both sides:
  dead   a store to [R+K] (R not ebp/esp) overwritten by a later store to [R+K] with no read of it between
  rel    a read of [R+K] after a store to [R+K] in the same block (the stored value not forwarded); `n`
         when only stores through the same base register R lie between (neighbouring fields of the same
         object: the PickWin shape), `x` when a store through another base lies between (possible alias)
Keyed by displacement K, the image's events are compared with ours; a function is listed when the image
has a `dead` or `rel/n` event at a K where ours has fewer.  IMG > OURS at K: the original reached that
aggregate partly through a pointer (`RECT *r = &m_`, or an inline taking `&m_` such as rect_width /
rect_height / set_rect / offset_rect) and partly as members.  Screen only: read each site, confirm with
fe.py / recover measure.
--stores: list functions by stores to [R+K] (any non-frame base) the image makes at displacements where ours
makes fewer (a kept "dead" store across blocks, or a field ours never writes back)."""
import sys, os, re, argparse, collections, json
from concurrent.futures import ThreadPoolExecutor
H = os.path.dirname(os.path.abspath(__file__))
T = os.path.dirname(H)
sys.path.insert(0, T)
sys.path.insert(0, H)
import fe
import tailscan

FRAME = os.environ.get('AGG_FRAME') == '1'  # also frame slots [ebp - N] (local aggregates)
MEM = re.compile(r'(?:(byte|word|dword) ptr )?\[(e[abcd]x|esi|edi%s)(?: ([+-]) (0x[0-9a-f]+|\d+))?\]'
                 % ('|ebp' if FRAME else ''))


def hx(d):
    return ('-' if d < 0 else '+') + '%x' % abs(d)


def disp(m):
    v = int(m.group(4) or '0', 0)
    return -v if m.group(3) == '-' else v
REGS = {'eax': ('eax', 'ax', 'al', 'ah'), 'ebx': ('ebx', 'bx', 'bl', 'bh'), 'ecx': ('ecx', 'cx', 'cl', 'ch'),
        'edx': ('edx', 'dx', 'dl', 'dh'), 'esi': ('esi', 'si'), 'edi': ('edi', 'di')}
SUB = {s: r for r, ss in REGS.items() for s in ss}
NOWRITE = ('cmp', 'test', 'push')
EBB = os.environ.get('AGG_EBB') == '1'  # keep store state across a conditional jump's fall-through
BREAK = ('call', 'ret', 'jmp')


def events(ins):
    """ins: [(off, text, target)] -> list of (kind, disp, idx, store_idx)"""
    targets = set(t for _, _, t in ins if t is not None)
    out = []
    stores = {}  # (R, K, width) -> idx of last store, reads since
    seq = []  # stores in order: (idx, R)

    def reset():
        stores.clear()
        seq.clear()
    for i, (off, t, tg) in enumerate(ins):
        if off in targets:
            reset()
        mn, _, ops = t.partition(' ')
        if mn in BREAK or (mn.startswith('j') and not EBB) or mn.startswith('rep') or mn.startswith('movs') or mn.startswith('stos'):
            reset()
            continue
        parts = [p.strip() for p in ops.split(', ')] if ops else []
        if mn == 'lea':
            parts = parts[:1]
        # reads
        srcs = parts[1:] if mn in ('mov', 'movsx', 'movzx') else parts
        for p in srcs:
            m = MEM.search(p)
            if not m:
                continue
            key = (m.group(2), disp(m))
            for (r, k, w), (si, rd) in list(stores.items()):
                if (r, k) == key and not rd:
                    between = [b for j, b in seq if j > si]
                    kind = 'rel/n' if all(b == r for b in between) else 'rel/x'
                    out.append((kind, k, i, si))
                    stores[(r, k, w)] = (si, True)
        # store
        if parts and mn not in NOWRITE:
            m = MEM.search(parts[0])
            if m and mn not in ('lea',):
                r, k, w = m.group(2), disp(m), m.group(1)
                prev = stores.get((r, k, w))
                if prev is not None and not prev[1] and mn == 'mov':
                    out.append(('dead', k, i, prev[0]))
                stores[(r, k, w)] = (i, mn != 'mov')
                seq.append((i, r))
        # register writes invalidate stores based on that register
        if parts and mn not in NOWRITE and not MEM.search(parts[0]) and parts[0] in SUB:
            reg = SUB[parts[0]]
            for key in [q for q in stores if q[0] == reg]:
                del stores[key]
        if mn in ('xchg',) and len(parts) == 2 and parts[1] in SUB:
            reg = SUB[parts[1]]
            for key in [q for q in stores if q[0] == reg]:
                del stores[key]
    return out


def scan(unit, addr, text, c, cod):
    r = fe.evaluate(c, text, addr)
    if 'error' in r:
        return dict(addr=addr, unit=unit, error=r['error'])
    rows = r['rows']
    lo = r['lo']
    oins = [(x[0], x[1], x[3]) for k, o, x in rows if x is not None]
    iins = [(x[0], x[1], x[3]) for k, x, cc in rows if x is not None]
    oins.sort()
    iins.sort()
    lmap = tailscan.lines_of(cod, r['sym'], os.path.basename(unit).lower()) if cod else {}
    ie, oe = events(iins), events(oins)
    ci = collections.Counter((k, d) for k, d, _, _ in ie if k != 'rel/x')
    co = collections.Counter((k, d) for k, d, _, _ in oe if k != 'rel/x')
    extra = {kd: n - co.get(kd, 0) for kd, n in ci.items() if n > co.get(kd, 0)}
    less = {kd: n - ci.get(kd, 0) for kd, n in co.items() if n > ci.get(kd, 0)}
    isites = ['%-5s %-7s %08X %-36s <- %08X %s' % (k, hx(d), lo + iins[i][0], iins[i][1], lo + iins[s][0], iins[s][1])
              for k, d, i, s in ie if (k, d) in extra]
    osites = ['%-5s %-7s %4x %-5s %-36s <- %x %s' % (k, hx(d), oins[i][0], lmap.get(oins[i][0], '?'), oins[i][1],
                                                     oins[s][0], oins[s][1])
              for k, d, i, s in oe if (k, d) in less]
    # stores per displacement (any non-frame base): the image's stores ours does not have at all
    si, so = stores_by_disp(iins), stores_by_disp(oins)
    st = {d: len(v) - len(so.get(d, ())) for d, v in si.items() if len(v) > len(so.get(d, ()))}
    ssites = ['st    %-7s img %d ours %d  %s' % (hx(d), len(si[d]), len(so.get(d, ())),
                                                  ' '.join('%08X' % (lo + iins[i][0]) for i in si[d][:6]))
              for d in sorted(st)]
    li, lo_ = leas(iins), leas(oins)
    lea_img = {d: n - lo_.get(d, 0) for d, n in li.items() if n > lo_.get(d, 0)}
    lea_our = {d: n - li.get(d, 0) for d, n in lo_.items() if n > li.get(d, 0)}
    return dict(addr=addr, unit=unit, sym=r['sym'], diff=r['diff'], exact=r['exact'],
                lea_img=sorted(lea_img.items()), lea_our=sorted(lea_our.items()),
                extra=sum(extra.values()), less=sum(less.values()), st=sum(st.values()),
                img_sites=isites + ssites, our_sites=osites)


def leas(ins):
    """member addresses formed (`lea r, [R + K]`, R not ebp/esp), by K"""
    out = collections.Counter()
    for off, t, tg in ins:
        m = re.fullmatch(r'lea (e[abcd]x|esi|edi), \[(e[abcd]x|esi|edi) \+ (0x[0-9a-f]+)\]', t)
        if m and int(m.group(3), 0) >= 0x10:
            out[int(m.group(3), 0)] += 1
    return out


def stores_by_disp(ins):
    out = collections.defaultdict(list)
    for i, (off, t, tg) in enumerate(ins):
        mn, _, ops = t.partition(' ')
        if mn in NOWRITE or mn == 'lea' or not ops:
            continue
        m = MEM.search(ops.split(', ')[0])
        if m and m.group(4):
            out[disp(m)].append(i)
    return out


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
    l = ['%08X %-40s %-24s diff %4d  img-only %d  ours-only %d  img-only stores %d' % (
        x['addr'], x['sym'][:40], x['unit'][4:], x['diff'], x['extra'], x['less'], x.get('st', 0))]
    if sites:
        l += ['    IMG ' + s for s in x['img_sites']]
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
    ap.add_argument('--min', type=int, default=1)
    ap.add_argument('--stores', action='store_true', help='with --from: rank by image-only store displacements')
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
        if a.stores:
            fl = [x for x in allr if 'error' not in x and x['st'] >= a.min]
            fl.sort(key=lambda x: (-x['st'] / max(1, x['diff']), x['diff']))
        else:
            fl = [x for x in allr if 'error' not in x and x['extra'] >= a.min]
            fl.sort(key=lambda x: (-x['extra'], x['diff']))
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
