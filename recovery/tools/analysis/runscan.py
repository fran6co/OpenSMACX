#!/usr/bin/env python3
"""runscan.py [unit ...] [--fe FEALL.json] [--min-tree 3] [--min-fn 2] [--top 40] [--addr A,..] [--json F]
           [--level 1|2] [--sites N]

Repeated-statement-run screen (MATCHING_GUIDE 2.8, "helpers beat dead ifs"). Earlier recoveries
hand-wrote many runs of real calls that the original spelled as inline helpers; written out, they
leave /Ob2 pass-2 room that the original spent (Console::terraform: two say_tech runs and two
refunds as helpers refuse ~Scroll at early teardowns where dead-if probes moved nothing).

Every function body after an ORIGINAL marker is split into straight-line runs: consecutive simple
statements of one block (a no-else `if (c) { simple; ... }` counts as one composite statement).
Windows of 2-5 statements holding at least one call are normalised and counted:
  level 1: locals/parameters -> V, numbers -> K, strings -> S; globals, members, callees kept;
  level 2 (default): additionally every call argument list -> (A) and every subscript -> [I]
           (the helper's parameters can be any expression).
A window is listed when it occurs at 3+ places in the tree or 2+ places in one function, and is
not a sub-window of a longer listed window with the same sites. Rank: sites in non-exact
functions (fe json if given, else the marker's BYTE_EXACT token), then total. Sites in exact
functions are marked `x`: an inline helper must leave them exact (or be shown structurally right).
`scope` says whether all sites share one unit (file-local static inline) or span units (header).
Pure text screen, no compiler.
"""
import sys, os, re, json, glob, argparse, collections

HERE = os.path.dirname(os.path.abspath(__file__))
R = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))

KEYW = set('''if else for while do switch case default return break continue goto sizeof new delete
this const static unsigned signed int char short long float double void bool struct class enum
volatile register inline __inline __forceinline extern typedef try catch throw operator'''.split())
TYPEW = set('const unsigned signed int char short long float double void bool struct class enum volatile register static'.split())

TOK = re.compile(r'''\s+|//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|'(?:\\.|[^'\\])*'|0[xX][0-9a-fA-F]+[uUlL]*|\d+(?:\.\d*)?[uUlLfF]*|[A-Za-z_]\w*|->|::|<<=|>>=|\+\+|--|&&|\|\||[-+*/%&|^!=<>]=|<<|>>|.''', re.S)


def lex(text, line0, pos0=0):
    out = []
    line = line0
    for m in TOK.finditer(text):
        s = m.group(0)
        if not s.isspace() and not s.startswith('//') and not s.startswith('/*'):
            out.append(Tok(s, line, pos0 + m.start()))
        line += s.count('\n')
    return out


class Tok(tuple):
    """(text, line) with the token's offset in the file as .pos"""
    def __new__(cls, s, line, pos):
        t = tuple.__new__(cls, (s, line))
        t.pos = pos
        return t


def strip_pp(text):
    # preprocessor lines -> blank (keep line numbers)
    return re.sub(r'^[ \t]*#[^\n]*', lambda m: ' ' * len(m.group(0)), text, flags=re.M)


def body_span(text, start):
    """(open_brace, close_brace) of the definition after position start, or None"""
    i, depth = start, 0
    n = len(text)
    while i < n:
        if text.startswith('//', i):
            i = text.find('\n', i)
            if i < 0:
                return None
            continue
        ch = text[i]
        if ch == '(':
            depth += 1
        elif ch == ')':
            depth -= 1
        elif ch == ';' and depth == 0:
            return ('skip', i + 1)  # a declaration between the marker and its definition
        elif ch == '/' and text.startswith('/*', i):
            i = text.find('*/', i) + 1
        elif ch == '{' and depth == 0:
            break
        i += 1
    ob, d, j = i, 0, i
    while j < n:
        if text.startswith('//', j):
            j = text.find('\n', j)
            continue
        if text.startswith('/*', j):
            j = text.find('*/', j) + 2
            continue
        ch = text[j]
        if ch in '"\'':
            k = j + 1
            while k < n and text[k] != ch:
                k += 2 if text[k] == '\\' else 1
            j = k + 1
            continue
        if ch == '{':
            d += 1
        elif ch == '}':
            d -= 1
            if d == 0:
                return ob, j
        j += 1
    return None


