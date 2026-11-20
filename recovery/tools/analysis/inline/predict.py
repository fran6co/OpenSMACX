#!/usr/bin/env python3
"""predict.py <unit> <0xADDR>[,..] [--show] [--leads] [--scale K] [--free N] [--json F]
          | --tree [unit ..] [--from FEALL.json] [--json F]

Rule-8 /Ob2 decision predictor (MATCHING_GUIDE 2.8 rule 8).

Site tree: the unit compiled at /Od (no inlining, no tail merging) with /FAs: every function the unit
defines is listed with its call sites in source order, scope-exit destructor calls included one per exit.
A callee defined in the listing is an inline candidate (header / `inline` body = explicit; a marked .cpp
function = auto, under the per-callee cap).

Costs: a body's own size is its /Od instruction count without the frame code (`size`); its charge in
if-units is 0 when size <= FREE (free inline, 2.8 rule 3), else size / SCALE + 0.74. One if-unit
`if (ga[k] == 1000) mk(k);` is 7 /Od instructions. Budget: S of the caller (rule 2) is fitted.

Walk (rule 8): one depth-first pass in source order. In a body with budget S, site j expands when its
charge <= S - spent; its body then gets (S - spent) / (1 + later_j), later_j = inline sites after j in
the same body. Depth cap 8.

Decisions are compared as named call multisets (key = mangled prefix before '@@', as inlinemap.py):
`ours` from the real /Og compile, `image` from the image span. For each function the tool fits the S
range that reproduces ours, reports whether the image's counts lie on the same S line, and with
--leads searches the single changes that make the model give the image's counts at ours' S:
a site's charge +-d (`cost@site`), one more / one fewer later site in a body (`later@body`), S itself.
A lead names the site (caller line or callee body) whose spend differs from the original.
Implicit member and base destructor calls inside a destructor body are ordinary sites of that body, in
destruction order (members in reverse declaration order, then bases); no exemption (2.8 rule 9).

Teardown cross-jumping (2.8 rule 9): every caller-level destructor site is flattened into the calls it
leaves, each tagged with the EH state stores written before it (state -1 of an expanded body = the
caller's state at the call; every other state number is private to that copy, so it never matches
another site) and the destroyed object. Walking the teardowns from the last back, a teardown merges the
longest identical suffix with an earlier-kept host (a call whose store differs still merges, nothing
before it); one that matches no host keeps everything and becomes a host. terraform: sites 1-27 jump
into site 28, site 29 into the closing site 30, as in the image.

--ceiling also fits rule 7's total-size ceiling T (own /Od size plus every body expanded so far, free
ones too). It is one more free parameter: use it on ceiling-bound giants (analysis/ceilscan.py).
"""
import sys, os, re, json, argparse, subprocess, collections
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
sys.path.insert(0, os.path.dirname(os.path.dirname(HERE)))
import fe  # noqa: E402
import inlinemap as im  # noqa: E402
import callees as _callees  # noqa: E402
im.CRT.update(_callees.CRT)   # name every CRT callee the image calls (abs, rand, ...)

SCR = os.path.join(fe.SCR, 'predict')
FRAME = re.compile(r'^(push\s+(ebp|ebx|esi|edi|ecx|-1|\$L\d+)|pop\s+(ebp|ebx|esi|edi)|mov\s+ebp, esp|mov\s+esp, ebp|'
                   r'sub\s+esp, \d+|ret\b|leave|.*fs:__except_list.*|mov\s+DWORD PTR _this\$\[ebp\], ecx|'
                   r'call\s+__chkstk|mov\s+eax, \d+\s*;.*|mov\s+(DWORD|BYTE) PTR __\$EHRec\$.*)')
OVERHEAD = 0.74


