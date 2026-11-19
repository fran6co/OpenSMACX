#!/usr/bin/env python3
"""orphanowner.py [census|scan|show] ...  auto-inlined owner screen (MATCHING_GUIDE 2.8 "MapWin::click").

A real function in the image with no image callers and no address reference anywhere (no rel32 call/jmp,
no absolute dword in any section: data refs, vtables and thunk tables excluded) was only ever used where
/Ob2 expanded it. When another function of the same unit writes the same block out, that host's source
should call the orphan instead (MapWin::click 0x0046D5D0 / UNK4 0x0046C020).

  census [--min N] [--json F]
      every marker whose start address has no image reference; N = minimum body instructions (default 8)
  scan [unit ...] [--orphan A,..] [--min N] [--any-unit] [--exact-hosts] [--open] [--lines]
       [--thr R] [--prec R] [--named R] [--cover R] [--json F]
      for each orphan, search the image bodies of the other markers of its unit (non-exact hosts by default,
      --exact-hosts adds the BYTE_EXACT ones; --any-unit searches every unit: a hit there is TU evidence)
      for a window whose instruction shape equals the orphan's body. Tokens: mnemonic + operand kinds,
      registers masked, frame offsets masked, immediates kept, absolute addresses and call targets kept
      by name (the image is linked, so a global or a callee is a fixed address). Prologue/epilogue rows
      (push/pop of ebp/esi/edi/ebx, mov ebp,esp, sub/add esp, leave, ret) and argument loads from the
      frame are dropped on both sides, so a parameter bound to a register or a constant still aligns.
      score = matched orphan tokens / orphan tokens over the best host window (difflib), prec = matched
      share of the window, named = ratio of the named (call/global) sequences, cover = window share of
      the host (above --cover the host is a twin: an overload or copy, not an expansion). Output
      `CALLS` = the host's source already names the orphan (a budget question), `HIT` = it does not
      (--open keeps only these): the block is written out or goes through an invented twin
      (terraform_arid_count, emit_help, a forced-inline redraw). --lines maps the image window to our
      source lines through one /FAsc compile of the host unit (fe alignment, equal rows only; a site
      inside an expanded callee maps to the call's line). False positives are common below score 0.9:
      read `show` before editing (shared parse_says/pop sequences, show_all against hide_all).
      Census: 667 orphans of >= 8 instructions, 2026-11-19; 18 s tree-wide, 30 s with --lines.
  show <orphan> <host>   aligned rows of the best window.
Screen only: replace the block with a call, then fe/measure the host and the orphan."""
import sys, os, re, glob, json, struct, argparse, collections, difflib
H = os.path.dirname(os.path.abspath(__file__))
T_ = os.path.dirname(H)
R = os.path.dirname(os.path.dirname(T_))
sys.path.insert(0, T_)
sys.path.insert(0, H)
os.chdir(R)
import fe
import capstone
from capstone import x86 as X

md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
md.detail = True
IMG = fe._img
TEXT_VA, TEXT_VS, TEXT_RO = fe._secs[0]
MRE = re.compile(r'// ORIGINAL: 0x([0-9A-F]{8}) (\S+) 0x([0-9A-Fa-f]+)-0x([0-9A-Fa-f]+)([^\n]*)\n((?://(?! ORIGINAL:)[^\n]*\n)*)')


def markers():
    """-> [dict(unit, addr, sym, lo, hi, claimed, line)] for every ORIGINAL marker under src/."""
    out = []
    for p in sorted(glob.glob(R + '/src/**/*.cpp', recursive=True) + glob.glob(R + '/src/**/*.h', recursive=True)
                    + glob.glob(R + '/src/**/*.c', recursive=True)):
        t = open(p, encoding='latin-1').read()
        for m in MRE.finditer(t):
            sym = m.group(2)
            m2 = re.search(r'// symbol\s+(\S+)', m.group(6))
            if m2:
                sym = m2.group(1)
            frag = 'RECOVERY_FRAGMENT' in m.group(6)
            out.append(dict(unit=os.path.relpath(p, R), addr=int(m.group(1), 16), sym=sym, name=m.group(2),
                            lo=int(m.group(3), 16), hi=int(m.group(4), 16), claimed='BYTE_EXACT' in m.group(5),
                            frag=frag, line=t.count('\n', 0, m.start()) + 1))
    return out


_REFS = None