def params(sig_toks):
    names = set()
    # last '(' group at depth 0 before the body
    d, cur, groups = 0, [], []
    for s, _ in sig_toks:
        if s == '(':
            d += 1
            if d == 1:
                cur = []
                continue
        elif s == ')':
            d -= 1
            if d == 0:
                groups.append(cur)
                continue
        if d >= 1:
            cur.append(s)
    if not groups:
        return names
    g = groups[-1] if len(groups) == 1 else groups[0]
    part = []
    for s in g + [',']:
        if s == ',':
            ids = [x for x in part if re.match(r'[A-Za-z_]\w*$', x) and x not in KEYW]
            if ids:
                names.add(ids[-1])
            part = []
        elif s == '=':
            part.append('=')
            part = [x for x in part[:-1]]  # default arg: name is before '='
            part.append('\0')
        elif '\0' not in part:
            part.append(s)
    return names


def decl_name(st):
    """name declared by a statement token list, if it looks like a declaration"""
    toks = [s for s, _ in st]
    k = 0
    seen_type = 0
    while k < len(toks) and (re.match(r'[A-Za-z_]\w*$', toks[k]) or toks[k] in ('*', '&', '::', '<', '>')):
        if toks[k] in ('return', 'delete', 'goto') or toks[k] in KEYW and toks[k] not in TYPEW:
            return []
        k += 1
        seen_type += 1
        if k < len(toks) and toks[k] in ('=', ';', '[', ',', '(') and seen_type >= 2 and re.match(r'[A-Za-z_]\w*$', toks[k - 1]):
            if toks[k] == '(' and toks[k - 2] not in ('*', '&') and not re.match(r'[A-Za-z_]\w*$', toks[k - 2]):
                return []
            names = [toks[k - 1]]
            # further declarators: , name
            d = 0
            for j in range(k, len(toks) - 1):
                if toks[j] in '([{':
                    d += 1
                elif toks[j] in ')]}':
                    d -= 1
                elif toks[j] == ',' and d == 0:
                    q = j + 1
                    while q < len(toks) and toks[q] in ('*', '&'):
                        q += 1
                    if q < len(toks) and re.match(r'[A-Za-z_]\w*$', toks[q]):
                        names.append(toks[q])
            return names
    return []


def sym_name(sym):
    """source name of a decorated symbol (None when it cannot be checked)"""
    if sym.startswith('??0'):
        return sym[3:].split('@')[0]
    if sym.startswith('??1'):
        return '~' + sym[3:].split('@')[0]
    if sym.startswith('??'):
        return None
    if sym.startswith('?'):
        return sym[1:].split('@')[0]
    return sym.lstrip('_') or None


class Fn:
    def __init__(self, path, addr, sym, exact):
        self.path, self.addr, self.sym, self.exact = path, addr, sym, exact
        self.runs = []  # list of lists of (raw_tokens, line)
        self.locals = set()


def take_stmt(toks, i):
    """simple statement from i to its ';' (inclusive) -> (tokens, next)"""
    d = 0
    j = i
    while j < len(toks):
        s = toks[j][0]
        if s in '([{':
            d += 1
        elif s in ')]}':
            d -= 1
            if d < 0:
                return toks[i:j], j
        elif s == ';' and d == 0:
            return toks[i:j + 1], j + 1
        j += 1
    return toks[i:j], j


def paren(toks, i):
    """toks[i] == '(' -> index after matching ')'"""
    d = 0
    for j in range(i, len(toks)):
        if toks[j][0] == '(':
            d += 1
        elif toks[j][0] == ')':
            d -= 1
            if d == 0:
                return j + 1
    return len(toks)