def compile_listing(unit, srcpath, pfx, od):
    cmd = fe.compile_cmd(unit)
    if not cmd:
        raise SystemExit('unit not in compile_commands: ' + unit)
    os.makedirs(SCR, exist_ok=True)
    base = '%s/%s_%s_%s' % (SCR, os.path.basename(unit)[:-4], 'od' if od else 'og', pfx)
    out = []
    for t in cmd:
        if t.startswith('/Fo'):
            t = '/Fo' + base + '.obj'
        elif t.startswith('/Fd') or t.startswith('--repro-root') or t == '/W4':
            continue
        elif od and t in ('/Og', '/Ot', '/Ob2', '/Gs', '/Oy-'):
            continue
        elif t == fe.R + '/' + unit:
            t = srcpath
        out.append(t)
    out[1:1] = ['/FAs', '/Fa' + base + '.asm'] + (['/Od'] if od else [])
    if os.path.exists(base + '.asm'):
        os.remove(base + '.asm')
    p = subprocess.run(out, cwd=fe.R, env=dict(os.environ, VC6_PREFIX='%s/.opensmacx/wine/worker-%s' % (fe.MAIN, pfx)),
                       capture_output=True, text=True)
    if not os.path.exists(base + '.asm'):
        raise SystemExit('compile failed\n' + (p.stdout + p.stderr)[-3000:])
    return open(base + '.asm', errors='replace').read().replace('\r', '')


def procs(asm):
    """-> {mangled: dict(sites=[(line, callee)], size=n, file=...)}; text$x funclets are left out."""
    res = {}
    cur_file = None
    for m in re.finditer(r'^; File ([^\n]+)\n|^(\S+) PROC NEAR[^\n]*\n(.*?)^\2 ENDP', asm, re.S | re.M):
        if m.group(1):
            cur_file = m.group(1).strip()
            continue
        name, body = m.group(2), m.group(3)
        body = body.split('_TEXT\tENDS')[0]
        sites, stores, recv, size, line, st, rc = [], [], [], 0, 0, (), None
        for l in body.split('\n'):
            ml = re.match(r'^; (\d+)\s+:', l)
            if ml:
                line = int(ml.group(1))
                continue
            if not l.startswith('\t') or l.startswith('\t;'):
                continue
            s = l.strip().split(';')[0].strip()
            if not s or s.endswith('ENDS') or s.endswith('SEGMENT'):
                continue
            ms = re.match(r'mov\s+(?:DWORD|BYTE) PTR __\$EHRec\$\[ebp\+8\], (-?\d+)', s)
            if ms:
                st = st + (int(ms.group(1)),)
            mr = re.match(r'(?:lea|mov)\s+ecx, (.*)', s)
            if mr:
                rc = mr.group(1)
            mc = re.match(r'call\s+(\S+)', s)
            if mc and not mc.group(1).startswith(('DWORD', '__chkstk')):
                sites.append((line, mc.group(1)))
                stores.append(st)    # EH state stores since the previous call (cross-jump boundaries)
                recv.append(rc)      # the receiver (destroyed object) of a caller-level teardown
                st, rc = (), None
            if not FRAME.match(s):
                size += 1
        res[name] = dict(sites=sites, stores=stores, recv=recv, size=size, file=cur_file)
    return res


def undname(names):
    p = subprocess.run(['llvm-undname'] + list(names), capture_output=True, text=True)
    out, lines = {}, [l for l in p.stdout.split('\n')]
    i = 0
    while i < len(lines) - 1:
        if lines[i] in names:
            out[lines[i]] = lines[i + 1]
            i += 2
        else:
            i += 1
    return out


