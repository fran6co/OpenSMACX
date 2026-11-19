#!/usr/bin/env python3
"""retscan.py [--all] [--min N]
Return-type census from the image alone (MATCHING_GUIDE 3.4): for every direct call to a mapped function,
classify how the caller consumes the result in the instruction after the call (and its `add esp, N`):

  d   dword use   test eax,eax / cmp eax,.. / mov r32,eax / mov [m32],eax / push eax / arithmetic on eax
  b   byte use    test al,al / cmp al,.. / mov [m8],al / mov r8,al
  sxb movsx r32,al   zxb movzx r32,al or `and eax,0xff`      (char / unsigned char or bool)
  w   word use    test ax,ax / cmp ax / mov [m16],ax    sxw movsx r32,ax   zxw movzx / and eax,0xffff
  -   result ignored (eax overwritten or not read next)

and the callee's own epilogue: the last write of the result before each `ret` (`mov al,..`, `xor eax,eax`,
`mov eax,..`, `movsx eax,..`). The declared return type comes from the decorated name. Listed: callees whose
image uses contradict it (byte/word uses of a function declared int, dword uses of one declared char/bool,
the other extension), ranked by the number of non-exact callers."""
import sys, os, re, struct, bisect, argparse, collections
sys.path.insert(0, 'recovery/tools'); sys.path.insert(0, 'recovery/tools/analysis')
import fe, tumap

ap = argparse.ArgumentParser(); ap.add_argument('--all', action='store_true'); ap.add_argument('--min', type=int, default=1)
a = ap.parse_args()
mk = tumap.load()   # lo,hi,unit,name,exact,path
texts = {}
MARK = re.compile(r'// ORIGINAL: 0x([0-9A-F]{8}) (\S+) [^\n]*\n((?://(?! ORIGINAL:)[^\n]*\n)*)')
symof = {}
for root, _, fs in os.walk('src'):
    for f in fs:
        if f.endswith(('.cpp', '.h')):
            t = open(os.path.join(root, f), encoding='latin-1').read()
            for m in MARK.finditer(t):
                s2 = re.search(r'// symbol\s+(\S+)', m.group(3))
                symof.setdefault(int(m.group(1), 16), s2.group(1) if s2 else m.group(2))


def rettype(sym):
    m = re.match(r'\?[^@]+(?:@[^@]+)*@@(.*)$', sym)
    if not m or sym.startswith('??'):
        return None
    r = m.group(1)
    if r[0] == 'Y':
        r = r[2:]
    elif r[0] in 'CDKLST':
        r = r[2:]
    elif r[0] in 'ABEFIJMNQRUV':
        r = r[3:]
    else:
        return None
    for k, v in (('_N', 'bool'), ('X', 'void'), ('H', 'int'), ('I', 'uint'), ('J', 'long'), ('K', 'ulong'), ('D', 'char'),
                 ('C', 'schar'), ('E', 'uchar'), ('F', 'short'), ('G', 'ushort'), ('M', 'float'), ('N', 'double'),
                 ('PA', 'ptr'), ('PB', 'ptr'), ('W4', 'enum'), ('?A', 'class'), ('V', 'class'), ('U', 'class'), ('AA', 'ref'), ('P6', 'ptr')):
        if r.startswith(k):
            return v
    return None


WIDTHOF = {'bool': 1, 'char': 1, 'schar': 1, 'uchar': 1, 'short': 2, 'ushort': 2, 'int': 4, 'uint': 4, 'long': 4, 'ulong': 4,
           'ptr': 4, 'enum': 4}
los = [m[0] for m in mk]
starts = {m[0]: m for m in mk}
uses = collections.defaultdict(lambda: collections.Counter())
callers = collections.defaultdict(lambda: collections.defaultdict(set))