def parse_stmt(toks, i, fn, run):
    """parse one statement at i; append simple ones to run or close the run. -> next index"""
    s = toks[i][0]
    if s == '{':
        j = parse_block(toks, i + 1, fn)
        close(fn, run)
        return j
    if s == '}':
        return i  # caller handles
    if s == 'if':
        j = paren(toks, i + 1)
        cond = toks[i + 1:j]
        bstart = j
        sub = []
        if j < len(toks) and toks[j][0] == '{':
            k = parse_block(toks, j + 1, fn, collect=sub)
        else:
            k = parse_stmt(toks, j, fn, sub_run := [])
            sub = [sub_run[:]] if sub_run else [[]]
            close(fn, sub_run)
        has_else = k < len(toks) and toks[k][0] == 'else'
        body = toks[bstart:k]
        bt = [x for x, _ in body]
        simple_body = (not has_else and 1 <= bt.count(';') <= 5 and bt.count('{') <= 1
                       and not set(bt) & {'return', 'break', 'continue', 'goto', 'if', 'for', 'while', 'switch', 'do', 'case', 'throw'})
        if simple_body:
            run.append((toks[i:bstart] + body, toks[i][1], 'if'))
        else:
            close(fn, run)
        if has_else:
            k2 = parse_stmt(toks, k + 1, fn, er := [])
            close(fn, er)
            close(fn, run)
            return k2
        return k
    if s in ('for', 'while', 'switch'):
        j = paren(toks, i + 1)
        close(fn, run)
        k = parse_stmt(toks, j, fn, r2 := [])
        close(fn, r2)
        return k
    if s == 'do':
        close(fn, run)
        k = parse_stmt(toks, i + 1, fn, r2 := [])
        close(fn, r2)
        st, k = take_stmt(toks, k)  # while (...);
        return k
    if s in ('case', 'default'):
        close(fn, run)
        j = i
        while j < len(toks) and toks[j][0] != ':':
            j += 1
        return j + 1
    if s in ('return', 'break', 'continue', 'goto', 'throw'):
        close(fn, run)
        st, k = take_stmt(toks, i)
        run.append(None)  # a jump inside a composite body disqualifies it
        close(fn, run)
        return k
    if re.match(r'[A-Za-z_]\w*$', s) and i + 1 < len(toks) and toks[i + 1][0] == ':' :
        close(fn, run)  # label
        return i + 2
    if s == ';':
        return i + 1
    if s in ('__try', 'try'):
        close(fn, run)
        return i + 1
    st, k = take_stmt(toks, i)
    if k == i:
        return i + 1
    for nm in decl_name(st):
        fn.locals.add(nm)
    run.append((st, toks[i][1], 's'))
    return k


def close(fn, run):
    seg = []
    for x in run:
        if x is None:
            if seg:
                fn.runs.append(seg)
            seg = []
        else:
            seg.append(x)
    if seg:
        fn.runs.append(seg)
    run.clear()


def parse_block(toks, i, fn, collect=None):
    run = []
    while i < len(toks) and toks[i][0] != '}':
        j = parse_stmt(toks, i, fn, run)
        if j == i:
            j = i + 1
        i = j
    if collect is not None:
        collect.append(run[:])
    close(fn, run)
    return i + 1


def norm(tokens, locs, level):
    return ' '.join(norm_list(tokens, locs, level))


