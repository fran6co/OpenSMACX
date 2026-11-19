#!/usr/bin/env python3
"""armwo.py <unit> <0xADDR> [--max N] [--top K] [--write] [--only a,b,c,d,t] [--show KEY]
Arm-structure screen (MATCHING_GUIDE 2.7 rule (b) and its relatives; the sibling of gotowo.py).
Parses the function body into statements and generates whole-file variants of four spellings:
  d  `if (c) goto L; S; L:`        -> `if (!(c)) { S }` or `if (c) { } else { S }` (an empty first arm),
                                       per goto site (keep / negate / empty arm), so chains become else-if
                                       chains or nested blocks; adjacent negated sites also as one `&&`.
  t  a shared tail block T (the statements after a label, or a suffix of a statement list, ending in a
     return or the function end) written out at subsets of its predecessors: goto sites at any depth and
     the fallthrough ends of the preceding if/else-if chain (each arm, and the implicit else).
  c  statements repeated at the end of every arm of an if/else(-if) chain collapsed after the join.
  s  the 1-2 statements after an if/else chain sunk into every arm (the reverse of c).
Every variant is fe'd (perm.run_deck); the best by rows are then fingerprinted: call multiset difference
against the image, frame size difference, candset.py roles on one side only + disputed register pairs.
Ranking: calls, frame, candset, rows (each lower is better). A variant whose frame is further from the
image than the base is never written. --write replaces the unit when the best variant beats the base
(on the ranking key; fewer rows at an equal structure, or a better structure at any row count).
Env: FE_PFX_BASE (lane prefixes), FE_JOBS."""
import sys, os, re, io, itertools, argparse, collections, contextlib
H = os.path.dirname(os.path.abspath(__file__))
T = os.path.dirname(H)
R = os.path.dirname(os.path.dirname(T))
sys.path.insert(0, T)
sys.path.insert(0, H)
import perm

KW_CTRL = {'if', 'else', 'while', 'for', 'do', 'switch', 'return', 'goto', 'break', 'continue', 'case', 'default'}


# ---------------------------------------------------------------- tokenizer
def tokenize(s):
    toks = []
    i, n = 0, len(s)
    while i < n:
        c = s[i]
        if c in ' \t\r\n':
            i += 1
        elif s.startswith('//', i):
            j = s.find('\n', i)
            i = n if j < 0 else j
        elif s.startswith('/*', i):
            j = s.find('*/', i + 2)
            i = n if j < 0 else j + 2
        elif c == '#':
            raise ValueError('preprocessor line in body')
        elif c in '"\'':
            j = i + 1
            while j < n and s[j] != c:
                j += 2 if s[j] == '\\' else 1
            toks.append(('lit', s[i:j + 1], i, j + 1))
            i = j + 1
        elif c.isalpha() or c == '_':
            j = i
            while j < n and (s[j].isalnum() or s[j] == '_'):
                j += 1
            toks.append(('id', s[i:j], i, j))
            i = j
        elif c.isdigit():
            j = i
            while j < n and (s[j].isalnum() or s[j] in '._'):
                j += 1
            toks.append(('num', s[i:j], i, j))
            i = j
        else:
            if s.startswith('::', i) or s.startswith('->', i):
                toks.append(('p', s[i:i + 2], i, i + 2))
                i += 2
            else:
                toks.append(('p', c, i, i + 1))
                i += 1
    return toks


# ---------------------------------------------------------------- statement tree
class N:
    def __init__(self, kind, s, e, **kw):
        self.kind, self.s, self.e = kind, s, e
        self.__dict__.update(kw)

    def __repr__(self):
        return '%s[%d:%d]' % (self.kind, self.s, self.e)


