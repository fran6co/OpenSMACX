#!/usr/bin/env python3
"""reorder.py <unit> [--hoist] [--write] [--force] [--out F]

Put a unit's annotation blocks in image address order (MATCHING_GUIDE 2.8: a unit's literal run is its
definition order). A block is the leading comment lines, the ORIGINAL marker with its RECOVERY_BINDING and
fact lines, and the reader's own region (the body); a marker without a body keeps the global or guarded
block right below it, and a block that opens `#pragma auto_inline(off)` keeps the line that closes it.
File-scope code between blocks (globals, helpers, #define) travels in front of the block that followed it.
`#pragma` and `#if` lines between blocks are barriers: blocks are only sorted between two barriers, so no
function crosses a pragma region. --hoist folds mid-file #include lines into the first include run. Blocks
above the first include run stay where they are.
Repairs, each printed: `HOIST` moves file-scope code above its first user after a C2065; `FORWARD` adds a
forward declaration when the definition is a later marked function; `GLUE` keeps a marker whose region ends
at the next marker right above a marker block.
Checks before --write: the source reader returns the same records (identity, facts, claims, region text
without #include lines); every code section of the object is byte-identical with the same relocation
targets (literals by text). --force writes with listed code changes. Also prints the unitlits.py distance
before and after and the RECOVERY_OBJECT address-order descents.
Not checked here: `// symbol _$E<n>` facts of static initialisers (numbered by the front-end counter; fe
reports `nosymbol` when one moves: rewrite with match_recovery.source.writer.write_symbols) and
RECOVERY_LITERAL $SG numbers (`recover rebind-literals`)."""
import sys, os, re, difflib
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE)); sys.path.insert(0, HERE)
import fe
R = fe.R
from match_recovery.source import read_text
from pathlib import Path

def records(text, path):
    return read_text(text, Path(path))

from match_recovery.source import reader as _rd

INC = re.compile(r'^\s*#\s*include\b')
def extents(text, path):
    """[(record, start_index, end_index)] with the reader's own region rule (also for FILE-mode markers)"""
    lines = text.split('\n')
    blk = []
    st = False
    for l in lines:
        blk.append(st); st = _rd._block_state_after(l, st)
    out = []
    for r in sorted(records(text, path), key=lambda r: r.line):
        s = r.line - 1
        try:
            e = _rd._region_end(lines, s, r.name, in_block=blk[s])
        except Exception:
            e = None
        if e is None: e = s
        out.append((r, s, e))
    return lines, blk, out

def keys(text, path):
    lines, blk, ex = extents(text, path)
    return sorted((r.address, r.name, r.symbol, r.body, r.context, r.kind, r.byte_exact, r.semantic,
            '\n'.join(l for l in lines[s:e + 1] if not INC.match(l)), r.image_spans, str(r.source_role), r.source_role_reason, r.levers, r.ruled_out,
            r.unrecoverable, r.deferred, str(r.state), str(r.mode), str(r.function_fragment)) for r, s, e in ex)

