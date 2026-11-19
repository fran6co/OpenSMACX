#!/usr/bin/env python3
"""behavscan.py <unit> <0xADDR[,..]> [--src ALT] [--pfx N] [-v] | --tree [unit ...] [--from FEALL.json] [--json F] [--top N]
Behaviour-difference screen (MATCHING_GUIDE 3.7 "an image jump that skips a call ours always makes is a
behaviour bug, not layout"): a transcription error is a call or store that runs under other conditions than
the image's, or a test with another operator or constant. Layout differences (block order, inverted jcc, tail
merging, register choice) must not show, so nothing here compares positions or registers:

  guards  every named call (image: ORIGINAL markers, IAT; ours: relocations) and every store to a field
          `[reg+disp]` (reg not ebp/esp) or to a global (image: RECOVERY_OBJECT + offset; ours: relocation
          symbol + addend) gets the set of predicates it is control dependent on, transitively (post-dominator
          CDG over the function's CFG, jump tables included). A predicate is (relation, constant, truth) from the
          flag setter before the jcc, canonicalised so that `jle 7` = `jl 8` and `jge K` = not `jl K`; reg-reg
          compares read as (rel, x). Calls/stores of one key are paired between the sides by minimum guard
          difference, and each pair whose guard sets differ is printed:
            INVERT   the same predicate with the opposite truth (the call sits in the other arm)
            CONST    the same relation and truth with another constant
            OP       the same constant and truth, another relation (`<` against `<=`, signed against unsigned)
            IMG-ONLY / OURS-ONLY  a predicate one side's site depends on and the other's does not
  calls   named callees whose counts differ (a missing or extra call)
  tests   in fe's differing stretches, the rows aligned by register-free skeleton: cmp/test with another
          immediate, a jcc with another relation (truth ignored: an inverted jcc is layout)

The score ranks INVERT, CONST and OP above one-side predicates, and calls above stores. Loops and switch
lowering add noise (a guard plus bottom test, subtract chains): read every hit against the image."""
import sys, os, re, json, struct, collections, argparse, subprocess, difflib
H = os.path.dirname(os.path.abspath(__file__))
T = os.path.dirname(H)
sys.path.insert(0, T)
sys.path.insert(0, H)
import fe
import callees as CL

NEG = {'e': 'ne', 'ne': 'e', 'l': 'ge', 'ge': 'l', 'le': 'g', 'g': 'le', 'b': 'ae', 'ae': 'b',
       'be': 'a', 'a': 'be', 's': 'ns', 'ns': 's', 'o': 'no', 'no': 'o', 'p': 'np', 'np': 'p'}
CANON = {'e': ('eq', True), 'ne': ('eq', False), 'l': ('lt', True), 'ge': ('lt', False), 'le': ('le', True),
         'g': ('le', False), 'b': ('ltu', True), 'ae': ('ltu', False), 'be': ('leu', True), 'a': ('leu', False),
         's': ('s', True), 'ns': ('s', False), 'o': ('o', True), 'no': ('o', False), 'p': ('p', True), 'np': ('p', False)}
JALIAS = {'jz': 'je', 'jnz': 'jne', 'jc': 'jb', 'jnc': 'jae', 'jnae': 'jb', 'jnb': 'jae', 'jna': 'jbe', 'jnbe': 'ja',
          'jnge': 'jl', 'jnl': 'jge', 'jng': 'jle', 'jnle': 'jg', 'jpe': 'jp', 'jpo': 'jnp'}
NORET = {'_exit', 'exit', '_abort', 'abort', '__CxxThrowException@8', 'ExitProcess', '__amsg_exit'}
FRAME = re.compile(r'\[(?:ebp|esp)\b')
REGS = r'\b(?:e?[abcd]x|[abcd][lh]|e?si|e?di|e?bp|esp)\b'


def jcond(m):
    m = JALIAS.get(m, m)
    return m[1:] if m.startswith('j') and m[1:] in CANON else None


def imm(s):
    try:
        v = int(s, 0)
    except ValueError:
        return None
    return v - (1 << 32) if v >= 1 << 31 else v


class Ins:
    __slots__ = ('off', 'm', 'op', 'size', 'tgt', 'callee', 'store', 'txt', 'table', 'ka')

    def __init__(s, off, m, op, size):
        s.off, s.m, s.op, s.size = off, m, op, size
        s.tgt = s.callee = s.store = s.table = s.ka = None
        s.txt = m + ' ' + op


