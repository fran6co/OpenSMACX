#!/usr/bin/env python3
"""litgap.py <unit> [--src ALT.cpp] [--pfx N] [--ctx K] [--all] | --tree [unit ...] [--json F]

String-literal gap screen (MATCHING_GUIDE 2.8, "Header inlines behind dead probes"). VC6 emits every
string literal of a unit into its .data in source order ($SG<n>, numbered by the front-end counter that
the count canaries depend on), even when the only use is an argument an expanded inline ignores. The
expanded `rnd(bounds, "LABEL")` sites of the original therefore left their labels in the image as
literals no instruction references, in the middle of the unit's other literals; a source that writes
`rand() % K` (or another stand-in) has neither the literal nor its counter slot.

Per unit: our object's .data strings in offset order, the image's strings over the span our strings
map to (each of ours located by content near the previous match), aligned. Prints the image strings
our object lacks (`U` = no code reference in the image: an argument dropped by an expansion, or a
literal of an unrecovered table) with the nearest matched neighbours, and our strings the image lacks.
--tree runs every unit (FE_PFX_BASE/FE_JOBS) and lists the units with image-only `U` strings.
Screen only: literal binding and data layout are checked by `recover check-data`."""
import sys, os, re, struct, json, argparse, difflib
from concurrent.futures import ThreadPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
import fe  # noqa: E402

R = fe.R
_VALS = None
_DATA = None


def code_refs():
    """every 4-byte value in the image's .text (a superset of the literal addresses code references)"""
    global _VALS
    if _VALS is None:
        va, vs, ro = [s for s in fe._secs if s[0] == 0x401000][0]
        tb = fe._img[ro:ro + vs]
        _VALS = set(struct.unpack_from('<I', tb, i)[0] for i in range(len(tb) - 3))
    return _VALS


def image_strings():
    """[(va, text)] of 4-aligned NUL-terminated printable strings in the image's data sections"""
    global _DATA
    if _DATA is None:
        _DATA = []
        for va, vs, ro in fe._secs:
            if va == 0x401000:
                continue
            blob = fe._img[ro:ro + vs]
            for m in re.finditer(rb'[\x09\x0a\x0d\x20-\x7e\xa0-\xff]+(?=\x00)', blob):
                a = va + m.start()
                if a % 4 == 0 and (m.start() == 0 or blob[m.start() - 1] == 0):
                    _DATA.append((a, m.group(0).decode('latin-1')))
    return _DATA


def our_strings(c):
    """[(offset, text)] of the object's $SG literals in .data order"""
    out = []
    for sy in c.symbols.values():
        if not sy.name.startswith('$SG') or sy.section <= 0:
            continue
        sec = c.sections[sy.section - 1]
        if not sec.name.startswith('.data'):
            continue
        d = sec.data
        e = d.find(b'\0', sy.value)
        out.append((sy.section, sy.value, d[sy.value:e].decode('latin-1')))
    out.sort()
    return [(v, s) for _, v, s in out]