def chunks(text, path):
    lines, blk, ex = extents(text, path)
    spans = []
    for r, s, e in ex:
        j = s
        if blk[s]:
            while j > 0 and blk[j]: j -= 1
        while j > 0 and lines[j - 1].lstrip().startswith('//') and not blk[j - 1]:
            j -= 1
        k = s
        while k + 1 < len(lines) and lines[k + 1].lstrip().startswith('//') and not blk[k + 1] and 'ORIGINAL:' not in lines[k + 1] and 'RECOVERY_ROLE' not in lines[k + 1]:
            k += 1
        if e <= k and k + 1 < len(lines) and lines[k + 1].strip() and not re.match(r'\s*#\s*(include|define|undef|pragma)', lines[k + 1]) and not blk[k + 1]:
            # a marker without a function body whose subject (a global, a guarded block) sits right below it
            while k + 1 < len(lines) and lines[k + 1].strip():
                k += 1
        spans.append([j, max(e, k), r])
    spans.sort(key=lambda x: x[0])
    out = []
    for j, e, r in spans:
        if out and j <= out[-1][1]:
            out[-1][1] = max(out[-1][1], e); out[-1][2].append(r)
        else:
            out.append([j, e, [r]])
    for c in out:
        k = c[1]
        while k + 1 < len(lines) and blk[k + 1] : k += 1
        if k > c[1]: c[1] = k
    # a block that opens an auto_inline/inline_depth region keeps the line that closes it
    for c in out:
        t = '\n'.join(lines[c[0]:c[1] + 1])
        if t.count('#pragma auto_inline(off)') > t.count('#pragma auto_inline(on)'):
            k = c[1] + 1
            while k < len(lines) and not lines[k].strip(): k += 1
            if k < len(lines) and lines[k].strip() == '#pragma auto_inline(on)':
                c[1] = k
    # blocks above the end of the first #include run stay in the preamble
    first = next((i for i, l in enumerate(lines) if INC.match(l) and not blk[i]), None)
    if first is not None:
        last = first
        i = first + 1
        while i < len(lines) and (INC.match(lines[i]) or not lines[i].strip() or lines[i].lstrip().startswith('//') or blk[i] or re.match(r'\s*#\s*define\s+\w+\s*(//.*)?$', lines[i])):
            if INC.match(lines[i]): last = i
            i += 1
        out = [c for c in out if c[0] > last]
    return lines, out

PP = re.compile(r'^\s*#\s*(pragma|if|ifdef|ifndef|else|elif|endif|include)\b')

INC = re.compile(r'^\s*#\s*include\b')
def hoist_includes(text, path):
    lines, ch = chunks(text, path)
    pre_end = ch[0][0]
    last_inc = max([i for i in range(pre_end) if INC.match(lines[i])], default=None)
    have = set(re.sub(r'\s*//.*', '', lines[i]).strip() for i in range(pre_end) if INC.match(lines[i]))
    take = []; drop = set()
    for i in range(1, len(ch)):
        a, b = ch[i - 1][1] + 1, ch[i][0]
        pp = [k for k in range(a, b) if PP.match(lines[k])]
        if pp and all(INC.match(lines[k]) for k in pp):
            for k in pp:
                drop.add(k)
                key = re.sub(r'\s*//.*', '', lines[k]).strip()
                if key not in have:
                    have.add(key); take.append(lines[k])
    if not drop: return text, 0
    out = []
    for i, l in enumerate(lines):
        if i in drop: continue
        if last_inc is None and i == pre_end: out += take + ['']
        out.append(l)
        if i == last_inc: out += take
    return '\n'.join(out), len(drop)

def iscode(ls):
    return [l for l in ls if l.strip() and not l.lstrip().startswith('//')]

def plan(text, path, pins=()):
    """-> pieces [dict(kind='pre'|'inter'|'body'|'tail'|'fwd', lines, addr, bodiless)], report, moved, n, nseg"""
    lines, ch = chunks(text, path)
    lines2, blk, _ = extents(text, path)
    items = []
    report = []
    for i, (j, e, rs) in enumerate(ch):
        prev_end = ch[i - 1][1] + 1 if i else None
        inter = lines[prev_end:j] if i else []
        barrier = any(PP.match(l) for l in inter)
        code = iscode(inter)
        if code and not barrier:
            report.append(('CODE before %08X' % rs[0].address, code[:3]))
        if barrier:
            report.append(('BARRIER before %08X' % rs[0].address, [l for l in inter if PP.match(l)]))
        body = lines[j:e + 1]
        bodiless = not any('{' in lines[q] for q in range(j, e + 1) if not blk[q] and not lines[q].lstrip().startswith('//'))
        items.append(dict(addr=min(r.address for r in rs), inter=inter, body=body, barrier=barrier, bodiless=bodiless))
    segs, cur = [], []
    for it in items:
        if it['barrier'] and cur:
            segs.append(cur); cur = []
        cur.append(it)
    segs.append(cur)
    pieces = [dict(kind='pre', lines=lines[:ch[0][0]])]
    moved = 0
    for seg in segs:
        order = sorted(seg, key=lambda it: it['addr'])
        if seg[0]['barrier']:
            pieces.append(dict(kind='inter', lines=seg[0]['inter'], barrier=True))
        for it in order:
            if not (it is seg[0] and seg[0]['barrier']) and it['inter']:
                pieces.append(dict(kind='inter', lines=it['inter']))
            pieces.append(dict(kind='body', lines=it['body'], addr=it['addr'], bodiless=it['bodiless']))
        moved += sum(1 for a, b in zip(seg, order) if a is not b)
    pieces.append(dict(kind='tail', lines=lines[ch[-1][1] + 1:]))
    normalize(pieces)
    return pieces, report, moved, len(items), len(segs)

