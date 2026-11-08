"""Source-spelling mutations and a seeded search over one function body.

Pure text transforms plus a scorer-agnostic hill-climbing/annealing loop. The
loop never decides what counts as a match: the caller's scorer returns a
distance, and only its own ``exact`` verdict ends the search. Nothing here
reads or writes files or invokes a compiler; ``source_permute`` owns that.

The transforms are token-level over a C++ function body. They aim to keep the
program meaning (statement swaps require a conservative independence check),
but they are not proofs of equivalence: the byte comparison is the test, and a
variant that does not compile is discarded by the caller's scorer.
"""

from __future__ import annotations

from dataclasses import dataclass, field
import math
import random
import re
import time

# --------------------------------------------------------------- tokenizer

_PUNCT = sorted("""
>>= <<= ... ->* :: -> ++ -- << >> <= >= == != && || += -= *= /= %= &= |= ^= .*
{ } [ ] ( ) ; : , . ? ~ ! + - * / % ^ & | = < > #
""".split(), key=len, reverse=True)

_TOKEN = re.compile(
    r"(?P<ws>[ \t\r\n\f\v]+)"
    r"|(?P<comment>//[^\n]*|/\*.*?\*/)"
    r"|(?P<string>L?\"(?:\\.|[^\"\\\n])*\")"
    r"|(?P<char>L?'(?:\\.|[^'\\\n])*')"
    r"|(?P<number>(?:0[xX][0-9a-fA-F]+|\d+\.?\d*(?:[eE][+-]?\d+)?|\.\d+(?:[eE][+-]?\d+)?)[uUlLfFiI0-9]*)"
    r"|(?P<ident>[A-Za-z_$][A-Za-z_0-9$]*)"
    r"|(?P<punct>" + "|".join(re.escape(p) for p in _PUNCT) + ")",
    re.S)


@dataclass(frozen=True)
class Token:
    kind: str           # ws, comment, string, char, number, ident, punct, pp
    text: str
    start: int
    end: int


class Unsupported(ValueError):
    """The body uses syntax the transforms refuse to reason about."""


def tokenize(text: str) -> list[Token]:
    tokens, index, line_start = [], 0, True
    while index < len(text):
        if line_start and text[index] == "#":
            end = text.find("\n", index)
            end = len(text) if end < 0 else end
            if text[index:end].rstrip().endswith("\\"):
                raise Unsupported("preprocessor line splicing is not supported")
            tokens.append(Token("pp", text[index:end], index, end))
            index, line_start = end, False
            continue
        match = _TOKEN.match(text, index)
        if match is None:
            raise Unsupported(f"cannot tokenize body near offset {index}: {text[index:index + 20]!r}")
        kind = match.lastgroup
        value = match.group()
        if kind == "ident" and value in ("R", "LR", "u8R", "uR", "UR") and text.startswith('"', match.end()):
            raise Unsupported("raw string literals are not supported")
        tokens.append(Token(kind, value, index, match.end()))
        if kind == "ws":
            line_start = line_start or "\n" in value
        elif kind != "comment":
            line_start = False
        index = match.end()
    return tokens


# ------------------------------------------------------------ vocabulary

KEYWORDS = frozenset("""
if else for while do switch case default break continue return goto sizeof new delete
throw try catch __try __except __finally this operator typedef static_cast reinterpret_cast
const_cast dynamic_cast typename template using namespace public private protected
""".split())
TYPE_WORDS = frozenset("""
int unsigned signed char short long bool float double void const volatile static register
struct class enum union auto __int8 __int16 __int32 __int64 wchar_t
""".split())
_JUMPS = frozenset(("return", "break", "continue", "goto", "throw"))
_LITERALS = frozenset(("this", "true", "false", "nullptr", "NULL"))
_CASTS = frozenset(("static_cast", "reinterpret_cast", "const_cast", "dynamic_cast"))

