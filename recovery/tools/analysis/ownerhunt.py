#!/usr/bin/env python3
"""ownerhunt.py <unit> <0xADDR> [--src ALT.cpp] [--maxlen N] [--lines A-B] [--kinds inline,static]
                [--top K] [--limit N] [--write K] [--repeat]

Missing inline OWNER search (MATCHING_GUIDE 2.8 "Missing owner, not missing budget" and
"Called early, expanded late ... the early sites are one inline level deeper").

The image's /Ob2 decisions of one function are its decision vector: for every inline callee the
expanded (E) / called (c) string over its sites (inlinemap.py), and the EH teardown depth per root
run of the unwind map (ehcensus.py). A site that the image calls early and expands late, or a
teardown that our source expands deeper than the image, means the original spent pass-1 / pass-2
room on an inline helper (an owner) that the decompiler-derived body flattened.

This tool parses the function body into statements, enumerates contiguous sibling ranges (a loop,
an if-arm, a run of statements) that can be lifted mechanically, wraps each into a file-local
inline helper `oh_owner(...)` (the locals and parameters the range uses are passed by reference;
`this` becomes `self`; implicit members are rewritten `self->x` from the compiler's undeclared-
identifier errors), compiles every variant (FE_PFX_BASE / FE_JOBS lanes) and ranks them by
  (EH distance, named call-multiset distance, decision distance, fe rows).
EH distance = sum over aligned root runs of |depth difference| (+8 per run count difference);
decision distance = sites whose E/c differ, image against ours, over every inline callee.
Ranges that contain return/goto/labels, a break/continue without their loop, or declare a local
used after the range are skipped (they need a hand-written owner).

--repeat: for the top candidates, other places in the tree that contain the range's first two
statements (whitespace-normalised): an owner used twice is strong evidence and gives it a name.
--write K writes candidate K (rank order) to the unit (a screen result, not a finished spelling:
rename the helper, tighten its parameters to what the image's code shape shows, tag it).
"""
import sys, os, re, glob, argparse, collections
from concurrent.futures import ThreadPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.dirname(HERE))
import fe  # noqa: E402
import inlinemap as im  # noqa: E402
import ehcensus as eh  # noqa: E402
import framescan as fs  # noqa: E402

R = fe.R
KW = set('if else for while do switch return break continue goto case default sizeof new delete '
         'typedef struct class union enum const static unsigned signed volatile register int char short long '
         'float double void bool true false this operator public private protected template typename '
         'throw try catch inline __forceinline extern'.split())
TYPEWORDS = set('const unsigned signed volatile register static struct class enum'.split())


# ---------------------------------------------------------------- lexing
def mask(t):
    """comments, strings and char literals blanked (same length, newlines kept)"""
    o = list(t)
    i, n = 0, len(t)
    while i < n:
        c = t[i]
        if t.startswith('//', i):
            j = t.find('\n', i)
            j = n if j < 0 else j
            for k in range(i, j):
                o[k] = ' '
            i = j
        elif t.startswith('/*', i):
            j = t.find('*/', i + 2)
            j = n if j < 0 else j + 2
            for k in range(i, j):
                if o[k] != '\n':
                    o[k] = ' '
            i = j
        elif c in '"\'':
            j = i + 1
            while j < n and t[j] != c:
                j += 2 if t[j] == '\\' else 1
            for k in range(i + 1, min(j, n)):
                if o[k] != '\n':
                    o[k] = ' '
            i = j + 1
        else:
            i += 1
    return ''.join(o)


def match_close(m, i):
    """m[i] is an opener; -> index of its closer"""
    op = m[i]
    cl = {'(': ')', '{': '}', '[': ']'}[op]
    d = 0
    for j in range(i, len(m)):
        if m[j] == op:
            d += 1
        elif m[j] == cl:
            d -= 1
            if d == 0:
                return j
    raise ValueError('unbalanced')


