#!/usr/bin/env python3
"""prosave.py [unit ...] [--addr A[,A]] [--src ALT.cpp] [--census] [--class abcd] [--json OUT] [--jobs N] [--pfx-base N]
Prologue-save / epilogue-shape screen (MATCHING_GUIDE 3.7 "Prologue saves and return tails").

For every non-exact function it compares, image against our compile:
  up    callee-saved registers pushed before the first branch (jcc/jmp/ret), in push order
  def   callee-saved registers saved later (shrink-wrapped past an early-out), in push order
  tails every `pop ..; ret` epilogue: the registers it pops and the number of ways into it
        (jumps that land in it or at most 3 instructions before it, plus the fallthrough)
and prints one line per function that differs, with its classes:
  a  the image saves up front what ours defers
  b  ours saves up front what the image defers
  c  the number of return tails differs (image duplicates the epilogue where ours jumps, or the reverse)
  d  same saved set, different push order
  s  the saved register set itself differs (allocation; listed, not a prologue lever)
`--census` reads the BYTE_EXACT functions instead (image bytes only, no compile) and tabulates the
prologue shape against the source's guard spelling:
  shape   U = every save up front, the first branch is an early exit and the entry block
          leaves at least one saved register untouched, D = some save deferred,
          - = no early exit before the saves are needed (not counted)
  guard   first control statement at the top level of the body:
          ER `if (c) return ..;`   EG `if (c) goto L;`   WRAP `if (c) { .. }` as the last statement (or
          followed only by `return x;`)   IF another if   LOOP/SW/other
  nret    `return` statements in the body, `in` = how many sit inside a nested block, goto count
A register push counts as a save when the register has not been written earlier in the listing and an
epilogue pops it.  Screen only; confirm with fe.py and `recover measure`."""
import sys, os, re, json, argparse, collections
from concurrent.futures import ThreadPoolExecutor
T = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, T)
import fe

CS = ('ebx', 'esi', 'edi', 'ebp')
SUB = {'bl': 'ebx', 'bh': 'ebx', 'bx': 'ebx', 'si': 'esi', 'di': 'edi', 'bp': 'ebp'}
WR = ('mov', 'lea', 'xor', 'add', 'sub', 'pop', 'movsx', 'movzx', 'imul', 'and', 'or', 'sar', 'shl', 'shr', 'neg',
      'not', 'inc', 'dec', 'sbb', 'adc', 'set', 'xchg', 'cmov')
TAILOK = re.compile(r'^(pop (ebx|esi|edi|ebp|ecx)|mov esp, ebp|add esp, \S+|leave|mov dword ptr fs:\[0\], ecx|'
                    r'mov ecx, dword ptr \[(ebp|esp)[^\]]*\])$')

RETVAL = re.compile(r'^(xor eax, eax|mov eax, \S+|or eax, 0xffffffff|mov al, \S+|sbb eax, eax|mov eax, dword ptr \[[^\]]*\])$')