BINARY_PRECEDENCE = {
    "*": 13, "/": 13, "%": 13, "+": 12, "-": 12, "<<": 11, ">>": 11,
    "<": 10, "<=": 10, ">": 10, ">=": 10, "==": 9, "!=": 9, "&": 8, "^": 7, "|": 6,
    "&&": 5, "||": 4,
}
ASSIGNMENTS = frozenset(("=", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>="))
COMMUTATIVE = frozenset(("+", "*", "&", "|", "^", "==", "!="))
_FLIP = {"<": ">", ">": "<", "<=": ">=", ">=": "<="}
_COMPOUND = {op + "=": op for op in ("+", "-", "*", "/", "%", "&", "|", "^", "<<", ">>")}
_OPEN, _CLOSE = "([{", ")]}"


def _is_operand_end(token: Token | None) -> bool:
    if token is None:
        return False
    if token.kind in ("number", "string", "char"):
        return True
    if token.kind == "ident":
        return token.text not in KEYWORDS or token.text == "this"
    return token.text in (")", "]")


def _is_operand_start(token: Token | None) -> bool:
    if token is None:
        return False
    if token.kind in ("number", "string", "char"):
        return True
    if token.kind == "ident":
        return token.text not in KEYWORDS or token.text in ("this", "sizeof") or token.text in _CASTS
    return token.text in ("(", "!", "~", "-", "+", "*", "&", "++", "--", "::")


# ------------------------------------------------------------ statements

@dataclass
class Node:
    kind: str                   # block simple decl jump label pp if for while do switch opaque
    start: int                  # significant-token index, inclusive
    end: int                    # exclusive
    children: list = field(default_factory=list)        # block statements
    then: "Node | None" = None
    other: "Node | None" = None   # else branch
    body: "Node | None" = None
    cond: tuple | None = None     # (start, end) inside parentheses
    init: tuple | None = None
    step: tuple | None = None


class Body:
    """A tokenized, statement-parsed function body (the text inside its braces)."""

    def __init__(self, text: str, parameters: str = ""):
        self.text = text
        self.tokens = tokenize(text)
        self.sig = [t for t in self.tokens if t.kind not in ("ws", "comment")]
        self.match = self._brackets()
        self.opening = {close: open_ for open_, close in self.match.items()}
        self.statements = self._parse_list(0, len(self.sig))
        self.locals = set(_parameter_names(parameters))
        for node in self.walk():
            if node.kind == "decl":
                self.locals.update(self.declared(node))
            if node.kind == "for" and node.init and self._is_decl(*node.init):
                self.locals.update(self._declared_names(node.init[0], node.init[1]))

    # -- structure -----------------------------------------------------
    def _brackets(self) -> dict[int, int]:
        stack, match = [], {}
        for index, token in enumerate(self.sig):
            if token.kind != "punct":
                continue
            if token.text in _OPEN:
                stack.append(index)
            elif token.text in _CLOSE:
                if not stack or _OPEN[_CLOSE.index(token.text)] != self.sig[stack[-1]].text:
                    raise Unsupported("unbalanced brackets in function body")
                match[stack.pop()] = index
        if stack:
            raise Unsupported("unbalanced brackets in function body")
        return match

    def _parse_list(self, start: int, end: int) -> list[Node]:
        nodes, index = [], start
        while index < end:
            node = self._statement(index, end)
            nodes.append(node)
            index = node.end
        return nodes

    def _paren(self, index: int) -> int:
        if index >= len(self.sig) or self.sig[index].text != "(":
            raise Unsupported("expected a parenthesized condition")
        return self.match[index]

    def _statement(self, index: int, end: int) -> Node:
        token = self.sig[index]
        text = token.text
        if token.kind == "pp":
            return Node("pp", index, index + 1)
        if text == "{":
            close = self.match[index]
            return Node("block", index, close + 1, children=self._parse_list(index + 1, close))
        if token.kind == "ident" and text == "if":
            close = self._paren(index + 1)
            then = self._statement(close + 1, end)
            node = Node("if", index, then.end, then=then, cond=(index + 2, close))
            if then.end < end and self.sig[then.end].text == "else":
                other = self._statement(then.end + 1, end)
                node.other, node.end = other, other.end
            return node
        if token.kind == "ident" and text in ("while", "switch"):
            close = self._paren(index + 1)
            body = self._statement(close + 1, end)
            return Node(text, index, body.end, body=body, cond=(index + 2, close))
        if token.kind == "ident" and text == "for":
            close = self._paren(index + 1)
            parts, depth, mark = [], 0, index + 2
            for at in range(index + 2, close):
                value = self.sig[at].text
                if value in _OPEN:
                    depth += 1
                elif value in _CLOSE:
                    depth -= 1
                elif value == ";" and depth == 0:
                    parts.append((mark, at))
                    mark = at + 1
            parts.append((mark, close))
            if len(parts) != 3:
                raise Unsupported("unsupported for-statement header")
            body = self._statement(close + 1, end)
            return Node("for", index, body.end, body=body, init=parts[0], cond=parts[1], step=parts[2])
        if token.kind == "ident" and text == "do":
            body = self._statement(index + 1, end)
            if body.end >= end or self.sig[body.end].text != "while":
                raise Unsupported("do statement without while")
            close = self._paren(body.end + 1)
            if close + 1 >= end or self.sig[close + 1].text != ";":
                raise Unsupported("do-while without semicolon")
            return Node("do", index, close + 2, body=body, cond=(body.end + 2, close))
        if token.kind == "ident" and text in ("try", "__try"):
            at = self.match[index + 1] + 1 if index + 1 < end and self.sig[index + 1].text == "{" else None
            if at is None:
                raise Unsupported("unsupported try statement")
            while at < end and self.sig[at].text in ("catch", "__except", "__finally"):
                if self.sig[at].text != "__finally":
                    at = self._paren(at + 1) + 1
                else:
                    at += 1
                if at >= end or self.sig[at].text != "{":
                    raise Unsupported("unsupported handler")
                at = self.match[at] + 1
            return Node("opaque", index, at)
        if text == "else":
            raise Unsupported("else without if")
        if token.kind == "ident" and text in ("case", "default"):
            at = index
            while at < end and self.sig[at].text != ":":
                at += 1
            return Node("label", index, at + 1)
        if (token.kind == "ident" and text not in KEYWORDS and text not in TYPE_WORDS
                and index + 1 < end and self.sig[index + 1].text == ":"):
            return Node("label", index, index + 2)
        at = index
        while at < end and self.sig[at].text != ";":
            at = self.match[at] + 1 if self.sig[at].text in _OPEN else at + 1
        if at >= end:
            raise Unsupported("statement without semicolon")
        if text in _JUMPS:
            kind = "jump"
        elif text == ";":
            kind = "empty"
        else:
            kind = "decl" if self._is_decl(index, at) else "simple"
        return Node(kind, index, at + 1)

    def walk(self, nodes=None):
        for node in self.statements if nodes is None else nodes:
            yield node
            yield from self.walk(node.children)
            for child in (node.then, node.other, node.body):
                if child is not None:
                    yield from self.walk([child])

    def blocks(self):
        """Every statement list, the outer body first."""
        yield self.statements
        for node in self.walk():
            if node.kind == "block":
                yield node.children

    # -- declarations ----------------------------------------------------
    def _is_decl(self, start: int, end: int) -> bool:
        if start >= end:
            return False
        first = self.sig[start]
        if first.kind != "ident":
            return False
        if first.text in TYPE_WORDS:
            return True
        if first.text in KEYWORDS:
            return False
        at = start + 1
        while at + 1 < end and self.sig[at].text == "::" and self.sig[at + 1].kind == "ident":
            at += 2
        if at < end and self.sig[at].text == "<":
            depth = 0
            while at < end:
                depth += {"<": 1, ">": -1, ">>": -2}.get(self.sig[at].text, 0)
                at += 1
                if depth <= 0:
                    break
        while at < end and self.sig[at].text in ("*", "&", "const", "volatile"):
            at += 1
        return (at < end and self.sig[at].kind == "ident" and self.sig[at].text not in KEYWORDS
                and at + 1 <= end and (at + 1 == end or self.sig[at + 1].text in ("=", ";", ",", "[", "(")))

    def _declarator_heads(self, start: int, end: int) -> list[tuple[int, int]]:
        """(head start, name index) per declarator; the head excludes initializers."""
        heads, at, mark = [], start, start
        while at <= end:
            value = self.sig[at].text if at < end else ";"
            if at < end and value in _OPEN and value != "(":
                at = self.match[at] + 1
                continue
            if value in ("=", ",", ";", "(") or at == end:
                names = [i for i in range(mark, at) if self.sig[i].kind == "ident"
                         and self.sig[i].text not in TYPE_WORDS and self.sig[i].text not in KEYWORDS]
                if names:
                    heads.append((mark, names[-1]))
                # skip the initializer up to the next top-level comma
                while at < end and self.sig[at].text not in (",", ";"):
                    at = self.match[at] + 1 if self.sig[at].text in _OPEN else at + 1
                at += 1
                mark = at
                continue
            at += 1
        return heads

    def _declared_names(self, start: int, end: int) -> list[str]:
        return [self.sig[name].text for _, name in self._declarator_heads(start, end)]

    def declared(self, node: Node) -> list[str]:
        return self._declared_names(node.start, node.end - 1)

    def decl_head_end(self, node: Node) -> int:
        """Index of the first declarator's `=`/`(`/`;` - the type part ends before it."""
        heads = self._declarator_heads(node.start, node.end - 1)
        return heads[0][1] + 1 if heads else node.start

    # -- text --------------------------------------------------------------
    def span(self, start: int, end: int) -> tuple[int, int]:
        """Character span of significant tokens [start, end)."""
        return self.sig[start].start, self.sig[end - 1].end

    def source(self, start: int, end: int) -> str:
        low, high = self.span(start, end)
        return self.text[low:high]

    def binary_at(self, index: int) -> bool:
        token = self.sig[index]
        if token.kind != "punct" or token.text not in BINARY_PRECEDENCE:
            return False
        previous = self.sig[index - 1] if index > 0 else None
        if not _is_operand_end(previous):
            # `x++ + y`: a postfix operator also ends an operand.
            if previous is None or previous.text not in ("++", "--") or index < 2 \
                    or not _is_operand_end(self.sig[index - 2]):
                return False
        if previous is not None and previous.text == ")" and token.text in ("*", "&", "-", "+"):
            opening = self.opening[index - 1]
            if _looks_like_cast(self.sig[opening + 1:index - 1]):
                return False
        if index + 1 >= len(self.sig) or not _is_operand_start(self.sig[index + 1]):
            return False
        if token.text in ("<", ">") and self._in_template(index):
            return False
        return True

    def _in_template(self, index: int) -> bool:
        for at in range(index - 1, max(-1, index - 12), -1):
            value = self.sig[at].text
            if value in (";", "{", "}", "(", ")"):
                return False
            if value == "<" and at > 0 and self.sig[at - 1].text in _CASTS | {"template"}:
                return True
        return index > 0 and self.sig[index - 1].text in _CASTS | {"template"}

    def operand_left(self, index: int, precedence: int, floor: int = 0) -> int:
        """First token index of the left operand of the binary operator at `index`."""
        at = index - 1
        while at >= floor:
            token = self.sig[at]
            value = token.text
            if value in _CLOSE:
                at = self.opening[at] - 1
                continue
            if value in _OPEN or value in (";", ",", "?", ":") or token.kind == "pp":
                break
            if value in ASSIGNMENTS or (token.kind == "ident" and value in KEYWORDS
                                        and value not in _CASTS | {"this", "sizeof"}):
                break
            if value in BINARY_PRECEDENCE and self.binary_at(at) and BINARY_PRECEDENCE[value] < precedence:
                break
            at -= 1
        return at + 1

    def operand_right(self, index: int, precedence: int, ceiling: int | None = None) -> int:
        """Exclusive end of the right operand of the binary operator at `index`."""
        ceiling = len(self.sig) if ceiling is None else ceiling
        at = index + 1
        while at < ceiling:
            token = self.sig[at]
            value = token.text
            if value in _OPEN:
                at = self.match[at] + 1
                continue
            if value in _CLOSE or value in (";", ",", "?", ":") or token.kind == "pp":
                break
            if value in ASSIGNMENTS:
                break
            if value in BINARY_PRECEDENCE and self.binary_at(at) and BINARY_PRECEDENCE[value] <= precedence:
                break
            at += 1
        return at

    def top_level_operators(self, start: int, end: int) -> list[int]:
        found, at = [], start
        while at < end:
            value = self.sig[at].text
            if value in _OPEN:
                at = self.match[at] + 1
                continue
            if value in BINARY_PRECEDENCE and self.binary_at(at):
                found.append(at)
            at += 1
        return found

    def has_top(self, start: int, end: int, values) -> bool:
        at = start
        while at < end:
            value = self.sig[at].text
            if value in _OPEN:
                at = self.match[at] + 1
                continue
            if value in values:
                return True
            at += 1
        return False

    # -- effects -------------------------------------------------------------
    def effects(self, start: int, end: int) -> dict:
        """Conservative names/writes/calls/memory for tokens [start, end)."""
        names, writes = set(), set()
        call = mem_read = mem_write = False
        for at in range(start, end):
            token = self.sig[at]
            following = self.sig[at + 1].text if at + 1 < end else ""
            if token.kind == "ident" and token.text not in KEYWORDS and token.text not in TYPE_WORDS:
                names.add(token.text)
                if following == "(":
                    call = True
                elif (token.text not in self.locals and token.text not in _LITERALS
                      and not token.text.isupper()):
                    mem_read = True
            if token.text in ("->", "[") or (token.text == "*" and not self.binary_at(at)):
                mem_read = True
            if token.text == ")" and following == "(":
                call = True
            if token.text in ("new", "delete", "throw"):
                call = True
            if token.text in ASSIGNMENTS or token.text in ("++", "--"):
                if token.text in ("++", "--"):
                    if at > start and _is_operand_end(self.sig[at - 1]):
                        low, high = self.operand_left(at, 15, start), at
                    else:
                        low, high = at + 1, self.operand_right(at, 14, end)
                else:
                    low, high = self.operand_left(at, 2, start), at
                target = [self.sig[i] for i in range(low, high)]
                writes.update(t.text for t in target if t.kind == "ident")
                roots = [t for t in target if t.kind == "ident" and t.text not in KEYWORDS]
                if (any(t.text in ("->", "[", "*") for t in target)
                        or not roots or roots[0].text not in self.locals):
                    mem_write = True
        return {"names": names, "writes": writes, "call": call,
                "mem_read": mem_read, "mem_write": mem_write}

    def node_effects(self, node: Node) -> dict:
        if node.kind != "decl":
            return self.effects(node.start, node.end)
        # A declaration writes its names; only its initializers are expressions.
        total = {"names": set(), "writes": set(), "call": False, "mem_read": False, "mem_write": False}
        for _head, name in self._declarator_heads(node.start, node.end - 1):
            at = name + 1
            if at < node.end and self.sig[at].text == "[":
                at = self.match[at] + 1
            low = at + 1 if at < node.end and self.sig[at].text == "=" else at
            high = low
            while high < node.end - 1 and self.sig[high].text != ",":
                high = self.match[high] + 1 if self.sig[high].text in _OPEN else high + 1
            part = self.effects(low, high)
            for key in ("names", "writes"):
                total[key] |= part[key]
            for key in ("call", "mem_read", "mem_write"):
                total[key] |= part[key]
            total["names"].add(self.sig[name].text)
            total["writes"].add(self.sig[name].text)
        return total

    def independent(self, first: Node, second: Node) -> bool:
        a, b = self.node_effects(first), self.node_effects(second)
        if a["writes"] & b["names"] or b["writes"] & a["names"]:
            return False
        for x, y in ((a, b), (b, a)):
            if x["call"] and (y["call"] or y["mem_read"] or y["mem_write"]):
                return False
            if x["mem_write"] and (y["mem_read"] or y["mem_write"]):
                return False
        return True


def _looks_like_cast(tokens) -> bool:
    if not tokens:
        return False
    if any(t.kind not in ("ident", "punct") for t in tokens):
        return False
    if any(t.kind == "punct" and t.text not in ("*", "&", "::", "<", ">") for t in tokens):
        return False
    words = [t for t in tokens if t.kind == "ident"]
    if not words or any(t.text in KEYWORDS for t in words):
        return False
    return any(t.text in TYPE_WORDS for t in words) or any(t.text in ("*", "&") for t in tokens) \
        or (len(tokens) == 1 and tokens[0].text[:1].isupper() and not tokens[0].text.isupper())


def _parameter_names(parameters: str) -> list[str]:
    try:
        tokens = [t for t in tokenize(parameters) if t.kind not in ("ws", "comment")]
    except Unsupported:
        return []
    names, depth, last = [], 0, None
    for token in tokens + [Token("punct", ",", 0, 0)]:
        if token.text in "([<":
            depth += 1
        elif token.text in ")]>":
            depth -= 1
        elif token.text in (",", "=") and depth == 0:
            if last is not None:
                names.append(last)
            last = None
            if token.text == "=":
                depth += 100     # skip default argument until the next comma
            continue
        if token.kind == "ident" and token.text not in TYPE_WORDS and token.text not in KEYWORDS and depth == 0:
            last = token.text
        if token.text == "," and depth >= 100:
            depth -= 100
    return names


# ------------------------------------------------------------- mutations

@dataclass(frozen=True)
class Mutation:
    operator: str
    start: int          # character offsets into the body text
    end: int
    replacement: str
    description: str

    def apply(self, text: str) -> str:
        return text[:self.start] + self.replacement + text[self.end:]


def _edit(body: Body, operator: str, start: int, end: int, replacement: str, note: str) -> Mutation:
    low, high = body.span(start, end)
    return Mutation(operator, low, high, replacement, note)


def _wrap(text: str) -> str:
    return "(" + text + ")"


def _braced(body: Body, node: Node) -> str:
    text = body.source(node.start, node.end)
    return text if node.kind == "block" else "{ " + text + " }"


def _protected(body: Body) -> set[int]:
    """Token indices no expression mutation may touch: declaration type parts, pp lines, labels."""
    protected = set()
    for node in body.walk():
        if node.kind == "decl":
            protected.update(range(node.start, body.decl_head_end(node)))
        elif node.kind in ("pp", "label", "opaque"):
            protected.update(range(node.start, node.end))
    return protected


def swap_adjacent(body: Body) -> list[Mutation]:
    sites = []
    movable = ("simple", "decl")
    for statements in body.blocks():
        for first, second in zip(statements, statements[1:]):
            if first.kind in movable and second.kind in movable and body.independent(first, second):
                low, _ = body.span(first.start, first.end)
                _, high = body.span(second.start, second.end)
                a, b = body.source(first.start, first.end), body.source(second.start, second.end)
                middle = body.text[body.span(first.start, first.end)[1]:body.span(second.start, second.end)[0]]
                sites.append(Mutation("swap_statements", low, high, b + middle + a,
                                      f"swap `{a[:40]}` with `{b[:40]}`"))
    return sites


def reorder_declarations(body: Body) -> list[Mutation]:
    """Exchange two (not necessarily adjacent) declarations in one block."""
    sites = []
    for statements in body.blocks():
        decls = [index for index, node in enumerate(statements) if node.kind == "decl"]
        for i, left in enumerate(decls):
            for right in decls[i + 1:]:
                if right == left + 1:
                    continue        # swap_adjacent covers neighbours
                a, b = statements[left], statements[right]
                between = statements[left + 1:right]
                if not (body.independent(a, b) and all(body.independent(a, x) and body.independent(b, x)
                                                       for x in between)):
                    continue
                if any(n.kind not in ("simple", "decl", "empty") for n in between):
                    continue
                a_low, a_high = body.span(a.start, a.end)
                b_low, b_high = body.span(b.start, b.end)
                text = (body.source(b.start, b.end) + body.text[a_high:b_low]
                        + body.source(a.start, a.end))
                sites.append(Mutation("reorder_declarations", a_low, b_high, text,
                                      f"exchange declarations of {body.declared(a)} and {body.declared(b)}"))
    return sites


def commute(body: Body) -> list[Mutation]:
    sites, protected = [], _protected(body)
    for index, token in enumerate(body.sig):
        if index in protected or token.text not in COMMUTATIVE or not body.binary_at(index):
            continue
        precedence = BINARY_PRECEDENCE[token.text]
        low = body.operand_left(index, precedence)
        high = body.operand_right(index, precedence)
        if low >= index or high <= index + 1 or protected & set(range(low, high)):
            continue
        left, right = body.source(low, index), body.source(index + 1, high)
        same = [at for at in body.top_level_operators(low, index)
                if BINARY_PRECEDENCE[body.sig[at].text] == precedence]
        if same:
            left = _wrap(left)          # `a - b + c` -> `c + (a - b)`
        a_low, _ = body.span(low, index)
        _, b_high = body.span(index + 1, high)
        sites.append(Mutation("commute", a_low, b_high, f"{right} {token.text} {left}",
                              f"commute `{body.source(low, high)[:60]}`"))
    return sites


def flip_comparison(body: Body) -> list[Mutation]:
    sites, protected = [], _protected(body)
    for index, token in enumerate(body.sig):
        if index in protected or token.text not in _FLIP or not body.binary_at(index):
            continue
        low = body.operand_left(index, 10)
        high = body.operand_right(index, 10)
        if low >= index or high <= index + 1:
            continue
        if any(BINARY_PRECEDENCE[body.sig[at].text] == 10 for at in body.top_level_operators(low, index)):
            continue
        left, right = body.source(low, index), body.source(index + 1, high)
        a_low, _ = body.span(low, index)
        _, b_high = body.span(index + 1, high)
        sites.append(Mutation("flip_comparison", a_low, b_high, f"{right} {_FLIP[token.text]} {left}",
                              f"flip `{body.source(low, high)[:60]}`"))
    return sites


def toggle_compound(body: Body) -> list[Mutation]:
    sites = []
    for node in body.walk():
        if node.kind != "simple":
            continue
        start, end = node.start, node.end - 1          # without `;`
        operators = [at for at in range(start, end) if body.sig[at].text in ASSIGNMENTS]
        if len(operators) != 1 or body.has_top(start, end, (",", "?")):
            continue
        at, = operators
        value = body.sig[at].text
        lhs = body.source(start, at)
        if any(t.text in ("++", "--") or (t.kind == "ident" and i + 1 < at and body.sig[i + 1].text == "(")
               for i, t in enumerate(body.sig[start:at], start)):
            continue
        if value in _COMPOUND:
            op = _COMPOUND[value]
            rhs = body.source(at + 1, end)
            if any(BINARY_PRECEDENCE[body.sig[k].text] <= BINARY_PRECEDENCE[op]
                   for k in body.top_level_operators(at + 1, end)):
                rhs = _wrap(rhs)
            sites.append(_edit(body, "toggle_compound", start, end, f"{lhs} = {lhs} {op} {rhs}",
                               f"expand `{body.source(start, end)[:60]}`"))
        elif value == "=":
            width = at - start
            rhs_start = at + 1
            if end - rhs_start <= width + 1:
                continue
            if [t.text for t in body.sig[rhs_start:rhs_start + width]] != [t.text for t in body.sig[start:at]]:
                continue
            op_index = rhs_start + width
            op = body.sig[op_index].text
            if op + "=" not in _COMPOUND or not body.binary_at(op_index):
                continue
            rest_ops = body.top_level_operators(op_index + 1, end)
            if any(BINARY_PRECEDENCE[body.sig[k].text] <= BINARY_PRECEDENCE[op] for k in rest_ops):
                continue
            if any(BINARY_PRECEDENCE[body.sig[k].text] < BINARY_PRECEDENCE[op]
                   for k in body.top_level_operators(rhs_start, op_index)):
                continue
            sites.append(_edit(body, "toggle_compound", start, end,
                               f"{lhs} {op}= {body.source(op_index + 1, end)}",
                               f"contract `{body.source(start, end)[:60]}`"))
    return sites


def _negate(body: Body, start: int, end: int) -> str:
    text = body.source(start, end)
    if body.sig[start].text == "!" and start + 1 < end:
        rest = start + 1
        if body.sig[rest].text == "(" and body.match[rest] == end - 1:
            return body.source(rest + 1, end - 1)
        if end - rest == 1 or (body.sig[rest].kind == "ident" and not body.top_level_operators(rest, end)
                               and not body.has_top(rest, end, ("?",))):
            return body.source(rest, end)
    return "!(" + text + ")"


def swap_if_branches(body: Body) -> list[Mutation]:
    sites = []
    for node in body.walk():
        if node.kind != "if" or node.other is None:
            continue
        cond = _negate(body, *node.cond)
        text = f"if ({cond}) {_braced(body, node.other)} else {_braced(body, node.then)}"
        sites.append(_edit(body, "swap_if_branches", node.start, node.end, text,
                           f"invert `if ({body.source(*node.cond)[:50]})`"))
    return sites


def _contains(body: Body, node: Node, word: str, *, nested_loops: bool) -> bool:
    """`word` inside node, ignoring nested loops/switches when `nested_loops` is False."""
    def visit(item):
        if item.kind in ("simple", "jump", "decl", "label"):
            return any(body.sig[i].text == word for i in range(item.start, item.end))
        if not nested_loops and item.kind in ("for", "while", "do", "switch") and item is not node:
            return False
        children = list(item.children) + [c for c in (item.then, item.other, item.body) if c is not None]
        return any(visit(child) for child in children) or item.kind == "opaque"
    return visit(node)


def _parent_lists(body: Body):
    """Map id(node) -> containing statement list, for nodes directly in a block."""
    parents = {}
    for statements in body.blocks():
        for item in statements:
            parents[id(item)] = statements
    return parents


def loop_forms(body: Body) -> list[Mutation]:
    sites = []
    parents = _parent_lists(body)
    for node in body.walk():
        if node.kind == "for":
            if _contains(body, node.body, "continue", nested_loops=False):
                continue
            init = body.source(*node.init) if node.init[0] < node.init[1] else ""
            cond = body.source(*node.cond) if node.cond[0] < node.cond[1] else "1"
            step = body.source(*node.step) if node.step[0] < node.step[1] else ""
            if not init and step == "":
                text = f"while ({cond}) {body.source(node.body.start, node.body.end)}"
            else:
                if id(node) not in parents:
                    continue
                inner = body.source(node.body.start, node.body.end)
                inner = inner[1:-1] if node.body.kind == "block" else " " + inner
                tail = f" {step};" if step else ""
                loop = f"while ({cond}) {{{inner}{tail} }}"
                text = f"{init}; {loop}" if init else loop
            sites.append(_edit(body, "loop_form", node.start, node.end, text, "for -> while"))
        elif node.kind == "while":
            cond = body.source(*node.cond)
            sites.append(_edit(body, "loop_form", node.start, node.end,
                               f"for (; {cond}; ) {body.source(node.body.start, node.body.end)}",
                               "while -> for"))
    # `init; while (c) { ...; step; }` -> `for (init; c; step) { ... }`
    for statements in body.blocks():
        for before, node in zip(statements, statements[1:]):
            if (before.kind not in ("simple", "decl") or node.kind != "while" or node.body.kind != "block"
                    or not node.body.children or node.body.children[-1].kind != "simple"
                    or _contains(body, node.body, "continue", nested_loops=False)):
                continue
            last = node.body.children[-1]
            condition = {body.sig[i].text for i in range(*node.cond)}
            if not (body.node_effects(before)["writes"] & condition
                    and body.node_effects(last)["writes"] & condition):
                continue        # only a loop counter's own init and step join the header
            init = body.source(before.start, before.end - 1)
            step = body.source(last.start, last.end - 1)
            rest = node.body.children[:-1]
            inner = (body.text[body.span(node.body.start, node.body.end)[0] + 1:body.span(last.start, last.end)[0]]
                     if rest else " ")
            text = f"for ({init}; {body.source(*node.cond)}; {step}) {{{inner.rstrip()} }}"
            low, _ = body.span(before.start, before.end)
            _, high = body.span(node.start, node.end)
            sites.append(Mutation("loop_form", low, high, text, "while -> for with init/step"))
    return sites


_INT_CAST = re.compile(r"^\(\s*int\s*\)$")


def int_casts(body: Body) -> list[Mutation]:
    sites, protected = [], _protected(body)
    for index, token in enumerate(body.sig):
        if index in protected:
            continue
        # remove `(int)` before an operand
        if (token.text == "(" and index + 2 < len(body.sig) and body.sig[index + 1].text == "int"
                and body.sig[index + 2].text == ")" and index + 3 < len(body.sig)
                and _is_operand_start(body.sig[index + 3])):
            sites.append(_edit(body, "int_cast", index, index + 3, "", "remove (int)"))
            continue
        # remove `static_cast<int>(X)` -> `(X)`
        if (token.text == "static_cast" and index + 4 < len(body.sig)
                and [t.text for t in body.sig[index + 1:index + 4]] == ["<", "int", ">"]
                and body.sig[index + 4].text == "("):
            sites.append(_edit(body, "int_cast", index, index + 4, "", "remove static_cast<int>"))
            continue
        # add `(int)` around a binary operand that is a primary
        if token.text in BINARY_PRECEDENCE and body.binary_at(index):
            right = index + 1
            stop = body.operand_right(index, 15)
            cast_group = (body.sig[right].text == "(" and
                          _looks_like_cast(body.sig[right + 1:body.match[right]]))
            if (stop > right and stop - right <= 12 and not cast_group
                    and body.sig[right].kind not in ("string", "number")
                    and body.sig[right].text not in ("-", "!", "~")):
                low, high = body.span(right, stop)
                sites.append(Mutation("int_cast", low, high, "(int)" + body.text[low:high],
                                      f"cast `{body.text[low:high][:40]}` to int"))
            left = body.operand_left(index, 15)
            if (left < index and index - left <= 12 and body.sig[left].kind not in ("string", "number")
                    and not (left > 0 and body.sig[left - 1].text == ")")):
                low, high = body.span(left, index)
                sites.append(Mutation("int_cast", low, high, "(int)" + body.text[low:high],
                                      f"cast `{body.text[low:high][:40]}` to int"))
    return sites


def _assignment(body: Body, node: Node):
    """(lhs text, rhs start, rhs end) for `lhs = rhs;` or None."""
    if node.kind != "simple":
        return None
    start, end = node.start, node.end - 1
    operators = [at for at in range(start, end) if body.sig[at].text == "="]
    if len(operators) != 1 or any(body.sig[at].text in ASSIGNMENTS - {"="} for at in range(start, end)):
        return None
    at, = operators
    if at == start or any(t.kind == "ident" and body.sig[i + 1].text == "("
                          for i, t in enumerate(body.sig[start:at], start)):
        return None
    return body.source(start, at), at + 1, end


def _single(body: Body, node: Node | None) -> Node | None:
    if node is None:
        return None
    if node.kind == "block":
        return node.children[0] if len(node.children) == 1 else None
    return node


def ternary_forms(body: Body) -> list[Mutation]:
    sites = []
    for node in body.walk():
        assign = _assignment(body, node)
        target = None
        if assign is not None:
            lhs, low, high = assign
            target = f"{lhs} = "
        elif node.kind == "jump" and body.sig[node.start].text == "return" and node.end - node.start > 2:
            low, high, target = node.start + 1, node.end - 1, "return "
        if target is not None:
            questions = [at for at in range(low, high) if body.sig[at].text == "?"]
            if (len(questions) == 1 and body.has_top(low, high, ("?",))
                    and not body.has_top(low, high, (",",))):
                q, = questions
                colons = [at for at in range(q + 1, high) if body.sig[at].text == ":"]
                if len(colons) == 1:
                    colon, = colons
                    cond = body.source(low, q)
                    if cond.startswith("(") and body.match.get(low) == q - 1:
                        cond = body.source(low + 1, q - 1)
                    text = (f"if ({cond}) {{ {target}{body.source(q + 1, colon)}; }} "
                            f"else {{ {target}{body.source(colon + 1, high)}; }}")
                    sites.append(_edit(body, "ternary", node.start, node.end, text, "?: -> if/else"))
        if node.kind == "if" and node.other is not None:
            then, other = _single(body, node.then), _single(body, node.other)
            if then is None or other is None:
                continue
            a, b = _assignment(body, then), _assignment(body, other)
            if a is not None and b is not None and a[0].strip() == b[0].strip():
                left = body.source(a[1], a[2])
                right = body.source(b[1], b[2])
                text = f"{a[0].rstrip()} = {_wrap(body.source(*node.cond))} ? {left} : {right};"
            elif (then.kind == other.kind == "jump" and body.sig[then.start].text == "return" == body.sig[other.start].text
                  and then.end - then.start > 2 and other.end - other.start > 2):
                text = (f"return {_wrap(body.source(*node.cond))} ? {body.source(then.start + 1, then.end - 1)}"
                        f" : {body.source(other.start + 1, other.end - 1)};")
            else:
                continue
            sites.append(_edit(body, "ternary", node.start, node.end, text, "if/else -> ?:"))
    return sites


def shift_multiply(body: Body) -> list[Mutation]:
    sites, protected = [], _protected(body)
    for index, token in enumerate(body.sig):
        if index in protected or token.text not in ("*", "<<") or not body.binary_at(index):
            continue
        precedence = BINARY_PRECEDENCE[token.text]
        low, high = body.operand_left(index, precedence), body.operand_right(index, precedence)
        if low >= index or high != index + 2 or body.sig[index + 1].kind != "number":
            continue
        try:
            value = int(body.sig[index + 1].text.rstrip("uUlL"), 0)
        except ValueError:
            continue
        left = body.source(low, index)
        if body.top_level_operators(low, index):
            left = _wrap(left)
        if token.text == "*":
            if value < 2 or value & (value - 1):
                continue
            text = f"({left} << {value.bit_length() - 1})"
        else:
            if not 0 < value < 31:
                continue
            text = f"({left} * {1 << value})"
        sites.append(_edit(body, "shift_multiply", low, high, text, f"rewrite `{body.source(low, high)[:50]}`"))
    return sites


def temporaries(body: Body) -> list[Mutation]:
    """Hoist a call/subexpression into `int __pmN`, or inline a single-use local."""
    sites = []
    parents = _parent_lists(body)
    used = {t.text for t in body.sig if t.kind == "ident"}
    fresh = next(f"tmp_p{n}" for n in range(1000) if f"tmp_p{n}" not in used)
    for node in body.walk():
        if id(node) not in parents or node.kind not in ("simple", "jump", "if"):
            continue
        low, high = (node.cond if node.kind == "if" else (node.start, node.end - 1))
        if node.kind == "jump" and body.sig[node.start].text != "return":
            continue
        if node.kind == "jump":
            low += 1
        if any(body.sig[at].text in ("&&", "||", "?") for at in range(low, high)):
            continue
        if body.has_top(low, high, (",",)):
            continue
        candidates = set()
        for index in range(low, high):
            token = body.sig[index]
            if token.text in BINARY_PRECEDENCE and body.binary_at(index):
                precedence = BINARY_PRECEDENCE[token.text]
                candidates.add((body.operand_left(index, precedence, low), index))
                candidates.add((index + 1, body.operand_right(index, precedence, high)))
            elif (token.kind == "ident" and token.text not in KEYWORDS and index + 1 < high
                  and body.sig[index + 1].text == "(" and body.match[index + 1] < high
                  and (index == 0 or body.sig[index - 1].text not in (".", "->", "::"))
                  and not (index == low and body.match[index + 1] + 1 == high and node.kind == "simple")):
                candidates.add((index, body.match[index + 1] + 1))     # a free call
        for a, b in sorted(candidates):
            if b - a < 2:
                continue
            expression = body.source(a, b)
            start, _ = body.span(node.start, node.end)
            e_low, e_high = body.span(a, b)
            statement = body.text[start:body.span(node.start, node.end)[1]]
            offset = e_low - start
            replaced = statement[:offset] + fresh + statement[e_high - start:]
            sites.append(Mutation("temporary", start, start + len(statement),
                                  "int " + fresh + " = " + expression + "; " + replaced,
                                  f"hoist `{expression[:40]}`"))
    # inline: `T name = expr;` used exactly once in the immediately following statement
    for statements in body.blocks():
        for decl, after in zip(statements, statements[1:]):
            if decl.kind != "decl":
                continue
            names = body.declared(decl)
            heads = body._declarator_heads(decl.start, decl.end - 1)
            if len(names) != 1 or len(heads) != 1:
                continue
            name = names[0]
            equals = heads[0][1] + 1
            if equals >= decl.end - 1 or body.sig[equals].text != "=":
                continue
            uses = [i for i, t in enumerate(body.sig) if t.text == name and t.kind == "ident"
                    and not decl.start <= i < decl.end]
            if len(uses) != 1 or not after.start <= uses[0] < after.end:
                continue
            if after.kind not in ("simple", "jump", "if", "decl"):
                continue
            use = uses[0]
            neighbours = (body.sig[use - 1].text if use > 0 else "",
                          body.sig[use + 1].text if use + 1 < len(body.sig) else "")
            if (neighbours[1] in ASSIGNMENTS or "++" in neighbours or "--" in neighbours
                    or neighbours[0] in ("&", ".", "->", "::")):
                continue        # written, address-taken or a member name: not a value use
            if any(body.sig[at].text in ("&&", "||", "?") for at in range(after.start, uses[0])):
                continue
            expression = body.source(equals + 1, decl.end - 1)
            d_low, _ = body.span(decl.start, decl.end)
            a_low, a_high = body.span(after.start, after.end)
            u_low, u_high = body.span(uses[0], uses[0] + 1)
            statement = body.text[a_low:u_low] + _wrap(expression) + body.text[u_high:a_high]
            sites.append(Mutation("temporary", d_low, a_high, statement, f"inline `{name}`"))
    return sites


def split_declarations(body: Body) -> list[Mutation]:
    """`T x = e;` <-> `T x; x = e;` (the assignment directly follows the declaration)."""
    sites = []
    for statements in body.blocks():
        for index, node in enumerate(statements):
            if node.kind != "decl":
                continue
            heads = body._declarator_heads(node.start, node.end - 1)
            if len(heads) != 1 or body.sig[node.start].text in ("const", "static"):
                continue
            head, name = heads[0]
            equals = name + 1
            if equals < node.end - 1 and body.sig[equals].text == "=":
                if any(body.sig[i].text in ("&", "[") for i in range(head, name)):
                    continue        # references and arrays need their initializer
                low, high = body.span(node.start, node.end)
                text = (body.source(node.start, equals) + "; " + body.sig[name].text + " = "
                        + body.source(equals + 1, node.end - 1) + ";")
                sites.append(Mutation("split_declaration", low, high, text,
                                      f"split declaration of {body.sig[name].text}"))
            elif equals == node.end - 1 and index + 1 < len(statements):
                after = statements[index + 1]
                if (after.kind == "simple" and after.end - after.start > 3
                        and body.sig[after.start].text == body.sig[name].text
                        and body.sig[after.start + 1].text == "="):
                    rest = body.source(after.start + 2, after.end - 1)
                    if body.sig[name].text in {t.text for t in body.sig[after.start + 2:after.end]}:
                        continue
                    low, _ = body.span(node.start, node.end)
                    _, high = body.span(after.start, after.end)
                    text = body.source(node.start, node.end - 1) + " = " + rest + ";"
                    sites.append(Mutation("split_declaration", low, high, text,
                                          f"join declaration of {body.sig[name].text}"))
    return sites


# ------------------------------------------------------ dead budget probes
#
# VC6 /Ob2 gives each caller an inline budget spent on call sites in source
# order; a fully inlined static helper of chained ``if (x == k) x = c;`` called
# with a constant folds to no code but still spends it. Placing one in a
# caller changes which later inline candidates (destructor teardowns, small
# helpers) fit, and so the EH unwind topology. The probe is a marked stand-in:
# it is never recovered code, only evidence about where the original spent
# budget. ``probe_helpers`` derives the file-scope helper definitions that the
# probe calls in a body need; the source driver splices them before the target.

PROBE_PREFIX = "permute_budget_probe_"
PROBE_FORMS = ("flat", "nested")
PROBE_SIZES = (1, 2, 4, 6, 8, 12, 16, 20, 24, 32)
PROBE_ARGUMENT = 2
PROBE_LIMIT = 2              # probes the random search may hold at once
_PROBE_NOTE = "/* stand-in: dead budget probe */"
_PROBE_CALL = re.compile(r"\b" + PROBE_PREFIX + r"(nested_)?([1-9][0-9]*)\(" + str(PROBE_ARGUMENT)
                         + r"\); " + re.escape(_PROBE_NOTE) + r"(?:[ \t]*\n[ \t]*)?")


def probe_name(size: int, form: str = "flat") -> str:
    if type(size) is not int or size < 1 or form not in PROBE_FORMS:
        raise ValueError("budget probe needs a positive size and a known form")
    return PROBE_PREFIX + ("nested_" if form == "nested" else "") + str(size)


def probe_call(size: int, form: str = "flat") -> str:
    return f"{probe_name(size, form)}({PROBE_ARGUMENT}); {_PROBE_NOTE}"


def probe_calls(text: str) -> list[tuple[int, int, int, str]]:
    """(start, end, size, form) of every probe call statement in a body."""
    return [(match.start(), match.end(), int(match[2]), "nested" if match[1] else "flat")
            for match in _PROBE_CALL.finditer(text)]


def probe_helpers(text: str) -> str:
    """File-scope stand-in definitions for the probes a body calls; empty when none."""
    flat, nested = set(), set()
    for _start, _end, size, form in probe_calls(text):
        (nested if form == "nested" else flat).add(size)
    flat |= nested
    out = []
    for size in sorted(flat):
        lines = "".join(f"    if (x == {k}) x = {3 * k + 1};\n" for k in range(size))
        out.append(f"// stand-in: dead /Ob2 budget probe from recover permute; not recovered code.\n"
                   f"static int {probe_name(size)}(int x)\n{{\n{lines}    return x;\n}}\n")
    for size in sorted(nested):
        inner = probe_name(size)
        out.append(f"// stand-in: nested dead /Ob2 budget probe from recover permute; not recovered code.\n"
                   f"static int {probe_name(size, 'nested')}(int x)\n{{\n"
                   f"    if ({inner}(x) == 0) return 1;\n    return {inner}(x + 1);\n}}\n")
    return "".join(out)


def probe_positions(body: Body) -> list[tuple[int, str]]:
    """(character offset, label) of every statement boundary inside a block.

    Before each statement that is not a label, and before each block's
    closing brace. Labels are skipped: the statement after one is included.
    """
    seen, positions = set(), []

    def add(offset, label):
        if offset not in seen:
            seen.add(offset)
            positions.append((offset, label))

    def line(offset):
        return body.text.count("\n", 0, offset) + 1

    lists = [(body.statements, len(body.text))]
    for node in body.walk():
        if node.kind == "block":
            lists.append((node.children, body.sig[node.end - 1].start))
    for statements, closing in lists:
        for node in statements:
            if node.kind in ("label", "pp"):
                continue
            offset = body.sig[node.start].start
            add(offset, f"line {line(offset)} before `{' '.join(body.source(node.start, node.end).split())[:40]}`")
        add(closing, f"line {line(closing)} at block end")
    return sorted(positions)


def _probe_insertion(text: str, offset: int, call: str) -> str:
    line_start = text.rfind("\n", 0, offset) + 1
    indent = text[line_start:offset]
    if indent.strip():
        return call + " "
    return call + "\n" + indent


def insert_probe(text: str, offset: int, size: int, form: str = "flat") -> str:
    return text[:offset] + _probe_insertion(text, offset, probe_call(size, form)) + text[offset:]


def budget_probes(body: Body, *, sizes=PROBE_SIZES, forms=PROBE_FORMS,
                  limit: int = PROBE_LIMIT) -> list[Mutation]:
    """Insert, resize or remove a marked dead budget probe (opt-in stand-in operator)."""
    sites = []
    present = probe_calls(body.text)
    for start, end, size, form in present:
        sites.append(Mutation("budget_probe", start, end, "", f"remove {form} probe {size}"))
        for other in sizes:
            if other != size:
                call = probe_call(other, form)
                sites.append(Mutation("budget_probe", start, start + len(probe_call(size, form)), call,
                                      f"resize {form} probe {size} -> {other}"))
    if len(present) >= limit:
        return sites
    occupied = {start for start, *_ in present}
    for offset, label in probe_positions(body):
        if offset in occupied:
            continue
        for form in forms:
            for size in sizes:
                call = probe_call(size, form)
                sites.append(Mutation("budget_probe", offset, offset,
                                      _probe_insertion(body.text, offset, call),
                                      f"{form} probe {size} {label}"))
    return sites


def probe_variants(text: str, parameters: str = "", *, sizes=PROBE_SIZES,
                   forms=PROBE_FORMS) -> list[tuple[str, str]]:
    """Every single-probe variant of a body, as (description, text), in sweep order."""
    body = Body(text, parameters)
    variants = []
    for offset, label in probe_positions(body):
        for form in forms:
            for size in sizes:
                variants.append((f"{form} probe {size} {label}", insert_probe(text, offset, size, form)))
    return variants


OPERATORS = {
    "split_declaration": split_declarations,
    "swap_statements": swap_adjacent,
    "reorder_declarations": reorder_declarations,
    "commute": commute,
    "flip_comparison": flip_comparison,
    "toggle_compound": toggle_compound,
    "swap_if_branches": swap_if_branches,
    "loop_form": loop_forms,
    "int_cast": int_casts,
    "ternary": ternary_forms,
    "shift_multiply": shift_multiply,
    "temporary": temporaries,
}
# Stand-in operators insert marked non-recovered code; they run only when named.
OPT_IN_OPERATORS = {
    "budget_probe": budget_probes,
}
DEFAULT_OPERATORS = tuple(OPERATORS)
OPERATORS.update(OPT_IN_OPERATORS)


def mutation_sites(text: str, parameters: str = "", operators=None) -> list[Mutation]:
    body = Body(text, parameters)
    sites = []
    for name in (operators or DEFAULT_OPERATORS):
        sites.extend(OPERATORS[name](body))
    return sites


# --------------------------------------------------------------- search

@dataclass(frozen=True)
class Score:
    """A caller's distance to the target; ``exact`` is the caller's own verdict."""
    distance: int
    exact: bool
    detail: dict = field(default_factory=dict, compare=False)


@dataclass
class SearchResult:
    start: Score
    best: Score
    best_text: str
    iterations: int = 0
    compiles: int = 0
    compile_failures: int = 0
    cache_hits: int = 0
    accepted: int = 0
    no_sites: int = 0
    interrupted: bool = False
    stop_reason: str = ""
    elapsed_seconds: float = 0.0
    improvements: list = field(default_factory=list)
    operator_stats: dict = field(default_factory=dict)
    minimize_compiles: int = 0
    reverted_hunks: int = 0


def _normalized(text: str) -> str:
    return " ".join(t.text for t in tokenize(text) if t.kind not in ("ws", "comment"))


def _score_many(texts, scorer, batch_scorer):
    """Scores in input order; a batch scorer may evaluate them concurrently."""
    if not texts:
        return []
    if batch_scorer is None:
        return [scorer(item) for item in texts]
    scores = list(batch_scorer(list(texts)))
    if len(scores) != len(texts):
        raise ValueError("batch scorer must return one score per variant")
    return scores


def search(text: str, scorer, *, iterations: int = 200, seed: int = 0, time_limit: float | None = None,
           parameters: str = "", operators=None, equal_accept: float = 0.4, temperature: float = 0.3,
           restart_after: int = 40, max_chain: int = 3, scale: int = 1,
           on_improve=None, on_trial=None, clock=time.monotonic, minimize: bool = True,
           batch_scorer=None, batch: int = 1) -> SearchResult:
    """Seeded hill climbing with annealing over body spellings.

    ``scorer(body_text)`` returns a ``Score`` or None when the variant does
    not compile. Lower distances are better; ``Score.exact`` ends the search.
    Equal-score moves are accepted with ``equal_accept`` probability and worse
    ones with ``exp(-delta / (scale * T))``, ``T`` falling linearly to zero.
    KeyboardInterrupt stops the loop and returns the best result so far.
    With ``minimize`` the best variant's unneeded line hunks are then reverted.

    With ``batch`` > 1 each round derives candidates from the round's current
    text until it holds ``batch`` uncached ones (at most ``4 * batch``
    derivations; exactly one when ``batch`` is 1), scores those together through
    ``batch_scorer(texts)`` (which may compile them concurrently), then applies
    acceptance to them in generation order. The trajectory depends only on the
    seed and ``batch``, never on completion order; ``batch`` 1 is the serial
    search. Each candidate counts as one iteration.
    """
    if type(iterations) is not int or iterations < 0:
        raise ValueError("iterations must be a nonnegative integer")
    if type(batch) is not int or batch < 1:
        raise ValueError("batch must be a positive integer")
    rng = random.Random(seed)
    started = clock()
    start = scorer(text) if batch_scorer is None else _score_many([text], scorer, batch_scorer)[0]
    if start is None:
        raise ValueError("the unchanged body does not compile through the scorer")
    result = SearchResult(start=start, best=start, best_text=text, compiles=1)
    seen = {_normalized(text): start}
    current, current_score = text, start
    since_improvement = 0
    if start.exact:
        result.stop_reason = "already_exact"
        result.elapsed_seconds = clock() - started
        return result

    def derive(base):
        candidate, applied = base, []
        chain = 1 + (rng.random() < 0.25) + (rng.random() < 0.1)
        for _ in range(min(chain, max_chain)):
            try:
                sites = mutation_sites(candidate, parameters, operators)
            except Unsupported:
                sites = []
            if not sites:
                break
            by_operator = {}
            for site in sites:
                by_operator.setdefault(site.operator, []).append(site)
            # Choose an operator first so one prolific operator cannot starve the rest.
            site = rng.choice(by_operator[rng.choice(sorted(by_operator))])
            try:
                following = site.apply(candidate)
                _normalized(following)
            except Unsupported:
                continue
            candidate = following
            applied.append(site)
        return candidate, applied

    try:
        iteration = 0
        stopped = False
        while iteration < iterations and not stopped:
            if time_limit is not None and clock() - started >= time_limit:
                result.stop_reason = "time_limit"
                break
            # Generate the round from the round's current text.
            # Cached spellings cost nothing, so a round keeps deriving until it holds
            # ``batch`` uncached candidates (bounded, so a saturated space still ends rounds).
            round_, pending = [], {}
            for _ in range(min(1 if batch == 1 else 4 * batch, iterations - iteration)):
                if len(pending) >= batch:
                    break
                iteration += 1
                candidate, applied = derive(current)
                if not applied:
                    round_.append((iteration, None, None, applied))
                    continue
                key = _normalized(candidate)
                if key not in seen and key not in pending:
                    pending[key] = candidate
                round_.append((iteration, candidate, key, applied))
            keys = list(pending)
            for key, score in zip(keys, _score_many([pending[key] for key in keys], scorer, batch_scorer)):
                seen[key] = score
            fresh = set(keys)
            # Apply acceptance in generation order.
            for number, candidate, key, applied in round_:
                result.iterations = number
                if candidate is None:
                    result.no_sites += 1
                    continue
                stats = result.operator_stats.setdefault(applied[-1].operator,
                                                         {"tried": 0, "failed": 0, "improved": 0})
                stats["tried"] += 1
                score = seen[key]
                if key in fresh:
                    fresh.discard(key)
                    result.compiles += 1
                    if on_trial is not None:
                        on_trial(number, score, applied)
                else:
                    result.cache_hits += 1
                if score is None:
                    result.compile_failures += 1
                    stats["failed"] += 1
                    continue
                delta = score.distance - current_score.distance
                progress = number / max(1, iterations)
                heat = temperature * max(0.0, 1.0 - progress)
                if (delta < 0 or (delta == 0 and rng.random() < equal_accept)
                        or (delta > 0 and heat > 0 and rng.random() < math.exp(-delta / (scale * heat)))):
                    current, current_score = candidate, score
                    result.accepted += 1
                if score.distance < result.best.distance or (score.exact and not result.best.exact):
                    result.best, result.best_text = score, candidate
                    stats["improved"] += 1
                    since_improvement = 0
                    result.improvements.append({"iteration": number, "distance": score.distance,
                                                "exact": score.exact,
                                                "mutations": [m.description for m in applied]})
                    if on_improve is not None:
                        on_improve(candidate, score)
                    if score.exact:
                        result.stop_reason = "exact"
                        stopped = True
                        break
                else:
                    since_improvement += 1
                    if since_improvement >= restart_after:
                        current, current_score, since_improvement = result.best_text, result.best, 0
        else:
            if not stopped and not result.stop_reason:
                result.stop_reason = "iterations"
    except KeyboardInterrupt:
        result.interrupted = True
        result.stop_reason = "interrupted"
    if minimize and not result.interrupted and result.best_text != text:
        try:
            _minimize(text, result, scorer, seen, batch_scorer)
        except KeyboardInterrupt:
            result.interrupted = True
    result.elapsed_seconds = clock() - started
    return result


def _minimize(original: str, result: SearchResult, scorer, seen: dict, batch_scorer=None) -> None:
    """Revert line hunks of the best variant that its score does not need.

    Annealing accepts neutral moves, so the best text drifts. Each hunk is
    reverted alone; the revert is kept when the score is no worse (and still
    exact when it was exact). The best score never gets worse. A batch scorer
    evaluates every single-hunk revert of a pass together; the kept revert is
    the same one the serial pass keeps (the last qualifying hunk).
    """
    import difflib
    while True:
        before = original.splitlines(keepends=True)
        after = result.best_text.splitlines(keepends=True)
        hunks = []
        for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(None, before, after, autojunk=False).get_opcodes():
            if tag == "replace" and i2 - i1 == j2 - j1:
                hunks.extend((tag, i1 + k, i1 + k + 1, j1 + k, j1 + k + 1) for k in range(i2 - i1))
            elif tag != "equal":
                hunks.append((tag, i1, i2, j1, j2))
        candidates = []
        for _tag, i1, i2, j1, j2 in reversed(hunks):
            candidate = "".join(after[:j1] + before[i1:i2] + after[j2:])
            try:
                key = _normalized(candidate)
                Body(candidate)
            except Unsupported:
                continue
            candidates.append((candidate, key))
        fresh = set()
        if batch_scorer is not None:
            texts = {key: candidate for candidate, key in candidates if key not in seen}
            for key, score in zip(texts, _score_many(list(texts.values()), scorer, batch_scorer)):
                seen[key] = score
                result.compiles += 1
                result.minimize_compiles += 1
            fresh = set(texts)
        for candidate, key in candidates:
            if key in fresh:
                score = seen[key]
                fresh.discard(key)
            elif key in seen:
                score = seen[key]
                result.cache_hits += 1
            else:
                score = scorer(candidate)
                seen[key] = score
                result.compiles += 1
                result.minimize_compiles += 1
            if (score is not None and score.distance <= result.best.distance
                    and (score.exact or not result.best.exact)):
                result.best, result.best_text = score, candidate
                result.reverted_hunks += 1
                break
        else:
            return
