#!/usr/bin/env python3
"""webscan.py [unit ...] [--addr A[,A]] [--from FEALL.json] [--json OUT] [--all] [--src ALT.cpp] [--min-uses N]
One name, one value (MATCHING_GUIDE 3.3 "One name reused for unrelated values"; 2.7 use_ability).
Lists, per non-exact function, the two variable shapes where our source and the original may disagree on
how many named locals carry the values:

  REUSE  one named local or parameter with 2+ disjoint def-use webs (one name reused for unrelated values:
         decompiler-style sources reuse iVar/local_ names).  The original often had a separate name.
  COPY   a plain copy `b = a;` between two named locals (both live after it, or a's range ends where b's
         begins).  The original sometimes had one reassigned variable (Console::use_ability 0x004D7F60).

Source webs come from the /FAsc listing: reaching definitions over the listing's block CFG for every
source reference (regprio.py's machinery).  Frame-homed names get instruction-level webs on their own slot
(the listing names the slot `_name$[ebp]`), and each web's slots in the image are read through the fe
alignment (equal rows, and replace rows whose instructions differ only in the frame offset).  Verdicts:

  SPLIT   (REUSE) our one slot carries 2+ webs; the image keeps them in different slots.  Strong.
  SHARED  (REUSE) the image's aligned accesses use one slot for all of them as well.  Weak: VC6 packs
          disjoint ranges into one slot anyway, and draw_expenses' `wide` read SHARED yet the split gained
          134 rows by reordering the other slots.  Try it (--sweep).
  REG     (REUSE) the name has no frame slot in our compile (register or folded): judge by candset/regmap.
  MERGE   (COPY)  a and b have separate slots in ours; the image's aligned accesses of both use one slot.
  APART   (COPY)  the image keeps separate slots too.  Probably original; listed with -v.
  REGCOPY (COPY)  one of them has no frame slot in ours.

The frame (ours vs image) prints on every function line: a REUSE/COPY on a function whose frame differs
is the first place to try.  Loop-counter webs (`for (x = ...`) are tagged `for`; a name whose webs are
all loop counters is the sibling-counter shape (analysis/loopsweep.py --rule sibling) and listed with -v.
Screen only: try each split/merge with fe.py and framescan.py --addr, then judge by frame, slot map,
regmap/candset agreement, rows (MATCHING_GUIDE 1.1)."""
import sys, os, re, json, argparse, collections, subprocess
from concurrent.futures import ThreadPoolExecutor
H = os.path.dirname(os.path.abspath(__file__))
T = os.path.dirname(H)
sys.path.insert(0, T)
sys.path.insert(0, H)
import fe
import regprio
import framescan

SLOTRE = re.compile(r'\[ebp ([-+]) (0x[0-9a-f]+|\d+)\]')
NAMERE = re.compile(r'_(\w+?)\$\d*\[ebp(?:\+(\d+))?\]')
PURE = ('mov', 'lea', 'movsx', 'movzx', 'pop', 'fstp', 'fistp', 'fst', 'fist')
READONLY = ('cmp', 'test', 'push', 'fild', 'fld', 'imul', 'idiv', 'div', 'mul', 'call', 'jmp', 'fcomp', 'fcom',
            'fmul', 'fadd', 'fdiv', 'fsub', 'fdivr', 'fsubr', 'ficom', 'ficomp', 'fiadd', 'fimul', 'fidiv', 'fisub')


def listing_text(unit, src, pfx):
    cmd = fe.compile_cmd(unit)
    asm = fe.SCR + '/ws_%d_%s.cod' % (os.getpid(), pfx)
    obj = fe.SCR + '/ws_%d_%s.obj' % (os.getpid(), pfx)
    out = []
    for t in cmd:
        if t.startswith('/Fo'):
            t = '/Fo' + obj
        elif t.startswith('/Fd') or t.startswith('--repro-root') or t == '/W4':
            continue
        elif t == fe.R + '/' + unit:
            t = os.path.abspath(src)
        out.append(t)
    out.insert(-1, '/FAsc')
    out.insert(-1, '/Fa' + asm)
    env = dict(os.environ, VC6_PREFIX='%s/.opensmacx/wine/worker-%s' % (fe.MAIN, pfx))
    subprocess.run(out, cwd=fe.SCR, env=env, capture_output=True, text=True)
    try:
        return open(asm, errors='replace').read().replace('\r', '')
    except OSError:
        return ''
    finally:
        os.path.exists(asm) and os.remove(asm)
        os.path.exists(obj) and os.remove(obj)


