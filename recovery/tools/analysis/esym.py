#!/usr/bin/env python3
"""esym.py <unit.cpp> [...] [--write] [--pfx N]

Static-initializer symbols after a regroup. VC6 names a unit's dynamic initializers and their atexit
callbacks `_$E<n>` by a per-unit counter, so every `// ORIGINAL: 0x.. _$E.. FILE` marker carries a
`// symbol _$E<n>` line naming what the unit emits today. Moving an object's definition (or anything
that shifts the counter) to another unit leaves those lines stale and fe/measure find no symbol.

For each `$E` marker hosted in the unit the fresh symbol is identified from the compile alone:
equal bytes outside relocation fields, and every relocation target the tree can name (objects by
RECOVERY_OBJECT address, functions by ORIGINAL marker, sibling `$E` markers through their own
assignment) at the address the image holds at that field. A set the targets cannot tell apart is
assigned in order (counter order is definition order, which is thunk address order). `--write`
rewrites the `// symbol` lines; without it the changes are listed. Markers with no candidate are
reported as NOT EMITTED (the object's definition is missing from the unit, or its type has no
dynamic initializer here)."""
import sys, os, re, struct, glob, argparse
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE)); sys.path.insert(0, HERE)
import fe  # noqa: E402
R = fe.R
MK = re.compile(r'// ORIGINAL: 0x([0-9A-F]{8}) (\S+) 0x([0-9A-Fa-f]+)-0x([0-9A-Fa-f]+)[^\n]*\n((?://(?! ORIGINAL:)[^\n]*\n)*)')
ENAME = re.compile(r'_?\$E\d+$')
_NAMES = None


def names():
    """decorated name -> image address, from every RECOVERY_OBJECT and ORIGINAL marker of the tree"""
    global _NAMES
    if _NAMES is None:
        _NAMES = {}
        for p in glob.glob(R + '/src/**/*.cpp', recursive=True) + glob.glob(R + '/src/**/*.h', recursive=True):
            t = open(p, encoding='latin-1').read()
            for m in re.finditer(r'// RECOVERY_OBJECT: v\d+ 0x([0-9A-F]{8}) (?:0x[0-9A-F]{8} )?(\S+)', t):
                _NAMES.setdefault(m.group(2), int(m.group(1), 16))
            for m in MK.finditer(t):
                s = re.search(r'// symbol\s+(\S+)', m.group(5))
                n = s.group(1) if s else m.group(2)
                if not ENAME.match(n):
                    _NAMES.setdefault(n, int(m.group(1), 16))
    return _NAMES


def u32(b, o):
    return struct.unpack_from('<I', b, o)[0]


