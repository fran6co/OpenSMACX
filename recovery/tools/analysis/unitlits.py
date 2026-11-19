#!/usr/bin/env python3
"""unitlits.py <unit> [--src ALT.cpp] [-q] | --who LABEL|0xVA [-n N]

A unit's literal run against the image's (MATCHING_GUIDE 2.8, "A unit's literal run is its definition order").
Image side: every data-section string referenced from a function of this unit (markers; `// compile_context`
honoured), in address order, plus the unreferenced strings (`U`: labels of expanded rnd(b, "LABEL") calls and
other ignored inline arguments) whose nearest referenced neighbour belongs to the unit. Our side: the object's
$SG literals in .data order. Prints the difflib opcodes and `LITS <unit> image N ours M differing D`.
An insert/delete pair of the same strings inside one function is a source-order fact (arm order, a negated
test); strings only the image has are usually named objects shared by several functions.
--who prints the image's literal run around one label with the functions that reference each neighbour.
Screen only. FE_PFX_BASE selects the compiler prefix."""
import sys, os, re, struct, bisect, pickle, difflib
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE)); sys.path.insert(0, HERE)
import fe, litgap, giantcallee
R = fe.R
C = fe.SCR + '/unitlits_index.pkl'
def index():
    if os.path.exists(C):
        return pickle.load(open(C, 'rb'))
    img = litgap.image_strings()
    S = {a for a, _ in img}
    va, vs, ro = [s for s in fe._secs if s[0] == 0x401000][0]
    tb = fe._img[ro:ro + vs]
    refs = {}
    for i in range(len(tb) - 3):
        v = struct.unpack_from('<I', tb, i)[0]
        if v in S:
            refs.setdefault(v, []).append(va + i)
    pickle.dump((img, refs), open(C, 'wb'))
    return img, refs
img, refs = index()
mks, _ = giantcallee.load_markers()
mks.sort(key=lambda k: k.lo)
los = [k.lo for k in mks]
def who(a):
    i = bisect.bisect_right(los, a) - 1
    if i >= 0 and mks[i].lo <= a < mks[i].hi:
        k = mks[i]
        return '%s@%x[%s]' % (k.name, a, os.path.basename(k.path))
    return '?@%x' % a
def owner(a):
    i = bisect.bisect_right(los, a) - 1
    if i >= 0 and mks[i].lo <= a < mks[i].hi:
        return mks[i]
    return None
_T = {}
def unit_of(k):
    if k.path not in _T: _T[k.path] = open(k.path, encoding='latin-1').read()
    m = re.compile(r'// compile_context (\S+)').search(_T[k.path], k.pos, k.end)
    return m.group(1) if m else os.path.relpath(k.path, R)
_own = None
def image_seq(unit):
    global _own
    if _own is None:
        _own = []
        for a, s in img:
            us = set(); ns = []
            for r in refs.get(a, []):
                k = owner(r)
                if k: us.add(unit_of(k)); ns.append(k.name)
            _own.append((us, ns))
    own = _own
    seq = []
    n = len(img)
    for i, (a, s) in enumerate(img):
        us, ns = own[i]
        if not (0x680000 < a < 0x6a0000): continue
        if a in refs:
            if unit in us: seq.append((a, s, ns[0]))
        else:
            j = i - 1
            while j >= 0 and img[j][0] not in refs: j -= 1
            k = i + 1
            while k < n and img[k][0] not in refs: k += 1
            if j >= 0 and k < n and (unit in own[j][0] or unit in own[k][0]):
                seq.append((a, s, 'U'))
    return seq
def litdiff(unit, c, quiet=True):
    seq = image_seq(unit)
    ours = [s for _, s in litgap.our_strings(c)]
    A = [s for _, s, _ in seq]
    sm = difflib.SequenceMatcher(None, A, ours, autojunk=False)
    bad = 0; out = []
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == 'equal': continue
        bad += max(i2 - i1, j2 - j1)
        ctxb = seq[i1 - 1] if i1 else None
        out.append('%s after %s' % (tag, ('%x %r %s' % ctxb) if ctxb else '-'))
        for i in range(i1, i2): out.append('   IMG  %x %-24r %s' % seq[i])
        for j in range(j1, j2): out.append('   OURS %r' % ours[j][:30])
    return bad, len(A), len(ours), out
def who_cli(args):
    n = 6
    if '-n' in args:
        i = args.index('-n'); n = int(args[i + 1]); del args[i:i + 2]
    for q in args:
        idx = [i for i, (a, s) in enumerate(img) if (a == int(q, 16) if q.startswith('0x') else s == q)]
        for i in idx:
            print('==', q)
            for j in range(max(0, i - n), min(len(img), i + n + 1)):
                a, s = img[j]
                print(' %s %x %-22r %s' % ('>' if j == i else ' ', a, s[:22], ' '.join(who(r) for r in refs.get(a, [])[:6]) or 'U'))

if __name__ == '__main__':
    args = sys.argv[1:]
    if '--who' in args:
        args.remove('--who'); who_cli(args); sys.exit(0)
    q = '-q' in args
    if q: args.remove('-q')
    src = None
    if '--src' in args:
        i = args.index('--src'); src = args[i + 1]; del args[i:i + 2]
    unit = args[0] if args[0].startswith('src/') else 'src/' + args[0]
    c, err = fe.compile_unit(unit, src or R + '/' + unit, os.environ.get('FE_PFX_BASE', '2'))
    if c is None:
        print('COMPILE FAIL', err[:300]); sys.exit(1)
    bad, na, nb, out = litdiff(unit, c)
    if not q: print('\n'.join(out))
    print('LITS %s image %d ours %d differing %d' % (unit, na, nb, bad))