def skipws(m, i):
    while i < len(m) and m[i].isspace():
        i += 1
    return i


WORD = re.compile(r'[A-Za-z_]\w*')


class St:
    __slots__ = ('a', 'b', 'kind', 'blocks')

    def __init__(s, a, b, kind, blocks):
        s.a, s.b, s.kind, s.blocks = a, b, kind, blocks


def parse_stmt(m, i):
    i = skipws(m, i)
    if m[i] == '{':
        j = match_close(m, i)
        return St(i, j + 1, 'block', [parse_list(m, i + 1, j)])
    w = WORD.match(m, i)
    word = w.group(0) if w else ''
    if word in ('if', 'for', 'while', 'switch'):
        p = skipws(m, w.end())
        q = match_close(m, p)
        body = parse_stmt(m, q + 1)
        blocks = [[body] if body.kind != 'block' else body.blocks[0]]
        end = body.b
        if word == 'if':
            k = skipws(m, end)
            w2 = WORD.match(m, k)
            if w2 and w2.group(0) == 'else':
                eb = parse_stmt(m, w2.end())
                blocks.append([eb] if eb.kind != 'block' else eb.blocks[0])
                end = eb.b
        return St(i, end, word, blocks)
    if word == 'do':
        body = parse_stmt(m, w.end())
        k = skipws(m, body.b)
        w2 = WORD.match(m, k)
        p = skipws(m, w2.end())
        q = match_close(m, p)
        e = m.index(';', q) + 1
        return St(i, e, 'do', [[body] if body.kind != 'block' else body.blocks[0]])
    if word in ('case', 'default') or (w and re.match(r'\s*:(?!:)', m[w.end():w.end() + 3]) and word not in KW):
        e = i
        while True:
            e = m.index(':', e)
            if m[e + 1:e + 2] != ':' and m[e - 1:e] != ':':
                break
            e += 2
        return St(i, e + 1, 'label', [])
    # plain statement up to ';' at depth 0
    j = i
    while m[j] != ';':
        if m[j] in '({[':
            j = match_close(m, j)
        j += 1
    return St(i, j + 1, 'stmt', [])


def parse_list(m, a, b):
    out = []
    i = skipws(m, a)
    while i < b:
        if m[i] == ';':  # empty statement
            i = skipws(m, i + 1)
            continue
        s = parse_stmt(m, i)
        out.append(s)
        i = skipws(m, s.b)
    return out


def jumps_ok(m, s, in_loop, in_switch):
    """break/continue only inside a loop (or a switch, for break) that is part of the range"""
    if s.kind == 'stmt':
        seg = m[s.a:s.b]
        if re.search(r'\bcontinue\b', seg) and not in_loop:
            return False
        if re.search(r'\bbreak\b', seg) and not (in_loop or in_switch):
            return False
        return True
    il = in_loop or s.kind in ('for', 'while', 'do')
    isw = in_switch or s.kind == 'switch'
    if s.kind in ('if', 'switch', 'for', 'while'):  # the condition itself
        head = m[s.a:m.index('(', s.a)]
    return all(jumps_ok(m, x, il, isw) for blk in s.blocks for x in blk)


def sibling_lists(lst, depth=0):
    yield lst, depth
    for s in lst:
        for blk in s.blocks:
            yield from sibling_lists(blk, depth + 1)