def normalize(pieces):
    # a bodiless marker block must not come to sit right above file-scope code (its region would grow):
    # such code hops above the run of bodiless blocks
    i = 0
    while i < len(pieces):
        p = pieces[i]
        if p['kind'] == 'inter' and iscode(p['lines']) and not p.get('barrier'):
            j = i
            while j > 0 and ((pieces[j - 1]['kind'] == 'body' and pieces[j - 1]['bodiless']) or
                             (pieces[j - 1]['kind'] == 'inter' and not iscode(pieces[j - 1]['lines']))):
                j -= 1
            while j < i and pieces[j]['kind'] != 'body': j += 1
            if j < i:
                pieces.insert(j, pieces.pop(i))
        i += 1
    # a bodiless block right above code that cannot hop (a barrier, the tail): the block moves up instead,
    # to sit right above the nearest earlier block with a body
    for _ in range(50):
        done = True
        for i, p in enumerate(pieces):
            if not (p['kind'] == 'body' and p['bodiless']): continue
            j = i + 1
            while j < len(pieces) and pieces[j]['kind'] == 'inter' and not iscode(pieces[j]['lines']) and not pieces[j].get('barrier'): j += 1
            if j < len(pieces) and (pieces[j]['kind'] == 'tail' and not iscode(pieces[j]['lines'])): continue
            if j >= len(pieces) or pieces[j]['kind'] == 'body': continue
            k = i - 1
            while k >= 0 and not (pieces[k]['kind'] == 'body' and not pieces[k]['bodiless']): k -= 1
            if k < 0: continue
            pieces.insert(k, pieces.pop(i)); done = False; break
        if done: break

def render(pieces):
    out = []; owner = []
    for pi, p in enumerate(pieces):
        ls = p['lines']
        if not ls: continue
        if out and out[-1].strip() and ls[0].strip():
            out.append(''); owner.append(pi)
        out.extend(ls); owner.extend([pi] * len(ls))
    return '\n'.join(out), owner

def signature(lines, name):
    """forward declaration text of the col-0 function `name` defined in lines"""
    for i, l in enumerate(lines):
        if l[:1] not in (' ', '\t', '/', '*', '#', '}', '') and re.search(r'\b%s\s*\(' % re.escape(name), l):
            sig = []
            for q in range(i, min(i + 12, len(lines))):
                sig.append(lines[q])
                if '{' in lines[q]: break
            s = ' '.join(x.strip() for x in sig)
            if '{' not in s: return None
            s = s[:s.index('{')].strip()
            s = re.sub(r'\s*:\s*[A-Za-z_]\w*\(.*$', '', s) if '::' not in s else s
            s = re.sub(r'^(static\s+)?inline\s+', lambda m: (m.group(1) or '') + 'inline ', s)
            return s + ';'
    return None

