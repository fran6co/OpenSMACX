#!/usr/bin/env python3
"""inlinemap.py <unit> <0xADDR> [--src ALT.cpp] [--pfx N] [--callee KEY ...] [--seq] [--all]

/Ob2 inline decisions of one function, image against ours, per inline-able callee.

Both call streams (call rel32 and tail `jmp rel32` to other functions) are parsed into a token
stream in which a run of calls that equals the call sequence of a known inline function's
out-of-line copy in the IMAGE (its ORIGINAL marker span) becomes one token `F*` (expanded);
a call to F stays `F` (called out of line). Expansion signatures nest: an expanded ~Popup may
contain ~Scroll as a call or as `~Scroll*`.

Candidate inline functions: every callee whose call count differs between the two sides, the
callees of those copies (recursively), and every --callee KEY. A KEY is the mangled prefix
before '@@' (`??1Scroll`, `?close@Popup`), matched against both sides' symbols.

Output, per candidate F, two aligned rows (image / ours) over F's sites in stream order:
  E = expanded, c = called, . = no site on that side; then the site positions that disagree.
--seq prints the whole aligned token streams. Read with recovery/MATCHING_GUIDE.md 2.8.
"""
import sys, os, re, glob, difflib, argparse, collections
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
import fe  # noqa: E402

R = fe.R


def key(name):
    """mangled prefix before '@@' (class + member), or the C name"""
    if name.startswith('_') and '@' not in name:
        return name
    i = name.find('@@')
    return name[:i] if i > 0 else name


def markers():
    a2s, spans = {}, {}
    for p in glob.glob(R + '/src/**/*.cpp', recursive=True):
        t = open(p, encoding='latin-1').read()
        for m in re.finditer(r'// ORIGINAL: 0x([0-9A-F]{8}) (\S+) 0x([0-9A-Fa-f]+)-0x([0-9A-Fa-f]+)[^\n]*\n((?://[^\n]*\n)*)', t):
            sym = m.group(2)
            m2 = re.search(r'// symbol\s+(\S+)', m.group(5))
            if m2:
                sym = m2.group(1)
            a = int(m.group(1), 16)
            a2s[a] = sym
            spans.setdefault(key(sym), (int(m.group(3), 16), int(m.group(4), 16)))
    return a2s, spans


A2S, SPANS = markers()
CRT = {0x645470: '_strcat', 0x645460: '_strcpy', 0x6453E0: '_strlen', 0x6465F0: '_memset', 0x645930: '_memcpy',
       0x644EF2: '_free', 0x645660: '_strcmp', 0x645550: '__chkstk', 0x64558A: '??2@YAPAXI@Z', 0x6458C8: '__ftol'}


def img_stream(lo, hi):
    out = []
    for ins in fe.md.disasm(fe.imgbytes(lo, hi), lo):
        if ins.mnemonic not in ('call', 'jmp') or ins.size != 5:
            continue
        try:
            t = int(ins.op_str, 16)
        except ValueError:
            continue
        if ins.mnemonic == 'jmp' and lo <= t < hi:
            continue
        n = CRT.get(t) or A2S.get(t, 'sub_%x' % t)
        out.append((ins.address, key(n)))
    return out


def our_stream(c, sym):
    s = c.function(sym)
    sec = c.sections[s.section - 1]
    rm = {r[0]: r for r in sec.relocations}
    out = []
    for ins in fe.md.disasm(sec.data, 0):
        if ins.mnemonic in ('call', 'jmp') and ins.size == 5 and (ins.address + 1) in rm:
            sy = c.symbols.get(rm[ins.address + 1][1])
            n = sy.name if sy else '?'
            if ins.mnemonic == 'jmp' and n == sym:
                continue
            out.append((ins.address, key(n)))
    return out


_RAW = {}
_SIG = {}


def raw_signature(k):
    """call sequence of the image's out-of-line copy of k (main span only), or None"""
    if k not in _RAW:
        sp = SPANS.get(k)
        _RAW[k] = [n for a, n in img_stream(*sp)] if sp else None
    return _RAW[k]