# ---------------------------------------------------------------- function and its locals
def find_function(text, m, addr):
    mk = re.search(r'// ORIGINAL: 0x%08X ' % addr, text)
    if not mk:
        raise SystemExit('no ORIGINAL marker')
    # start of the comment block that holds the marker
    head = text.rfind('\n\n', 0, mk.start())
    k = mk.start()
    while True:
        p = text.rfind('\n', 0, k - 1) + 1
        if text[p:k].lstrip().startswith('//') and p < k:
            k = p
        else:
            break
    # definition: the first `[Class::]name(...) {` after the marker
    sym = re.match(r'// ORIGINAL: 0x%08X (\S+)' % addr, text[mk.start():]).group(1)
    sm = re.match(r'\?(\?[0-9A-Z_]|[A-Za-z_]\w*)@(?:([A-Za-z_]\w*)@)?@', sym)
    if sm and not sm.group(1).startswith('?'):
        name, cls = sm.group(1), sm.group(2)
    elif sm and sm.group(1) == '?1':
        name, cls = '~' + sm.group(2), sm.group(2)
    else:
        name, cls = sym.lstrip('_'), None
    pat = re.compile(r'(?:%s\s*::\s*)?%s\s*\(' % (re.escape(cls or ''), re.escape(name)) if cls
                     else r'(?<![\w:])%s\s*\(' % re.escape(name))
    for fm in pat.finditer(m, mk.end()):
        p = m.index('(', fm.start())
        q = match_close(m, p)
        j = skipws(m, q + 1)
        while m.startswith('const', j):
            j = skipws(m, j + 5)
        if m[j] == '{':
            sig_a = m.rfind('\n', 0, fm.start()) + 1
            break
    else:
        raise SystemExit('definition of %s not found' % name)
    return k, sig_a, j, match_close(m, j)


DECL = re.compile(r'\s*((?:(?:const|unsigned|signed|struct|class|enum|static|register|volatile)\s+)*)'
                  r'([A-Za-z_]\w*(?:::[A-Za-z_]\w*)*(?:<[^;<>]*>)?)(?=[\s\*&])\s*((?:[\*&]\s*|const\s+)*)'
                  r'([A-Za-z_]\w*)\b\s*(\[|=|;|,|\()')


def split_top(s, sep=','):
    out, d, cur = [], 0, ''
    for ch in s:
        if ch in '([{<':
            d += 1
        elif ch in ')]}>':
            d -= 1
        if ch == sep and d == 0:
            out.append(cur)
            cur = ''
        else:
            cur += ch
    out.append(cur)
    return out


def decl_of(stmt_text):
    """-> [(name, type_prefix, stars, dims)] if the statement is a declaration"""
    mm = DECL.match(stmt_text)
    if not mm:
        return []
    quals, base = mm.group(1), mm.group(2)
    if base in KW and base not in ('int', 'char', 'short', 'long', 'float', 'double', 'void', 'bool', 'unsigned', 'signed'):
        if base not in ('int', 'char', 'short', 'long', 'float', 'double', 'bool'):
            return []
    if mm.group(4) in KW:
        return []
    btype = (quals + base).strip()
    rest = stmt_text[mm.start(3):].rstrip().rstrip(';')
    out = []
    for d in split_top(rest):
        d = d.strip()
        dm = re.match(r'((?:[\*&]\s*|const\s+)*)([A-Za-z_]\w*)\s*((?:\[[^\]]*\]\s*)*)', d)
        if not dm:
            return out
        out.append((dm.group(2), btype, dm.group(1).replace(' ', '').replace('const', ''), dm.group(3).replace(' ', '')))
    return out


def params_of(sig):
    p = sig.index('(')
    q = match_close(sig, p)
    inner = sig[p + 1:q].strip()
    out = {}
    if not inner or inner == 'void':
        return out
    for x in split_top(inner):
        x = x.split('=')[0].strip()
        mm = re.match(r'(.*?)([A-Za-z_]\w*)\s*((?:\[[^\]]*\]\s*)*)$', x, re.S)
        if mm and mm.group(1).strip():
            t = mm.group(1).strip()
            stars = ''
            while t.endswith('*') or t.endswith('&'):
                stars = t[-1] + stars
                t = t[:-1].rstrip()
            out[mm.group(2)] = (t, stars.replace('&', ''), mm.group(3).replace(' ', ''))
    return out


