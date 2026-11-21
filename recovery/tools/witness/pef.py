"""Read a classic Mac OS PEF container (the Mac SMAC/SMACX Carbon builds).

The Mac port is a second compile (CodeWarrior, PowerPC) of the game's source.
Its traceback tables name every function with its C++ signature, so it is
primary evidence for names, helpers and call structure. Witness binaries stay
in ignored .opensmacx/witness/; nothing derived from them is committed.

    from witness.pef import Pef
    p = Pef('.opensmacx/witness/mac/carbon/SMACX Carbon')
    p.functions        # [(offset, size, mangled_name)] sorted by offset
    p.calls(fn)        # ordered callee labels of one function
"""
import bisect
import re
import struct

_IDENT = re.compile(rb'[A-Za-z_~<.$][A-Za-z0-9_~<>,@$:.*&\[\]\(\) -]*\Z')


class Pef:
    def __init__(self, path):
        self.path = path
        d = self.raw = open(path, 'rb').read()
        if d[:12] != b'Joy!peffpwpc':
            raise ValueError('not a PowerPC PEF container')
        nsec = struct.unpack_from('>H', d, 32)[0]
        self.sections = []
        for i in range(nsec):
            _, addr, total, unpacked, packed, off, kind, _, _, _ = struct.unpack_from('>iIIIIIBBBB', d, 40 + 28 * i)
            self.sections.append(dict(kind=kind, addr=addr, total=total, unpacked=unpacked, packed=packed, off=off))
        code = [s for s in self.sections if s['kind'] == 0][0]
        self.code = d[code['off']:code['off'] + code['packed']]
        self.code_index = self.sections.index(code)
        data = [s for s in self.sections if s['kind'] in (1, 2)][0]
        self.data_index = self.sections.index(data)
        blob = d[data['off']:data['off'] + data['packed']]
        self.data = bytearray(_unpack_pidata(blob) if data['kind'] == 2 else blob)
        self.data.extend(b'\0' * (data['total'] - len(self.data)))
        self._loader()
        self.functions = sorted(self._traceback_names())
        self._starts = [f[0] for f in self.functions]
        self._by_start = {f[0]: f for f in self.functions}

    # -- loader section: imports, relocations, entry points -----------------
    def _loader(self):
        d = self.raw
        lsec = [s for s in self.sections if s['kind'] == 4][0]
        lo = lsec['off']
        (main_sec, main_off, _, _, _, _, nlibs, nimp, nrelsec, reloc_off, str_off, _, _, _) = \
            struct.unpack_from('>iIiIiIIIIIIIII', d, lo)
        p = lo + 56
        self.libraries = []
        for _ in range(nlibs):
            name_off, _, _, count, first, _, _, _ = struct.unpack_from('>IIIIIBBH', d, p)
            p += 24
            self.libraries.append((_cstr(d, lo + str_off + name_off), first, count))
        self.imports = []
        for _ in range(nimp):
            w = struct.unpack_from('>I', d, p)[0]
            p += 4
            self.imports.append(_cstr(d, lo + str_off + (w & 0xFFFFFF)))
        # Relocation headers: which section, how many 16-bit instructions.
        headers = []
        for _ in range(nrelsec):
            sec, _, count, first = struct.unpack_from('>HHII', d, p)
            p += 12
            headers.append((sec, count, first))
        self.import_slots = {}   # data offset -> import name
        self.code_slots = set()  # data offsets relocated by the code section address
        for sec, count, first in headers:
            if sec != self.data_index:
                continue
            instr = [struct.unpack_from('>H', d, lo + reloc_off + first + 2 * i)[0] for i in range(count)]
            self._run_relocations(instr)
        self.toc = None
        if main_sec == self.data_index:
            # main's transition vector: [code offset, TOC]; the TOC word is data-relative.
            self.main_code = struct.unpack_from('>I', self.data, main_off)[0]
            self.toc = struct.unpack_from('>I', self.data, main_off + 4)[0]

    def _run_relocations(self, instr):
        pos = 0
        imp_index = 0
        sect_c, sect_d = self.code_index, self.data_index
        i = 0

        def word_add(kind):
            if kind == 'C':
                self.code_slots.add(pos)

        def run(k, n):
            nonlocal pos, imp_index, sect_c, sect_d
            if k == 'skip':
                pos += 4 * n
            elif k == 'C':
                for _ in range(n):
                    word_add('C')
                    pos += 4
            elif k == 'D':
                pos += 4 * n
            elif k == 'tv12':
                for _ in range(n):
                    word_add('C')
                    pos += 12
            elif k == 'tv8':
                for _ in range(n):
                    word_add('C')
                    pos += 8
            elif k == 'vt8':
                pos += 8 * n
            elif k == 'imp':
                for _ in range(n):
                    self.import_slots[pos] = self.imports[imp_index]
                    imp_index += 1
                    pos += 4

        history = []
        while i < len(instr):
            w = instr[i]
            start = i
            i += 1
            if w >> 14 == 0b00:                       # RelocBySectDWithSkip
                skip, cnt = (w >> 6) & 0xFF, w & 0x3F
                run('skip', skip)
                run('D', cnt)
            elif w >> 13 == 0b010:                    # RelocGroup
                sub, n = (w >> 9) & 0xF, (w & 0x1FF) + 1
                run({0: 'C', 1: 'D', 2: 'tv12', 3: 'tv8', 4: 'vt8', 5: 'imp'}[sub], n)
            elif w >> 12 == 0b0110:                   # RelocSm*
                sub, idx = (w >> 9) & 0x7, w & 0x1FF
                if sub == 0:
                    self.import_slots[pos] = self.imports[idx]
                    imp_index = idx + 1
                    pos += 4
                elif sub == 1:
                    sect_c = idx
                elif sub == 2:
                    sect_d = idx
                elif sub == 3:
                    pos += 4
            elif w >> 12 == 0b1000:                   # RelocIncrPosition
                pos += (w & 0xFFF) + 1
            elif w >> 12 == 0b1001:                   # RelocSmRepeat
                chunks, repeat = ((w >> 8) & 0xF) + 1, (w & 0xFF) + 1
                block = history[-chunks:]
                for _ in range(repeat):
                    for b in block:
                        self._replay(b, run)
                continue
            elif w >> 10 == 0b101000:                 # RelocSetPosition
                pos = ((w & 0x3FF) << 16) | instr[i]
                i += 1
            elif w >> 10 == 0b101001:                 # RelocLgByImport
                idx = ((w & 0x3FF) << 16) | instr[i]
                i += 1
                self.import_slots[pos] = self.imports[idx]
                imp_index = idx + 1
                pos += 4
            elif w >> 10 == 0b101100:                 # RelocLgRepeat
                chunks, repeat = ((w >> 6) & 0xF) + 1, ((w & 0x3F) << 16) | instr[i]
                i += 1
                block = history[-chunks:]
                for _ in range(repeat):
                    for b in block:
                        self._replay(b, run)
                continue
            elif w >> 10 == 0b101101:                 # RelocLgSetOrBySection
                i += 1
                pos += 4 if ((w >> 6) & 0xF) == 0 else 0
            history.append(instr[start:i])

    def _replay(self, words, run):
        w = words[0]
        if w >> 14 == 0b00:
            run('skip', (w >> 6) & 0xFF)
            run('D', w & 0x3F)
        elif w >> 13 == 0b010:
            run({0: 'C', 1: 'D', 2: 'tv12', 3: 'tv8', 4: 'vt8', 5: 'imp'}[(w >> 9) & 0xF], (w & 0x1FF) + 1)

    # -- traceback tables ---------------------------------------------------
    def _traceback_names(self):
        code = self.code
        out = []
        for m in re.finditer(rb'\x00\x00\x00\x00', code):
            tb = m.start() + 4
            if tb % 4 or tb + 8 > len(code):
                continue
            ver, lang, f1, f2, _, _, fixed, flt = struct.unpack_from('>8B', code, tb)
            if ver != 0 or lang > 13 or not f2 & 0x40:
                continue
            p = tb + 8
            if fixed + (flt >> 1) > 0:
                p += 4
            tboff = None
            if f1 & 0x20:
                if p + 4 > len(code):
                    continue
                tboff = struct.unpack_from('>I', code, p)[0]
                p += 4
            if f2 & 0x80:
                p += 4
            if f1 & 0x08:
                if p + 4 > len(code):
                    continue
                n = struct.unpack_from('>I', code, p)[0]
                if n > 64:
                    continue
                p += 4 + 4 * n
            if p + 2 > len(code):
                continue
            ln = struct.unpack_from('>H', code, p)[0]
            if not 0 < ln < 400 or p + 2 + ln > len(code):
                continue
            name = code[p + 2:p + 2 + ln]
            if not _IDENT.match(name) or tboff is None or tboff > m.start() or tboff % 4:
                continue
            out.append((m.start() - tboff, tboff, name.decode('latin-1')))
        return out

    # -- code queries -------------------------------------------------------
    def function_at(self, offset):
        i = bisect.bisect_right(self._starts, offset) - 1
        if i >= 0:
            f = self.functions[i]
            if f[0] <= offset < f[0] + f[1]:
                return f
        return None

    def glue_target(self, offset):
        """Name of the import a cross-TOC glue stub at `offset` reaches, if any."""
        if offset + 4 > len(self.code):
            return None
        w = struct.unpack_from('>I', self.code, offset)[0]
        # lwz r12, disp(r2)
        if w >> 16 == 0x8182 and self.toc is not None:
            disp = w & 0xFFFF
            if disp & 0x8000:
                disp -= 0x10000
            slot = self.toc + disp
            name = self.import_slots.get(slot)
            if name:
                return name
        return None

    def calls(self, fn, unnamed=True):
        """Ordered `bl` targets of one function: demangled names, import names,
        or `@0xOFFSET` for unnamed code (statically linked runtime)."""
        start, size, _ = fn
        out = []
        for pc in range(start, start + size, 4):
            w = struct.unpack_from('>I', self.code, pc)[0]
            if w >> 26 != 18 or w & 3 != 1:
                continue
            li = w & 0x03FFFFFC
            if li & 0x02000000:
                li -= 0x04000000
            target = pc + li
            f = self._by_start.get(target)
            if f:
                out.append(demangle(f[2]))
                continue
            g = self.glue_target(target)
            if g:
                out.append(g)
            elif unnamed:
                out.append('@0x%X' % target)
        return out


