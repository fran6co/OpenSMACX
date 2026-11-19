#!/usr/bin/env python3
"""outside.py --from FEALL.json [--min 1] [--max 15] [--levers sig,pos,inl,typ] [--only ADDR,..] [--json OUT] [--tree]
Outside-the-body lever screens for near-misses (companion of residue.py; MATCHING_GUIDE 3.2 "Residue census").
Every variant leaves the function's body text alone and changes one thing around it, then fes the function in a
shadow copy of src/ (build/recovery/shadow/<lane>/src), so header edits never touch the worktree:
  sig  each `int`/`BOOL` parameter as unsigned int, short, char; the return type int as unsigned int, short, char
       (definition and every matching declaration; the changed COMDAT is found by its undecorated prefix)
  pos  the function moved to the end of its unit (definition order / unit counters; pad sweeps are unitpad.py --at,
       locpad.py and sympad.py)
  inl  one header inline the body calls by name (or one that such an inline calls) respelt: N a bare `return E;` as a named
       local, B a named result as a bare return, P every parameter copied to a local first
  inl1 (not in the default set) the same callees with one parameter at a time copied to a local
  typ  the declared type of one global or field the body names, signedness flipped in the header
       (int <-> unsigned int, short <-> unsigned short, char <-> unsigned char and the stdint spellings)
Prints every variant whose rows differ from the base; `--tree` runs feall.py in the shadow for each variant that
lowers the rows and prints what it loses and gains tree-wide. Screen only; report header candidates to the owner of
the header instead of committing them. FE_PFX_BASE / FE_JOBS lanes."""
import sys, os, re, json, argparse, subprocess, shutil, collections
R = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
sys.path.insert(0, R + '/recovery/tools'); sys.path.insert(0, R + '/recovery/tools/analysis')
import fe
import tailscan
from concurrent.futures import ThreadPoolExecutor

SH = R + '/build/recovery/shadow'


def shadow(k):
    d = '%s/%d' % (SH, k)
    os.makedirs(d + '/build/recovery/fe', exist_ok=True)
    subprocess.run(['rsync', '-a', '--delete', R + '/src/', d + '/src/'], check=True)
    for n in ('recovery', 'packages', 'pyproject.toml', 'uv.lock'):
        if not os.path.lexists(d + '/' + n) and os.path.exists(R + '/' + n):
            os.symlink(R + '/' + n, d + '/' + n)
    return d


def compile_shadow(unit, overlay, k, pfx, listing=False):
    """overlay: {relpath under the root: text}. -> (Coff, listing text or None)"""
    d = '%s/%d' % (SH, k); saved = {}
    for rel, text in overlay.items():
        p = d + '/' + rel
        saved[rel] = open(p, encoding='latin-1').read()
        open(p, 'w', encoding='latin-1').write(text)
    try:
        cmd = fe.compile_cmd(unit)
        obj = '%s/build/recovery/fe/o.obj' % d; asm = '%s/build/recovery/fe/o.cod' % d
        out = []
        for t in cmd:
            if t.startswith('/Fo'):
                t = '/Fo' + obj
            elif t.startswith('/Fd') or t.startswith('--repro-root') or t == '/W4':
                continue
            else:
                t = t.replace(R + '/src', d + '/src')
            out.append(t)
        if listing:
            out.insert(-1, '/FAsc'); out.insert(-1, '/Fa' + asm)
        for p in (obj, asm):
            os.path.exists(p) and os.remove(p)
        env = dict(os.environ, VC6_PREFIX='%s/.opensmacx/wine/worker-%s' % (fe.MAIN, pfx))
        for attempt in (0, 1):
            subprocess.run(out, cwd=d + '/build/recovery/fe', env=env, capture_output=True, text=True)
            if os.path.exists(obj):
                break
        if not os.path.exists(obj):
            return None, None
        c = fe.Coff(open(obj, 'rb').read())
        cod = open(asm, errors='replace').read() if listing and os.path.exists(asm) else None
        return c, cod
    finally:
        for rel, text in saved.items():
            open(d + '/' + rel, 'w', encoding='latin-1').write(text)


