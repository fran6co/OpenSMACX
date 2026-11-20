#!/usr/bin/env python3
"""retmap.py [unit ...] [--all] [--addr A,..] [--json F] [--show]
Return-site map (MATCHING_GUIDE 3.7 "Epilogue copy or branch"): for every non-exact function (--all: every
marker) list the `ret` tails on each side of fe's alignment and pair them. A tail is the run of value set /
pops / `mov esp,ebp` / EH unlink before a `ret`; it is entered by fallthrough (F) and by J jumps (jcc or jmp)
into any of its instructions. Per function it prints the `ret` count image/ours and every unpaired tail:

  IMG-COPY   the image has an epilogue copy where ours branches (ours: `jcc`/`jmp` into another tail);
             ours writes that exit so that it reaches a shared tail: a `||` exit, a `return` written where
             the image had a `goto` to the final return, or an arm that ends in the return's statements
  OURS-COPY  ours has a copy where the image branches to a shared tail: split a `||` exit into separate
             `if (..) goto done;` tests, or let the exit fall to the final return (MATCHING_GUIDE 3.7)

each with the image address, our offset and our source line (`L123`; image-only rows get the line of the
nearest aligned row). --show prints the full tail list of each side. Markers with `// compile_context <tu>` are
measured in that unit's compile (one listing per lane and context). Screen only: confirm with fe.py."""
import sys, os, re, json, argparse, collections
from concurrent.futures import ThreadPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
sys.path.insert(0, HERE)
import fe  # noqa: E402
import tailscan  # noqa: E402

MARK = re.compile(r'// ORIGINAL: 0x([0-9A-F]{8}) (\S+) 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)')
TAILI = re.compile(r'^(pop |leave|mov esp, ebp|add esp, |lea esp, |mov eax, |mov al, |xor eax, eax|or eax, -1|'
                   r'mov ecx, dword ptr \[ebp - |mov dword ptr fs:|movsx eax, |movzx eax, |sbb eax|neg eax|'
                   r'mov ax, |fld )')


def side(rows, k):
    """[(row index, instruction)] for side k (1 image, 2 ours)"""
    return [(i, r[k]) for i, r in enumerate(rows) if r[k] is not None]


def tails(seq):
    """-> list of dict(ret=row, start=row, value=text, F=bool, J=[row of each jump into the tail],
    jcc=n conditional jumps among J)"""
    offs = {ins[0]: n for n, (_, ins) in enumerate(seq)}
    out = []
    for n, (ri, ins) in enumerate(seq):
        if not ins[1].startswith('ret'):
            continue
        s = n
        while s > 0 and TAILI.match(seq[s - 1][1][1]):
            s -= 1
        val = '?'
        for m in range(s, n):
            t = seq[m][1][1]
            if re.match(r'(mov eax, |xor eax, eax|or eax, -1|mov al, |movsx eax|movzx eax)', t):
                val = t
        prev = seq[s - 1][1][1] if s > 0 else 'ret'
        F = not re.match(r'(jmp|ret)', prev)
        span = set(seq[m][1][0] for m in range(s, n + 1))
        J, jcc = [], 0
        for m, (rj, ij) in enumerate(seq):
            if ij[3] is not None and ij[3] in span and ij[1].startswith('j'):
                J.append(rj)
                jcc += 0 if ij[1].startswith('jmp') else 1
        out.append(dict(ret=ri, start=seq[s][0], value=val, F=F, J=J, jcc=jcc,
                        off=seq[s][1][0], fall_jcc=bool(re.match(r'j(?!mp)', prev))))
    return out


def nearest_line(rows, ri, lmap):
    for d in range(0, 40):
        for i in (ri - d, ri + d):
            if 0 <= i < len(rows) and rows[i][2] is not None and rows[i][2][0] in lmap:
                return lmap[rows[i][2][0]]
    return '?'


def scan_fn(res, lmap):
    rows = res['rows']
    I, O = tails(side(rows, 1)), tails(side(rows, 2))
    # pair tails whose ret rows are aligned (same row) or within the same differing stretch
    paired_i, paired_o = set(), set()
    oby = {t['ret']: k for k, t in enumerate(O)}
    for k, t in enumerate(I):
        if t['ret'] in oby:
            paired_i.add(k)
            paired_o.add(oby[t['ret']])
    # second pass: an unpaired image tail and an unpaired ours tail with the same value within 6 rows
    for k, t in enumerate(I):
        if k in paired_i:
            continue
        for j, u in enumerate(O):
            if j not in paired_o and abs(u['ret'] - t['ret']) <= 6 and u['value'] == t['value']:
                paired_i.add(k)
                paired_o.add(j)
                break
    flags = []
    for k, t in enumerate(I):
        if k not in paired_i:
            flags.append(dict(kind='IMG-COPY', img=res['lo'] + t['off'], ours=None, value=t['value'],
                              entry=('F' if t['F'] else '') + ('J%d' % len(t['J']) if t['J'] else ''),
                              line=nearest_line(rows, t['start'], lmap)))
    for j, u in enumerate(O):
        if j not in paired_o:
            # the image jump in the same stretch: the nearest image jcc/jmp row
            img = None
            for d in range(0, 12):
                for i in (u['start'] - d, u['start'] + d):
                    if 0 <= i < len(rows) and rows[i][1] is not None and rows[i][1][1].startswith('j'):
                        img = res['lo'] + rows[i][1][0]
                        break
                if img:
                    break
            flags.append(dict(kind='OURS-COPY', img=img, ours=u['off'], value=u['value'],
                              entry=('F' if u['F'] else '') + ('J%d' % len(u['J']) if u['J'] else ''),
                              line=lmap.get(u['off'], nearest_line(rows, u['start'], lmap))))
    return I, O, flags


