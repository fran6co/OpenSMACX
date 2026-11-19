#!/usr/bin/env python3
"""objmove.py --into DEST.cpp (--range LO-HI[,LO-HI] | --addr A,B,..) [--skip A,B] [--place] [--dry]

Moves object DEFINITIONS to the unit whose data run holds them (MATCHING_GUIDE 2.5, "Data order is unit
evidence too"; analysis/datarun.py lists the candidates). Mechanical: no function body is edited.

A definition is the top-level piece (tumerge.pieces: the item and the comments above it) that carries a
`// RECOVERY_OBJECT: vN 0xADDR` line with ADDR selected. For every host file:
  * the piece is cut out of the host;
  * when no header declares the name (`extern ... NAME` in src/**/*.h), an `extern` declaration derived from
    the definition takes its place in the host (other users already had their own declaration);
  * a piece that is `static` moves as it is (reported: the host must not name it any more).
In DEST the pieces go after the last #include (and pad block), initialised objects first, then the
uninitialised ones, each group in image address order; with --place an initialised object that lies behind one of
DEST's own function literals goes behind that function instead (data and literals are emitted in source order;
a screen: the literal scan can name the wrong function); a definition another moved definition names in its
initialiser is hoisted in front of it. DEST's own `extern` lines for the moved names are dropped (redundant,
and after the definition of a class object VC6 would emit its initializer twice).
Screen with feall on host and DEST units, `recover check-claims`, `recover rebind-literals --unit`."""
import sys, os, re, argparse
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE)); sys.path.insert(0, HERE)
import tumerge  # noqa: E402
R = tumerge.R
OBJ = re.compile(r'// RECOVERY_OBJECT: v\d+ 0x([0-9A-F]{8}) ')


def header_declared(name, _cache={}):
    if not _cache:
        txt = []
        for dp, dn, fn in os.walk(R + '/src'):
            for f in fn:
                if f.endswith('.h'):
                    txt.append(open(os.path.join(dp, f), encoding='latin-1').read())
        _cache['t'] = '\n'.join(txt)
    return re.search(r'^[ \t]*(?:[A-Z_]+\s+)?extern\s+[^;{}()=]*\b' + re.escape(name) + r'\b[^;{}()=]*;', _cache['t'], re.M) is not None


def dest_point(t):
    """offset just past the last header #include (`<x>` or `"x.h"`; an `.inc` inside an initialiser is not one) and
    the pad block that follows it"""
    last = 0
    for m in re.finditer(r'^[ \t]*#[ \t]*include[ \t]*(?:<[^>\n]*>|"[^"\n]*\.h")[^\n]*\n', t, re.M):
        last = m.end()
    m = re.compile(r'(?:[ \t]*//[^\n]*\n)*[ \t]*// CANARY-PADS:[^\n]*\n(?:[ \t]*(?:void|extern)[^\n]*Pad\w*(?:\(\))?;[ \t]*\n)*').match(t, last)
    return m.end() if m else last