class Parser:
    def __init__(self, text):
        self.t = text
        self.k = tokenize(text)
        self.i = 0

    def peek(self, d=0):
        j = self.i + d
        return self.k[j] if j < len(self.k) else ('eof', '', len(self.t), len(self.t))

    def take(self, v=None):
        tk = self.peek()
        if v is not None and tk[1] != v:
            raise ValueError('expected %r got %r at %d' % (v, tk[1], tk[2]))
        self.i += 1
        return tk

    def balanced(self, open_='(', close=')'):
        a = self.take(open_)
        d = 1
        while d:
            tk = self.take()
            if tk[0] == 'eof':
                raise ValueError('eof')
            if tk[1] == open_:
                d += 1
            elif tk[1] == close:
                d -= 1
        return a[2], self.k[self.i - 1][3]

    def stmts(self, end='}'):
        out = []
        while self.peek()[1] != end and self.peek()[0] != 'eof':
            out.append(self.stmt())
        return out

    def stmt(self):
        tk = self.peek()
        v = tk[1]
        if v == '{':
            self.take()
            ch = self.stmts()
            e = self.take('}')
            return N('block', tk[2], e[3], ch=ch)
        if v == ';':
            self.take()
            return N('simple', tk[2], tk[3], sk='empty')
        if tk[0] == 'id' and self.peek(1)[1] == ':' and v not in ('default', 'public', 'private') and self.peek(2)[1] != ':':
            self.take(); c = self.take()
            return N('label', tk[2], c[3], name=v)
        if v == 'case':
            self.take()
            while self.peek()[1] != ':':
                self.take()
            c = self.take()
            return N('case', tk[2], c[3])
        if v == 'default' and self.peek(1)[1] == ':':
            self.take(); c = self.take()
            return N('case', tk[2], c[3])
        if v == 'if':
            self.take()
            cs, ce = self.balanced()
            th = self.stmt()
            el = None
            if self.peek()[1] == 'else':
                self.take()
                el = self.stmt()
            return N('if', tk[2], (el or th).e, cond=(cs + 1, ce - 1), th=th, el=el)
        if v in ('while', 'for', 'switch'):
            self.take()
            hs, he = self.balanced()
            b = self.stmt()
            return N('loop', tk[2], b.e, lk=v, body=b)
        if v == 'do':
            self.take()
            b = self.stmt()
            self.take('while')
            self.balanced()
            e = self.take(';')
            return N('loop', tk[2], e[3], lk='do', body=b)
        if v in ('__try', 'try'):
            raise ValueError('try')
        # simple statement up to ';' (braces only inside initializers)
        d = 0
        sk = {'return': 'return', 'goto': 'goto', 'break': 'break', 'continue': 'continue'}.get(v, 'expr')
        target = self.peek(1)[1] if v == 'goto' else None
        while True:
            x = self.take()
            if x[0] == 'eof':
                raise ValueError('eof in statement')
            if x[1] in '({[':
                d += 1
            elif x[1] in ')}]':
                d -= 1
            elif x[1] == ';' and d == 0:
                return N('simple', tk[2], x[3], sk=sk, target=target)


def walk(n):
    yield n
    for c in children_of(n):
        yield from walk(c)


def children_of(n):
    if n.kind == 'block':
        return n.ch
    if n.kind == 'if':
        return [n.th] + ([n.el] if n.el else [])
    if n.kind == 'loop':
        return [n.body]
    return []


def lists(root):
    """every statement list (block children), with the block node"""
    for n in walk(root):
        if n.kind == 'block':
            yield n


def terminates(n):
    if n.kind == 'simple':
        return n.sk in ('return', 'goto', 'break', 'continue')
    if n.kind == 'block':
        return bool(n.ch) and terminates(n.ch[-1])
    if n.kind == 'if':
        return n.el is not None and terminates(n.th) and terminates(n.el)
    return False


def negate(c):
    """the natural spelling of !(c): flip one top-level comparison, drop a leading !, or !x for a primary"""
    c = c.strip()
    tk = tokenize(c)
    d = 0
    top = []
    for k, (kind, v, a, b) in enumerate(tk):
        if v in '([':
            d += 1
        elif v in ')]':
            d -= 1
        elif d == 0 and kind == 'p':
            top.append((k, v))
    ops = [v for k, v in top]
    two = re.findall(r'[=!<>]=|&&|\|\||[<>?]', ''.join(ops))
    if not top:
        return '!' + c
    if ops == ['!'] and tk[0][1] == '!' and tk[1][1] == '(' and tk[-1][1] == ')':
        return c[2:-1].strip() if False else c[1:].strip()[1:-1].strip()
    if tk[0][1] == '!' and all(v in ('.', '->', '::') for v in ops[1:]):
        return c[1:].strip()
    if all(v in ('.', '->', '::') for v in ops):
        return '!' + c
    # one comparison at the top level (written as two tokens by the tokenizer)
    flip = {'==': '!=', '!=': '==', '<': '>=', '>': '<=', '<=': '>', '>=': '<'}
    pos = [(k, v) for k, v in top if v in '=!<>']
    if len(pos) in (1, 2) and not re.search(r'&&|\|\||\?', c.replace('->', '')):
        k0 = pos[0][0]
        op = tk[k0][1] + (tk[k0 + 1][1] if len(pos) == 2 and pos[1][0] == k0 + 1 and tk[k0 + 1][2] == tk[k0][3] else '')
        if op in flip and (len(pos) == 1) == (len(op) == 1) and op not in ('!', '='):
            a0, b0 = tk[k0][2], tk[k0 + len(op) - 1][3]
            return c[:a0].rstrip() + ' ' + flip[op] + ' ' + c[b0:].lstrip()
    return '!(%s)' % c