def classify(t):
    mn, _, op = t.partition(' ')
    ops = [x.strip() for x in op.split(',')]
    if mn in ('test', 'cmp'):
        if ops[0] == 'al' or (len(ops) > 1 and ops[1] == 'al'):
            return 'b'
        if ops[0] == 'ax' or (len(ops) > 1 and ops[1] == 'ax'):
            return 'w'
        if 'eax' in ops:
            return 'd'
    if mn == 'movsx' and len(ops) > 1:
        return {'al': 'sxb', 'ax': 'sxw'}.get(ops[1])
    if mn == 'movzx' and len(ops) > 1:
        return {'al': 'zxb', 'ax': 'zxw'}.get(ops[1])
    if mn == 'and' and ops[0] == 'eax':
        return {'0xff': 'zxb', '0xffff': 'zxw'}.get(ops[1], 'd')
    if mn == 'mov' and len(ops) > 1:
        if ops[1] == 'al':
            return 'b'
        if ops[1] == 'ax':
            return 'w'
        if ops[1] == 'eax':
            return 'd'
        if ops[0] in ('eax', 'al', 'ax'):
            return '-'
    if mn == 'push' and ops[0] == 'eax':
        return 'd'
    if mn == 'lea':
        return '-' if ops[0] == 'eax' and 'eax' not in ops[1] else ('d' if 'eax' in ops[1] else None)
    if mn in ('add', 'sub', 'imul', 'neg', 'shl', 'sar', 'shr', 'or', 'xor', 'inc', 'dec', 'cdq') and 'eax' in op:
        if mn == 'xor' and ops == ['eax', 'eax'] or mn == 'or' and ops == ['eax', '0xffffffff']:
            return '-'
        return 'd'
    return '-' if mn in ('call', 'ret', 'jmp', 'pop') else None


for lo, hi, unit, name, exact, path in mk:
    try:
        O = fe.imgdis(lo, hi)
    except Exception:
        continue
    raw = fe.imgbytes(lo, hi)
    for k, (off, txt, size, tgt) in enumerate(O):
        if not txt.startswith('call') or size != 5 or raw[off] != 0xE8:
            continue
        t = lo + off + 5 + struct.unpack_from('<i', raw, off + 1)[0]
        if t not in starts:
            continue
        j = k + 1
        if j < len(O) and re.match(r'add esp, ', O[j][1]):
            j += 1
        c = None
        # skip moves that do not touch eax (argument reloads)
        for j2 in range(j, min(len(O), j + 3)):
            c = classify(O[j2][1])
            if c is not None:
                break
            if O[j2][1].startswith('j'):
                break
        c = c or '?'
        uses[t][c] += 1
        callers[t][c].add((lo, exact))
rows = []
for t, cnt in uses.items():
    sym = symof.get(t, starts[t][3])
    rt = rettype(sym)
    if rt is None:
        continue
    w = WIDTHOF.get(rt)
    nb = cnt['b'] + cnt['sxb'] + cnt['zxb']
    nw = cnt['w'] + cnt['sxw'] + cnt['zxw']
    nd = cnt['d']
    why = None
    if rt == 'void' and (nd + nb + nw):
        why = 'void but result used'
    elif w == 4 and (nb or nw) and (nb + nw) * 2 >= nd:
        why = 'declared %s, image uses %s' % (rt, 'byte' if nb >= nw else 'word')
    elif w == 1 and (nd or nw):
        why = 'declared %s, image uses dword/word' % rt
    elif w == 2 and (nd or nb):
        why = 'declared %s, image uses dword/byte' % rt
    elif rt in ('char', 'schar') and cnt['zxb'] > cnt['sxb']:
        why = 'declared signed char, image zero-extends'
    elif rt in ('uchar', 'bool') and cnt['sxb'] > cnt['zxb']:
        why = 'declared %s, image sign-extends' % rt
    elif rt == 'short' and cnt['zxw'] > cnt['sxw']:
        why = 'declared short, image zero-extends'
    elif rt == 'ushort' and cnt['sxw'] > cnt['zxw']:
        why = 'declared ushort, image sign-extends'
    if why or a.all:
        bad = set()
        for c, s in callers[t].items():
            if c in ('-', '?'):
                continue
            bad |= {x for x in s if not x[1]}
        rows.append((len(bad), sum(cnt.values()), t, sym, rt, why or '', dict(cnt), starts[t][4]))
rows.sort(key=lambda r: (-r[0], -r[1]))
for nb_, n, t, sym, rt, why, cnt, exact in rows:
    if n < a.min:
        continue
    print('%08X %s %-50s ret=%-6s nonexact_callers=%-3d %s  %s' % (t, 'X' if exact else '-', sym[:50], rt, nb_, why, ' '.join('%s:%d' % kv for kv in sorted(cnt.items()))))
print('callees with typed results: %d, flagged: %d' % (len(uses), len(rows)))