def solve(unit, pfx):
    """-> [(addr, old symbol, new symbol|None, how)] for the `$E` markers hosted in unit"""
    path = R + '/' + unit
    t = open(path, encoding='latin-1').read()
    mk = []
    for m in MK.finditer(t):
        s = re.search(r'// symbol\s+(\S+)', m.group(5))
        cur = s.group(1) if s else m.group(2)
        if ENAME.match(m.group(2)) or ENAME.match(cur):
            mk.append((int(m.group(1), 16), int(m.group(3), 16), int(m.group(4), 16), cur))
    if not mk:
        return []
    c, err = fe.compile_unit(unit, path, pfx)
    if c is None:
        raise SystemExit('COMPILE FAIL %s\n%s' % (unit, err))
    nm = names()
    cands = {}
    for sy in c.symbols.values():
        if ENAME.match(sy.name) and sy.section > 0 and sy.type & 0x20:
            sec = c.sections[sy.section - 1]
            cands[sy.name] = sec
    at = {a: i for i, (a, lo, hi, cur) in enumerate(mk)}
    poss = []           # per marker: {symbol: [(sibling marker index, sibling symbol)]}
    for a, lo, hi, cur in mk:
        img = fe.imgbytes(lo, hi)
        ok = {}
        for name, sec in cands.items():
            d = sec.data
            if len(d) < len(img) or len(d) - len(img) > 15 or any(b not in (0x90, 0xCC) for b in d[len(img):]):
                continue        # the object's section is padded to its alignment
            rel = {o: (si, ty) for o, si, ty in sec.relocations if ty in (6, 20)}
            mask = set()
            for o in rel:
                mask.update(range(o, o + 4))
            if any(d[i] != img[i] for i in range(len(img)) if i not in mask):
                continue
            good = True; sib = []; ver = 0
            for o, (si, ty) in rel.items():
                tn = c.symbols[si].name
                v = u32(img, o)
                add = u32(d, o)
                tgt = v if ty == 6 else (lo + o + 4 + v) & 0xffffffff
                if ty == 20:
                    add = 0
                if ENAME.match(tn):
                    if tgt - add not in at:
                        good = False; break
                    sib.append((at[tgt - add], tn))
                elif tn in nm:
                    if (nm[tn] + add) & 0xffffffff != tgt:
                        good = False; break
                    ver += 1
            if good:
                ok[name] = (sib, ver)
        # a candidate whose targets are all named at the image's addresses beats one with an unnamed target
        best = max((v for s_, v in ok.values()), default=0)
        poss.append({n_: s_ for n_, (s_, v) in ok.items() if v == best})
    changed = True
    while changed:
        changed = False
        for i, ok in enumerate(poss):
            for name in list(ok):
                if any(sn not in poss[j] for j, sn in ok[name]):
                    del ok[name]; changed = True
        # a marker with one candidate takes it from everybody else; sibling links pin their targets
        for i, ok in enumerate(poss):
            if len(ok) == 1:
                name = next(iter(ok))
                for j, o2 in enumerate(poss):
                    if j != i and name in o2:
                        del o2[name]; changed = True
                for j, sn in ok[name]:
                    for other in list(poss[j]):
                        if other != sn:
                            del poss[j][other]; changed = True
    out = []
    num = lambda s: int(re.search(r'\d+$', s).group(0))
    # ambiguous sets: same candidate sets are assigned in address / counter order
    groups = {}
    for i, ok in enumerate(poss):
        if len(ok) > 1:
            groups.setdefault(frozenset(ok), []).append(i)
    order = {}
    for key, idx in groups.items():
        syms = sorted(key, key=num)
        if len(syms) == len(idx):
            for i, s in zip(sorted(idx, key=lambda i: mk[i][0]), syms):
                order[i] = s
    for i, (a, lo, hi, cur) in enumerate(mk):
        ok = poss[i]
        if len(ok) == 1:
            out.append((a, cur, next(iter(ok)), 'targets'))
        elif i in order:
            out.append((a, cur, order[i], 'order'))
        elif not ok:
            out.append((a, cur, None, 'NOT EMITTED'))
        else:
            out.append((a, cur, None, 'AMBIGUOUS ' + ' '.join(sorted(ok, key=num))))
    return out


def apply(unit, res):
    path = R + '/' + unit
    t = open(path, encoding='latin-1').read()
    n = 0
    for a, cur, new, how in res:
        if new is None or new == cur:
            continue
        m = re.search(r'// ORIGINAL: 0x%08X (\S+) [^\n]*\n((?://(?! ORIGINAL:)[^\n]*\n)*)' % a, t)
        blk = m.group(0)
        if re.search(r'// symbol\s+\S+', blk):
            blk2 = re.sub(r'(// symbol\s+)\S+', lambda x: x.group(1) + new, blk, count=1)
        else:
            blk2 = blk + '// symbol    ' + new + '\n'
        t = t[:m.start()] + blk2 + t[m.end():]
        n += 1
    if n:
        open(path, 'w', encoding='latin-1').write(t)
    return n


def run(units, write=False, pfx=2, quiet=False):
    bad = 0
    for u in units:
        res = solve(u, pfx)
        ch = [r for r in res if r[2] is not None and r[2] != r[1]]
        miss = [r for r in res if r[2] is None]
        bad += len(miss)
        if not quiet or ch or miss:
            print('ESYM %s: %d $E markers, %d stale, %d unresolved%s' % (u, len(res), len(ch), len(miss), ' (written)' if write and ch else ''))
        for a, cur, new, how in res:
            if new is None:
                print('  0x%08X %-8s %s' % (a, cur, how))
            elif new != cur and not quiet:
                print('  0x%08X %-8s -> %-8s (%s)' % (a, cur, new, how))
        if write:
            apply(u, res)
    return bad


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='+'); ap.add_argument('--write', action='store_true'); ap.add_argument('--pfx', type=int, default=int(os.environ.get('FE_PFX_BASE', '21')))
    a = ap.parse_args()
    sys.exit(1 if run([u if u.startswith('src/') else 'src/' + u for u in a.units], a.write, a.pfx) else 0)
