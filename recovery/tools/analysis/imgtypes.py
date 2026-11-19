#!/usr/bin/env python3
"""imgtypes.py [--layouts CACHE.json] [--refresh] [--field SUBSTR] [--min N] [--uses] [--json OUT] [--feall FEALL.json]
Image-side type census (MATCHING_GUIDE 3.4). Unlike typescan.py this does not pair our instructions
with the image's: it reads the IMAGE alone, so it covers non-exact functions wherever they access a
bound global.

Every function span (ORIGINAL markers) is disassembled; each memory operand whose displacement lies
inside a source-bound global (RECOVERY_OBJECT bindings via the package's reference targets) is named
through the global's decorated type and the CodeView class layouts of our own compile (struct arrays:
offset modulo the element size). Per field it collects the multiset of access forms

  movsx/movzx b|w        extension loads (the field's signedness for byte/word fields)
  ld b|w|d               plain load (xor r,r before or `and r,0xff` after -> counted as zx)
  st b|w|d               store
  cmp/test/and/or.. b|w|d  memory-operand arithmetic (narrow forms of a mask test are normal)
  S / U                  for dword loads and compares: the sign-revealing use (typescan.sig)

and prints the fields whose image forms contradict the declared type:

  OVER    accessed with an operand wider than the declared field (a short read as a dword)
  NARROW  a declared dword/word field loaded with movsx/movzx/narrow mov from a narrower operand
  SIGN    byte/word: declared signed but the image zero-extends (or the reverse);
          dword: signed uses of a field declared unsigned (or the reverse, weak)
  INNER   accessed inside a declared scalar (not at its start)
  NOLEAF  accessed at an offset where the layout has no scalar (padding, bitfield, unknown)

sorted by the number of non-exact functions with a contradicting access. The class layouts come from
one /Z7 compile of every unit (about 2 min, cached in --layouts). Screen only."""
import sys, os, re, json, argparse, collections, bisect, pathlib
from concurrent.futures import ThreadPoolExecutor
T = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, T)
sys.path.insert(0, T + '/analysis')
import fe  # noqa: E402
import typescan as ts  # noqa: E402
import capstone  # noqa: E402
from capstone import x86 as X  # noqa: E402

MARK = re.compile(r'// ORIGINAL: 0x([0-9A-F]{8}) (\S+) 0x([0-9A-Fa-f]+)-0x([0-9A-Fa-f]+)([^\n]*)')
WN = {1: 'b', 2: 'w', 4: 'd', 8: 'q'}


def units():
    cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
    us = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/')]
    return [u for u in us if '/vendor/' not in u and u.endswith('.cpp')]


def layouts(cache, refresh, jobs, base):
    if cache and os.path.exists(cache) and not refresh:
        return json.load(open(cache))
    us = [u for u in units() if '/uv/' not in u]
    fe.compile_cmd(us[0])

    def lane(k):
        out = {}
        for i in range(k, len(us), jobs):
            c, _, err = ts.compile_cod(us[i], fe.R + '/' + us[i], base + k)
            if c is None:
                print('ERR', us[i], (err or '')[:120], file=sys.stderr)
                continue
            ty = ts.Types(c)
            for cn, (size, fl, _) in ty.classes.items():
                if cn in out or not size:
                    continue
                out[cn] = dict(size=size, leaves=[[o, [list(x) for x in d], ow, f, w, sg, tn]
                                                  for o, d, ow, f, w, sg, tn in ty.flat(cn)])
        return out
    lay = {}
    with ThreadPoolExecutor(jobs) as ex:
        for o in ex.map(lane, range(jobs)):
            for k, v in o.items():
                if k not in lay or len(v['leaves']) > len(lay[k]['leaves']):
                    lay[k] = v
    if cache:
        json.dump(lay, open(cache, 'w'))
    return lay


