#!/usr/bin/env python3
"""tumap.py [--text] [--lits] [--units] [--cands] [--inside] [--unit U] [--json F]
tumap.py --merge A.cpp B.cpp [...] --into C.cpp [--top] [--dry]
tumap.py --move 0xADDR[,0xADDR] --into C.cpp [--top] [--dry]

Translation-unit evidence map (MATCHING_GUIDE 2.5, "Unit boundaries"). An original unit contributes one
contiguous .text run in link order (COMDAT copies of header inlines aside) and one contiguous run of string
literals in .data, the units in the same relative order in both sections.

Image side only (no compile): every ORIGINAL marker gives a function span and a current unit (its file, or
its `// compile_context`); every string literal of the image is attributed to the units whose functions
reference it (4-byte scan of the function spans).

  --text    .text runs in address order: unit, span, functions, per-unit flag set, literal span of the run
  --lits    literal runs in .data order: unit, span, strings (shared and unreferenced strings shown apart)
  --units   per unit: number of .text runs and literal runs, flags
  --cands   (a) merge candidates: units adjacent in .text whose literal runs are adjacent in the same order;
            `B-STARTS-UNIT` = static-initializer thunks sit at the start of the second run (a hard boundary),
            `no-init-boundary` = nothing in the image separates the two. --strong keeps the pairs with the
            same flags, no boundary and at least --min functions on both sides.
            (b) split candidates: units with two or more .text runs (each run with its literal span)
  --inside  (c) functions sitting alone (runs of <= 2 functions) inside another unit's run
  default   --units --cands --inside

--merge moves the bodies of the listed units into C.cpp in image address order (whole annotation blocks:
each top-level chunk of a file is keyed by the first ORIGINAL address it holds; chunks without a marker
stay attached to the next marked chunk of their file; the includes are united, C's first), removes the
emptied files and rewrites CMakeLists.txt, recovery/project.json and `// compile_context` annotations.
Purely mechanical: no body is edited. Screen only; fe/feall and `recover check-claims` judge the result."""
import sys, os, re, struct, bisect, json, argparse, collections, subprocess
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE)); sys.path.insert(0, HERE)
import fe, litgap, giantcallee  # noqa: E402
R = fe.R
LIT_LO, LIT_HI = 0x680000, 0x6a0000


def rel(p):
    return os.path.relpath(p, R)


def load():
    mks, texts = giantcallee.load_markers()
    cc = re.compile(r'// compile_context (\S+)')
    out = []
    seen = set()
    for k in mks:
        if k.hi <= k.lo or not (0x401000 <= k.lo < 0x680000):
            continue
        m = cc.search(texts[k.path], k.pos, k.end)
        k_unit = m.group(1) if m else rel(k.path)
        if (k.lo, k_unit) in seen:
            continue
        seen.add((k.lo, k_unit))
        out.append((k.lo, k.hi, k_unit, k.name, k.exact, rel(k.path)))
    out.sort()
    return out


def flags(unit):
    c = fe.compile_cmd(unit)
    if not c:
        return '?'
    keep = [t for t in c if re.fullmatch(r'/O[a-z0-9-]+|/G[A-Za-z0-9]+|/Ob\d', t)]
    return ' '.join(sorted(set(keep) - {'/Og', '/Ot', '/Gs', '/Gy', '/GX'})) or '-'


def is_hdr(u):
    return u.endswith('.h')


def lib_of(u):
    for p in ('src/jackal/', 'src/caviar/', 'src/uv/', 'src/vox/', 'src/crt/', 'src/zlib/'):
        if u.startswith(p):
            return p[4:-1]
    return 'game'


def text_runs(fs, transparent_hdr=True):
    """[[unit, lo, hi, [functions]]]; header-hosted markers (COMDAT inlines) do not break a run"""
    runs = []
    for f in fs:
        u = f[2]
        if transparent_hdr and is_hdr(u):
            if runs:
                runs[-1][4].append(f)
            continue
        if runs and runs[-1][0] == u:
            runs[-1][2] = f[1]; runs[-1][3].append(f)
        else:
            runs.append([u, f[0], f[1], [f], []])
    return runs


def literal_owners(fs):
    """[(va, text, {unit}, [function])] for every image string in the literal region"""
    img = litgap.image_strings()
    S = {a for a, _ in img if LIT_LO < a < LIT_HI}
    los = [f[0] for f in fs]
    va, vs, ro = [s for s in fe._secs if s[0] == 0x401000][0]
    tb = fe._img[ro:ro + vs]
    own = collections.defaultdict(list)
    for f in fs:
        o = f[0] - va
        for i in range(o, f[1] - va - 3):
            v = struct.unpack_from('<I', tb, i)[0]
            if v in S:
                own[v].append(f)
    out = []
    for a, s in img:
        if LIT_LO < a < LIT_HI:
            o = own.get(a, [])
            out.append((a, s, {f[2] for f in o}, [f[3] for f in o]))
    return out