def rows_of(c, text, addr, sym=None):
    """fe rows of the marker; with sym, the marker's symbol is replaced first (a signature variant)"""
    if sym:
        idn = fe.ident(text, addr)
        text = text.replace('// ORIGINAL: 0x%08X %s ' % (addr, idn[0]), '// ORIGINAL: 0x%08X %s ' % (addr, sym), 1)
        text = re.sub(r'(// ORIGINAL: 0x%08X [^\n]*\n(?://(?! ORIGINAL:)[^\n]*\n)*?)// symbol\s+\S+' % addr,
                      lambda m: m.group(1) + '// symbol    ' + sym, text, 1)
    r = fe.evaluate(c, text, addr)
    if 'error' in r:
        return None
    return 0 if r['exact'] else max(r['diff'], 1)


def variant_symbol(c, sym):
    """the COMDAT a signature variant produced: same undecorated prefix, another parameter/return encoding"""
    if '@@' not in sym:
        return None
    pre = sym[:sym.index('@@') + 2]
    cand = [s.name for s in c.symbols.values() if s.name.startswith(pre) and s.section > 0 and s.name != sym]
    ok = []
    for n in cand:
        try:
            c.function(n); ok.append(n)
        except Exception:
            pass
    return ok[0] if len(ok) == 1 else None


# ---- source parsing -------------------------------------------------------------------------------------------
def definition(text, addr):
    """-> (head_start, body_open, body_close, head text) of the function defined under the marker"""
    m = re.search(r'// ORIGINAL: 0x%08X [^\n]*\n' % addr, text)
    if not m:
        return None
    i = m.end()
    if text.rfind('/*', 0, m.start()) > text.rfind('*/', 0, m.start()):
        i = text.index('*/', i) + 2   # the marker sits inside a doc comment
    while True:
        if text.startswith('//', i):
            i = text.index('\n', i) + 1
        elif text.startswith('/*', i):
            i = text.index('*/', i) + 2
        elif text[i] in ' \t\r\n':
            i += 1
        else:
            break
    b = text.find('{', i)
    nxt = text.find('// ORIGINAL: ', i)
    if b < 0 or (0 <= nxt < b) or ';' in text[i:b] or '(' not in text[i:b] or '=' in text[i:b]:
        # other items stand between the marker and the function: find the definition by the symbol's name
        sym = fe.ident(text, addr)[0]
        mn = re.match(r'\?(\w+)@', sym)
        if not mn:
            return None
        lim = nxt if nxt >= 0 else len(text); found = None
        mc = re.match(r'\?\w+@(\w+)@@', sym)
        pat = r'\b%s\s*\(' % re.escape(mn.group(1))
        if mc:
            pat = r'\b%s::%s\s*\(' % (re.escape(mc.group(1)), re.escape(mn.group(1))); lim = len(text)
        for mm in re.finditer(pat, text[i:lim]):
            j = i + mm.end(); depth = 1
            while j < lim and depth:
                depth += text[j] == '('; depth -= text[j] == ')'; j += 1
            tail = re.match(r'\s*(?:const\s*)?\{', text[j:j + 40])
            if tail:
                ls = text.rfind('\n', 0, i + mm.start()) + 1
                if not text[ls:i + mm.start()].strip():
                    ls = text.rfind('\n', 0, ls - 1) + 1
                found = (ls, j + tail.end() - 1); break
        if not found:
            return None
        i, b = found
    depth = 0; j = b
    while j < len(text):
        ch = text[j]
        if ch == '{':
            depth += 1
        elif ch == '}':
            depth -= 1
            if depth == 0:
                break
        elif ch == '"' or ch == "'":
            q = ch; j += 1
            while text[j] != q:
                j += 2 if text[j] == '\\' else 1
        elif text.startswith('//', j):
            j = text.index('\n', j)
        elif text.startswith('/*', j):
            j = text.index('*/', j) + 1
        j += 1
    return i, b, j, text[i:b]


def split_params(s):
    out = []; depth = 0; cur = ''
    for ch in s:
        if ch in '(<[':
            depth += 1
        elif ch in ')>]':
            depth -= 1
        if ch == ',' and depth == 0:
            out.append(cur); cur = ''
        else:
            cur += ch
    if cur.strip():
        out.append(cur)
    return out


HEAD = re.compile(r'^(?P<ret>.*?)(?P<qual>(?:\w+::)*)(?P<name>~?\w+)\s*\((?P<params>.*)\)\s*(?P<tail>const)?\s*$', re.S)