def parse_listing(t, sym):
    """regprio.cod_listing's parser over an already compiled listing"""
    i = t.find(sym + ' PROC')
    if i < 0:
        return None
    e = t.find(sym + ' ENDP', i)
    items = []
    pend = None
    lastoff = -1
    for l in t[i:e].split('\n'):
        m = re.match(r'; (\d+)\s+:', l)
        if m:
            items.append(('line', int(m.group(1))))
            continue
        m = re.match(r'(\$L\d+):$', l)
        if m:
            items.append(('label', m.group(1)))
            continue
        m = re.match(r'  ([0-9a-f]{5})\t([0-9a-f ]+)(?:\t+ ?(.*))?$', l)
        ins = None
        if m:
            off = int(m.group(1), 16)
            if m.group(3):
                ins = m.group(3)
            else:
                pend = off
                continue
        else:
            m = re.match(r'\t[0-9a-f ]+\t+ ?(\S.*)$', l)
            if m and pend is not None:
                off, ins, pend = pend, m.group(1), None
        if ins:
            if off < lastoff:
                break
            lastoff = off
            ins = re.sub(r'\s+', ' ', ins.split(';')[0]).strip()
            m = re.match(r'DD (\$L\d+)$', ins)
            items.append(('dd', m.group(1)) if m else ('ins', off, ins))
    return items


def slot_key(txt):
    m = SLOTRE.search(txt)
    if not m:
        return None
    d = int(m.group(2), 0)
    return -d if m.group(1) == '-' else d


def shape(txt):
    return SLOTRE.sub('[ebp+S]', txt)


def access(tx, name):
    """-> (use, def, part) of variable `name` by listing instruction tx"""
    ms = [m for m in NAMERE.finditer(tx) if m.group(1) == name]
    if not ms:
        return False, False, False
    mn = tx.split(' ', 1)[0]
    ops = tx.split(' ', 1)[1] if ' ' in tx else ''
    first = ops.split(',', 1)[0]
    indst = any(m.group(1) == name for m in NAMERE.finditer(first)) and ',' in ops or (
        mn in ('inc', 'dec', 'neg', 'not', 'fstp', 'fistp', 'fst', 'fist', 'pop') and NAMERE.search(first))
    part = bool(re.search(r'\b(BYTE|WORD|QWORD|TBYTE) PTR _%s\$' % re.escape(name), tx))
    if mn == 'lea':
        return True, False, True  # address taken: a use that ends the analysis' precision
    if indst:
        if mn in PURE:
            if part and mn not in ('fstp', 'fst', 'fistp', 'fist'):
                return True, True, part  # partial write keeps the rest
            return False, True, part
        if mn in READONLY:
            return True, False, part
        return True, True, part
    return True, False, part


class Code:
    """instruction-level CFG over the /FAsc listing (regprio.Func blocks)"""

    def __init__(self, F):
        self.F = F
        self.ins = F.ins
        self.idx = {off: n for n, (off, ln, tx) in enumerate(F.ins)}
        self.succ = []
        bstart = {b['n']: self.idx.get(b['start']) for b in F.blocks}
        for n, (off, ln, tx) in enumerate(F.ins):
            b = F.block_of(off)
            nxt = F.ins[n + 1][0] if n + 1 < len(F.ins) else None
            if nxt is not None and F.block_of(nxt) == b:
                self.succ.append([n + 1])
            else:
                self.succ.append([bstart[s] for s in F.succ[b] if bstart.get(s) is not None])

    def webs(self, name, entry):
        acc = [access(tx, name) for off, ln, tx in self.ins]
        if not any(u or d for u, d, p in acc):
            return None, False
        part = any(p for u, d, p in acc)
        defs = [n for n, (u, d, p) in enumerate(acc) if d]
        parent = {}

        def find(x):
            parent.setdefault(x, x)
            while parent[x] != x:
                parent[x] = parent[parent[x]]
                x = parent[x]
            return x
        starts = [(d, self.succ[d]) for d in defs]
        if entry:
            starts.append(('E', [0]))
        for d, ss in starts:
            seen = set()
            work = list(ss)
            while work:
                n = work.pop()
                if n in seen:
                    continue
                seen.add(n)
                u, dd, p = acc[n]
                if u:
                    parent[find(('u', n))] = find(('d', d))
                if dd:
                    continue
                work.extend(self.succ[n])
        groups = collections.defaultdict(lambda: [set(), set()])
        for d in defs + (['E'] if entry else []):
            groups[find(('d', d))][0].add(d)
        for n, (u, d, p) in enumerate(acc):
            if u:
                groups[find(('u', n))][1].add(n)
        out = []
        for g in groups.values():
            if not g[1]:
                continue  # a dead store (or an unused entry value)
            out.append(g)
        return out, part


