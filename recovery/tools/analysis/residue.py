#!/usr/bin/env python3
"""residue.py --from FEALL.json [--max 15] [--min 1] [--json OUT] [--show] | <unit> <addr[,addr]> [--src ALT.cpp] [--show]
Residue classifier for near-misses (MATCHING_GUIDE 3.2 "Residue census"). Compiles each unit once (fe.py), groups the
differing rows of every selected function into windows (runs of `!!` rows, gaps of up to 2 equal rows joined) and
classifies each window by what separates the two sides:
  scratch   the same instructions under a renaming of eax/ecx/edx only (`rot+k`: one eax->ecx->edx rotation)
  saved     the same under a renaming of ebx/esi/edi/ebp only (callee-saved swap)
  regs      the same under a renaming that mixes scratch and callee-saved registers
  order     the same instructions in another order (`order+regs`: after a renaming)
  operand   same mnemonics and the same operands, paired differently (`a+b` loaded as `b+a`, swapped compare)
  reuse     the same instruction shapes, but one side reuses a register where the other takes a fresh one
            (mov eax,[G]; mov eax,[eax+K] against mov edx,[G]; mov eax,[edx+K]): no renaming maps the sides
  slot      same instructions except [ebp/esp +- N] displacements; imm: except other numbers
  bias      same instructions except [reg + K] displacements off by one delta per register (a biased pointer)
  addr-order  the same address with its terms in another order ([eax + edi + 1] / [edi + eax + 1])
  zero-reg  one side keeps 0 in a register (xor r,r; cmp x,r; mov [m],r), the other uses the immediate
  copy(img|ours)  that side copies a register (mov a, b) and continues with the copy: a named value there
  fold      a memory operand used in place on one side, loaded into a register first on the other
  type(sign) / type(width)  same rows except movsx/movzx, jl/jb, sar/shr.. or the operand width
  extra     ours has instructions the image lacks, the rest of the window equal; missing: the reverse
            (`+regs`/`+order` when the paired rest needs a renaming or a reorder)
  layout    a run one side has here and the other side has elsewhere in the function (moved block), an inverted
            branch, or rows that differ only in their jump target (`!>`)
  other     none of the above
The function's class is the sorted set of its windows' classes. It also prints the facts that bound the outside
levers: instruction count and frame size on both sides, the image's stack arguments (`ret N` for callee-cleanup,
highest [ebp+N] argument read), whether the image reads ecx before writing it (thiscall) against the symbol's
convention, and the image's direct callers with how many of them sit in BYTE_EXACT markers (an exact caller pushes
the image's arguments, so it confirms the parameter count and the pushed widths). Screen only; FE_PFX_BASE / FE_JOBS lanes."""
import sys, os, re, json, argparse, collections
R = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
sys.path.insert(0, R + '/recovery/tools')
import fe
from concurrent.futures import ThreadPoolExecutor

SCRATCH = ('eax', 'ecx', 'edx'); SAVED = ('ebx', 'esi', 'edi', 'ebp')
SUB = {}
for b, names in dict(eax='eax ax al ah', ecx='ecx cx cl ch', edx='edx dx dl dh', ebx='ebx bx bl bh',
                     esi='esi si', edi='edi di', ebp='ebp bp', esp='esp sp').items():
    for i, n in enumerate(names.split()):
        SUB[n] = (b, i)
REG = re.compile(r'\b(' + '|'.join(sorted(SUB, key=len, reverse=True)) + r')\b')
NUM = re.compile(r'\b(?:0x[0-9a-f]+|\d+)\b')
SLOT = re.compile(r'\[(ebp|esp) ([+-]) (0x[0-9a-f]+|\d+)\]')


def skel(ins):
    """instruction with registers replaced by width placeholders, plus the list of base registers in order"""
    regs = []

    def f(m):
        b, w = SUB[m.group(1)]
        if b == 'esp':
            return m.group(1)
        regs.append(b)
        return 'R%d' % w
    return REG.sub(f, ins), regs


