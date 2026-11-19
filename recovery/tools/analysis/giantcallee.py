#!/usr/bin/env python3
"""giantcallee.py [--giants N] [--add NAME,..] [--max-size BYTES] [--gap BYTES] [--addr A,..] [--copies-only]
                [--json F] | --probes

Header-inline screen (MATCHING_GUIDE 2.8, "Header inlines behind dead probes"). A small helper whose
out-of-line copy in the image is called only by budget-exhausted giants, and sits next to their code
or to other such copies, was a header inline: every other caller expanded it. Earlier recoveries
often defined such a helper as a plain function in its own .cpp and hand-wrote its body at the
other call sites (the dead `*_budget_probe` calls then stood in for the spend the expansion made).

Pure image + text screen, no compiler:
  1. image call graph over every ORIGINAL marker span (direct `call rel32`, tail `jmp rel32`);
  2. giants = the N largest marker spans (default 30) plus --add names (source or mangled names);
  3. a candidate is a marker of at most --max-size bytes with at least one image caller, every image
     caller a giant;
  4. its neighbourhood: the previous/next marker in address order (within --gap bytes of padding),
     tagged G (a giant), C (another candidate) or H (a header inline, `// body` pointer); `run` counts
     the contiguous stretch of C/H copies it belongs to;
  5. its source status: HDR (marker has a `// body <header>` pointer) or PLAIN (defined in its .cpp);
  6. hand-written copies of its body in the tree: the body's tokens (braces, type keywords, a leading
     `return` and a final `return <local>;` dropped) with parameters matching any balanced expression
     (the same text at each use) and the helper's locals matching any identifier, searched in every
     function body except the definition; each copy names its host marker (`x` = BYTE_EXACT host,
     `G` = a giant).
--probes counts the dead budget-probe call sites in the tree (the number a header inline retires).
"""
import sys, os, re, glob, json, argparse, collections, struct
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
sys.path.insert(0, HERE)
import fe  # noqa: E402
import runscan  # noqa: E402

R = fe.R
MRE = re.compile(r'// ORIGINAL: 0x([0-9A-F]{8}) (\S+) 0x([0-9A-Fa-f]+)-0x([0-9A-Fa-f]+)([^\n]*)\n((?://(?! ORIGINAL:)[^\n]*\n)*)')
PROBE_RE = re.compile(r'\b(\w*budget_probe\w*|dw_probe|click_probe|bb_probe|hof_probe|pick_service_probe|uv2_probe_flat)\s*\(')


class Mk:
    __slots__ = ('addr', 'sym', 'lo', 'hi', 'exact', 'path', 'line', 'pos', 'end', 'body_hdr', 'name')


def body_pointer(t, m):
    """header path of a `// body <path>` pointer in the marker's comment block or right after it"""
    mb = re.search(r'// body\s+(\S+)', m.group(6))
    if mb:
        return mb.group(1)
    i = m.end()
    close = t.find('*/', i)
    nxt = t.find('// ORIGINAL:', i)
    if close >= 0 and (nxt < 0 or close < nxt) and '{' not in t[i:close] and ';' not in t[i:close]:
        i = close + 2
    for ln in t[i:i + 2000].split('\n')[1 if t[i - 1:i] != '\n' else 0:]:
        s = ln.strip()
        if not s:
            continue
        if not s.startswith('//'):
            return None
        mb = re.match(r'// body\s+(\S+)', s)
        if mb:
            return mb.group(1)
    return None


def sig_params(sig):
    """parameter names of the last top-level parenthesised list in a signature"""
    j = sig.rfind(')')
    if j < 0:
        return []
    d, i = 0, j
    while i >= 0:
        if sig[i] == ')':
            d += 1
        elif sig[i] == '(':
            d -= 1
            if d == 0:
                break
        i -= 1
    inner = sig[i + 1:j]
    parts, d, cur = [], 0, ''
    for ch in inner:
        if ch in '([':
            d += 1
        elif ch in ')]':
            d -= 1
        if ch == ',' and d == 0:
            parts.append(cur)
            cur = ''
        else:
            cur += ch
    parts.append(cur)
    out = []
    for p in parts:
        p = p.split('=')[0]
        mu = re.search(r'UNUSED\s*\(\s*(\w+)\s*\)', p)
        if mu:
            out.append(mu.group(1))
            continue
        ids = [x for x in re.findall(r'[A-Za-z_]\w*', p) if x not in runscan.KEYW and x not in DROP]
        if ids and len(re.findall(r'[A-Za-z_]\w*', p)) >= 2:
            out.append(ids[-1])
    return out