def parse_head(head):
    m = HEAD.match(head.strip())
    if not m:
        return None
    ps = split_params(m.group('params'))
    if len(ps) == 1 and ps[0].strip() in ('void', ''):
        ps = []
    return m, ps


def ptype(p):
    """(type text, name) of a parameter declaration"""
    p = p.split('=')[0].strip()
    m = re.match(r'^(.*?)(\w+)\s*(\[[^\]]*\])?$', p, re.S)
    if not m or not m.group(1).strip():
        return p, ''
    return m.group(1).strip(), m.group(2)


def decl_sites(name, nparams, files):
    """declarations `name(...)` with nparams parameters ending in `;` (or an inline body) in the given texts"""
    out = []
    for rel, t in files.items():
        for m in re.finditer(r'\b%s\s*\(' % re.escape(name), t):
            j = m.end(); depth = 1
            while j < len(t) and depth:
                depth += t[j] == '('; depth -= t[j] == ')'; j += 1
            params = t[m.end():j - 1]
            tail = t[j:j + 40].lstrip()
            if not (tail.startswith(';') or tail.startswith('const;') or tail.startswith('const ;') or tail.startswith('= 0')):
                continue
            ps = split_params(params)
            if len(ps) == 1 and ps[0].strip() in ('void', ''):
                ps = []
            if len(ps) != nparams:
                continue
            ls = t.rfind('\n', 0, m.start()) + 1
            pre = t[ls:m.start()]
            if re.search(r'[=(,]|\breturn\b', pre) or not pre.strip():
                continue   # a call, not a declaration
            out.append((rel, ls, m.start(), m.end(), j - 1))
    return out


def retype(decl, new):
    ty, nm = ptype(decl)
    lead = re.match(r'\s*', decl).group(0)
    return lead + new + (' ' + nm if nm else '')


INTS = ('int', 'BOOL', 'signed int')


def sig_variants(unit, text, addr, headers):
    d = definition(text, addr)
    if not d:
        return []
    i, b, e, head = d
    ph = parse_head(head)
    if not ph:
        return []
    m, ps = ph
    name = m.group('name'); out = []
    hs = dict(headers)
    sites_h = decl_sites(name, len(ps), hs)
    want = [re.sub(r'\s+', '', ptype(p)[0]) for p in ps]
    inc = closure(unit, text, hs)
    cls = m.group('qual').rstrip(':').split('::')[-1] if m.group('qual') else None

    def keep(s):
        rel, ls, s0, s1, s2 = s
        if rel not in inc:
            return False
        if [re.sub(r'\s+', '', ptype(p)[0]) for p in split_params(hs[rel][s1:s2]) if p.strip() not in ('void', '')] != want:
            return False
        if cls:
            ht = hs[rel]
            for cm in re.finditer(r'\b(?:class|struct)\s+%s\b[^;{]*\{' % re.escape(cls), ht):
                depth = 0; j = cm.end() - 1
                while j < len(ht):
                    depth += ht[j] == '{'; depth -= ht[j] == '}'
                    if depth == 0:
                        break
                    j += 1
                if cm.end() <= s0 < j:
                    return True
            return False
        return True
    sites_h = [s for s in sites_h if keep(s)]
    sites_c = [s for s in decl_sites(name, len(ps), {unit: text}) if not (i <= s[2] < b)]

    def build(newps=None, newret=None):
        ov = {}
        # definition
        mm = HEAD.match(head.strip())
        ret = mm.group('ret') if newret is None else newret(mm.group('ret'))
        if ret is None:
            return None
        params = ', '.join(x.strip() for x in (newps if newps is not None else ps))
        nh = '%s%s%s(%s)%s ' % (ret, mm.group('qual'), name, params, ' const' if mm.group('tail') else '')
        t2 = text[:i] + nh + text[b:]
        # other declarations in the unit text (offsets before i are unaffected; later ones shift)
        for rel, ls, s0, s1, s2 in sorted(sites_c, key=lambda x: -x[2]):
            sh = (len(nh) - (b - i)) if s0 > b else 0
            t2 = edit_decl(t2, ls + sh, s0 + sh, s1 + sh, s2 + sh, newps, newret)
        ov[unit] = t2
        for rel in {s[0] for s in sites_h}:
            t = hs[rel]
            for _, ls, s0, s1, s2 in sorted([s for s in sites_h if s[0] == rel], key=lambda x: -x[2]):
                t = edit_decl(t, ls, s0, s1, s2, newps, newret)
            ov[rel] = t
        return ov

    def edit_decl(t, ls, s0, s1, s2, newps, newret):
        if newps is not None:
            old = split_params(t[s1:s2])
            if len(old) == len(newps):
                merged = []
                for o, n in zip(old, newps):
                    oty, onm = ptype(o); nty, nnm = ptype(n)
                    dflt = ('=' + o.split('=', 1)[1]) if '=' in o else ''
                    merged.append((nty + (' ' + onm if onm else '') + (' ' + dflt if dflt else '')))
                t = t[:s1] + ', '.join(merged) + t[s2:]
        if newret is not None:
            pre = t[ls:s0]
            mq = re.match(r'^(.*?)((?:\w+::)*)$', pre, re.S)
            r2 = newret(mq.group(1))
            if r2 is not None:
                t = t[:ls] + r2 + mq.group(2) + t[s0:]
        return t
    for k, p in enumerate(ps):
        ty, nm = ptype(p)
        if ty in INTS:
            for new in ('unsigned int', 'short', 'char'):
                nps = list(ps); nps[k] = retype(p, new)
                ov = build(newps=nps)
                if ov:
                    out.append(('sig:param%d(%s) %s->%s' % (k, nm, ty, new), ov, True))
    mret = re.search(r'\b(int|BOOL)\s*(?:__cdecl|__stdcall|__fastcall)?\s*$', m.group('ret'))
    if mret:
        for new in ('unsigned int', 'short', 'char'):
            def f(r, new=new):
                r2, n = re.subn(r'\b(int|BOOL)(\s*(?:__cdecl|__stdcall|__fastcall)?\s*)$', lambda q: new + q.group(2), r)
                return r2 if n else None
            ov = build(newret=f)
            if ov:
                out.append(('sig:return %s->%s' % (mret.group(1), new), ov, True))
    return out