def line_depths(text):
    """(brace depth, paren depth) at each line start"""
    lines = text.split('\n')
    starts = [0]
    for ln in lines:
        starts.append(starts[-1] + len(ln) + 1)
    ev = sorted((tk[2], tk[1]) for tk in tokenize(text) if tk[1] in '{}()')
    out = []
    depth, par, q = 1, 0, 0
    for j, ln in enumerate(lines):
        while q < len(ev) and ev[q][0] < starts[j]:
            v = ev[q][1]
            depth += {'{': 1, '}': -1}.get(v, 0)
            par += {'(': 1, ')': -1}.get(v, 0)
            q += 1
        out.append((depth - (1 if ln.strip().startswith('}') else 0), par))
    return out


def reindent(old, new):
    """re-indent new (a function body): a line taken from old keeps its old indentation shifted by its
    change of brace depth; a synthesized line is indented from the line before it"""
    import difflib
    ol = old.split('\n')
    nl = new.split('\n')
    od = line_depths(old)
    nd = line_depths(new)
    src = {}
    for t_, i1, i2, j1, j2 in difflib.SequenceMatcher(None, ol, nl, autojunk=False).get_opcodes():
        if t_ == 'equal':
            for k in range(i2 - i1):
                src[j1 + k] = i1 + k
    bytext = collections.defaultdict(list)
    for i, ln in enumerate(ol):
        if ln.strip():
            bytext[ln.strip()].append(i)
    out = []
    anchor = (4, 1)
    for j, ln in enumerate(nl):
        st = ln.strip()
        lab = bool(re.match(r'^[A-Za-z_]\w*:\s*$', st)) and not st.startswith('default')
        if not st:
            # a blank line of old stays; whitespace left behind by a removed statement goes
            if j not in src and j not in (0, len(nl) - 1) and (ln or (out and not out[-1].strip())):
                continue
            out.append(ln if j in (0, len(nl) - 1) else '')
            continue
        i = src.get(j)
        if i is None and bytext.get(st):
            i = min(bytext[st], key=lambda q: abs(q - j))
        if lab:
            ind = len(ol[i]) - len(ol[i].lstrip()) if i is not None else 0
        elif i is not None:
            ind = max(0, len(ol[i]) - len(ol[i].lstrip()) + 4 * (nd[j][0] - od[i][0]))
        else:
            ai, ae = anchor
            ind = max(0, ai + 4 * (nd[j][0] - ae)) + (8 if nd[j][1] > 0 else 0)
        out.append(' ' * ind + st)
        if not lab and nd[j][1] == 0:
            anchor = (ind, nd[j][0])
    return '\n'.join(out)


def norm(s):
    return re.sub(r'\s+', '', s)


# ---------------------------------------------------------------- function location
def locate(t, addr):
    m = re.search(r'// ORIGINAL: 0x%s ' % addr[2:].upper().zfill(8), t)
    if not m:
        raise SystemExit('no marker')
    j = t.find('\n', m.end()) + 1
    # a marker written inside a block comment: continue after its close
    q, inblock = 0, False
    while q < m.start():
        if inblock:
            e = t.find('*/', q)
            if e < 0 or e >= m.start():
                break
            q, inblock = e + 2, False
        else:
            a1, a2 = t.find('//', q, m.start()), t.find('/*', q, m.start())
            if a2 < 0:
                break
            if 0 <= a1 < a2:
                q = t.find('\n', a1) + 1
            else:
                q, inblock = a2 + 2, True
    if inblock:
        j = t.find('*/', m.start()) + 2
    # skip comment lines after the marker, then the signature up to the first '{' at depth 0
    p = j
    while True:
        nl = t.find('\n', p)
        ln = t[p:nl]
        if ln.lstrip().startswith('//') or not ln.strip():
            p = nl + 1
            continue
        if ln.lstrip().startswith('/*'):
            p = t.find('*/', p) + 2
            continue
        break
    sig_start = p
    d = 0
    q = p
    while True:
        c = t[q]
        if t.startswith('//', q):
            q = t.find('\n', q)
            continue
        if t.startswith('/*', q):
            q = t.find('*/', q) + 2
            continue
        if c == '(':
            d += 1
        elif c == ')':
            d -= 1
        elif c == '{' and d == 0:
            break
        elif c == ';' and d == 0:
            sig_start = q + 1  # a declaration between the marker and the definition
        q += 1
    bs = q
    d = 0
    while True:
        c = t[q]
        if c == '{':
            d += 1
        elif c == '}':
            d -= 1
            if d == 0:
                break
        elif c in '"\'':
            q2 = q + 1
            while t[q2] != c:
                q2 += 2 if t[q2] == '\\' else 1
            q = q2
        elif t.startswith('//', q):
            q = t.find('\n', q) - 1
        q += 1
    sig = re.sub(r'/\*.*?\*/|//[^\n]*', '', t[sig_start:bs], flags=re.S).strip()
    isvoid = bool(re.match(r'\s*(static\s+|inline\s+|__inline\s+)*void\b', sig)) or (
        not re.search(r'\w[\w\s\*&:<>,]*\s[\*&]*[\w:~]+\s*\(', sig.split('(')[0] + '(') and '::' in sig)
    return bs, q + 1, isvoid