def load_markers():
    out = []
    files = sorted(glob.glob(R + '/src/**/*.cpp', recursive=True) + glob.glob(R + '/src/**/*.h', recursive=True))
    texts = {}
    for p in files:
        t = open(p, encoding='latin-1').read()
        texts[p] = t
        for m in MRE.finditer(t):
            k = Mk()
            k.addr = int(m.group(1), 16)
            k.sym = m.group(2)
            m2 = re.search(r'// symbol\s+(\S+)', m.group(6))
            if m2:
                k.sym = m2.group(1)
            k.lo, k.hi = int(m.group(3), 16), int(m.group(4), 16)
            k.exact = 'BYTE_EXACT' in m.group(5)
            k.path = p
            k.pos, k.end = m.start(), m.end()
            k.line = t.count('\n', 0, m.start()) + 1
            k.body_hdr = body_pointer(t, m)
            k.name = runscan.sym_name(k.sym) or k.sym
            out.append(k)
    return out, texts


def call_graph(mks):
    """callee addr -> Counter(caller addr -> number of call sites)"""
    starts = {k.lo: k for k in mks}
    callers = collections.defaultdict(collections.Counter)
    seen = set()
    for k in mks:
        if (k.lo, k.hi) in seen or k.hi <= k.lo:
            continue
        seen.add((k.lo, k.hi))
        data = fe.imgbytes(k.lo, k.hi)
        if not data:
            continue
        for ins in fe.md.disasm(data, k.lo):
            if ins.size != 5 or ins.mnemonic not in ('call', 'jmp'):
                continue
            try:
                t = int(ins.op_str, 16)
            except ValueError:
                continue
            if k.lo <= t < k.hi or t not in starts:
                continue
            callers[t][k.lo] += 1
    return callers


# ---------------------------------------------------------------- body text
def definition(mk, texts):
    """(text, open_brace, close_brace, path) of the helper's definition, or None"""
    if mk.body_hdr:
        p = R + '/' + mk.body_hdr
        t = texts.get(p)
        if t is None:
            return None
        nm = mk.name.split('::')[-1]
        # the header definition carries the address comment, or is the first definition of the name
        cand = [m.start() for m in re.finditer(r'[^\n]*\b%s\s*\([^;{]*\)[^;{]*\{[^\n]*' % re.escape(nm), t)]
        tag = [c for c in cand if ('%08X' % mk.addr) in t[c:t.find('\n', c)].upper()]
        for c in tag + cand:
            bs = runscan.body_span(t, c)
            if bs and bs[0] != 'skip':
                return t, bs[0], bs[1], p
        return None
    t = texts[mk.path]
    i = mk.end
    for _ in range(4):
        bs = runscan.body_span(t, i)
        if bs is None:
            return None
        if bs[0] == 'skip':
            i = bs[1]
            continue
        return t, bs[0], bs[1], mk.path
    return None


DROP = set('{ }'.split()) | runscan.TYPEW | {'uint32_t', 'int32_t', 'uint16_t', 'int16_t', 'uint8_t', 'int8_t', 'BOOL', 'LPSTR', 'DWORD', 'WORD', 'BYTE'}


def toks_of(text):
    return [s for s, _ in runscan.lex(runscan.strip_pp(text), 1)]


def pattern(text, ob, cb):
    """normalised token pattern of a body: literals, ('P', i) parameters, ('L', i) locals"""
    sig_start = max(0, text.rfind(';', 0, ob), text.rfind('}', 0, ob), text.rfind('*/', 0, ob))
    sig = re.sub(r'//[^\n]*', '', text[sig_start:ob])
    sig = sig[:sig.rfind(')') + 1]
    prm = sig_params(sig)
    body = toks_of(text[ob + 1:cb])
    # locals: identifiers declared in the body
    loc = set()
    for i in range(1, len(body)):
        if re.match(r'[A-Za-z_]\w*$', body[i]) and body[i - 1] in DROP and body[i - 1] not in ('{', '}') \
                and i + 1 < len(body) and body[i + 1] in ('=', ';', ',', '['):
            loc.add(body[i])
    # drop a final `return local;`
    if len(body) >= 3 and body[-3] == 'return' and body[-2] in loc and body[-1] == ';':
        body = body[:-3]
    body = [s for s in body if s not in DROP]
    if body and body[0] == 'return':
        body = body[1:]
    if body and body[-1] == ';':
        body = body[:-1]
    pi = {p: i for i, p in enumerate(prm)}
    li = {l: i for i, l in enumerate(sorted(loc))}
    pat = []
    for s in body:
        if s in pi:
            pat.append(('P', pi[s]))
        elif s in li:
            pat.append(('L', li[s]))
        else:
            pat.append(s)
    return pat, prm, sorted(loc)


