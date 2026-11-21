#!/usr/bin/env python3
"""lastblock.py <unit> <0xADDR[,..]>|--all [--src ALT.cpp] [--min K] [--slack N]
   lastblock.py --tree [unit ...] [--skip 0xA,..] [--json OUT] [--min K]
"Block laid after all exits" screen (MATCHING_GUIDE 3.7, "An `||` goto site moves its label block last",
"Host orientation", par-4 2026-11-20).

VC6 lays a return-ending block reached by an `||` site (`if (A || B) goto L;`, `if (A || B) { ...; return; }`)
after every other exit and makes it the cross-jump host; separate `if (A) goto L; if (B) goto L;` sites lay
it at its flow position.  Bare `if (A || B) return;` statements toggle which copy of a shared tail hosts
(join_session: the opening `||` guard written as two returns plus `if (!f) close(); else return id;`).

Per function, the fe alignment's unaligned runs are paired across the sides (same masked instruction text,
K or more rows): a MOVED block.  Its tail distance on each side is the number of instructions after it that
are not exit code (teardown calls, epilogue pops, `ret`, `mov eax,K`, `jmp` and other moved blocks); a block
with distance <= --slack is laid LAST.  Listed:
  IMG-LAST   the image lays the block last, ours mid-function (the original reached it by an `||` site or an
             if/else whose other arm was laid first: merge the gotos/ifs that reach it into one `||`, or
             invert the if/else)
  OURS-LAST  ours lays it last, the image mid-function (split an `||` site of ours; or a toggle elsewhere)
For each, the block's source lines (ours /FAsc) and the source sites that reach it: goto sites of a label
on the block's first line (`gotos=N`, N>=2 = merge candidate), else the enclosing `if` and its condition
kind (`||`, `&&`, `c`), the previous statement when it returns/gotos the same way (separate sites that an
`||` would merge), and the function's bare `||` return count (the orientation toggles, `tog=N`).
Screen only; one /FAsc compile per unit.  Confirm by hand with fe.py --all.
Run from the worktree root: FE_PFX_BASE=21 FE_JOBS=5 uv run python recovery/tools/analysis/lastblock.py --tree"""
import sys, os, re, json, argparse
from concurrent.futures import ThreadPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE)); sys.path.insert(0, HERE)
import fe
from tailscan import compile_listing, lines_of, markers
from regprio import strip_code
from labelscan import if_cond, if_kind

EXIT = re.compile(r'^(ret|pop |leave|mov esp, ebp|jmp |mov eax, (0x[0-9a-f]+|\d+)$|xor eax, eax|or eax, -1|'
                  r'mov dword ptr fs:\[0\]|add esp, |mov ecx, dword ptr \[(ebp|esp)|lea ecx, |'
                  r'mov (byte|dword) ptr \[(ebp|esp) [-+] [^\]]*\], (bl|0xffffffff|-1|\w+)$|call <ext>|nop)')


def runs(rows, side):
    """maximal runs of rows present only on one side -> [(first row index, last row index, [texts])]"""
    out, cur = [], None
    for i, (k, o, c) in enumerate(rows):
        x = o if side == 0 else c
        y = c if side == 0 else o
        if k == '!!' and x is not None and y is None:
            if cur is None:
                cur = [i, i, []]
            cur[1] = i
            cur[2].append(x[1])
        else:
            if cur and k == '!!' and x is None:
                continue  # an other-side-only row inside our run does not break it
            if cur:
                out.append(tuple(cur))
            cur = None
    if cur:
        out.append(tuple(cur))
    return out


def seq(rows, side):
    return [r[1 + side] for r in rows if r[1 + side] is not None]


def tail_distance(ins, end_off, moved_offs):
    """instructions after end_off that are not exit code nor part of another moved block"""
    n = 0
    for off, t, _, _ in ins:
        if off <= end_off or off in moved_offs:
            continue
        if not EXIT.match(t):
            n += 1
    return n


def match(a, b):
    if a == b:
        return 1.0
    import difflib
    return difflib.SequenceMatcher(None, a, b, autojunk=False).ratio()


LAB = re.compile(r'^\s*([A-Za-z_]\w*)\s*:(?!:)')
RETGOTO = re.compile(r'\b(return\b[^;]*;|goto\s+\w+\s*;)')


def stmt_head(code, n, lo):
    """the `if (` statement that opens the block containing line n (index into code) -> (index, text)"""
    depth = 0
    for j in range(n, lo - 1, -1):
        s = code[j]
        depth += s.count('}') - s.count('{') if j != n else 0
        if depth < 0 or (j < n and depth <= 0 and re.search(r'\bif\s*\(', s)):
            k = j
            txt = s
            while k > lo and not re.search(r'\b(if|while|for)\s*\(|\belse\b', code[k]):
                k -= 1
                txt = code[k] + ' ' + txt
            m = re.search(r'\bif\s*\(', txt)
            if m:
                return k, txt[m.start():]
            if re.search(r'\belse\b', txt):
                return k, 'else'
            return k, txt.strip()
    return None, None