def fix_loop(pieces, unit, tmp, pfx, log):
    for _ in range(60):
        normalize(pieces)
        text, owner = render(pieces)
        open(tmp, 'w', encoding='latin-1').write(text)
        c, err = fe.compile_unit(unit, tmp, pfx)
        if c is not None:
            B = {k[0]: k for k in keys(text, R + '/' + unit)}
            grown = [ad for ad, k in ORIG.items() if ad in B and B[ad][8] != k[8]]
            hit = False
            for p in pieces:
                if p['kind'] == 'body' and not p['bodiless'] and any(('0x%08X' % ad) in l for ad in grown for l in p['lines'] if 'ORIGINAL:' in l):
                    p['bodiless'] = True; hit = True
                    log.append('GLUE %08X: its region ends at the next marker, kept right above a marker block' % p['addr'])
            if hit: continue
            return text, c
        m = re.search(r"\((\d+)\) : error C2065: '(\w+)' : undeclared identifier", err)
        if not m:
            log.append('UNFIXABLE: ' + err[:600]); return text, None
        ln, name = int(m.group(1)) - 1, m.group(2)
        P = owner[ln]
        pat = re.compile(r'\b%s\b' % re.escape(name))
        D = None
        for qi in range(P + 1, len(pieces)):
            q = pieces[qi]
            if q['kind'] in ('inter', 'tail') and not q.get('barrier') and any(pat.search(l) for l in iscode(q['lines'])):
                D = qi; break
            if q['kind'] == 'body' and any(pat.search(l) and l[:1] not in (' ', '\t', '/', '*') for l in q['lines']):
                D = qi; break
        if D is None:
            log.append('UNFIXABLE: no later definition of %s' % name); return text, None
        q = pieces[D]
        # insertion point: before P, and above the bodiless run / own inter right above it
        at = P
        if pieces[P]['kind'] == 'body':
            while at > 0 and ((pieces[at - 1]['kind'] == 'body' and pieces[at - 1]['bodiless']) or
                              (pieces[at - 1]['kind'] == 'inter' and not pieces[at - 1].get('barrier') and not iscode(pieces[at - 1]['lines']))):
                at -= 1
            while at < P and pieces[at]['kind'] != 'body': at += 1
        if q['kind'] == 'inter':
            pieces.insert(at, pieces.pop(D))
            log.append('HOIST file-scope code defining %s above %s' % (name, ('%08X' % pieces[P + 1]['addr']) if pieces[P + 1]['kind'] == 'body' else 'code'))
        elif q['kind'] == 'tail':
            log.append('UNFIXABLE: %s defined in the file tail' % name); return text, None
        else:
            sig = signature(q['lines'], name)
            if not sig:
                log.append('UNFIXABLE: no signature for %s' % name); return text, None
            pieces.insert(at, dict(kind='inter', lines=[sig], fwd=True))
            log.append('FORWARD %s' % sig)
    log.append('UNFIXABLE: too many rounds')
    return text, None

def seam(out, piece):
    if out and piece and out[-1].strip() and piece[0].strip():
        out.append('')
    out.extend(piece)

def code_map(c):
    out = {}
    import litgap
    def lit(sy):
        sec = c.sections[sy.section - 1]
        d = sec.data; e = d.find(b'\0', sy.value)
        return 'SG:' + d[sy.value:e].decode('latin-1')
    for sy in c.symbols.values():
        if sy.section <= 0 or sy.value != 0 or sy.storage != 2: continue
        sec = c.sections[sy.section - 1]
        if not (sec.flags & 0x20): continue
        rel = []
        for off, si, ty in sec.relocations:
            t = c.symbols[si]
            nm = t.name
            if nm.startswith('$SG') and t.section > 0: nm = lit(t)
            elif re.match(r'\$[LT]\d+|__ehfuncinfo|__unwind|_\$', nm) or nm.startswith('$'): nm = re.sub(r'\d+', 'N', nm)
            rel.append((off, nm, ty))
        out[re.sub(r'(\?%[^@]*?)\d+@@', r'\1N@@', sy.name)] = (sec.data, tuple(rel))
    return out