# ---------------------------------------------------------------- edits
class Ed:
    """non-overlapping replacements on the body text"""
    def __init__(self, body):
        self.b = body
        self.r = []

    def rep(self, s, e, txt):
        self.r.append((s, e, txt))
        return self

    def text(self):
        out = self.b
        for s, e, txt in sorted(self.r, key=lambda x: (x[0], x[1]), reverse=True):
            out = out[:s] + txt + out[e:]
        return out


def txt(body, n):
    return body[n.s:n.e]


def is_goto_if(n, L=None):
    if n.kind != 'if' or n.el is not None:
        return None
    th = n.th
    if th.kind == 'block' and len(th.ch) == 1:
        th = th.ch[0]
    if th.kind == 'simple' and th.sk == 'goto' and (L is None or th.target == L):
        return th.target
    return None


def combos(n, choices, cap):
    if n == 0:
        return []
    if len(choices) ** n <= cap:
        return list(itertools.product(choices, repeat=n))
    out = []
    for c in choices[1:]:
        out.append(tuple([c] * n))
        for k in range(n):
            out.append(tuple(c if q == k else choices[0] for q in range(n)))
    return out


def gen_d(body, root, cap):
    """goto-if -> structured arms"""
    out = []
    for blk in lists(root):
        ch = blk.ch
        for j, lab in enumerate(ch):
            if lab.kind != 'label':
                continue
            L = lab.name
            sites = [i for i in range(j) if is_goto_if(ch[i], L)]
            if not sites:
                continue
            ngoto = len(re.findall(r'\bgoto\s+%s\s*;' % re.escape(L), body))
            for cb in combos(len(sites), ('keep', 'neg', 'else', 'and'), cap):
                if all(c == 'keep' for c in cb):
                    continue
                # 'and' joins a site with the next one (adjacent, both converted)
                bad = False
                for q, c in enumerate(cb):
                    if c == 'and' and (q + 1 >= len(sites) or sites[q + 1] != sites[q] + 1 or cb[q + 1] not in ('neg', 'and')):
                        bad = True
                if bad:
                    continue
                # chunks: each statement with the comments and blank lines after it
                seq = [body[ch[k].s:ch[k + 1].s] for k in range(j)] + [txt(body, ch[j])]
                conds = {i: body[ch[i].cond[0]:ch[i].cond[1]] for i in sites}
                jc = j
                pend_and = []
                for q in reversed(range(len(sites))):
                    i, c = sites[q], cb[q]
                    if c == 'keep':
                        continue
                    if c == 'and':
                        # merged into the following neg site already built at i+1: rebuild it
                        nxt = seq[i + 1]
                        m = re.match(r'if \(!\((.*?)\)\) \{\n', nxt, re.S)
                        seq[i + 1] = nxt.replace('if (', 'if (%s && ' % negate(conds[i]), 1)
                        seq = seq[:i] + seq[i + 1:]
                        jc -= 1
                        continue
                    inner = ''.join(seq[i + 1:jc]).rstrip()
                    if not inner.strip():
                        bad = True
                        break
                    tv = seq[i][len(txt(body, ch[i])):] if not seq[i].startswith('if (') or i not in sites else \
                        seq[i][len(txt(body, ch[i])):]
                    if c == 'neg':
                        new = 'if (%s) {%s%s\n}\n' % (negate(conds[i]), tv.rstrip(' ') or '\n', inner.lstrip('\n') if tv.endswith('\n') else inner)
                    else:
                        new = 'if (%s) {\n} else {%s%s\n}\n' % (conds[i], tv.rstrip(' ') or '\n', inner.lstrip('\n') if tv.endswith('\n') else inner)
                    seq = seq[:i] + [new] + seq[jc:]
                    jc = i + 1
                if bad:
                    continue
                kept = sum(1 for c in cb if c == 'keep')
                if ngoto - (len(sites) - kept) == 0:
                    # drop the label (keep a ';' if it ends the block)
                    seq[-1] = ';' if j + 1 == len(ch) else ''
                new = ''.join(seq)
                e = Ed(body).rep(ch[0].s, ch[j].e, new)
                out.append(('d:%s:%s' % (L, ''.join(c[0] for c in cb)), e.text()))
    return out


def arm_ends(n):
    """fallthrough arm ends of an if/else-if chain: list of ('arm', node) or ('else', ifnode)"""
    res = []
    if n.kind != 'if':
        return res
    if not terminates(n.th):
        res.append(('arm', n.th))
    if n.el is None:
        res.append(('else', n))
    elif n.el.kind == 'if':
        res += arm_ends(n.el)
    elif not terminates(n.el):
        res.append(('arm', n.el))
    return res


