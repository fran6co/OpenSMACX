#!/usr/bin/env python3
"""loopexitscan.py <unit> <0xADDR[,..]> [--src ALT.cpp] [--pfx N] | --tree [unit ..] [--from FEALL.json] [--json F]

Loop-exit counter store screen (MATCHING_GUIDE 3.7 Frame and slots, Path::find 0x0059A530).  When a homed
variable is a loop's counter and VC6 runs the loop on a register (or strength-reduces it away), it still
stores the counter's exit value to the home: `mov [ebp - N], K` with K the loop bound, right after the loop
or hoisted before it.  Per non-exact function, the stores of an immediate K to an ebp slot are counted on
both sides (image slots mapped to ours through the aligned rows' votes, as stmovescan does); K must be a
loop bound on either side (`cmp x, K` followed within two instructions by a backward conditional jump).
  IMG  the image stores K to the slot more often than ours: that variable was the loop counter in the
       original (`v = K;` + a separate `for (j ..; j < K; ..)` -> `for (v ..; v < K; v++)`), or ours
       uses another counter.
  MOVE equal counts but an image store is unaligned (the Path::find case: ours writes `v = K;` itself).
  OURS the reverse: ours runs a loop on a homed variable the original did not (a separate counter).
Printed per flag: image address of the store (IMG) or our offset and source line (OURS), the slot, K and
counts.  A source pass (--src-pass) lists `v = K;` assignments within 6 lines of a `for` over another
variable bounded by K, the Path::find spelling, without compiling.  Screen only: the rewrite is
behaviour-preserving only when the loop has no break, the body does not read v and v's later value is K."""
import sys, os, re, json, argparse
from concurrent.futures import ThreadPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
sys.path.insert(0, HERE)
import fe
import tailscan
from rotscan import own_context
from stmovescan import slot_map

ST = re.compile(r'^mov (?:byte|word|dword) ptr \[ebp - (0x[0-9a-f]+)\], (0x[0-9a-f]+|\d+)$')
CMP = re.compile(r'^cmp [^,]+, (0x[0-9a-f]+|\d+)$')


def bounds(ins):
    out = set()
    for k, x in enumerate(ins):
        m = CMP.match(x[1])
        if not m:
            continue
        for y in ins[k + 1:k + 3]:
            if y[1].startswith('j') and not y[1].startswith('jmp') and y[3] is not None and y[3] <= x[0]:
                out.add(int(m.group(1), 0))
                break
    return out


def scan(unit, addr, text, c, cod):
    res = fe.evaluate(c, text, addr)
    if 'error' in res:
        return dict(addr=addr, error=res['error'])
    if res['exact']:
        return dict(addr=addr, sym=res['sym'], exact=True, flags=[])
    rows, lo = res['rows'], res['lo']
    lmap = tailscan.lines_of(cod, res['sym'], os.path.basename(unit).lower()) if cod else {}
    I = [r[1] for r in rows if r[1] is not None]
    O = [r[2] for r in rows if r[2] is not None]
    bnd = bounds(I) | bounds(O)
    smap = slot_map(rows)
    ci, co = {}, {}
    moved = set()
    for k_, x, y in rows:
        if x is None:
            continue
        m = ST.match(x[1])
        if m and 1 < int(m.group(2), 0) <= 4096:
            key = (smap.get(m.group(1), m.group(1)), int(m.group(2), 0))
            ci.setdefault(key, []).append(x)
            if k_ == '!!' and (y is None or not ST.match(y[1]) or ST.match(y[1]).group(2) != m.group(2)):
                moved.add(key)
    for x in O:
        m = ST.match(x[1])
        if m and 1 < int(m.group(2), 0) <= 4096:
            co.setdefault((m.group(1), int(m.group(2), 0)), []).append(x)
    flags = []
    for key in sorted(set(ci) | set(co)):
        a, b = ci.get(key, []), co.get(key, [])
        if len(a) == len(b) and key not in moved:
            continue
        side = 'IMG' if len(a) > len(b) else 'OURS' if len(a) < len(b) else 'MOVE'
        where = ['%x' % (lo + x[0]) for x in a] + (['ours'] + ['+%x %s' % (x[0], lmap.get(x[0], '?')) for x in b] if side == 'MOVE' else []) if side != 'OURS' else \
            ['+%x %s' % (x[0], lmap.get(x[0], '?')) for x in b]
        flags.append(dict(side=side, slot=key[0], k=key[1], bound=key[1] in bnd, img=len(a), ours=len(b), where=where))
    # an IMG and an OURS flag of the same K and count excess are a slot permutation, not a counter
    for f in flags:
        if f['side'] in ('IMG', 'OURS'):
            for g in flags:
                if g is not f and g['side'] not in ('IMG', 'OURS', f['side']) or g is f:
                    continue
                if g['side'] != f['side'] and g['k'] == f['k'] and abs(g['img'] - g['ours']) == abs(f['img'] - f['ours']) \
                        and not g.get('perm'):
                    f['perm'] = g['perm'] = True
                    break
    flags = [f for f in flags if not f.get('perm')]
    return dict(addr=addr, unit=unit, sym=res['sym'], diff=res['diff'], exact=False, flags=flags)