def refs():
    """-> (rel targets: Counter of rel32 call/jmp targets in .text, abs: set of dword values anywhere)."""
    global _REFS
    if _REFS is None:
        rel = collections.Counter()
        text = IMG[TEXT_RO:TEXT_RO + TEXT_VS]
        for op in (b'\xe8', b'\xe9'):
            i = text.find(op)
            while i >= 0:
                if i + 5 <= len(text):
                    t = (TEXT_VA + i + 5 + struct.unpack_from('<i', text, i + 1)[0]) & 0xffffffff
                    rel[t] += 1
                i = text.find(op, i + 1)
        # jcc rel32 (0f 8x) also reaches function starts (tail jumps)
        i = text.find(b'\x0f')
        while i >= 0:
            if i + 6 <= len(text) and 0x80 <= text[i + 1] <= 0x8f:
                rel[(TEXT_VA + i + 6 + struct.unpack_from('<i', text, i + 2)[0]) & 0xffffffff] += 1
            i = text.find(b'\x0f', i + 1)
        _REFS = (rel, IMG)
    return _REFS


def referenced(addr, lo=None, hi=None):
    """rel32 hits from outside [lo,hi) are not separable from false byte hits; any hit counts."""
    rel, img = refs()
    if rel.get(addr):
        return 'rel'
    if img.find(struct.pack('<I', addr)) >= 0:
        return 'abs'
    return None


def body(lo, hi):
    return [i for i in md.disasm(fe.imgbytes(lo, hi), lo)]


CS = {'ebp', 'esi', 'edi', 'ebx'}


def tok(ins, a2s):
    """normalised token of one image instruction, or None for a prologue/epilogue/argument row.
    -> (shape, named) with named = the callee/global name or None."""
    mn = ins.mnemonic
    ops = ins.operands
    if mn in ('ret', 'leave', 'nop', 'int3'):
        return None
    if mn in ('push', 'pop') and len(ops) == 1 and ops[0].type == X.X86_OP_REG and ins.reg_name(ops[0].reg) in CS:
        return None
    if mn in ('sub', 'add') and len(ops) == 2 and ops[0].type == X.X86_OP_REG and ins.reg_name(ops[0].reg) == 'esp':
        return None
    if mn == 'mov' and len(ops) == 2 and ops[0].type == X.X86_OP_REG and ops[1].type == X.X86_OP_REG and \
            {ins.reg_name(ops[0].reg), ins.reg_name(ops[1].reg)} == {'ebp', 'esp'}:
        return None
    named = None
    parts = []
    for o in ops:
        if o.type == X.X86_OP_REG:
            parts.append('R')
        elif o.type == X.X86_OP_IMM:
            v = o.imm & 0xffffffff
            if mn == 'call' or mn.startswith('j'):
                if mn == 'call':
                    named = a2s.get(v, 'sub_%x' % v)
                    parts.append(named)
                else:
                    parts.append('L')
            elif 0x400000 <= v < 0xa00000:
                named = a2s.get(v, '0x%x' % v)
                parts.append('&' + named)
            else:
                parts.append('#%x' % v)
        elif o.type == X.X86_OP_MEM:
            m = o.mem
            base = ins.reg_name(m.base) if m.base else None
            if base in ('esp', 'ebp') and not m.index:
                # frame slot or argument: an argument load (mov r,[arg]) vanishes when inlined
                if mn == 'mov' and len(ops) == 2 and ops[0].type == X.X86_OP_REG and o is ops[1]:
                    return None
                parts.append('[F]')
            elif not m.base and not m.index and m.segment == 0:
                v = m.disp & 0xffffffff
                if 0x400000 <= v < 0xa00000:
                    named = a2s.get(v, '0x%x' % v)
                    parts.append('[' + named + ']')
                else:
                    parts.append('[#%x]' % v)
            else:
                d = m.disp & 0xffffffff
                if 0x400000 <= d < 0xa00000:
                    named = a2s.get(d, '0x%x' % d)
                    parts.append('[R+%s%s]' % (named, '*%d' % m.scale if m.index else ''))
                else:
                    parts.append('[R%s+%x]' % ('*%d' % m.scale if m.index else '', m.disp & 0xffffffff))
    return (mn + ' ' + ','.join(parts), named)


def toks(lo, hi, a2s):
    out = []
    for ins in body(lo, hi):
        t = tok(ins, a2s)
        if t:
            out.append((ins.address, t[0], t[1]))
    return out