def shape(L):
    """L = fe.dis rows (off, text, size, tgt) -> dict(up, deferred, tails, first, frame)"""
    n = len(L)
    frame = n > 1 and L[0][1] == 'push ebp' and L[1][1] == 'mov ebp, esp'
    # epilogues
    tails = []
    idx = {r[0]: i for i, r in enumerate(L)}
    for k, r in enumerate(L):
        if not r[1].startswith('ret'):
            continue
        j = k
        pops = []
        while j > 0:
            if TAILOK.match(L[j - 1][1]):
                j -= 1
            elif RETVAL.match(L[j - 1][1]) and j > 1 and re.match(r'pop (ebx|esi|edi|ebp)$', L[j - 2][1]):
                j -= 2  # the result set between the pops (`pop edi; pop esi; xor eax, eax; pop ebx; ret`)
                pops.append(L[j][1][4:])
                continue
            else:
                break
            m = re.match(r'pop (ebx|esi|edi|ebp)$', L[j][1])
            if m:
                pops.append(m.group(1))
        # a `mov ecx, [..]` / `pop ecx` that is not part of the epilogue only widens the window: harmless
        tails.append([j, k, tuple(reversed(pops))])
    popped = set(p for t in tails for p in t[2])
    if frame:
        popped.discard('ebp')
    # saves
    written = set()
    saves = []
    first = None
    for i, r in enumerate(L):
        mn, _, op = r[1].partition(' ')
        if first is None and (mn.startswith('j') or mn.startswith('ret')):
            first = i
        if mn == 'push' and op in popped and op not in written and op not in [s[0] for s in saves]:
            if not (frame and op == 'ebp'):
                saves.append((op, i))
            continue
        if mn.startswith(WR):
            d = op.split(',')[0].strip()
            d = SUB.get(d, d)
            if d in CS:
                written.add(d)
    if first is None:
        first = n
    up = tuple(s for s, i in saves if i < first)
    # registers the entry block really touches (anything but their own save)
    used = set()
    for i, r in enumerate(L[:first + 1]):
        if any(i == j for _, j in saves):
            continue
        for g in re.findall(r'\b(ebx|esi|edi|ebp|bl|bh|bx|si|di|bp)\b', r[1]):
            used.add(SUB.get(g, g))
    idle = tuple(s for s in up if s not in used)
    de = tuple(s for s, i in saves if i > first)
    # ways into each tail
    out = []
    for j, k, pops in tails:
        lo = max(0, j - 3)
        # do not reach back across an unconditional transfer
        for q in range(j - 1, lo - 1, -1):
            if L[q][1].startswith(('jmp', 'ret')):
                lo = q + 1
                break
        offs = {L[q][0] for q in range(lo, k + 1)}
        ways = sum(1 for r in L if r[3] is not None and r[3] in offs)
        if j > 0 and not L[j - 1][1].startswith(('jmp', 'ret')):
            ways += 1
        out.append((pops, ways, j))
    # early exit: the first branch (conditional) lands in a tail window, or its fallthrough returns within 4 rows
    early = False
    if first < n and L[first][1].startswith('j') and not L[first][1].startswith('jmp'):
        tg = L[first][3]
        ti = idx.get(tg)
        if ti is not None:
            for pops, ways, j in out:
                k = next(kk for jj, kk, pp in tails if jj == j)
                if max(0, j - 3) <= ti <= k:
                    early = True
        for q in range(first + 1, min(n, first + 6)):
            if L[q][1].startswith('ret'):
                early = True
                break
            if L[q][1].startswith(('j', 'call')):
                break
    return dict(up=up, de=de, tails=[(p, w) for p, w, j in out], first=first, frame=frame, early=early, idle=idle)


def fmt(s):
    return 'up=%s def=%s tails=%s' % (','.join(s['up']) or '-', ','.join(s['de']) or '-',
                                      ' '.join('%s*%d' % ('/'.join(x[1:] for x in p) or '0', w) for p, w in s['tails']) or '-')


def classes(si, so):
    c = ''
    ai, ao = set(si['up']) | set(si['de']), set(so['up']) | set(so['de'])
    if set(si['up']) - set(so['up']) and (set(si['up']) - set(so['up'])) & set(so['de']):
        c += 'a'
    if set(so['up']) - set(si['up']) and (set(so['up']) - set(si['up'])) & set(si['de']):
        c += 'b'
    if len(si['tails']) != len(so['tails']):
        c += 'c'
    if ai == ao and (si['up'] + si['de']) != (so['up'] + so['de']) and 'a' not in c and 'b' not in c:
        c += 'd'
    if ai != ao:
        c += 's'
    return c


# ---- source side -------------------------------------------------------------------------------------------

def strip(t):
    t = re.sub(r'//[^\n]*', '', t)
    t = re.sub(r'/\*.*?\*/', '', t, flags=re.S)
    t = re.sub(r'"(?:\\.|[^"\\])*"', '""', t)
    t = re.sub(r"'(?:\\.|[^'\\])*'", "'c'", t)
    return t


def body_of(text, addr):
    m = re.search(r'// ORIGINAL: 0x%08X [^\n]*\n' % addr, text)
    if not m:
        return None
    i = m.end()
    e = re.search(r'\n}[ \t]*(?:\n|$)', text[i:])
    if not e:
        return None
    t = text[i:i + e.start() + 2]
    nxt = t.find('// ORIGINAL: ')
    if nxt >= 0:
        return None
    t = strip(t)
    b = t.find('{')
    # a constructor's initialiser list ends at the first `{` at column-0 depth; good enough for a census
    return t[:b], t[b + 1:-1] if b >= 0 else None


def stmts(body):
    """top-level statements of a brace body"""
    out = []
    cur = ''
    pd = bd = 0
    i = 0
    while i < len(body):
        ch = body[i]
        cur += ch
        if ch == '(':
            pd += 1
        elif ch == ')':
            pd -= 1
        elif ch == '{':
            bd += 1
        elif ch == '}':
            bd -= 1
            if bd == 0 and pd == 0:
                rest = body[i + 1:].lstrip()
                if rest.startswith('else') or (cur.lstrip().startswith('do') and rest.startswith('while')):
                    i += 1
                    continue
                out.append(cur.strip())
                cur = ''
        elif ch == ';' and pd == 0 and bd == 0:
            s = cur.strip()
            # `if (c) x; else y;`
            rest = body[i + 1:].lstrip()
            if rest.startswith('else') and re.match(r'(if|else)\b', s):
                i += 1
                continue
            out.append(s)
            cur = ''
        elif ch == ':' and pd == 0 and bd == 0 and re.fullmatch(r'\s*[A-Za-z_]\w*:', cur) and body[i + 1:i + 2] != ':':
            out.append('LABEL ' + cur.strip())
            cur = ''
        i += 1
    if cur.strip():
        out.append(cur.strip())
    return out


