#!/usr/bin/env python3
"""capscan.py [--max-callers N] [--min N] [--json F] [unit ...]

Tree-wide per-callee auto-inline cap screen (MATCHING_GUIDE 2.8, "The per-callee cap counts IL nodes").
For every marker the image never calls (orphan) or calls at most N times (rel32), appends one
one-statement probe `void __capprobe_K() { CALL; __capprobe_g = K; }` per marker to a copy of its unit
(one compile per unit, FE_PFX_BASE.. lanes) and reports the markers that our source still CALLS from such
a trivial caller: they are over the auto-inline cap, while the image expanded them somewhere (an orphan
always). Arguments are `*(T *)__capprobe_p` per parameter type (the cap excludes argument expressions),
member calls are qualified (`((C *)p)->C::f(...)`) so virtuals are probed too. Constructors, destructors,
operators, function-pointer and variadic parameters are skipped. The same compile reports whether the
callee is fe-exact (only those are respelling candidates: the out-of-line copy must stay exact).
Explicit `inline` members are not capped (2.8 rule 2) and read EXPANDED. Units whose last /Ob flag is not /Ob2
(pulldown and the other /Ob1 library units, uv/) never auto-inline and are skipped.  Screen only."""
import sys, os, re, json, argparse, collections, threading
from concurrent.futures import ThreadPoolExecutor
H = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, H)
sys.path.insert(0, os.path.dirname(H))
import fe  # noqa: E402

TYPEKW = {'int', 'unsigned', 'signed', 'char', 'short', 'long', 'bool', 'float', 'double', 'void', 'const',
          'volatile', 'struct', 'class', 'enum', '__int64', 'BOOL', 'DWORD', 'UINT', 'WORD', 'BYTE', 'LPARAM',
          'WPARAM', 'HWND', 'LPSTR', 'LPCSTR', 'HDC', 'LRESULT'}


def split_params(s):
    out, d, cur = [], 0, ''
    for ch in s:
        if ch in '(<[':
            d += 1
        elif ch in ')>]':
            d -= 1
        if ch == ',' and d == 0:
            out.append(cur)
            cur = ''
        else:
            cur += ch
    if cur.strip():
        out.append(cur)
    return [p.strip() for p in out]


def ptype(p):
    p = re.sub(r'/\*.*?\*/', '', p).split('=')[0].strip()
    if not p or p == 'void':
        return None
    if '(' in p or '...' in p:
        raise ValueError(p)
    arr = re.search(r'\[[^\]]*\]\s*$', p)
    if arr:
        p = p[:arr.start()].strip() + ' *'
    m = re.match(r'^(.*?[\s\*&])([A-Za-z_]\w*)$', p)
    if m and m.group(1).strip() and m.group(2) not in TYPEKW and m.group(1).strip() not in ('struct', 'class', 'enum',
                                                                                          'unsigned', 'signed'):
        p = m.group(1).strip()
    return p


DEF = re.compile(r'(?m)^(?!\s*//)([^\n;{}()#]*?)\b((?:[A-Za-z_]\w*::)*)(~?[A-Za-z_]\w*)\s*\(([^{};]*?)\)\s*(const)?\s*\{')