def extern_of(piece, name):
    """`extern T name[dims];` from the definition's code (None when it cannot be derived)"""
    code = tumerge.strip_code(piece)
    code = re.sub(r'^[ \t]*#[^\n]*\n', '', code, flags=re.M).strip()
    m = re.match(r'(.*?\b' + re.escape(name) + r'\b(?:\s*\[[^\]]*\])*)\s*(=|;)', code, re.S)
    if not m or '(' in m.group(1):
        return None
    head = ' '.join(m.group(1).split())
    if re.match(r'static\b', head):
        return None
    if re.match(r'extern\b', head):
        return head + ';'
    return 'extern ' + head + ';'


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--into', required=True); ap.add_argument('--range'); ap.add_argument('--addr'); ap.add_argument('--skip', default='')
    ap.add_argument('--dry', action='store_true'); ap.add_argument('--place', action='store_true')
    a = ap.parse_args()
    rngs = [tuple(int(x, 16) for x in r.split('-')) for r in a.range.split(',')] if a.range else []
    addrs = {int(x, 16) for x in a.addr.split(',')} if a.addr else set()
    skip = {int(x, 16) for x in a.skip.split(',') if x}
    want = lambda v: (v in addrs or any(lo <= v < hi for lo, hi in rngs)) and v not in skip
    dest = a.into
    moved = []      # (init?, addr, text, names, host)
    edits = {}
    for dp, dn, fn in os.walk(R + '/src'):
        for f in sorted(fn):
            p = os.path.relpath(os.path.join(dp, f), R)
            if not f.endswith('.cpp') or p == dest:
                continue
            t = open(R + '/' + p, encoding='latin-1').read()
            if not any(want(int(m.group(1), 16)) for m in OBJ.finditer(t)):
                continue
            pcs = tumerge.pieces(t)
            assert ''.join(pcs) == t, p
            out = []
            for pc in pcs:
                # preprocessor lines and other annotations (imports, bindings) above the definition stay in the host
                first = OBJ.search(pc)
                if first:
                    last = 0
                    for m in re.finditer(r'^[ \t]*(?:#[^\n]*(?:\\\n[^\n]*)*|// RECOVERY_(?!OBJECT|LITERAL)[^\n]*|// (?:CANARY|HEADER)-PADS[^\n]*)\n', pc[:first.start()], re.M):
                        last = m.end()
                    if last:
                        out.append(pc[:last]); pc = pc[last:]
                oa = [int(m.group(1), 16) for m in OBJ.finditer(pc)]
                sel = [v for v in oa if want(v)]
                if not sel:
                    out.append(pc); continue
                kind, names = tumerge.piece_names(pc)
                if kind == 'decl' and re.search(r'=', tumerge.strip_code(pc)) and re.match(r'\s*extern\b', re.sub(r'^[ \t]*#[^\n]*\n', '', tumerge.strip_code(pc), flags=re.M).strip()):
                    kind = 'object'     # `extern const T name[] = {...};` is a definition
                if kind != 'object' or tumerge.ORIG.search(pc) or re.search(r'^[ \t]*#(?![ \t]*include[ \t]*"[^"]*\.inc")', pc, re.M):
                    print('  SKIP %s 0x%08X: piece is %s%s' % (p, sel[0], kind or 'no item', ' with an ORIGINAL marker' if tumerge.ORIG.search(pc) else ''))
                    out.append(pc); continue
                if len(sel) != len(oa):
                    print('  NOTE %s: piece of 0x%08X also binds %s' % (p, sel[0], ' '.join('0x%08X' % v for v in oa if v not in sel)))
                name = sorted(names)[0]
                code = tumerge.strip_code(pc)
                static = re.search(r'^\s*static\b', re.sub(r'^[ \t]*#[^\n]*\n', '', code, flags=re.M).strip()) is not None
                init = '=' in code
                repl = ''
                if static:
                    print('  static %s (%s) moves as it is' % (name, p))
                elif not header_declared(name):
                    ex = extern_of(pc, name)
                    if ex is None:
                        print('  WARN %s: no extern derived for %s; declare it by hand' % (p, name))
                    else:
                        repl = ex + '\n'
                        print('  extern left in %s: %s' % (p, ex))
                moved.append((init, min(sel), pc.strip('\n') + '\n', names, p))
                out.append(repl)
            edits[p] = ''.join(out)
    if not moved:
        print('nothing to move'); return 1
    # order: initialised first, by address; hoist dependencies
    moved.sort(key=lambda x: (not x[0], x[1]))
    order = []; done = set()
    def emit(i):
        if i in done:
            return
        done.add(i)
        body = tumerge.strip_code(moved[i][2])
        for j, mv in enumerate(moved):
            if j != i and j not in done and any(re.search(r'\b' + re.escape(n) + r'\b', body) for n in mv[3]):
                emit(j)
        order.append(i)
    for i in range(len(moved)):
        emit(i)
    td = open(R + '/' + dest, encoding='latin-1').read()
    pe = dest_point(td)
    pre, body = td[:pe], td[pe:]
    names = set().union(*[m[3] for m in moved])
    for nm in sorted(names):
        rx = re.compile(r'^[ \t]*extern\s+[^;{}()=]*\b' + re.escape(nm) + r'\b[^;{}()=]*;[^\n]*\n', re.M)
        for m in list(rx.finditer(body))[::-1]:
            print('  local declaration removed from %s: %s' % (dest, m.group(0).strip()))
            body = body[:m.start()] + body[m.end():]
    # an initialised object that lies after one of DEST's own function literals was defined after that function
    # (VC6 emits data and literals in source order): it goes behind that function's piece, not to the top
    place = {}
    if any(m[0] for m in moved) and a.place:
        import tumap
        fs = tumap.load()
        lits = sorted((la, ns) for la, s, us, ns in tumap.literal_owners(fs) if {u for u in us if not u.endswith('.h')} == {dest} and ns)
        addr_of = {f[3]: f[0] for f in fs if f[2] == dest or f[5] == dest}
        import bisect
        la = [x[0] for x in lits]
        for i in order:
            if not moved[i][0]:
                continue
            k = bisect.bisect_left(la, moved[i][1]) - 1
            if k >= 0:
                fa = max(addr_of.get(n, 0) for n in lits[k][1])
                if fa:
                    place[i] = fa
    top = [i for i in order if i not in place]
    block = ('\n' + ''.join(moved[i][2] for i in top)) if top else ''
    full = pre + block + body
    if place:
        pcs = tumerge.pieces(full)
        late = ''
        for i in order:
            if i not in place:
                continue
            mark = '// ORIGINAL: 0x%08X ' % place[i]
            ks = [k for k, pc in enumerate(pcs) if mark in pc and tumerge.piece_names(pc)[0] == 'function']
            if not ks:
                print('  WARN 0x%08X: no function piece for the anchor 0x%08X in %s; placed at the top' % (moved[i][1], place[i], dest))
                late += moved[i][2]; place[i] = None
                continue
            pcs[ks[-1]] = pcs[ks[-1]] + '\n' + moved[i][2]
        full = ''.join(pcs)
        if late:
            full = full.replace(pre, pre + '\n' + late, 1)
    hosts = sorted({m[4] for m in moved})
    print('OBJMOVE -> %s: %d definitions (%d initialised) from %s' % (dest, len(moved), sum(1 for m in moved if m[0]), ' '.join(hosts)))
    for i in order:
        print('  0x%08X %-40s %-24s %s' % (moved[i][1], ' '.join(sorted(moved[i][3])), moved[i][4], 'after 0x%08X' % place[i] if place.get(i) else 'top'))
    if a.dry:
        return 0
    open(R + '/' + dest, 'w', encoding='latin-1').write(full)
    for p, t in edits.items():
        open(R + '/' + p, 'w', encoding='latin-1').write(t)
    return 0


if __name__ == '__main__':
    sys.exit(main())
