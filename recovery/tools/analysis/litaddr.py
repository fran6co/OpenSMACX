#!/usr/bin/env python3
"""litaddr.py FE.json [--json OUT] [--shared] : literal-address evidence (MATCHING_GUIDE 2.8, "A pushed literal's
address decides inline against written out").

FE.json is a feall.py --out list (any subset of {unit, addr}). Per function, in code order:
  image: every immediate (push/mov/lea/cmp operand, never a call/jmp target) that is a separator literal of the shared
         header-inline family (" " 0x00682820, ", " ": " ")" "(" "%" 0x00682E90.., ". " 0x006867E0) -> S, or a unit-local
         copy of the same text -> L;
  ours:  each relocation to a ??_C@ COMDAT literal (an inline body's own text) -> S, a $SG literal (written at the
         site, or passed as an argument) -> L, one of the named stand-in arrays (BufferJoinSpace, SocialWin*Text) -> N.
Prints the functions whose sequences differ. An image S against our L is a written-out header inline; an image L
against our S is an inline (or helper) that the original wrote out; N marks a named stand-in.
--shared: also every other literal whose address the image references from functions of two or more units (a
COMDAT of an inline body, or a named global: read the instruction), compared as S/L counts per text. Data reads
that happen to point at a string ("mov eax, [addr]") show up here too; check them. Lanes FE_PFX_BASE/FE_JOBS."""
import sys, os, json, struct, argparse, collections
from concurrent.futures import ThreadPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE)); sys.path.insert(0, HERE)
import fe, litgap  # noqa: E402
R = fe.R
SH = {0x00682820: ' ', 0x00682E90: ', ', 0x00682E94: ': ', 0x00682E98: ')', 0x00682E9C: '(', 0x00682EA0: '%',
      0x006867E0: '. '}
TX = set(SH.values())
NAMED = {'BufferJoinSpace': ' ', 'SocialWinCommaSpaceText': ', ', 'SocialWinColonSpaceText': ': ',
         'SocialWinCloseParenText': ')', 'SocialWinOpenParenText': '('}
_LIT = None


def lits():
    global _LIT
    if _LIT is None:
        _LIT = {a: s for a, s in litgap.image_strings() if 0x680000 < a < 0x6a0000}
    return _LIT


def imgrefs(lo, hi):
    L = lits(); out = []
    for ins in fe.md.disasm(fe.imgbytes(lo, hi), lo):
        if ins.mnemonic == 'call' or ins.mnemonic.startswith('j'):
            continue
        for k in range(ins.size - 3):
            v = struct.unpack_from('<I', ins.bytes, k)[0]
            if v in L:
                out.append((v, ins.address))
    return out


def cstr(data, off):
    return data[off:data.find(b'\0', off)].decode('latin-1')


def ourrefs(c, sym):
    s = c.function(sym); sec = c.sections[s.section - 1]; out = []
    for off, si, ty in sorted(sec.relocations):
        y = c.symbols.get(si)
        if y is None:
            continue
        base = y.name.lstrip('?_').split('@')[0]
        if base in NAMED:
            out.append((NAMED[base], 'N')); continue
        if not y.section or y.section < 1:
            continue
        if y.name.startswith('??_C@'):
            out.append((cstr(c.sections[y.section - 1].data, y.value), 'S'))
        elif y.name.startswith('$SG'):
            out.append((cstr(c.sections[y.section - 1].data, y.value), 'L'))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('fe'); ap.add_argument('--json'); ap.add_argument('--shared', action='store_true')
    a = ap.parse_args()
    d = json.load(open(a.fe)); byu = {}
    for x in d:
        byu.setdefault(x['unit'], []).append(int(x['addr'], 16))
    kind = {}
    if a.shared:
        sys.path.insert(0, HERE); import tumap
        un = collections.defaultdict(set)
        for f in tumap.load():
            for v, _ in imgrefs(f[0], f[1]):
                if not f[2].endswith('.h'):
                    un[v].add(f[2])
        kind = {v: 'S' if len(us) >= 2 else 'L' for v, us in un.items()}
    base = int(os.environ.get('FE_PFX_BASE', '21')); J = int(os.environ.get('FE_JOBS', '5'))
    jobs = list(byu.items()); fe.compile_cmd('src/x.cpp'); res = {}
    L = lits()

    def unit(u, al, pfx):
        c, err = fe.compile_unit(u, R + '/' + u, pfx)
        if c is None:
            print('ERR', u, err[:200]); return
        text = open(R + '/' + u, encoding='latin-1').read()
        for ad in al:
            idn = fe.ident(text, ad)
            if not idn:
                continue
            sym, lo, hi = idn
            try:
                o = ourrefs(c, sym)
            except Exception:
                continue
            ir = imgrefs(lo, hi)
            img = [(SH[v], 'S') if v in SH else (L[v], 'L') for v, _ in ir if v in SH or L[v] in TX]
            ours = [x for x in o if x[0] in TX]
            r = dict(unit=u, sym=sym, img=img, ours=ours, at=['%X' % w for v, w in ir if v in SH or L[v] in TX])
            if a.shared:
                ic = collections.Counter((L[v], kind.get(v, 'L')) for v, _ in ir if L[v] not in TX)
                oc = collections.Counter(x for x in o if x[0] not in TX)
                r['shared'] = [(t, ic[(t, 'S')], ic[(t, 'L')], oc[(t, 'S')], oc[(t, 'L')])
                               for t in sorted({t for t, _ in ic} & {t for t, _ in oc})
                               if (ic[(t, 'S')] and oc[(t, 'L')]) or (ic[(t, 'L')] and oc[(t, 'S')])]
            res['%08X' % ad] = r

    def lane(k):
        for u, al in jobs[k::J]:
            unit(u, al, base + k)
    with ThreadPoolExecutor(J) as ex:
        list(ex.map(lane, range(J)))
    n = 0; fmt = lambda q: ' '.join('%s%r' % (k, t) for t, k in q)
    for ad, r in sorted(res.items()):
        if r['img'] != r['ours'] or r.get('shared'):
            n += 1
            print('%s %-40s %s' % (ad, r['sym'][:40], r['unit']))
            if r['img'] != r['ours']:
                print('   img : %s\n   ours: %s' % (fmt(r['img']), fmt(r['ours'])))
            for t, a1, a2, b1, b2 in r.get('shared', []):
                print('   %r image S%d/L%d ours S%d/L%d' % (t, a1, a2, b1, b2))
    print(n, 'functions differ')
    if a.json:
        json.dump(res, open(a.json, 'w'), indent=0)


if __name__ == '__main__':
    main()
