#!/usr/bin/env python3
"""structscan.py [unit ...] [--addr A[,A]] [--json F] [--near N] [--kinds F,G,I]

Field/global/constant SET screen (MATCHING_GUIDE 3.7), independent of instruction pairing: for every
non-BYTE_EXACT marker, decode both sides with capstone (ours with every relocated field replaced by its
source-bound image address, so globals compare as addresses) and collect per side

  F  struct field offsets: a memory operand [reg + d] whose base register is typed by a linear taint pass
     (`this` = ecx at entry and its copies; a register loaded with the address of a bound global
     `lea r,[G]` / `mov r,G` / `add r,G` = that global with its bias; a register loaded from a bound pointer
     global `mov r,[G]` = `*G`). Absolute operands [G + d] / [idx*s + G + d] are typed as (G, d).
     Untyped register bases fall into `?`.
  G  global addresses touched (memory operands and address immediates), named by the nearest bound symbol
  I  immediates of cmp/test/and/or/xor/add/sub/imul/shl/shr/sar/mov/push (frame adjustments, EH state
     stores and address-valued immediates excluded)

and prints what one side has and the other never has. A mislabelled field is an image-only offset plus an
ours-only offset of the same type, usually close together (`near` pairs are printed first). Folding and
induction-variable differences also produce one-sided offsets: read each lead against the image.
Unbound relocations on our side (string literals, unmapped globals) are listed as `ours-unbound`.
"""
import argparse
import bisect
import json
import os
import pathlib
import re
import struct
import sys
from collections import Counter, defaultdict
from concurrent.futures import ThreadPoolExecutor

T = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, T)
import fe  # noqa: E402
import capstone  # noqa: E402
from capstone import x86  # noqa: E402
from match_recovery.source.asm import CodeReference, string_literal_bytes  # noqa: E402
from match_recovery.source_selection import measurement_reference_targets  # noqa: E402
from match_recovery.measurement import Measurement  # noqa: E402

R = fe.R
MARK = re.compile(r'// ORIGINAL: (0x[0-9A-F]{8}) \S+ 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)')
MD = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
MD.detail = True
IMGLO, IMGHI = 0x400000, 0xA00000
FAKE = 0xC0000000
IMM_MN = {'cmp', 'test', 'and', 'or', 'xor', 'add', 'sub', 'imul', 'shl', 'shr', 'sar', 'mov', 'push', 'lea'}
FULL = {}
for _r in ('eax', 'ebx', 'ecx', 'edx', 'esi', 'edi', 'ebp', 'esp'):
    FULL[_r] = _r
for _a, _b in (('ax', 'eax'), ('al', 'eax'), ('ah', 'eax'), ('bx', 'ebx'), ('bl', 'ebx'), ('bh', 'ebx'),
               ('cx', 'ecx'), ('cl', 'ecx'), ('ch', 'ecx'), ('dx', 'edx'), ('dl', 'edx'), ('dh', 'edx'),
               ('si', 'esi'), ('di', 'edi')):
    FULL[_a] = _b


def source_objects():
    """mangled name -> image address of every `// RECOVERY_OBJECT:` binding in src/ (objects the
    measurement targets do not carry because no measured function references them by name)"""
    out = {}
    for path in pathlib.Path(R, 'src').rglob('*.cpp'):
        for m in re.finditer(r'// RECOVERY_OBJECT: \S+ 0x([0-9A-Fa-f]{8})\s.*?(\?\S+)',
                             path.read_text(encoding='latin-1')):
            out.setdefault(m.group(2), int(m.group(1), 16))
    # plain globals whose declaration or definition carries the address as a trailing comment
    # (`extern RulesResourceinfo ResourceInfo[N];  // 0x00945F50`), keyed by the bare identifier
    for path in list(pathlib.Path(R, 'src').rglob('*.h')) + list(pathlib.Path(R, 'src').rglob('*.cpp')):
        for m in re.finditer(r'^\s*(?:extern\s+)?[\w:<>,\s\*]+?\b([A-Za-z_]\w*)\s*(?:\[[^\]]*\])*\s*;'
                             r'\s*//\s*(?:@\s*)?0x([0-9A-Fa-f]{8})\b',
                             path.read_text(encoding='latin-1'), re.M):
            out.setdefault('=' + m.group(1), int(m.group(2), 16))
    return out


