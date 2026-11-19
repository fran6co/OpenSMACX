#!/usr/bin/env python3
"""rotscan.py <unit> <0xADDR[,..]> [--src ALT.cpp] [--pfx N] [--track] | --tree [unit ..] [--from FEALL.json] [--json F]

Scratch-register rotation scan (MATCHING_GUIDE 3.2 S1/S7, 3.7 "Cross-jumping ground truth" rule 2).
VC6 hands out eax -> ecx -> edx round-robin per new scratch value across the whole function, so one
scratch load more or less anywhere re-pairs every later register choice and every later tail merge.
This tool asks one question per function: where is ours one scratch load off from the image?

1. Phase walk. Both listings are aligned with eax/ecx/edx abstracted (fe.align over abstracted text).
   Every instruction that starts a new value in a scratch register (schedscan.alloc_reg: mov/movsx/
   movzx/lea r,.. not reading r, xor r,r; thiscall receivers excluded) is a load. Each aligned pair of
   loads gets the phase offset (ours - image) mod 3 (+1 = ours one load ahead, +2 = one behind). The
   offsets are grouped by our source line (from the /FAsc listing; image rows take the line of their
   aligned partner) in our code order, and the FIRST statement where the offset leaves 0 and stays off
   (most of the next --persist paired loads are off) is the divergence: the report prints the image
   address, our line, the offset, and the window since the last in-sync load: the loads present on one
   side only (unaligned rows) with their lines and the load counts of both sides in the window. A load
   only on our side in the window is the usual cause (a reload the image reuses, a temporary the image
   folds, a materialised constant); one only on the image side the reverse. S7 named values sit
   outside the cursor (lowest free register) and can make single pairs read 0 inside an off stretch:
   the persistence test skips them.
2. Merge phases. Every unconditional `jmp` present on one side only (or with another target, `!>`) is
   a tail merge (cross-jump or join) in one build and not the other. For each, the side without the
   merge keeps a copy of the tail: the copy is compared with that side's instructions at the merge
   target, and the report prints the rotation k (0, +1, +2) under which the copy equals the host and
   for how many instructions. k != 0 = the two sites sit a non-multiple of three loads apart on that
   side (rule 2), so the merge is lost by the phase, not by the source form; k == 0 with a full match
   = the copies are identical, so the merge was refused by something else (pending pops, successor,
   host choice; 3.7).
--track prints the whole per-statement phase track.
Tree mode (--tree, optional unit filter, or --from feall.json) scans every non-exact function and prints
one line per function: DIVERGE (with address/line/offset/window counts), EQUAL (no off stretch), or
NOPAIRS; FE_JOBS lanes from FE_PFX_BASE. Screen only: confirm with fe.py."""
import sys, os, re, json, argparse
from concurrent.futures import ThreadPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
sys.path.insert(0, HERE)
import fe
import tailscan
from schedscan import abstract, alloc_reg as _alloc_reg, rename, ROT, SCR

EPI = re.compile(r'(pop |leave|ret|jmp L|mov esp, |mov ecx, dword ptr \[e[bs]p [-+] 0x[0-9a-f]+\]$|mov dword ptr fs:)')


def alloc_reg(seq, i):
    """schedscan's allocation test, minus the return value: `mov eax, K` / `xor eax, eax` followed only by
    epilogue instructions (pops, EH unlink, ret, a jmp to the shared epilogue) is fixed eax, not a cursor
    step (probe rot6g: the six `return 1` leave the f1(b) loads at eax, ecx, edx, eax, ...)"""
    r = _alloc_reg(seq, i)
    if r == 'eax' and re.match(r'(mov eax, (-?0x[0-9a-f]+|\d+)$|xor eax, eax$)', seq[i]):
        j = i + 1
        while j < len(seq) and j < i + 8 and EPI.match(seq[j]):
            if seq[j].startswith(('ret', 'jmp')):
                return None
            j += 1
    return r

UNCOND = re.compile(r'(jmp|ret)\b')


def side(rows, k):
    xs = [r[k] for r in rows if r[k]]
    xs.sort(key=lambda x: x[0])
    return xs