def main():
    a = sys.argv[1:]
    write = '--write' in a
    if write: a.remove('--write')
    outp = None
    if '--out' in a:
        i = a.index('--out'); outp = a[i + 1]; del a[i:i + 2]
    unit = [x for x in a if not x.startswith('--')][0]
    path = R + '/' + unit
    text = open(path, encoding='latin-1').read()
    hoisted = 0
    text0 = text
    if '--hoist' in a:
        text, hoisted = hoist_includes(text, path)
        print('  hoisted/dropped %d mid-file #include lines' % hoisted)
    pieces, report, moved, n, nseg = plan(text, path)
    text_h = text
    text = text0
    print('%s: %d blocks, %d segments, %d positions change' % (unit, n, nseg, moved))
    for k, v in report: print('  ', k, v)
    if moved == 0:
        print('  already in order'); return 0
    tmp = outp or (fe.SCR + '/reorder_' + unit.replace('/', '_'))
    pfx = os.environ.get('FE_PFX_BASE', '2')
    log = []
    global ORIG
    ORIG = {k[0]: k for k in keys(text0, path)}
    new, c1 = fix_loop(pieces, unit, tmp, pfx, log)
    for l in log: print('  ', l)
    A = keys(text0, path); B = keys(new, path)
    if A != B:
        print('  READER MISMATCH: %d vs %d records' % (len(A), len(B)))
        da = {k[0]: k for k in A}; db = {k[0]: k for k in B}
        for ad in sorted(set(da) | set(db)):
            if da.get(ad) != db.get(ad):
                x, y = da.get(ad), db.get(ad)
                if x is None or y is None: print('    %08X missing/duplicate' % ad); continue
                for fi, (p, q) in enumerate(zip(x, y)):
                    if p != q:
                        if fi == 8:
                            pl, ql = p.split('\n'), q.split('\n')
                            print('    %08X region %d -> %d lines; old tail %r | new tail %r' % (ad, len(pl), len(ql), pl[-1][:50], ql[-1][:50]))
                        else: print('    %08X field %d: %r -> %r' % (ad, fi, str(p)[:60], str(q)[:60]))
        ok = False
    else:
        ok = True
    if c1 is None:
        print('  COMPILE FAIL'); return 1
    c0, e0 = fe.compile_unit(unit, path, pfx)
    m0, m1 = code_map(c0), code_map(c1)
    ch = [s for s in sorted(set(m0) | set(m1)) if m0.get(s) != m1.get(s)]
    print('  code sections %d -> %d, changed %d' % (len(m0), len(m1), len(ch)))
    for s in ch[:40]:
        why = 'missing' if s not in m0 or s not in m1 else ('bytes' if m0[s][0] != m1[s][0] else 'relocs')
        print('    CHANGED %s (%s)' % (s, why))
    import unitlits
    b0 = unitlits.litdiff(unit, c0)[0]; b1 = unitlits.litdiff(unit, c1)[0]
    print('  LITS differing %d -> %d' % (b0, b1))
    def objorder(tx):
        ad = [int(x, 16) for x in re.findall(r'RECOVERY_OBJECT: v\d 0x([0-9A-Fa-f]{8})', tx)]
        sec = lambda a: max((s[0] for s in fe._secs if s[0] <= a), default=0)
        return sum(1 for x, y in zip(ad, ad[1:]) if sec(x) == sec(y) and y < x), len(ad)
    print('  OBJ descents %d -> %d of %d' % (objorder(text)[0], objorder(new)[0], objorder(new)[1]))
    if ok and (not ch or '--force' in sys.argv) and write:
        open(path, 'w', encoding='latin-1').write(new); print('  WROTE')
    elif write:
        print('  NOT WRITTEN')
    return 0

if __name__ == '__main__':
    sys.exit(main())