def probe_expr(text, mpos, k):
    """call expression for the definition following the marker at mpos, or None"""
    m = DEF.search(text, mpos, mpos + 20000)
    if not m:
        return None
    pre, qual, name = m.group(1), m.group(2).rstrip(':'), m.group(3)
    if name in ('if', 'while', 'for', 'switch', 'return') or name.startswith('~') or 'operator' in pre + name:
        return None
    if qual and qual.split('::')[-1] == name:
        return None
    try:
        tys = [ptype(p) for p in split_params(m.group(4))]
    except ValueError:
        return None
    args = ', '.join('*(%s *)__capprobe_p' % t for t in tys if t)
    if qual:
        if re.search(r'\bstatic\b', pre):
            return '%s::%s(%s)' % (qual, name, args)
        return '((%s *)__capprobe_p)->%s::%s(%s)' % (qual, qual, name, args)
    return '%s(%s)' % (name, args)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='*')
    ap.add_argument('--max-callers', type=int, default=0)
    ap.add_argument('--min', type=int, default=4)
    ap.add_argument('--all', action='store_true', help='every marker, whatever its image references')
    ap.add_argument('--json', default=os.path.join(fe.SCR, 'capscan.json'))
    a = ap.parse_args()
    import orphanowner as oo
    ms = oo.markers()
    if a.all:
        cand = [dict(m, nrefs=oo.refs()[0].get(m['addr'], 0)) for m in ms if not m['frag']]
    else:
        cand = oo.census(ms, a.min, a.max_callers)
    byu = collections.defaultdict(list)
    for m in cand:
        if m['unit'].endswith('.cpp') or m['unit'].endswith('.c'):
            byu[m['unit']].append(m)
    if a.units:
        byu = {u: v for u, v in byu.items() if u in a.units or os.path.basename(u) in a.units}
    fe.compile_cmd(next(iter(byu)) if byu else '')

    def ob2(u):
        obs = [x for x in (fe.compile_cmd(u) or []) if x.startswith('/Ob')]
        return bool(obs) and obs[-1] == '/Ob2'
    skipped = sorted(u for u in byu if not ob2(u))
    byu = {u: v for u, v in byu.items() if u not in skipped}
    jobs = int(os.environ.get('FE_JOBS', '5'))
    base = int(os.environ.get('FE_PFX_BASE', '21'))
    units = sorted(byu)
    lock = threading.Lock()

    def lane(j):
        out = []
        for i in range(j, len(units), jobs):
            u = units[i]
            t = open(fe.R + '/' + u, encoding='latin-1').read()
            probes = []
            for m in byu[u]:
                mm = re.search(r'// ORIGINAL: 0x%08X ' % m['addr'], t)
                if not mm:
                    continue
                e = probe_expr(t, mm.start(), len(probes))
                if e:
                    probes.append((m, e))
            if not probes:
                continue
            add = '\nint __capprobe_g;\nvoid *__capprobe_p;\n' + ''.join(
                'void __capprobe_%d() { %s; __capprobe_g = %d; }\n' % (k, e, k) for k, (m, e) in enumerate(probes))
            src = os.path.join(fe.SCR, 'capscan_%d.cpp' % j)
            c, err = fe.compile_unit(u, src, str(base + j)) if open(src, 'w', encoding='latin-1').write(t + add) else (None, '')
            bad = set()
            if c is None:
                # drop the probes the compiler names, retry once per error line
                ln0 = t.count('\n') + 4
                for l in (err or '').split('\n'):
                    mm = re.search(r'\((\d+)\)\s*:\s*error', l)
                    if mm:
                        bad.add(int(mm.group(1)) - ln0)
                probes2 = [p for k, p in enumerate(probes) if k not in bad]
                add = '\nint __capprobe_g;\nvoid *__capprobe_p;\n' + ''.join(
                    'void __capprobe_%d() { %s; __capprobe_g = %d; }\n' % (k, e, k) for k, (m, e) in enumerate(probes2))
                open(src, 'w', encoding='latin-1').write(t + add)
                c, err = fe.compile_unit(u, src, str(base + j))
                probes = probes2
                if c is None:
                    out.append(dict(unit=u, error=(err or '')[:300]))
                    continue
            full = t + add
            for k, (m, e) in enumerate(probes):
                try:
                    s = c.function('?__capprobe_%d@@YAXXZ' % k)
                except Exception:
                    continue
                sec = c.sections[s.section - 1]
                names = set()
                for r in sec.relocations:
                    try:
                        names.add(c.symbols[r[1]].name)
                    except Exception:
                        pass
                sym = fe.ident(full, m['addr'])[0]
                with lock:
                    try:
                        ev = fe.evaluate(c, full, m['addr'])
                    except Exception:
                        ev = {}
                out.append(dict(unit=u, addr='%08X' % m['addr'], sym=sym, nrefs=m.get('nrefs', 0),
                                called=sym in names, exact=ev.get('exact'), rows=ev.get('diff'), expr=e))
        return out

    res = []
    with ThreadPoolExecutor(jobs) as ex:
        for o in ex.map(lane, range(jobs)):
            res += o
    json.dump(res, open(a.json, 'w'), indent=0)
    errs = [r for r in res if 'error' in r]
    ok = [r for r in res if 'error' not in r]
    called = [r for r in ok if r['called']]
    print('probed %d markers in %d units (%d unit compiles failed); over the cap (CALLED): %d, of them fe-exact %d' % (
        len(ok), len({r['unit'] for r in ok}), len(errs), len(called), sum(1 for r in called if r['exact'])))
    print('skipped (not /Ob2: no auto-inline, so no cap): %s' % ' '.join(os.path.basename(u) for u in skipped))
    for r in errs:
        print('COMPILE FAIL', r['unit'], r['error'][:150].replace('\n', ' | '))
    for r in sorted(called, key=lambda r: (r['unit'], r['addr'])):
        print('CALLED r%d %s %-44s %s %s' % (r['nrefs'], r['addr'], r['sym'][:44], 'EXACT' if r['exact'] else 'rows=%s' % r['rows'],
                                       r['unit']))
    return 0


if __name__ == '__main__':
    sys.exit(main())