def object_address(name):
    if name in OBJECTS:
        return OBJECTS[name]
    m = re.match(r'\?([A-Za-z_]\w*)@@3', name)
    return OBJECTS.get('=' + m.group(1)) if m else None


OBJECTS = source_objects()


class Namer:
    def __init__(self, targets):
        known = dict(OBJECTS)
        known.update(targets.external)
        pairs = sorted(set((a, n) for n, a in known.items() if IMGLO <= a < IMGHI))
        self.addrs = [a for a, _ in pairs]
        self.names = [n for _, n in pairs]

    def big(self, i):
        """the bound symbol before an address is an aggregate the address may lie inside"""
        return i + 1 < len(self.addrs) and self.addrs[i + 1] - self.addrs[i] > 64

    def __call__(self, address):
        if address >= FAKE:
            return _TL.names.get(address & ~0xFFFFF, '?fake%X' % address), (address & 0xFFFFF) - 0x80000
        if address in IMPORTS:
            return 'imp:' + IMPORTS[address], 0
        i = bisect.bisect_right(self.addrs, address) - 1
        if 0x401000 <= address < 0x664000 and not (i >= 0 and self.addrs[i] == address):
            return '?code', 0   # switch tables, EH funclets: our side names them by section-local labels
        t = cstr(address) if 0x664000 <= address < 0x9C3000 else None
        if t is not None:
            return 'str:' + t[:40], 0
        if i >= 0 and self.addrs[i] == address:
            return self.names[i], 0
        if i < 0 or address - self.addrs[i] > 0x40000:
            return '0x%X' % address, 0
        return self.names[i], address - self.addrs[i]




def _imports():
    """IAT slot address -> imported function name"""
    out = {}
    b = fe._base
    rd = lambda va, n: fe.imgbytes(va, va + n)
    d = b + struct.unpack_from('<I', fe._img, fe._pe + 24 + 96 + 8)[0]
    while True:
        oft, _, _, nm, ft = struct.unpack('<5I', rd(d, 20))
        if not ft:
            break
        k = 0
        while True:
            t = struct.unpack('<I', rd(b + (oft or ft) + 4 * k, 4))[0]
            if not t:
                break
            if not t & 0x80000000:
                raw = rd(b + t + 2, 80)
                out[b + ft + 4 * k] = raw.split(b'\0')[0].decode('latin-1')
            k += 1
        d += 20
    return out


IMPORTS = _imports()


def cstr(va):
    """printable NUL-terminated string at an image address, or None"""
    try:
        raw = fe.imgbytes(va, va + 256)
    except Exception:
        return None
    if not raw or b'\0' not in raw:
        return None
    t = raw.split(b'\0')[0]
    if len(t) <= 3 and va % 4 == 0 and 0x401000 <= struct.unpack_from('<I', raw)[0] < 0x664000:
        return None   # a code pointer (vtable slot), not text
    if not t or any(c < 9 or (13 < c < 32) for c in t):
        return None
    return t.decode('latin-1')
import threading
_TL = threading.local()   # per-thread, reset per compiled function: fake address -> unbound name


def fake(name, addend):
    idx, names = _TL.idx, _TL.names
    if name not in idx:
        idx[name] = FAKE + (len(idx) + 1) * 0x100000
        names[idx[name]] = name
    return idx[name] + 0x80000 + addend


def is_addr(v):
    return IMGLO <= v < IMGHI or FAKE <= v < 0xE0000000


def rn(r):
    return MD.reg_name(r) if r else None