def ptype(name, t, stars, dims):
    if dims:
        return '%s %s(&%s)%s' % (t, stars, name, dims)
    return '%s %s&%s' % (t, stars, name)


# ---------------------------------------------------------------- candidates
BAD = re.compile(r'\b(return|goto)\b')


def idents(s):
    out = []
    for mm in re.finditer(r'(?<![\w\.>:])([A-Za-z_]\w*)\b', s):
        a = mm.start()
        if s[max(0, a - 2):a] in ('->', '::') or s[a - 1:a] == '.':
            continue
        out.append(mm.group(1))
    return out


def candidates(text, m, body_a, body_b, sig, maxlen, line_rng):
    top = parse_list(m, body_a + 1, body_b)
    params = params_of(m[sig[0]:sig[1]])
    # locals: every declaration in the body, with its position
    decls = []
    for lst, _ in sibling_lists(top):
        for s in lst:
            if s.kind == 'stmt':
                for d in decl_of(m[s.a:s.b]):
                    decls.append((s.a, s.b) + d)
            elif s.kind == 'for':
                p = m.index('(', s.a)
                init = m[p + 1:m.index(';', p)] + ';'
                for d in decl_of(init):
                    decls.append((s.a, s.b) + d)
    out = []
    lineof = lambda pos: text.count('\n', 0, pos) + 1
    for lst, depth in sibling_lists(top):
        for i in range(len(lst)):
            for j in range(i, min(len(lst), i + maxlen)):
                a, b = lst[i].a, lst[j].b
                if line_rng and not (line_rng[0] <= lineof(a) and lineof(b) <= line_rng[1]):
                    continue
                seg = m[a:b]
                if BAD.search(seg) or any(s.kind == 'label' for s in lst[i:j + 1]):
                    continue
                if not all(jumps_ok(m, s, False, False) for s in lst[i:j + 1]):
                    continue
                if '(' not in seg:
                    continue
                if re.search(r'\bcase\b|\bdefault\s*:', seg) and 'switch' not in seg:
                    continue
                # locals declared inside the range must not be used after it
                inner = [d for d in decls if a <= d[0] < b]
                after = m[b:body_b]
                if any(re.search(r'\b%s\b' % re.escape(d[2]), after) for d in inner
                       if not any(x[2] == d[2] and x[0] >= b for x in decls)):
                    continue
                names = set(idents(seg))
                args = []
                for n in sorted(names, key=lambda n: seg.find(n)):
                    if any(d[2] == n for d in inner):
                        continue
                    prior = [d for d in decls if d[2] == n and d[0] < a]
                    if prior:
                        d = prior[-1]
                        args.append((n, ptype(n, d[3], d[4], d[5])))
                    elif n in params:
                        t, stars, dims = params[n]
                        args.append((n, ptype(n, t, stars, dims)))
                out.append(dict(a=a, b=b, depth=depth, n=j - i + 1, args=args,
                                kind=lst[i].kind if i == j else 'run', la=lineof(a), lb=lineof(b - 1)))
    return out