def imgslots(rows):
    """our offset -> image frame slot of the aligned instruction (same instruction up to the frame offset)"""
    out = {}
    for k, o, c in rows:
        if o is None or c is None:
            continue
        so = slot_key(c[1])
        if so is None:
            continue
        if k == '!!' and shape(o[1]) != shape(c[1]):
            continue
        si = slot_key(o[1])
        if si is not None:
            out[c[0]] = si
    return out


def lines_of(code, n_set):
    return sorted({code.ins[n][1] for n in n_set if n != 'E'})


def fmt_lines(ls, k=6):
    s = ','.join('L%d' % x for x in ls[:k])
    return s + (',..' if len(ls) > k else '')


def analyse(unit, src, text, c, lst, addr, verbose):
    r = fe.evaluate(c, text, addr)
    if 'error' in r or r.get('exact'):
        return None
    items = parse_listing(lst, r['sym'])
    if not items:
        return None
    F = regprio.Func(items)
    if not F.ins:
        return None
    code = Code(F)
    first, flines = regprio.func_source(text, addr)
    stripped = [regprio.strip_code(l) for l in flines]
    body = '\n'.join(stripped)
    names, params, excl, structs = regprio.declared_names(body[body.find('{') + 1:], body)
    names_set = set(names) - structs
    fs = r['rows']
    I = framescan.insns(fe.imgbytes(*fe.ident(text, addr)[1:]), fe.ident(text, addr)[1])
    try:
        sec = c.sections[c.function(r['sym']).section - 1]
        O = framescan.insns(sec.data, 0)
        fo = framescan.frame(O)[0]
    except Exception:
        fo = -1
    fi = framescan.frame(I)[0]
    isl = imgslots(fs)
    rec = dict(unit=unit, addr='%08X' % addr, sym=r['sym'], diff=r['diff'], frame=[fi, fo], reuse=[], copy=[])

    # ---- source webs (regprio): every named local and parameter
    lineof = {off: ln for off, ln, tx in F.ins}
    linecode = collections.defaultdict(list)
    for off, ln, tx in F.ins:
        linecode[ln].append(off)
    codelines = sorted(linecode)
    refs = []
    for i, s in enumerate(stripped):
        if i == 0:
            continue
        ln = first + i
        for k, kind, col in regprio.refs_on_line(s, names_set, set(), False, 0):
            l2 = ln
            if l2 not in linecode:
                nxt = [x for x in codelines if x > l2]
                if not nxt:
                    continue
                l2 = nxt[0]
            b = F.block_of(linecode[l2][0] if kind == 'u' else linecode[l2][-1])
            seq = ln * 1000 + (999 if kind == 'd' else col)
            refs.append(dict(key=k, kind=kind, block=b, line=ln, seq=seq))
    webs = regprio.build_webs(F, refs, set(params))
    byname = collections.defaultdict(list)
    for w in webs.values():
        rs = [refs[i] for i in w['refs']]
        if not any(x['kind'] in ('u', 'ud') for x in rs):
            continue
        if not any(x['kind'] in ('d', 'ud') for x in rs) and w['key'] not in params:
            continue
        byname[w['key']].append(rs)

    def is_for(name, ln):
        s = stripped[ln - first] if 0 <= ln - first < len(stripped) else ''
        return re.search(r'\bfor\s*\(\s*(?:int\s+)?%s\s*=' % re.escape(name), s) is not None

    # ---- REUSE
    for name, ws in sorted(byname.items()):
        if len(ws) < 2 or name in excl:
            continue
        mw, part = code.webs(name, name in params)
        desc = []
        for rs in sorted(ws, key=lambda rs: min(x['line'] for x in rs)):
            dl = sorted({x['line'] for x in rs if x['kind'] in ('d', 'ud')})
            ul = sorted({x['line'] for x in rs})
            desc.append(dict(lines=ul, defs=dl, loop=any(is_for(name, l) for l in dl)))
        allfor = all(d['loop'] for d in desc)
        verdict = 'REG'
        slots = []
        if mw:
            for d, u in mw:
                ss = collections.Counter(isl[code.ins[n][0]] for n in u | (d - {'E'}) if code.ins[n][0] in isl)
                slots.append((sorted(lines_of(code, u)), dict(ss)))
            if len(mw) >= 2:
                tops = [max(ss, key=ss.get) for ls, ss in slots if ss]
                verdict = 'SPLIT' if len(set(tops)) >= 2 else 'SHARED'
            else:
                verdict = 'ONEWEB'  # the machine joined the source webs (a read before the second def path)
        if not verbose and (verdict == 'ONEWEB' or allfor):
            continue
        rec['reuse'].append(dict(name=name, verdict=verdict, param=name in params, allfor=allfor, webs=desc,
                                 slots=[[fmt_lines(ls), {('%+#x' % k): v for k, v in ss.items()}] for ls, ss in slots],
                                 part=part))

    # ---- COPY: `b = a;` between named locals
    for i, s in enumerate(stripped):
        m = re.match(r'\s*(?:[A-Za-z_][\w:]*\s+)?([A-Za-z_]\w*)\s*=\s*([A-Za-z_]\w*)\s*;\s*$', s)
        if not m or m.group(1) == m.group(2):
            continue
        b, a_ = m.group(1), m.group(2)
        if b not in names_set or a_ not in names_set or b in excl or a_ in excl:
            continue
        ln = first + i
        sa = collections.Counter()
        sb = collections.Counter()
        homed = [False, False]
        for off, l2, tx in F.ins:
            if NAMERE.search(tx):
                for mm in NAMERE.finditer(tx):
                    if mm.group(1) == a_:
                        homed[0] = True
                        if off in isl:
                            sa[isl[off]] += 1
                    elif mm.group(1) == b:
                        homed[1] = True
                        if off in isl:
                            sb[isl[off]] += 1
        if all(homed):
            common = set(sa) & set(sb)
            verdict = 'MERGE' if common else 'APART'
        else:
            verdict = 'REGCOPY'
            common = set()
        # is a still used after the copy (both live) or is the copy a hand-off?
        later = [x for x in refs if x['key'] == a_ and x['line'] > ln]
        handoff = not later or later[0]['kind'] == 'd'
        bweb = next((sorted({x['line'] for x in rs}) for rs in byname.get(b, ())
                     if any(x['line'] == ln and x['kind'] == 'd' for x in rs)), [])
        aweb = next((sorted({x['line'] for x in rs}) for rs in byname.get(a_, ()) if any(x['line'] == ln for x in rs)), [])
        if not verbose and verdict == 'APART':
            continue
        rec['copy'].append(dict(line=ln, dst=b, src=a_, verdict=verdict, handoff=handoff, bweb=bweb, aweb=aweb,
                                common=['%+#x' % k for k in sorted(common)],
                                img_src={('%+#x' % k): v for k, v in sa.items()},
                                img_dst={('%+#x' % k): v for k, v in sb.items()}))
    if not rec['reuse'] and not rec['copy']:
        return None
    return rec