def rename(I, C):
    """bijection of base registers that turns C's rows into I's rows pairwise, or None"""
    if len(I) != len(C):
        return None
    fw, bw = {}, {}
    for a, b in zip(I, C):
        sa, ra = skel(a); sb, rb = skel(b)
        if sa != sb or len(ra) != len(rb):
            return None
        for x, y in zip(ra, rb):
            if fw.setdefault(y, x) != x or bw.setdefault(x, y) != y:
                return None
    return {k: v for k, v in fw.items() if k != v}


def regclass(mp):
    moved = set(mp) | set(mp.values())
    if moved <= set(SCRATCH):
        for k in (1, 2):
            if all(SCRATCH[(SCRATCH.index(a) + k) % 3] == b for a, b in mp.items()):
                return 'scratch(rot+%d)' % k
        return 'scratch'
    if moved <= set(SAVED):
        return 'saved'
    return 'regs'


def anyrename(I, C):
    """multiset equality under some bijection: try the pairwise renaming of the sorted skeleton order"""
    if len(I) != len(C):
        return None
    ka = sorted(I, key=lambda s: skel(s)[0]); kb = sorted(C, key=lambda s: skel(s)[0])
    if [skel(s)[0] for s in ka] != [skel(s)[0] for s in kb]:
        return None
    import itertools
    regs = sorted({r for s in C for r in skel(s)[1]} | {r for s in I for r in skel(s)[1]})
    if len(regs) > 6:
        return None
    for perm in itertools.permutations(regs):
        mp = dict(zip(regs, perm))
        if sorted(apply(s, mp) for s in C) == sorted(I):
            return {k: v for k, v in mp.items() if k != v}
    return None


def apply(ins, mp):
    def f(m):
        b, w = SUB[m.group(1)]
        if b not in mp:
            return m.group(1)
        return ([n for n, (bb, ww) in SUB.items() if bb == mp[b] and ww == w] or ['?%s%d' % (mp[b], w)])[0]
    return REG.sub(f, ins)


def opers(ins):
    m, _, o = ins.partition(' ')
    return m, sorted(x.strip() for x in o.split(','))


JCC = dict(je='jne', jne='je', jl='jge', jge='jl', jg='jle', jle='jg', jb='jae', jae='jb', ja='jbe', jbe='ja',
           js='jns', jns='js')


SWAPJ = dict(jl='jg', jg='jl', jge='jle', jle='jge', jb='ja', ja='jb', jae='jbe', jbe='jae', je='je', jne='jne')
ADDR = re.compile(r'\[([^\]]+)\]')


def canon(ins):
    """address terms sorted: [eax + edi + 1] == [edi + eax + 1]"""
    def f(m):
        s = m.group(1).replace(' - ', ' + -')
        return '[' + ' + '.join(sorted(x.strip() for x in s.split(' + '))) + ']'
    return ADDR.sub(f, ins)


def zeroreg(I, C):
    """one side keeps zero in a register (xor r,r; cmp x,r; mov [m],r; push r), the other uses the immediate"""
    def norm(L):
        z = {m.group(1) for s in L for m in [re.match(r'xor (\w+), (\w+)$', s)] if m and m.group(1) == m.group(2)}
        out = []
        for s in L:
            m = re.match(r'xor (\w+), (\w+)$', s)
            if m and m.group(1) == m.group(2):
                continue
            m = re.match(r'test (\w+), (\w+)$', s)
            if m and m.group(1) == m.group(2):
                s = 'cmp %s, 0' % m.group(1)
            out.append(s)
        return out, z
    a, za = norm(I); b, zb = norm(C)
    if len(a) != len(b) or a == b and not (za or zb):
        return False
    zr = None
    for x, y in zip(a, b):
        if x == y:
            continue
        ok = False
        for p, q in ((x, y), (y, x)):
            m = re.match(r'(.*[ ,])(e[a-ds][xip])$', p)
            if m and q == m.group(1) + '0' and zr in (None, m.group(2)):
                zr = m.group(2); ok = True
        if not ok:
            return False
    return True