def make_variant(text, m, fstart, sig_a, cand, cls, kind, members=()):
    a, b = cand['a'], cand['b']
    seg = text[a:b]
    args = list(cand['args'])
    if cls and (re.search(r'\bthis\b', m[a:b]) or members):
        seg = re.sub(r'\bthis\b', 'self', seg)
        for x in members:
            if '::' in x:  # C2352 'A::f': qualified or plain call of a member function
                q, f = x.rsplit('::', 1)
                seg = re.sub(r'(?<![\w\.>:])(?:%s::)?%s\s*\(' % (re.escape(q), re.escape(f)),
                             lambda mm: 'self->' + mm.group(0), seg)
            else:
                seg = re.sub(r'(?<![\w\.>:])\b%s\b' % re.escape(x), 'self->' + x, seg)
        args = [('this', '%s *self' % cls)] + args
    if kind == 'value':  # read-only scalars by value: the inline's parameter homes
        ro = []
        for n, ty in args:
            assigned = re.search(r'(?<![\w\.>])%s\s*(?:[-+*/%%&|^]|<<|>>)?=(?!=)|(?:\+\+|--)\s*%s\b|\b%s\s*(?:\+\+|--)|&\s*%s\b'
                                 % ((re.escape(n),) * 4), m[a:b])
            ro.append((n, ty if assigned or '(&' in ty or n == 'this' else ty.replace('&' + n, n)))
        args = ro
    kw = {'inline': 'static inline void', 'value': 'static inline void', 'static': 'static void',
          'force': 'static __forceinline void'}[kind]
    helper = '%s oh_owner(%s)\n{\n    %s\n}\n\n' % (kw, ', '.join(t for _, t in args), seg)
    call = 'oh_owner(%s);' % ', '.join(n for n, _ in args)
    # Insert right before the definition line: the marker block may sit inside a /* */ comment.
    return text[:sig_a] + helper + text[sig_a:a] + call + text[b:], len(helper)


# ---------------------------------------------------------------- measuring
FAST = False  # --fast: call multiset and EH only (inlinemap's parse is slow on giants)
SIGS = ('??1StringList=?remove_all@StringStruct,??1StringStruct',)


def eh_runs(fi):
    if not isinstance(fi, dict):
        return []
    s = []
    for v in fi['par']:
        if v == -1 or not s:
            s.append(1)
        else:
            s[-1] += 1
    return s


def eh_dist(I, O):
    a, b = eh_runs(I), eh_runs(O)
    d = 8 * abs(len(a) - len(b))
    return d + sum(abs(x - y) for x, y in zip(a, b))


def measure(c, text, addr):
    sym, lo, hi = fe.ident(text, addr)
    r = fe.evaluate(c, text, addr)
    if FAST:
        img, ours, per = im.img_stream(lo, hi), im.our_stream(c, sym), {}
    else:
        img, ours, rows, per, cands = im.analyse(c, sym, lo, hi, (), SIGS)
    known = set(im.key(s) for s in im.A2S.values())
    keep = lambda n: n in known or n == 'oh_owner' or 'oh_owner' in n
    ci = collections.Counter(n for _, n in img if keep(n))
    co = collections.Counter(n for _, n in ours if keep(n))
    calls = sum(abs(ci[n] - co[n]) for n in set(ci) | set(co))
    vec = {}
    dec = 0
    for n, sites in per.items():
        ri = ''.join(s[0][2] if s[0] else '.' for s in sites)
        ro = ''.join(s[1][2] if s[1] else '.' for s in sites)
        vec[n] = (ri, ro)
        dec += sum(1 for x, y in zip(ri, ro) if x != y)
    I, O = eh.img_fi(lo), eh.our_fi(c, sym)
    fi_, fo_ = framedist(c, sym, lo, hi)
    return dict(frame=fi_, slots=fo_, eh=eh_dist(I, O), runs=(eh_runs(I), eh_runs(O)), calls=calls, dec=dec, rows=r.get('diff', 9999),
                exact=r.get('exact'), vec=vec,
                cdiff=sorted((n, ci[n], co[n]) for n in set(ci) | set(co) if ci[n] != co[n]))


def framedist(c, sym, lo, hi):
    """-> (|frame size difference|, frame slots on one side only): the frame rung and the
    parameter-home evidence (an owner's by-value parameters get fresh slots at its entry)"""
    try:
        s = c.function(sym)
        sec = c.sections[s.section - 1]
        I = fs.insns(fe.imgbytes(lo, hi), lo)
        O = fs.insns(sec.data, 0)
        a, b = fs.frame(I), fs.frame(O)
        si = set(fs.slots(I, a[0], a[2], a[3]))
        so = set(fs.slots(O, b[0], b[2], b[3]))
        return abs(a[0] - b[0]), len(si ^ so)
    except Exception:  # noqa: BLE001
        return 0, 0