def if_parts(s):
    """`if (cond) rest` -> rest"""
    i = s.find('(')
    d = 0
    for j in range(i, len(s)):
        if s[j] == '(':
            d += 1
        elif s[j] == ')':
            d -= 1
            if d == 0:
                return s[j + 1:].strip()
    return ''


def kind(s, last):
    if re.match(r'if\b', s):
        r = if_parts(s)
        r1 = r[1:].strip() if r.startswith('{') else r
        if re.match(r'return\b[^;]*;\s*}?$', r1):
            return 'ER'
        if re.match(r'goto\b[^;]*;\s*}?$', r1):
            return 'EG'
        if re.search(r'\belse\b', r) and re.search(r'}\s*else\b', r):
            return 'IFELSE'
        return 'WRAP' if last else 'IF'
    if re.match(r'(for|while|do)\b', s):
        return 'LOOP'
    if re.match(r'switch\b', s):
        return 'SW'
    if re.match(r'return\b', s):
        return 'RET'
    if s.startswith('LABEL'):
        return 'LABEL'
    return None


def source_features(text, addr):
    b = body_of(text, addr)
    if not b or b[1] is None:
        return None
    head, body = b
    st = stmts(body)
    ks = []
    for i, s in enumerate(st):
        rest = [x for x in st[i + 1:]]
        last = all(re.match(r'return\b', x) for x in rest)
        ks.append(kind(s, last))
    guard = next((k for k in ks if k), 'NONE')
    nret = len(re.findall(r'\breturn\b', body))
    top = sum(1 for k in ks if k == 'RET')
    er = sum(1 for k in ks if k == 'ER')
    return dict(guard=guard, nret=nret, inner=nret - top - er, top=top, er=er, ngoto=len(re.findall(r'\bgoto\b', body)),
                trail=bool(ks and ks[-1] == 'RET'), void=bool(re.search(r'\bvoid\b[^()]*\(', head)) and not re.search(r'return\s+[^;\s]', body),
                seq=[k for k in ks if k])


# ---- drivers -----------------------------------------------------------------------------------------------

RX = re.compile(r'// ORIGINAL: 0x([0-9A-F]{8}) \S+ 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)')


def units_of(sel):
    cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
    units = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/')]
    if sel:
        units = [u for u in units if u in sel or os.path.basename(u) in sel]
    return units


def scan_unit(unit, addrs, pfx, src=None):
    path = src or fe.R + '/' + unit
    text = open(path, encoding='latin-1').read()
    c, err = fe.compile_unit(unit, os.path.abspath(path), pfx)
    if c is None:
        return [dict(error=err, unit=unit)]
    out = []
    ctxc = {}
    for addr in addrs:
        idn = fe.ident(text, addr)
        if not idn:
            continue
        sym, lo, hi = idn
        cc_ = c
        m = re.search(r'// ORIGINAL: 0x%08X [^\n]*\n((?://(?! ORIGINAL)[^\n]*\n)*)' % addr, text)
        mc = m and re.search(r'// compile_context (\S+)', m.group(1))
        if mc and mc.group(1) != unit and not src:
            if mc.group(1) not in ctxc:
                ctxc[mc.group(1)] = fe.compile_unit(mc.group(1), fe.R + '/' + mc.group(1), pfx)[0]
            if ctxc[mc.group(1)] is not None:
                cc_ = ctxc[mc.group(1)]
        try:
            s = cc_.function(sym)
        except Exception:
            continue
        sec = cc_.sections[s.section - 1]
        I = fe.imgdis(lo, hi)
        O = fe.dis(sec.data, 0, sec.relocations, selfsec=s.section, syms=cc_.symbols)
        r = fe.evaluate(cc_, text, addr)
        if r.get('exact'):
            continue
        si, so = shape(I), shape(O)
        out.append(dict(unit=unit, addr='%08X' % addr, sym=sym, diff=r.get('diff', -1), cls=classes(si, so), img=si, ours=so,
                        src=source_features(text, addr)))
    return out