def probe_call(mangled, dem):
    """-> (params text, call statement) for a probe caller, or None"""
    m = re.match(r'^(?:(public|protected|private): )?(?:(virtual|static) )?(.*?)(__cdecl|__thiscall|__stdcall|__fastcall) '
                 r'(.+?)\((.*)\)( const)?$', dem)
    if not m or m.group(1) in ('protected', 'private'):
        return None
    name, args = m.group(5), m.group(6)
    if '`' in name or 'operator' in name:
        return None
    types = [] if args in ('void', '') else [t.strip() for t in re.split(r',(?![^<]*>)', args)]
    if any(t == '...' or '(' in t for t in types):
        return None
    params = ['%s __a%d' % (t, i) for i, t in enumerate(types)]
    actual = ', '.join('__a%d' % i for i in range(len(types)))
    if '::' in name and m.group(2) != 'static' and m.group(4) == '__thiscall':
        cls, meth = name.rsplit('::', 1)
        if meth.startswith('~'):
            # an explicit destructor call is never expanded: a local object's closing teardown is the site
            # (the default constructor, when inline, is charged with it)
            return '', '{ %s __lo; __cpg[62] = 0; }' % cls
        params.insert(0, '%s%s *__o' % ('const ' if m.group(7) else '', cls))
        call = '__o->%s::%s(%s);' % (cls, meth, actual)
    else:
        call = '%s(%s);' % (name, actual)
    return ', '.join(params), call


def ref_inline(n):
    ifs = ' '.join('if (__cpg[%d] == 1000) __cpu(%d);' % (i, i) for i in range(n))
    return 'inline void __cpr%d(int k) { __cpu(k); %s }' % (n, ifs)


