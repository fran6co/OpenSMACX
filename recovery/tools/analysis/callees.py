#!/usr/bin/env python3
"""callees.py <unit> <0xADDR> [--src ALT.cpp] [--pfx N]
Named call multisets for one function: the image's direct calls are resolved through the source's
ORIGINAL markers (and `// symbol` overrides), its `call [IAT]` through the image's import directory;
ours through the object's relocations. Prints the callees on one side only (name, image count, our count).
`named_diff(c, text, addr)` returns that symmetric difference as one number (armwo's call key);
indirect calls through a register or member pointer count under one name, `<indirect>`.
Image calls to a function without a marker keep a `sub_<addr>` name, so they never match our side: they
add a constant to every variant of a function and do not change the ranking."""
import sys, os, re, glob, struct, collections, argparse
H = os.path.dirname(os.path.abspath(__file__))
T = os.path.dirname(H)
sys.path.insert(0, T)
import fe

CRT = {0x645470: '_strcat', 0x645460: '_strcpy', 0x64601d: '_rand', 0x6453e0: '_strlen', 0x644f3a: '_abs',
       0x64fc88: '__itoa', 0x644ef2: '_free', 0x644dff: '_exit', 0x645660: '_strcmp', 0x645550: '__chkstk',
       0x64558a: '??2@YAPAXI@Z', 0x6458c8: '__ftol', 0x647851: '_floor', 0x645da7: '_atoi', 0x645dd0: '_strchr',
       0x6468d0: '_strstr', 0x645e90: '_strncpy', 0x6465f0: '_memset', 0x645930: '_memcpy',
       # learned from aligned call sequences over the non-exact functions (2026-11-19)
       0x646178: '_fread', 0x64603f: '_fwrite', 0x6463e4: '_sin', 0x646494: '_cos', 0x6456e4: '??_M@YGXPAXIHP6EX0@Z@Z',
       0x6457c2: '??_L@YGXPAXIHP6EX0@Z1@Z', 0x6458a0: '_strrchr', 0x64fdf0: '__strnicmp', 0x6470a6: '_malloc',
       0x6474e0: '_memmove', 0x645398: '_atexit', 0x6466ef: '_tolower', 0x646ed0: '_calloc', 0x64fd20: '__stricmp',
       0x646544: '_sqrt', 0x6458f0: '_strncmp', 0x646f5d: '__beginthread', 0x64628f: '_toupper', 0x650231: '__hypot',
       0x6473f0: '_memchr', 0x64557f: '??3@YAXPAX@Z', 0x6501f0: '_DirectSoundCreate@12', 0x647067: '__endthread',
       0x646829: '__mkdir', 0x634280: '_compress', 0x634330: '_uncompress', 0x5d44fc: '_DirectDrawCreate@12',
       0x6474a0: '_strcspn', 0x644dc6: '_ICSendMessage@16', 0x64697a: '_isupper', 0x645c70: '_memcmp'}
ALIAS = {'__strcmpi': '__stricmp'}

_A2S = None
_IAT = None


def addr2sym():
    global _A2S
    if _A2S is None:
        _A2S = {}
        for p in glob.glob(fe.R + '/src/**/*.cpp', recursive=True) + glob.glob(fe.R + '/src/**/*.h', recursive=True):
            t = open(p, encoding='latin-1').read()
            for m in re.finditer(r'// ORIGINAL: 0x([0-9A-F]{8}) (\S+) 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+[^\n]*\n((?://(?! ORIGINAL:)[^\n]*\n)*)', t):
                m2 = re.search(r'// symbol\s+(\S+)', m.group(3))
                _A2S[int(m.group(1), 16)] = m2.group(1) if m2 else m.group(2)
        for a, n in CRT.items():
            _A2S.setdefault(a, n)
    return _A2S