def _objects():
    CL.addr2sym()
    if not hasattr(_objects, 'cache'):
        ext = {}
        for p in __import__('glob').glob(fe.R + '/src/**/*.cpp', recursive=True) + __import__('glob').glob(fe.R + '/src/**/*.h', recursive=True):
            for m in re.finditer(r'RECOVERY_OBJECT: v\d+ 0x([0-9A-Fa-f]+) (?:0x([0-9A-Fa-f]+) )?(\S+)', open(p, encoding='latin-1').read()):
                a = int(m.group(1), 16)
                if m.group(2) or a not in ext:
                    ext[a] = (int(m.group(2), 16) if m.group(2) else None, m.group(3))
        objs = sorted(ext.items())
        # an object without an extent (v1) runs to the next object's start
        _objects.cache = [(a, e if e is not None else (objs[k + 1][0] if k + 1 < len(objs) else a + 4), n)
                          for k, (a, (e, n)) in enumerate(objs)]
    return _objects.cache


def img_global(a):
    import bisect
    objs = _objects()
    i = bisect.bisect_right(objs, (a, 1 << 40, '')) - 1
    if i >= 0 and objs[i][0] <= a < max(objs[i][1], objs[i][0] + 1):
        return '%s+%d' % (objs[i][2], a - objs[i][0])
    return 'g_%x' % a


def mem_store_key(op, glob_of):
    """first operand of a store -> key or None (frame slots and pointer-register stores without disp are kept as field 0)"""
    m = re.match(r'(?:byte|word|dword|qword) ptr (?:\w+:)?\[([^\]]*)\]', op)
    if not m:
        return None
    inner = m.group(1)
    if FRAME.search('[' + inner) or 'esp' in inner:
        return None
    g = glob_of(inner)
    if g is not None:
        return g
    d = re.search(r'([+-]) (0x[0-9a-f]+|\d+)$', inner)
    disp = (int(d.group(2), 0) * (-1 if d.group(1) == '-' else 1)) if d else 0
    w = op.split()[0]
    return 'f%s+%d' % (w[0], disp)


def listing_image(lo, hi):
    a2s, imp = CL.addr2sym(), CL.iat()
    out = []
    data = fe.imgbytes(lo, hi)
    for ins in fe.md.disasm(data, lo):
        x = Ins(ins.address - lo, ins.mnemonic, ins.op_str, ins.size)
        b = ins.bytes
        if ins.mnemonic in ('cmp', 'test', 'mov', 'push', 'lea'):
            v = imm(ins.op_str.split(',')[-1].strip())
            if v is not None and 0x400000 <= v < 0xa00000:
                x.ka = 'A:' + img_global(v)
        if ins.mnemonic == 'call':
            if b[0] == 0xE8:
                t = int(ins.op_str, 16)
                x.callee = None if lo <= t < hi else a2s.get(t, 'sub_%x' % t)
            elif b[0] == 0xFF and b[1] == 0x15:
                slot = struct.unpack_from('<I', b, 2)[0]
                x.callee = imp.get(slot, a2s.get(slot, CL._OBJ.get(slot, 'iat_%x' % slot)))
            else:
                x.callee = '<indirect>'
        elif ins.mnemonic.startswith('j') or ins.mnemonic.startswith('loop'):
            if re.fullmatch(r'0x[0-9a-f]+', ins.op_str):
                t = int(ins.op_str, 16)
                if lo <= t < hi:
                    x.tgt = t - lo
            else:
                m = re.search(r'\*4 \+ (0x[0-9a-f]+)\]', ins.op_str)
                if m and ins.mnemonic == 'jmp':
                    ta = int(m.group(1), 16)
                    tl = []
                    if lo <= ta < hi + 0x1000:
                        k = 0
                        while True:
                            e = struct.unpack_from('<I', fe.imgbytes(ta + 4 * k, ta + 4 * k + 4))[0]
                            if not (lo <= e < hi):
                                break
                            tl.append(e - lo)
                            k += 1
                    x.table = tl
                    if lo <= ta < hi:
                        hi = min(hi, ta)
        elif ins.mnemonic in ('mov', 'add', 'sub', 'inc', 'dec', 'or', 'and', 'xor', 'fstp', 'fst', 'fistp', 'mov', 'neg', 'shl', 'sar', 'shr') and ' ptr ' in ins.op_str.split(',')[0]:
            def gl(inner):
                m = re.fullmatch(r'(0x[0-9a-f]{6,})', inner.strip())
                if m:
                    return img_global(int(m.group(1), 16))
                m = re.search(r'([+-]) (0x[0-9a-f]{6,})$', inner)
                if m and int(m.group(2), 16) >= 0x400000:
                    return img_global(int(m.group(2), 16)) + '[]'
                return None
            x.store = mem_store_key(ins.op_str.split(',')[0], gl)
        out.append(x)
        if x.off >= hi - lo:
            break
    out = [x for x in out if x.off < hi - lo]
    return trim(out)