def bare_or_returns(code, lo, hi):
    n = 0
    for j in range(lo, hi + 1):
        s = code[j]
        if re.search(r'\bif\s*\(', s) and not re.search(r'\belse\b', s):
            h = s
            k = j
            while h.count('(') > h.count(')') and k < hi:
                k += 1
                h += ' ' + code[k]
            c = if_cond(h[re.search(r'\bif\s*\(', h).start():])
            rest = h + ' ' + (code[k + 1] if k + 1 <= hi else '')
            if if_kind(c) == '||' and re.search(r'\)\s*\{?\s*return\b[^;]*;', rest):
                n += 1
    return n


def describe(code, lines, lo, hi):
    """source context of a block whose ours lines are `lines` (1-based unit lines)"""
    if not lines:
        return 'no source lines'
    first = min(lines) - 1
    out = []
    # label on/just before the first line?
    lab = None
    for j in (first, first - 1):
        if lo <= j <= hi:
            m = LAB.match(code[j])
            if m and m.group(1) not in ('case', 'default', 'public', 'private', 'protected'):
                lab = m.group(1)
    if lab:
        sites = [j + 1 for j in range(lo, hi + 1) if re.search(r'\bgoto\s+%s\s*;' % lab, code[j])]
        ors = 0
        for s in sites:
            k, h = stmt_head(code, s - 1, lo) if not re.search(r'\bif\s*\(', code[s - 1]) else (s - 1, code[s - 1])
            if h and h.startswith('if') and if_kind(if_cond(h)) == '||':
                ors += 1
        out.append('label %s gotos=%d (%s) or-sites=%d%s' % (lab, len(sites), ','.join('L%d' % s for s in sites),
                                                              ors, '  MERGE?' if len(sites) >= 2 and not ors else ''))
    else:
        k, h = stmt_head(code, first, lo)
        if h:
            kind = if_kind(if_cond(h)) if h.startswith('if') else h[:20]
            out.append('arm of L%d %s [%s]' % (k + 1, kind, re.sub(r'\s+', ' ', h)[:70]))
            if h.startswith('if') and kind != '||':
                # previous sibling statement: a separate test leaving the same way?
                p = k - 1
                while p >= lo and not code[p].strip():
                    p -= 1
                if p >= lo:
                    prev = ' '.join(code[max(lo, p - 3):p + 1])
                    blk = ' '.join(code[first:first + 3])
                    rp, rb = RETGOTO.findall(prev), RETGOTO.findall(blk)
                    if rp and rb and rp[-1] == rb[0]:
                        out.append('prev site L%d leaves the same way (%s): one || site?' % (p + 1, rp[-1]))
    return '; '.join(out)


def scan(unit, addr, text, c, cod, kmin, slack):
    r = fe.evaluate(c, text, addr)
    if 'error' in r:
        return None
    if r['exact']:
        return None
    rows = r['rows']
    lmap = lines_of(cod, r['sym'], os.path.basename(unit).lower())
    A, B = runs(rows, 0), runs(rows, 1)
    I = seq(rows, 0)
    C = seq(rows, 1)
    Ioffs = {x[0] for (k, o, c_) in rows for x in [o] if x and k == '!!' and c_ is None}
    Coffs = {x[0] for (k, o, c_) in rows for x in [c_] if x and k == '!!' and o is None}
    code = [strip_code(l) for l in text.split('\n')]
    flines = [int(v[1:]) for v in lmap.values() if v.startswith('L')]
    if not flines:
        return None
    lo, hi = min(flines) - 1, max(flines) - 1
    while lo > 0 and not re.search(r'\)\s*\{?\s*$', code[lo - 1] if lo else '') and lo > min(flines) - 40:
        lo -= 1
    tog = bare_or_returns(code, min(flines) - 1, hi)
    hits = []
    used = set()
    for a in A:
        if len(a[2]) < kmin:
            continue
        best = None
        for j, b in enumerate(B):
            if j in used or len(b[2]) < kmin:
                continue
            m = match(a[2], b[2])
            if m >= 0.8 and (best is None or m > best[0]):
                best = (m, j, b)
        if not best:
            continue
        used.add(best[1])
        b = best[2]
        oi_end = rows[a[1]][1][0]
        oc_end = rows[b[1]][2][0]
        di = tail_distance(I, oi_end, Ioffs - {rows[i][1][0] for i in range(a[0], a[1] + 1) if rows[i][1]})
        dc = tail_distance(C, oc_end, Coffs - {rows[i][2][0] for i in range(b[0], b[1] + 1) if rows[i][2]})
        if (di <= slack) == (dc <= slack):
            continue
        kind = 'IMG-LAST' if di <= slack else 'OURS-LAST'
        ln = sorted({int(lmap[rows[i][2][0]][1:]) for i in range(b[0], b[1] + 1)
                     if rows[i][2] and rows[i][2][0] in lmap and lmap[rows[i][2][0]].startswith('L')})
        desc = describe(code, ln, min(flines) - 1, hi)
        hits.append(dict(kind=kind, n=len(a[2]), img=r['lo'] + rows[a[0]][1][0], ours=rows[b[0]][2][0],
                         dist_img=di, dist_ours=dc, lines=ln[:6], src=desc))
    if not hits:
        return None
    return dict(unit=unit, addr='%08X' % addr, sym=r['sym'], diff=r['diff'], tog=tog, hits=hits)


