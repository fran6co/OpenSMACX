#!/usr/bin/env python3
"""accdiff.py [unit ...] [--all] [--json OUT] [--min N]
Access-form differences in the non-exact functions: inside each differing block of fe's alignment,
pair an image instruction with ours when both name the same memory operand (same address expression
with registers abstracted; a relocated global by its symbol) but differ in how they access it:

  ext     movsx against movzx / plain mov (signedness of a byte/word field, or a cast)
  width   byte/word/dword operand width differs (field width, or a narrowed mask test)
  jcc     the same dword compare followed by a signed against an unsigned condition

Rows are keyed by the operand (`?Sym@@..+off` for globals, `[R + disp]` for members) and ranked by
the number of non-exact functions they occur in: a wrongly declared field shows up as one key in
many functions, a local cast as one function. Screen only."""
import sys, os, re, json, argparse, collections
from concurrent.futures import ThreadPoolExecutor
T = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, T)
sys.path.insert(0, T + '/analysis')
import fe  # noqa: E402
import typescan as ts  # noqa: E402

MARK = re.compile(r'// ORIGINAL: 0x([0-9A-F]{8}) (\S+) 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)')
SJ = ts.SIGNED_J
UJ = ts.UNSIGNED_J - {'adc', 'sbb'}


def cls(stream, k):
    off, text, size, _ = stream[k]
    mn = text.split(' ')[0]
    m = ts.MEM.search(text)
    w = ts.WIDTH[m.group(1)]
    sg, _ = ts.sig(stream, k)
    ext = 'sx' if mn == 'movsx' else 'zx' if (mn == 'movzx' or (mn == 'mov' and w < 4 and sg == 'U')) else '-'
    j = None
    if mn == 'cmp':
        j = ts.jcc_after(stream, k)
    return mn, w, ext, j


def scan_unit(unit, pfx, allm):
    text = open(fe.R + '/' + unit, encoding='latin-1').read()
    c, err = fe.compile_unit(unit, fe.R + '/' + unit, pfx)
    if c is None:
        return [('ERR', unit, err)]
    out = []
    for m in MARK.finditer(text):
        if 'BYTE_EXACT' in m.group(3) and not allm:
            continue
        addr = int(m.group(1), 16)
        r = fe.evaluate(c, text, addr)
        if 'error' in r or r['exact']:
            continue
        try:
            s = c.function(r['sym'])
            sec = c.sections[s.section - 1]
            rel = sorted((va, c.symbols[i].name if c.symbols.get(i) else '?',
                          int.from_bytes(sec.data[va:va + 4], 'little', signed=True)) for va, i, k in sec.relocations)
        except Exception:
            rel = []
        rows = r['rows']
        O = [x[1] for x in rows if x[1] is not None]
        C = [x[2] for x in rows if x[2] is not None]
        oi = {x[0]: i for i, x in enumerate(O)}
        ci = {x[0]: i for i, x in enumerate(C)}

        def name(cc):
            for va, sym, add in rel:
                if cc[0] <= va < cc[0] + cc[2]:
                    return '%s%+d' % (sym, add) if add else sym
            return None
        k = 0
        while k < len(rows):
            if rows[k][0] != '!!':
                k += 1
                continue
            k2 = k
            while k2 < len(rows) and rows[k2][0] == '!!':
                k2 += 1
            bo = [x[1] for x in rows[k:k2] if x[1] is not None and ts.MEM.search(x[1][1])]
            bc = [x[2] for x in rows[k:k2] if x[2] is not None and ts.MEM.search(x[2][1])]
            # only blocks where the multiset of address expressions matches: pair in order per expression
            eo = collections.defaultdict(list)
            for o in bo:
                eo[ts.shape(o[1])[1]].append(o)
            ec = collections.defaultdict(list)
            for cc in bc:
                ec[ts.shape(cc[1])[1]].append(cc)
            for sh, cs in ec.items():
                if 'esp' in sh or 'ebp' in sh or sh == 'R':
                    continue
                os_ = eo.get(sh, [])
                if len(os_) != len(cs):
                    continue
                for o, cc in zip(os_, cs):
                    a_, b_ = cls(O, oi[o[0]]), cls(C, ci[cc[0]])
                    kind = None
                    if a_[1] != b_[1]:
                        kind = 'width'
                    elif a_[2] != b_[2] and (a_[2] != '-' or b_[2] != '-') and a_[1] < 4:
                        kind = 'ext'
                    elif a_[3] and b_[3] and a_[3] != b_[3]:
                        kind = 'jcc'
                    if not kind:
                        continue
                    key = name(cc) or '[%s]' % sh
                    out.append(dict(key=key, kind=kind, unit=unit, addr='%08X' % addr, fn=r['sym'],
                                    at='%08X' % (r['lo'] + o[0]), img=o[1], ours=cc[1]))
            k = k2
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='*')
    ap.add_argument('--all', action='store_true')
    ap.add_argument('--json')
    ap.add_argument('--min', type=int, default=1)
    ap.add_argument('--jobs', type=int, default=int(os.environ.get('FE_JOBS', '5')))
    ap.add_argument('--pfx-base', type=int, default=int(os.environ.get('FE_PFX_BASE', '21')))
    a = ap.parse_args()
    cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
    units = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/')]
    units = [u for u in units if '/vendor/' not in u and u.endswith('.cpp')]
    if a.units:
        units = [u for u in units if u in a.units or os.path.basename(u) in a.units]
    fe.compile_cmd(units[0])

    def lane(k):
        r = []
        for i in range(k, len(units), a.jobs):
            r += scan_unit(units[i], a.pfx_base + k, a.all)
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
    by = collections.defaultdict(list)
    for x in res:
        by[(x['key'], x['kind'])].append(x)
    rows = sorted(by.items(), key=lambda kv: (-len({x['addr'] for x in kv[1]}), -len(kv[1])))
    for (key, kind), xs in rows:
        fns = {x['addr'] for x in xs}
        if len(fns) < a.min:
            continue
        print('%-5s %-48s functions=%d sites=%d' % (kind, key[:48], len(fns), len(xs)))
        for x in xs[:5]:
            print('      %s %-30s img %-36s ours %s' % (x['at'], x['fn'][:30], x['img'][:36], x['ours'][:36]))
    print('differing accesses: %d in %d functions, %d keys' % (len(res), len({x['addr'] for x in res}), len(by)))


if __name__ == '__main__':
    main()