def trim(out):
    while out and out[-1].m in ('nop', 'int3'):
        out.pop()
    if not (out and out[-1].m == 'call'):
        while out and not out[-1].m.startswith(('ret', 'jmp')):
            out.pop()
    return out


def listing_ours(c, sym):
    s = c.function(sym)
    sec = c.sections[s.section - 1]
    rm = {r[0]: r for r in sec.relocations}
    body = fe.dis(sec.data, 0, sec.relocations, selfsec=s.section, syms=c.symbols)
    end = body[-1][0] + body[-1][2] if body else len(sec.data)
    out = []

    def rsym(at):
        r = rm.get(at)
        if not r:
            return None, None
        sy = c.symbols.get(r[1])
        add = struct.unpack_from('<I', sec.data, at)[0]
        return sy, add
    for ins in fe.md.disasm(sec.data[:end], 0):
        x = Ins(ins.address, ins.mnemonic, ins.op_str, ins.size)
        b = ins.bytes
        relo = [o for o in range(ins.address, ins.address + ins.size) if o in rm]
        if ins.mnemonic in ('cmp', 'test', 'mov', 'push', 'lea') and (ins.address + ins.size - 4) in rm and imm(ins.op_str.split(',')[-1].strip()) is not None:
            sy, add = rsym(ins.address + ins.size - 4)
            if sy is not None:
                x.ka = 'A:%s+%d' % (sy.name, add)
        if ins.mnemonic == 'call':
            if relo and (b[0] == 0xE8 or (b[0] == 0xFF and b[1] == 0x15)):
                sy, _ = rsym(relo[0])
                x.callee = None if sy and sy.name == sym else (CL.shared_name(CL.norm_ours(sy.name)) if sy else '?')
            elif b[0] == 0xE8:
                x.callee = None
            else:
                x.callee = '<indirect>'
        elif ins.mnemonic.startswith('j') or ins.mnemonic.startswith('loop'):
            if re.fullmatch(r'0x[0-9a-f]+', ins.op_str) and not relo:
                x.tgt = int(ins.op_str, 16)
            elif ins.mnemonic == 'jmp' and '*4' in ins.op_str and relo:
                sy, add = rsym(relo[0])
                tl = []
                if sy is not None and sy.section == s.section:
                    ta = sy.value + add
                    k = 0
                    while ta + 4 * k + 4 <= len(sec.data) and (ta + 4 * k) in rm:
                        sy2, add2 = rsym(ta + 4 * k)
                        tl.append(sy2.value + add2)
                        k += 1
                x.table = tl
        elif ' ptr ' in ins.op_str.split(',')[0] and ins.mnemonic in ('mov', 'add', 'sub', 'inc', 'dec', 'or', 'and', 'xor', 'fstp', 'fst', 'fistp', 'neg', 'shl', 'sar', 'shr'):
            def gl(inner, ins=ins, relo=relo):
                if not relo:
                    return None
                sy, add = rsym(relo[0])
                if sy is None:
                    return None
                nm = sy.name
                # name it the way the image side does: object symbol + offset
                d = re.search(r'([+-]) (0x[0-9a-f]+|\d+)$', inner)
                regpart = re.sub(r'0x5a5a5a5a|0x[0-9a-f]{6,}', '', inner)
                if re.search(REGS, regpart):
                    return '%s+%d[]' % (nm, add)
                return '%s+%d' % (nm, add)
            x.store = mem_store_key(ins.op_str.split(',')[0], gl)
        out.append(x)
    return trim(out)


def unify_globals(img, ours):
    """our relocation symbol names -> the image's object names (decorated -> marker name) where possible"""
    names = {}
    for o in _objects():
        names[o[2]] = o[2]
    return names


# ---------------------------------------------------------------- CFG / control dependence
def cfg(L):
    if not L:
        return [], {}, {}
    offs = [x.off for x in L]
    idx = {o: i for i, o in enumerate(offs)}
    leaders = {0}
    for i, x in enumerate(L):
        if x.tgt is not None and x.tgt in idx:
            leaders.add(idx[x.tgt])
        if x.table:
            for t in x.table:
                if t in idx:
                    leaders.add(idx[t])
        if x.m.startswith(('j', 'ret', 'loop')) or (x.m == 'call' and x.callee in NORET):
            if i + 1 < len(L):
                leaders.add(i + 1)
    ls = sorted(leaders)
    blocks = []
    for k, s in enumerate(ls):
        e = ls[k + 1] if k + 1 < len(ls) else len(L)
        blocks.append((s, e))
    bof = {s: k for k, (s, e) in enumerate(blocks)}
    succ = {}
    for k, (s, e) in enumerate(blocks):
        last = L[e - 1]
        ss = []
        m = last.m
        fall = bof.get(e)
        if m.startswith('ret') or (m == 'call' and last.callee in NORET):
            pass
        elif m == 'jmp':
            if last.tgt is not None and last.tgt in idx:
                ss.append(('', bof[idx[last.tgt]]))
            elif last.table:
                for t in sorted(set(last.table)):
                    if t in idx:
                        ss.append(('', bof[idx[t]]))
        elif jcond(m):
            if last.tgt is not None and last.tgt in idx:
                ss.append(('T', bof[idx[last.tgt]]))
            if fall is not None:
                ss.append(('F', fall))
        else:
            if fall is not None:
                ss.append(('', fall))
        succ[k] = ss
    return blocks, succ, idx