def cmpswap(I, C):
    """cmp a,b / jcc against cmp b,a / swapped jcc"""
    if len(I) != len(C):
        return False
    hit = False; i = 0
    while i < len(I):
        if I[i] == C[i]:
            i += 1; continue
        ma = re.match(r'cmp (.+), (.+)$', I[i]); mb = re.match(r'cmp (.+), (.+)$', C[i])
        if ma and mb and ma.groups() == mb.groups()[::-1] and i + 1 < len(I):
            ja, jb = I[i + 1].split()[0], C[i + 1].split()[0]
            if SWAPJ.get(ja) == jb or JCC.get(SWAPJ.get(ja, ''), '') == jb:
                hit = True; i += 2; continue
        return False
    return hit


def fold(I, C):
    """a memory operand folded into the operation on one side, loaded into a register first on the other"""
    if abs(len(I) - len(C)) != 1:
        return False
    L, S = (I, C) if len(I) > len(C) else (C, I)
    mem = lambda X: sorted(m for s in X for m in ADDR.findall(canon(s)))
    mn = lambda X: collections.Counter(s.split()[0] for s in X)
    d = mn(L) - mn(S)
    return mem(L) == mem(S) and sum(d.values()) == 1 and list(d)[0] == 'mov' and not (mn(S) - mn(L))


SIGN = dict(movzx='movsx', jb='jl', ja='jg', jae='jge', jbe='jle', shr='sar', div='idiv', mul='imul')


def signless(s):
    m, _, o = s.partition(' ')
    return SIGN.get(m, m) + ' ' + o


def widthless(s):
    s = re.sub(r'\b(byte|word|dword) ptr', 'ptr', s)
    s = REG.sub(lambda m: SUB[m.group(1)][0], s)
    return re.sub(r'^(movsx|movzx) ', 'mov ', s)


_CALLERS = None


def callers(addr):
    """image call sites of addr -> (number of callers, how many sit in BYTE_EXACT markers)"""
    global _CALLERS
    if _CALLERS is None:
        spans = []
        for dp, dn, fn in os.walk(R + '/src'):
            for f in fn:
                if f.endswith(('.cpp', '.h')):
                    for m in re.finditer(r'// ORIGINAL: 0x([0-9A-F]{8}) \S+ 0x([0-9A-Fa-f]+)-0x([0-9A-Fa-f]+)([^\n]*)', open(os.path.join(dp, f), encoding='latin-1').read()):
                        spans.append((int(m.group(2), 16), int(m.group(3), 16), int(m.group(1), 16), 'BYTE_EXACT' in m.group(4)))
        spans.sort(); starts = {s[2] for s in spans}
        import bisect, struct
        keys = [s[0] for s in spans]; _CALLERS = collections.defaultdict(list)
        for va, vs, ro in fe._secs[:1]:
            data = fe._img[ro:ro + vs]; i = data.find(b'\xe8')
            while i >= 0 and i + 5 <= len(data):
                tgt = (va + i + 5 + struct.unpack_from('<i', data, i + 1)[0]) & 0xffffffff
                if tgt in starts:
                    k = bisect.bisect_right(keys, va + i) - 1
                    if k >= 0 and spans[k][0] <= va + i < spans[k][1]:
                        _CALLERS[tgt].append((spans[k][2], spans[k][3]))
                i = data.find(b'\xe8', i + 1)
    cs = _CALLERS.get(addr, [])
    return len(cs), sum(1 for a, ex in cs if ex)


def copy(I, C):
    """one side copies a register (mov a, b) and goes on with the copy: a named value on that side"""
    if abs(len(I) - len(C)) != 1:
        return None
    L, S = (I, C) if len(I) > len(C) else (C, I)
    for i, s in enumerate(L):
        m = re.match(r'mov (e[a-ds][xip]), (e[a-ds][xip])$', s)
        if not m:
            continue
        a, b = m.groups()
        for sub in ({a: b}, {b: a}):
            L2 = L[:i] + [apply(x, sub) for x in L[i + 1:]]
            if L2 == S or rename(L2, S) is not None or sorted(L2) == sorted(S):
                return 'copy(img)' if L is I else 'copy(ours)'
    return None


