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

import contextlib
import io
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

# (class, method, signature) this run has already forwarded.
EMITTED: set = set()


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
            # A FUNCTION-POINTER PARAMETER NAMES ITSELF INSIDE THE PARENS.
            # `int(__cdecl *)() a5` is not a declaration; `int(__cdecl *a5)()`
            # is, and appending blindly produced the former.
            pointer = re.match(r"(.*\*)\s*(\)\s*\(.*\))\s*$", param)
            param = (f"{pointer.group(1)}{name}{pointer.group(2)}"
                     if pointer else f"{param} {name}")
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


def _cast_receiver(expression: str, klass: str) -> str:
    """`expression` as something `->method()` can be applied to."""
    expression = expression.strip()
    if expression == "this":
        return ""
    if re.fullmatch(r"[A-Za-z_]\w*", expression):
        return expression + "->"
    if expression.startswith("&") and re.fullmatch(r"&\s*[A-Za-z_]\w*",
                                                   expression):
        return expression[1:].strip() + "."
    return f"reinterpret_cast<{klass} *>({expression})->"


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
        # A COMPILER-GENERATED NAME IS NOT AN IDENTIFIER. `??_G` demangles to
        # `` `scalar deleting destructor' ``, which cannot be declared - the
        # tool wrote it into a header verbatim and CL reported an unknown
        # character.
        if self.method and not re.fullmatch(r"~?[A-Za-z_]\w*", self.method):
            out.append(f"`{self.method}` is a compiler-generated name, not "
                       f"something a declaration can spell")
        # A DESTRUCTOR TAKES NOTHING AND RETURNS NOTHING, whatever the seam's
        # typedef says. The typedefs here describe the ADJUSTOR - which takes
        # the flags word a deleting destructor is passed - so declaring from
        # them produced `void ~RadioButton(int)`, which is two hard errors.
        if self.method.startswith("~") and (self.params
                                            or self.returns != "void"):
            out.append(f"{self.klass}::{self.method} is a destructor and the "
                       f"seam's typedef gives it a signature")
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
                # AN EXPRESSION IS FINE IF IT CAN BE CAST. `ORIGINAL()`
                # swallowed it by erasing the type; a `reinterpret_cast` to
                # the class puts the type back, which is what the adjustor
                # thunks need - they compute `object - vtordisp` and had no
                # other way to name the receiver.
                if re.search(r"[;{}]", expression):
                    out.append(f"the receiver `{expression.strip()}` is not "
                               f"an expression this can cast")
                continue
            # A LOCAL ALREADY OF THE RIGHT TYPE NEEDS NO RETYPE. mapwin.cpp
            # computes `MapWin *const base = ...` and calls through it; the
            # file-scope test refused that, and the only thing it actually
            # needed was the method declared.
            local = re.search(
                rf"\b{re.escape(self.klass)}\s*\*+\s*(?:const\s+)?"
                rf"{receiver}\s*[;=]", _code(body(self.source)))
            if receiver != "this" and not local and not re.search(
                    rf"^(extern\s+)?[\w:]+\s*\*+\s*(const\s+)?{receiver}\s*[;=]",
                    "\n".join(body(p) for p in tree()), re.M):
                out.append(f"`{receiver}` is neither a file-scope declaration "
                           f"nor a local already typed {self.klass} *")
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
        # A LIFECYCLE METHOD CANNOT BE DECLARED BY NAME. The first bare-tree
        # run reported exactly four convertible seams, applied them, and broke
        # the build four ways: `void Console::Console(int)` is a constructor
        # with a return type (C2380), `void Win::~Win()` a destructor with one
        # (C2631). Every one of the four was a constructor or destructor -
        # the survivors of this tool's other refusals are SKEWED toward
        # lifecycle seams, because ordinary methods with a live wrapper get
        # refused on the wrapper and lifecycle ones often kept none. The
        # remedy is a REAL constructor/destructor, which is `compiler_work.py`
        # 'construct' shape work, not a declaration this tool can write.
        # Destructor seams have their own narrower door: `--dtors`.
        if seam.method == seam.klass or seam.method.startswith("~"):
            print(f"  - {seam.pointer} -> {seam.klass}::{seam.method}")
            print(f"      a {'destructor (see --dtors)' if seam.method.startswith('~') else 'constructor'}"
                  f" cannot be declared as a named method; it needs the real thing")
            continue
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
            return (_cast_receiver(match.group(1), seam.klass)
                    + qualifier + seam.method)
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
        # WHAT THIS RUN HAS ALREADY WRITTEN counts as defined. Two adjustor
        # thunks routinely target ONE method - `thunk1_MapWin::on_redraw` and
        # `thunk3_MapWin::on_redraw` both reach `MapWin::on_redraw` - and
        # `_defined_in_build` cannot see a forwarder added minutes ago because
        # it skips pending_bodies.cpp by design.
        key = (seam.klass, seam.method, _signature(seam.params))
        already = re.search(
            rf"\b{re.escape(seam.klass)}::{re.escape(seam.method)}\s*"
            rf"\([^;{{}}]*\)\s*\{{", _code(body(PENDING)))
        defined = (PENDING if (key in EMITTED or already) else
                   _defined_in_build(seam.klass, seam.method,
                                     _signature(seam.params)))
        EMITTED.add(key)
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