def postdom(nb, succ):
    EXIT = nb
    allb = set(range(nb + 1))
    pd = {b: set(allb) for b in range(nb)}
    pd[EXIT] = {EXIT}
    sc = {b: [t for _, t in succ.get(b, [])] or [EXIT] for b in range(nb)}
    changed = True
    order = list(range(nb - 1, -1, -1))
    while changed:
        changed = False
        for b in order:
            new = set.intersection(*[pd[t] for t in sc[b]]) | {b}
            if new != pd[b]:
                pd[b] = new
                changed = True
    ipd = {}
    for b in range(nb):
        cand = pd[b] - {b}
        # immediate post-dominator: the candidate post-dominated by all others
        best = None
        for x in cand:
            if all(y in pd[x] for y in cand):
                best = x
                break
        ipd[b] = best
    return pd, ipd


def constreg(L, i, r):
    """a callee-saved register holding a constant at L[i] (last write before it in listing order)"""
    scratch = r in ('eax', 'ecx', 'edx')
    if r not in ('esi', 'edi', 'ebx') and not scratch:
        return None
    for k in range(i - 1, max(-1, i - (8 if scratch else 400)), -1):
        x = L[k]
        ops = [o.strip() for o in x.op.split(',')]
        if x.m in ('call',) or x.m.startswith('j'):
            if scratch:
                return None
            continue
        if ops and ops[0] == r and x.m not in ('cmp', 'test', 'push'):
            if x.m == 'xor' and len(ops) == 2 and ops[1] == r:
                return 0
            if x.m == 'mov' and len(ops) == 2 and imm(ops[1]) is not None and not x.ka:
                return imm(ops[1])
            if x.m == 'or' and len(ops) == 2 and imm(ops[1]) == -1:
                return -1
            return None
        if x.m == 'pop' and ops[0] == r:
            return None
    return None


def predicate(L, blocks, b):
    s, e = blocks[b]
    j = L[e - 1]
    cc = jcond(j.m)
    if not cc:
        return None
    rel, truth = CANON[cc]
    K = None
    kind = '?'
    for i in range(e - 2, s - 1, -1):
        x = L[i]
        if x.m in ('cmp', 'test', 'and', 'or', 'sub', 'add', 'inc', 'dec', 'xor', 'sar', 'shl', 'shr', 'neg', 'sbb', 'adc'):
            ops = [o.strip() for o in x.op.split(',')]
            kind = x.m
            if x.m == 'cmp' and len(ops) == 2 and x.ka:
                K = x.ka
            elif x.m == 'cmp' and len(ops) == 2:
                K = imm(ops[1])
                if K is None:
                    k1, k0 = constreg(L, i, ops[1]), constreg(L, i, ops[0])
                    if k1 is not None:
                        K = k1
                    elif k0 is not None:
                        # `cmp K, y`: K rel y -> y against K with the relation mirrored
                        K = k0
                        if rel in ('lt', 'ltu'):
                            rel, K, truth = rel, K + 1, not truth
                        elif rel in ('le', 'leu'):
                            rel, truth = ('lt' if rel == 'le' else 'ltu'), not truth
                    else:
                        K = 'x'
                elif re.match(r'(?:byte ptr|[abcd]l\b|[abcd]h\b)', ops[0]) and K >= 128 and rel == 'eq':
                    K -= 256
                elif re.match(r'(?:word ptr|[abcd]x\b|si\b|di\b)', ops[0]) and K >= 0x8000 and rel == 'eq':
                    K -= 0x10000
            elif x.m == 'test' and len(ops) == 2:
                if ops[0] == ops[1]:
                    K = 0
                else:
                    v = imm(ops[1])
                    if v is not None:
                        rel = 'bit' if rel == 'eq' else rel
                        K = v & 0xffffffff
                    else:
                        K = 'tx'
            elif x.m == 'inc':
                K = -1 if rel == 'eq' else 'inc'
            elif x.m == 'dec':
                K = 1 if rel == 'eq' else 'dec'
            else:
                K = x.m
            break
        if x.m in ('call',):
            break
    if K is None:
        K = '?'
    if isinstance(K, int) and rel in ('le', 'leu'):
        rel, K = ('lt' if rel == 'le' else 'ltu'), K + 1
    return (rel, K, truth, kind)