CTX = {}  # (lane prefix, context unit) -> (Coff, listing), one compile per lane


def scan_unit(unit, pfx, mode, only=None, show=False):
    text = open(fe.R + '/' + unit, encoding='latin-1').read()
    c, cod = tailscan.compile_listing(unit, fe.R + '/' + unit, pfx)
    if c is None:
        return [('ERR', unit, 'compile failed')]
    out = []
    for m in MARK.finditer(text):
        addr = int(m.group(1), 16)
        if only and addr not in only:
            continue
        # A marker with `// compile_context <tu>` is measured in that unit's compile (as fe.py and
        # `recover measure` do); the listing's line numbers then name the file that holds the body.
        blk = re.match(r'[^\n]*\n((?://(?! ORIGINAL)[^\n]*\n)*)', text[m.end():])
        mc = blk and re.search(r'// compile_context (src/\S+\.cpp)', blk.group(1))
        cc_, cod_ = c, cod
        if mc and mc.group(1) != unit:
            ctx = mc.group(1)
            if (pfx, ctx) not in CTX:
                CTX[(pfx, ctx)] = (tailscan.compile_listing(ctx, fe.R + '/' + ctx, pfx)
                                   if os.path.exists(fe.R + '/' + ctx) else (None, None))
            cc_, cod_ = CTX[(pfx, ctx)]
            if cc_ is None:
                out.append(('ERR', unit, 'context %s of %08X failed' % (ctx, addr)))
                continue
        r = fe.evaluate(cc_, text, addr)
        if 'error' in r or (mode == 'nonexact' and r['exact']):
            continue
        lmap = tailscan.lines_of(cod_, r['sym'], os.path.basename(unit).lower()) if cod_ else {}
        I, O, fl = scan_fn(r, lmap)
        if fl or show:
            d = dict(unit=unit, addr='%08X' % addr, fn=r['sym'], exact=r['exact'], diff=r['diff'],
                     rets=(len(I), len(O)), flags=fl)
            if show:
                d['img_tails'] = ['%08X %s %s%s' % (r['lo'] + t['off'], t['value'], 'F' if t['F'] else '',
                                                   'J%d(%dc)' % (len(t['J']), t['jcc']) if t['J'] else '') for t in I]
                d['our_tails'] = ['+%04X %s %s%s %s' % (t['off'], t['value'], 'F' if t['F'] else '',
                                                       'J%d(%dc)' % (len(t['J']), t['jcc']) if t['J'] else '',
                                                       lmap.get(t['off'], '')) for t in O]
            out.append(d)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='*')
    ap.add_argument('--all', action='store_true')
    ap.add_argument('--json')
    ap.add_argument('--show', action='store_true')
    ap.add_argument('--addr', default='')
    ap.add_argument('--jobs', type=int, default=int(os.environ.get('FE_JOBS', '5')))
    ap.add_argument('--pfx-base', type=int, default=int(os.environ.get('FE_PFX_BASE', '21')))
    a = ap.parse_args()
    mode = 'all' if a.all else 'nonexact'
    only = set(int(x, 16) for x in a.addr.split(',') if x) or None
    cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
    units = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/')]
    units = [u for u in units if '/vendor/' not in u and u.endswith('.cpp')]
    if a.units:
        units = [u for u in units if u in a.units or os.path.basename(u) in a.units]
    fe.compile_cmd(units[0])

    def lane(j):
        r = []
        for i in range(j, len(units), a.jobs):
            r += scan_unit(units[i], a.pfx_base + j, mode, only, a.show)
        return r
    res = []
    with ThreadPoolExecutor(a.jobs) as ex:
        for o in ex.map(lane, range(a.jobs)):
            res += o
    for e in res:
        if isinstance(e, tuple):
            print('ERROR', e[1], e[2])
    res = [x for x in res if isinstance(x, dict)]
    if a.json:
        json.dump(res, open(a.json, 'w'), indent=0)
    tot = collections.Counter()
    for x in sorted(res, key=lambda x: (x['diff'], x['unit'], x['addr'])):
        print('%s %s %-40s %s diff=%d ret=%d/%d' % ('EXACT' if x['exact'] else 'NONEX', x['addr'], x['fn'][:40],
                                                   x['unit'], x['diff'], x['rets'][0], x['rets'][1]))
        for f in x['flags']:
            tot[f['kind']] += 1
            print('   %-9s img %-8s ours %-6s %-24s %-6s %s' % (
                f['kind'], '%08X' % f['img'] if f['img'] else '-', '+%04X' % f['ours'] if f['ours'] is not None
                else '-', f['value'][:24], f['entry'], f['line']))
        for k in ('img_tails', 'our_tails'):
            for t in x.get(k, []):
                print('      %s %s' % (k[:3], t))
    print('functions=%d %s' % (len(res), dict(tot)))


if __name__ == '__main__':
    main()