def epilogue(X):
    return any(s.startswith('ret') for s in X) and sum(1 for s in X if s.startswith(('pop', 'ret', 'mov esp', 'add esp', 'xor eax', 'mov eax', 'j'))) >= len(X) - 1


def core(I, C):
    """class of a window whose two sides have the same length"""
    if not I and not C:
        return 'same'
    if I == C:
        return 'same'
    mp = rename(I, C)
    if mp:
        return regclass(mp)
    if sorted(I) == sorted(C):
        return 'order'
    if [canon(x) for x in I] == [canon(x) for x in C]:
        return 'addr-order'
    if zeroreg(I, C):
        return 'zero-reg'
    if cmpswap(I, C):
        return 'operand'
    if len(I) == len(C) and all(a == b or signless(a) == signless(b) for a, b in zip(I, C)):
        return 'type(sign)'
    if len(I) == len(C) and all(a == b or widthless(a) == widthless(b) for a, b in zip(I, C)):
        return 'type(width)'
    r = copy(I, C)
    if r:
        return r
    if fold(I, C):
        return 'fold'
    if len(I) == len(C):
        if all(SLOT.sub('[S]', a) == SLOT.sub('[S]', b) for a, b in zip(I, C)):
            return 'slot'
        if bias(I, C):
            return 'bias'
        if all(NUM.sub('N', a) == NUM.sub('N', b) for a, b in zip(I, C)):
            return 'imm'
        if all(a == b or (a.split()[0] in JCC and JCC[a.split()[0]] == b.split()[0]) for a, b in zip(I, C)):
            return 'layout'
        if sorted(opers(a) for a in I) == sorted(opers(b) for b in C):
            return 'operand'
        fa = collections.Counter(x for a in I for x in [a.split()[0]] + opers(a)[1])
        fb = collections.Counter(x for b in C for x in [b.split()[0]] + opers(b)[1])
        if fa == fb:
            return 'operand'
        mp = anyrename(I, C)
        if mp:
            return 'order+' + regclass(mp)
        # slots and a renaming together
        IS = [SLOT.sub('[S]', a) for a in I]; CS = [SLOT.sub('[S]', b) for b in C]
        mp = rename(IS, CS)
        if mp is not None:
            return 'slot+' + regclass(mp) if mp else 'slot'
        if all(skel(a)[0] == skel(b)[0] for a, b in zip(I, C)):
            return 'reuse'
    return None


DISP = re.compile(r'\[(e[a-ds][xip])(?: ([+-]) (0x[0-9a-f]+|\d+))?\]')


def bias(I, C):
    """the same rows except [reg + K] displacements that differ by one delta per base register: a loop or object
    pointer that is biased differently"""
    delta = {}; hit = False
    for a, b in zip(I, C):
        if a == b:
            continue
        ma = list(DISP.finditer(a)); mb = list(DISP.finditer(b))
        if len(ma) != len(mb) or not ma or DISP.sub('[D]', a) != DISP.sub('[D]', b):
            return False
        for x, y in zip(ma, mb):
            if x.group(1) != y.group(1):
                return False
            kx = int(x.group(3), 0) * (-1 if x.group(2) == '-' else 1) if x.group(3) else 0
            ky = int(y.group(3), 0) * (-1 if y.group(2) == '-' else 1) if y.group(3) else 0
            if kx != ky:
                if x.group(1) in ('ebp', 'esp') or delta.setdefault(x.group(1), kx - ky) != kx - ky:
                    return False
                hit = True
    return hit