def collect(code, org, namer):
    """-> dict F: {(type, off): [org addr]}, G: {(sym, off)}, I: {value: [org addr]}"""
    F, G, I = defaultdict(list), defaultdict(list), defaultdict(list)
    typ = {'ecx': ('this', 0)}
    for ins in MD.disasm(code, org):
        mn = ins.mnemonic
        ops = list(ins.operands)
        # memory operands
        for op in ops:
            if op.type != x86.X86_OP_MEM:
                continue
            b, ix, d = rn(op.mem.base), rn(op.mem.index), op.mem.disp & 0xFFFFFFFF
            if op.mem.segment and rn(op.mem.segment) == 'fs':
                continue
            if b in ('ebp', 'esp'):
                continue
            if is_addr(d):
                s, k = namer(d)
                if mn.startswith('f') and not b and not ix and d < FAKE and op.size in (4, 8) \
                        and (k or s.startswith('0x')):
                    s, k = 'flt:' + fe.imgbytes(d, d + op.size).hex(), 0
                if s == '?code':
                    continue
                if ix and (op.mem.scale > 1 or b):
                    # [b + i*s + G + k]: k mixes the field with a folded index (VC6 biases either side): the
                    # global only
                    G[(s, None)].append(ins.address)
                    continue
                G[(s, k)].append(ins.address)
                if b or ix:
                    F[('[]' + s, k)].append(ins.address)
                else:
                    F[(s, k)].append(ins.address)
                continue
            if b is None:
                continue
            t = typ.get(b)
            sd = d - 0x100000000 if d & 0x80000000 else d
            if t is None:
                F[('?', sd)].append(ins.address)
            else:
                F[(t[0], t[1] + sd)].append(ins.address)
        # immediates
        for op in ops:
            if op.type != x86.X86_OP_IMM:
                continue
            v = op.imm & 0xFFFFFFFF
            if is_addr(v):
                if mn not in ('call',) and not mn.startswith('j'):
                    s, k = namer(v)
                    if s != '?code':
                        G[(s, k)].append(ins.address)
                continue
            if mn not in IMM_MN:
                continue
            if mn in ('add', 'sub') and ops[0].type == x86.X86_OP_REG and rn(ops[0].reg) == 'esp':
                continue
            if ops[0].type == x86.X86_OP_MEM and rn(ops[0].mem.base) == 'ebp' and ops[0].mem.disp == -4:
                continue   # EH state
            bits = 8 * (op.size or 4)
            v &= (1 << bits) - 1
            if ops[0].type == x86.X86_OP_REG and rn(ops[0].reg) in ('ah', 'bh', 'ch', 'dh'):
                v <<= 8   # `test ah, 4` is the dword test of 0x400
                bits = 16
            if v >> (bits - 1):
                v |= 0xFFFFFFFF ^ ((1 << bits) - 1)   # sign-extend to 32 bits
            # width-free key: a byte/word mask and its full-register spelling are one constant
            if v & 0xFFFFFF00 in (0, 0xFFFFFF00):
                v &= 0xFF
            elif v & 0xFFFF0000 in (0, 0xFFFF0000):
                v &= 0xFFFF
            I[v].append(ins.address)
        # taint update (linear)
        if mn == 'call':
            for r in ('eax', 'ecx', 'edx'):
                typ.pop(r, None)
            continue
        if not ops or ops[0].type != x86.X86_OP_REG:
            continue
        dst = FULL.get(rn(ops[0].reg))
        if dst is None:
            continue
        new = None
        if mn == 'mov' and len(ops) == 2:
            src = ops[1]
            if src.type == x86.X86_OP_REG and rn(src.reg) == FULL.get(rn(src.reg)):
                new = typ.get(rn(src.reg))
            elif src.type == x86.X86_OP_IMM and is_addr(src.imm & 0xFFFFFFFF):
                s, k = namer(src.imm & 0xFFFFFFFF)
                new = (s, k)
            elif src.type == x86.X86_OP_MEM and not src.mem.base and not src.mem.index \
                    and is_addr(src.mem.disp & 0xFFFFFFFF):
                s, k = namer(src.mem.disp & 0xFFFFFFFF)
                new = ('*%s+%d' % (s, k) if k else '*' + s, 0)
        elif mn == 'lea' and len(ops) == 2:
            m = ops[1].mem
            d = m.disp & 0xFFFFFFFF
            if is_addr(d):
                s, k = namer(d)
                if mn.startswith('f') and not b and not ix and d < FAKE and op.size in (4, 8) \
                        and (k or s.startswith('0x')):
                    s, k = 'flt:' + fe.imgbytes(d, d + op.size).hex(), 0
                new = (s, k)
            elif m.base and not m.index and typ.get(rn(m.base)):
                t = typ[rn(m.base)]
                new = (t[0], t[1] + m.disp)
        elif mn == 'add' and len(ops) == 2 and ops[1].type == x86.X86_OP_IMM:
            v = ops[1].imm & 0xFFFFFFFF
            if is_addr(v):
                s, k = namer(v)
                new = (s, k)
            elif typ.get(dst):
                new = None   # pointer stepping: drop (an IV, not a field)
        if new is None:
            typ.pop(dst, None)
        else:
            typ[dst] = new
    return F, G, I