def append_to(ed, body, kind, node, code):
    if kind == 'else':
        ed.rep(node.e, node.e, ' else {\n%s\n}' % code)
    elif node.kind == 'block':
        ed.rep(node.e - 1, node.e - 1, '%s\n' % code)
    else:
        ed.rep(node.s, node.e, '{\n%s\n%s\n}' % (txt(body, node), code))


def gen_t(body, root, isvoid, cap):
    """shared tails written out at subsets of their predecessors"""
    out = []
    top = root
    for blk in lists(root):
        ch = blk.ch
        # candidate tails: (start index, end index exclusive, code, needs_return)
        tails = []
        for r, n in enumerate(ch):
            if n.kind == 'simple' and n.sk == 'return':
                for s in range(max(0, r - 5), r + 1):
                    tails.append((s, r + 1, False))
        if blk is top:
            for s in range(max(0, len(ch) - 5), len(ch)):
                if not (ch[-1].kind == 'simple' and ch[-1].sk == 'return'):
                    tails.append((s, len(ch), isvoid))
        seen = set()
        for s, e, needret in tails:
            seg = ch[s:e]
            if any(x.kind in ('label', 'case') for x in seg) or not seg:
                if not (seg and seg[0].kind == 'label' and all(x.kind not in ('label', 'case') for x in seg[1:])):
                    continue
            if any(x.kind == 'simple' and x.sk == 'expr' and re.match(r'\s*(const\s+)?(int|char|unsigned|short|long|[A-Z]\w*\s*[\*&]?\s+\w+\s*[=;])', txt(body, x)) for x in seg):
                continue  # declarations would change scope
            if sum(len(norm(txt(body, x))) for x in seg) > 600:
                continue
            lab = seg[0].name if seg[0].kind == 'label' else None
            code_nodes = seg[1:] if lab else seg
            if not code_nodes:
                continue
            code = '\n'.join(txt(body, x) for x in code_nodes) + ('\nreturn;' if needret else '')
            if '"' in code:
                continue  # a copied string literal is a second image literal (units are unpooled)
            preds = []
            if lab:
                for g in walk(root):
                    if g.kind == 'simple' and g.sk == 'goto' and g.target == lab:
                        preds.append(('goto', g))
            if s > 0:
                preds += arm_ends(ch[s - 1])
            if not preds:
                continue
            key0 = (s, e)
            if key0 in seen:
                continue
            seen.add(key0)
            for cb in combos(len(preds), (0, 1), cap):
                if not any(cb):
                    continue
                ed = Ed(body)
                for (k, nd), c in zip(preds, cb):
                    if not c:
                        continue
                    if k == 'goto':
                        ed.rep(nd.s, nd.e, '{\n%s\n}' % code)
                    else:
                        append_to(ed, body, k, nd, code)
                # every predecessor written out: the original copy is dead
                if all(cb) and s > 0 and (ch[s - 1].kind == 'if' or terminates(ch[s - 1])):
                    ed.rep(ch[s].s, ch[e - 1].e, '')
                try:
                    out.append(('t:%d-%d%s:%s' % (s, e, ':' + lab if lab else '', ''.join(map(str, cb))), ed.text()))
                except Exception:
                    pass
    return out


def chain_arms(n):
    """arms of an if/else-if chain with a final else, or None"""
    arms = [n.th]
    while n.el is not None and n.el.kind == 'if':
        n = n.el
        arms.append(n.th)
    if n.el is None:
        return None
    arms.append(n.el)
    return arms


def gen_c(body, root):
    """collapse statements repeated at the end of every arm; sink following statements into every arm"""
    out = []
    for blk in lists(root):
        ch = blk.ch
        for i, n in enumerate(ch):
            if n.kind != 'if':
                continue
            arms = chain_arms(n)
            if not arms:
                continue
            if all(a.kind == 'block' and a.ch for a in arms):
                k = 0
                while all(len(a.ch) > k for a in arms) and len({norm(txt(body, a.ch[-1 - k])) for a in arms}) == 1 and \
                        arms[0].ch[-1 - k].kind == 'simple':
                    k += 1
                for kk in range(1, k + 1):
                    if '"' in ''.join(txt(body, x) for x in arms[0].ch[-kk:]):
                        break
                    ed = Ed(body)
                    for a in arms:
                        ed.rep(a.ch[-kk].s, a.ch[-1].e, '')
                    common = '\n'.join(txt(body, x) for x in arms[0].ch[-kk:])
                    ed.rep(n.e, n.e, '\n' + common)
                    out.append(('c:%d:%d' % (n.s, kk), ed.text()))
            # sink
            for kk in (1, 2):
                seg = ch[i + 1:i + 1 + kk]
                if len(seg) < kk or any(x.kind not in ('simple', 'if', 'block') for x in seg):
                    continue
                if any(x.kind == 'simple' and x.sk == 'expr' and re.match(r'\s*(const\s+)?(int|char|unsigned|short|long|[A-Z]\w*\s*[\*&]?\s+\w+\s*[=;])', txt(body, x)) for x in seg):
                    continue
                if sum(len(norm(txt(body, x))) for x in seg) > 400:
                    continue
                code = '\n'.join(txt(body, x) for x in seg)
                if '"' in code:
                    continue
                ed = Ed(body)
                for a in arms:
                    if terminates(a):
                        continue
                    append_to(ed, body, 'arm', a, code)
                ed.rep(seg[0].s, seg[-1].e, '')
                out.append(('s:%d:%d' % (n.s, kk), ed.text()))
    return out