def guards(L):
    blocks, succ, idx = cfg(L)
    nb = len(blocks)
    if not nb:
        return {}, blocks
    pd, ipd = postdom(nb, succ)
    cd = collections.defaultdict(set)   # block -> {(pred-block, label)}
    for a in range(nb):
        for lab, bb in succ.get(a, []):
            if lab not in ('T', 'F'):
                continue
            stop = ipd[a]
            y = bb
            seen = set()
            while y is not None and y != stop and y < nb and y not in seen:
                seen.add(y)
                cd[y].add((a, lab))
                y = ipd.get(y)
    preds = {a: predicate(L, blocks, a) for a in range(nb)}
    # a latch test (backward jcc) reads the induction form the optimiser chose (i < 4 against i*4 < 12): not
    # comparable between the sides, and its loop is reached through the guard test that reads the same bound
    for a in range(nb):
        jx = L[blocks[a][1] - 1]
        if jx.tgt is not None and jx.tgt <= jx.off:
            preds[a] = None
    memo = {}

    def closure(b):
        if b in memo:
            return memo[b]
        memo[b] = frozenset()
        out = set()
        stack = [b]
        seen = {b}
        while stack:
            y = stack.pop()
            for a, lab in cd.get(y, ()):
                p = preds.get(a)
                if p:
                    rel, K, truth, kind = p
                    out.add((rel, K, truth if lab == 'T' else not truth))
                if a not in seen:
                    seen.add(a)
                    stack.append(a)
        memo[b] = frozenset(out)
        return memo[b]
    bl = {}
    for k, (s, e) in enumerate(blocks):
        for i in range(s, e):
            bl[i] = k
    inv = {(p[0], p[1]) for p in preds.values() if p}
    guards.where = collections.defaultdict(list)   # (rel, K) -> offsets of the jccs testing it
    for a_, p in preds.items():
        if p:
            guards.where[(p[0], p[1])].append(L[blocks[a_][1] - 1].off)
    return {i: closure(bl[i]) for i in range(len(L))}, inv


def clean(g, common):
    """drop predicates held with both truths (loop tests reached around the back edge) and predicates the other
    side never tests anywhere (another loop or compare form: not comparable site by site)"""
    return frozenset(p for p in g if (p[0], p[1], not p[2]) not in g and (p[0], p[1]) in common)


def sites(L, G):
    out = collections.defaultdict(list)
    for i, x in enumerate(L):
        if x.callee:
            out[('C', x.callee)].append((x.off, G.get(i, frozenset())))
        elif x.store:
            out[('S', x.store)].append((x.off, G.get(i, frozenset())))
    return out


def bitshift(a, b):
    return any(a == b << s or b == a << s for s in (8, 16, 24))


def classify(gi, go, common=None):
    a, b = set(gi - go), set(go - gi)
    for p in list(a):
        # `test ah, 4` against `test dword, 0x400`: one bit seen through another byte
        q = next((q for q in b if p[0] == q[0] == 'bit' and p[2] == q[2] and isinstance(p[1], int) and isinstance(q[1], int) and bitshift(p[1], q[1])), None)
        if q:
            a.discard(p)
            b.discard(q)
    hits = []
    for p in sorted(a, key=str):
        rel, K, t = p
        if (rel, K, not t) in b:
            hits.append(('INVERT', p, (rel, K, not t)))
            b.discard((rel, K, not t))
            continue
        q = next((q for q in b if q[0] == rel and q[2] == t and isinstance(K, int) and isinstance(q[1], int)), None)
        if q:
            hits.append(('CONST', p, q))
            b.discard(q)
            continue
        q = next((q for q in b if q[1] == K and q[2] == t and q[0] != rel and isinstance(K, int)), None)
        if q:
            hits.append(('OP', p, q))
            b.discard(q)
            continue
        q = next((q for q in b if q[0] == rel and q[2] == t and (q[1] == 'x') != (K == 'x')), None)
        if q:
            # a compare against a register on one side and a constant on the other: a constant register
            hits.append(('KREG', p, q))
            b.discard(q)
            continue
        if common is None or (rel, K) in common:
            hits.append(('IMG-ONLY' if isinstance(K, int) else 'IMG-ONLY?', p, None))
    for q in sorted(b, key=str):
        if common is None or (q[0], q[1]) in common:
            hits.append(('OURS-ONLY' if isinstance(q[1], int) else 'OURS-ONLY?', None, q))
    return hits


W = {'INVERT': 8, 'CONST': 6, 'OP': 6, 'IMG-ONLY': 2, 'OURS-ONLY': 2, 'KREG': 0.25, 'IMG-ONLY?': 0.5, 'OURS-ONLY?': 0.5}


