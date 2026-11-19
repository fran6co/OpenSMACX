#!/usr/bin/env python3
"""typescan.py [unit ...] [--addr A[,A]] [--field SUBSTR] [--min N] [--uses] [--jobs N] [--json OUT]
Field-type scan: for every recovered function (exact and non-exact), find each member and global access
in our compile, name the field it reads, and classify how the IMAGE uses the same access:

  S  signed:   movsx; jl/jge/jg/jle/setl.. after a dword compare; cdq + idiv; idiv/sar on the value
  U  unsigned: movzx or xor r32,r32 + mov r8/r16 (or `and r32,0xff`); jb/jae/ja/jbe/setb..; xor edx + div;
               div/shr on the value
  W  width:    the image accesses the same operand with another width (byte/word/dword)

then compare the image's uses with the field's declared type in our source (from the object's CodeView
types and the decorated global names) and print the fields whose image uses contradict the declaration,
with counts (image uses in non-exact / exact functions), sample image addresses, and the casts on that
field in our source (WORKAROUND(type) lines and plain `(type)` casts).

How a use is attributed: the compile runs with /FAsc, so every instruction has its source line(s).
[reg+disp] operands are named by the identifiers on those lines that are members at offset `disp`
(flattened through nested structs and arrays); global operands (`?Bases@@3PAUBase@@A+0x1c`) are named
from the decorated global's struct type (addend modulo the struct size for arrays) or scalar type.
Byte and word fields are judged by their extension only (a narrow value is promoted to int, so its
compares are signed whatever its type); dword fields by compares, divides and shifts. A compare against
an unsigned other operand is unsigned whatever the field's type: read the verdict as a majority.

The image instruction for one of our accesses: the aligned row when fe.py's alignment pairs them, or in a
differing block the image instruction with the same operand shape (registers abstracted).

Default output: one row per field whose image uses imply the other signedness (or another width), sorted
by uses in non-exact functions: `FIELD <key> decl=<type> img S/U nonexact=a/b exact=c/d W=... casts=n`
followed by sample addresses and the cast lines. --uses prints every attributed use of the selected
fields (--field). Screen only; confirm with `recover measure`."""
import sys, os, re, json, struct, argparse, collections, subprocess
from concurrent.futures import ThreadPoolExecutor
T = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, T)
import fe

SIGNED_J = {'jl', 'jge', 'jg', 'jle', 'setl', 'setge', 'setg', 'setle', 'cmovl', 'cmovge', 'cmovg', 'cmovle'}
UNSIGNED_J = {'jb', 'jae', 'ja', 'jbe', 'setb', 'setae', 'seta', 'setbe', 'adc', 'sbb', 'jc', 'jnc'}
WIDTH = {'byte': 1, 'word': 2, 'dword': 4, 'qword': 8}
MEM = re.compile(r'(byte|word|dword|qword) ptr (?:\w+:)?\[([^\]]*)\]')
REG32 = {'eax', 'ebx', 'ecx', 'edx', 'esi', 'edi', 'ebp', 'esp'}
SUB = {'al': 'eax', 'ah': 'eax', 'ax': 'eax', 'bl': 'ebx', 'bh': 'ebx', 'bx': 'ebx', 'cl': 'ecx', 'ch': 'ecx',
       'cx': 'ecx', 'dl': 'edx', 'dh': 'edx', 'dx': 'edx', 'si': 'esi', 'di': 'edi'}


def full(r):
    return SUB.get(r, r)