# ---------------------------------------------------------------- r: field adjust through a reference
SIMPLE_T = r'(?:unsigned\s+|signed\s+)?(?:int|short|char|long)|unsigned|BYTE|WORD|DWORD|uint8_t|uint16_t|uint32_t|int8_t|int16_t|int32_t'
_FTYPE = {}


def field_types(name):
    """declared simple types of a member/global called `name` in the headers (and units)"""
    if name not in _FTYPE:
        import glob
        if not _FTYPE.get('\0'):
            _FTYPE['\0'] = [open(p, encoding='latin-1').read() for p in
                            glob.glob(R + '/src/**/*.h', recursive=True) + glob.glob(R + '/src/**/*.cpp', recursive=True)]
        ts = set()
        for t in _FTYPE['\0']:
            for m in re.finditer(r'\b(%s)\s+(?:\w+\s*,\s*)*%s\s*[;\[,=]' % (SIMPLE_T, re.escape(name)), t):
                ts.add(re.sub(r'\s+', ' ', m.group(1)))
        _FTYPE[name] = sorted(ts) or ['int']
    return _FTYPE[name]


def top_assign(s):
    """index of the top-level plain '=' in an expression statement, or -1"""
    d = 0
    for i, ch in enumerate(s):
        if ch in '([{':
            d += 1
        elif ch in ')]}':
            d -= 1
        elif ch == '=' and d == 0 and s[i - 1:i] not in ('=', '!', '<', '>', '+', '-', '*', '/', '%', '&', '|', '^') \
                and s[i + 1:i + 2] != '=':
            return i
    return -1


def lv_ok(lv):
    v = [x[1] for x in tokenize(lv)]
    return bool(v) and any(x in ('.', '->', '[') for x in v) and '(' not in v and '++' not in lv and '--' not in lv \
        and v[0] not in ('int', 'char', 'short', 'unsigned', 'const', 'return')


def sub_lv(expr, lv, rep):
    """replace every whole occurrence of the token sequence lv in expr; -> (text, count)"""
    et = tokenize(expr)
    lt = [x[1] for x in tokenize(lv)]
    out, last, i, n = [], 0, 0, 0
    while i <= len(et) - len(lt):
        if [x[1] for x in et[i:i + len(lt)]] == lt and (i == 0 or et[i - 1][1] not in ('.', '->')) and \
                (i + len(lt) == len(et) or et[i + len(lt)][1] not in ('.', '->', '[', '(')):
            out.append(expr[last:et[i][2]])
            out.append(rep)
            last = et[i + len(lt) - 1][3]
            i += len(lt)
            n += 1
        else:
            i += 1
    out.append(expr[last:])
    return ''.join(out), n