def pos_variants(unit, text, addr):
    m = re.search(r'// ORIGINAL: 0x%08X ' % addr, text)
    d = definition(text, addr)
    if not d:
        return []
    s = m.start()
    c0 = text.rfind('/*', 0, s); c1 = text.rfind('*/', 0, s)
    if c0 > c1:
        s = c0
    e = d[2] + 1
    block = text[s:e]
    if block.count('// ORIGINAL: ') != 1 or '#include' in block:
        return []   # the marker stands apart from its definition
    rest = text[:s] + text[e:]
    return [('pos:moved to the end of the unit', {unit: rest.rstrip('\n') + '\n\n' + block + '\n'}, False)]


INLINE_HEAD = re.compile(r'^[ \t]*(?:MEASURED\s+|static\s+|inline\s+|__forceinline\s+|virtual\s+)*(?P<ret>[\w:<>\*& ]+?)\s*(?:__cdecl\s+|__stdcall\s+|__fastcall\s+)?(?P<name>~?\w+)\s*\((?P<params>[^;{}]*)\)\s*(?:const\s*)?\{', re.M)


def inline_at(htext, line):
    """the inline function definition of a header that encloses the 1-based line -> (name, ret, params, body_open, body_close)"""
    off = 0
    for _ in range(line - 1):
        off = htext.find('\n', off) + 1
    best = None
    for m in INLINE_HEAD.finditer(htext, max(0, off - 4000), off + 400):
        b = m.end() - 1; depth = 0; j = b
        while j < len(htext):
            if htext[j] == '{':
                depth += 1
            elif htext[j] == '}':
                depth -= 1
                if depth == 0:
                    break
            j += 1
        le = htext.find('\n', off)
        if m.start() <= le and j >= off and m.group('ret').strip() not in ('return', 'else', 'if', 'while', 'for', 'switch', 'new'):
            if best is None or m.start() > best[0]:
                best = (m.start(), m.group('name'), m.group('ret').strip(), m.group('params'), b, j)
    return best and best[1:]


