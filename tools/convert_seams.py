#!/usr/bin/env -S uv run python
"""Turn a file's `original_method<T>(0x...)` pointers into real methods.

WHY THIS EXISTS. A seam spelled

    typedef void (OriginalObject::*func_x)(int a);
    func_x SomeX = original_method<func_x>(0x00401000);
    ...
    (ORIGINAL(this)->*SomeX)(a);

compiles `call dword ptr [SomeX]` where the image has `call rel32`, so every
body that reaches a sibling this way is capped at MNEMONIC_ONLY. The fix is
always the same three edits - declare the method on its class, forward it from
pending_bodies.cpp, call it directly - and doing them by hand is where the
mistakes live: basewin.cpp had two seams whose wrapper bodies already carried
the method's name, and rewriting the pointer turned them into infinite
recursion that the compiler accepted without a word.

WHAT IT WILL NOT DO. It converts a seam only when every fact it needs is
stated, and reports the rest rather than guessing:

  * the typedef must be `RET (OriginalObject::*NAME)(PARAMS)` - the parameter
    spelling comes from there, never from the mangled name, because the
    catalogue's return types are known to be wrong (see decomp/mangled.py).
  * the address must resolve to a `Class::method` the file's class owns.
  * every use of the pointer must be a `(ORIGINAL(expr)->*NAME)(...)` call.
    A bare mention - passing the pointer, comparing it - is left alone.
  * a same-named body already in the file is REPORTED, not rewritten: it is
    either the wrapper this replaces (delete it) or a real recovery (keep the
    pointer). Only a human can tell those apart.

--apply writes; without it, this prints what it would do.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from decomp import read
from decomp.mangled import qualified_name

REPO_ROOT = Path(__file__).resolve().parent.parent
PENDING = (REPO_ROOT / "src" / "pending_bodies.cpp").resolve()
PENDING_ANCHOR = "// ?write_raw_l@Buffer@@QAEHPADHHH@Z at 0x005DBD00"

TYPEDEF = re.compile(
    r"typedef\s+(?P<ret>[\w:*&\s]+?)\s*\(\s*OriginalObject::\*\s*"
    r"(?P<alias>\w+)\s*\)\s*\((?P<params>[^)]*)\)\s*;")
BINDING = re.compile(
    # `static` and `const` are part of the binding: leaving either behind
    # turns the next definition in the file into a file-scope static member.
    r"(?P<indent>[ \t]*)(?:static\s+|const\s+)*(?P<alias>\w+)\s+"
    r"(?P<name>\w+)\s*=\s*\n?\s*"
    r"original_method<(?P=alias)>\(\s*(?P<address>0x[0-9A-Fa-f]+)\s*\)\s*;\n")
EXTERN = re.compile(r"^extern\s+[\w*&\s]+?\b(?P<name>\w+)\s*;[^\n]*\n", re.M)


_TREE: dict[Path, str] = {}


def body(path: Path) -> str:
    return tree()[path]


def rewrite(path: Path, text: str) -> None:
    """Every edit goes back into the one store the next question reads.

    A per-file `touched` dict loses edits the moment two sources share a
    header: netdaemon.cpp and net.cpp both declare methods on `class Net`, and
    the second run would read net_class.h from disk and write back a copy
    without the first's declaration. Nothing complains - the calls compile
    until the linker, or in that case until CL says `synch` is not a member.
    """
    tree()[path] = text


def tree() -> dict[Path, str]:
    """Every build input's text, read once.

    Both of this tool's refusals are tree-wide questions and neither is
    answerable from one file: `func_dialog_close` is declared in dialog.h and
    used from listbox.h and checkbox.h, and a seam pointer bound in popup.cpp
    is called from guarded_teardowns.cpp. Asking either locally deletes a
    typedef three other files need, or converts a pointer a fourth still reads.
    """
    if not _TREE:
        for path in sorted((REPO_ROOT / "src").glob("*.[ch]*")):
            if path.suffix in (".c", ".h", ".cpp", ".hpp"):
                _TREE[path.resolve()] = path.read_text()
    return _TREE


SCOPE: set[Path] = set()

# Which class each `void *` receiver has already been given. A global can only
# have one type, and netdaemon.cpp binds 0x0093CD90 to both `Net::get` and
# `NetDaemon::synch` - so one of the two catalogued names is wrong about its
# receiver, which is a question for a person and not for a rewrite.
RETYPED: dict[str, str] = {}


def elsewhere(source: Path) -> str:
    """The tree minus every file this run may rewrite.

    A seam pointer is routinely bound in one file and called from several -
    `WinOriginalClose` is named in five. Those call sites are converted too,
    so what has to be EMPTY is not "uses outside this .cpp" but "uses this run
    will not touch". Anything left there is a use the conversion would break.
    """
    return "\n".join(text for path, text in tree().items()
                      if path not in SCOPE)


def _names_by_address() -> dict[int, str]:
    out = {}
    for record in read(REPO_ROOT / "src"):
        out.setdefault(record.address, record.name or record.symbol)
    return out


def _split_params(params: str) -> list[str]:
    """Parameter declarations, one per element, commas at depth 0 only."""
    out, depth, current = [], 0, ""
    for char in params:
        if char in "(<[":
            depth += 1
        elif char in ")>]":
            depth -= 1
        if char == "," and depth == 0:
            out.append(current.strip())
            current = ""
        else:
            current += char
    if current.strip():
        out.append(current.strip())
    return [p for p in out if p not in ("void", "")]


# Words that end a TYPE and can never be the parameter's name. `unsigned long`
# is two of them, and taking the last word as a name left the forwarder
# passing a literal `long` as an argument.
TYPE_WORDS = frozenset("""void bool char short int long float double signed
unsigned const volatile struct class union enum""".split())


def _argument_names(params: list[str]) -> list[str]:
    """The trailing identifier of each declaration, or a positional name."""
    out = []
    for index, param in enumerate(params):
        match = re.search(r"(\w+)\s*(\[\s*\])?$", param)
        name = match.group(1) if match else ""
        # A trailing identifier is only a NAME if something is left when it
        # goes. `LPSTR` and `int` are whole declarations; `LPSTR text` is not.
        if name in TYPE_WORDS or (name and
                                  not param[:match.start(1)].strip()):
            name = ""
        if not name:
            name = f"a{index + 1}"
            param = f"{param} {name}"
            params[index] = param
        out.append(name)
    return out


BASES = re.compile(r"\bclass\s+(?P<name>\w+)\s*:\s*(?P<bases>[^{]+)\{")


def _reaches_publicly(derived: str, base: str, depth: int = 0) -> bool | None:
    """Whether `derived` can name `base`'s members, or None if unrelated.

    This tree spells its hierarchies `class Popup : BasePop` - PRIVATE, since
    that is what `class` means - so a call the seam made through a
    pointer-to-member does not compile once it becomes `BasePop::init(...)`.
    Widening the inheritance is a real modelling decision (it changes what the
    rest of the tree may call), so the conversion stops instead.

    A path that exists but is private answers False, and only NO path answers
    None: PopMenu reaches BasePop through Popup, and reporting "unrelated"
    there would let the conversion through on the strength of not looking.
    """
    if derived == base:
        return True
    if depth > 8:
        return None
    related = False
    for text in tree().values():
        for match in BASES.finditer(text):
            if match.group("name") != derived:
                continue
            for spec in match.group("bases").split(","):
                spec = spec.strip()
                if not spec:
                    continue
                deeper = _reaches_publicly(spec.split()[-1], base, depth + 1)
                if deeper is None:
                    continue
                related = True
                if deeper and re.match(r"(virtual\s+)?public\b", spec):
                    return True
    return False if related else None


DEFINITION = re.compile(r"^[\w:<>,*&\s]*?\b(\w+)::~?\w+\s*\(", re.M)


def _enclosing_class(text: str, index: int) -> str:
    """The class whose member body contains `index`, if any.

    Needed because an unqualified call is subject to name HIDING:
    `PopMenu::init()` forwards to `BasePop::init(int, long)`, and rewriting
    the seam to a bare `init(0, 0)` finds PopMenu's own nullary `init` and
    fails on the argument count rather than reaching the base.
    """
    found = ""
    for match in DEFINITION.finditer(text, 0, index):
        found = match.group(1)
    return found


def _receiver(expression: str) -> str:
    """How to reach the method from what `ORIGINAL(...)` was handed.

    `this` needs no receiver at all, and `&x` is an object the seam took the
    address of only because a pointer-to-member needs one - so it goes back to
    being a dot. Anything else keeps the arrow it already had.
    """
    if expression == "this":
        return ""
    if expression.startswith("&"):
        return expression[1:].strip() + "."
    return expression + "->"


# Anchored at line start and required to END the declaration, so a `void *`
# PARAMETER never matches: sound.h spells `init_sound(MapWin *window, ...)` and
# a seam whose receiver is a local called `window` must not rewrite it.
VOID_GLOBAL = re.compile(
    r"^(?P<extern>extern\s+)?void\s*\*\s*(?P<name>{})\s*(?=[;=])", re.M)


def _retype_receiver(expression: str, klass: str) -> None:
    """Give a `void *` global the type the seam proved it has.

    `(ORIGINAL(DatalinkMain)->*DatalinkExec)(...)` type-checks whatever
    DatalinkMain is, so the tree can leave it `void *`; `DatalinkMain->exec()`
    cannot. The seam is the evidence for the type - it was bound to a method of
    exactly one class - so the declaration stops lying rather than the call
    site acquiring a cast.
    """
    name = re.fullmatch(r"[A-Za-z_]\w*", expression.lstrip("&"))
    if name is None:
        return
    pattern = re.compile(VOID_GLOBAL.pattern.format(re.escape(name.group(0))),
                         re.M)
    for path in sorted(tree()):
        if not pattern.search(body(path)):
            continue
        fixed = pattern.sub(lambda m: f"{m.group('extern') or ''}{klass} "
                                      f"*{m.group('name')}", body(path))
        fixed = re.sub(
            rf"({re.escape(name.group(0))}\s*=\s*)(?:reinterpret_cast<void\s*\*>|\(void\s*\*\))",
            lambda m: m.group(1) + f"({klass} *)", fixed)
        # `extern K *X;` needs K to be a type here even though it needs no
        # definition, and the header that declared it `void *` deliberately
        # did not include K's.
        if path.suffix == ".h" and not re.search(
                rf"\b(class|struct)\s+{re.escape(klass)}\b", fixed):
            last = fixed.rfind('#include')
            end = fixed.index("\n", last) + 1 if last >= 0 else 0
            fixed = fixed[:end] + f"\nclass {klass};\n" + fixed[end:]
        rewrite(path, fixed)
        RETYPED[name.group(0)] = klass


def _signature(params: str | list[str]) -> tuple[str, ...]:
    """A parameter list reduced to comparable types.

    ARITY is not enough to tell overloads apart: Dialog has an `init(int)` and
    an `init(Heap *)`, and matching by count handed the int seam the Heap
    body's definition - so no forwarder was emitted and the link failed on a
    symbol the tree had just started calling.
    """
    if isinstance(params, str):
        params = _split_params(params)
    out = []
    for param in params:
        text = re.sub(r"\b(const|volatile|struct|class)\b", " ",
                      _type_of(param))
        out.append(re.sub(r"\s+", "", text))
    return tuple(out)


def _type_of(param: str) -> str:
    """A parameter declaration with its name removed, if it has one."""
    match = re.search(r"(\w+)\s*(\[\s*\])?$", param)
    if match is None or match.group(1) in TYPE_WORDS:
        return param
    stripped = param[:match.start(1)].strip()
    return stripped or param


class Seam:
    def __init__(self, source: Path, header: Path, match: re.Match,
                 typedef: re.Match, symbol: str | None):
        self.source, self.header, self.match = source, header, match
        self.pointer = match.group("name")
        self.address = match.group("address")
        self.symbol = symbol
        self.returns = typedef.group("ret").strip()
        self.params = _split_params(typedef.group("params"))
        self.arguments = _argument_names(self.params)
        self.alias = typedef.group("alias")
        qualified = qualified_name(symbol) if symbol else ""
        self.klass, _, self.method = qualified.rpartition("::")

    @property
    def declaration(self) -> str:
        return f"{self.returns} {self.method}({', '.join(self.params)});"

    def forwarder(self) -> str:
        types = ", ".join(_type_of(p) for p in self.params)
        signature = f"{self.klass} *, void *" + (f", {types}" if types else "")
        passed = "this, nullptr" + (
            ", " + ", ".join(self.arguments) if self.arguments else "")
        return (f"{self.returns} {self.klass}::{self.method}"
                f"({', '.join(self.params)}) {{  // {self.address}\n"
                f"    typedef {self.returns}(__fastcall *pending)({signature});\n"
                f"    {'' if self.returns == 'void' else 'return '}"
                f"PENDING_BODY({self.address}, pending)({passed});\n"
                f"}}\n\n")

    def blockers(self) -> list[str]:
        out = []
        if not self.symbol:
            return [f"{self.address} is in no annotation"]
        if not self.klass:
            out.append(f"{self.symbol} names no class")
        outside = sum(len(re.findall(rf"\b{self.pointer}\b", text))
                      for path, text in tree().items()
                      if path not in SCOPE and self.pointer in text)
        if outside:
            out.append(f"{self.pointer} is named {outside} time(s) in files "
                       f"this run does not rewrite")
        scoped = "\n".join(text for path, text in tree().items()
                            if path in SCOPE and self.pointer in text)
        uses = len(re.findall(rf"\b{self.pointer}\b", scoped))
        calls = len(re.findall(
            rf"\(\s*ORIGINAL\([^()]*\)\s*->\*\s*{self.pointer}\s*\)", scoped))
        declared = 2 + calls  # the binding, the extern declaration, the calls
        if uses > declared:
            out.append(f"{self.pointer} is used {uses - declared} time(s) "
                       f"other than as a call")
        for expression in set(re.findall(
                rf"\(\s*ORIGINAL\(([^()]*)\)\s*->\*\s*{self.pointer}\s*\)",
                "\n".join(body(p) for p in SCOPE))):
            receiver = expression.strip().lstrip("&")
            # `ORIGINAL(...)` swallows any expression because a
            # pointer-to-member call type-checks against OriginalObject.
            # `x->method()` does not: console.cpp hands it `self + 0x22a2c`,
            # where the arrow would bind tighter than the addition, and
            # editgroup.cpp hands it a `void *const` LOCAL, which no
            # declaration rewrite can reach.
            if not re.fullmatch(r"this|[A-Za-z_]\w*", receiver):
                out.append(f"the receiver `{expression.strip()}` is an "
                           f"expression, not a name")
                continue
            if receiver != "this" and not re.search(
                    rf"^(extern\s+)?[\w:]+\s*\*+\s*(const\s+)?{receiver}\s*[;=]",
                    "\n".join(body(p) for p in tree()), re.M):
                out.append(f"`{receiver}` is not a file-scope declaration - "
                           f"a local cannot be retyped from here")
                continue
            seen = RETYPED.get(receiver)
            if seen and seen != self.klass:
                out.append(f"{receiver} is already the {seen} this run gave "
                           f"it; {self.symbol} calls it a {self.klass}")
            enclosing = _enclosing_class(
                body(self.source),
                body(self.source).find(f"ORIGINAL({expression})"))
            if enclosing and enclosing != self.klass and _reaches_publicly(
                    enclosing, self.klass) is False:
                out.append(f"{enclosing} inherits {self.klass} privately, so "
                           f"a direct call needs the hierarchy widened")
        if _wrapper_span(body(self.source), self.klass, self.method,
                         self.pointer):
            return out
        if re.search(rf"\b{re.escape(self.klass)}::{self.method}\s*\(",
                     _code(body(self.source))):
            out.append(f"{self.klass}::{self.method} already has a body in "
                       f"{self.source.name} - is it the wrapper this replaces, "
                       f"or a recovery?")
        return out


def seams(source: Path) -> list[Seam]:
    header = source.with_suffix(".h")
    text = body(source)
    header_text = body(header) if header in tree() else ""
    typedefs = {m.group("alias"): m
                for m in TYPEDEF.finditer(text + "\n" + header_text)}
    names = _names_by_address()
    out = []
    for match in BINDING.finditer(text):
        typedef = typedefs.get(match.group("alias"))
        if typedef is None:
            print(f"  ? {match.group('name')}: no OriginalObject typedef")
            continue
        address = int(match.group("address"), 16)
        out.append(Seam(source, header, match, typedef, names.get(address)))
    return out


def _class_body(text: str, klass: str) -> tuple[int, int] | None:
    """The span between `class K ... {` and its closing brace."""
    opening = re.search(rf"\bclass\s+{re.escape(klass)}\b[^{{;]*\{{", text)
    if opening is None:
        return None
    depth, index = 1, opening.end()
    while index < len(text) and depth:
        depth += {"{": 1, "}": -1}.get(text[index], 0)
        index += 1
    return opening.end(), index - 1


def _declaring_header(klass: str, method: str,
                      signature: tuple[str, ...]) -> tuple[Path, bool] | None:
    """The header that declares `class klass`, and whether `method` is in it.

    Searched across the whole tree, not just the source's own header: a seam
    in graphicwin.cpp routinely names a Buffer or Win method, and putting the
    declaration in graphicwin.h would be a second, conflicting one.
    """
    for header in sorted(p for p in tree() if p.suffix == ".h"):
        text = body(header)
        span = _class_body(text, klass)
        if span is None:
            continue
        inside = text[span[0]:span[1]]
        # ARITY, not just the name: PopMenu declares `init()` and the seam
        # wants `init(int, int)`. Treating the name as enough leaves the
        # overload undeclared and the call site failing on argument count.
        declared = any(
            _signature(m.group(1)) == signature
            for m in re.finditer(rf"\b{re.escape(method)}\s*\(([^)]*)\)",
                                 inside))
        return header, declared
    return None


COMMENT = re.compile(r"/\*.*?\*/|//[^\n]*", re.S)


def _code(text: str) -> str:
    """The text with comments blanked, newlines kept so offsets still line up.

    Every prose block in this tree names the function it is about, so
    `BasePop::init` appears in popmenu.cpp only as "whatever BasePop::init
    returns" - which read as a definition and cost the forwarder.
    """
    return COMMENT.sub(lambda m: re.sub(r"[^\n]", " ", m.group(0)), text)


def _wrapper_span(text: str, klass: str, method: str,
                  pointer: str) -> tuple[int, int] | None:
    """The span of a body whose whole job is to call the seam.

    A same-named body in the file is normally a reason to STOP - it is either
    the wrapper this conversion replaces or a real recovery, and getting that
    backwards makes the function call itself, which compiles. But one shape is
    not ambiguous: a body that does nothing except the seam call is the
    wrapper, and it goes.
    """
    opening = re.search(
        rf"^(?:#pragma[^\n]*\n)?[\w:<>,*&\s]*?\b{re.escape(klass)}::"
        rf"{re.escape(method)}\s*\([^;{{}}]*\)\s*\{{", _code(text), re.M)
    if opening is None:
        return None
    depth, index = 1, opening.end()
    while index < len(text) and depth:
        depth += {"{": 1, "}": -1}.get(text[index], 0)
        index += 1
    inside = _code(text[opening.end():index - 1]).strip()
    if not re.fullmatch(
            rf"(return\s+)?\(\s*ORIGINAL\([^()]*\)\s*->\*\s*"
            rf"{re.escape(pointer)}\s*\)\s*\([^;]*\)\s*;", inside):
        return None
    start = opening.start()
    # A `#pragma auto_inline(off)` above and its `(on)` below belong to the
    # wrapper, not to whatever surrounds it.
    before = text.rfind("#pragma auto_inline(off)", 0, start)
    if before >= 0 and not text[before:start].strip().count("\n\n"):
        start = before
    after = text.find("#pragma auto_inline(on)", index)
    if 0 <= after <= index + 2:
        index = text.index("\n", after) + 1
    return start, index


def _defined_in_build(klass: str, method: str,
                      signature: tuple[str, ...]) -> Path | None:
    """The build input that already defines `klass::method`, if any.

    A seam whose target is ALREADY RECOVERED needs no forwarder - only the
    call sites rewritten. Emitting one anyway is a duplicate definition, and
    the link error names pending_bodies.cpp rather than the seam, so it reads
    as a bug in the forwarder rather than in the conversion.
    """
    for path in sorted(p for p in tree() if p.suffix == ".cpp"):
        if path == PENDING:
            continue
        for match in re.finditer(
                rf"\b{re.escape(klass)}::{re.escape(method)}\s*"
                rf"\(([^;{{}}]*)\)\s*(?:const\s*)?\{{", _code(body(path))):
            # By ARITY: Dialog defines one `init` and the seams want three
            # more, and a name match hands all four to the same definition.
            if _signature(match.group(1)) == signature:
                return path
    return None


def convert(source: Path, apply: bool) -> int:
    """Convert every convertible seam bound in `source`."""
    header = source.with_suffix(".h")
    done = 0
    for seam in seams(source):
        blockers = seam.blockers()
        if blockers:
            print(f"  - {seam.pointer} -> {seam.klass}::{seam.method}")
            for blocker in blockers:
                print(f"      {blocker}")
            continue

        found = _declaring_header(seam.klass, seam.method,
                                  _signature(seam.params))
        if found is None:
            print(f"  - {seam.pointer}: no `class {seam.klass}` under src/")
            continue
        owner, already = found

        # 0. A pure forwarding wrapper is what this conversion replaces.
        span = _wrapper_span(body(source), seam.klass, seam.method,
                             seam.pointer)
        if span:
            rewrite(source, body(source)[:span[0]] + body(source)[span[1]:])

        # 1. The binding goes.
        rewrite(source, body(source).replace(seam.match.group(0), ""))

        # 2. Every call site, in every file this run owns, loses the pointer.
        pattern = rf"\(\s*ORIGINAL\(([^()]*)\)\s*->\*\s*{seam.pointer}\s*\)"
        def substitute(match: re.Match, text: str = "") -> str:
            enclosing = _enclosing_class(text, match.start())
            qualifier = "" if enclosing == seam.klass else f"{seam.klass}::"
            return _receiver(match.group(1).strip()) + qualifier + seam.method
        for path in sorted(SCOPE):
            if seam.pointer not in body(path):
                continue
            for expression in set(re.findall(pattern, body(path))):
                _retype_receiver(expression.strip(), seam.klass)
            source_text = body(path)
            text = re.sub(
                pattern,
                lambda m: substitute(m, source_text), source_text)
            text = EXTERN.sub(
                lambda m: "" if m.group("name") == seam.pointer else m.group(0),
                text)
            rewrite(path, text)

        # 3. The typedef goes too, but only once nothing names it.
        alive = re.sub(rf"typedef[^;]*\b{seam.alias}\b[^;]*;", "",
                       "\n".join(tree().values()))
        if not re.search(rf"\b{seam.alias}\b", alive):
            dead = TYPEDEF.pattern.replace("(?P<alias>\\w+)", seam.alias)
            for path in sorted(tree()):
                if seam.alias in body(path):
                    rewrite(path, re.sub(dead + r"\n?", "", body(path)))

        # 4. The method is declared on its class, once.
        if not already:
            owner_text = body(owner)
            span = _class_body(owner_text, seam.klass)
            rewrite(owner, owner_text[:span[0]]
                    + f"\n public:\n  // {seam.address}, a pending_bodies "
                      f"forwarder.\n  {seam.declaration}\n"
                    + owner_text[span[0]:])

        # 5. Every .cpp that now dereferences the class includes its header - a
        #    forward declaration was enough for `extern K *X;` and is not
        #    enough for `X->method()`.
        include = f'#include "{owner.name}"\n'
        for path in sorted(SCOPE) + [PENDING]:
            text = body(path)
            if path.suffix != ".cpp" or include in text:
                continue
            if f"{seam.method}(" not in text and path is not PENDING:
                continue
            last = text.rfind('#include "')
            if last < 0:
                continue
            end = text.index("\n", last) + 1
            rewrite(path, text[:end] + include + text[end:])

        # 6. An unrecovered target gets a forwarder; a recovered one must not.
        defined = _defined_in_build(seam.klass, seam.method,
                                    _signature(seam.params))
        if defined is None:
            rewrite(PENDING, body(PENDING).replace(
                PENDING_ANCHOR, seam.forwarder() + PENDING_ANCHOR))
        else:
            rewrite(PENDING, body(PENDING).replace(include, "")
                    if include not in body(source) else body(PENDING))
        where = "forwarded" if defined is None else f"already in {defined.name}"
        print(f"  + {seam.pointer} -> {seam.klass}::{seam.method} "
              f"({seam.address}, {where})")
        done += 1
    return done


PRIVATE_BASE = re.compile(
    r"^class\s+(?P<name>\w+)\s*:\s*(?P<bases>[^{;]+)\{", re.M)


def widen(apply: bool) -> int:
    """Make every implicit base `public`.

    `class Popup : BasePop` is PRIVATE, because that is what `class` means, and
    this tree wrote every hierarchy that way. A body that needs to reach a base
    method the image reaches with a direct `call rel32` then cannot compile the
    call at all, and stands in a function-pointer seam instead - which is the
    single largest reason convert_seams refuses.

    Access specifiers change nothing about layout, so this is free. It only
    adds the keyword where NONE is written; `private` and `protected` spelled
    out are decisions and are left alone.
    """
    changed = 0
    for path in sorted(p for p in tree() if p.suffix == ".h"):
        text = body(path)
        out = []
        for match in PRIVATE_BASE.finditer(text):
            bases = []
            for spec in match.group("bases").split(","):
                spec = spec.strip()
                if not spec:
                    continue
                if re.match(r"(public|private|protected|virtual\s+(public|"
                            r"private|protected))\b", spec):
                    bases.append(spec)
                    continue
                bases.append(re.sub(r"^virtual\s+", "virtual public ", spec)
                             if spec.startswith("virtual")
                             else f"public {spec}")
                changed += 1
            out.append((match.group(0),
                        f"class {match.group('name')} : "
                        f"{', '.join(bases)} {{"))
        for old, new in out:
            if old != new:
                text = text.replace(old, new)
        rewrite(path, text)
    if apply:
        for path, text in tree().items():
            if text != path.read_text():
                path.write_text(text)
    return changed


FREE = re.compile(
    r"^(?P<alias>func_\w+)\s*\*const\s+(?P<name>\w+)\s*=\s*"
    r"\((?P=alias)\s*\*\)\s*(?P<address>0x[0-9A-Fa-f]+)\s*;[^\n]*\n", re.M)
FREE_TYPEDEF = re.compile(
    r"typedef\s+(?P<ret>[\w:*&\s]+?)\s*\(\s*__cdecl\s+(?P<alias>func_\w+)"
    r"\s*\)\s*\((?P<params>[^)]*)\)\s*;\n")


def free(apply: bool) -> int:
    """Convert `func_X *const P = (func_X *)0x...` bindings to real calls.

    THE SAME COST AS A MEMBER SEAM, in a different spelling: a call through
    the pointer compiles `call dword ptr [P]` where the image has `call rel32`.
    0x00592EE0 was bound in two headers at once, with disagreeing return types,
    and both files' callers paid for it.

    A free function, so there is no receiver to get wrong and no class to find
    - the whole conversion is a forwarder, a declaration and a rename.
    """
    names = _names_by_address()
    files = tree()
    changed = 0
    for header in sorted(p for p in files if p.suffix == ".h"):
        for match in list(FREE.finditer(body(header))):
            alias, pointer = match.group("alias"), match.group("name")
            address = int(match.group("address"), 16)
            symbol = names.get(address, "")
            if not symbol.startswith("?") or "@@Y" not in symbol:
                print(f"  - {pointer}: {match.group('address')} is "
                      f"{symbol or 'in no annotation'}, not a free function")
                continue
            ident = qualified_name(symbol)
            typedef = FREE_TYPEDEF.search(
                "\n".join(files.values()).replace("\r", ""))
            typedef = next((m for m in FREE_TYPEDEF.finditer(
                "\n".join(files.values())) if m.group("alias") == alias), None)
            if typedef is None:
                print(f"  - {pointer}: no `typedef ... (__cdecl {alias})`")
                continue
            params = _split_params(typedef.group("params"))
            arguments = _argument_names(params)
            returns = typedef.group("ret").strip()
            declaration = (f"{returns} __cdecl {ident}"
                           f"({', '.join(params)});")
            if re.search(rf"(?<![\w:]){re.escape(ident)}\s*\(",
                         _code("\n".join(v for k, v in files.items()
                                         if k.suffix == ".h"))):
                print(f"  - {pointer}: {ident} is already declared somewhere")
                continue
            forwarder = (
                f"{returns} __cdecl {ident}({', '.join(params)}) {{"
                f"  // {match.group('address')}\n"
                f"    typedef {returns}(__cdecl *pending)"
                f"({', '.join(_type_of(p) for p in params)});\n"
                f"    {'' if returns == 'void' else 'return '}"
                f"PENDING_BODY({match.group('address')}, pending)"
                f"({', '.join(arguments)});\n}}\n\n")
            rewrite(header, body(header).replace(
                match.group(0),
                f"// {match.group('address')}, a pending_bodies forwarder.\n"
                f"{declaration}\n"))
            for path in sorted(files):
                if pointer in body(path):
                    rewrite(path, re.sub(rf"(?<![\w:])\b{pointer}\s*\(",
                                         f"{ident}(", body(path)))
            rewrite(PENDING, body(PENDING).replace(
                PENDING_ANCHOR, forwarder + PENDING_ANCHOR))
            include = f'#include "{header.name}"\n'
            if include not in body(PENDING):
                last = body(PENDING).rfind('#include "')
                end = body(PENDING).index("\n", last) + 1
                rewrite(PENDING, body(PENDING)[:end] + include
                        + body(PENDING)[end:])
            print(f"  + {pointer} -> {ident} ({match.group('address')})")
            changed += 1
    if apply:
        for path, text in files.items():
            if text != path.read_text():
                path.write_text(text)
    return changed


if __name__ == "__main__":
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    apply = "--apply" in sys.argv
    if "--widen" in sys.argv:
        count = widen("--apply" in sys.argv)
        print(f"{count} base(s) "
              f"{'widened' if '--apply' in sys.argv else 'widenable'}")
        raise SystemExit(0)
    if "--free" in sys.argv:
        count = free("--apply" in sys.argv)
        print(f"{count} free-function binding(s) "
              f"{'converted' if '--apply' in sys.argv else 'convertible'}")
        raise SystemExit(0)
    SCOPE.update(Path(a).resolve() for a in args)
    SCOPE.update(Path(a).resolve().with_suffix(".h") for a in args)
    SCOPE.intersection_update(tree())
    before = dict(tree())
    total = 0
    for name in args:
        print(name)
        total += convert(Path(name).resolve(), apply)
    if apply:
        for path, text in tree().items():
            if text != before[path]:
                path.write_text(text)
    print(f"{total} seam(s) {'converted' if apply else 'convertible'}")