# The thunks reach a destructor under two names - `...DtorTarget` bound with
# `func_deleting_dtor`, and `...ElementTeardown` bound with
# `func_thiscall_teardown` in the owning class's own file. The shape at the
# CALL SITE is identical; only the binding's spelling and home differ.
DTOR_CALL = re.compile(
    r"\(\s*ORIGINAL\(self\)\s*->\*\s*(?P<name>\w+)\s*\)\s*\(\s*\)\s*;")
DTOR_BINDING = re.compile(
    r"^(?:func_deleting_dtor|func_thiscall_teardown)\s+(?P<name>\w+)\s*=\s*"
    r"\n?\s*original_method<(?:func_deleting_dtor|func_thiscall_teardown)>"
    r"\(\s*(?P<address>0x[0-9A-Fa-f]+)\s*\)\s*;\n", re.M)
STUB_DTOR = re.compile(r"^(?P<indent>\s*)~(?P<klass>\w+)\(\)\s*\{\s*;?\s*\}"
                       r"[^\n]*\n", re.M)


def dtors(apply: bool) -> int:
    """Convert scalar deleting destructors to real destructor calls.

    THE SHAPE. `deleting_thunks.cpp` models each `??_GClass@@UAEPAXI@Z` as
    `(ORIGINAL(self)->*ClassDtorTarget)()` followed by a guarded
    `operator delete`. The pointer costs the `call rel32` the image makes to
    `??1Class@@QAE@XZ`, and there are 48 of them.

    THE HALF THAT IS NOT OBVIOUS. Simply calling `~Class()` makes it WORSE,
    because most of these classes carry an empty inline stub - `~Class() { ; }`
    - which emits nothing at all, so the call disappears instead of becoming
    direct. The stub has to become a DECLARATION with a pending_bodies
    forwarder behind it, and then the qualified call reaches the image's body.

    Qualified - `p->Class::~Class()` - because these destructors are virtual
    and the image's call is not.
    """
    names = _names_by_address()
    files = tree()
    changed = 0
    source = (REPO_ROOT / "src" / "deleting_thunks.cpp").resolve()
    # EVERY POINTER THE THUNKS CALL, wherever it is bound. The binding for
    # `EffectElementTeardown` lives in fx.cpp, not beside the thunk that uses
    # it, so looking only in deleting_thunks.cpp missed a dozen.
    wanted = {m.group("name") for m in DTOR_CALL.finditer(body(source))}
    bindings = []
    for path in sorted(files):
        for m in DTOR_BINDING.finditer(body(path)):
            if m.group("name") in wanted:
                bindings.append((path, m))
    for home, match in bindings:
        pointer, address = match.group("name"), match.group("address")
        symbol = names.get(int(address, 16), "")
        found = re.fullmatch(r"\?\?1(\w+)@@QAE@XZ", symbol)
        if not found:
            print(f"  - {pointer}: {address} is {symbol or 'in no annotation'}"
                  f", not a `??1Class@@QAE@XZ`")
            continue
        klass = found.group(1)
        header = next((h for h in sorted(files)
                       if h.suffix == ".h" and _class_body(files[h], klass)),
                      None)
        if header is None:
            print(f"  - {pointer}: no `class {klass}` under src/")
            continue
        span = _class_body(files[header], klass)
        inside = files[header][span[0]:span[1]]
        stub = next((m for m in STUB_DTOR.finditer(inside)
                     if m.group("klass") == klass), None)
        declared = re.search(rf"~{klass}\s*\(", inside)
        if stub is None and declared:
            print(f"  - {pointer}: {klass}'s destructor in {header.name} is a "
                  f"real body, not an empty stub")
            continue
        if re.search(rf"\b{klass}::~{klass}\s*\(", _code(body(PENDING))):
            print(f"  - {pointer}: {klass}::~{klass} is already forwarded")
            continue
        users = [p for p in sorted(files) if pointer in body(p)]
        # A pointer the owning file also PASSES somewhere - fx.cpp hands
        # `EffectElementTeardown` to a vector teardown - is storage, not just
        # a call.
        # A pointer the owning file also PASSES somewhere - fx.cpp hands
        # `EffectElementTeardown` to a vector teardown iterator - is real
        # storage and must stay. The thunk's CALL still converts; only the
        # binding survives.
        passed = [p for p in users
                  if re.search(rf"[(,]\s*{pointer}\s*[),]", body(p))]
        # `original_address(P)` reads the pointer as a VALUE to rebind another
        # seam with; there is no call to rewrite and removing the binding
        # breaks it.
        indirect = [p for p in users
                    if re.search(rf"original_address\(\s*{pointer}\s*\)",
                                 body(p))]
        if indirect:
            print(f"  - {pointer}: {indirect[0].name} reads it as a value "
                  f"through original_address()")
            continue
        note = (f"  // {address} is not recovered: a pending_bodies "
                f"forwarder, because\n  // an empty inline stub emits nothing "
                f"and the deleting destructor\n  // needs a `call rel32`.\n"
                f"  ~{klass}();\n")
        if stub is not None:
            rewrite(header, files[header].replace(stub.group(0), note))
        else:
            # NO DESTRUCTOR AT ALL. hypothesis_layouts.h describes layouts and
            # declares no members, so there is nothing to replace - the
            # declaration is added, public, at the top of the class body.
            rewrite(header, files[header][:span[0]]
                    + f"\n public:\n{note}" + files[header][span[0]:])
        rewrite(PENDING, body(PENDING).replace(
            PENDING_ANCHOR,
            f"{klass}::~{klass}() {{  // ??1{klass}@@QAE@XZ at {address}\n"
            f"    typedef void(__fastcall *pending)({klass} *, void *);\n"
            f"    PENDING_BODY({address}, pending)(this, nullptr);\n}}\n\n"
            + PENDING_ANCHOR))
        if not passed:
            rewrite(home, body(home).replace(match.group(0), ""))
        # EVERY USE, NOT JUST THE THUNK'S. atexit_thunks.cpp reaches the same
        # pointers on fixed addresses - `ORIGINAL(reinterpret_cast<void *>
        # (0x006A7628))->*BaseWinDtorTarget` - and removing the binding without
        # rewriting those leaves an unresolved symbol.
        call = re.compile(rf"\(\s*ORIGINAL\((?P<receiver>[^;]*?)\)\s*->\*"
                          rf"\s*{pointer}\s*\)\s*\(\s*\)")
        for path in (users if not passed else [source]):
            rewrite(path, call.sub(
                lambda m: f"static_cast<{klass} *>({m.group('receiver')})"
                          f"->{klass}::~{klass}()",
                body(path)))
            if not passed:
                rewrite(path, EXTERN.sub(
                    lambda m: "" if m.group("name") == pointer else m.group(0),
                    body(path)))
        for path in (source, PENDING, *users):
            include = f'#include "{header.name}"\n'
            if include not in body(path):
                last = body(path).rfind('#include "')
                end = body(path).index("\n", last) + 1
                rewrite(path, body(path)[:end] + include + body(path)[end:])
        print(f"  + {pointer} -> {klass}::~{klass} ({address})"
              + ("  [binding kept: it is passed elsewhere]" if passed else ""))
        changed += 1
    if apply:
        for path, text in files.items():
            if text != path.read_text():
                path.write_text(text)
    return changed


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
    r"^(?P<alias>func\w*)\s*\*const\s+(?P<name>\w+)\s*=\s*"
    r"\((?P=alias)\s*\*\)\s*(?P<address>0x[0-9A-Fa-f]+)\s*;[^\n]*\n", re.M)