def inline_defs(ht):
    """inline function definitions of a header -> [(name, ret, params, body_open, body_close)]"""
    out = []
    for m in INLINE_HEAD.finditer(ht):
        if not m.group('ret').strip() or m.group('ret').strip().split()[-1] in ('return', 'else', 'if', 'while', 'for', 'switch', 'new', 'do'):
            continue
        b = m.end() - 1; depth = 0; j = b
        while j < len(ht):
            if ht[j] == '{':
                depth += 1
            elif ht[j] == '}':
                depth -= 1
                if depth == 0:
                    break
            j += 1
        out.append((m.group('name'), m.group('ret').strip(), m.group('params'), b, j))
    return out


def inl_variants(unit, text, addr, headers, single=False):
    """VC6's line table gives an expansion the call's line, so the expanded callees are found by name: every
    inline definition of the included headers that the body calls, and the ones those bodies call"""
    d = definition(text, addr)
    if not d:
        return []
    inc = closure(unit, text, headers)
    defs = {rel: inline_defs(headers[rel]) for rel in inc}
    body = re.sub(r'//[^\n]*|"(?:\\.|[^"\\])*"', '', text[d[1]:d[2]])
    called = set(re.findall(r'\b(\w+)\s*\(', body))
    for rel, L in defs.items():
        for name, ret, params, b, e in L:
            if name in called:
                called |= {('2', x) for x in re.findall(r'\b(\w+)\s*\(', headers[rel][b:e])}
    lvl2 = {x[1] for x in called if isinstance(x, tuple)}; lvl1 = {x for x in called if isinstance(x, str)}
    out = []
    for rel, L in sorted(defs.items()):
        ht = headers[rel]
        for name, ret, params, b, e in L:
            if name not in lvl1 and name not in lvl2:
                continue
            ln = ht.count('\n', 0, b) + 1
            body_nc = re.sub(r'//[^\n]*', '', ht[b + 1:e]).strip()
            ret_t = re.sub(r'\b(MEASURED|static|inline|__forceinline|virtual)\b', '', ret).strip()
            mk = lambda nb: ht[:b] + '{ ' + nb + ' }' + ht[e + 1:]
            where = '%s:%d %s%s' % (rel, ln, name, '' if name in lvl1 else ' (nested)')
            m1 = re.fullmatch(r'return\s+(.+);', body_nc, re.S)
            if m1 and ';' not in m1.group(1) and ret_t and ret_t != 'void':
                out.append(('inl:N %s named result' % where, {rel: mk('%s fe_result = %s; return fe_result;' % (ret_t, m1.group(1)))}, False))
            m2 = re.fullmatch(r'(?:const\s+)?[\w:\*& ]+?\b(\w+)\s*=\s*([^;]+);\s*return\s+(\w+);', body_nc, re.S)
            if m2 and m2.group(1) == m2.group(3):
                out.append(('inl:B %s bare return' % where, {rel: mk('return %s;' % m2.group(2))}, False))
            ps = [ptype(p) for p in split_params(params)] if params.strip() not in ('', 'void') else []
            ps = [(ty, n) for ty, n in ps if n and '&' not in ty and '[' not in ty]
            if single:
                out = [v for v in out if not v[0].startswith('inl:')]   # only the per-parameter forms
                for ty, n in (ps if body_nc and '#' not in body_nc and len(ps) > 1 else []):
                    nb = re.sub(r'(?<![\w.>])%s\b' % re.escape(n), 'fe_' + n, body_nc)
                    out.append(('inl1:P1 %s parameter %s copied to a local' % (where, n), {rel: mk('%s fe_%s = %s; ' % (ty, n, n) + nb)}, False))
                continue
            if ps and body_nc and '#' not in body_nc:
                nb = body_nc; pre = ''
                for ty, n in ps:
                    nb = re.sub(r'(?<![\w.>])%s\b' % re.escape(n), 'fe_' + n, nb)
                    pre += '%s fe_%s = %s; ' % (ty, n, n)
                out.append(('inl:P %s parameters copied to locals' % where, {rel: mk(pre + nb)}, False))
    return out


FLIP = [('unsigned int', 'int'), ('uint32_t', 'int'), ('unsigned short', 'short'), ('uint16_t', 'short'),
        ('unsigned char', 'char'), ('uint8_t', 'char'), ('int32_t', 'unsigned int'), ('int16_t', 'unsigned short'),
        ('int8_t', 'unsigned char'), ('int', 'unsigned int'), ('short', 'unsigned short'), ('char', 'unsigned char'),
        ('BOOL', 'unsigned int'), ('unsigned', 'int')]