# ---------------------------------------------------------------- CodeView types (lenient, screen only)
PRIM = {0x10: (1, 'S', 'signed char'), 0x20: (1, 'U', 'unsigned char'), 0x70: (1, 'S', 'char'),
        0x68: (1, 'S', 'int8'), 0x69: (1, 'U', 'uint8'), 0x30: (1, 'U', 'bool'),
        0x11: (2, 'S', 'short'), 0x21: (2, 'U', 'unsigned short'), 0x72: (2, 'S', 'int16'), 0x73: (2, 'U', 'uint16'),
        0x12: (4, 'S', 'long'), 0x22: (4, 'U', 'unsigned long'), 0x74: (4, 'S', 'int'), 0x75: (4, 'U', 'unsigned int'),
        0x13: (8, 'S', '__int64'), 0x23: (8, 'U', 'unsigned __int64'), 0x40: (4, 'F', 'float'), 0x41: (8, 'F', 'double'),
        0x31: (2, 'U', 'bool16'), 0x32: (4, 'U', 'bool32')}


class Cur:
    def __init__(s, d, at=0):
        s.d, s.at = d, at

    def u(s, f):
        v = struct.unpack_from(f, s.d, s.at)[0]
        s.at += struct.calcsize(f)
        return v

    def num(s):
        v = s.u('<H')
        if v < 0x8000:
            return v
        f = {0x8000: '<b', 0x8001: '<h', 0x8002: '<H', 0x8003: '<i', 0x8004: '<I', 0x8009: '<q', 0x800A: '<Q'}[v]
        return s.u(f)

    def name(s):
        n = s.d[s.at]
        v = s.d[s.at + 1:s.at + 1 + n].decode('latin-1')
        s.at += 1 + n
        return v

    def pad(s):
        if s.at < len(s.d) and s.d[s.at] >= 0xF0:
            s.at += s.d[s.at] - 0xF0