OPEN, CLOSE = {'(': ')', '[': ']'}, {')', ']'}


def match_at(hay, i, pat, k=0, env=None):
    """end index when pat[k:] matches hay[i:], else -1"""
    env = env if env is not None else {}
    while k < len(pat):
        p = pat[k]
        if isinstance(p, str):
            if i >= len(hay) or hay[i] != p:
                return -1
            i += 1
            k += 1
            continue
        kind, n = p
        if (kind, n) in env:
            b = env[(kind, n)]
            if hay[i:i + len(b)] != b:
                return -1
            i += len(b)
            k += 1
            continue
        if kind == 'L':
            if i >= len(hay) or not re.match(r'[A-Za-z_]\w*$', hay[i]):
                return -1
            env2 = dict(env)
            env2[(kind, n)] = hay[i:i + 1]
            return match_at(hay, i + 1, pat, k + 1, env2)
        # parameter: balanced expression, shortest first
        d, j = 0, i
        while j < len(hay) and j - i < 40:
            s = hay[j]
            if s in OPEN:
                d += 1
            elif s in CLOSE:
                if d == 0:
                    break
                d -= 1
            elif d == 0 and s in (';', ',', '{', '}'):
                break
            j += 1
            if d == 0:
                env2 = dict(env)
                env2[(kind, n)] = hay[i:j]
                e = match_at(hay, j, pat, k + 1, env2)
                if e >= 0:
                    return e
        return -1
    return i


def find_copies(pat, hosts, skip):
    """[(path, line, host marker)] of every place pat matches"""
    lits = [(k, p) for k, p in enumerate(pat) if isinstance(p, str)]
    if len(lits) < 2 or not any(re.match(r'[A-Za-z_]\w*$', p) for _, p in lits):
        return []
    k0, anchor = next(((k, p) for k, p in lits if re.match(r'[A-Za-z_]\w*$', p)), lits[0])
    out = []
    for (path, mk), (hay, lines) in hosts.items():
        if (path, mk.addr if mk else None) == skip:
            continue
        for j, s in enumerate(hay):
            if s != anchor:
                continue
            for i in range(j - k0 * 8 if k0 else j, j + 1):
                if i < 0:
                    continue
                if match_at(hay, i, pat) >= 0:
                    out.append((path, lines[i], mk))
                    break
    return out


def host_index(mks, texts):
    """{(path, marker): (tokens, lines)} for every function body after a marker (cpp files)"""
    by_path = collections.defaultdict(list)
    for k in mks:
        by_path[k.path].append(k)
    out = {}
    for p, ks in by_path.items():
        if not p.endswith('.cpp'):
            continue
        t = texts[p]
        for k in ks:
            if k.body_hdr:
                continue
            i = k.end
            for _ in range(4):
                bs = runscan.body_span(t, i)
                if bs is None:
                    break
                if bs[0] == 'skip':
                    i = bs[1]
                    continue
                ob, cb = bs
                line0 = t.count('\n', 0, ob) + 1
                tk = runscan.lex(runscan.strip_pp(t[ob + 1:cb]), line0)
                tk = [(s, ln) for s, ln in tk if s not in DROP]
                out[(p, k)] = ([s for s, _ in tk], [ln for _, ln in tk])
                break
    return out