def phase_walk(I, O, lmap):
    """-> list of (img_off, our_off, offset, line, img_ins, our_ins) for every aligned pair of loads, plus
    the alignment opcodes over abstracted text"""
    ops = fe.align([abstract(x[1]) for x in I], [abstract(x[1]) for x in O])
    TI, TO = [x[1] for x in I], [x[1] for x in O]
    pairs, solo = [], []
    for t, a1, a2, b1, b2 in ops:
        if t == 'equal':
            for k in range(a2 - a1):
                ri, ro = alloc_reg(TI, a1 + k), alloc_reg(TO, b1 + k)
                if ri and ro:
                    pairs.append((I[a1 + k][0], O[b1 + k][0], (SCR.index(ro) - SCR.index(ri)) % 3,
                                  lmap.get(O[b1 + k][0], '?'), I[a1 + k][1], O[b1 + k][1]))
                elif ri or ro:
                    solo.append(('img' if ri else 'ours', I[a1 + k][0], O[b1 + k][0], I[a1 + k][1], O[b1 + k][1]))
        else:
            for k in range(a1, a2):
                if alloc_reg(TI, k):
                    # nearest our offset for a line: the next aligned our row
                    solo.append(('img', I[k][0], O[b1][0] if b1 < len(O) else None, I[k][1], None))
            for k in range(b1, b2):
                if alloc_reg(TO, k):
                    solo.append(('ours', I[a1][0] if a1 < len(I) else None, O[k][0], None, O[k][1]))
    pairs.sort(key=lambda p: p[1])
    return pairs, solo, ops


def divergence(pairs, persist):
    """index of the first pair that starts an off stretch, or None"""
    for i, p in enumerate(pairs):
        if p[2] == 0:
            continue
        nxt = pairs[i:i + persist]
        off = sum(1 for q in nxt if q[2] == p[2])
        if off * 2 > len(nxt) or (len(nxt) == 1):
            return i
    return None


def loads_between(seq, lo_off, hi_off):
    T = [y[1] for y in seq]
    n = 0
    for i, x in enumerate(seq):
        if lo_off < x[0] < hi_off and alloc_reg(T, i):
            n += 1
    return n


def merge_phases(rows, I, O, lmap, ops):
    """unconditional jumps on one side only (or another target) -> list of dicts"""
    # aligned maps img_off <-> our_off over the abstracted alignment
    i2o, o2i = {}, {}
    for t, a1, a2, b1, b2 in ops:
        if t == 'equal':
            for k in range(a2 - a1):
                i2o[I[a1 + k][0]] = O[b1 + k][0]
                o2i[O[b1 + k][0]] = I[a1 + k][0]
    iidx = {x[0]: n for n, x in enumerate(I)}
    oidx = {x[0]: n for n, x in enumerate(O)}
    out = []

    def rot_match(seq, a, b, maxlen=24, reach=40):
        """find the copy of the host block seq[b:] (up to its unconditional end) nearest seq[a] (within
        +-reach instructions, excluding the host itself), matching with scratch registers abstracted.
        -> (k, n, copy_text, host_text, copy_index): n instructions match abstracted; k = the rotation under
        which most of them are equal (None when nothing matches)"""
        H = []
        for x in seq[b:b + maxlen]:
            H.append(x[1])
            if UNCOND.match(x[1]):
                break
        AH = [abstract(x) for x in H]
        best = (0, 0, None)
        for c in range(max(0, a - reach), min(len(seq), a + reach)):
            if c == b:
                continue
            n = 0
            while n < len(H) and c + n < len(seq) and c + n != b and abstract(seq[c + n][1]) == AH[n]:
                n += 1
            if n > best[0] or (n == best[0] and n and abs(c - a) < abs(best[2] - a)):
                best = (n, -abs(c - a), c)
        n, _, c = best
        if n < min(3, len(H)):
            return (None, 0, seq[a][1] if a < len(seq) else '', H[0] if H else '', None)
        score = []
        for k in (0, 1, 2):
            score.append(sum(1 for j in range(n) if (rename(seq[c + j][1], ROT[k]) if k else seq[c + j][1]) == H[j]))
        k = max(range(3), key=lambda q: score[q])
        return (k, n, '', '', c)

    def host_of(tgt_off, mp, dst_idx, dst):
        """the other side's instruction index at a target offset (next aligned row at or after it)"""
        if tgt_off in mp:
            return dst_idx.get(mp[tgt_off])
        later = [k for k in mp if k >= tgt_off]
        return dst_idx.get(mp[min(later)]) if later else None

    for r in rows:
        tag, oi, oo = r
        ji = oi is not None and oi[1].startswith('jmp L')
        jo = oo is not None and oo[1].startswith('jmp L')
        if not (ji or jo) or (tag == '  '):
            continue
        if tag == '!>' and ji and jo:
            out.append(dict(kind='target', img=oi[0], ours=oo[0], line=lmap.get(oo[0], '?'),
                            img_tgt=oi[3], our_tgt=oo[3], our_tgt_line=lmap.get(oo[3], '?'),
                            img_tgt_in_ours=i2o.get(oi[3])))
            continue
        if ji and not jo:
            # image merges here; ours keeps a copy at our position after this row
            pos = oo[0] if oo else None
            if pos is None:
                # first our row after this one
                k = rows.index(r)
                pos = next((x[2][0] for x in rows[k:] if x[2]), None)
            host = host_of(oi[3], i2o, oidx, O)
            d = dict(kind='img-merge', img=oi[0], ours=pos, line=lmap.get(pos, '?'), tgt=oi[3])
            if pos is not None and host is not None:
                k_, n, cx, hx, ci = rot_match(O, oidx[pos], host)
                pos = O[ci][0] if ci is not None else pos
                d.update(ours=pos, line=lmap.get(pos, '?'), host=O[host][0], host_line=lmap.get(O[host][0], '?'), rot=k_, n=n, cx=cx, hx=hx,
                         loads=loads_between(O, min(O[host][0], pos), max(O[host][0], pos)))
            out.append(d)
        elif jo and not ji:
            pos = oi[0] if oi else None
            if pos is None:
                k = rows.index(r)
                pos = next((x[1][0] for x in rows[k:] if x[1]), None)
            host = host_of(oo[3], o2i, iidx, I)
            d = dict(kind='ours-merge', img=pos, ours=oo[0], line=lmap.get(oo[0], '?'), tgt=oo[3],
                     tgt_line=lmap.get(oo[3], '?'))
            if pos is not None and host is not None:
                k_, n, cx, hx, ci = rot_match(I, iidx[pos], host)
                pos = I[ci][0] if ci is not None else pos
                d.update(img=pos, host=I[host][0], rot=k_, n=n, cx=cx, hx=hx,
                         loads=loads_between(I, min(I[host][0], pos), max(I[host][0], pos)))
            out.append(d)
    return out