def ours_code(c, sym, targets, host):
    s = c.function(sym)
    sec = c.sections[s.section - 1]
    data = bytearray(sec.data)
    _TL.idx, _TL.names = {}, {}
    unbound = Counter()
    for va, index, kind in sec.relocations:
        if kind != 0x0006:
            continue
        tgt = c.symbols.get(index)
        if tgt is None:
            continue
        addend = int.from_bytes(data[va:va + 4], 'little', signed=True)
        ref = CodeReference(va, False, tgt.name, tgt.storage, addend, None)
        a = targets.address(ref, host)
        if a is None and not tgt.name.startswith(('$', '??_C@', '__real@')):
            a = object_address(tgt.name)
        if a is None and tgt.name.startswith('__imp_'):
            nm = re.sub(r'@\d+$', '', tgt.name[6:]).lstrip('_')
            a = next((k for k, v in IMPORTS.items() if v == nm), None)
            addend = 0 if a else addend
        if a is None and (tgt.name.startswith('$SG') or tgt.name.startswith('??_C@')):
            lit = string_literal_bytes(c, tgt, addend)
            if lit:
                t = lit.rstrip(b'\0').decode('latin-1')
                v = fake('str:' + t[:40], 0)
                data[va:va + 4] = struct.pack('<I', v & 0xFFFFFFFF)
                continue
        if a is None and tgt.name.startswith('__real@'):
            n = 4 if tgt.name.startswith('__real@4@') else 8
            raw = c.sections[tgt.section - 1].data[tgt.value:tgt.value + n] if tgt.section > 0 else b''
            if len(raw) == n:
                v = fake('flt:' + raw.hex(), 0)
                data[va:va + 4] = struct.pack('<I', v & 0xFFFFFFFF)
                continue
        if a is None:
            if tgt.name.startswith('$L') or tgt.storage == 3 and tgt.section == s.section:
                v = 0   # local labels / switch tables
            else:
                v = fake(tgt.name, addend)
                if not tgt.name.startswith('??_C@'):
                    unbound[tgt.name] += 1
        else:
            v = a + addend
        data[va:va + 4] = struct.pack('<I', v & 0xFFFFFFFF)
    calls = {}
    for va, index, kind in sec.relocations:
        if kind == 0x0014:
            tgt = c.symbols.get(index)
            if tgt is not None and va >= 1 and data[va - 1] == 0xE8:
                ref = CodeReference(va, True, tgt.name, tgt.storage, -4, None)
                try:
                    a = targets.address(ref, host)
                except Exception:
                    a = None
                calls[va - 1] = (a, tgt.name)
            data[va:va + 4] = b'\0\0\0\0'
    _TL.calls = calls
    return bytes(data), unbound


def call_refs(rows, lo, namer):
    """C: fe-aligned `call <ext>` rows whose direct targets differ (a sibling function with the same
    signature, a wrong overload). Our target is the bound image address of the called symbol; an
    unbound callee is compared by name against the image target's bound name."""
    calls = getattr(_TL, 'calls', {})
    ctx = int(os.environ.get('SS_CTX', '0'))
    win = int(os.environ.get('SS_CALLWIN', '8'))
    # per side, the targets of every direct call, in order, to drop a swap of two nearby calls
    img_t, our_t = [], []
    for flag, o, cc in rows:
        if o is not None and o[1].startswith('call') and fe.imgbytes(lo + o[0], lo + o[0] + 1) == b'\xe8':
            img_t.append(img_call(lo + o[0]))
        else:
            img_t.append(None)
        our_t.append(calls.get(cc[0]) if cc is not None else None)
    out = []
    for n, (flag, o, cc) in enumerate(rows):
        if o is None or cc is None or o[1] != cc[1] or not o[1].startswith('call') or '<ext>' not in o[1]:
            continue
        ti, to = img_t[n], our_t[n]
        if ti is None or to is None:
            continue
        a, name = to
        if a is not None and a == ti:
            continue
        ni = namer(ti)[0] if namer(ti)[1] == 0 else '0x%X' % ti
        if a is None and (name == ni or name.lstrip('_').split('@')[0] == ni.lstrip('_?').split('@')[0]):
            continue
        if ctx and not all(0 <= m < len(rows) and rows[m][0] == '  '
                           for m in list(range(n - ctx, n)) + list(range(n + 1, n + ctx + 1))):
            continue
        near = range(max(0, n - win), min(len(rows), n + win + 1))
        if any(img_t[m] is not None and a is not None and img_t[m] == a for m in near) and \
                any(our_t[m] is not None and our_t[m][0] == ti for m in near):
            continue   # two nearby calls in swapped order
        out.append(('%X' % (lo + o[0]), '%x' % cc[0], '0x%X %s' % (ti, ni[:50]),
                    ('0x%X ' % a if a is not None else 'unbound ') + name[:50]))
    return out