def scan_unit(unit, addrs, pfx, verbose, src=None):
    src = os.path.abspath(src) if src else fe.R + '/' + unit
    text = open(src, encoding='latin-1').read()
    c, err = fe.compile_unit(unit, src, pfx)
    if c is None:
        return [dict(unit=unit, error=str(err))]
    lst = listing_text(unit, src, pfx)
    out = []
    for addr in addrs:
        if not fe.ident(text, addr):
            continue
        if re.search(r'// ORIGINAL: 0x%08X [^\n]*\n(?://(?! ORIGINAL)[^\n]*\n)*// compile_context' % addr, text):
            continue
        try:
            rec = analyse(unit, src, text, c, lst, addr, verbose)
        except SystemExit:
            rec = None
        except Exception as e:
            rec = dict(unit=unit, addr='%08X' % addr, error=repr(e)[:200])
        if rec:
            out.append(rec)
    return out


# ------------------------------------------------------------------ sweep
def rename_line(line, old, new):
    return re.sub(r'(?<![\w.>:])%s\b' % re.escape(old), new, line)


def decl_type(lines, lo, hi, name):
    """-> (line index of the declaration, its type text) inside lines[lo:hi]"""
    for k in range(lo, hi):
        c = regprio.strip_code(lines[k])
        m = re.search(r'(?:^|[;{(])\s*((?:const\s+|unsigned\s+|signed\s+|struct\s+)*[A-Za-z_][\w:]*(?:\s*\*+)?)\s+\**\s*%s\s*(?:[=;,\[)])' % re.escape(name), c)
        if m and m.group(1).split()[-1] not in regprio.KEYWORDS:
            return k, m.group(1)
    return None, None