def norm_list(tokens, locs, level):
    out = []
    toks = [s for s, _ in tokens]
    i = 0
    n = len(toks)
    while i < n:
        s = toks[i]
        nxt = toks[i + 1] if i + 1 < n else ''
        prv = out[-1] if out else ''
        if level >= 2 and s == '(' and prv and re.match(r'[A-Za-z_~]\w*$', prv) and prv not in KEYW | {'V'}:
            out.append('(A)')
            i = paren([(t, 0) for t in toks], i)
            continue
        if level >= 2 and s == '[':
            d = 0
            for j in range(i, n):
                if toks[j] == '[':
                    d += 1
                elif toks[j] == ']':
                    d -= 1
                    if d == 0:
                        break
            out.append('[I]')
            i = j + 1
            continue
        if s[0] == '"':
            out.append('S')
        elif s[0] == "'" or s[0].isdigit():
            out.append('K')
        elif re.match(r'[A-Za-z_]\w*$', s):
            if s in KEYW or prv in ('.', '->', '::') or nxt in ('(', '::'):
                out.append(s)
            elif s in locs:
                out.append('V')
            else:
                out.append(s)
        else:
            out.append(s)
        i += 1
    return out


def calls_in(tokens):
    toks = [s for s, _ in tokens]
    return [toks[i] for i in range(len(toks) - 1) if toks[i + 1] == '(' and re.match(r'[A-Za-z_~]\w*$', toks[i])
            and toks[i] not in KEYW]