def img_call(at):
    rel = struct.unpack('<i', fe.imgbytes(at + 1, at + 5))[0]
    return (at + 5 + rel) & 0xFFFFFFFF


def addr_operands(code, org, at):
    """address-valued operands (disp or imm) of the instruction at `at`"""
    out = []
    for ins in MD.disasm(code[at - org:at - org + 16], at, 1):
        for op in ins.operands:
            if op.type == x86.X86_OP_MEM:
                v = op.mem.disp & 0xFFFFFFFF
            elif op.type == x86.X86_OP_IMM and ins.mnemonic != 'call' and not ins.mnemonic.startswith('j'):
                v = op.imm & 0xFFFFFFFF
            else:
                continue
            if is_addr(v):
                out.append(v)
    return out


def aligned_refs(c, text, addr, lo, code, namer):
    """R: rows fe pairs with identical text (relocations masked) whose relocated targets differ: the
    refscan question asked of the aligned rows of a non-exact function (a wrong field of a global
    aggregate, a wrong global, a wrong constant index)"""
    r = fe.evaluate(c, text, addr)
    if 'error' in r:
        return []
    out = []
    # every address operand of each side, named: a REF row whose image target ours also uses
    # elsewhere and whose our target the image also uses is a scheduling swap, not a wrong reference
    alli, allo = Counter(), Counter()
    for flag, o, cc in r['rows']:
        if o is not None:
            for v in addr_operands(fe.imgbytes(lo + o[0], lo + o[0] + 16), lo + o[0], lo + o[0]):
                alli[namer(v)] += 1
        if cc is not None:
            for v in addr_operands(code, 0, cc[0]):
                allo[namer(v)] += 1
    mode = os.environ.get('SS_REF', '')
    block = mode == 'block' or mode.startswith('win')
    if block:
        img_end = max(lo + o[0] + o[2] for _, o, _c in r['rows'] if o is not None)
        our_end = max(cc[0] + cc[2] for _, _o, cc in r['rows'] if cc is not None)
        win = int(mode[3:] or 12) if mode.startswith('win') else None
        bi = block_operands(fe.imgbytes(lo, img_end), lo, namer, win)
        bo = block_operands(code[:our_end], 0, namer, win)
    ctx = int(os.environ.get('SS_CTX', '0'))
    rows = r['rows']
    for n, (flag, o, cc) in enumerate(rows):
        if o is None or cc is None or o[1] != cc[1] or '<r>' not in o[1]:
            continue
        if ctx and not all(0 <= m < len(rows) and rows[m][0] == '  '
                           for m in list(range(n - ctx, n)) + list(range(n + 1, n + ctx + 1))):
            continue   # SS_CTX=k: only rows inside an agreeing run (k equal rows each side), not misalignment
        vi = addr_operands(fe.imgbytes(lo + o[0], lo + o[0] + 16), lo + o[0], lo + o[0])
        vo = addr_operands(code, 0, cc[0])
        if len(vi) != len(vo):
            continue
        for a, b in zip(vi, vo):
            if b >= FAKE:
                continue   # our side unbound: no address to compare
            na, nb = namer(a), namer(b)
            if a == b or na == nb:
                continue
            if block:
                # a load-order swap: the image's block also touches our target and our block the image's
                si, so = bi.get(lo + o[0], ()), bo.get(cc[0], ())
                if nb in si and na in so:
                    continue
            elif os.environ.get('SS_ALLREF') is None and alli[na] == allo[na] and alli[nb] == allo[nb]:
                continue
            out.append(('%X' % (lo + o[0]), '%x' % cc[0], o[1], '%s%+#x' % na, '%s%+#x' % nb))
    return out


