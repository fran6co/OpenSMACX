#!/usr/bin/env python3
"""ceilscan.py [unit ...] [--rungs rungs.json] [--all]: which functions hit VC6's per-function /Ob2 size ceiling
(MATCHING_GUIDE 2.8 "Total-size ceiling").  One compile per unit with a FREE inline probe
(`ceil_probe_(K)`: one global store) inserted at the top and at the end (before the last top-level statement when
it is a `return`) of every non-exact function.  A free inline is refused only after the running total (the
caller's own size plus every accepted expansion) has crossed the ceiling, so `top E, end c` means the function is
ceiling-bound: from some source point on every first-level site is refused, whatever its cost.  Then find the
point with `--bisect`: probes after every top-level `return`/statement run, printing the last expanded one.
Screen only.  Units default to those with a non-exact function in rungs.json (`--all`: every function of the
listed units)."""
import sys, os, re, json, argparse, collections
from concurrent.futures import ThreadPoolExecutor
H = os.path.dirname(os.path.abspath(__file__))
T = os.path.dirname(H)
sys.path.insert(0, T)
sys.path.insert(0, H)
os.chdir(os.path.dirname(os.path.dirname(T)))
import fe, callees, spellsweep  # noqa: E402
R = fe.R
NPROBE = int(os.environ.get('CEIL_PROBES', '12'))
DEF = '\nvoid __cdecl ceil_sink_(int k);\nstatic inline void ceil_probe_(int k) { ceil_sink_(k); }\n'


def stmt_points(code, s):
    """offsets of lines that start a statement at any depth: the previous code line ends with ; { or }"""
    out = []
    prev = ''
    pos = 0
    for ln in code.split('\n'):
        st = ln.strip()
        if st and prev and prev[-1] in ';{}' and not re.match(r'(\}|else\b|case\b|default\b|\w+:(?!:)|#|\{)', st) \
                and not prev.startswith(('switch', 'do')) and re.match(r'\s+', ln):
            out.append(s + pos)
        if st:
            prev = st
        pos += len(ln) + 1
    return out[1:]