CS = ('ebx', 'esi', 'edi', 'ebp')
CSX = re.compile(r'\b(ebx|esi|edi|ebp|bx|si|di|bl|bh)\b')
FRAME = re.compile(r'(?:byte|word|dword|qword) ptr \[(?:ebp - 0x[0-9a-f]+|ebp \+ 0x[0-9a-f]+|esp \+ 0x[0-9a-f]+|esp)\]')
CS8 = {'bx': 'ebx', 'bl': 'ebx', 'bh': 'ebx', 'si': 'esi', 'di': 'edi'}


def operands(ins):
    m = re.match(r'(\S+)\s*(.*)', ins)
    ops, d, cur = [], 0, ''
    for ch in (m.group(2) if m else ''):
        if ch == '[':
            d += 1
        elif ch == ']':
            d -= 1
        if ch == ',' and d == 0:
            ops.append(cur.strip())
            cur = ''
        else:
            cur += ch
    if cur.strip():
        ops.append(cur.strip())
    return (m.group(1) if m else ''), ops


def vabs(op, ebp_frame):
    """an operand that is a whole callee-saved register or a frame slot -> 'V'; scratch registers -> R;
    callee-saved registers inside an address -> C"""
    if op in CS and not (op == 'ebp' and ebp_frame) or FRAME.fullmatch(op):
        return 'V'
    if op in CS8:
        return 'V8'
    return CSX.sub('C', abstract(op))


def vkey(op, ebp_frame):
    if FRAME.fullmatch(op):
        return 'slot'
    return CS8.get(op, op)