def census(units):
    rows = []
    for u in units:
        try:
            text = open(fe.R + '/' + u, encoding='latin-1').read()
        except OSError:
            continue
        for m in RX.finditer(text):
            if 'BYTE_EXACT' not in m.group(2):
                continue
            addr = int(m.group(1), 16)
            idn = fe.ident(text, addr)
            if not idn:
                continue
            try:
                s = shape(fe.imgdis(idn[1], idn[2]))
            except Exception:
                continue
            f = source_features(text, addr)
            if not f:
                continue
            nsave = len(s['up']) + len(s['de'])
            if nsave == 0:
                continue
            sh = 'D' if s['de'] else ('U' if s['early'] and s['idle'] else '-')
            rows.append(dict(unit=u, addr='%08X' % addr, sym=idn[0], shape=sh, nsave=nsave, nup=len(s['up']), ntails=len(s['tails']),
                             img=s, src=f))
    return rows


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='*')
    ap.add_argument('--addr', default='')
    ap.add_argument('--src')
    ap.add_argument('--census', action='store_true')
    ap.add_argument('--class', dest='cls', default='')
    ap.add_argument('--json')
    ap.add_argument('--all', action='store_true', help='include claimed markers (broken claims)')
    ap.add_argument('--jobs', type=int, default=int(os.environ.get('FE_JOBS', '5')))
    ap.add_argument('--pfx-base', type=int, default=int(os.environ.get('FE_PFX_BASE', '21')))
    a = ap.parse_args()
    units = units_of(a.units)
    if a.census:
        rows = census(units)
        tab = collections.Counter()
        for r in rows:
            f = r['src']
            key = (f['guard'], 'ret=%s' % ('1' if f['nret'] <= 1 else '2+'), 'in=%s' % ('0' if f['inner'] == 0 else '1+'),
                   'goto' if f['ngoto'] else '-', 'trail' if f['trail'] else '-')
            tab[(key, r['shape'])] += 1
        keys = sorted({k for k, _ in tab}, key=lambda k: -(tab[(k, 'U')] + tab[(k, 'D')]))
        print('%-44s %5s %5s %5s' % ('guard / returns / nested returns / goto / trailing return', 'U', 'D', '-'))
        for k in keys:
            print('%-44s %5d %5d %5d' % (' '.join(k), tab[(k, 'U')], tab[(k, 'D')], tab[(k, '-')]))
        print('exact functions with saves: %d  U=%d D=%d' % (len(rows), sum(r['shape'] == 'U' for r in rows), sum(r['shape'] == 'D' for r in rows)))
        if a.json:
            json.dump(rows, open(a.json, 'w'), indent=0)
        return
    only = {int(x, 16) for x in a.addr.split(',') if x}
    work = []
    for u in units:
        try:
            t = open(fe.R + '/' + u, encoding='latin-1').read()
        except OSError:
            continue
        ad = [int(m.group(1), 16) for m in RX.finditer(t)
              if (int(m.group(1), 16) in only if only else (a.all or 'BYTE_EXACT' not in m.group(2)))]
        if ad:
            work.append((u, ad))
    if not work:
        return
    fe.compile_cmd(work[0][0])

    def lane(k):
        r = []
        for i in range(k, len(work), a.jobs):
            r += scan_unit(work[i][0], work[i][1], a.pfx_base + k, a.src if only else None)
        return r
    res = []
    with ThreadPoolExecutor(a.jobs) as ex:
        for o in ex.map(lane, range(a.jobs)):
            res += o
    for e in [x for x in res if 'error' in x]:
        print('ERROR', e['unit'], (e['error'] or '')[:200])
    res = [x for x in res if 'error' not in x]
    res.sort(key=lambda x: (x['cls'].replace('s', 'z') or '~', x['diff']))
    n = collections.Counter()
    for x in res:
        for ch in x['cls']:
            n[ch] += 1
        if not x['cls'] and not only:
            continue
        if a.cls and not set(a.cls) & set(x['cls']):
            continue
        f = x['src'] or {}
        print('%-4s %s %-24s %-34s diff=%-4d src=%s ret=%s in=%s goto=%s' % (
            x['cls'] or '=', x['addr'], x['unit'][4:], x['sym'][:34], x['diff'], f.get('guard'), f.get('nret'), f.get('inner'), f.get('ngoto')))
        print('       img  ' + fmt(x['img']))
        print('       ours ' + fmt(x['ours']))
    print('non-exact functions: %d  a=%d b=%d c=%d d=%d s=%d' % (len(res), n['a'], n['b'], n['c'], n['d'], n['s']))
    if a.json:
        json.dump(res, open(a.json, 'w'), indent=0)


if __name__ == '__main__':
    main()