def site_weight(key):
    """teardown calls and stores through a bare pointer say little about the source's conditions"""
    k, n = key
    if k == 'C':
        return 0.3 if re.match(r'\?\?[1_]|\?close@', str(n)) else 1
    return 0.25 if re.match(r'f[a-z]\+0$', str(n)) else 0.5


def pair(ai, ao, amap=None):
    """pair two site lists: sites fe aligns row to row first, the rest by minimum symmetric guard difference"""
    amap = amap or {}
    # guard distance first (arms laid out in the other order align crosswise), fe alignment breaks ties
    cand = sorted(((len(gi ^ go), amap.get(oi) != oo, i, j) for i, (oi, gi) in enumerate(ai) for j, (oo, go) in enumerate(ao)))
    ui, uo, out = set(), set(), []
    for d, _, i, j in cand:
        if i in ui or j in uo:
            continue
        ui.add(i)
        uo.add(j)
        out.append((ai[i], ao[j]))
    return out, [ai[i] for i in range(len(ai)) if i not in ui], [ao[j] for j in range(len(ao)) if j not in uo]


def skel(t):
    t = re.sub(r'\[(?:ebp|esp)[^\]]*\]', '[S]', t)
    t = re.sub(REGS, 'R', t)
    t = re.sub(r'\b0x[0-9a-f]+\b|\b\d+\b', 'K', t)
    m = t.split(' ', 1)[0]
    if jcond(m):
        t = 'jcc' + t[len(m):]
    return t


def test_hits(res, img_by_off, our_by_off):
    rows = res['rows']
    hits = []
    i = 0
    while i < len(rows):
        if rows[i][0] == '  ':
            i += 1
            continue
        j = i
        while j < len(rows) and rows[j][0] != '  ':
            j += 1
        A = [r[1] for r in rows[i:j] if r[1] is not None]
        B = [r[2] for r in rows[i:j] if r[2] is not None]
        i = j
        sa, sb = [skel(a[1]) for a in A], [skel(b[1]) for b in B]
        for t, i1, i2, j1, j2 in difflib.SequenceMatcher(None, sa, sb, autojunk=False).get_opcodes():
            if t != 'equal':
                continue
            for k in range(i2 - i1):
                a, b = A[i1 + k], B[j1 + k]
                xa, xb = img_by_off.get(a[0]), our_by_off.get(b[0])
                if not xa or not xb:
                    continue
                if xa.m in ('cmp', 'test') and xa.m == xb.m:
                    pa = [o.strip() for o in xa.op.split(',')]
                    pb = [o.strip() for o in xb.op.split(',')]
                    if len(pa) == 2 and len(pb) == 2:
                        ka, kb = imm(pa[1]), imm(pb[1])
                        if ka is not None and kb is not None and ka != kb and not (abs(ka) > 0x400000 or abs(kb) > 0x400000):
                            # next jcc decides whether K+1 with le/lt is the same test
                            na = nxt_j(xa, img_by_off)
                            nb = nxt_j(xb, our_by_off)
                            ca, cb = (CANON.get(jcond(na.m) or '', (None,))[0] if na else None), (CANON.get(jcond(nb.m) or '', (None,))[0] if nb else None)
                            if ca and cb and norm(ca, ka) == norm(cb, kb):
                                continue
                            hits.append(('K', xa.off, xa.txt, xb.off, xb.txt, (na.m if na else ''), (nb.m if nb else '')))
                ja, jb = jcond(xa.m), jcond(xb.m)
                if ja and jb and CANON[ja][0] != CANON[jb][0]:
                    pa = prev_flag(xa, img_by_off)
                    pb = prev_flag(xb, our_by_off)
                    ka = imm(pa.op.split(',')[-1].strip()) if pa and pa.m == 'cmp' else None
                    kb = imm(pb.op.split(',')[-1].strip()) if pb and pb.m == 'cmp' else None
                    if ka is not None and kb is not None and norm(CANON[ja][0], ka) == norm(CANON[jb][0], kb):
                        continue
                    if ka is not None and kb is not None and norm(CANON[NEG[ja]][0], ka) == norm(CANON[jb][0], kb):
                        continue
                    hits.append(('J', xa.off, (pa.txt if pa else '') + ' ; ' + xa.m, xb.off, (pb.txt if pb else '') + ' ; ' + xb.m, '', ''))
    return hits


def norm(rel, K):
    if rel in ('le', 'leu'):
        return ('lt' if rel == 'le' else 'ltu', K + 1)
    return (rel, K)


_ORDER = {}