def signature(k, cands=None):
    """source-level call list of k: the image copy's calls with expansions inside the copy collapsed
    back to one call each (~Scroll's copy expands ~FlatButton; its signature lists ~FlatButton)."""
    if k in _SIG:
        return _SIG[k]
    raw = raw_signature(k)
    if not raw:
        _SIG[k] = raw
        return raw
    _SIG[k] = raw  # provisional, breaks recursion
    others = sorted(c for c in (cands or ()) if c != k)
    if others:
        toks = parse([(0, n) for n in raw], others)
        _SIG[k] = [n for _, n, _, _ in toks]
    return _SIG[k]


def parse(stream, cands, depth=0):
    """greedy left-to-right parse: -> tokens [(pos, name, 'E'|'c', children)]. An expansion is a run
    that matches the callee's signature, each item called or itself expanded; a run that matches at
    least two items (or the whole signature) and then stops counts too, since tail merging moves the
    rest of a teardown into a shared copy."""
    names = [n for a, n in stream]
    toks = []
    p = 0
    while p < len(names):
        best = None
        if depth < 4:
            for k in sorted(cands):  # deterministic tie-breaks
                sig = _SIG[k] if k in _SIG else raw_signature(k)
                if not sig or names[p] == k:
                    continue
                m = match(names, p, sig, cands, depth + 1)
                if m is not None and (best is None or m[0] > best[1]):
                    best = (k, m[0], m[1])
        if best:
            toks.append((stream[p][0], best[0], 'E', best[2]))
            p = best[1]
        else:
            toks.append((stream[p][0], names[p], 'c', []))
            p += 1
    return toks


def match(names, p, sig, cands, depth):
    """-> (end, children) or None"""
    q = p
    kids = []
    for n_done, item in enumerate(sig):
        if q < len(names) and names[q] == item:
            kids.append((item, 'c'))
            q += 1
            continue
        sub = (_SIG[item] if item in _SIG else raw_signature(item)) if item in cands and depth < 4 else None
        if sub:
            m = match(names, q, sub, cands, depth + 1)
            if m is not None:
                kids.append((item, 'E'))
                q = m[0]
                continue
        if n_done >= 2 or (n_done >= 1 and len(sig) > 1 and q - p >= 2):
            return q, kids
        return None
    return q, kids


def flatten(toks):
    """-> [(pos, name, E/c)] for top-level tokens plus their nested children (positions of parent)"""
    out = []
    for pos, n, st, kids in toks:
        out.append((pos, n, st, 0))
        for kn, ks in kids:
            out.append((pos, kn, ks, 1))
    return out


def analyse(c, sym, lo, hi, callees=(), sigs=()):
    """-> (img stream, our stream, aligned token rows, per-callee site pairs, candidates)"""
    _SIG.clear()  # collapsed signatures depend on the candidate set
    img = img_stream(lo, hi)
    ours = our_stream(c, sym)
    ci = collections.Counter(n for _, n in img)
    co = collections.Counter(n for _, n in ours)
    cands = set(callees)
    for spec in sigs:
        k, _, items = spec.partition('=')
        _RAW[k] = [x for x in items.split(',') if x]
        _SIG.pop(k, None)
        cands.add(k)
    for n in set(ci) | set(co):
        if ci[n] != co[n] and raw_signature(n) and not n.startswith('??0'):
            cands.add(n)
    # callees of a differing candidate's copy, two levels down (an expanded ~Popup shows ~Scroll)
    frontier = list(cands)
    for _ in range(2):
        nxt = []
        for k in frontier:
            for n in raw_signature(k) or []:
                if n not in cands and raw_signature(n) and not n.startswith('??0') and n != '__chkstk':
                    cands.add(n)
                    nxt.append(n)
        frontier = nxt
    # collapse expansions inside each copy, smallest copies first
    for k in sorted(cands, key=lambda k: (len(raw_signature(k) or []), k)):
        signature(k, cands)
    # a candidate whose copy makes no calls cannot be recognised when expanded
    cands = {k for k in cands if signature(k)}
    ti = parse(img, cands)
    to = parse(ours, cands)
    A = [n for _, n, _, _ in ti]
    B = [n for _, n, _, _ in to]
    rows = []  # aligned (img_tok or None, our_tok or None)
    for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(None, A, B, autojunk=False).get_opcodes():
        if tag == 'equal':
            rows += list(zip(ti[i1:i2], to[j1:j2]))
        else:
            rows += [(t, None) for t in ti[i1:i2]] + [(None, t) for t in to[j1:j2]]
    # per-callee aligned decisions (top level and one level of nesting)
    per = collections.OrderedDict()
    for x, y in rows:
        fx = flatten([x]) if x else []
        fy = flatten([y]) if y else []
        names = []
        for t in fx + fy:
            if t[1] in cands and t[1] not in names:
                names.append(t[1])
        for n in names:
            sx = [t for t in fx if t[1] == n]
            sy = [t for t in fy if t[1] == n]
            for q in range(max(len(sx), len(sy))):
                per.setdefault(n, []).append((sx[q] if q < len(sx) else None, sy[q] if q < len(sy) else None))
    return img, ours, rows, per, cands