def scan_file(path, exact_set, only_addrs):
    raw = open(path, encoding='latin-1').read()
    text = strip_pp(raw)
    rel = path[len(R) + 1:]
    fns = []
    for m in re.finditer(r'^// ORIGINAL: 0x([0-9A-F]{8}) (\S+) 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)', text, re.M):
        addr = int(m.group(1), 16)
        if only_addrs and addr not in only_addrs:
            continue
        # skip the marker's comment block
        q = m.end()
        while True:
            nl = text.find('\n', q)
            if nl < 0:
                break
            q = nl + 1
            rest = text[q:]
            if not (rest.lstrip(' ').startswith('//') or rest.startswith('\n')):
                break
        name = sym_name(m.group(2))
        sp = None
        first = None
        for _ in range(12):
            sp = body_span(text, q)
            if sp and sp[0] == 'skip':
                q, sp = sp[1], None
                continue
            if sp and first is None:
                first = sp
            if not sp or not name or re.search(r'(?<![\w~])%s\s*\(' % re.escape(name), text[q:sp[0]]):
                break
            if re.search(r'^// ORIGINAL: ', text[q:sp[0]], re.M):
                sp = None  # passed the next marker
                break
            q = sp[1] + 1  # an inline helper between the marker and its definition
            sp = None
        sp = sp or first
        if not sp:
            continue
        ob, cb = sp
        line0 = text.count('\n', 0, ob) + 1
        sig = lex(text[q:ob], 0)
        if exact_set is None:
            exact = 'BYTE_EXACT' in m.group(3)
        else:
            exact = m.group(1) not in exact_set
        fn = Fn(rel, addr, m.group(2), exact)
        fn.locals |= params(sig)
        toks = lex(text[ob + 1:cb], line0, ob + 1)
        fn.span = (m.start(), ob, cb)
        parse_block(toks + [Tok('}', 0, cb)], 0, fn)
        fns.append(fn)
    return fns


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='*')
    ap.add_argument('--fe', help='feall json: functions it lists non-exact are non-exact, every other marker exact')
    ap.add_argument('--min-tree', type=int, default=3)
    ap.add_argument('--min-fn', type=int, default=2)
    ap.add_argument('--top', type=int, default=40)
    ap.add_argument('--sites', type=int, default=12)
    ap.add_argument('--level', type=int, default=2)
    ap.add_argument('--addr', default='')
    ap.add_argument('--calls', type=int, default=1, help='minimum calls in a window')
    ap.add_argument('--distinct', type=int, default=2, help='minimum distinct statements in a window')
    ap.add_argument('--idioms', default='strcat,strcpy,strncpy,_itoa,itoa,sprintf,CharUpperA,parse_says,parse_num,memset,memcpy,strlen,strcmp,stricmp,_stricmp',
                    help='calls that do not make a window interesting on their own')
    ap.add_argument('--json')
    a = ap.parse_args()
    exact_set = None
    if a.fe:
        exact_set = {r['addr'] for r in json.load(open(a.fe)) if not r.get('exact')}  # the non-exact set
    only = {int(x, 16) for x in a.addr.split(',') if x} if a.addr else None
    paths = sorted(glob.glob(R + '/src/**/*.cpp', recursive=True))
    if a.units:
        paths = [p for p in paths if p[len(R) + 1:] in a.units or os.path.basename(p) in a.units]
    fns = []
    for p in paths:
        fns += scan_file(p, exact_set, None)
    idioms = set(x for x in a.idioms.split(',') if x)
    occ = collections.defaultdict(list)  # key -> [(fn, run_idx, start, n)]
    nkeys = {}
    for fi, fn in enumerate(fns):
        for ri, run in enumerate(fn.runs):
            ns = [norm(st, fn.locals, a.level) for st, _, _ in run]
            nkeys[(fi, ri)] = ns
            for w in range(2, 6):
                for s0 in range(0, len(run) - w + 1):
                    sl = run[s0:s0 + w]
                    nc = sum(len(calls_in(st)) for st, _, _ in sl)
                    if nc < a.calls:
                        continue
                    if len(set(ns[s0:s0 + w])) < a.distinct:
                        continue
                    if idioms and all(c in idioms for st, _, _ in sl for c in calls_in(st)):
                        continue
                    if any(len(x) < 3 for x in ns[s0:s0 + w]):
                        continue
                    occ[tuple(ns[s0:s0 + w])].append((fi, ri, s0, w))

    def listed(k, sites):
        if only and not any(fns[s[0]].addr in only for s in sites):
            return False
        if len(sites) >= a.min_tree:
            return True
        c = collections.Counter(s[0] for s in sites)
        return max(c.values()) >= a.min_fn

    keep = {k: v for k, v in occ.items() if listed(k, v)}

    def maximal(k, v):
        # drop if every site extends by the same neighbour statement (left or right)
        for side in (-1, 1):
            ext = set()
            for fi, ri, s0, w in v:
                ns = nkeys[(fi, ri)]
                j = s0 - 1 if side < 0 else s0 + w
                ext.add(ns[j] if 0 <= j < len(ns) else None)
            if len(ext) == 1 and None not in ext and len(k) < 5:
                return False
        return True

    res = []
    for k, v in keep.items():
        if not maximal(k, v):
            continue
        ne = [s for s in v if not fns[s[0]].exact]
        if not ne:
            continue
        units = {fns[s[0]].path for s in v}
        nf = len({s[0] for s in v})
        res.append(dict(key=k, n=len(v), nonexact=len(ne), exact=len(v) - len(ne), funcs=nf, units=len(units), sites=v))
    res.sort(key=lambda r: (-r['nonexact'], -r['n'], -len(r['key'])))
    out = []
    for r in res[:a.top]:
        scope = 'unit' if r['units'] == 1 else 'tree'
        print('== %d sites (%d non-exact, %d exact) in %d fns, %d units [%s]' % (r['n'], r['nonexact'], r['exact'], r['funcs'], r['units'], scope))
        for st in r['key']:
            print('     | ' + st[:150])
        for fi, ri, s0, w in r['sites'][:a.sites]:
            fn = fns[fi]
            run = fn.runs[ri]
            print('   %s %08X %-40s %s:%d' % ('x' if fn.exact else '.', fn.addr, fn.sym[:40], fn.path, run[s0][1]))
        if len(r['sites']) > a.sites:
            print('   ... %d more' % (len(r['sites']) - a.sites))
        out.append(dict(key=list(r['key']), n=r['n'], nonexact=r['nonexact'], exact=r['exact'], scope=scope,
                        sites=[dict(addr='%08X' % fns[fi].addr, sym=fns[fi].sym, path=fns[fi].path,
                                    line=fns[fi].runs[ri][s0][1], exact=fns[fi].exact) for fi, ri, s0, w in r['sites']]))
    print('windows listed %d (of %d repeated); functions scanned %d' % (len(res), len(keep), len(fns)))
    if a.json:
        json.dump(out, open(a.json, 'w'), indent=1)