DECL = r'^(?P<lead>[ \t]*(?:extern\s+)?(?:const\s+)?)(?P<ty>%s)(?P<mid>\s+(?:\w+\s*,\s*)*)(?P<name>%%s)\b(?P<rest>\s*(?:\[[^\]]*\]\s*)*(?::\s*\d+\s*)?[;,=])' % '|'.join(re.escape(a) for a, b in FLIP)


def closure(unit, text, headers):
    """headers the unit includes, transitively (by basename; a superset is harmless)"""
    byname = {}
    for rel in headers:
        byname.setdefault(os.path.basename(rel), []).append(rel)
    seen = set(); stack = [text]
    while stack:
        t = stack.pop()
        for inc in re.findall(r'#\s*include\s*"([^"]+)"', t):
            for rel in byname.get(os.path.basename(inc), []):
                if rel not in seen:
                    seen.add(rel); stack.append(headers[rel])
    return seen


def typ_variants(unit, text, addr, headers):
    d = definition(text, addr)
    if not d:
        return []
    inc = closure(unit, text, headers)
    headers = {k: v for k, v in headers.items() if k in inc}
    ph = parse_head(d[3])
    own = {ptype(p)[1] for p in ph[1]} if ph else set()
    own |= set(re.findall(r'\b(?:int|char|short|unsigned|BOOL|uint\d+_t|int\d+_t|long)\s+\**(\w+)\s*[=;,]', text[d[1]:d[2]]))
    body = re.sub(r'//[^\n]*|"(?:\\.|[^"\\])*"', '', text[d[1]:d[2]])
    ids = collections.OrderedDict((x, 1) for x in re.findall(r'\b[A-Za-z_]\w*\b', body))
    out = []; flip = dict(FLIP)
    for name in ids:
        if len(name) < 3 or name in own:
            continue
        rx = re.compile(DECL % re.escape(name), re.M)
        for rel, ht in headers.items():
            ms = list(rx.finditer(ht))
            if not ms or len(ms) > 3:
                continue
            for m in ms:
                if m.group('mid').strip():
                    continue   # one declaration of several names
                t2 = ht[:m.start('ty')] + flip[m.group('ty')] + ht[m.end('ty'):]
                ln = ht.count('\n', 0, m.start()) + 1
                out.append(('typ:%s:%d %s %s->%s' % (rel, ln, name, m.group('ty'), flip[m.group('ty')]), {rel: t2}, False))
    return out


def load_headers():
    hs = {}
    for dp, dn, fn in os.walk(R + '/src'):
        if '/vendor' in dp:
            continue
        for f in fn:
            if f.endswith('.h'):
                p = os.path.join(dp, f)
                hs[os.path.relpath(p, R)] = open(p, encoding='latin-1').read()
    return hs