def strings(sites):
    """-> (image E/c/. string, ours E/c/. string, nesting level string 0/1)"""
    ri = ''.join(s[0][2] if s[0] else '.' for s in sites)
    ro = ''.join(s[1][2] if s[1] else '.' for s in sites)
    lv = ''.join(str((s[0] or s[1])[3]) for s in sites)
    return ri, ro, lv


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('unit')
    ap.add_argument('addr')
    ap.add_argument('--src')
    ap.add_argument('--pfx', default=os.environ.get('FE_PFX_BASE', '21'))
    ap.add_argument('--callee', action='append', default=[])
    ap.add_argument('--seq', action='store_true')
    ap.add_argument('--sig', action='append', default=[], help='KEY=callee1,callee2: signature for a copy whose calls are all expanded (e.g. ??1StringList=?remove_all@StringStruct,??1StringStruct)')
    ap.add_argument('--all', action='store_true', help='also list callees with no disagreement')
    a = ap.parse_args()
    addr = int(a.addr, 16)
    src = a.src or R + '/' + a.unit
    text = open(src, encoding='latin-1').read()
    sym, lo, hi = fe.ident(text, addr)
    c, err = fe.compile_unit(a.unit, src, a.pfx)
    if c is None:
        raise SystemExit(err)
    img, ours, rows, per, cands = analyse(c, sym, lo, hi, a.callee, a.sig)
    print('%s %s  image calls %d, ours %d; %d inline candidates' % (a.addr, sym, len(img), len(ours), len(cands)))
    if a.seq:
        for x, y in rows:
            fx = '%x %s%s' % (x[0], x[1], '*' if x[2] == 'E' else '') if x else ''
            fy = '%x %s%s' % (y[0], y[1], '*' if y[2] == 'E' else '') if y else ''
            print('  %-50s %-50s%s' % (fx, fy, '' if x and y and x[1:3] == y[1:3] else '  <>'))
    for n, sites in per.items():
        ri = ''.join(s[0][2] if s[0] else '.' for s in sites)
        ro = ''.join(s[1][2] if s[1] else '.' for s in sites)
        if ri == ro and not a.all:
            continue
        bad = [q + 1 for q, s in enumerate(sites) if (s[0] and s[0][2]) != (s[1] and s[1][2])]
        print('\n%s  (%d sites; image E%d c%d, ours E%d c%d)' % (n, len(sites), ri.count('E'), ri.count('c'), ro.count('E'), ro.count('c')))
        print('  image %s' % ri)
        print('  ours  %s' % ro)
        if bad:
            pos = ['%d@%s/%s' % (q, '%x' % sites[q - 1][0][0] if sites[q - 1][0] else '-', '%x' % sites[q - 1][1][0] if sites[q - 1][1] else '-') for q in bad]
            print('  differ at site(image addr/our offset): ' + ' '.join(pos[:40]) + (' ...' if len(pos) > 40 else ''))


if __name__ == '__main__':
    main()