def measure_costs(unit, src, text, callees, pfx):
    """charge of each callee in if-units from probe callers at the B0 floor: `__cprN(1); <call>;` expands the
    call while charge <= S0 - (N + 0.74); a binary search over N gives charge = S0 - 0.74 - N* - 0.5.
    -> {mangled: charge | None (no probe possible)}"""
    import hashlib
    key = hashlib.md5((text + '|'.join(sorted(callees))).encode()).hexdigest()[:12]
    cache = '%s/costs_%s_%s.json' % (SCR, os.path.basename(unit)[:-4], key)
    if os.path.exists(cache):
        return json.load(open(cache))
    dem = undname(callees)
    calls = {}
    for c in callees:
        pc = probe_call(c, dem.get(c, ''))
        if pc:
            calls[c] = pc
    lo = {c: -1 for c in calls}   # largest N known to expand
    hi = {c: 53 for c in calls}   # smallest N known to refuse
    alive = dict(calls)
    while True:
        todo = {c: (lo[c] + hi[c]) // 2 for c in alive if hi[c] - lo[c] > 1}
        if not todo:
            break
        good, ok = probe_round(unit, src, text, alive, todo, pfx)
        for c in list(alive):
            if c not in todo:
                continue
            if c not in ok:            # probe did not compile
                alive.pop(c)
                lo[c] = None
                continue
            if c in good:
                lo[c] = todo[c]
            else:
                hi[c] = todo[c]
    S0 = 53.5
    res = {}
    for c in callees:
        if c not in calls or lo.get(c) is None:
            res[c] = None
        elif lo[c] == -1:
            res[c] = S0 + 1          # refused alone at the floor
        else:
            res[c] = max(0.0, S0 - OVERHEAD - (lo[c] + 0.5)) if lo[c] < 52 else 0.0
    json.dump(res, open(cache, 'w'), indent=0)
    return res


def probe_round(unit, src, text, calls, todo, pfx):
    """-> (callees whose probe site expanded, callees whose probe compiled)"""
    parts = ['\nint __cpg[64]; void __cpu(int);']
    parts += [ref_inline(n) for n in sorted(set(todo.values())) if n > 0]
    names = {}
    for i, (c, n) in enumerate(sorted(todo.items())):
        params, call = calls[c]
        names[i] = c
        parts.append('void __cp%d(%s) { %s %s __cpg[63] = 1; }' % (i, params, '__cpr%d(1);' % n if n > 0 else '', call))
    good, ok = set(), set()
    body = text + '\n'.join(parts) + '\n'
    path = '%s/cp_%s_%s.cpp' % (SCR, os.path.basename(unit)[:-4], pfx)
    open(path, 'w', encoding='latin-1').write(body)
    try:
        P = procs(compile_listing(unit, path, pfx, False))
    except SystemExit as e:
        if len(names) == 1:
            return set(), set()
        ks = sorted(todo)
        h = len(ks) // 2
        g1, o1 = probe_round(unit, src, text, calls, {k: todo[k] for k in ks[:h]}, pfx)
        g2, o2 = probe_round(unit, src, text, calls, {k: todo[k] for k in ks[h:]}, pfx)
        return g1 | g2, o1 | o2
    for name, d in P.items():
        m = re.match(r'\?__cp(\d+)@@', name)
        if not m or int(m.group(1)) not in names:
            continue
        c = names[int(m.group(1))]
        ok.add(c)
        if not any(x == c for _, x in d['sites']):
            good.add(c)
    return good, ok


def calls_by_key(asm_procs, sym):
    return collections.Counter(im.key(c) for _, c in asm_procs[sym]['sites'])


class Model:
    def __init__(self, P, unit_text, scale=7.0, free=13, cap=7 * 7, overrides=None):
        self.P = P
        self.scale, self.free, self.cap = scale, free, cap
        self.over = overrides or {}       # callee mangled -> charge override
        self.site_over = {}               # (body, index) -> charge delta
        self.later_over = {}              # body -> {index: extra later count}
        self.merge = os.environ.get('PREDICT_MERGE', '1') == '1'
        self.ceiling = False              # --ceiling: fit T as well
        self.T = None                     # rule 7 total-size ceiling in /Od instructions (fitted), None = off
        self.total = 0
        marked = set(re.findall(r'// ORIGINAL: 0x[0-9A-F]{8} (\S+)', unit_text))
        marked |= set(re.findall(r'// symbol\s+(\S+)', unit_text))
        self.auto = {n for n in P if n in marked}

    def inline(self, callee):
        if callee not in self.P or callee.startswith(('??_G', '??_E', '??_7')):
            return False
        if callee in self.auto:
            return self.P[callee]['size'] <= self.cap
        return True

    def charge(self, callee):
        if callee in self.over:
            return self.over[callee]
        sz = self.P[callee]['size']
        return 0.0 if sz <= self.free else sz / self.scale + OVERHEAD

    def walk(self, body, S, depth=0, path=(), out=None):
        """-> spent; appends (path+(j,), callee, 'E'|'c', depth) to out"""
        sites = [c for _, c in self.P[body]['sites'] if self.inline(c)] if body in self.P else []
        allsites = self.P[body]['sites'] if body in self.P else []
        idx = [j for j, (_, c) in enumerate(allsites) if self.inline(c)]
        spent = 0.0
        lo = self.later_over.get(body, {})
        for q, j in enumerate(idx):
            c = allsites[j][1]
            later = len(idx) - q - 1 + sum(v for k, v in lo.items() if k > j)
            ch = self.charge(c) + self.site_over.get((body, j), 0.0)
            # rule 7 ceiling: the caller's own size plus every body expanded so far (free ones too)
            over = self.T is not None and self.total + self.P[c]['size'] > self.T
            if depth < 8 and not over and ch <= S - spent + 1e-9:
                spent += ch
                self.total += self.P[c]['size']
                sub = (S - spent) / (1 + later)
                out.append((path + (j,), c, 'E', depth))
                k = len(out) - 1
                st = self.walk(c, sub, depth + 1, path + (j,), out)
                spent += st
                out[k] = out[k] + (ch, st, sub, allsites[j][0])
            else:
                out.append((path + (j,), c, 'c', depth))
        return spent

    def counts(self, sym, S):
        """-> Counter of called keys (sites refused at a reachable position + non-inline calls)"""
        out = []
        self.total = self.P[sym]['size']
        self.walk(sym, S, 0, (), out)
        exp = {x[0] for x in out if x[2] == 'E'}
        cnt = collections.Counter()

        def flat(body, path, entry, per=None):
            """-> [(key, boundary)] of the calls a site leaves in the code; boundary = the EH state stores
            written since the previous call. In an expanded body, state -1 (no member alive) is the
            caller's state at the call (`entry`); every other state number is private to that copy."""
            toks, cur = [], entry
            d = self.P.get(body, {'sites': [], 'stores': []})
            for j, (_, c) in enumerate(d['sites']):
                p = path + (j,)
                raw = d.get('stores', [()] * len(d['sites']))[j]
                bd = []
                for v in raw:
                    if not path:
                        cur = ('top', v)
                    else:
                        cur = entry if v == -1 else ('u', p, v)
                    bd.append(cur)
                bd = tuple(bd)
                if p in exp:
                    sub = flat(c, p, cur)
                    if sub:
                        sub[0] = (sub[0][0], bd + sub[0][1])
                else:
                    sub = [(im.key(c), bd)]
                toks += sub
                if per is not None:
                    obj = d['recv'][j] if 'recv' in d else None
                    sub = [(k, b, obj) for k, b in sub]   # another object's teardown never merges
                    per.append((c, sub))
            return toks

        def merged(a, b):
            """VC6 cross-jumping: the longest identical instruction suffix. A call whose preceding state
            store differs still merges (the jumper keeps its store and jumps to the call), nothing before."""
            n = 0
            while n < min(len(a), len(b)):
                x, y = a[-1 - n], b[-1 - n]
                if x[0] != y[0] or x[2] != y[2]:
                    break
                n += 1
                if x[1] != y[1]:
                    break
            return n

        # scope-exit teardowns (destructor sites at the caller's level) all end in an epilogue. Walked from
        # the last one back: a teardown whose suffix matches an earlier-kept host's tail jumps into it
        # (only the unmatched calls stay), one that matches none falls through into an epilogue of its own
        # and becomes a host (terraform: sites 1-27 into site 28, site 29 into the closing site 30).
        per = []
        flat(sym, (), ('top', -1), per)
        hosts = []
        keep = [None] * len(per)
        for q in range(len(per) - 1, -1, -1):
            c, toks = per[q]
            if self.merge and c.startswith('??1') and toks:
                best = max([merged(toks, h) for h in hosts] or [0])
                if best == 0:
                    hosts.append(toks)
                keep[q] = toks[:len(toks) - best]
            else:
                keep[q] = toks
        for toks in keep:
            for t in toks:
                cnt[t[0]] += 1
        return cnt, out


def distance(a, b, keys, merged=()):
    return sum(abs(a.get(k, 0) - b.get(k, 0)) for k in keys if k not in merged)


def fit(model, sym, target, keys, grid):
    """-> (best distance, [S values reaching it], counts at the first)"""
    best, Ss, first = None, [], None
    for S in grid:
        c, _ = model.counts(sym, S)
        d = distance(c, target, keys)
        if best is None or d < best:
            best, Ss, first = d, [S], c
        elif d == best:
            Ss.append(S)
    return best, Ss, first


def ranges(v, step):
    out = []
    for x in v:
        if out and abs(x - out[-1][1] - step) < 1e-6:
            out[-1][1] = x
        else:
            out.append([x, x])
    return ' '.join('%g' % a if a == b else '%g-%g' % (a, b) for a, b in out)


def analyse(unit, addr, text, Pod, Pog, model, leads=False, show=False):
    sym, lo, hi = fe.ident(text, addr)
    model.T = None
    if sym not in Pod or sym not in Pog:
        return dict(addr='%08X' % addr, sym=sym, error='not in listing')
    img = collections.Counter(n for _, n in im.img_stream(lo, hi))
    ours = calls_by_key(Pog, sym)
    # keys the model can move: inline callees reachable from sym
    reach, stack = set(), [sym]
    while stack:
        b = stack.pop()
        for _, c in Pod.get(b, {'sites': []})['sites']:
            if model.inline(c) and c not in reach:
                reach.add(c)
                stack.append(c)
    # callees the model moves: the inline candidates and everything their bodies call
    keys = sorted({im.key(c) for c in reach} | {im.key(c) for b in reach if b in Pod for _, c in Pod[b]['sites']})
    grid = [float(os.environ['PREDICT_S'])] if os.environ.get('PREDICT_S') else [x * 0.5 for x in range(0, 1201)]
    d_o, S_o, c_o = fit(model, sym, ours, keys, grid)
    if model.ceiling and d_o:
        # rule 7: fit the total-size ceiling T too (coarse T grid, then S at the best T)
        model.T = None
        _, full = model.counts(sym, grid[-1])
        tmax = model.total
        lo_t = Pod[sym]['size']
        best = (d_o, None)
        for q in range(41):
            model.T = lo_t + (tmax - lo_t) * q / 40.0
            d, Ss, _ = fit(model, sym, ours, keys, grid[::4])
            if d < best[0]:
                best = (d, model.T)
        model.T = best[1]
        if model.T is not None:
            d_o, S_o, c_o = fit(model, sym, ours, keys, grid)
    # the image target in model terms: ours' model counts moved by the image's difference from ours, so the
    # site counts that tail merging removes on both sides cancel (an offset the model does not know)
    tgt = collections.Counter({k: c_o[k] + img[k] - ours[k] for k in keys})
    d_i, S_i, c_i = fit(model, sym, tgt, keys, grid)
    r = dict(addr='%08X' % addr, sym=sym, keys=len(keys), sites=len(reach), T=model.T,
             ours_fit=d_o, ours_S=ranges(S_o, 0.5), img_fit=d_i, img_S=ranges(S_i, 0.5),
             diff={k: (img[k], ours[k]) for k in keys if img[k] != ours[k]})
    if show:
        S = S_o[len(S_o) // 2]
        _, out = model.counts(sym, S)
        r['walk_S'] = S
        r['walk'] = ['%s%s %s %s%s' % ('  ' * x[3], '.'.join(map(str, x[0])), x[2], x[1],
                                          '  line %d own %.2f subtree %.2f child budget %.2f' % (x[7], x[4], x[5], x[6]) if len(x) > 4 else '')
                     for x in out]
        r['model_vs'] = {k: (c_o[k], ours[k], img[k]) for k in keys if c_o[k] != ours[k] or ours[k] != img[k]}
    if leads and r['diff']:
        r['leads'] = find_leads(model, sym, tgt, ours, keys, S_o, reach)
    return r


def find_leads(model, sym, img, ours, keys, S_o, reach):
    """single changes that give the image counts at ours' S (each S of ours' range is tried)"""
    found = []
    Ss = S_o[::max(1, len(S_o) // 8)]
    bodies = [sym] + sorted(reach)
    base_d = min(distance(model.counts(sym, S)[0], img, keys) for S in Ss)
    for body in bodies:
        if body not in model.P:
            continue
        idx = [j for j, (_, c) in enumerate(model.P[body]['sites']) if model.inline(c)]
        for j in idx:
            for dlt in (-4, -2, -1, -0.5, 0.5, 1, 2, 4):
                model.site_over[(body, j)] = dlt
                d = min(distance(model.counts(sym, S)[0], img, keys) for S in Ss)
                del model.site_over[(body, j)]
                if d < base_d:
                    found.append((d, 'cost', body, j, dlt))
        for j in [-1] + idx:
            for dlt in (1, -1):
                model.later_over[body] = {j: dlt}
                d = min(distance(model.counts(sym, S)[0], img, keys) for S in Ss)
                model.later_over.pop(body)
                if d < base_d:
                    found.append((d, 'later', body, j, dlt))
    found.sort(key=lambda x: (x[0], abs(x[4])))
    out = []
    for d, kind, body, j, dlt in found[:25]:
        line, c = model.P[body]['sites'][j] if j >= 0 else (0, '-')
        where = 'own body' if body == sym else body
        out.append('%-5s d=%d  %s site %d (line %d, %s) %+g  in %s' % (kind, d, '' if kind == 'cost' else 'after', j, line, im.key(c), dlt, where))
    return dict(base=base_d, leads=out)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('unit', nargs='?')
    ap.add_argument('addrs', nargs='?')
    ap.add_argument('--show', action='store_true')
    ap.add_argument('--leads', action='store_true')
    ap.add_argument('--scale', type=float, default=7.0)
    ap.add_argument('--free', type=int, default=13)
    ap.add_argument('--src')
    ap.add_argument('--pfx', default=os.environ.get('FE_PFX_BASE', '21'))
    ap.add_argument('--json')
    ap.add_argument('--ceiling', action='store_true', help='also fit the rule-7 total-size ceiling (giants)')
    ap.add_argument('--no-measure', action='store_true', help='size proxy only, no probe callers')
    ap.add_argument('--tree', nargs='*')
    ap.add_argument('--from', dest='frm')
    a = ap.parse_args()
    work = []
    if a.tree is not None:
        rows = json.load(open(a.frm)) if a.frm else None
        work = tree_work(a.tree, rows)
    else:
        unit = a.unit if a.unit.startswith('src/') else 'src/' + a.unit
        work = [(unit, [int(x, 16) for x in a.addrs.split(',')])]
    allres = []
    for unit, addrs in work:
        src = a.src or fe.R + '/' + unit
        text = open(src, encoding='latin-1').read()
        try:
            Pod = procs(compile_listing(unit, src, a.pfx, True))
            Pog = procs(compile_listing(unit, src, a.pfx, False))
        except SystemExit as e:
            print(unit, 'compile failed', str(e)[:200])
            continue
        model = Model(Pod, text, a.scale, a.free)
        model.ceiling = a.ceiling
        if not a.no_measure:
            reach = set()
            for addr in addrs:
                idt = fe.ident(text, addr)
                stack = [idt[0]] if idt else []
                while stack:
                    b = stack.pop()
                    for _, c in Pod.get(b, {'sites': []})['sites']:
                        if model.inline(c) and c not in reach:
                            reach.add(c)
                            stack.append(c)
            costs = measure_costs(unit, src, text, sorted(reach), a.pfx)
            model.over = {c: v for c, v in costs.items() if v is not None}
            model.measured = costs
        for addr in addrs:
            r = analyse(unit, addr, text, Pod, Pog, model, a.leads, a.show)
            r['unit'] = unit
            allres.append(r)
            report(r)
    if a.json:
        json.dump(allres, open(a.json, 'w'), indent=1)


def report(r):
    if 'error' in r:
        print('%s %s %s' % (r['addr'], r['sym'], r['error']))
        return
    print('%s %-44s sites %3d  ours fit %d at S %s | image (offset) fit %d at S %s%s' % (
        r['addr'], r['sym'][:44], r['sites'], r['ours_fit'], r['ours_S'][:40], r['img_fit'], r['img_S'][:40],
        '  T %d' % r['T'] if r.get('T') else ''))
    if r['diff']:
        print('    image/ours: ' + ' '.join('%s %d/%d' % (k, a, b) for k, (a, b) in sorted(r['diff'].items())))
    if 'model_vs' in r:
        print('    walk at S=%g; model/ours/image: %s' % (r['walk_S'], ' '.join('%s %d/%d/%d' % (k, *v) for k, v in sorted(r['model_vs'].items()))))
        for l in r['walk']:
            print('      ' + l)
    if 'leads' in r:
        print('    leads (image distance at ours S, base %d):' % r['leads']['base'])
        for l in r['leads']['leads']:
            print('      ' + l)


def tree_work(units, rows):
    import glob
    out = []
    for p in sorted(glob.glob(fe.R + '/src/**/*.cpp', recursive=True)):
        u = p[len(fe.R) + 1:]
        if units and u not in units and os.path.basename(u) not in units:
            continue
        if not fe.compile_cmd(u):
            continue
        t = open(p, encoding='latin-1').read()
        addrs = [int(m.group(1), 16) for m in re.finditer(r'// ORIGINAL: 0x([0-9A-F]{8}) \S+ \S+([^\n]*)', t)
                 if 'BYTE_EXACT' not in m.group(2)]
        if addrs:
            out.append((u, addrs))
    return out


if __name__ == '__main__':
    main()