def gen_r(body, root):
    """a field adjusted through a local copy, directly (`f = g(f)`) or through `T &r = f` (MATCHING_GUIDE 3.7)"""
    out = []
    k = 0
    for blk in lists(root):
        ch = blk.ch
        for i, n in enumerate(ch):
            if n.kind != 'simple' or n.sk != 'expr':
                continue
            s = txt(body, n)
            if '"' in s:
                continue
            # A: `T v = <lv...>; lv = <...v...>;`, v unused after
            m = re.match(r'\s*(?:const\s+)?(%s)\s+(\w+)\s*=\s*(.*);\s*$' % SIMPLE_T, s, re.S)
            if m and i + 1 < len(ch) and ch[i + 1].kind == 'simple':
                s2 = txt(body, ch[i + 1])
                a = top_assign(s2)
                v, rhs1 = m.group(2), m.group(3)
                if a > 0 and s2.rstrip().endswith(';'):
                    lv, rhs2 = s2[:a].strip(), s2[a + 1:].rstrip()[:-1].strip()
                    rest = body[ch[i + 1].e:blk.e]
                    if lv_ok(lv) and sub_lv(rhs1, lv, '')[1] and re.search(r'\b%s\b' % v, rhs2) \
                            and not re.search(r'\b%s\b' % v, rest):
                        ind = re.match(r'\s*', s).group(0)
                        r1ref = sub_lv(rhs1, lv, v)[0]
                        for ty in field_types(re.findall(r'\w+', lv)[-1]):
                            e2 = re.sub(r'\b%s\b' % v, lambda _: '(' + r1ref + ')', rhs2)
                            out.append(('r:%d:ref:%s' % (n.s, ty), Ed(body).rep(n.s, ch[i + 1].e,
                                        '%s%s &%s = %s;\n%s = %s;' % (ind, ty, v, lv, v, e2)).text()))
                        e3 = re.sub(r'\b%s\b' % v, lambda _: '(' + rhs1 + ')', rhs2)
                        out.append(('r:%d:direct' % n.s, Ed(body).rep(n.s, ch[i + 1].e, '%s%s = %s;' % (ind, lv, e3)).text()))
                        continue
            # B: `lv = <...lv...>;`
            a = top_assign(s)
            if a <= 0:
                continue
            lv, rhs = s[:a].strip(), s[a + 1:].rstrip()[:-1].strip()
            if not lv_ok(lv) or re.match(r'(%s)\s' % SIMPLE_T, lv):
                continue
            k += 1
            v = re.findall(r'\w+', lv)[-1]
            if re.search(r'\b%s\b' % v, re.sub(r'(\.|->)\s*%s\b' % v, '', body)):
                v = 'adj%d' % k
            r_ref, cnt = sub_lv(rhs, lv, v)
            if not cnt:
                continue
            ind = re.match(r'\s*', s).group(0)
            for ty in field_types(re.findall(r'\w+', lv)[-1]):
                out.append(('r:%d:bref:%s' % (n.s, ty), Ed(body).rep(n.s, n.e,
                            '%s{\n%s%s &%s = %s;\n%s = %s;\n}' % (ind, ind, ty, v, lv, v, r_ref)).text()))
                out.append(('r:%d:bcopy:%s' % (n.s, ty), Ed(body).rep(n.s, n.e,
                            '%s{\n%s%s %s = %s;\n%s = %s;\n}' % (ind, ind, ty, v, lv, lv, r_ref)).text()))
    return out


