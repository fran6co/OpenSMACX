#!/usr/bin/env python3
"""storesched.py [unit ...] [--addr A,..] [--src ALT] [--max N] [--json F]
Moved-store screen (MATCHING_GUIDE 3.6 "A member setter's parameter schedules the store early").
For every non-exact function: the stores `mov [reg + K], value` that both sides emit but at different places
inside one differing stretch (the image's row and ours are at most --max rows apart, default 12).  Frame-slot
stores are left out.  `SETTER?` marks a store that only moved across argument pushes: the Gamma::exec shape.  Per store: image address, our offset, how many rows
earlier (-) or later (+) ours schedules it, and whether the base register is the function's `this`
(the register that received ecx at entry) or another object.  A store to ANOTHER object that ours emits late
is the setter case: try the class's inline setter (a parameter is a named value, a raw `obj.f = v` is not).
Screen only; confirm with fe.py."""
import sys, os, re, json, argparse, collections
from concurrent.futures import ThreadPoolExecutor
T = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, T)
import fe
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import prosave

ST = re.compile(r'^mov (byte|word|dword) ptr \[(\w+) \+ (0x[0-9a-f]+|\d+)\], (.+)$')


def this_reg(rows, side):
    """register that holds ecx (this) after the prologue on one side, or None"""
    n = 0
    for k, o, c in rows:
        r = o if side == 0 else c
        if r is None:
            continue
        n += 1
        m = re.match(r'mov (ebx|esi|edi|ebp), ecx$', r[1])
        if m:
            return m.group(1)
        if n > 14 or r[1].startswith(('call', 'j')):
            break
    return None


def moved(rows, mx):
    out = []
    n = len(rows)
    i = 0
    while i < n:
        if rows[i][0] != '!!':
            i += 1
            continue
        j = i
        while j < n and rows[j][0] == '!!':
            j += 1
        # extend the stretch over short equal gaps
        seg = rows[max(0, i - mx):min(n, j + mx)]
        base = max(0, i - mx)
        im = [(q, r[1]) for q, r in enumerate(seg) if r[0] == '!!' and r[1] is not None and r[2] is None and ST.match(r[1][1])]
        ou = [(q, r[2]) for q, r in enumerate(seg) if r[0] == '!!' and r[2] is not None and r[1] is None and ST.match(r[2][1])]
        # replaced rows hold both sides: split them
        for q, r in enumerate(seg):
            if r[0] == '!!' and r[1] is not None and r[2] is not None:
                if ST.match(r[1][1]) and r[1][1] != r[2][1]:
                    im.append((q, r[1]))
                if ST.match(r[2][1]) and r[1][1] != r[2][1]:
                    ou.append((q, r[2]))
        used = set()
        for qi, a in im:
            ma = ST.match(a[1])
            for qo, b in ou:
                if qo in used:
                    continue
                mb = ST.match(b[1])
                # same field and width; the base register and a register value may be renamed
                if ma.group(1) == mb.group(1) and ma.group(3) == mb.group(3) and abs(qo - qi) <= mx and qo != qi:
                    va, vb = ma.group(4), mb.group(4)
                    if (va == vb) or (not va[0].isdigit() and not vb[0].isdigit() and va[0] != '-' and vb[0] != '-'):
                        lo, hi = sorted((qi, qo))
                        calls_i = sum(1 for r in seg[lo:hi + 1] if r[1] is not None and r[1][1].startswith('call'))
                        calls_o = sum(1 for r in seg[lo:hi + 1] if r[2] is not None and r[2][1].startswith('call'))
                        between = [r for r in seg[lo + 1:hi] if r[0] == '  ']
                        pushes = sum(1 for r in between if r[1][1].startswith('push'))
                        out.append(dict(img=a[0], ours=b[0], text=a[1], otext=b[1], d=qo - qi, base_i=ma.group(2), base_o=mb.group(2),
                                        calls=(calls_i, calls_o), pushes=pushes, between=len(between)))
                        used.add(qo)
                        break
        i = j
    # one record per image row
    seen = set()
    res = []
    for x in out:
        if x['img'] in seen:
            continue
        seen.add(x['img'])
        res.append(x)
    return res


def scan_unit(unit, addrs, pfx, mx, src=None):
    path = src or fe.R + '/' + unit
    text = open(path, encoding='latin-1').read()
    c, err = fe.compile_unit(unit, os.path.abspath(path), pfx)
    if c is None:
        return [dict(error=err, unit=unit)]
    out = []
    for addr in addrs:
        r = fe.evaluate(c, text, addr)
        if 'error' in r or r.get('exact'):
            continue
        mv = moved(r['rows'], mx)
        if not mv:
            continue
        ti, to = this_reg(r['rows'], 0), this_reg(r['rows'], 1)
        for x in mv:
            x['this'] = (x['base_i'] == ti) if ti else None
        out.append(dict(unit=unit, addr='%08X' % addr, sym=r['sym'], lo=r['lo'], diff=r['diff'], stores=mv, this=ti))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='*')
    ap.add_argument('--addr', default='')
    ap.add_argument('--src')
    ap.add_argument('--max', type=int, default=12)
    ap.add_argument('--json')
    ap.add_argument('--jobs', type=int, default=int(os.environ.get('FE_JOBS', '5')))
    ap.add_argument('--pfx-base', type=int, default=int(os.environ.get('FE_PFX_BASE', '21')))
    a = ap.parse_args()
    units = prosave.units_of(a.units)
    only = {int(x, 16) for x in a.addr.split(',') if x}
    work = []
    for u in units:
        try:
            t = open(fe.R + '/' + u, encoding='latin-1').read()
        except OSError:
            continue
        ad = [int(m.group(1), 16) for m in prosave.RX.finditer(t)
              if (int(m.group(1), 16) in only if only else 'BYTE_EXACT' not in m.group(2))]
        if ad:
            work.append((u, ad))
    if not work:
        return
    fe.compile_cmd(work[0][0])

    def lane(k):
        r = []
        for i in range(k, len(work), a.jobs):
            r += scan_unit(work[i][0], work[i][1], a.pfx_base + k, a.max, a.src if only else None)
        return r
    res = []
    with ThreadPoolExecutor(a.jobs) as ex:
        for o in ex.map(lane, range(a.jobs)):
            res += o
    res = [x for x in res if 'error' not in x]
    res.sort(key=lambda x: (not any(s['this'] is False for s in x['stores']), x['diff']))
    for x in res:
        print('%s %-24s %-36s diff=%-4d this=%s' % (x['addr'], x['unit'][4:], x['sym'][:36], x['diff'], x['this']))
        for s in x['stores']:
            if s['base_i'] in ('esp', 'ebp') or s['base_o'] in ('esp', 'ebp'):
                continue  # frame slots
            print('    %x  %-52s ours %+d rows  %s  equal rows between %d (pushes %d)%s' % (
                x['lo'] + s['img'], s['text'], s['d'], {True: 'this', False: 'OTHER', None: '?'}[s['this']], s['between'], s['pushes'],
                '  SETTER?' if s['pushes'] and s['between'] == s['pushes'] else ''))
    print('functions with moved stores: %d (to another object: %d)' % (len(res), sum(any(s['this'] is False for s in x['stores']) for x in res)))
    if a.json:
        json.dump(res, open(a.json, 'w'), indent=0)


if __name__ == '__main__':
    main()