def iat():
    """IAT slot address -> imported name (by name or '#ordinal')."""
    global _IAT
    if _IAT is None:
        _IAT = {}
        img = fe._img
        pe = fe._pe
        opt = pe + 24
        imp_rva = struct.unpack_from('<I', img, opt + 96 + 8)[0]

        def off(rva):
            for va, vs, ro in fe._secs:
                if va - fe._base <= rva < va - fe._base + max(vs, 1):
                    return ro + rva - (va - fe._base)
            return None
        d = off(imp_rva)
        while d is not None:
            olt, _, _, name, ft = struct.unpack_from('<IIIII', img, d)
            if not ft:
                break
            src = off(olt or ft)
            k = 0
            while True:
                e = struct.unpack_from('<I', img, src + 4 * k)[0]
                if not e:
                    break
                if e & 0x80000000:
                    nm = '#%d' % (e & 0xFFFF)
                else:
                    o = off(e) + 2
                    nm = img[o:img.index(b'\0', o)].decode('latin-1')
                _IAT[fe._base + ft + 4 * k] = nm
                k += 1
            d += 20
    return _IAT


def norm_ours(name):
    if name.startswith('__imp_'):
        name = name[6:]
        if name.startswith('_'):
            name = name[1:]
        return re.sub(r'@\d+$', '', name)
    return ALIAS.get(name, name)


def image_calls(lo, hi):
    a2s, imp = addr2sym(), iat()
    out = collections.Counter()
    for ins in fe.md.disasm(fe.imgbytes(lo, hi), lo):
        if ins.mnemonic != 'call':
            continue
        b = ins.bytes
        if b[0] == 0xE8:
            t = int(ins.op_str, 16)
            if lo <= t < hi:
                continue
            out[a2s.get(t, 'sub_%x' % t)] += 1
        elif b[0] == 0xFF and b[1] == 0x15:
            slot = struct.unpack_from('<I', b, 2)[0]
            out[imp.get(slot, a2s.get(slot, 'iat_%x' % slot))] += 1
        else:
            out['<indirect>'] += 1
    return out


def our_calls(c, sym):
    s = c.function(sym)
    sec = c.sections[s.section - 1]
    rm = {r[0]: r for r in sec.relocations}
    out = collections.Counter()
    # the body ends where fe.dis ends it (a switch table may follow the code)
    body = fe.dis(sec.data, 0, sec.relocations, selfsec=s.section, syms=c.symbols)
    end = body[-1][0] + body[-1][2] if body else len(sec.data)
    for ins in fe.md.disasm(sec.data[:end], 0):
        if ins.mnemonic != 'call':
            continue
        b = ins.bytes
        at = ins.address + (1 if b[0] == 0xE8 else 2)
        if at in rm and (b[0] == 0xE8 or (b[0] == 0xFF and b[1] == 0x15)):
            sy = c.symbols.get(rm[at][1])
            out[norm_ours(sy.name) if sy else '?'] += 1
        elif b[0] == 0xE8:
            continue  # a local call
        else:
            out['<indirect>'] += 1
    return out


def diff(c, text, addr):
    idn = fe.ident(text, addr)
    if not idn:
        return None
    sym, lo, hi = idn
    try:
        co = our_calls(c, sym)
    except Exception:
        return None
    ci = image_calls(lo, hi)
    return ci, co


def named_diff(c, text, addr):
    d = diff(c, text, addr)
    if d is None:
        return None
    ci, co = d
    return sum(((ci - co) + (co - ci)).values())


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('unit')
    ap.add_argument('addr')
    ap.add_argument('--src')
    ap.add_argument('--pfx', default=os.environ.get('FE_PFX_BASE', '8'))
    a = ap.parse_args()
    unit = a.unit if a.unit.startswith('src/') else 'src/' + a.unit
    src = a.src or fe.R + '/' + unit
    text = open(src, encoding='latin-1').read()
    c, err = fe.compile_unit(unit, src, a.pfx)
    if c is None:
        print('COMPILE', err)
        return 2
    d = diff(c, text, int(a.addr, 16))
    if d is None:
        print('no symbol')
        return 2
    ci, co = d
    n = 0
    for k in sorted(set(ci) | set(co)):
        if ci[k] != co[k]:
            n += abs(ci[k] - co[k])
            print('%-50s img %d ours %d' % (k[:50], ci[k], co[k]))
    print('NAMED', n, 'img', sum(ci.values()), 'ours', sum(co.values()))
    return 0


if __name__ == '__main__':
    sys.exit(main())