def window(rows):
    """rows: [(k, o, c)] of one window -> (class, detail)"""
    I = [o[1] for k, o, c in rows if o is not None and k != '  ']
    C = [c[1] for k, o, c in rows if c is not None and k != '  ']
    if all(k == '!>' for k, o, c in rows if k != '  '):
        return 'layout', 'jump targets'
    I = [o[1] for k, o, c in rows if o is not None]
    C = [c[1] for k, o, c in rows if c is not None]
    r = core(I, C)
    if r:
        return r, ''
    I = [o[1] for k, o, c in rows if o is not None and k == '!!']
    C = [c[1] for k, o, c in rows if c is not None and k == '!!']
    r = core(I, C)
    if r and r != 'same':
        return r, ''
    io = [o[1] for k, o, c in rows if o is not None and c is None]
    co = [c[1] for k, o, c in rows if c is not None and o is None]
    if (io and not co and epilogue(io)) or (co and not io and epilogue(co)):
        return 'layout', 'return block placed here on one side'
    # unequal lengths (or nothing fits): drop the unpaired instructions through an LCS over skeleton-free text
    ops = fe.align([skel(x)[0] for x in I], [skel(x)[0] for x in C])
    io = [I[i] for t, i1, i2, j1, j2 in ops if t != 'equal' for i in range(i1, i2)]
    co = [C[j] for t, i1, i2, j1, j2 in ops if t != 'equal' for j in range(j1, j2)]
    ip = [I[i] for t, i1, i2, j1, j2 in ops if t == 'equal' for i in range(i1, i2)]
    cp = [C[j] for t, i1, i2, j1, j2 in ops if t == 'equal' for j in range(j1, j2)]
    rest = core(ip, cp)
    if rest and (io or co) and not (io and co):
        tag = ('missing(%d)' % len(io)) if io else ('extra(%d)' % len(co))
        return tag + ('' if rest == 'same' else '+' + rest), '; '.join(io or co)
    if len(I) != len(C):
        # the longer side's surplus as a multiset
        ca, cb = collections.Counter(I), collections.Counter(C)
        if not (cb - ca) and (ca - cb):
            return 'missing(%d)+order' % sum((ca - cb).values()), '; '.join((ca - cb).elements())
        if not (ca - cb) and (cb - ca):
            return 'extra(%d)+order' % sum((cb - ca).values()), '; '.join((cb - ca).elements())
    return 'other', ''


def frame(D):
    """frame size from a prologue (sub esp,N / push ecx / __chkstk), listing = [(off, text, size, tgt)]"""
    for off, t, sz, tg in D[:14]:
        m = re.match(r'sub esp, (0x[0-9a-f]+|\d+)', t)
        if m:
            return int(m.group(1), 0)
        m = re.match(r'mov eax, (0x[0-9a-f]+|\d+)', t)
        if m and int(m.group(1), 0) >= 0x1000:
            return int(m.group(1), 0)
        if t.startswith('call') or t.startswith('j'):
            break
    return 4 if any(t == 'push ecx' for off, t, sz, tg in D[:4]) and D[0][1] == 'push ebp' and D[2][1] == 'push ecx' else 0


