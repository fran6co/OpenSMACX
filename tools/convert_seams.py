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
PENDING = REPO_ROOT / "src" / "pending_bodies.cpp"

TYPEDEF = re.compile(
    r"typedef\s+(?P<ret>[\w:*&\s]+?)\s*\(\s*OriginalObject::\*\s*"
    r"(?P<alias>\w+)\s*\)\s*\((?P<params>[^)]*)\)\s*;")
BINDING = re.compile(
    r"(?P<indent>[ \t]*)(?P<alias>\w+)\s+(?P<name>\w+)\s*=\s*\n?\s*"
    r"original_method<(?P=alias)>\(\s*(?P<address>0x[0-9A-Fa-f]+)\s*\)\s*;\n")
EXTERN = re.compile(r"^extern\s+\w+\s+(?P<name>\w+)\s*;\n", re.M)


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


def _argument_names(params: list[str]) -> list[str]:
    """The trailing identifier of each declaration, or a positional name."""
    out = []
    for index, param in enumerate(params):
        match = re.search(r"(\w+)\s*(\[\s*\])?$", param)
        name = match.group(1) if match else ""
        # A trailing identifier is only a NAME if something is left when it
        # goes. `LPSTR` and `int` are whole declarations; `LPSTR text` is not.
        if name and not param[:match.start(1)].strip():
            name = ""
        if not name:
            name = f"a{index + 1}"
            param = f"{param} {name}"
            params[index] = param
        out.append(name)
    return out


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
        types = ", ".join(re.sub(r"\s*\w+\s*$", "", p) or p for p in self.params)
        signature = f"{self.klass} *, void *" + (f", {types}" if types else "")
        passed = "this, nullptr" + (
            ", " + ", ".join(self.arguments) if self.arguments else "")
        return (f"{self.returns} {self.klass}::{self.method}"
                f"({', '.join(self.params)}) {{  // {self.address}\n"
                f"    typedef {self.returns}(__fastcall *pending)({signature});\n"
                f"    {'' if self.returns == 'void' else 'return '}"
                f"PENDING_BODY({self.address}, pending)({passed});\n"
                f"}}\n\n")

    def blockers(self, source_text: str, header_text: str) -> list[str]:
        out = []
        if not self.symbol:
            return [f"{self.address} is in no annotation"]
        if not self.klass:
            out.append(f"{self.symbol} names no class")
        uses = len(re.findall(rf"\b{self.pointer}\b", source_text + header_text))
        calls = len(re.findall(
            rf"\(\s*ORIGINAL\([^()]*\)\s*->\*\s*{self.pointer}\s*\)", source_text))
        declared = 2 + calls  # the typedef's binding, the extern, the calls
        if uses > declared:
            out.append(f"{self.pointer} is used {uses - declared} time(s) "
                       f"other than as a call")
        if re.search(rf"\b{re.escape(self.klass)}::{self.method}\s*\(",
                     source_text):
            out.append(f"{self.klass}::{self.method} already has a body in "
                       f"{self.source.name} - is it the wrapper this replaces, "
                       f"or a recovery?")
        return out


def seams(source: Path) -> list[Seam]:
    header = source.with_suffix(".h")
    text = source.read_text()
    header_text = header.read_text() if header.exists() else ""
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


def _declaring_header(klass: str, method: str) -> tuple[Path, bool] | None:
    """The header that declares `class klass`, and whether `method` is in it.

    Searched across the whole tree, not just the source's own header: a seam
    in graphicwin.cpp routinely names a Buffer or Win method, and putting the
    declaration in graphicwin.h would be a second, conflicting one.
    """
    for header in sorted((REPO_ROOT / "src").glob("*.h")):
        text = header.read_text()
        span = _class_body(text, klass)
        if span is None:
            continue
        body = text[span[0]:span[1]]
        return header, bool(re.search(rf"\b{re.escape(method)}\s*\(", body))
    return None


def _defined_in_build(klass: str, method: str) -> Path | None:
    """The build input that already defines `klass::method`, if any.

    A seam whose target is ALREADY RECOVERED needs no forwarder - only the
    call sites rewritten. Emitting one anyway is a duplicate definition, and
    the link error names pending_bodies.cpp rather than the seam, so it reads
    as a bug in the forwarder rather than in the conversion.
    """
    for path in sorted((REPO_ROOT / "src").glob("*.cpp")):
        if re.search(rf"\b{re.escape(klass)}::{re.escape(method)}\s*\(",
                     path.read_text()):
            return path
    return None


def convert(source: Path, apply: bool) -> int:
    header = source.with_suffix(".h")
    text = source.read_text()
    header_text = header.read_text() if header.exists() else ""
    pending_text = PENDING.read_text()
    touched: dict[Path, str] = {}
    done = 0
    for seam in seams(source):
        blockers = seam.blockers(text, header_text)
        if blockers:
            print(f"  - {seam.pointer} -> {seam.klass}::{seam.method}")
            for blocker in blockers:
                print(f"      {blocker}")
            continue
        text = text.replace(seam.match.group(0), "")
        text = re.sub(
            rf"\(\s*ORIGINAL\(([^()]*)\)\s*->\*\s*{seam.pointer}\s*\)",
            lambda m: _receiver(m.group(1).strip()) + seam.method, text)
        header_text = EXTERN.sub(
            lambda m: "" if m.group("name") == seam.pointer else m.group(0),
            header_text)
        header_text = re.sub(
            rf"{TYPEDEF.pattern}".replace("(?P<alias>\\w+)", seam.alias),
            "", header_text)
        found = _declaring_header(seam.klass, seam.method)
        if found is None:
            print(f"  - {seam.pointer}: no `class {seam.klass}` under src/")
            continue
        owner, already = found
        if not already:
            owner_text = touched.get(owner)
            if owner_text is None:
                owner_text = header_text if owner == header else owner.read_text()
            span = _class_body(owner_text, seam.klass)
            owner_text = (
                owner_text[:span[0]]
                + f"\n public:\n  // {seam.address}, a pending_bodies forwarder.\n"
                + f"  {seam.declaration}\n"
                + owner_text[span[0]:])
            if owner == header:
                header_text = owner_text
            else:
                touched[owner] = owner_text
        defined = _defined_in_build(seam.klass, seam.method)
        if defined is None:
            pending_text = pending_text.replace(
                "// ?write_raw_l@Buffer@@QAEHPADHHH@Z at 0x005DBD00",
                seam.forwarder()
                + "// ?write_raw_l@Buffer@@QAEHPADHHH@Z at 0x005DBD00")
        where = "forwarded" if defined is None else f"already in {defined.name}"
        print(f"  + {seam.pointer} -> {seam.klass}::{seam.method} "
              f"({seam.address}, {where})")
        done += 1
    if apply and done:
        source.write_text(text)
        if header.exists():
            header.write_text(header_text)
        for path, body in touched.items():
            path.write_text(body)
        PENDING.write_text(pending_text)
    return done


if __name__ == "__main__":
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    apply = "--apply" in sys.argv
    total = 0
    for name in args:
        path = Path(name)
        print(path)
        total += convert(path, apply)
    print(f"{total} seam(s) {'converted' if apply else 'convertible'}")