def fmt(x):
    out = ['%s %-26s %-34s diff=%-4d tog=%d' % (x['addr'], x['unit'][4:], x['sym'][:34], x['diff'], x['tog'])]
    for h in x['hits']:
        out.append('  %-9s n=%-3d img@%x ours+%x dist img/ours=%d/%d lines %s' % (
            h['kind'], h['n'], h['img'], h['ours'], h['dist_img'], h['dist_ours'],
            ','.join('L%d' % l for l in h['lines'])))
        out.append('            ' + h['src'])
    return '\n'.join(out)


def run_unit(unit, addrs, src, pfx, kmin, slack):
    text = open(src, encoding='latin-1').read()
    c, cod = compile_listing(unit, src, pfx)
    if c is None:
        return []
    res = []
    for a in addrs:
        try:
            x = scan(unit, a, text, c, cod, kmin, slack)
        except Exception as e:  # one bad marker must not stop a tree pass
            x = dict(unit=unit, addr='%08X' % a, sym='error %s' % e, diff=0, tog=0, hits=[])
        if x:
            res.append(x)
    return res


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('unit', nargs='?')
    ap.add_argument('addr', nargs='?')
    ap.add_argument('--all', action='store_true')
    ap.add_argument('--tree', nargs='*')
    ap.add_argument('--src')
    ap.add_argument('--skip', default='')
    ap.add_argument('--pfx', default=os.environ.get('FE_PFX_BASE', '21'))
    ap.add_argument('--min', type=int, default=4)
    ap.add_argument('--slack', type=int, default=3)
    ap.add_argument('--json')
    a = ap.parse_args()
    if a.tree is not None:
        skip = {int(x, 16) for x in a.skip.split(',') if x}
        cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
        units = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/')]
        if a.tree:
            units = [u for u in units if u in a.tree or os.path.basename(u) in a.tree]
        work = []
        for u in units:
            try:
                t = open(fe.R + '/' + u, encoding='latin-1').read()
            except OSError:
                continue
            ctx = set(int(m.group(1), 16) for m in re.finditer(
                r'// ORIGINAL: 0x([0-9A-F]{8})[^\n]*\n(?:// [^\n]*\n){0,12}?// compile_context', t))
            ad = [x for x in markers(t) if x not in ctx and x not in skip]
            if ad:
                work.append((u, ad))
        jobs = int(os.environ.get('FE_JOBS', '5'))
        fe.compile_cmd(units[0])
        base = int(a.pfx)

        def lane(k):
            r = []
            for i in range(k, len(work), jobs):
                u, ad = work[i]
                r += run_unit(u, ad, fe.R + '/' + u, str(base + k), a.min, a.slack)
            return r
        allr = []
        with ThreadPoolExecutor(jobs) as ex:
            for o in ex.map(lane, range(jobs)):
                allr += o
        allr.sort(key=lambda x: (-sum(h['kind'] == 'IMG-LAST' for h in x['hits']), x['unit'], x['addr']))
        for x in allr:
            print(fmt(x))
        print('functions with a moved last block:', len(allr), ' IMG-LAST hits:',
              sum(h['kind'] == 'IMG-LAST' for x in allr for h in x['hits']))
        if a.json:
            json.dump(allr, open(a.json, 'w'), indent=0)
        return
    unit = a.unit if a.unit.startswith('src/') else 'src/' + a.unit
    src = os.path.abspath(a.src) if a.src else fe.R + '/' + unit
    text = open(src, encoding='latin-1').read()
    addrs = markers(text) if a.all else [int(x, 16) for x in a.addr.split(',')]
    for x in run_unit(unit, addrs, src, a.pfx, a.min, a.slack):
        print(fmt(x))


if __name__ == '__main__':
    main()