def block_operands(code, org, namer, win=None):
    """instruction address -> set of named address operands of its basic block (split at jumps, rets and
    local jump targets), or with `win` of the +-win instructions around it"""
    ins = list(MD.disasm(code, org))
    if win:
        names = []
        for i in ins:
            cur = set()
            for op in i.operands:
                if op.type == x86.X86_OP_MEM:
                    v = op.mem.disp & 0xFFFFFFFF
                elif op.type == x86.X86_OP_IMM and i.mnemonic != 'call' and not i.mnemonic.startswith('j'):
                    v = op.imm & 0xFFFFFFFF
                else:
                    continue
                if is_addr(v):
                    cur.add(namer(v))
            names.append(cur)
        return {i.address: set().union(*names[max(0, k - win):k + win + 1]) for k, i in enumerate(ins)}
    starts = {org}
    for i in ins:
        if i.mnemonic.startswith('j') or i.mnemonic.startswith('ret'):
            starts.add(i.address + i.size)
            if i.operands and i.operands[0].type == x86.X86_OP_IMM:
                starts.add(i.operands[0].imm & 0xFFFFFFFF)
    out, cur = {}, None
    members = []
    for i in ins:
        if i.address in starts or cur is None:
            cur = set()
        for op in i.operands:
            if op.type == x86.X86_OP_MEM:
                v = op.mem.disp & 0xFFFFFFFF
            elif op.type == x86.X86_OP_IMM and i.mnemonic != 'call' and not i.mnemonic.startswith('j'):
                v = op.imm & 0xFFFFFFFF
            else:
                continue
            if is_addr(v):
                cur.add(namer(v))
        out[i.address] = cur
    return out


def diff_sets(Fi, Fo):
    oi = {k: v for k, v in Fi.items() if k not in Fo}
    oo = {k: v for k, v in Fo.items() if k not in Fi}
    return oi, oo