def csave_walk(I, O, lmap, lo, maxreg=12):
    """first callee-saved occupant divergence: align with callee-saved registers and frame slots abstracted
    to one token, pair the whole-operand occupants, report where image and ours put a value in different
    callee-saved registers, or one side in a register and the other in a frame slot (a spill)."""
    ebp_i = any(x[1] == 'mov ebp, esp' for x in I[:4])
    ebp_o = any(x[1] == 'mov ebp, esp' for x in O[:4])

    def ab(x, f):
        mn, ops = operands(x)
        return mn + ' ' + ','.join(vabs(o, f) for o in ops)
    ops = fe.align([ab(x[1], ebp_i) for x in I], [ab(x[1], ebp_o) for x in O])
    pairs = []
    for t, a1, a2, b1, b2 in ops:
        if t != 'equal':
            continue
        for k in range(a2 - a1):
            ii, oo = I[a1 + k], O[b1 + k]
            _, oi = operands(ii[1])
            _, ou = operands(oo[1])
            for n, (x, y) in enumerate(zip(oi, ou)):
                if vabs(x, ebp_i).startswith('V'):
                    kx, ky = vkey(x, ebp_i), vkey(y, ebp_o)
                    pairs.append((ii[0], oo[0], kx, ky, n == 0 and not ii[1].startswith(('cmp', 'test', 'push')), ii[1], oo[1]))
    pairs.sort(key=lambda p: p[1])
    bad = [p for p in pairs if p[2] != p[3]]
    res = dict(npairs=len(pairs), nbad=len(bad))
    if not bad:
        return res
    f = bad[0]

    def defsite(seq, off, reg):
        """nearest earlier instruction of seq (address order) that writes reg"""
        if reg == 'slot':
            return None
        best = None
        for x in seq:
            if x[0] >= off:
                break
            mn, o = operands(x[1])
            if o and CS8.get(o[0], o[0]) == reg and mn not in ('cmp', 'test', 'push'):
                best = x
            elif mn == 'pop' and o and o[0] == reg:
                best = x
        return best
    di, do = defsite(I, f[0], f[2]), defsite(O, f[1], f[3])
    res['first'] = dict(img=lo + f[0], ours=f[1], line=lmap.get(f[1], '?'), img_occ=f[2], our_occ=f[3],
                        img_ins=f[5], our_ins=f[6],
                        img_def=('%x `%s`' % (lo + di[0], di[1])) if di else '-',
                        our_def=('%x %s `%s`' % (do[0], lmap.get(do[0], '?'), do[1])) if do else '-')
    # regions: runs of one (image occupant -> our occupant) disagreement
    regs, last = [], None
    for p in bad:
        k = (p[2], p[3])
        if k != last:
            regs.append([lo + p[0], p[1], lmap.get(p[1], '?'), p[2], p[3], 1])
            last = k
        else:
            regs[-1][5] += 1
    import collections
    res['map'] = collections.Counter('%s>%s' % (p[2], p[3]) for p in bad).most_common(5)
    res['regions'] = regs[:maxreg]
    res['nregions'] = len(regs)
    return res


def scan(unit, addr, text, c, cod, persist=5):
    res = fe.evaluate(c, text, addr)
    if 'error' in res:
        return dict(addr=addr, error=res['error'])
    lo = res['lo']
    lmap = tailscan.lines_of(cod, res['sym'], os.path.basename(unit).lower()) if cod else {}
    rows = res['rows']
    I, O = side(rows, 1), side(rows, 2)
    pairs, solo, ops = phase_walk(I, O, lmap)
    d = divergence(pairs, persist)
    out = dict(addr=addr, unit=unit, sym=res['sym'], diff=res['diff'], exact=res['exact'], npairs=len(pairs),
               off_pairs=sum(1 for p in pairs if p[2]), pairs=pairs, lo=lo)
    if d is not None:
        p = pairs[d]
        prev = next((q for q in reversed(pairs[:d]) if q[2] == 0), None)
        w_lo_o = prev[1] if prev else -1
        w_lo_i = prev[0] if prev else -1
        win = [s for s in solo if (s[2] is not None and w_lo_o < s[2] < p[1]) or
               (s[2] is None and s[1] is not None and w_lo_i < s[1] < p[0])]
        out.update(div=dict(img=lo + p[0], ours=p[1], line=p[3], off=p[2], img_ins=p[4], our_ins=p[5],
                            sync_img=(lo + prev[0]) if prev else None, sync_line=prev[3] if prev else None,
                            n_img=loads_between(I, w_lo_i, p[0]), n_ours=loads_between(O, w_lo_o, p[1]),
                            solo=[(s[0], (lo + s[1]) if s[1] is not None else None, s[2],
                                   lmap.get(s[2], '?') if s[2] is not None else '?', s[3] or s[4]) for s in win]))
    # the first load the two sides disagree on at all: a load on one side only (a reuse such as
    # `mov eax, [eax+8]` against a new register) or a pair in another register
    ev = [(s[2], 'only ' + s[0], s[1], s[2], '%s | %s' % (s[3] or '-', s[4] or '-')) for s in solo if s[2] is not None]
    ev += [(p[1], 'pair %+d' % (1 if p[2] == 1 else -1), p[0], p[1], '%s | %s' % (p[4], p[5])) for p in pairs if p[2]]
    if ev:
        e = min(ev)
        out['first'] = dict(kind=e[1], img=(lo + e[2]) if e[2] is not None else None, ours=e[3],
                            line=lmap.get(e[3], '?'), ins=e[4])
    out['merges'] = merge_phases(rows, I, O, lmap, ops)
    out['cs'] = csave_walk(I, O, lmap, lo)
    return out