def init_thunks():
    """addresses of the dynamic-initializer functions ($E<n>) in CRT table order (.CRT$XCU, found as the
    run of .text pointers around 0x682568). VC6 emits each at its object's definition point, so the first
    one of a unit (objects defined at the top of the file) marks where that unit starts."""
    def u32(a):
        return struct.unpack('<I', fe.imgbytes(a, a + 4))[0]
    a = 0x682568
    while 0x401000 <= u32(a - 4) < 0x680000:
        a -= 4
    out = []
    while 0x401000 <= u32(a) < 0x680000:
        out.append(u32(a)); a += 4
    return out


def lit_runs(lits):
    """[[unit|'*shared*', lo, hi, n, n_unref]] in address order. A string several units reference
    (a named object, or a header inline's literal) and unreferenced strings do not break a run."""
    runs = []
    pend = 0
    for a, s, us, ns in lits:
        us = {u for u in us if not is_hdr(u)}
        if len(us) != 1:
            if not us:
                pend += 1
            elif runs:
                runs[-1][5] += 1
            continue
        u = next(iter(us))
        if runs and runs[-1][0] == u:
            runs[-1][2] = a; runs[-1][3] += 1; runs[-1][4] += pend
        else:
            runs.append([u, a, a, 1, 0, 0])
        pend = 0
    return runs