def probes():
    n = collections.Counter()
    for p in sorted(glob.glob(R + '/src/**/*.cpp', recursive=True) + glob.glob(R + '/src/**/*.h', recursive=True)):
        for ln in open(p, encoding='latin-1'):
            s = ln.split('//')[0]
            for m in PROBE_RE.finditer(s):
                # a definition or declaration, not a call
                if re.search(r'\b(static|inline|void|int)\s+(inline\s+)?(void|int\s+)?\s*' + m.group(1) + r'\s*\(', s):
                    continue
                n[os.path.relpath(p, R)] += 1
    return n


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--giants', type=int, default=30)
    ap.add_argument('--add', default='base_build,battle_fight,communicate,probe,enemy_move,enemy_strategy,order_veh,'
                    'air_power,setup_text,gen_terrain_poly')
    ap.add_argument('--max-size', type=lambda x: int(x, 0), default=0x100)
    ap.add_argument('--gap', type=lambda x: int(x, 0), default=0x10)
    ap.add_argument('--addr', default='')
    ap.add_argument('--copies-only', action='store_true', help='list only candidates with hand-written copies')
    ap.add_argument('--json')
    ap.add_argument('--probes', action='store_true')
    a = ap.parse_args()
    if a.probes:
        n = probes()
        for p, c in n.most_common():
            print('%4d %s' % (c, p))
        print('PROBE CALLS', sum(n.values()))
        return 0
    mks, texts = load_markers()
    by_lo = {}
    for k in mks:
        by_lo.setdefault(k.lo, k)
    order = sorted(by_lo.values(), key=lambda k: k.lo)
    callers = call_graph(mks)
    big = sorted(order, key=lambda k: k.hi - k.lo, reverse=True)[:a.giants]
    giants = {k.lo for k in big}
    add = set(x for x in a.add.split(',') if x)
    for k in order:
        if k.name in add or k.sym in add:
            giants.add(k.lo)
    only = {int(x, 16) for x in a.addr.split(',') if x}
    cands = []
    for k in order:
        cs = callers.get(k.lo)
        if only and k.addr not in only:
            continue
        if not only and (not cs or k.hi - k.lo > a.max_size or k.lo in giants or not set(cs) <= giants):
            continue
        cands.append(k)
    cset = {k.lo for k in cands}
    idx = {k.lo: i for i, k in enumerate(order)}

    def tag(n):
        if n.lo in giants:
            return 'G'
        if n.lo in cset:
            return 'C'
        if n.body_hdr:
            return 'H'
        return '-'
    hosts = host_index(mks, texts)
    rows = []
    for k in cands:
        i = idx[k.lo]
        prv = order[i - 1] if i > 0 and k.lo - order[i - 1].hi <= a.gap else None
        nxt = order[i + 1] if i + 1 < len(order) and order[i + 1].lo - k.hi <= a.gap else None
        # contiguous run of C/H copies
        run, j = 1, i - 1
        while j >= 0 and order[j + 1].lo - order[j].hi <= a.gap and tag(order[j]) in 'CH':
            run += 1
            j -= 1
        j = i + 1
        while j < len(order) and order[j].lo - order[j - 1].hi <= a.gap and tag(order[j]) in 'CH':
            run += 1
            j += 1
        cs = callers.get(k.lo, {})
        cl = ' '.join('%s:%d' % (by_lo[c].name, n) for c, n in sorted(cs.items(), key=lambda x: -x[1]))
        d = definition(k, texts)
        copies = []
        pat = None
        if d:
            t, ob, cb, p = d
            pat, prm, loc = pattern(t, ob, cb)
            copies = find_copies(pat, hosts, (k.path, k.addr) if not k.body_hdr else (None, None))
            copies = [c for c in copies if not (c[2] and c[2].lo == k.lo)]
        if a.copies_only and not copies:
            continue
        rows.append(dict(addr='%08X' % k.addr, name=k.name, size=k.hi - k.lo, file=os.path.relpath(k.path, R),
                         status='HDR' if k.body_hdr else 'PLAIN', callers=cl,
                         prev=(tag(prv) + ':' + prv.name) if prv else '', next=(tag(nxt) + ':' + nxt.name) if nxt else '',
                         run=run,
                         copies=[dict(file=os.path.relpath(c[0], R), line=c[1], host=c[2].name if c[2] else '',
                                      host_addr='%08X' % c[2].addr if c[2] else '',
                                      exact=bool(c[2] and c[2].exact), giant=bool(c[2] and c[2].lo in giants))
                                 for c in copies],
                         pattern=' '.join(p if isinstance(p, str) else '%s%d' % p for p in (pat or []))[:120]))
    rows.sort(key=lambda r: (r['status'] != 'PLAIN', -len(r['copies']), r['addr']))
    print('giants (%d): %s' % (len(giants), ' '.join(sorted(by_lo[g].name for g in giants))))
    for r in rows:
        print('%s %-28s %4d %-5s run=%d prev=%-24s next=%-24s callers %s' % (
            r['addr'], r['name'][:28], r['size'], r['status'], r['run'], r['prev'][:24], r['next'][:24], r['callers']))
        print('    %s  pat: %s' % (r['file'], r['pattern']))
        for c in r['copies']:
            print('    copy %s:%d in %s %s%s%s' % (c['file'], c['line'], c['host'], c['host_addr'],
                                                 ' x' if c['exact'] else '', ' G' if c['giant'] else ''))
    print('CANDIDATES', len(rows), 'PLAIN', sum(r['status'] == 'PLAIN' for r in rows),
          'WITH-COPIES', sum(bool(r['copies']) for r in rows))
    if a.json:
        json.dump(rows, open(a.json, 'w'), indent=1)
    return 0


if __name__ == '__main__':
    sys.exit(main())