def tree(k, overlay, basejson, pfx_base):
    d = '%s/%d' % (SH, k); saved = {}
    for rel, text in overlay.items():
        saved[rel] = open(d + '/' + rel, encoding='latin-1').read()
        open(d + '/' + rel, 'w', encoding='latin-1').write(text)
    try:
        p = subprocess.run(['uv', 'run', 'python', d + '/recovery/tools/feall.py', '--all', '--base', basejson, '--out', d + '/build/tree.json', '--jobs', '1'],
                           cwd=d, capture_output=True, text=True, env=dict(os.environ, FE_PFX_BASE=str(pfx_base), FE_JOBS='1'))
        return '\n'.join(l for l in p.stdout.split('\n') if l.startswith(('LOST', 'GAINED', 'MOVED', '  -', '  +')))
    finally:
        for rel, text in saved.items():
            open(d + '/' + rel, 'w', encoding='latin-1').write(text)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--from', dest='frm', required=True); ap.add_argument('--max', type=int, default=15); ap.add_argument('--min', type=int, default=1)
    ap.add_argument('--levers', default='sig,pos,inl,typ'); ap.add_argument('--only'); ap.add_argument('--json')
    ap.add_argument('--tree'); ap.add_argument('--all', action='store_true', help='print every variant, not only the ones that move')
    a = ap.parse_args()
    base = int(os.environ.get('FE_PFX_BASE', '21')); jobs = int(os.environ.get('FE_JOBS', '5'))
    levers = a.levers.split(',')
    only = {x.upper().replace('0X', '') for x in a.only.split(',')} if a.only else None
    todo = [x for x in json.load(open(a.frm)) if not x['exact'] and a.min <= x['diff'] <= a.max and (not only or x['addr'] in only)]
    todo.sort(key=lambda x: (x['unit'], x['addr']))
    headers = load_headers()
    fe.compile_cmd('src/main.cpp')   # fill the command cache before the lanes start
    for k in range(jobs):
        shadow(base + k)   # one shadow per compiler prefix, so two runs on different prefixes never share one
    out = open(a.json, 'a') if a.json else None
    done = set()
    if a.json and os.path.exists(a.json):
        for l in open(a.json):
            try:
                done.add(json.loads(l)['addr'])
            except Exception:
                pass

    def one(x, k):
        unit = x['unit']; addr = int(x['addr'], 16); pfx = str(base + k); k = base + k
        text = open(R + '/' + unit, encoding='latin-1').read()
        if re.search(r'// ORIGINAL: 0x%08X [^\n]*\n(?://(?! ORIGINAL)[^\n]*\n)*// compile_context' % addr, text):
            return dict(addr=x['addr'], unit=unit, sym=x['sym'], skipped='compile_context')
        c, cod = compile_shadow(unit, {}, k, pfx)
        if c is None:
            return dict(addr=x['addr'], unit=unit, sym=x['sym'], skipped='compile')
        b0 = rows_of(c, text, addr)
        sym = fe.ident(text, addr)[0]
        vs = []
        if 'sig' in levers:
            vs += sig_variants(unit, text, addr, headers)
        if 'pos' in levers:
            vs += pos_variants(unit, text, addr)
        if 'inl' in levers:
            vs += inl_variants(unit, text, addr, headers)
        if 'inl1' in levers:
            vs += inl_variants(unit, text, addr, headers, single=True)
        if 'typ' in levers:
            vs += typ_variants(unit, text, addr, headers)
        res = []
        for key, ov, resym in vs:
            c2, _ = compile_shadow(unit, ov, k, pfx)
            if c2 is None:
                res.append((key, None)); continue
            t2 = ov.get(unit, text)
            s2 = variant_symbol(c2, sym) if resym else None
            try:
                r = rows_of(c2, t2, addr, s2)
            except Exception:
                r = None
            res.append((key, r))
        hits = []
        if a.tree:
            for (key, ov, resym), (_, r) in zip(vs, res):
                if r is not None and b0 is not None and r < b0 and not key.startswith(('pos', 'sig')):
                    hits.append((key, tree(k, ov, os.path.abspath(a.tree), k)))
        return dict(addr=x['addr'], unit=unit, sym=x['sym'], base=b0, variants=res, tree=hits)

    def lane(k):
        o = []
        for i in range(k, len(todo), jobs):
            if todo[i]['addr'] in done:
                continue
            try:
                r = one(todo[i], k)
            except Exception as ex:
                if os.environ.get('OUTSIDE_DEBUG'):
                    import traceback; traceback.print_exc()
                r = dict(addr=todo[i]['addr'], unit=todo[i]['unit'], sym=todo[i]['sym'], skipped='error %r' % (ex,))
            o.append(r)
            report(r)
            if out:
                out.write(json.dumps(r) + '\n'); out.flush()
        return o

    def report(r):
        L = ['%s %s %s base %s' % (r['addr'], r['unit'], r['sym'][:40], r.get('base', r.get('skipped')))]
        if 'variants' in r:
            n = collections.Counter(k.split(':')[0] for k, v in r['variants'])
            f = collections.Counter(k.split(':')[0] for k, v in r['variants'] if v is None)
            L[0] += '  variants ' + ' '.join('%s=%d(%d failed)' % (k, n[k], f[k]) for k in n)
            for k, v in r['variants']:
                if v is not None and (a.all or v != r['base']):
                    L.append('    %4d  %s' % (v, k))
            for k, t in r.get('tree', []):
                L.append('    TREE %s\n%s' % (k, '\n'.join('        ' + l for l in t.split('\n'))))
        print('\n'.join(L), flush=True)
    with ThreadPoolExecutor(jobs) as ex:
        list(ex.map(lane, range(jobs)))


if __name__ == '__main__':
    main()