def report(r, track=False):
    L = []
    if 'error' in r:
        return ['RS %08X error %s' % (r['addr'], r['error'])]
    head = 'RS %08X %-34s diff=%-4d loads=%d off=%d' % (r['addr'], r['sym'][:34], r['diff'], r['npairs'], r['off_pairs'])
    if 'div' in r:
        d = r['div']
        L.append(head + '  DIVERGE %x %s %+d (ours %s); window img %d / ours %d loads since %s %s' % (
            d['img'], d['line'], d['off'] if d['off'] == 1 else -1, 'ahead' if d['off'] == 1 else 'behind',
            d['n_img'], d['n_ours'], ('%x' % d['sync_img']) if d['sync_img'] else 'entry', d['sync_line'] or ''))
        L.append('    first off pair: image `%s`  ours `%s`' % (d['img_ins'], d['our_ins']))
        f = r.get('first')
        if f and f['ours'] < d['ours']:
            L.append('    first disagreement earlier: %s img %s ours %x %s `%s`' % (
                f['kind'], ('%x' % f['img']) if f['img'] else '-', f['ours'], f['line'], f['ins']))
        for s in d['solo']:
            L.append('    only %-4s load  img %-7s ours %-5s %-8s `%s`' % (
                s[0], ('%x' % s[1]) if s[1] else '-', ('%x' % s[2]) if s[2] is not None else '-', s[3], s[4]))
    elif r['npairs']:
        L.append(head + '  EQUAL')
    else:
        L.append(head + '  NOPAIRS')
    cs = r.get('cs') or {}
    if cs.get('first'):
        f = cs['first']
        L.append('    callee-saved: %d of %d occupant pairs differ; first %x %s image %s ours %s  (image `%s` | ours `%s`)' % (
            cs['nbad'], cs['npairs'], f['img'], f['line'], f['img_occ'], f['our_occ'], f['img_ins'], f['our_ins']))
        L.append('      image %s set by %s; ours %s set by %s' % (f['img_occ'], f['img_def'], f['our_occ'], f['our_def']))
        L.append('      occupant map (image>ours): %s' % ', '.join('%s x%d' % kv for kv in cs['map']))
        for g in cs['regions'][1:6]:
            L.append('      then %x %-8s image %s ours %s (%d)' % (g[0], g[2], g[3], g[4], g[5]))
    for m in r.get('merges', []):
        if m['kind'] == 'target':
            L.append('    jmp-target  img %x->%x  ours %x(%s)->%x(%s) image target is ours %s' % (
                r['lo'] + m['img'], r['lo'] + m['img_tgt'], m['ours'], m['line'], m['our_tgt'], m['our_tgt_line'],
                ('%x' % m['img_tgt_in_ours']) if m['img_tgt_in_ours'] is not None else '(unaligned)'))
        elif m['kind'] == 'img-merge':
            L.append('    img-merge   img %x->%x; ours copy at %s(%s) vs host %s(%s): %s' % (
                r['lo'] + m['img'], r['lo'] + m['tgt'], ('%x' % m['ours']) if m['ours'] is not None else '?', m['line'],
                ('%x' % m['host']) if 'host' in m else '?', m.get('host_line', '?'),
                ('rot+%d for %d ins, %d loads apart' % (m['rot'], m['n'], m['loads'])) if m.get('rot') is not None else 'no copy of the host block on this side: site `%s` host `%s`' % (m.get('cx'), m.get('hx'))))
        else:
            L.append('    ours-merge  ours %x(%s)->%x(%s); image copy at %s vs host %s: %s' % (
                m['ours'], m['line'], m['tgt'], m['tgt_line'], ('%x' % (r['lo'] + m['img'])) if m['img'] is not None else '?',
                ('%x' % (r['lo'] + m['host'])) if 'host' in m else '?',
                ('rot+%d for %d ins, %d loads apart' % (m['rot'], m['n'], m['loads'])) if m.get('rot') is not None else 'no copy of the host block on this side: site `%s` host `%s`' % (m.get('cx'), m.get('hx'))))
    if track:
        last = None
        for p in r['pairs']:
            if (p[3], p[2]) != last:
                L.append('      %x %-9s %+d   image `%s`  ours `%s`' % (r['lo'] + p[0], p[3], p[2], p[4], p[5]))
                last = (p[3], p[2])
    return L


