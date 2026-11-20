"""refcount.py: per-variable SOURCE reference counts of one recovered function (MATCHING_GUIDE 3.3, frame-slot
packing count).  VC6 ranks packing units by a reference count that includes register-resident references after
copy propagation, not only frame accesses (battle_fight rx: `rx = x;` with x at every later site is ONE reference,
the all-rx loop about 13).  Lexical occurrences in our source are that count's first approximation: a copy
`rx = x;` already writes the later uses as x.

  import refcount; RC = refcount.Counts(text, addr)
  RC.count('rx')          -> lexical references of rx in the function body (declaration without initialiser excluded)
  RC.count('c#123', lines) -> block-scope variable (VC6 `_c$123`): occurrences of c in the block that declares it,
                              chosen by the source lines of its accesses
  RC.lines(name)          -> Counter line -> references
compound=True counts `x += k`, `++x`, `x++` as two references (a use and a definition)."""
import re, collections

KW_OP = {'return', 'case', 'sizeof', 'else', 'delete', 'throw', 'do', 'new', 'goto'}
TYPEKW = {'int', 'unsigned', 'signed', 'char', 'short', 'long', 'bool', 'float', 'double', 'void', 'const',
          'static', 'register', 'volatile', 'struct', 'class', 'enum', 'auto'}
ID = re.compile(r'[A-Za-z_]\w*')


def _strip(text):
    """comments and string/char literals -> spaces (newlines kept)."""
    out, i, n = [], 0, len(text)
    while i < n:
        c = text[i]
        if text.startswith('//', i):
            j = text.find('\n', i)
            j = n if j < 0 else j
            out.append(' ' * (j - i))
            i = j
        elif text.startswith('/*', i):
            j = text.find('*/', i + 2)
            j = n if j < 0 else j + 2
            out.append(re.sub(r'[^\n]', ' ', text[i:j]))
            i = j
        elif c in '"\'':
            j = i + 1
            while j < n and text[j] != c:
                j += 2 if text[j] == '\\' else 1
            j = min(j + 1, n)
            out.append(' ' * (j - i))
            i = j
        else:
            out.append(c)
            i += 1
    return ''.join(out)


class Counts:
    def __init__(self, text, addr, compound=True):
        self.compound = compound
        s = _strip(text)
        m = re.search(r'// ORIGINAL: 0x%08X ' % addr, text)
        if not m:
            raise ValueError('no marker %08X' % addr)
        # body: first '{' at paren depth 0 after the marker block, to its match
        p = m.end()
        depth = 0
        while p < len(s):
            ch = s[p]
            if ch == '(':
                depth += 1
            elif ch == ')':
                depth -= 1
            elif ch == '{' and depth == 0:
                break
            elif ch == ';' and depth == 0:
                # a declaration before the definition (prototype): keep looking
                pass
            p += 1
        b0 = p
        d = 0
        while p < len(s):
            if s[p] == '{':
                d += 1
            elif s[p] == '}':
                d -= 1
                if d == 0:
                    break
            p += 1
        self.s, self.b0, self.b1 = s, b0, p
        self.line0 = s.count('\n', 0, b0) + 1
        self.occ = collections.defaultdict(list)  # name -> [(pos, line, weight)]
        body = s[b0:p]
        line = self.line0
        last = 0
        for mm in ID.finditer(body):
            line += body.count('\n', last, mm.start())
            last = mm.start()
            nm = mm.group(0)
            pre = body[max(0, mm.start() - 80):mm.start()].rstrip()
            if pre.endswith('.') or pre.endswith('->') or pre.endswith('::'):
                continue
            post = body[mm.end():mm.end() + 24].lstrip()
            if post.startswith('::') or post.startswith('('):
                continue
            # declaration: previous token is a type-ish identifier (or * / & after one) and no initialiser
            pm = re.search(r'([A-Za-z_]\w*|[*&>])\s*[*&]*\s*$', pre[-60:])
            prev_tok = pm.group(1) if pm else ''
            is_decl = bool(prev_tok) and (prev_tok in TYPEKW or (ID.fullmatch(prev_tok) and prev_tok not in KW_OP
                                                                   and not pre.endswith(')')))
            if is_decl and not post.startswith('=') and not post.startswith('('):
                if post[:1] in (';', ',', '[', ')'):
                    self.occ[nm].append((b0 + mm.start(), line, 0))  # declaration marker, no reference
                    continue
            w = 1
            if compound and (re.match(r'(\+\+|--|[-+*/%&|^]=|<<=|>>=)', post) or pre.endswith('++') or pre.endswith('--')):
                w = 2
            self.occ[nm].append((b0 + mm.start(), line, w))

    def _block(self, name, lines):
        """occurrences of the block-scope variable declared nearest before the first of `lines`."""
        occ = self.occ.get(name, [])
        if not lines:
            return occ
        first = min(lines)
        s = self.s
        # declaration sites: occurrences preceded by a type token
        decls = []
        for pos, ln, w in occ:
            pre = s[max(self.b0, pos - 60):pos].rstrip()
            pm = re.search(r'([A-Za-z_]\w*)\s*[*&]*\s*$', pre)
            if pm and (pm.group(1) in TYPEKW or (pm.group(1) not in KW_OP and not pre.endswith(')')
                                                 and re.search(r'[;{}(]\s*[\w:<>]+\s*[*&]*\s*$', pre))):
                decls.append((pos, ln))
        cand = [d for d in decls if d[1] <= first]
        if not cand:
            return occ
        dpos = max(cand)[0]
        # scope: to the close of the enclosing block (VC6 for-scope runs to the enclosing block too)
        depth, p = 0, dpos
        while p < self.b1:
            if s[p] == '{':
                depth += 1
            elif s[p] == '}':
                depth -= 1
                if depth < 0:
                    break
            p += 1
        return [o for o in occ if dpos <= o[0] <= p]

    def count(self, name, lines=None):
        base = name.split('#')[0]
        occ = self._block(base, lines) if '#' in name else self.occ.get(base, [])
        return sum(w for _, _, w in occ)

    def lines(self, name, lines=None):
        base = name.split('#')[0]
        occ = self._block(base, lines) if '#' in name else self.occ.get(base, [])
        c = collections.Counter()
        for _, ln, w in occ:
            c[ln] += w
        return c