def variants(text, rec):
    """-> [(key, new text)] split/merge variants of one function's candidates"""
    lines = text.split('\n')
    addr = int(rec['addr'], 16)
    first, flines = regprio.func_source(text, addr)
    lo, hi = first - 1, first - 1 + len(flines)
    body = lines[lo:hi]
    brace = next(k for k in range(lo, hi) if '{' in regprio.strip_code(lines[k]))
    out = []
    for x in rec['reuse']:
        if x['allfor'] or x['part']:
            continue
        name = x['name']
        dk, ty = decl_type(lines, lo, hi, name)
        if ty is None:
            continue
        webs = x['webs']
        for n in range(1, len(webs)):
            mine = set(webs[n]['lines'])
            other = set().union(*[set(w['lines']) for j, w in enumerate(webs) if j != n])
            if mine & other or dk + 1 in mine:
                continue
            new = name + '_w%d' % n
            L = list(lines)
            for ln in mine:
                L[ln - 1] = rename_line(L[ln - 1], name, new)
            ind = re.match(r'\s*', lines[dk]).group(0) if dk > brace else '    '
            at = dk + 1 if dk > brace else brace + 1
            L.insert(at, '%s%s %s;' % (ind, ty.strip(), new))
            out.append(('split %s web%d L%d' % (name, n, min(mine)), '\n'.join(L)))
    for x in rec['copy']:
        if not x['handoff'] or not x['bweb']:
            continue
        a_, b = x['src'], x['dst']
        L = list(lines)
        if '=' not in L[x['line'] - 1]:
            continue
        L[x['line'] - 1] = re.sub(r'\S.*', '', L[x['line'] - 1]) + '// ' + L[x['line'] - 1].strip()
        for ln in x['bweb']:
            if ln != x['line']:
                L[ln - 1] = rename_line(L[ln - 1], b, a_)
        out.append(('merge %s into %s L%d' % (b, a_, x['line']), '\n'.join(L)))
    return out


def sweep(res, jobs, base):
    """compile every variant; print the ones that move rows or frame toward the image"""
    import queue
    import tempfile
    pool = queue.Queue()
    for n in range(jobs):
        pool.put(str(base + n))
    tasks = []
    for rec in res:
        if 'error' in rec:
            continue
        unit = rec['unit']
        text = open(fe.R + '/' + unit, encoding='latin-1').read()
        for key, vt in variants(text, rec):
            tasks.append((rec, key, vt))

    def run(t):
        rec, key, vt = t
        pf = pool.get()
        try:
            d = os.path.dirname(fe.R + '/' + rec['unit'])
            fd, path = tempfile.mkstemp(suffix='.cpp', prefix='.ws_', dir=d)
            os.write(fd, vt.encode('latin-1'))
            os.close(fd)
            try:
                c, err = fe.compile_unit(rec['unit'], path, pf)
                if c is None:
                    return rec, key, None, None
                r = fe.evaluate(c, vt, int(rec['addr'], 16))
                try:
                    sec = c.sections[c.function(r['sym']).section - 1]
                    fo = framescan.frame(framescan.insns(sec.data, 0))[0]
                except Exception:
                    fo = -1
                return rec, key, r.get('diff'), fo
            finally:
                os.remove(path)
        finally:
            pool.put(pf)
    with ThreadPoolExecutor(jobs) as ex:
        out = list(ex.map(run, tasks))
    for rec, key, diff, fo in out:
        fi, f0 = rec['frame']
        if diff is None:
            tag = 'FAIL'
        else:
            better = abs(fo - fi) < abs(f0 - fi)
            worse = abs(fo - fi) > abs(f0 - fi)
            tag = 'FRAME+' if better else ('FRAME-' if worse else ('ROWS+' if diff < rec['diff'] else
                                                                  ('same' if diff == rec['diff'] else 'rows-')))
        print('SWEEP %-6s %s %s %-40s rows %d -> %s frame %#x -> %s (img %#x)' % (
            tag, rec['addr'], rec['unit'], key, rec['diff'], diff, f0, '%#x' % fo if fo is not None else '-', fi))
    return out