# ---------------------------------------------------------------- --try: helper screen

# a dead budget probe statement: a call of a *probe* function, or a line of calls tagged WORKAROUND(inline)
PROBE_LINE = re.compile(r'^[ \t]*(?:\w*probe\w*\([^;\n]*\);[^\n]*|(?:\w+\([^;\n]*\);\s*)+//\s*WORKAROUND\(inline\)[^\n]*)\n', re.M)


def windows_in(fn, wmin=2, wmax=5, min_sites=2):
    """level-1 windows repeated (non-overlapping) inside one function -> [(key, [(run, s0, w)])]"""
    if wmin == 1:
        min_sites = max(min_sites, 3)
    occ = collections.defaultdict(list)
    for ri, run in enumerate(fn.runs):
        for w in range(wmin, wmax + 1):
            for s0 in range(0, len(run) - w + 1):
                sl = run[s0:s0 + w]
                if not any(calls_in(st) for st, _, _ in sl):
                    continue
                if any(decl_name(st) for st, _, _ in sl):
                    continue
                if any('probe' in c or c.endswith('_spend') for st, _, _ in sl for c in calls_in(st)):
                    continue  # existing dead budget probes are what the screen replaces
                key = tuple(norm(st, fn.locals, 1) for st, _, _ in sl)
                occ[key].append((ri, s0, w))
    out = []
    for key, v in occ.items():
        chosen, used = [], set()
        for ri, s0, w in v:
            cells = {(ri, k) for k in range(s0, s0 + w)}
            if cells & used:
                continue
            used |= cells
            chosen.append((ri, s0, w))
        if len(chosen) >= min_sites:
            out.append((key, chosen))
    # drop windows contained in a longer window with the same number of sites
    keep = []
    for key, v in out:
        sub = False
        for k2, v2 in out:
            if len(k2) > len(key) and len(v2) == len(v) and any(k2[i:i + len(key)] == key for i in range(len(k2) - len(key) + 1)):
                sub = True
                break
        if not sub:
            keep.append((key, v))
    keep.sort(key=lambda kv: (-len(kv[1]) * len(kv[0]), -len(kv[0])))
    return keep


def helperize(text, fn, sites, name, members=()):
    """-> new text with the window at each site replaced by a call of a template inline helper.
    members: identifiers that are implicit members of the caller's class (written self->X in the helper)"""
    runs = [[t for st, _, _ in fn.runs[ri][s0:s0 + w] for t in st] for ri, s0, w in sites]
    stmts0 = [st for st, _, _ in fn.runs[sites[0][0]][sites[0][1]:sites[0][1] + sites[0][2]]]
    n = len(runs[0])
    if any(len(r) != n for r in runs):
        return None
    nl = [norm_list(r, fn.locals, 1) for r in runs]
    params = {}  # tuple of raw tokens -> index
    kinds = []
    slot = []
    for i in range(n):
        raw = tuple(r[i][0] for r in runs)
        local = any(x[i] == 'V' for x in nl)
        if raw[0] == 'this' or local or len(set(raw)) > 1:
            if raw not in params:
                params[raw] = len(params)
                kinds.append('val' if raw[0] == 'this' or not local else 'ref')
            slot.append(params[raw])
        else:
            slot.append(None)
    body, k = [], 0
    self_used = False
    for st in stmts0:
        line = []
        prev = ''
        for t in st:
            if slot[k] is not None:
                line.append('p%d' % slot[k])
            elif t[0] in members and prev not in ('.', '->', '::'):
                line.append('p_self->' + t[0])
                self_used = True
            else:
                line.append(t[0])
            prev = t[0]
            k += 1
        body.append('    ' + ' '.join(line))
    if self_used:
        params[('this',) * len(runs)] = len(params)
        kinds.append('val')
        body = [b.replace('p_self->', 'p%d->' % (len(params) - 1)) for b in body]
    np_ = len(params)
    tmpl = 'template <%s> ' % ', '.join('class T%d' % i for i in range(np_)) if np_ else ''
    sig = ', '.join(('T%d &p%d' if kinds[i] == 'ref' else 'T%d p%d') % (i, i) for i in range(np_))
    helper = '// WORKAROUND(decl): runscan helper screen\n%sstatic inline void %s(%s) {\n%s\n}\n' % (tmpl, name, sig, '\n'.join(body))
    order = sorted(params.items(), key=lambda kv: kv[1])
    edits = []
    for si, r in enumerate(runs):
        args = ', '.join(raw[si] for raw, _ in order)
        a, b = r[0].pos, r[-1].pos + len(r[-1][0])
        edits.append((a, b, '%s(%s);' % (name, args)))
    out = text
    for a, b, rep in sorted(edits, reverse=True):
        out = out[:a] + rep + out[b:]
    mk = fn.span[0]
    return out[:mk] + helper + out[mk:]