# ---------------------------------------------------------------- structural fingerprint
def fingerprint(unit, path, addr, pfx):
    import fe
    text = open(path, encoding='latin-1').read()
    c, err = fe.compile_unit(unit, path, pfx)
    if c is None:
        return None
    r = fe.evaluate(c, text, int(addr, 16))
    if 'error' in r:
        return None
    rows = r['rows']
    def calls(side):
        cnt = collections.Counter()
        for k, o, cc in rows:
            x = o if side == 0 else cc
            if x is not None and x[1].startswith('call'):
                cnt[x[1]] += 1
        return cnt
    ci, co = calls(0), calls(1)
    cdiff = sum(((ci - co) + (co - ci)).values())
    def frame(side):
        ins = [(x[0], x[1].split(' ', 1)[0], x[1].split(' ', 1)[1] if ' ' in x[1] else '', 0)
               for k, o, cc in rows for x in ((o if side == 0 else cc),) if x is not None]
        ins.sort()
        import framescan
        return framescan.frame(ins)
    try:
        fi, fo = frame(0), frame(1)
        fd = abs(fi[0] - fo[0]) + (8 if fi[1] != fo[1] else 0)
    except Exception:
        fd = 0
    cand = 0
    try:
        import candset, regprio
        items, _ = regprio.cod_listing(unit, path, pfx, r['sym'])
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            candset.report(r, text, items, int(addr, 16), sections=('only', 'order'))
        sec = None
        for ln in buf.getvalue().split('\n'):
            if ln.startswith('---'):
                sec = ln
            elif ln.startswith('  ') and sec:
                cand += 1
    except Exception as e:
        print('candset failed', path, repr(e)[:200])
        cand = 999
    try:
        import callees
        named = callees.named_diff(c, text, int(addr, 16))
    except Exception as e:
        print('callees failed', repr(e)[:200])
        named = None
    if named is None:
        named = 999
    return dict(named=named, calls=cdiff, frame=fd, cand=cand, rows=r['diff'], exact=r['exact'])


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('unit')
    ap.add_argument('addr')
    ap.add_argument('--max', type=int, default=80)
    ap.add_argument('--cap', type=int, default=27)
    ap.add_argument('--top', type=int, default=6)
    ap.add_argument('--only', default='d,t,c,r')
    ap.add_argument('--write', action='store_true')
    ap.add_argument('--show')
    ap.add_argument('--out')
    ap.add_argument('--fp')
    ap.add_argument('--pfx', type=int)
    a = ap.parse_args()
    unit = a.unit if a.unit.startswith('src/') else 'src/' + a.unit
    os.chdir(R)
    if a.fp:
        import json
        print(json.dumps(fingerprint(unit, a.fp, a.addr, a.pfx)))
        return 0
    t = open(unit, encoding='latin-1').read()
    bs, be, isvoid = locate(t, a.addr)
    body = t[bs + 1:be - 1]
    try:
        root = N('block', 0, len(body), ch=Parser(body).stmts(end='\x00'))
    except Exception as e:
        print('PARSE', a.addr, e)
        return 2
    only = set(a.only.split(','))
    vs = []
    if 'd' in only:
        vs += gen_d(body, root, a.cap)
    if 't' in only:
        vs += gen_t(body, root, isvoid, a.cap)
    if 'c' in only:
        vs += gen_c(body, root)
    if 'r' in only:
        vs += gen_r(body, root)
    seen = {norm(body)}
    deck = [('base', t)]
    for k, b in vs:
        nb = norm(b)
        if nb in seen:
            continue
        seen.add(nb)
        deck.append((k, t[:bs + 1] + reindent(body, b) + t[be - 1:]))
    # round-robin over sites so a long function's first labels do not take the whole budget
    groups = collections.OrderedDict()
    for k, v in deck[1:]:
        groups.setdefault(k.rsplit(':', 1)[0], []).append((k, v))
    rr = []
    while groups and len(rr) < a.max:
        for g in list(groups):
            if groups[g]:
                rr.append(groups[g].pop(0))
            if not groups[g]:
                del groups[g]
    deck = [deck[0]] + rr[:a.max]
    if a.show:
        import difflib
        for ln in difflib.unified_diff(t.split("\n"), dict(deck)[a.show].split("\n"), lineterm="", n=2):
            print(ln)
        return 0
    print('variants', len(deck) - 1)
    if len(deck) == 1:
        return 0
    jobs = int(os.environ.get('FE_JOBS', '5'))
    res = perm.run_deck(unit, a.addr, deck, jobs=jobs)
    res.sort(key=lambda r: r[1])
    base = [r for r in res if r[3] == 'base'][0]
    pfx = int(os.environ.get('FE_PFX_BASE', '8'))
    d = R + '/build/recovery/fe/perm'
    top = [r for r in res if r[3] != 'base' and r[1] < 9999][:a.top]
    cand = [base] + top
    paths = []
    for q, r in enumerate(cand):
        p = '%s/fp%d_%d.cpp' % (d, pfx, q)
        open(p, 'w', encoding='latin-1').write(dict(deck)[r[3]])
        paths.append(p)
    from concurrent.futures import ThreadPoolExecutor
    import queue
    lanes = queue.Queue()
    for k in range(min(jobs, len(cand))):
        lanes.put(pfx + k)
    def fp(q):
        ln = lanes.get()
        try:
            import subprocess, json
            o = subprocess.run(['uv', 'run', 'python', os.path.abspath(__file__), unit, a.addr, '--fp', paths[q], '--pfx', str(ln)],
                               cwd=R, capture_output=True, text=True).stdout.strip().split('\n')[-1]
            try:
                return json.loads(o)
            except Exception:
                return None
        finally:
            lanes.put(ln)
    with ThreadPoolExecutor(min(jobs, len(cand))) as ex:
        fps = list(ex.map(fp, range(len(cand))))
    rank = []
    for r, f in zip(cand, fps):
        if f is None:
            continue
        rank.append(((f['named'], f['calls'], f['frame'], f['cand'], f['rows']), r[3], f))
    bf = [x for x in rank if x[1] == 'base']
    if not bf:
        print('base fingerprint failed')
        return 2
    bk = bf[0][0]
    slack = max(5, bk[4] // 7)
    # rows may rise (within the slack) only for a callee, call-count or frame win; a candset-only win keeps rows
    rank = [x for x in rank if x[1] == 'base' or (x[2]['frame'] <= bk[2] and (
        x[0][4] <= bk[4] or (x[0][:3] < bk[:3] and x[0][4] <= bk[4] + slack)))]
    rank.sort(key=lambda x: x[0])
    for k, key, f in rank:
        print('%-28s named %d calls %d frame %d cand %d rows %d%s' % (key[:28], k[0], k[1], k[2], k[3], k[4], ' EXACT' if f['exact'] else ''))
    best = rank[0]
    print('BASE %s %d BEST %s %d key %s' % (a.addr, bk[4], best[1], best[0][4], best[0]))
    if best[1] != 'base' and best[0] < bk:
        if a.out:
            open(a.out, 'w', encoding='latin-1').write(dict(deck)[best[1]])
        print('KEY', ' '.join(map(str, bk)), '->', ' '.join(map(str, best[0])))
        if a.write:
            open(unit, 'w', encoding='latin-1').write(dict(deck)[best[1]])
            print('WROTE', best[1])
    return 0


if __name__ == '__main__':
    sys.exit(main())