def leaf_at(lay, cname, disp):
    """-> (kind, leaf): kind 'at' (starts here), 'inner' (inside a scalar), None"""
    best = None
    for o, d, ow, f, w, sg, tn in lay[cname]['leaves']:
        d2 = tuple(tuple(x) for x in d)
        if ts.covers(o, d2, disp):
            return 'at', (ow, f, w, sg, tn)
        for k in range(1, w):
            if ts.covers(o, d2, disp - k):
                best = ('inner', (ow, f, w, sg, tn))
    return best or (None, None)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--layouts', default=fe.SCR + '/imgtypes_layouts.json')
    ap.add_argument('--refresh', action='store_true')
    ap.add_argument('--field', default='')
    ap.add_argument('--min', type=int, default=1)
    ap.add_argument('--uses', action='store_true')
    ap.add_argument('--json')
    ap.add_argument('--feall', help='feall --all report: exactness per marker (default: BYTE_EXACT token)')
    ap.add_argument('--jobs', type=int, default=int(os.environ.get('FE_JOBS', '5')))
    ap.add_argument('--pfx-base', type=int, default=int(os.environ.get('FE_PFX_BASE', '21')))
    a = ap.parse_args()
    lay = layouts(a.layouts, a.refresh, a.jobs, a.pfx_base)

    from match_recovery.source_selection import measurement_reference_targets
    from match_recovery.measurement import Measurement
    root = pathlib.Path(fe.R).resolve()
    targets = measurement_reference_targets(root, Measurement.read(root))
    objs = sorted((ad, n) for n, ad in targets.external.items() if n.startswith('?') and '@@3' in n)
    oaddr = [x[0] for x in objs]
    alladdr = sorted(set(targets.external.values()))
    # object extents: our own link map (the size up to our next symbol), capped by the next bound address
    oursize = {}
    mp = fe.MAIN + '/build/OpenSMACX.map'
    if os.path.exists(mp):
        rows_ = []
        for l in open(mp, encoding='latin-1'):
            mm = re.match(r'\s*[0-9a-f]{4}:[0-9a-f]{8}\s+(\S+)\s+([0-9a-f]{8})\s', l)
            if mm:
                rows_.append((int(mm.group(2), 16), mm.group(1)))
        rows_.sort()
        for (a0, n0), (a1, _) in zip(rows_, rows_[1:]):
            if a1 > a0:
                oursize.setdefault(n0, a1 - a0)

    def obj_at(addr):
        i = bisect.bisect_right(oaddr, addr) - 1
        if i < 0:
            return None
        ad, name = objs[i]
        j = bisect.bisect_right(alladdr, ad)
        nxt = alladdr[j] if j < len(alladdr) else ad + 0x100000
        if name in oursize:
            nxt = min(nxt, ad + oursize[name])
        else:
            nxt = min(nxt, ad + 0x40)
        gk = ts.global_kind(name)
        if not gk:
            return None
        gname = re.match(r'\?(\w+)@@', name).group(1)
        off = addr - ad
        if (gk[0] == 'struct' and gk[2] or gk[0] == 'scalar' and gk[4]) and oursize.get(name, 0) == 4 \
                and not re.search(r'@@3(PA|QA)Y', name):
            # `T *p` and `T a[]` decorate alike: a 4-byte object is the pointer
            return (gname, (4, 'U', 'pointer'), 'at' if off == 0 else 'inner', 0) if off < 4 else None
        if gk[0] == 'struct':
            if gk[1] not in lay:
                return None
            size = lay[gk[1]]['size']
            if gk[2]:
                if addr >= nxt:
                    return None
                off %= size
            elif off >= size:
                return None
            kind, leaf = leaf_at(lay, gk[1], off)
            if kind is None:
                return ('%s::+0x%x' % (gk[1], off), None, 'noleaf', 0)
            ow, f, w, sg, tn = leaf
            return ('%s::%s' % (ow, f), (w, sg, tn), kind, 0)
        w, sg, tn, arr = gk[1], gk[2], gk[3], gk[4]
        if arr:
            if addr >= nxt:
                return None
            return (gname + '[]', (w, sg, tn + '[]'), 'at' if off % w == 0 else 'inner', 0)
        if off >= w:
            return None
        return (gname, (w, sg, tn), 'at' if off == 0 else 'inner', 0)

    exactmap = {}
    if a.feall:
        for x in json.load(open(a.feall)):
            exactmap[x['addr']] = x['exact']
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.detail = True
    uses = []
    seen = set()
    for u in units():
        try:
            text = open(fe.R + '/' + u, encoding='latin-1').read()
        except OSError:
            continue
        for m in MARK.finditer(text):
            addr = int(m.group(1), 16)
            lo, hi = int(m.group(3), 16), int(m.group(4), 16)
            if addr in seen or hi <= lo or hi - lo > 0x20000:
                continue
            seen.add(addr)
            exact = exactmap.get('%08X' % addr, 'BYTE_EXACT' in m.group(5))
            try:
                O = fe.imgdis(lo, hi)
            except Exception:
                continue
            for k, (off, txt, size, _) in enumerate(O):
                if '<r>' not in txt or 'ptr' not in txt:
                    continue
                ins = next(md.disasm(fe.imgbytes(lo + off, lo + off + size), lo + off), None)
                if ins is None:
                    continue
                for oi, op in enumerate(ins.operands):
                    if op.type != X.X86_OP_MEM or op.mem.segment:
                        continue
                    d = op.mem.disp & 0xFFFFFFFF
                    if not (0x400000 <= d < 0xA00000):
                        continue
                    o = obj_at(d)
                    if not o:
                        continue
                    key, decl, kind, _ = o
                    if a.field and a.field not in key:
                        continue
                    mn = ins.mnemonic
                    w = op.size
                    sg, _w = ts.sig(O, k)
                    if mn in ('movsx', 'movzx'):
                        form = '%s %s' % (mn, WN.get(w, '?'))
                    elif mn == 'mov':
                        form = ('st ' if oi == 0 else 'ld ') + WN.get(w, '?')
                        if oi == 1 and w < 4 and sg == 'U':
                            form = 'ldzx ' + WN.get(w, '?')
                    elif mn.startswith('f'):
                        form = '%s %s' % (mn, WN.get(w, '?'))
                    else:
                        form = '%s %s' % (mn, WN.get(w, '?'))
                    if w == 4 and sg:
                        form += ' ' + sg
                    uses.append(dict(key=key, decl=decl, kind=kind, fn=m.group(2), addr='%08X' % addr, unit=u,
                                     exact=exact, at='%08X' % (lo + off), form=form, w=w, sg=sg, mn=mn, load=oi == 1,
                                     txt=ins.mnemonic + ' ' + ins.op_str))
    if a.json:
        json.dump(uses, open(a.json, 'w'), indent=0)
    by = collections.defaultdict(list)
    for x in uses:
        by[x['key']].append(x)
    rows = []
    for key, us in by.items():
        decl = us[0]['decl']
        flags = collections.defaultdict(list)
        for x in us:
            if x['kind'] == 'noleaf':
                flags['NOLEAF'].append(x)
                continue
            maskop = x['mn'] in ('test', 'and', 'or', 'xor')
            if x['kind'] == 'inner':
                if not maskop:
                    flags['INNER'].append(x)
                continue
            dw, ds, tn = decl
            if ds == 'F' or x['mn'].startswith('f'):
                if not x['mn'].startswith('f') and ds == 'F' and x['w'] != dw:
                    flags['OVER' if x['w'] > dw else 'NARROW'].append(x)
                continue
            if tn.startswith('pointer'):
                if x['w'] != 4:
                    flags['NARROW'].append(x)
                continue
            if x['w'] > dw:
                if not (dw == 1 and tn.endswith('[]')):   # string/byte-array copies by dword
                    flags['OVER'].append(x)
            elif x['w'] < dw and (x['mn'] in ('movsx', 'movzx') or x['form'].startswith(('ldzx', 'st ')) or x['mn'] in ('cmp', 'add', 'sub', 'inc', 'dec')):
                flags['NARROW'].append(x)
            elif x['w'] < dw and x['mn'] == 'mov':
                flags['narrowld'].append(x)
            elif x['w'] == dw:
                s = None
                if x['mn'] == 'movsx':
                    s = 'S'
                elif x['mn'] == 'movzx' or x['form'].startswith('ldzx'):
                    s = 'U'
                elif dw == 4:
                    s = x['sg']
                if s and s != ds:
                    flags['SIGN'].append(x)
                elif s:
                    flags['agree'].append(x)
        for fl, xs in flags.items():
            if fl in ('agree', 'narrowld'):
                continue
            if fl == 'NOLEAF' and key.endswith('::+0x0'):
                continue   # vptr
            if fl == 'SIGN' and len(xs) <= len(flags.get('agree', ())) and not a.field:
                # dword: unsigned uses can come from the other operand; byte/word: mixed -> casts
                if decl[0] == 4 and decl[1] == 'S':
                    continue
            nonex = {x['addr'] for x in xs if not x['exact']}
            ex = {x['addr'] for x in xs if x['exact']}
            if len(xs) >= a.min:
                rows.append((len(nonex), len(ex), fl, key, decl, xs, us, len(flags.get('agree', ()))))
    rows.sort(key=lambda r: (-r[0], -r[1], r[3]))
    for nn, ne, fl, key, decl, xs, us, agree in rows:
        forms = collections.Counter(x['form'] for x in us)
        allnon = len({x['addr'] for x in us if not x['exact']})
        print('%-6s %-46s decl=%-18s fns nonexact=%d exact=%d (of %d nonexact users) contra=%d agree=%d' % (
            fl, key, decl[2] if decl else '-', nn, ne, allnon, len(xs), agree))
        print('       forms: ' + ', '.join('%s x%d' % kv for kv in forms.most_common(12)))
        for x in xs[:4]:
            print('       %s %s %-28s %s' % (x['at'], 'X' if x['exact'] else '-', x['fn'][:28], x['txt']))
        if a.uses:
            for x in us:
                print('       use %s %s %-28s %-10s %s' % (x['at'], 'X' if x['exact'] else '-', x['fn'][:28], x['form'], x['txt']))
    print('globals bound: %d, fields seen: %d, accesses: %d, rows: %d' % (len(objs), len(by), len(uses), len(rows)))


if __name__ == '__main__':
    main()