def try_mode(a):
    import threading
    from concurrent.futures import ThreadPoolExecutor
    sys.path.insert(0, HERE)
    sys.path.insert(0, os.path.dirname(HERE))
    import fe
    import inlinemap as im
    unit = a.unit
    addr = int(a.try_, 16)
    text = open(R + '/' + unit, encoding='latin-1').read()
    fns = scan_file(R + '/' + unit, None, None)
    fn = [f for f in fns if f.addr == addr][0]
    ob, cb = fn.span[1], fn.span[2]
    body = text[ob:cb]
    noprobe = text[:ob] + PROBE_LINE.sub('', body) + text[cb:] if PROBE_LINE.search(body) else None
    wins = windows_in(fn, a.wmin)[:a.max]
    deck = [('base', text)]
    if noprobe:
        deck.append(('noprobe', noprobe))
    remake = {}

    def strip(v):
        f2 = [f for f in scan_file_text(unit, v) if f.addr == addr][0]
        return v[:f2.span[1]] + PROBE_LINE.sub('', v[f2.span[1]:f2.span[2]]) + v[f2.span[2]:]

    def combined(mem):
        v, used = text, 0
        for wi in range(len(wins)):
            f2 = [f for f in scan_file_text(unit, v) if f.addr == addr][0]
            k = [x for x in windows_in(f2, a.wmin) if x[0] == wins[wi][0]]
            if not k:
                continue
            v2 = helperize(v, f2, k[0][1], 'runscan_h%d' % wi, mem)
            if v2:
                v, used = v2, used + 1
        return v if used > 1 else None
    for wi, (key, sites) in enumerate(wins):
        v = helperize(text, fn, sites, 'runscan_h%d' % wi)
        if v is None:
            continue
        deck.append(('h%d' % wi, v))
        remake['h%d' % wi] = (lambda wi, sites: lambda mem: helperize(text, fn, sites, 'runscan_h%d' % wi, mem))(wi, sites)
        if noprobe:
            deck.append(('h%d-np' % wi, strip(v)))
            remake['h%d-np' % wi] = (lambda wi, sites: lambda mem: strip(helperize(text, fn, sites, 'runscan_h%d' % wi, mem)))(wi, sites)
    if len(wins) > 1:
        v = combined(set())
        if v:
            deck.append(('all', v))
            remake['all'] = combined
            if noprobe:
                deck.append(('all-np', strip(v)))
                remake['all-np'] = lambda mem: (lambda v: v and strip(v))(combined(mem))
    exact_addrs = [int(m.group(1), 16) for m in re.finditer(r'// ORIGINAL: 0x([0-9A-F]{8}) \S+ 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)', text)
                   if 'BYTE_EXACT' in m.group(2)]
    jobs = int(os.environ.get('FE_JOBS', '5'))
    base = int(os.environ.get('FE_PFX_BASE', '21'))
    lock = threading.Lock()
    fe.compile_cmd(unit)
    os.makedirs(fe.SCR, exist_ok=True)

    def lane(k):
        res = []
        for i in range(k, len(deck), jobs):
            key, t = deck[i]
            path = '%s/runscan%d.cpp' % (fe.SCR, base + k)
            mem = set()
            for _ in range(4):
                open(path, 'w', encoding='latin-1').write(t)
                c, err = fe.compile_unit(unit, path, str(base + k))
                miss = set(re.findall(r"error C2065: '(\w+)'", err or '')) if c is None else set()
                if not miss or key not in remake or miss <= mem:
                    break
                mem |= miss
                t2 = remake[key](mem)
                if t2 is None:
                    break
                t = t2
            if c is None:
                res.append((i, key, None, (err or '')[-300:]))
                continue
            r = fe.evaluate(c, t, addr)
            sym, lo, hi = fe.ident(t, addr)
            with lock:
                _, _, _, per, _ = im.analyse(c, sym, lo, hi, [], [])
            strs = {}
            dis = 0
            for nme, st in per.items():
                ri_, ro_, _ = im.strings(st)
                strs[nme] = (ri_, ro_)
                dis += sum(1 for x, y in zip(ri_, ro_) if x != y)
            inexact = set()
            if a.unit_exact:
                for ea in exact_addrs:
                    try:
                        if not fe.evaluate(c, t, ea)['exact']:
                            inexact.add(ea)
                    except Exception:
                        pass
            res.append((i, key, dict(diff=r['diff'], exact=r['exact'], dis=dis, strs=strs, inexact=inexact), None))
        return res
    out = []
    with ThreadPoolExecutor(jobs) as ex:
        for o in ex.map(lane, range(jobs)):
            out += o
    out.sort()
    b = out[0][2]
    print('%s 0x%08X: %d repeated windows, probe %s' % (unit, addr, len(wins), 'present' if noprobe else 'none'))
    for wi, (key, sites) in enumerate(wins):
        print('  h%d x%d: %s' % (wi, len(sites), ' / '.join(x[:60] for x in key)))
    if b:
        print('image      ' + ' '.join('%s %s' % (n.lstrip('?').split('@')[0], st[0]) for n, st in b['strs'].items() if st[0] != st[1])[:240])
    for i, key, r, err in out:
        if r is None:
            print('%-10s FAIL %s' % (key, err.strip().split('\n')[-1][:120]))
            continue
        moved = ['%s %s>%s' % (n.lstrip('?').split('@')[0], b['strs'].get(n, ('', ''))[1], s[1]) for n, s in r['strs'].items()
                 if b and s[1] != b['strs'].get(n, ('', ''))[1]]
        broke = sorted(r['inexact'] - b['inexact']) if b else []
        print('%-10s rows=%-5d dis=%-4d%s %s%s' % (key, r['diff'], r['dis'], ' EXACT' if r['exact'] else '',
              ' '.join(moved)[:200], (' broke=' + ','.join('%08X' % x for x in broke)) if broke else ''))


def scan_file_text(unit, text):
    path = os.path.join(R, 'build', 'runscan_tmp_%d.cpp' % os.getpid())
    os.makedirs(os.path.dirname(path), exist_ok=True)
    open(path, 'w', encoding='latin-1').write(text)
    try:
        return scan_file(path, None, None)
    finally:
        os.unlink(path)


if __name__ == '__main__':
    if '--try' in sys.argv:
        ap = argparse.ArgumentParser()
        ap.add_argument('unit')
        ap.add_argument('--try', dest='try_', required=True)
        ap.add_argument('--max', type=int, default=24)
        ap.add_argument('--unit-exact', action='store_true')
        ap.add_argument('--wmin', type=int, default=2, help='1: also single call statements repeated 3+ times')
        try_mode(ap.parse_args())
    else:
        main()