def _cstr(d, p):
    return d[p:d.index(b'\0', p)].decode('latin-1')


def _varint(b, i):
    v = 0
    while True:
        c = b[i]
        i += 1
        v = (v << 7) | (c & 0x7F)
        if not c & 0x80:
            return v, i


def _unpack_pidata(b):
    out = bytearray()
    i = 0
    while i < len(b):
        op, cnt = b[i] >> 5, b[i] & 0x1F
        i += 1
        if cnt == 0:
            cnt, i = _varint(b, i)
        if op == 0:
            out.extend(b'\0' * cnt)
        elif op == 1:
            out.extend(b[i:i + cnt])
            i += cnt
        elif op == 2:
            rep, i = _varint(b, i)
            block = b[i:i + cnt]
            i += cnt
            out.extend(block * (rep + 1))
        elif op in (3, 4):
            custom, i = _varint(b, i)
            rep, i = _varint(b, i)
            if op == 3:
                common = b[i:i + cnt]
                i += cnt
            else:
                common = b'\0' * cnt
            out.extend(common)
            for _ in range(rep):
                out.extend(b[i:i + custom])
                i += custom
                out.extend(common)
        else:
            raise ValueError('bad pidata opcode %d' % op)
    return bytes(out)


def demangle(name):
    """`.wrap_cent__6BufferFP4FontPcP7tagRECT` -> `Buffer::wrap_cent`.

    Only owner and base name: enough to match Windows markers. Template owners
    keep their CodeWarrior spelling (`ClassList<12StringStruct>`).
    """
    n = name.lstrip('.')
    m = re.match(r'^(__[a-z]+|.+?)__(?:(\d+)|(?=F)|(?=Q))', n)
    if not m:
        return n
    rest = n[m.end():]
    owner = rest[:int(m.group(2))] if m.group(2) else ''
    base = {'__ct': '<ctor>', '__dt': '<dtor>'}.get(m.group(1), m.group(1))
    return (owner + '::' if owner else '') + base