def sigfacts(sym, O):
    """image-side facts about the signature: ret N, highest stack argument read, ecx read before written"""
    f = {}
    rets = {t for off, t, sz, tg in O if t.startswith('ret')}
    n = [int(t.split()[1], 0) for t in rets if len(t.split()) > 1]
    f['retn'] = n[0] if n else 0
    ebpf = len(O) > 1 and O[0][1] == 'push ebp' and O[1][1] == 'mov ebp, esp'
    hi = 0
    if ebpf:
        for off, t, sz, tg in O:
            for m in re.finditer(r'\[ebp \+ (0x[0-9a-f]+|\d+)\]', t):
                hi = max(hi, int(m.group(1), 0))
        f['args_read'] = max(0, (hi - 4) // 4)
    ecx = None
    for i, (off, t, sz, tg) in enumerate(O):
        if t.startswith(('call', 'j', 'ret')):
            break
        if t == 'push ecx' and i and O[i - 1][1] == 'mov ebp, esp':
            continue   # a 4-byte frame
        m = re.match(r'(\w+) ([^,]+)(?:, (.+))?', t)
        if not m:
            continue
        mn, a, b = m.groups()
        rd = (b or '') + ' ' + (a if mn in ('push', 'cmp', 'test', 'add', 'sub', 'and', 'or') or '[' in a else '')
        if mn == 'xor' and a == b:
            rd = ''
        if re.search(r'\b(ecx|cx|cl|ch)\b', rd):
            ecx = 'read'; break
        if re.search(r'\b(ecx|cx|cl)\b', a) and mn in ('mov', 'lea', 'xor', 'pop', 'movsx', 'movzx'):
            ecx = 'written'; break
    f['ecx'] = ecx
    m = re.match(r'\?[^@]*(?:@[^@]+)*@@(?:([QUAISE]|[A-Z])([A-Z])([A-Z])?)', sym)
    conv = None
    mm = re.search(r'@@([A-Z])([A-Z])(E?)', sym)
    if sym.startswith('?') and mm:
        a, b, e = mm.groups()
        if a == 'Y':
            conv = {'A': 'cdecl', 'G': 'stdcall', 'I': 'fastcall'}.get(b, b)
        elif a in 'SKC':
            conv = 'static ' + {'A': 'cdecl', 'G': 'stdcall', 'I': 'fastcall'}.get(b, b)
        elif e == 'E':
            conv = 'thiscall'
        else:
            conv = {'A': 'member cdecl', 'G': 'member stdcall'}.get(b, a + b)
    f['conv'] = conv
    notes = []
    if conv == 'thiscall' and ecx == 'written':
        notes.append('this-unused(static?)')
    if conv in ('cdecl', 'static cdecl') and ecx == 'read':
        notes.append('ecx-read(thiscall/fastcall?)')
    if conv in ('cdecl', 'static cdecl', 'member cdecl') and f['retn']:
        notes.append('ret %d on a cdecl symbol' % f['retn'])
    f['notes'] = notes
    return f


def classify(res):
    rows = res['rows']; wins = []; cur = None; gap = 0
    for i, r in enumerate(rows):
        if r[0] != '  ':
            if cur is None:
                cur = [i, i]
            else:
                cur[1] = i
            gap = 0
        elif cur is not None:
            gap += 1
            if gap > 2:
                wins.append(tuple(cur)); cur = None
    if cur is not None:
        wins.append(tuple(cur))
    out = []
    for a, b in wins:
        w = rows[a:b + 1]
        c, d = window(w)
        o0 = next((o for k, o, c_ in w if o is not None), None)
        out.append(dict(at=res['lo'] + o0[0] if o0 else None, rows=sum(1 for k, o, c_ in w if k != '  '), cls=c, detail=d,
                        img=[o[1] for k, o, c_ in w if o is not None and k != '  '],
                        ours=[c_[1] for k, o, c_ in w if c_ is not None and k != '  '],
                        text=['%s %-44s | %s' % (k, o[1] if o else '', c_[1] if c_ else '') for k, o, c_ in w]))
    # moved blocks: an 'other'/missing window whose image-only rows are ours-only rows of another window
    unp_i = collections.Counter(); unp_c = collections.Counter()
    for w in out:
        unp_i.update(w['img']); unp_c.update(w['ours'])
    if len(out) > 1 and unp_i == unp_c and any(w['cls'] in ('other',) or w['cls'].startswith(('missing', 'extra')) for w in out):
        for w in out:
            if w['cls'] == 'other' or w['cls'].startswith(('missing', 'extra')):
                w['cls'] = 'layout'; w['detail'] = 'moved block'
    return out


def facts(res, O, C):
    f = sigfacts(res['sym'], O)
    f.update(img=res['img'], ours=res['ours'], frame_img=frame(O), frame_ours=frame(C))
    f['callers'], f['exact_callers'] = callers(res['lo'])
    return f


def run_unit(unit, addrs, pfx, src=None):
    path = os.path.abspath(src) if src else R + '/' + unit
    text = open(path, encoding='latin-1').read()
    c, err = fe.compile_unit(unit, path, pfx)
    if c is None:
        c, err = fe.compile_unit(unit, path, pfx)   # transient wine failures
    out = []
    if c is None:
        return [dict(unit=unit, addr='%08X' % a, error='compile') for a in addrs]
    ctx = {}
    for a in addrs:
        cc = c
        m = re.search(r'// ORIGINAL: 0x%08X [^\n]*\n((?://(?! ORIGINAL)[^\n]*\n)*)' % a, text)
        mc = m and re.search(r'// compile_context (\S+)', m.group(1))
        if mc and mc.group(1) != unit and not src:
            if mc.group(1) not in ctx:
                ctx[mc.group(1)] = fe.compile_unit(mc.group(1), R + '/' + mc.group(1), pfx)[0]
            cc = ctx[mc.group(1)] or c
        r = fe.evaluate(cc, text, a)
        if 'error' in r:
            out.append(dict(unit=unit, addr='%08X' % a, error=r['error'])); continue
        O = [o for k, o, c_ in r['rows'] if o is not None]; C = [c_ for k, o, c_ in r['rows'] if c_ is not None]
        w = classify(r)
        cls = sorted({x['cls'] for x in w})
        out.append(dict(unit=unit, addr='%08X' % a, sym=r['sym'], diff=r['diff'], exact=r['exact'], cls=cls, windows=w,
                        facts=facts(r, O, C)))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('unit', nargs='?'); ap.add_argument('addr', nargs='?')
    ap.add_argument('--from', dest='frm'); ap.add_argument('--max', type=int, default=15); ap.add_argument('--min', type=int, default=1)
    ap.add_argument('--src'); ap.add_argument('--json'); ap.add_argument('--show', action='store_true')
    a = ap.parse_args()
    base = int(os.environ.get('FE_PFX_BASE', '21')); jobs = int(os.environ.get('FE_JOBS', '5'))
    work = collections.OrderedDict()
    if a.frm:
        for x in json.load(open(a.frm)):
            if not x['exact'] and a.min <= x['diff'] <= a.max:
                work.setdefault(x['unit'], []).append(int(x['addr'], 16))
    else:
        u = a.unit if a.unit.startswith('src/') else 'src/' + a.unit
        work[u] = [int(x, 16) for x in a.addr.split(',')]
    units = list(work)

    def lane(k):
        o = []
        for i in range(k, len(units), jobs):
            o += run_unit(units[i], work[units[i]], str(base + k), a.src)
        return o
    res = []
    with ThreadPoolExecutor(jobs) as ex:
        for o in ex.map(lane, range(jobs)):
            res += o
    res.sort(key=lambda x: (x.get('diff', 0), x['addr']))
    tally = collections.Counter()
    for x in res:
        if 'error' in x:
            print(x['addr'], x['unit'], 'ERROR', x['error']); continue
        if x['exact']:
            print(x['addr'], x['unit'], 'EXACT'); continue
        f = x['facts']
        tally['|'.join(x['cls'])] += 1
        print('%s %3d %-22s %-34s %s  n %d/%d frame %d/%d callers %d (%d exact) %s' % (
            x['addr'], x['diff'], os.path.basename(x['unit']), x['sym'][:34], ' '.join(x['cls']), f['img'], f['ours'],
            f['frame_img'], f['frame_ours'], f['callers'], f['exact_callers'], ' '.join(f['notes'])))
        if a.show:
            for w in x['windows']:
                print('    @%X %-18s %d rows %s' % (w['at'] or 0, w['cls'], w['rows'], w['detail']))
                for l in w['text']:
                    print('        ' + l)
    print('--- classes')
    for k, v in tally.most_common():
        print('%4d %s' % (v, k))
    if a.json:
        json.dump(res, open(a.json, 'w'), indent=0)


if __name__ == '__main__':
    main()