def report(r):
    if 'error' in r:
        return ['LX %08X error %s' % (r['addr'], r['error'])]
    return ['LX %08X %-34s diff=%-4d %-4s%s [ebp-%s] = %d  img %d ours %d  at %s' % (
        r['addr'], r['sym'][:34], r['diff'], f['side'], '*' if f['bound'] else ' ', f['slot'], f['k'], f['img'], f['ours'],
        ' '.join(f['where']))
        for f in r['flags']]


def run_unit(unit, addrs, src, pfx):
    text = open(src, encoding='latin-1').read()
    c, cod = tailscan.compile_listing(unit, src, pfx)
    if c is None:
        return [dict(addr=a, error='COMPILE FAIL') for a in addrs]
    out = []
    for a in addrs:
        try:
            out.append(scan(unit, a, text, c, cod))
        except Exception as e:
            out.append(dict(addr=a, error=repr(e)))
    for r in out:
        r['unit'] = unit
    return out


ASG = re.compile(r'^\s*(\w+)\s*=\s*(\w+)\s*;')
FOR = re.compile(r'for\s*\(\s*(?:int\s+)?(\w+)\s*=[^;]*;\s*(\w+)\s*<=?\s*([\w]+)\s*;')


def src_pass(units):
    for u in units:
        try:
            L = open(fe.R + '/' + u, encoding='latin-1').read().split('\n')
        except OSError:
            continue
        for n, s in enumerate(L):
            m = ASG.match(s)
            if not m or m.group(2).isdigit() and int(m.group(2)) < 2:
                continue
            for d in range(-6, 7):
                if d == 0 or not 0 <= n + d < len(L):
                    continue
                f = FOR.search(L[n + d])
                if f and f.group(3) == m.group(2) and f.group(1) != m.group(1):
                    print('LXS %s:%d `%s` near %d `%s`' % (u, n + 1, s.strip(), n + d + 1, L[n + d].strip()))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('unit', nargs='?')
    ap.add_argument('addr', nargs='?')
    ap.add_argument('--src')
    ap.add_argument('--pfx', default=os.environ.get('FE_PFX_BASE', '8'))
    ap.add_argument('--tree', nargs='*')
    ap.add_argument('--from', dest='frm')
    ap.add_argument('--src-pass', action='store_true')
    ap.add_argument('--json')
    a = ap.parse_args()
    cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
    all_units = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/')]
    if a.src_pass:
        src_pass(sorted(set(all_units)))
        return
    if a.tree is not None or a.frm:
        work = {}
        if a.frm:
            for r in json.load(open(a.frm)):
                if not r.get('exact'):
                    work.setdefault(r['unit'] if r['unit'].startswith('src/') else 'src/' + r['unit'], []).append(int(str(r['addr']), 16))
        else:
            units = all_units
            if a.tree:
                units = [u for u in units if u in a.tree or os.path.basename(u) in a.tree or u[4:] in a.tree]
            for u in units:
                try:
                    t = open(fe.R + '/' + u, encoding='latin-1').read()
                except OSError:
                    continue
                ad = [x for x in tailscan.markers(t) if own_context(t, x, u)]
                if ad:
                    work[u] = ad
        work = sorted(work.items())
        jobs = int(os.environ.get('FE_JOBS', '5'))
        fe.compile_cmd(work[0][0])
        base = int(a.pfx)

        def lane(k):
            r = []
            for i in range(k, len(work), jobs):
                u, ad = work[i]
                r += run_unit(u, ad, fe.R + '/' + u, str(base + k))
            return r
        allr = []
        with ThreadPoolExecutor(jobs) as ex:
            for o in ex.map(lane, range(jobs)):
                allr += o
    else:
        unit = a.unit if a.unit.startswith('src/') else 'src/' + a.unit
        allr = run_unit(unit, [int(x, 16) for x in a.addr.split(',')], a.src or fe.R + '/' + unit, a.pfx)
    allr.sort(key=lambda r: (r.get('diff', 0), r['addr']))
    nf = 0
    for r in allr:
        for l in report(r):
            print(l)
        nf += bool(r.get('flags'))
    print('functions with flagged exit stores: %d of %d' % (nf, len(allr)))
    if a.json:
        json.dump(allr, open(a.json, 'w'), indent=0)


if __name__ == '__main__':
    main()