def nxt_j(x, by):
    o = _ORDER.get(id(by))
    if o is None or len(o) != len(by):
        o = _ORDER[id(by)] = sorted(by)
    import bisect
    k = bisect.bisect_right(o, x.off)
    for t in o[k:k + 3]:
        if jcond(by[t].m):
            return by[t]
    return None


def prev_flag(x, by):
    o = _ORDER.get(id(by))
    if o is None or len(o) != len(by):
        o = _ORDER[id(by)] = sorted(by)
    import bisect
    k = bisect.bisect_left(o, x.off)
    for t in reversed(o[max(0, k - 3):k]):
        if by[t].m in ('cmp', 'test'):
            return by[t]
    return None


def analyse(c, text, addr):
    idn = fe.ident(text, addr)
    if not idn:
        return None
    sym, lo, hi = idn
    try:
        LO = listing_ours(c, sym)
    except Exception as e:
        return dict(addr='%08X' % addr, sym=sym, error=str(e))
    LI = listing_image(lo, hi)
    GI, invi = guards(LI)
    wi = guards.where
    GO, invo = guards(LO)
    wo = guards.where
    common = invi & invo
    GI = {k: clean(v, common | {q for q in invi if isinstance(q[1], int) and q not in invo}) for k, v in GI.items()}
    GO = {k: clean(v, common | {q for q in invo if isinstance(q[1], int) and q not in invi}) for k, v in GO.items()}
    si, so = sites(LI, GI), sites(LO, GO)
    hits = []
    res = fe.evaluate(c, text, addr)
    amap = {}
    for k_, o_, c_ in res.get('rows', []):
        if o_ is not None and c_ is not None and k_ != '!!':
            amap[o_[0]] = c_[0]
    groups = {}
    # an unmarked image callee (sub_X: a CRT routine without a name) pairs with our one-side callee of equal count
    ion = {k: len(v) for k, v in si.items() if k[0] == 'C' and k not in so and str(k[1]).startswith('sub_')}
    oon = {k: len(v) for k, v in so.items() if k[0] == 'C' and k not in si}
    for k, n in ion.items():
        m = [q for q, c in oon.items() if c == n]
        if len(m) == 1:
            so[k] = so.pop(m[0])
            del oon[m[0]]
    for key in sorted(set(si) | set(so), key=str):
        ai, ao = si.get(key, []), so.get(key, [])
        pr, ri, ro = pair(ai, ao, amap)
        if key[0] == 'C' and len(ai) != len(ao):
            hits.append(dict(kind='COUNT', key=key, img=len(ai), ours=len(ao),
                             img_at=['%x' % (lo + o) for o, _ in ri], ours_at=['%x' % o for o, _ in ro], score=5 * abs(len(ai) - len(ao))))
        for (oi, gi), (oo, go) in pr:
            if gi == go:
                continue
            for h in classify(gi, go, common):
                g = groups.setdefault((h[0], h[1], h[2]), [])
                g.append((key, '%x' % (lo + oi), '%x' % oo))
    # one hit per predicate difference: a test placed elsewhere moves every site below it
    for (kind, pi, po), ss in groups.items():
        nc = sum(1 for k, _, _ in ss if k[0] == 'C')
        w = W[kind] * max(site_weight(k) for k, _, _ in ss)
        ss.sort(key=lambda s: (s[0][0] != 'C', int(s[1], 16)))
        k0, ia, oa = ss[0]
        jw = lambda w, p, base: ','.join('%x' % (base + o) for o in w.get((p[0], p[1]), [])[:6]) if p else ''
        hits.append(dict(kind=kind, key=k0, img_at=ia, ours_at=oa, img_pred=pi, ours_pred=po, n=len(ss), ncalls=nc,
                         img_jcc=jw(wi, pi, lo), ours_jcc=jw(wo, po, 0),
                         sites=[(k[0] + ':' + str(k[1]), x, y) for k, x, y in ss], score=w))
    th = []
    if 'rows' in res:
        bi = {x.off: x for x in LI}
        bo = {x.off: x for x in LO}
        _ORDER.clear()
        for h in test_hits(res, bi, bo):
            th.append(dict(kind='TEST-' + h[0], img_at='%x' % (lo + h[1]), img=h[2], ours_at='%x' % h[3], ours=h[4], jn=h[5] + '/' + h[6], score=3))
    return dict(addr='%08X' % addr, sym=sym, rows=res.get('diff'), hits=hits, tests=th,
                score=sum(h['score'] for h in hits) + sum(h['score'] for h in th))


def fmtp(p):
    if p is None:
        return '-'
    rel, K, t = p
    return '%s%s %s' % ('' if t else '!', rel, K)