def score(x):
    return (x['eh'], x['calls'], x['dec'], x['frame'], x['slots'], x['rows'])


def run_one(unit, addr, text, m, fstart, sig_a, cand, cls, kind, pfx, scr):
    members = []
    for _ in range(6):
        vt, hl = make_variant(text, m, fstart, sig_a, cand, cls, kind, members)
        path = '%s/oh_%s.cpp' % (scr, pfx)
        open(path, 'w', encoding='latin-1').write(vt)
        c, err = fe.compile_unit(unit, path, pfx)
        if c is not None:
            try:
                return measure(c, vt, addr), vt
            except Exception as e:  # noqa: BLE001
                return dict(error='measure: %s' % e), vt
        und = re.findall(r"error C2065: '(\w+)' : undeclared identifier", err or '')
        und += re.findall(r"error C3861: '(\w+)'", err or '')
        und += re.findall(r"error C2352: '(\w+::\w+)'", err or '')
        und += re.findall(r"error C2597: illegal reference to (?:data )?member '\w+::(\w+)'", err or '')
        new = [u for u in und if u not in members]
        if not cls or not new:
            return dict(error=(err or '')[:200]), vt
        members += new
    return dict(error='fixup limit'), None


def repeats(text_by_file, seg_text, here):
    sts = [re.sub(r'\s+', ' ', x).strip() for x in re.split(r'(?<=;)', seg_text) if x.strip()]
    if len(sts) < 2:
        return []
    key = ' '.join(sts[:2])
    hits = []
    for p, t in text_by_file.items():
        norm = re.sub(r'\s+', ' ', t)
        k = norm.find(key)
        while k >= 0:
            ln = t.count('\n', 0, len(re.sub(r'\s+', ' ', t[:0])))  # approximate
            hits.append(os.path.relpath(p, R))
            k = norm.find(key, k + 1)
    c = collections.Counter(hits)
    if c.get(here, 0) <= 1:
        c.pop(here, None)
    return sorted(c.items())


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('unit')
    ap.add_argument('addr')
    ap.add_argument('--src')
    ap.add_argument('--maxlen', type=int, default=6)
    ap.add_argument('--lines')
    ap.add_argument('--kinds', default='inline')
    ap.add_argument('--top', type=int, default=15)
    ap.add_argument('--limit', type=int, default=400)
    ap.add_argument('--mindepth', type=int, default=0)
    ap.add_argument('--write', type=int)
    ap.add_argument('--repeat', action='store_true')
    ap.add_argument('--moved', action='store_true', help='rank only the candidates that move a decision')
    ap.add_argument('--vec', action='store_true', help='print the decision vectors of the top candidates')
    ap.add_argument('--fast', action='store_true', help='skip the per-site decision vector (giant functions)')
    a = ap.parse_args()
    global FAST
    FAST = a.fast
    addr = int(a.addr, 16)
    unit = a.unit
    src = a.src or R + '/' + unit
    text = open(src, encoding='latin-1').read()
    m = mask(text)
    fstart, sig_a, body_a, body_b = find_function(text, m, addr)
    sig = (sig_a, body_a)
    sigtext = m[sig_a:body_a]
    cm = re.search(r'([A-Za-z_]\w*)::~?[A-Za-z_]\w*\s*\(', sigtext)
    cls = cm.group(1) if cm else None
    line_rng = tuple(int(x) for x in a.lines.split('-')) if a.lines else None
    cands = [c for c in candidates(text, m, body_a, body_b, sig, a.maxlen, line_rng) if c['depth'] >= a.mindepth]
    if len(cands) > a.limit:  # prefer coherent single statements (loops, ifs) and short runs
        cands.sort(key=lambda c: (c['n'] > 1, c['n'], -(c['b'] - c['a'])))
        cands = cands[:a.limit]
        cands.sort(key=lambda c: c['a'])
    pfx0 = int(os.environ.get('FE_PFX_BASE', '21'))
    jobs = int(os.environ.get('FE_JOBS', '5'))
    scr = fe.R + '/build/recovery/fe/ownerhunt'
    os.makedirs(scr, exist_ok=True)
    fe.compile_cmd(unit)
    c0, err = fe.compile_unit(unit, src, pfx0)
    if c0 is None:
        raise SystemExit(err)
    base = measure(c0, text, addr)
    print('%s  base: eh %d calls %d dec %d frame %d slots %d rows %d   EH runs image %s ours %s' % (
        a.addr, base['eh'], base['calls'], base['dec'], base['frame'], base['slots'], base['rows'], base['runs'][0], base['runs'][1]))
    for n, (ri, ro) in base['vec'].items():
        if ri != ro:
            print('   %-28s image %s\n   %-28s ours  %s' % (n[:28], ri, '', ro))
    print('   call diff', base['cdiff'][:12])
    work = [(c, k) for c in cands for k in a.kinds.split(',')]
    print('%d candidate ranges x %d kinds = %d variants' % (len(cands), len(a.kinds.split(',')), len(work)))
    lanes = [work[i::jobs] for i in range(jobs)]
    res = []

    def lane(k):
        out = []
        for cand, kind in lanes[k]:
            r, vt = run_one(unit, addr, text, m, fstart, sig_a, cand, cls, kind, pfx0 + k, scr)
            out.append((cand, kind, r, vt))
        return out
    with ThreadPoolExecutor(jobs) as ex:
        for o in ex.map(lane, range(jobs)):
            res += o
    ok = [x for x in res if 'error' not in x[2]]
    bad = len(res) - len(ok)
    ok.sort(key=lambda x: score(x[2]) + (x[0]['n'],))
    print('%d compiled, %d failed' % (len(ok), bad))
    for x in [x for x in res if 'error' in x[2]][:3]:
        print('   fail L%d: %s' % (x[0]['la'], x[2]['error'][:150].replace('\n', ' | ')))
    bs = score(base)
    sig0 = (base['runs'][1], tuple(sorted(base['vec'].items())))
    moved = [x for x in ok if (x[2]['runs'][1], tuple(sorted(x[2]['vec'].items()))) != sig0]
    print('%d candidates move the decision vector or the EH runs; best EH %s\n' % (
        len(moved), min([x[2]['eh'] for x in ok] or [None])))
    if a.moved:
        ok = moved
    tbf = None
    if a.repeat:
        tbf = {p: open(p, encoding='latin-1').read() for p in glob.glob(R + '/src/**/*.cpp', recursive=True)}
    for rank, (cand, kind, r, vt) in enumerate(ok[:a.top]):
        seg = ' '.join(text[cand['a']:cand['b']].split())
        mark = '<' if score(r) < bs else ('=' if score(r) == bs else ' ')
        print('%2d %s eh %3d calls %3d dec %3d frame %3d slots %3d rows %5d  L%d-%d d%d %-5s %s  %s' % (
            rank, mark, r['eh'], r['calls'], r['dec'], r['frame'], r['slots'], r['rows'], cand['la'], cand['lb'], cand['depth'], cand['kind'], kind,
            seg[:110]))
        if a.vec:
            print('     EH runs ours', r['runs'][1], ' call diff', r['cdiff'][:8])
            for n, (ri, ro) in r['vec'].items():
                if ri != ro:
                    print('     %-26s %s / %s' % (n[:26], ri, ro))
        if tbf is not None:
            rp = repeats(tbf, text[cand['a']:cand['b']], unit)
            if rp:
                print('     recurs in', rp)
    if a.write is not None:
        cand, kind, r, vt = ok[a.write]
        open(src, 'w', encoding='latin-1').write(vt)
        print('wrote candidate', a.write)


if __name__ == '__main__':
    main()