def scan_unit(unit, addrs, pfx, targets, namer, near):
    text = open(R + '/' + unit, encoding='latin-1').read()
    c, err = fe.compile_unit(unit, R + '/' + unit, pfx)
    if c is None:
        return [dict(unit=unit, addr='?', error='compile failed')]
    host = pathlib.Path(R + '/' + unit).resolve()
    out, contexts = [], {}
    for addr in addrs:
        if re.search(r'// ORIGINAL: 0x%08X [^\n]*\n// RECOVERY_FRAGMENT' % addr, text):
            continue
        idn = fe.ident(text, addr)
        if not idn:
            continue
        sym, lo, hi = idn
        m = re.search(r'// ORIGINAL: 0x%08X [^\n]*\n((?://(?! ORIGINAL)[^\n]*\n)*)' % addr, text)
        cc_ = c
        mc = m and re.search(r'// compile_context (\S+)', m.group(1))
        if mc and mc.group(1) != unit:
            if mc.group(1) not in contexts:
                contexts[mc.group(1)] = fe.compile_unit(mc.group(1), R + '/' + mc.group(1), pfx)[0]
            cc_ = contexts[mc.group(1)]
            if cc_ is None:
                continue
        try:
            code, unb = ours_code(cc_, sym, targets, host)
        except Exception:
            continue
        # trim our code the way fe does (switch tables after the code)
        rows = fe.dis(cc_.sections[cc_.function(sym).section - 1].data, 0, cc_.sections[cc_.function(sym).section - 1].relocations,
                      selfsec=cc_.function(sym).section, syms=cc_.symbols)
        end = rows[-1][0] + rows[-1][2] if rows else len(code)
        img = fe.imgdis(lo, hi)
        iend = img[-1][0] + img[-1][2] if img else hi - lo
        Fi, Gi, Ii = collect(fe.imgbytes(lo, lo + iend), lo, namer)
        Fo, Go, Io = collect(code[:end], 0, namer)
        res = dict(unit=unit, addr='%08X' % addr, sym=sym, unbound=dict(unb))
        res['R'] = aligned_refs(cc_, text, addr, lo, code, namer)
        if os.environ.get('SS_CALLS'):
            r_ = fe.evaluate(cc_, text, addr)
            res['C'] = call_refs(r_['rows'], lo, namer) if 'error' not in r_ else []
        fi, fo = diff_sets(Fi, Fo)
        # pairs of the same type
        pairs = []
        for (ti, di), ai in fi.items():
            for (to, do), ao in fo.items():
                if ti == to and abs(di - do) <= near:
                    pairs.append((ti, di, do, ['%X' % x for x in ai[:3]], ['%x' % x for x in ao[:3]]))
        res['pairs'] = pairs
        res['F_img'] = [(t, d, ['%X' % x for x in a[:3]]) for (t, d), a in sorted(fi.items(), key=str)]
        res['F_ours'] = [(t, d, ['%x' % x for x in a[:3]]) for (t, d), a in sorted(fo.items(), key=str)]
        gi, go = diff_sets(Gi, Go)
        res['G_img'] = [('%s%+#x' % k if k[1] else k[0] + ('[*]' if k[1] is None else ''), ['%X' % x for x in a[:3]]) for k, a in sorted(gi.items(), key=str)]
        res['G_ours'] = [('%s%+#x' % k if k[1] else k[0] + ('[*]' if k[1] is None else ''), ['%x' % x for x in a[:3]]) for k, a in sorted(go.items(), key=str)
                         if k[0] not in unb]
        # an image-only address that is no bound symbol's start may be what an unbound name of ours is
        res['G_img'] = [(n + ('?' if unb and ('+' in n or n.startswith('0x')) else ''), at) for n, at in res['G_img']]
        res['F_ours'] = [x for x in res['F_ours'] if x[0].lstrip('[]*') not in unb]
        ii, io = diff_sets(Ii, Io)
        res['I_img'] = [(k, ['%X' % x for x in a[:3]]) for k, a in sorted(ii.items())]
        res['I_ours'] = [(k, ['%x' % x for x in a[:3]]) for k, a in sorted(io.items())]
        out.append(res)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='*')
    ap.add_argument('--addr', default='')
    ap.add_argument('--json')
    ap.add_argument('--near', type=int, default=0x40)
    ap.add_argument('--claimed', action='store_true')
    ap.add_argument('--jobs', type=int, default=int(os.environ.get('FE_JOBS', '5')))
    a = ap.parse_args()
    base = int(os.environ.get('FE_PFX_BASE', '21'))
    want = {int(x, 16) for x in a.addr.split(',') if x}
    cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
    units = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/')]
    if a.units:
        units = [u for u in units if u in a.units or os.path.basename(u) in a.units]
    work = []
    for u in units:
        try:
            t = open(R + '/' + u, encoding='latin-1').read()
        except OSError:
            continue
        addrs = [int(m.group(1), 16) for m in MARK.finditer(t)
                 if (a.claimed or want or 'BYTE_EXACT' not in m.group(2))
                 and (not want or int(m.group(1), 16) in want)]
        if addrs:
            work.append((u, addrs))
    root = pathlib.Path(R).resolve()
    targets = measurement_reference_targets(root, Measurement.read(root))
    namer = Namer(targets)
    fe.compile_cmd(work[0][0])

    def lane(k):
        res = []
        for i in range(k, len(work), a.jobs):
            u, ad = work[i]
            res += scan_unit(u, ad, base + k, targets, namer, a.near)
        return res
    found = []
    with ThreadPoolExecutor(a.jobs) as ex:
        for part in ex.map(lane, range(a.jobs)):
            found += part
    found.sort(key=lambda x: (x['unit'], x['addr']))
    if a.json:
        json.dump(found, open(a.json, 'w'), indent=0)
    for f in found:
        if 'error' in f:
            print('ERR', f['unit'], f['error'])
            continue
        if not (f['R'] or f.get('C') or f['pairs'] or f['F_img'] or f['G_img'] or f['I_img'] or f['F_ours'] or f['G_ours'] or f['I_ours']):
            continue
        print('==', f['addr'], f['unit'], f['sym'][:60])
        for x in f['R']:
            print('   REF  img %s ours %s %-40s img %s ours %s' % (x[0], x[1], x[2][:40], x[3], x[4]))
        for x in f.get('C', ()):
            print('   CALL img %s ours %s img %s ours %s' % x)
        for p in f['pairs']:
            print('   PAIR %-28s img %+#x %s  ours %+#x %s' % (p[0][:28], p[1], ','.join(p[3]), p[2], ','.join(p[4])))
        for key in ('F_img', 'F_ours'):
            for t, d, at in f[key]:
                print('   %-6s %-28s %+#x %s' % (key, t[:28], d, ','.join(at)))
        for key in ('G_img', 'G_ours'):
            for s, at in f[key]:
                print('   %-6s %s %s' % (key, s[:50], ','.join(at)))
        for key in ('I_img', 'I_ours'):
            if f[key]:
                print('   %-6s %s' % (key, ' '.join('%#x@%s' % (v if v >= 0 else v, at[0]) for v, at in f[key])))
        if f['unbound']:
            print('   ours-unbound', ' '.join(f['unbound']))
    print('functions', len(found), 'with pairs', sum(1 for f in found if f.get('pairs')))


if __name__ == '__main__':
    main()