def a2s_map(ms):
    d = {}
    for m in ms:
        d.setdefault(m['addr'], m['sym'])
    return d


def short_name(mangled):
    m = re.match(r'\?\?1(\w+)@', mangled)
    if m:
        return '~' + m.group(1)
    m = re.match(r'\?\?0(\w+)@', mangled)
    if m:
        return m.group(1)
    m = re.match(r'\?(\w+)@', mangled)
    if m:
        return m.group(1)
    m = re.match(r'_?(\w+)', mangled)
    return m.group(1) if m else None


def best_window(O, Hs):
    """O, Hs: token lists [(addr, shape, named)]. -> (score, named_ok, ilo, ihi, matched) for the best host window."""
    A = [x[1] for x in O]
    n = len(A)
    B = [x[1] for x in Hs]
    if not n or len(B) < n // 2:
        return None
    # candidate starts: positions where a rare orphan token occurs in the host
    freq = collections.Counter(B)
    keys = sorted(set(A), key=lambda s: (freq.get(s, 0) == 0, freq.get(s, 0)))
    anchors = [k for k in keys if freq.get(k, 0)][:3]
    starts = set()
    for k in anchors:
        oi = A.index(k)
        for j, b in enumerate(B):
            if b == k:
                starts.add(max(0, j - oi - n // 4))
    best = None
    for s in sorted(starts):
        e = min(len(B), s + n + n // 2 + 2)
        sm = difflib.SequenceMatcher(None, A, B[s:e], autojunk=False)
        blocks = sm.get_matching_blocks()
        matched = sum(b.size for b in blocks)
        if not matched:
            continue
        first = min(b.b for b in blocks if b.size)
        last = max(b.b + b.size for b in blocks if b.size)
        sc = matched / n
        # window precision: matched host tokens over the window's host tokens
        prec = matched / max(1, last - first)
        key = (sc * prec, sc)
        if best is None or key > best[0]:
            best = (key, sc, prec, s + first, s + last, matched)
    if best is None:
        return None
    _, sc, prec, ilo, ihi, matched = best
    on = [x[2] for x in O if x[2]]
    hn = [x[2] for x in Hs[ilo:ihi] if x[2]]
    named_ok = difflib.SequenceMatcher(None, on, hn, autojunk=False).ratio() if on else None
    return dict(score=sc, prec=prec, named=named_ok, ilo=ilo, ihi=ihi, matched=matched, n=n)


def census(ms, mn):
    out = []
    starts = {m['addr'] for m in ms}
    for m in ms:
        if m['frag'] or m['hi'] - m['lo'] < 4:
            continue
        r = referenced(m['addr'])
        if r:
            continue
        nb = len([1 for i in body(m['lo'], m['hi']) if i.mnemonic not in ('nop', 'int3')])
        if nb < mn:
            continue
        m = dict(m, ninsn=nb)
        out.append(m)
    return out


_FE = None


def fe_exact():
    """addr -> exact from a feall json when given (FEALL env), else claimed flag."""
    global _FE
    if _FE is None:
        _FE = {}
        p = os.environ.get('FEALL')
        if p and os.path.exists(p):
            for d in json.load(open(p)):
                try:
                    _FE[int(d['addr'], 16)] = d['exact']
                except Exception:
                    pass
    return _FE


def scan(ms, orphans, a):
    a2s = a2s_map(ms)
    byunit = collections.defaultdict(list)
    for m in ms:
        if not m['frag']:
            byunit[m['unit']].append(m)
    fx = fe_exact()
    hits = []
    tokc = {}

    def tk(m):
        k = (m['lo'], m['hi'])
        if k not in tokc:
            tokc[k] = toks(m['lo'], m['hi'], a2s)
        return tokc[k]
    texts = {}

    def hbody(h):
        if h['unit'] not in texts:
            texts[h['unit']] = open(R + '/' + h['unit'], encoding='latin-1').read().split('\n')
        L = texts[h['unit']]
        nxt = min([m['line'] for m in byunit[h['unit']] if m['line'] > h['line']] + [len(L) + 1])
        return '\n'.join(L[h['line'] - 1:nxt - 1])
    for o in orphans:
        O = tk(o)
        short = short_name(o['sym'] if o['sym'].startswith('?') else o['name'])
        if len(O) < a.min:
            continue
        pool = [m for u, l in byunit.items() for m in l] if a.any_unit else byunit[o['unit']]
        for h in pool:
            if h['addr'] == o['addr']:
                continue
            ex = fx.get(h['addr'], h['claimed'])
            if ex and not a.exact_hosts:
                continue
            Hs = tk(h)
            if len(Hs) < len(O) * 0.6:
                continue
            w = best_window(O, Hs)
            if not w or w['score'] < a.thr or w['prec'] < a.prec:
                continue
            if w['named'] is not None and w['named'] < a.named:
                continue
            cover = (w['ihi'] - w['ilo']) / max(1, len(Hs))
            if cover > a.cover:   # a twin (overload, copy), not a host that expands the orphan
                continue
            called = bool(short) and re.search(r'(?<![\w~])' + re.escape(short) + r'\s*\(', hbody(h)) is not None
            hits.append(dict(called=called, orphan='%08X' % o['addr'], osym=o['sym'], ounit=o['unit'], host='%08X' % h['addr'],
                             hsym=h['sym'], hunit=h['unit'], hexact=bool(ex), score=round(w['score'], 3),
                             prec=round(w['prec'], 3), named=None if w['named'] is None else round(w['named'], 3),
                             n=w['n'], cover=round(cover, 2), img_lo='%x' % Hs[w['ilo']][0], img_hi='%x' % Hs[w['ihi'] - 1][0]))
    return hits


_UC = {}


def unit_compile(unit, pfx):
    """-> (Coff, cod text, source text) of one raw /FAsc compile of a unit (cached)."""
    if unit in _UC:
        return _UC[unit]
    import subprocess
    from match_recovery.pe_coff import Coff
    cmd = fe.compile_cmd(unit)
    if not cmd:
        _UC[unit] = None
        return None
    asm = fe.SCR + '/oo_%d.cod' % os.getpid()
    obj = fe.SCR + '/oo_%d.obj' % os.getpid()
    out = []
    for t in cmd:
        if t.startswith('/Fo'):
            t = '/Fo' + obj
        elif t.startswith('/Fd') or t.startswith('--repro-root') or t == '/W4':
            continue
        out.append(t)
    out.insert(-1, '/FAsc')
    out.insert(-1, '/Fa' + asm)
    env = dict(os.environ, VC6_PREFIX='%s/.opensmacx/wine/worker-%s' % (fe.MAIN, pfx))
    for f in (asm, obj):
        os.path.exists(f) and os.remove(f)
    subprocess.run(out, cwd=fe.SCR, env=env, capture_output=True, text=True)
    if not (os.path.exists(asm) and os.path.exists(obj)):
        _UC[unit] = None
        return None
    r = (Coff(open(obj, 'rb').read()), open(asm, errors='replace').read().replace('\r', ''),
         open(R + '/' + unit, encoding='latin-1').read())
    os.remove(asm)
    os.remove(obj)
    _UC[unit] = r
    return r


def cod_lines(cod, sym):
    """-> {offset: source line} of one function in a /FAsc listing."""
    i = cod.find(sym + ' PROC')
    if i < 0:
        return {}
    e = cod.find(sym + ' ENDP', i)
    res, line, last = {}, 0, -1
    for l in cod[i:e].split('\n'):
        m = re.match(r'; (\d+)\s*:', l)
        if m:
            line = int(m.group(1))
            continue
        m = re.match(r'  ([0-9a-f]{5})\t', l)
        if m:
            off = int(m.group(1), 16)
            if off < last:
                break
            last = off
            res[off] = line
    return res


def host_lines(h, lo, hi, pfx):
    """our source lines whose code aligns (fe equal rows) with image [lo, hi] of host h."""
    u = unit_compile(h['unit'], pfx)
    if not u:
        return None
    c, cod, text = u
    r = fe.evaluate(c, text, h['addr'])
    if 'error' in r:
        return None
    ol = cod_lines(cod, r['sym'])
    lines = []
    for k, o, c_ in r['rows']:
        if o and c_ and lo <= h['lo'] + o[0] <= hi and c_[0] in ol:
            lines.append(ol[c_[0]])
    lines = [x for x in lines if x]
    if not lines:
        return None
    lines.sort()
    q = lines[len(lines) // 10], lines[(len(lines) * 9) // 10]
    return dict(lo=lines[0], hi=lines[-1], p10=q[0], p90=q[1], diff=r['diff'], exact=r['exact'])


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('cmd', choices=['census', 'scan', 'show'])
    ap.add_argument('rest', nargs='*')
    ap.add_argument('--min', type=int, default=8)
    ap.add_argument('--thr', type=float, default=0.6, help='minimum matched share of the orphan tokens')
    ap.add_argument('--prec', type=float, default=0.75, help='minimum matched share of the host window')
    ap.add_argument('--named', type=float, default=0.6, help='minimum ratio of the named (call/global) sequences')
    ap.add_argument('--cover', type=float, default=0.8, help='maximum window share of the host (above: a twin)')
    ap.add_argument('--orphan')
    ap.add_argument('--any-unit', action='store_true')
    ap.add_argument('--exact-hosts', action='store_true')
    ap.add_argument('--json')
    ap.add_argument('--lines', action='store_true')
    ap.add_argument('--open', action='store_true', help='only hits whose host does not call the orphan already')
    a = ap.parse_args()
    ms = markers()
    if a.cmd == 'census':
        cs = census(ms, a.min)
        by = collections.Counter(m['unit'] for m in cs)
        for m in cs:
            print('ORPHAN %08X %-40s %-34s n=%d %s' % (m['addr'], m['sym'][:40], m['unit'], m['ninsn'],
                                                      'BYTE_EXACT' if m['claimed'] else ''))
        print('orphans', len(cs), 'units', len(by))
        if a.json:
            json.dump(cs, open(a.json, 'w'), indent=0)
        return
    if a.cmd == 'show':
        a2s = a2s_map(ms)
        oa, ha = int(a.rest[0], 16), int(a.rest[1], 16)
        o = next(m for m in ms if m['addr'] == oa)
        h = next(m for m in ms if m['addr'] == ha)
        O = toks(o['lo'], o['hi'], a2s)
        Hs = toks(h['lo'], h['hi'], a2s)
        w = best_window(O, Hs)
        print(w)
        A = [x[1] for x in O]
        B = [x[1] for x in Hs[w['ilo']:w['ihi']]]
        for t, i1, i2, j1, j2 in difflib.SequenceMatcher(None, A, B, autojunk=False).get_opcodes():
            for k in range(max(i2 - i1, j2 - j1)):
                oa_ = O[i1 + k] if i1 + k < i2 else None
                hb = Hs[w['ilo'] + j1 + k] if j1 + k < j2 else None
                print('%s %-6s %-44s | %-6s %s' % ('  ' if t == 'equal' else '!!', '%x' % oa_[0] if oa_ else '',
                                                   oa_[1] if oa_ else '', '%x' % hb[0] if hb else '', hb[1] if hb else ''))
        return
    cs = census(ms, a.min)
    if a.orphan:
        want = {int(x, 16) for x in a.orphan.split(',')}
        cs = [m for m in cs if m['addr'] in want]
    if a.rest:
        cs = [m for m in cs if m['unit'] in a.rest or os.path.basename(m['unit']) in a.rest]
    hits = scan(ms, cs, a)
    if a.open:
        hits = [h for h in hits if not h['called']]
    hits.sort(key=lambda h: (-h['score'] * h['prec'], h['orphan']))
    if a.lines:
        pfx = os.environ.get('FE_PFX_BASE', '21')
        for h in hits:
            if h['called'] or (h['hexact'] and not a.exact_hosts):
                continue
            hm = next(m for m in ms if m['addr'] == int(h['host'], 16) and m['unit'] == h['hunit'])
            h['src'] = host_lines(hm, int(h['img_lo'], 16), int(h['img_hi'], 16), pfx)
    for h in hits:
        print('%s %s %-30s -> %s %-34s %s %s img %s-%s score=%.2f prec=%.2f named=%s n=%d cover=%.2f' % (
            'CALLS' if h['called'] else 'HIT  ', h['orphan'], h['osym'][:30], h['host'], h['hsym'][:34], 'EXACT' if h['hexact'] else 'open',
            os.path.basename(h['hunit']) + ('' if h['hunit'] == h['ounit'] else '(!=' + os.path.basename(h['ounit']) + ')'),
            h['img_lo'], h['img_hi'], h['score'], h['prec'], h['named'], h['n'], h['cover']))
        if h.get('src'):
            x = h['src']
            print('      src %s:%d-%d (core %d-%d) host fe diff=%d' % (h['hunit'], x['lo'], x['hi'], x['p10'], x['p90'], x['diff']))
    print('orphans', len(cs), 'hits', len(hits))
    if a.json:
        json.dump(hits, open(a.json, 'w'), indent=0)


if __name__ == '__main__':
    main()