class Types:
    """Classes by name -> (size, [(offset, name, typeindex)]), scalar kinds per type index."""

    def __init__(s, coff):
        s.recs = []
        s.classes = {}
        s._flat = {}
        for sec in coff.sections:
            if sec.name != '.debug$T':
                continue
            c = Cur(sec.data, 4)
            while c.at + 4 <= len(c.d):
                n = c.u('<H')
                s.recs.append((struct.unpack_from('<H', c.d, c.at)[0], c.d[c.at + 2:c.at + n]))
                c.at += n
            break
        for i, (leaf, d) in enumerate(s.recs):
            try:
                if leaf in (0x1004, 0x1005, 0x1006):
                    c = Cur(d)
                    c.u('<H')
                    prop = c.u('<H')
                    fl = c.u('<I')
                    if leaf != 0x1006:
                        c.u('<I'); c.u('<I')
                    size = c.num()
                    nm = c.name()
                    if prop & 0x80:
                        continue
                    s.classes.setdefault(nm, (size, fl, i + 0x1000))
            except Exception:
                pass

    def rec(s, ti):
        k = ti - 0x1000
        return s.recs[k] if 0 <= k < len(s.recs) else (None, b'')

    def members(s, fl):
        leaf, d = s.rec(fl)
        out = []
        if leaf != 0x1203:
            return out
        c = Cur(d)
        try:
            while c.at < len(d):
                lf = c.u('<H')
                if lf == 0x1405:
                    c.u('<H'); ty = c.u('<I'); off = c.num(); nm = c.name()
                    out.append((off, nm, ty))
                elif lf == 0x1400:
                    c.u('<H'); ty = c.u('<I'); off = c.num()
                    out.append((off, None, ty))
                elif lf in (0x1401, 0x1402):
                    c.u('<H'); c.u('<I'); c.u('<I'); c.num(); c.num()
                elif lf == 0x140B:
                    at = c.u('<H'); c.u('<I')
                    if ((at >> 2) & 7) in (4, 6):
                        c.u('<I')
                    c.name()
                elif lf in (0x1403, 0x1406, 0x1407, 0x1408):
                    c.at += 6; c.name()
                elif lf in (0x1409, 0x140A):
                    c.at += 6
                elif lf == 0x1502:
                    c.u('<H'); c.num(); c.name()
                else:
                    break
                c.pad()
        except Exception:
            pass
        return out

    def kind(s, ti, depth=0):
        """-> ('scalar', width, sign, name) | ('class', name) | ('array', elemti, count, elemsize) | None"""
        if depth > 10:
            return None
        if ti < 0x1000:
            if ti & 0x700:
                return ('scalar', 4, 'U', 'pointer')
            p = PRIM.get(ti & 0xFF)
            return ('scalar',) + p if p else None
        leaf, d = s.rec(ti)
        c = Cur(d)
        try:
            if leaf == 0x1001:
                return s.kind(c.u('<I'), depth + 1)
            if leaf == 0x1002:
                return ('scalar', 4, 'U', 'pointer')
            if leaf in (0x1507, 0x1007):
                c.u('<H'); c.u('<H'); ut = c.u('<I'); c.u('<I'); nm = c.name()
                k = s.kind(ut, depth + 1)
                return ('scalar', k[1], k[2], 'enum ' + nm) if k and k[0] == 'scalar' else None
            if leaf in (0x1503, 0x1003):
                et = c.u('<I'); c.u('<I'); size = c.num()
                ek = s.size(et)
                return ('array', et, size // ek if ek else 0, ek)
            if leaf in (0x1004, 0x1005, 0x1006):
                c.u('<H'); prop = c.u('<H'); c.u('<I')
                if leaf != 0x1006:
                    c.u('<I'); c.u('<I')
                c.num()
                return ('class', c.name())
            if leaf == 0x1205:
                return None  # bitfield
        except Exception:
            return None
        return None

    def size(s, ti):
        k = s.kind(ti)
        if not k:
            return 0
        if k[0] == 'scalar':
            return k[1]
        if k[0] == 'class':
            return s.classes.get(k[1], (0,))[0]
        return k[2] * k[3]

    def flat(s, cname):
        """-> [(offset, dims, owner, field, width, sign, typename)] for every scalar leaf of cname;
        dims = ((stride, count), ...) of the arrays the leaf repeats in, innermost first."""
        if cname in s._flat:
            return s._flat[cname]
        s._flat[cname] = []
        out = []
        if cname not in s.classes:
            return out
        size, fl, _ = s.classes[cname]
        for off, nm, ty in s.members(fl):
            out += s._leaves(ty, off, cname, nm, 0)
        s._flat[cname] = out
        return out

    def _leaves(s, ty, off, owner, nm, depth):
        k = s.kind(ty)
        if not k or depth > 6:
            return []
        if k[0] == 'scalar':
            return [(off, (), owner, nm, k[1], k[2], k[3])]
        if k[0] == 'class':
            return [(off + o, d, ow, f, w, sg, tn) for o, d, ow, f, w, sg, tn in s.flat(k[1])]
        et, cnt, esz = k[1], k[2], k[3]
        if not cnt or not esz:
            return []
        inner = s._leaves(et, 0, owner, nm, depth + 1)
        return [(off + o, d + ((esz, cnt),), ow, f, w, sg, tn + ('[]' if s.kind(et) and s.kind(et)[0] == 'scalar' else ''))
                for o, d, ow, f, w, sg, tn in inner]

    def by_name(s):
        """field name -> every flattened leaf of every class (offsets relative to that class)"""
        if getattr(s, '_byname', None) is None:
            s._byname = collections.defaultdict(set)
            for cname in list(s.classes):
                for e in s.flat(cname):
                    s._byname[e[3]].add(e)
        return s._byname

    def at(s, cname, disp, names=None):
        """leaves of cname at disp (an element start)."""
        return [(ow, f, w, sg, tn) for o, d, ow, f, w, sg, tn in s.flat(cname)
                if covers(o, d, disp) and (names is None or f in names)]


def covers(o, dims, disp):
    rem = disp - o
    if rem < 0:
        return False
    for st, cnt in sorted(dims, reverse=True):
        q = rem // st
        if q >= cnt:
            return False
        rem -= q * st
    return rem == 0


DECO = {'C': (1, 'S', 'signed char'), 'D': (1, 'S', 'char'), 'E': (1, 'U', 'unsigned char'), 'F': (2, 'S', 'short'),
        'G': (2, 'U', 'unsigned short'), 'H': (4, 'S', 'int'), 'I': (4, 'U', 'unsigned int'), 'J': (4, 'S', 'long'),
        'K': (4, 'U', 'unsigned long'), '_N': (1, 'U', 'bool'), '_J': (8, 'S', '__int64'), '_K': (8, 'U', 'unsigned __int64')}


def global_kind(sym):
    """decorated data name -> ('struct', name, is_array) | ('scalar', w, sign, tn, is_array) | None"""
    m = re.match(r'\?(\w+)@@3(PA|QA|)(?:Y\w+)?(.*)$', sym)
    if not m:
        return None
    arr = bool(m.group(2))
    t = m.group(3)
    t = re.sub(r'^[ABCD](?=[UVHIJKCDEFG_W])', '', t) if arr else t
    m2 = re.match(r'[UV](\w+)@@', t)
    if m2:
        return ('struct', m2.group(1), arr)
    m2 = re.match(r'W4(\w+)@@', t)
    if m2:
        return ('scalar', 4, 'S', 'enum ' + m2.group(1), arr)
    for k in ('_N', '_J', '_K', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K'):
        if t.startswith(k):
            return ('scalar',) + DECO[k] + (arr,)
    return None


# ---------------------------------------------------------------- compile with /FAsc
def compile_cod(unit, src, pfx):
    cmd = fe.compile_cmd(unit)
    if not cmd:
        return None, None, 'unit not in compile_commands'
    tag = 'ts_%d_%s' % (os.getpid(), pfx)
    obj, cod = fe.SCR + '/' + tag + '.obj', fe.SCR + '/' + tag + '.cod'
    out = []
    for t in cmd:
        if t.startswith('/Fo'):
            t = '/Fo' + obj
        elif t.startswith('/Fd') or t.startswith('--repro-root') or t == '/W4':
            continue
        elif t == fe.R + '/' + unit:
            t = os.path.abspath(src)
        out.append(t)
    if '/Z7' not in out and '/Zi' not in out:
        out.insert(-1, '/Z7')  # Jackal units build without debug types; the layouts come from CodeView
    out.insert(-1, '/FAsc')
    out.insert(-1, '/Fa' + cod)
    env = dict(os.environ, VC6_PREFIX='%s/.opensmacx/wine/worker-%s' % (fe.MAIN, pfx))
    for f in (obj, cod):
        os.path.exists(f) and os.remove(f)
    p = subprocess.run(out, cwd=fe.SCR, env=env, capture_output=True, text=True)
    if not os.path.exists(obj) or not os.path.exists(cod):
        return None, None, (p.stdout + p.stderr)[-600:]
    c = fe.Coff(open(obj, 'rb').read())
    listing = open(cod, encoding='latin-1', errors='replace').read()
    os.remove(obj)
    os.remove(cod)
    return c, listing, None


INS = re.compile(r'^  ([0-9a-f]{5})\t[0-9a-f ]+\t (\w+)\s*(.*)$')
INS0 = re.compile(r'^  ([0-9a-f]{5})\t[0-9a-f ]+$')
INS1 = re.compile(r'^\t[0-9a-f ]+\t+ (\w+)\s*(.*)$')
SRCL = re.compile(r'^; (\d+)\s*:(.*)$')


def parse_listing(listing):
    """-> {proc symbol: {offset: (operand text with symbols, source text)}}"""
    procs = {}
    cur = None
    src = []
    last_src = ''
    fresh = False
    pend = None
    for l in listing.split('\n'):
        if l.endswith('COMDAT') and ' PROC NEAR' in l or (' PROC NEAR' in l):
            cur = {}
            procs[l.split()[0]] = cur
            src = []
            last_src = ''
            continue
        if cur is None:
            continue
        if ' ENDP' in l:
            cur = None
            continue
        m = SRCL.match(l)
        if m:
            if not fresh:
                src = []
                fresh = True
            src.append(m.group(2).split('//')[0])
            continue
        m = INS.match(l)
        if m:
            off, txt = int(m.group(1), 16), m.group(2) + ' ' + m.group(3)
        else:
            m0 = INS0.match(l)
            if m0:
                pend = int(m0.group(1), 16)
                continue
            m1 = INS1.match(l) if pend is not None else None
            if not m1:
                continue
            off, txt = pend, m1.group(1) + ' ' + m1.group(2)
        pend = None
        if fresh:
            last_src = ' '.join(src)
            fresh = False
        cur[off] = (txt, last_src)
    return procs


IDENT = re.compile(r'[A-Za-z_]\w*')


def sig(stream, k):
    """Classify how instruction k (a memory access) is used: -> (sign or None, width)"""
    off, text, size, _ = stream[k]
    mn, _, op = text.partition(' ')
    m = MEM.search(op)
    if not m:
        return None, 0
    w = WIDTH[m.group(1)]
    dst = op.split(',')[0].strip()
    if mn == 'movsx':
        return 'S', w
    if mn == 'movzx':
        return 'U', w
    if mn in ('idiv', 'imul') and mn == 'idiv':
        return 'S', w
    if mn == 'div':
        return 'U', w
    if mn == 'mov' and w < 4 and not dst.startswith(('byte', 'word')):
        r = full(dst)
        for j in range(max(0, k - 3), k):
            if stream[j][1] == 'xor %s, %s' % (r, r):
                return 'U', w
        for j in range(k + 1, min(len(stream), k + 4)):
            t = stream[j][1]
            if re.match(r'and %s, 0x(ff|ffff)$' % r, t):
                return 'U', w
            if r in t:
                break
        return None, w
    if w != 4:
        return None, w
    if mn == 'cmp':
        return jcc_after(stream, k), w
    if mn in ('mov', 'add', 'sub', 'imul') and dst in REG32:
        return follow(stream, k + 1, {dst}), w
    return None, w


def follow(stream, k, taint):
    """Follow a dword value (tainted registers) forward through moves and add/sub/imul/lea to the first
    sign-revealing use: a compare + jcc, cdq + idiv, xor edx + div, idiv/div, sar/shr."""
    for j in range(k, min(len(stream), k + 12)):
        t = stream[j][1]
        mj, _, oj = t.partition(' ')
        ops = [x.strip() for x in oj.split(',')] if oj else []
        d = ops[0] if ops else ''
        srcs = ' '.join(ops[1:])
        if mj == 'cdq':
            if 'eax' in taint:
                for j2 in range(j + 1, min(len(stream), j + 4)):
                    if stream[j2][1].startswith('idiv'):
                        return 'S'
            taint.discard('edx')
            continue
        if mj == 'xor' and oj == 'edx, edx' and 'eax' in taint:
            for j2 in range(j + 1, min(len(stream), j + 4)):
                if stream[j2][1].startswith('div'):
                    return 'U'
        if mj in ('idiv', 'div'):
            if d in taint or 'eax' in taint:
                return 'S' if mj == 'idiv' else 'U'
            return None
        if mj in ('sar', 'shr') and d in taint:
            return 'S' if mj == 'sar' else 'U'
        if mj in ('cmp', 'test') and mj == 'cmp' and any(o in taint for o in ops):
            return jcc_after(stream, j)
        if mj.startswith('j') or mj in ('ret', 'leave'):
            return None
        if mj == 'call':
            taint -= {'eax', 'ecx', 'edx'}
        elif mj in ('mov', 'lea') and d in REG32:
            if any(r in srcs for r in taint) and not (mj == 'mov' and 'ptr' in srcs):
                taint.add(d)
            else:
                taint.discard(d)
        elif mj in ('add', 'sub', 'imul') and d in REG32:
            if any(r in srcs for r in taint):
                taint.add(d)
        elif d in REG32 and d in taint and mj not in ('push', 'test', 'neg', 'inc', 'dec', 'shl'):
            taint.discard(d)
        if not taint:
            return None
    return None


def jcc_after(stream, k):
    """the condition that consumes the flags of compare k (only flag-neutral moves in between)"""
    for j in range(k + 1, min(len(stream), k + 4)):
        mn = stream[j][1].split(' ')[0]
        if mn in SIGNED_J:
            return 'S'
        if mn in UNSIGNED_J:
            # a switch's range check (cmp; ja default; jmp [reg*4+table]) is unsigned whatever the type
            if any(re.match(r'jmp dword ptr \[\w+\*4', stream[j2][1]) for j2 in range(j + 1, min(len(stream), j + 3))):
                return None
            return 'U'
        if mn in ('mov', 'lea', 'push', 'pop'):
            continue
        return None
    return None


def shape(text):
    m = MEM.search(text)
    if not m:
        return None
    inner = re.sub(r'\b(e?[a-d]x|e?[sd]i|ebp|esp|[a-d][lh])\b', 'R', m.group(2))
    return m.group(1), inner


def scan_unit(unit, pfx, only, sel_field):
    src = fe.R + '/' + unit
    text = open(src, encoding='latin-1').read()
    c, listing, err = compile_cod(unit, src, pfx)
    if c is None:
        return [('ERR', unit, err)]
    ty = Types(c)
    procs = parse_listing(listing)
    uses = []
    rx = re.compile(r'// ORIGINAL: 0x([0-9A-F]{8}) \S+ 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)')
    for m in rx.finditer(text):
        addr = int(m.group(1), 16)
        if only and addr not in only:
            continue
        r = fe.evaluate(c, text, addr)
        if 'error' in r:
            continue
        sym = r['sym']
        lst = procs.get(sym, {})
        rows = r['rows']
        exact = r['exact']
        O = [x[1] for x in rows if x[1] is not None]
        C = [x[2] for x in rows if x[2] is not None]
        oi = {x[0]: i for i, x in enumerate(O)}
        ci = {x[0]: i for i, x in enumerate(C)}
        # pairs (our instr, image instr or None): aligned rows, else same operand shape in the differing block
        pairs = []
        k = 0
        while k < len(rows):
            if rows[k][0] != '!!':
                pairs.append((rows[k][2], rows[k][1]))
                k += 1
                continue
            k2 = k
            while k2 < len(rows) and rows[k2][0] == '!!':
                k2 += 1
            blk_o = [x[1] for x in rows[k:k2] if x[1] is not None]
            used = set()
            for x in rows[k:k2]:
                cc = x[2]
                if cc is None:
                    continue
                sh = shape(cc[1])
                hit = None
                if sh:
                    for o in blk_o:
                        if o[0] not in used and shape(o[1]) and shape(o[1])[1] == sh[1]:
                            hit = o
                            break
                    if hit is None:  # width differs: same address expression
                        for o in blk_o:
                            if o[0] not in used and shape(o[1]) and shape(o[1])[1] == sh[1]:
                                hit = o
                                break
                if hit is not None:
                    used.add(hit[0])
                pairs.append((cc, hit))
            k = k2
        for cc, o in pairs:
            if cc is None or not MEM.search(cc[1]):
                continue
            ent = lst.get(cc[0])
            if not ent:
                continue
            optext, srcline = ent
            fields = attribute(ty, optext, cc[1], srcline)
            if not fields:
                continue
            ours_sig, ow = sig(C, ci[cc[0]])
            img_sig, iw = (sig(O, oi[o[0]]) if o is not None else (None, 0))
            for key, decl_w, decl_s, tn in fields:
                if sel_field and sel_field not in key:
                    continue
                uses.append(dict(key=key, decl=(decl_w, decl_s, tn), unit=unit, fn=sym, addr='%08X' % addr,
                                 exact=exact, img_at=('%08X' % (r['lo'] + o[0])) if o is not None else None,
                                 img=img_sig, iw=iw, ours=ours_sig, ow=ow, ours_text=cc[1],
                                 img_text=o[1] if o is not None else None, line=srcline.strip()[:120]))
    return uses


def attribute(ty, optext, dis_text, srcline):
    """-> [(key, width, sign, typename)] for the field an operand names (unique type or nothing)."""
    m = re.search(r'PTR (\?[^\s\[+\],]+)(?:\[[^\]]*\])?(?:\+(\d+))?(?:\[[^\]]*\])?', optext)
    cands = []
    if m and m.group(1).startswith('?'):
        sym = m.group(1)
        add = int(m.group(2) or 0)
        mm = re.search(r'\+(\d+)\]', optext[m.end(1):]) if not m.group(2) else None
        if mm:
            add = int(mm.group(1))
        gk = global_kind(sym)
        if not gk:
            return []
        gname = re.match(r'\?(\w+)@@', sym).group(1)
        if gk[0] == 'struct':
            if gk[1] not in ty.classes:
                return []
            size = ty.classes[gk[1]][0]
            disp = add % size if gk[2] and size else add
            for ow, f, w, sg, tn in ty.at(gk[1], disp):
                cands.append(('%s::%s' % (ow, f), w, sg, tn))
        else:
            cands.append((gname, gk[1], gk[2], gk[3] + ('[]' if gk[4] else '')))
    elif m is None:
        mm = MEM.search(dis_text)
        if not mm or 'ebp' in mm.group(2) or 'esp' in mm.group(2) or '<r>' in mm.group(2):
            return []
        d = re.search(r'\+ (0x[0-9a-f]+|\d+)$', mm.group(2))
        disp = int(d.group(1), 0) if d else 0
        if re.search(r'\*\d', mm.group(2)) and not d:
            return []
        names = set(IDENT.findall(srcline))
        if not names:
            return []
        idx = ty.by_name()
        for nm in names:
            for o, d, ow, f, w, sg, tn in idx.get(nm, ()):
                if covers(o, d, disp):
                    cands.append(('%s::%s' % (ow, f), w, sg, tn))
    kinds = {(w, sg) for _, w, sg, _ in cands}
    keys = {k for k, _, _, _ in cands}
    if len(kinds) != 1 or len(keys) > 3:
        return []
    # one type: attribute to every candidate key only when there is a single one
    if len(keys) != 1:
        return []
    k, w, sg, tn = cands[0]
    if sg == 'F' or tn == 'pointer':
        return []
    return [(k, w, sg, tn)]


def casts(key):
    fld = key.split('::')[-1]
    out = []
    pat = re.compile(r'\((?:const\s+)?(?:unsigned|signed|u?int(?:8|16|32)?(?:_t)?|char|short|long|uint\w*|int\w*|DWORD|BYTE|WORD|size_t)(?:\s+\w+)?\s*\*?\)\s*\(?[\w\->\.\[\]\s\(]*?\b%s\b|static_cast<[^>]*>\([^;]*\b%s\b|WORKAROUND\(type\)[^\n]*\b%s\b' % (
        re.escape(fld), re.escape(fld), re.escape(fld)))
    for root, _, files in os.walk(fe.R + '/src'):
        for f in files:
            if not f.endswith(('.cpp', '.h')):
                continue
            p = os.path.join(root, f)
            try:
                for i, l in enumerate(open(p, encoding='latin-1'), 1):
                    if fld in l and pat.search(l):
                        out.append('%s:%d: %s' % (p[len(fe.R) + 1:], i, l.strip()[:140]))
            except OSError:
                pass
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='*')
    ap.add_argument('--addr', default='')
    ap.add_argument('--field', default='')
    ap.add_argument('--min', type=int, default=1, help='minimum contradicting image uses')
    ap.add_argument('--uses', action='store_true')
    ap.add_argument('--jobs', type=int, default=int(os.environ.get('FE_JOBS', '5')))
    ap.add_argument('--pfx-base', type=int, default=int(os.environ.get('FE_PFX_BASE', '21')))
    ap.add_argument('--json')
    a = ap.parse_args()
    cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
    units = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/')]
    units = [u for u in units if '/vendor/' not in u and '/uv/' not in u and u.endswith('.cpp')]
    if a.units:
        units = [u for u in units if u in a.units or os.path.basename(u) in a.units]
    only = {int(x, 16) for x in a.addr.split(',') if x}
    fe.compile_cmd(units[0])

    def lane(k):
        r = []
        for i in range(k, len(units), a.jobs):
            r += scan_unit(units[i], a.pfx_base + k, only, a.field)
        return r
    res = []
    with ThreadPoolExecutor(a.jobs) as ex:
        for o in ex.map(lane, range(a.jobs)):
            res += o
    errs = [x for x in res if isinstance(x, tuple)]
    uses = [x for x in res if isinstance(x, dict)]
    if a.json:
        json.dump(uses, open(a.json, 'w'), indent=0)
    by = collections.defaultdict(list)
    for u in uses:
        by[u['key']].append(u)
    rows = []
    for key, us in by.items():
        decl = us[0]['decl']
        cnt = collections.Counter()
        for u in us:
            if u['img']:
                cnt[(u['exact'], u['img'])] += 1
            if u['img_text'] and u['iw'] and u['iw'] != decl[0] and u['ow'] == decl[0]:
                cnt[(u['exact'], 'W%d' % u['iw'])] += 1
        opp = 'U' if decl[1] == 'S' else 'S'
        contra = cnt[(False, opp)] + cnt[(True, opp)]
        agree = cnt[(False, decl[1])] + cnt[(True, decl[1])]
        wid = sum(v for (e, s), v in cnt.items() if s.startswith('W'))
        paired = sum(1 for u in us if u['img_text'])
        if a.field or contra >= a.min and contra > agree or wid >= max(2, a.min) and wid * 10 >= paired:
            nonex = sum(1 for u in us if not u['exact'])
            rows.append((key, decl, cnt, contra, agree, wid, nonex, us))
    rows.sort(key=lambda r: (-(r[3] - r[4]), -r[5], -r[6]))
    for key, decl, cnt, contra, agree, wid, nonex, us in rows:
        opp = 'U' if decl[1] == 'S' else 'S'
        ws = ' '.join('%s:%d/%d' % (s, cnt[(False, s)], cnt[(True, s)]) for s in sorted({s for _, s in cnt if s.startswith('W')}))
        cl = casts(key)
        print('FIELD %-44s decl=%-16s img %s=%d/%d %s=%d/%d (nonexact/exact) %s uses=%d nonexact=%d casts=%d' % (
            key, decl[2], opp, cnt[(False, opp)], cnt[(True, opp)], decl[1], cnt[(False, decl[1])], cnt[(True, decl[1])],
            ('W ' + ws) if ws else '', len(us), nonex, len(cl)))
        samp = [u for u in us if u['img'] == opp or (u['iw'] and u['iw'] != decl[0] and u['ow'] == decl[0])][:6]
        for u in samp:
            print('    %s %s img %-34s ours %-34s %s' % (u['img_at'], u['fn'][:38], (u['img_text'] or '')[:34], u['ours_text'][:34], u['line'][:60]))
        for l in cl[:8]:
            print('    cast %s' % l)
        if a.uses:
            for u in us:
                print('    use %s %s %s img=%s/%s ours=%s/%s %s | %s' % (u['img_at'], 'X' if u['exact'] else '-', u['fn'][:30], u['img'], u['iw'], u['ours'], u['ow'], u['img_text'], u['ours_text']))
    for e in errs:
        print('ERROR', e[1], (e[2] or '')[:200])
    print('fields attributed: %d, uses: %d, flagged: %d' % (len(by), len(uses), len(rows)))


if __name__ == '__main__':
    main()