def report(r, verbose=False, top=40):
    if r is None:
        return
    if 'error' in r:
        print('BS %s %s ERROR %s' % (r['addr'], r['sym'][:40], r['error']))
        return
    print('BS %s %-44s rows=%s score=%.1f hits=%d tests=%d' % (r['addr'], r['sym'][:44], r['rows'], r['score'], len(r['hits']), len(r['tests'])))
    hs = sorted(r['hits'], key=lambda h: -h['score'])
    for h in hs[:top if not verbose else 10 ** 6]:
        k = h['key'][0] + ':' + str(h['key'][1])[:48]
        if h['kind'] == 'COUNT':
            print('   COUNT     %-50s img %d ours %d  img-unpaired %s ours-unpaired %s' % (k, h['img'], h['ours'], ','.join(h['img_at']), ','.join(h['ours_at'])))
        else:
            print('   %-9s %-50s img %s [%s]  ours %s [%s]  sites %d (calls %d)' % (h['kind'], k, h['img_at'], fmtp(h['img_pred']), h['ours_at'], fmtp(h['ours_pred']), h['n'], h['ncalls']))
            print('             tested at img %s | ours %s' % (h.get('img_jcc', ''), h.get('ours_jcc', '')))
            if verbose:
                for s in h['sites'][1:]:
                    print('             %-50s img %s ours %s' % (s[0][:50], s[1], s[2]))
    for h in r['tests'][:top if not verbose else 10 ** 6]:
        print('   %-9s img %s %-34s ours %s %-34s %s' % (h['kind'], h['img_at'], h['img'][:34], h['ours_at'], h['ours'][:34], h['jn']))


def worker(unit, addrs, pfx, src=None, out=None, verbose=False):
    src = src or fe.R + '/' + unit
    text = open(src, encoding='latin-1').read()
    c, err = fe.compile_unit(unit, src, pfx)
    res = []
    if c is None:
        print('COMPILE', unit, err)
        return res
    for a in addrs:
        try:
            r = analyse(c, text, a)
        except Exception as e:
            r = dict(addr='%08X' % a, sym='?', error=repr(e))
        if r is None:
            continue
        r['unit'] = unit
        res.append(r)
        if out is None:
            report(r, verbose)
    return res


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('unit', nargs='?')
    ap.add_argument('addr', nargs='?')
    ap.add_argument('rest', nargs='*')
    ap.add_argument('--src')
    ap.add_argument('--pfx', default=fe.PFX)
    ap.add_argument('--tree', action='store_true')
    ap.add_argument('--from', dest='frm')
    ap.add_argument('--json')
    ap.add_argument('--top', type=int, default=60)
    ap.add_argument('--worker', action='store_true')
    ap.add_argument('-v', action='store_true')
    a = ap.parse_args()
    if a.worker:
        unit = a.unit
        addrs = [int(x, 16) for x in a.addr.split(',')]
        res = worker(unit, addrs, a.pfx, out=True)
        print('JSON' + json.dumps(res, default=list))
        return 0
    if not a.tree:
        unit = a.unit if a.unit.startswith('src/') else 'src/' + a.unit
        worker(unit, [int(x, 16) for x in a.addr.split(',')], a.pfx, a.src, verbose=a.v)
        return 0
    units = [x for x in [a.unit, a.addr] + a.rest if x]
    feall = json.load(open(a.frm or T + '/feall.json'))
    work = collections.OrderedDict()
    for r in feall:
        if r.get('exact') or r.get('diff', -1) < 0 or r['addr'] == '?':
            continue
        if units and r['unit'] not in units and os.path.basename(r['unit']) not in units:
            continue
        work.setdefault(r['unit'], []).append(r['addr'])
    jobs = int(os.environ.get('FE_JOBS', '5'))
    base = int(os.environ.get('FE_PFX_BASE', fe.PFX))
    items = list(work.items())
    from concurrent.futures import ThreadPoolExecutor

    def lane(k):
        out = []
        for i in range(k, len(items), jobs):
            u, ad = items[i]
            p = subprocess.run([sys.executable, os.path.abspath(__file__), u, ','.join(ad), '--worker', '--pfx', str(base + k)],
                               capture_output=True, text=True)
            for l in p.stdout.split('\n'):
                if l.startswith('JSON'):
                    out += json.loads(l[4:])
            if p.returncode:
                print('FAIL', u, p.stderr[-400:], file=sys.stderr)
        return out
    res = []
    with ThreadPoolExecutor(jobs) as ex:
        for o in ex.map(lane, range(jobs)):
            res += o
    res.sort(key=lambda r: -r.get('score', 0))
    if a.json:
        json.dump(res, open(a.json, 'w'), indent=0, default=list)
    for r in res[:a.top]:
        report(r, top=12)
    print('functions', len(res), 'with hits', sum(1 for r in res if r.get('hits') or r.get('tests')))
    return 0


if __name__ == '__main__':
    sys.exit(main())