def show(rec, verbose):
    if 'error' in rec:
        print('ERR', rec.get('unit'), rec.get('addr', ''), rec['error'])
        return
    fi, fo = rec['frame']
    print('FUNC %s %s %s diff=%d frame img=%#x ours=%#x%s' % (rec['addr'], rec['unit'], rec['sym'], rec['diff'], fi, fo,
                                                            '' if fi == fo else ' d=%+d' % (fo - fi)))
    for x in rec['reuse']:
        print('  REUSE %-6s %s%s%s' % (x['verdict'], x['name'], ' (param)' if x['param'] else '',
                                       ' allfor' if x['allfor'] else ''))
        for w in x['webs']:
            print('      web defs=%s uses=%s%s' % (fmt_lines(w['defs']), fmt_lines(w['lines']), ' for' if w['loop'] else ''))
        for ls, ss in x['slots']:
            print('      slot-web %s img %s' % (ls, ' '.join('%s:%d' % kv for kv in ss.items()) or '-'))
    for x in rec['copy']:
        print('  COPY  %-7s L%d %s = %s%s img src %s dst %s' % (
            x['verdict'], x['line'], x['dst'], x['src'], ' handoff' if x['handoff'] else ' both-live',
            ' '.join('%s:%d' % kv for kv in x['img_src'].items()) or '-',
            ' '.join('%s:%d' % kv for kv in x['img_dst'].items()) or '-'))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('units', nargs='*')
    ap.add_argument('--addr', default='')
    ap.add_argument('--src')
    ap.add_argument('--from', dest='frm')
    ap.add_argument('--all', action='store_true', help='include claimed markers')
    ap.add_argument('--jobs', type=int, default=int(os.environ.get('FE_JOBS', '5')))
    ap.add_argument('--json')
    ap.add_argument('--sweep', action='store_true', help='compile a split variant per later web and a merge variant per '
                    'hand-off copy; print rows and frame of each (temporary files next to the unit)')
    ap.add_argument('-v', '--verbose', action='store_true', help='also SHARED/ONEWEB/APART and sibling loop counters')
    a = ap.parse_args()
    base = int(os.environ.get('FE_PFX_BASE', '14'))
    work = collections.defaultdict(list)
    if a.frm:
        for e in json.load(open(a.frm)):
            if e.get('exact'):
                continue
            if a.units and e['unit'] not in a.units and e['unit'][4:] not in a.units:
                continue
            work[e['unit']].append(int(e['addr'], 16))
    else:
        units = [u if u.startswith('src/') else 'src/' + u for u in a.units]
        for u in units:
            text = open(fe.R + '/' + u, encoding='latin-1').read()
            want = {int(x, 16) for x in a.addr.split(',') if x}
            for m in re.finditer(r'// ORIGINAL: 0x([0-9A-F]{8}) [^\n]*', text):
                ad = int(m.group(1), 16)
                if want and ad not in want:
                    continue
                if not want and not a.all and 'BYTE_EXACT' in m.group(0):
                    continue
                work[u].append(ad)
    jobs = list(work.items())
    res = []
    if len(jobs) == 1:
        res = scan_unit(jobs[0][0], jobs[0][1], str(base), a.verbose, a.src)
    else:
        import queue
        pool = queue.Queue()
        for n in range(a.jobs):
            pool.put(str(base + n))

        def run(u, ads):
            pf = pool.get()
            try:
                return scan_unit(u, ads, pf, a.verbose)
            finally:
                pool.put(pf)
        with ThreadPoolExecutor(a.jobs) as ex:
            futs = [ex.submit(run, u, ads) for u, ads in jobs]
            for f in futs:
                res += f.result()
    for rec in res:
        show(rec, a.verbose)
    nsplit = sum(1 for r in res for x in r.get('reuse', ()) if x['verdict'] == 'SPLIT')
    nmerge = sum(1 for r in res for x in r.get('copy', ()) if x['verdict'] == 'MERGE')
    print('functions %d split %d merge %d reuse %d copy %d' % (
        len(res), nsplit, nmerge, sum(len(r.get('reuse', ())) for r in res), sum(len(r.get('copy', ())) for r in res)))
    if a.json:
        json.dump(res, open(a.json, 'w'), indent=1)
    if a.sweep:
        sweep(res, a.jobs, base)


if __name__ == '__main__':
    main()