def own_context(text, addr, unit):
    """False for a marker measured in another unit's compile (`// compile_context <tu>`)"""
    m = re.search(r'// ORIGINAL: 0x%08X [^\n]*\n((?://(?! ORIGINAL)[^\n]*\n)*)' % addr, text)
    mc = m and re.search(r'// compile_context (\S+)', m.group(1))
    return not mc or mc.group(1) == unit


def run_unit(unit, addrs, src, pfx, persist):
    text = open(src, encoding='latin-1').read()
    c, cod = tailscan.compile_listing(unit, src, pfx)
    if c is None:
        return [dict(addr=a, error='COMPILE FAIL') for a in addrs]
    out = []
    for a in addrs:
        try:
            out.append(scan(unit, a, text, c, cod, persist))
        except Exception as e:  # one bad marker must not stop a tree pass
            out.append(dict(addr=a, error=repr(e)))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('unit', nargs='?')
    ap.add_argument('addr', nargs='?')
    ap.add_argument('--src')
    ap.add_argument('--pfx', default=os.environ.get('FE_PFX_BASE', '8'))
    ap.add_argument('--persist', type=int, default=5)
    ap.add_argument('--track', action='store_true')
    ap.add_argument('--tree', nargs='*')
    ap.add_argument('--from', dest='frm', help='feall.py --out JSON: its non-exact functions')
    ap.add_argument('--all', action='store_true', help='tree/unit: include BYTE_EXACT markers (validation)')
    ap.add_argument('--json')
    a = ap.parse_args()
    if a.tree is not None or a.frm:
        work = {}
        if a.frm:
            for r in json.load(open(a.frm)):
                if not r.get('exact'):
                    work.setdefault(r['unit'] if r['unit'].startswith('src/') else 'src/' + r['unit'], []).append(int(str(r['addr']), 16))
        else:
            cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
            units = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/')]
            if a.tree:
                units = [u for u in units if u in a.tree or os.path.basename(u) in a.tree or u[4:] in a.tree]
            for u in units:
                try:
                    t = open(fe.R + '/' + u, encoding='latin-1').read()
                except OSError:
                    continue
                ad = [x for x in tailscan.markers(t, a.all) if own_context(t, x, u)]
                if ad:
                    work[u] = ad
        work = sorted(work.items())
        jobs = int(os.environ.get('FE_JOBS', '5'))
        fe.compile_cmd(work[0][0])
        base = int(a.pfx)

        def lane(k):
            r = []
            for i in range(k, len(work), jobs):
                u, ad = work[i]
                r += run_unit(u, ad, fe.R + '/' + u, str(base + k), a.persist)
            return r
        allr = []
        with ThreadPoolExecutor(jobs) as ex:
            for o in ex.map(lane, range(jobs)):
                allr += o
        allr = [x for x in allr if 'error' in x or not x['exact']] if not a.all else allr
        allr.sort(key=lambda x: ('div' not in x, -x.get('diff', 0)))
        for x in allr:
            for l in report(x, a.track):
                print(l)
        n = lambda f: sum(1 for x in allr if 'error' not in x and f(x))
        print('callee-saved divergent %d of %d' % (n(lambda x: (x.get('cs') or {}).get('first')), len(allr)))
        print('functions %d: phase-divergent %d, phase-equal %d, no paired loads %d, errors %d; with a one-side merge %d' % (
            len(allr), n(lambda x: 'div' in x), n(lambda x: 'div' not in x and x['npairs']),
            n(lambda x: not x['npairs'] and 'div' not in x), sum(1 for x in allr if 'error' in x), n(lambda x: x['merges'])))
        if a.json:
            for x in allr:
                x.pop('pairs', None)
            json.dump(allr, open(a.json, 'w'), indent=0)
        return
    unit = a.unit if a.unit.startswith('src/') else 'src/' + a.unit
    src = os.path.abspath(a.src) if a.src else fe.R + '/' + unit
    addrs = [int(x, 16) for x in a.addr.split(',')]
    for r in run_unit(unit, addrs, src, a.pfx, a.persist):
        print('\n'.join(report(r, a.track)))


if __name__ == '__main__':
    main()