def main():
    ap = argparse.ArgumentParser()
    for o in ('text', 'lits', 'units', 'cands', 'inside', 'dry', 'top', 'strong', 'deps'):
        ap.add_argument('--' + o, action='store_true')
    ap.add_argument('--unit'); ap.add_argument('--json'); ap.add_argument('--min', type=int, default=3)
    ap.add_argument('--merge', nargs='+'); ap.add_argument('--into'); ap.add_argument('--move'); ap.add_argument('--include', action='append')
    ap.add_argument('--new-like'); ap.add_argument('--after')
    a = ap.parse_args()
    if a.merge:
        import tumerge
        return tumerge.merge(a.merge, a.into, a.dry, top=a.top)
    if a.move:
        import tumerge
        ad = {int(x, 16) for x in a.move.split(',')}
        hosts = sorted({f[5] for f in load() if f[0] in ad})
        return tumerge.merge(hosts, a.into, a.dry, addrs=ad, top=a.top, with_inc=a.include, new_like=a.new_like, after=a.after, deps=a.deps)
    fs = load()
    runs = text_runs(fs)
    lits = literal_owners(fs)
    lr = lit_runs(lits)
    by_unit_t = collections.defaultdict(list)
    for i, r in enumerate(runs):
        by_unit_t[r[0]].append(i)
    by_unit_l = collections.defaultdict(list)
    for i, r in enumerate(lr):
        by_unit_l[r[0]].append(i)

    def lit_span_of_run(r):
        """literal addresses referenced by this text run's functions and by no other unit"""
        names = {f[3] for f in r[3]}
        los = {f[0] for f in r[3]}
        xs = [a_ for a_, s, us, ns in lits if us == {r[0]} and names & set(ns)]
        return (min(xs), max(xs), len(xs)) if xs else None

    if not (a.text or a.lits or a.units or a.cands or a.inside):
        a.units = a.cands = a.inside = True
    inits = sorted(init_thunks())

    def init_at_start(i):
        """initializer thunks between the end of the previous run and the first function of run i that is
        not itself a thunk (S = the run starts with its unit's static initializers), and thunks later in it"""
        r = runs[i]
        j = i
        while j > 0 and all(f[0] in inits for f in runs[j - 1][3]):
            j -= 1          # runs made only of thunks (hosted in the object's class file) belong to this start
        lo = runs[j - 1][2] if j else 0x401000
        first = next((f[0] for f in r[3] if f[0] not in inits and not f[3].startswith('$E')), r[1])
        head = [x for x in inits if lo <= x <= first]
        later = [x for x in inits if first < x < r[2]]
        return head, later

    if a.text:
        print('# .text runs (address order); INIT n = the run starts with n static-initializer thunks (a unit start)')
        for i, r in enumerate(runs):
            if a.unit and a.unit not in r[0]:
                continue
            ls = lit_span_of_run(r)
            head, later = init_at_start(i)
            print('T%04d %08X-%08X %4d fn %-36s [%s] lits %s%s%s%s' % (
                i, r[1], r[2], len(r[3]), r[0], flags(r[0]),
                '%08X-%08X n=%d' % ls if ls else '-', '  +hdr %d' % len(r[4]) if r[4] else '',
                '  INIT %d' % len(head) if head else '', '  (+%d later)' % len(later) if later else ''))
    if a.lits:
        print('# literal runs (.data order)')
        for i, r in enumerate(lr):
            if a.unit and a.unit not in r[0]:
                continue
            print('L%04d %08X-%08X %4d str %-36s unref %d shared %d' % (i, r[1], r[2], r[3], r[0], r[4], r[5]))
    if a.units:
        print('# units: text runs / literal runs / functions / flags')
        for u in sorted(by_unit_t, key=lambda u: runs[by_unit_t[u][0]][1]):
            if a.unit and a.unit not in u:
                continue
            n = sum(len(runs[i][3]) for i in by_unit_t[u])
            print('U %-40s text %3d  lits %3d  fn %4d  [%s]' % (u, len(by_unit_t[u]), len(by_unit_l.get(u, [])), n, flags(u)))
    if a.cands:
        print('# (a) merge candidates: adjacent in .text; literal adjacency in the same order')
        for i in range(len(runs) - 1):
            A, B = runs[i], runs[i + 1]
            if A[0] == B[0]:
                continue
            if a.unit and a.unit not in A[0] and a.unit not in B[0]:
                continue
            if lib_of(A[0]) != lib_of(B[0]):
                continue
            la, lb = lit_span_of_run(A), lit_span_of_run(B)
            gap = B[1] - A[2]
            if la and lb:
                # strings strictly between the two spans that belong to a third unit
                lo_, hi_ = (la[1], lb[0]) if la[1] < lb[0] else (lb[1], la[0])
                order = 'same' if la[1] < lb[0] else ('REVERSED' if lb[1] < la[0] else 'INTERLEAVED')
                third = collections.Counter()
                for a_, s, us, ns in lits:
                    if lo_ < a_ < hi_:
                        for u in us:
                            if u not in (A[0], B[0]) and not is_hdr(u):
                                third[u] += 1
                lit = '%s between=%s' % (order, dict(third) if third else 'none')
            else:
                lit = 'no-literals(%s)' % ('A' if not la else 'B') if (la or lb) else 'no-literals(both)'
            fa, fb = flags(A[0]), flags(B[0])
            single = len(by_unit_t[A[0]]) == 1 and len(by_unit_t[B[0]]) == 1
            head, _ = init_at_start(i + 1)
            if a.strong and (head or fa != fb or len(A[3]) < a.min or len(B[3]) < a.min):
                continue
            print('M T%04d %-30s (%d fn) | %-30s (%d fn) gap %3d  flags %s  lits %s  %s%s' % (
                i, A[0][4:], len(A[3]), B[0][4:], len(B[3]), gap, 'same' if fa == fb else '%s|%s' % (fa, fb), lit,
                'B-STARTS-UNIT(init %d)' % len(head) if head else 'no-init-boundary',
                '' if single else '  (multi-run unit)'))
        print('# (b) units with two or more .text runs')
        for u in sorted(by_unit_t, key=lambda u: -len(by_unit_t[u])):
            if len(by_unit_t[u]) < 2 or (a.unit and a.unit not in u):
                continue
            print('S %-40s %d runs' % (u, len(by_unit_t[u])))
            for i in by_unit_t[u]:
                r = runs[i]; ls = lit_span_of_run(r)
                prev = runs[i - 1][0] if i else '-'; nxt = runs[i + 1][0] if i + 1 < len(runs) else '-'
                print('    T%04d %08X-%08X %4d fn  lits %-28s after %s before %s' % (
                    i, r[1], r[2], len(r[3]), '%08X-%08X n=%d' % ls if ls else '-', prev[4:], nxt[4:]))
    if a.inside:
        print('# (c) short runs (<= 2 functions) enclosed by one other unit')
        for i in range(1, len(runs) - 1):
            r = runs[i]
            if len(r[3]) <= 2 and runs[i - 1][0] == runs[i + 1][0] and (not a.unit or a.unit in r[0] or a.unit in runs[i - 1][0]):
                print('I T%04d %08X %-28s in %-28s %s' % (i, r[1], r[0][4:], runs[i - 1][0][4:], ','.join(f[3] for f in r[3])))
    if a.json:
        json.dump({'text': [[r[0], r[1], r[2], [f[3] for f in r[3]]] for r in runs],
                   'lits': [r[:6] for r in lr]}, open(a.json, 'w'), indent=0)


if __name__ == '__main__':
    main()