def insert_probes(t, addrs, every=False):
    """-> text, {addr: [(K, offset)]}; K = addr low 20 bits * 64 + probe index"""
    ins = []
    where = {}
    for a in addrs:
        try:
            s, e = spellsweep.span(t, a)
        except Exception:
            continue
        body = t[s:e]
        # first line of the function body: after the signature's `{` that follows the last marker comment line
        m = None
        for m_ in re.finditer(r'^[^/\n][^\n]*\{\n', body, re.M):
            m = m_
            break
        if not m:
            continue
        top = s + m.end()
        code = re.sub(r'//[^\n]*', lambda x: ' ' * len(x.group(0)), body)
        # last top-level statement: the last line indented by 4 spaces
        last = [x for x in re.finditer(r'\n    (?! )(\S[^\n]*)', code)]
        end = e + 1  # before the closing brace
        if last and last[-1].group(1).startswith('return'):
            end = s + last[-1].start() + 1
        elif not last or last[-1].group(1).startswith('}'):
            sp = stmt_points(code, s)  # the closing brace may be unreachable (every arm returns)
            end = sp[-1] if sp else end
        pts = [top, end]
        if every:
            pts = [top] + stmt_points(code, s) + [end]
            pts = sorted(set(pts))
            if len(pts) > NPROBE:  # each expanded probe adds size too: a few, evenly spread, plus the end
                pts = [pts[i * (len(pts) - 1) // (NPROBE - 1)] for i in range(NPROBE)]
        k0 = (a & 0xFFFFF) * 64
        where[a] = []
        for i, p in enumerate(pts[:63]):
            ins.append((p, '    ceil_probe_(%d);\n' % (k0 + i)))
            where[a].append((k0 + i, p, t.count('\n', 0, p) + 1))
    for p, txt in sorted(ins, reverse=True):
        t = t[:p] + txt + t[p:]
    # the probe definition first: it needs no header, and some units include headers late
    return DEF.lstrip('\n') + t, where


def decisions(c, sym, ks):
    """K -> 'E' (push K; call ceil_sink_: expanded) / 'c' (push K; call ceil_probe_) / '?' (not found)"""
    s = c.function(sym)
    sec = c.sections[s.section - 1]
    rm = {r[0]: r for r in sec.relocations}
    out = {}
    pend = None
    for x in fe.md.disasm(bytes(sec.data), 0):
        if x.mnemonic == 'push' and re.fullmatch(r'0x[0-9a-f]+|\d+', x.op_str) and int(x.op_str, 0) in ks:
            pend = int(x.op_str, 0)
        elif x.mnemonic == 'call' and pend is not None:
            r = rm.get(x.address + 1)
            nm = c.symbols.get(r[1]).name if r and c.symbols.get(r[1]) else ''
            if 'ceil_sink_' in nm:
                out.setdefault(pend, 'E')
            elif 'ceil_probe_' in nm:
                out[pend] = 'c'
            pend = None
    return out


def unit_scan(unit, addrs, pfx, every):
    t = open(R + '/' + unit, encoding='latin-1').read()
    t2, where = insert_probes(t, addrs, every)
    path = '%s/build/recovery/fe/ceil%s.cpp' % (R, pfx)
    open(path, 'w', encoding='latin-1').write(t2)
    c, err = fe.compile_unit(unit, path, pfx)
    if c is None:
        return [dict(unit=unit, error=(err or '')[:300])]
    out = []
    for a, ps in where.items():
        idn = fe.ident(t2, a)
        if not idn:
            continue
        try:
            d = decisions(c, idn[0], {k for k, p, l in ps})
        except Exception as ex:
            out.append(dict(unit=unit, addr='%08X' % a, error=repr(ex)[:80]))
            continue
        seq = ''.join(d.get(k, '?') for k, p, l in ps)
        lines = [l for k, p, l in ps]  # lines of the unprobed source
        out.append(dict(unit=unit, addr='%08X' % a, sym=idn[0], seq=seq, lines=lines))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='*')
    ap.add_argument('--rungs', default=R + '/build/recovery/rungs/rungs.json')
    ap.add_argument('--all', action='store_true')
    ap.add_argument('--addr')
    ap.add_argument('--bisect', action='store_true', help='a probe after every top-level statement')
    ap.add_argument('--out')
    a = ap.parse_args()
    base = int(os.environ.get('FE_PFX_BASE', '2'))
    jobs = int(os.environ.get('FE_JOBS', '5'))
    rs = json.load(open(a.rungs))
    want = collections.defaultdict(list)
    for r in rs:
        if 'addr' not in r or (a.units and r['unit'] not in a.units):
            continue
        if a.all or not r.get('exact'):
            want[r['unit']].append(int(r['addr'], 16))
    # rungs.json is a cache that may cover only some units (a scoped rungs.py run): read the markers of any
    # named unit, or of the unit that holds a requested --addr, from the source instead
    def norm(u):
        u = u if u.endswith('.cpp') else u + '.cpp'
        if os.path.exists(R + '/' + u):
            return u
        for d, _, fs in os.walk(R + '/src'):
            if os.path.basename(u) in fs:
                return os.path.relpath(os.path.join(d, os.path.basename(u)), R)
        return u

    def markers(u):
        t = open(R + '/' + u, encoding='latin-1').read()
        return [(int(m.group(1), 16), bool(m.group(2))) for m in
                re.finditer(r'// ORIGINAL: 0x([0-9A-Fa-f]{8}) \S+ \S+( BYTE_EXACT)?', t)]
    for u in [norm(x) for x in a.units]:
        if u not in want and os.path.exists(R + '/' + u):
            want[u] = [x for x, ex in markers(u) if a.all or not ex]
    if a.addr:
        aa = {int(x, 16) for x in a.addr.split(',')}
        miss = aa - {x for v in want.values() for x in v}
        if miss:
            for d, _, fs in os.walk(R + '/src'):
                for f in fs:
                    if f.endswith('.cpp'):
                        u = os.path.relpath(os.path.join(d, f), R)
                        for x, ex in markers(u):
                            if x in miss and x not in want[u]:
                                want[u].append(x)
        want = {u: [x for x in v if x in aa] for u, v in want.items() if any(x in aa for x in v)}
    units = sorted(want)
    res = []
    for u in units:
        fe.compile_cmd(u)  # lazy cache: fill before the lanes

    def lane(k):
        o = []
        for i in range(k, len(units), jobs):
            o += unit_scan(units[i], want[units[i]], str(base + k), a.bisect)
        return o
    with ThreadPoolExecutor(jobs) as ex:
        for o in ex.map(lane, range(jobs)):
            res += o
    bound = [r for r in res if r.get('seq') and 'c' in r['seq']]
    for r in sorted(res, key=lambda r: (r.get('seq', '') .count('c') == 0, r.get('unit'), r.get('addr'))):
        if 'error' in r:
            print('ERR %s %s %s' % (r['unit'], r.get('addr', ''), r['error'][:100]))
        elif 'c' in r['seq'] or a.bisect:
            last = max((i for i, ch in enumerate(r['seq']) if ch == 'E'), default=-1)
            print('%s %s %-45s %s  last E at L%s' % (r['unit'], r['addr'], r['sym'][:45], r['seq'], r['lines'][last] if last >= 0 else '-'))
    print('%d functions probed, %d ceiling-bound (a free end probe called)' % (sum(1 for r in res if r.get('seq')), len(bound)))
    if a.out:
        json.dump(res, open(a.out, 'w'), indent=0)


if __name__ == '__main__':
    main()