# temp.h numbers its typedefs (`func5`) and omits `__cdecl`, so both are
# optional here; the file is the tree's oldest and its shapes are not the ones
# the newer headers use.
FREE_TYPEDEF = re.compile(
    r"typedef\s+(?P<ret>[\w:*&\s]+?)\s*(?:\(\s*(?:__cdecl\s+)?"
    r"(?P<alias>func\w*)\s*\)|(?:__cdecl\s+)?(?P<alias2>func\w*))"
    # `[^)]*` for the parameters stops at the FIRST `)`, and a callback
    # parameter has one - `typedef int func7(LPCSTR, ..., int(__cdecl *)())`
    # read as taking two. `_split_params` counts depth, so hand it everything
    # up to the last `)` on the line.
    r"\s*\((?P<params>[^;]*)\)\s*;\n")


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
            typedef = next(
                (m for m in FREE_TYPEDEF.finditer("\n".join(files.values()))
                 if (m.group("alias") or m.group("alias2")) == alias), None)
            if typedef is None:
                print(f"  - {pointer}: no `typedef ... (__cdecl {alias})`")
                continue
            # A TYPEDEF SHARED BY SEVERAL BINDINGS IS NOT EVIDENCE. temp.h
            # reuses `func5`, `func13` and friends across functions whose
            # signatures differ, so the typedef describes one of them and lies
            # about the rest - `social_set(faction_id)` compiled against a
            # three-parameter declaration.
            sharing = sum(1 for m in FREE.finditer("\n".join(files.values()))
                          if m.group("alias") == alias)
            if sharing > 1:
                print(f"  - {pointer}: {alias} is shared by {sharing} "
                      f"bindings, so it describes at most one of them")
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
    if "--dtors" in sys.argv:
        count = dtors("--apply" in sys.argv)
        print(f"{count} deleting destructor(s) "
              f"{'converted' if '--apply' in sys.argv else 'convertible'}")
        raise SystemExit(0)
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
    # NO ARGUMENTS MEANS THE WHOLE TREE. Without this, `for name in args` over
    # an empty `args` converted nothing and printed `0 seam(s) convertible` -
    # the SAME SENTENCE it prints when the work is genuinely finished. Run bare,
    # which is the obvious way to ask "is there any seam work left?", it
    # answered "no" over 48 live bindings in 17 files, four of them convertible
    # and the rest carrying refusals nobody could see. It reported that for as
    # long as anyone ran it that way.
    #
    # The whole tree is also the RIGHT scope, not just a convenience.
    # `elsewhere()` refuses a seam whose pointer is read from a file this run
    # will not rewrite, so a narrow invocation MANUFACTURES its own refusals:
    # `WinOriginalClose` is named in five files and is unconvertible from any
    # one of them alone.
    if not args:
        args = [str(p) for p in sorted((REPO_ROOT / "src").glob("*.cpp"))]
    SCOPE.update(Path(a).resolve() for a in args)
    SCOPE.update(Path(a).resolve().with_suffix(".h") for a in args)
    SCOPE.intersection_update(tree())
    before = dict(tree())
    total = 0
    for name in args:
        # The file name only when the file has something to say. Printing it
        # unconditionally was fine for the three files anyone passed by hand
        # and buries four findings under 130 bare paths now that bare means
        # everything.
        buffer = io.StringIO()
        with contextlib.redirect_stdout(buffer):
            found = convert(Path(name).resolve(), apply)
        if buffer.getvalue().strip():
            print(name)
            print(buffer.getvalue(), end="")
        total += found
    if apply:
        for path, text in tree().items():
            if text != before[path]:
                path.write_text(text)
    # A COUNT THAT DOES NOT SAY WHAT IT COUNTED IS NOT AN ANSWER. That is the
    # whole defect above, in one line of output.
    print(f"{total} seam(s) {'converted' if apply else 'convertible'} "
          f"of {len(args):,} file(s) scanned")
    print("  --dtors, --widen and --free are separate populations and are "
          "not counted here")