def gaps(c, ctx=1, all_=False):
    ours = our_strings(c)
    if not ours:
        return None
    img = image_strings()
    pos = {}
    for i, (a, s) in enumerate(img):
        pos.setdefault(s, []).append(i)
    # anchor each of our strings to the first image occurrence after the previous anchor (within a window)
    anchors = []
    last = None
    for k, (_, s) in enumerate(ours):
        cand = pos.get(s, [])
        pick = None
        for i in cand:
            if last is None or 0 < i - last <= 400:
                pick = i
                break
        if pick is None and last is None and cand:
            pick = cand[0]
        if pick is not None:
            anchors.append((k, pick))
            last = pick
    if len(anchors) < 2:
        return dict(ours=len(ours), anchored=len(anchors), missing=[], extra=[])
    # our strings before the first / after the last anchor extend the span by as many image strings
    lo = max(0, anchors[0][1] - anchors[0][0])
    hi = min(len(img) - 1, anchors[-1][1] + (len(ours) - 1 - anchors[-1][0]))
    refs = code_refs()
    # unreferenced literals right before/after the span (a unit's first or last literal is often a label)
    for _ in range(4):
        if lo > 0 and img[lo - 1][0] not in refs:
            lo -= 1
        if hi + 1 < len(img) and img[hi + 1][0] not in refs:
            hi += 1
    span = img[lo:hi + 1]
    A = [s for _, s in span]
    B = [s for _, s in ours]
    missing, extra = [], []
    for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(None, A, B, autojunk=False).get_opcodes():
        if tag in ('delete', 'replace'):
            for i in range(i1, i2):
                a, s = span[i]
                u = a not in refs
                if u or all_:
                    before = A[max(0, i - ctx):i]
                    after = A[i + 1:i + 1 + ctx]
                    missing.append(dict(va='%x' % a, text=s, unref=u, before=before, after=after))
        if tag in ('insert', 'replace'):
            for j in range(j1, j2):
                extra.append(B[j])
    return dict(ours=len(ours), anchored=len(anchors), lo='%x' % span[0][0], hi='%x' % span[-1][0],
                missing=missing, extra=extra)


def one(unit, src, pfx, ctx, all_):
    c, err = fe.compile_unit(unit, src, pfx)
    if c is None:
        return dict(error=err)
    return gaps(c, ctx, all_)


def show(unit, g):
    if g is None:
        print('%s: no literals' % unit)
        return
    if 'error' in g:
        print('%s: COMPILE FAIL %s' % (unit, g['error'][:200]))
        return
    print('%s: ours %d literals, %d anchored, image %s-%s' % (unit, g['ours'], g['anchored'], g.get('lo', '?'), g.get('hi', '?')))
    for m in g['missing']:
        print('  %s %s %-24r  after %s  before %s' % (m['va'], 'U' if m['unref'] else ' ', m['text'][:24],
                                                    [x[:20] for x in m['before']], [x[:20] for x in m['after']]))
    if g['extra']:
        print('  ours only: %s' % [x[:24] for x in g['extra'][:20]])


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='*')
    ap.add_argument('--src')
    ap.add_argument('--pfx', default=os.environ.get('FE_PFX_BASE', '2'))
    ap.add_argument('--ctx', type=int, default=1)
    ap.add_argument('--all', action='store_true', help='also image-only strings that code references')
    ap.add_argument('--tree', action='store_true')
    ap.add_argument('--json')
    a = ap.parse_args()
    norm = lambda u: u if u.startswith('src/') else 'src/' + u
    if not a.tree:
        unit = norm(a.units[0])
        g = one(unit, a.src or R + '/' + unit, a.pfx, a.ctx, a.all)
        show(unit, g)
        return 0
    fe.compile_cmd('src/alpha.cpp')
    units = sorted(k[len(R) + 1:] for k in fe._CC if k.startswith(R + '/src/'))
    if a.units:
        units = [u for u in units if u in map(norm, a.units)]
    image_strings()
    code_refs()
    jobs = int(os.environ.get('FE_JOBS', '5'))
    base = int(a.pfx)

    def lane(k):
        return [(u, one(u, R + '/' + u, str(base + k), a.ctx, a.all)) for u in units[k::jobs]]
    res = []
    with ThreadPoolExecutor(jobs) as ex:
        for o in ex.map(lane, range(jobs)):
            res += o
    res.sort()
    tot = 0
    for u, g in res:
        if g and 'error' not in g and any(m['unref'] for m in g['missing']):
            n = sum(m['unref'] for m in g['missing'])
            tot += n
            show(u, g)
    print('UNREFERENCED IMAGE-ONLY LITERALS', tot)
    if a.json:
        json.dump(res, open(a.json, 'w'), indent=1)
    return 0


if __name__ == '__main__':
    sys.exit(main())
