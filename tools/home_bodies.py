#!/usr/bin/env -S uv run python
"""Home an artifact body into the product TU that owns it.

A body in src/recovered/ or src/unrecovered/ cannot be MEASURED - the build
never compiles those files - so no agent can finish it and no claim on it can
be checked. Homing moves it into the translation unit that owns its class,
where the gate sees it. MISMATCH after a homing is PROGRESS: the body went
from unverifiable to measured.

    tools/home_bodies.py --class Palette              # proposals, no writes
    tools/home_bodies.py --class Palette --apply      # unclaimed batch
    tools/home_bodies.py --class Palette --claimed --apply 0x005FE6D0
    tools/home_bodies.py --tu log.cpp                 # free functions

THE ORDERING RULE, and why it is not a flag you can ignore: 963 BYTE_EXACT
claims live in these scaffolds. Homing an UNCLAIMED body is free - nothing
can regress. Homing a CLAIMED one can legitimately turn the gate red: a
different TU means different includes, and VC6 may lower identical source
differently there. So unclaimed bodies batch freely; a claimed body moves
ONE PER COMMIT through --claimed <addr>, and if the gate goes REGRESSED the
fix is the include context or reverting that single homing.

WHAT ONE HOMING DOES. Take the marker, its fact block, and the definition
from the artifact; append them to the owning TU; declare the method on its
class in the header if no declaration is there; remove a PENDING_BODY
forwarder of the same member from pending_bodies.cpp, whose definition this
one replaces; delete the artifact file. Then rebuild. A body that does not
compile is REVERTED whole - TU edit, declaration, forwarder removal, file -
and the compiler's first error is printed as the class-modelling blocker it
is. The tool never leaves a half-homed body behind.
"""

from __future__ import annotations

import collections
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from decomp import read
from decomp.asm import build_inputs

REPO = Path(__file__).resolve().parent.parent
SRC = REPO / "src"
PENDING = SRC / "pending_bodies.cpp"

MARKER = re.compile(r"^// ORIGINAL: 0x", re.M)
DEFINITION = re.compile(
    r"^[\w:<>,*&\s]*?\b(?P<klass>\w+)::(?P<method>~?\w+)\s*\((?P<params>[^)]*)\)"
    r"(?:\s*const)?\s*\{", re.M)


def owner_of(record) -> str | None:
    parts = (record.name or "").split("@")
    if len(parts) > 1 and parts[0].startswith("?") and parts[1]:
        return parts[1]
    return None


def code(text: str) -> str:
    """Comments blanked, offsets kept - prose must not look like code."""
    text = re.sub(r"/\*.*?\*/", lambda m: " " * len(m.group(0)), text,
                  flags=re.S)
    return re.sub(r"//[^\n]*", lambda m: " " * len(m.group(0)), text)


def product_tu(klass: str, records) -> Path | None:
    """The .cpp defining the most members of `klass`; stem-matched wins ties."""
    counts: dict = collections.Counter()
    for path in sorted(SRC.glob("*.cpp")):
        if "recovered" in path.parts or "unrecovered" in path.parts:
            continue
        n = len(re.findall(rf"\b{re.escape(klass)}::~?\w+\s*\(",
                           code(path.read_text(errors="replace"))))
        if n:
            counts[path] = n
    if not counts:
        return None
    best = max(counts.values())
    tied = [p for p, n in counts.items() if n == best]
    if len(tied) > 1:
        for p in tied:
            if p.stem.lower() == klass.lower():
                return p
    return tied[0]


def free_definition_span(text: str, ident: str):
    """(head_end, definition) for a FREE function named `ident`.

    The first brace-opened definition whose name is exactly `ident` -
    declarations end in `;`, so they cannot shadow it, and helpers defined
    above it have their own names.
    """
    for match in re.finditer(rf"\b({re.escape(ident)})\s*\(([^;{{}}]*)\)\s*\{{",
                             text):
        start = text.rfind("\n", 0, match.start()) + 1
        prefix = text[start:match.start()]
        if not re.fullmatch(r"[\w:<>,*&\s]*", prefix):
            continue
        return start, text[match.start():].rstrip() + "\n"
    return None


def split_artifact(text: str) -> tuple[str, str] | None:
    """(marker+facts+prose, definition) - the scaffold preamble stays behind.

    The head is the run of `//` lines from the marker on - some artifacts
    have no blank line before the definition, so an index-of-blank-line cut
    silently swallowed whole bodies.
    """
    marker = MARKER.search(text)
    if marker is None:
        return None
    idx = text.index("\n", marker.start()) + 1
    while idx < len(text):
        line_end = text.find("\n", idx)
        line = text[idx:line_end if line_end != -1 else len(text)]
        if not line.startswith("//"):
            break
        idx = line_end + 1 if line_end != -1 else len(text)
    head = text[marker.start():idx].rstrip()
    definition = DEFINITION.search(text, idx)
    if definition is None:
        return None
    return head, text[definition.start():].rstrip() + "\n"


def class_span(text: str, klass: str):
    opening = re.search(rf"\b(?:class|struct)\s+{re.escape(klass)}\b[^{{;]*{{",
                        code(text))
    if opening is None:
        return None
    depth, i = 1, opening.end()
    while i < len(text) and depth:
        depth += {"{": 1, "}": -1}.get(text[i], 0)
        i += 1
    return opening.end(), i - 1


def declared(header_text: str, klass: str, method: str) -> bool:
    span = class_span(header_text, klass)
    if span is None:
        return False
    return bool(re.search(rf"\b{re.escape(method)}\s*\(",
                          header_text[span[0]:span[1]]))


def pending_forwarder_span(text: str, klass: str, method: str):
    """The PENDING_BODY forwarder defining klass::method, if one exists."""
    match = re.search(rf"[\w:<>,*& ]+\b{re.escape(klass)}::{re.escape(method)}"
                      rf"\s*\([^;{{}}]*\)\s*\{{", code(text))
    if match is None:
        return None
    depth, i = 1, match.end()
    while i < len(text) and depth:
        depth += {"{": 1, "}": -1}.get(text[i], 0)
        i += 1
    start = text.rfind("\n", 0, match.start()) + 1
    return start, i


def build() -> tuple[bool, str]:
    done = subprocess.run(["cmake", "--build", "build"], cwd=REPO,
                          capture_output=True, text=True)
    out = done.stdout + done.stderr
    error = next((line.strip() for line in out.splitlines()
                  if re.search(r"error C\d+|LNK\d{4}:.*error|unresolved external",
                               line)), "")
    return done.returncode == 0, error


def main() -> int:
    args = sys.argv[1:]
    apply = "--apply" in args
    claimed_flag = "--claimed" in args
    wanted_addr = None
    if claimed_flag:
        addrs = [a for a in args if a.startswith("0x")]
        if not addrs:
            print("--claimed needs the one address to move")
            return 2
        wanted_addr = int(addrs[0], 16)

    klass = None
    tu_arg = None
    for flag, value in zip(args, args[1:] + [""]):
        if flag == "--class":
            klass = value
        if flag == "--tu":
            tu_arg = Path(value)

    records = read(SRC)
    built = build_inputs(REPO / "build" / "compile_commands.json")

    population = []
    for record in records:
        path = str(record.path)
        if "recovered" not in path and "unrecovered" not in path:
            continue
        if klass and owner_of(record) != klass:
            continue
        if tu_arg and record.path.name != tu_arg.name:
            continue
        population.append(record)

    claimed = [r for r in population if r.byte_exact or r.semantic]
    unclaimed = [r for r in population if not (r.byte_exact or r.semantic)]

    print(f"{len(population)} artifact body(ies) owned by "
          f"{klass or tu_arg and tu_arg.name}: "
          f"{len(unclaimed)} unclaimed, {len(claimed)} claimed")
    if claimed_flag:
        targets = [r for r in claimed if r.address == wanted_addr]
        if not targets:
            print(f"0x{wanted_addr:08X} is not a claimed artifact body here")
            return 2
        targets = targets[:1]
    else:
        targets = unclaimed
        if claimed and not klass and not tu_arg:
            pass
        elif claimed:
            print(f"  ({len(claimed)} CLAIMED body(s) need --claimed <addr>, "
                  f"one per commit)")

    if not targets:
        print("nothing to home")
        return 0

    if not apply:
        for record in sorted(targets, key=lambda r: r.address):
            state = "CLAIMED" if (record.byte_exact or record.semantic) \
                else "unclaimed"
            print(f"  would home {record.address_hex} [{state}] "
                  f"{record.path.name} -> {record.name}")
        print(f"\ndry run: {len(targets)} proposal(s); pass --apply to move"
              f" them (claimed bodies one at a time via --claimed <addr>)")
        return 0

    failures = kept = 0
    carried: set = set()
    for record in sorted(targets, key=lambda r: r.address):
        if tu_arg is not None:
            ident = (record.name or "").split("@")[0].lstrip("?")
            pair = free_definition_span(record.path.read_text(errors="replace"),
                                        ident)
        else:
            pair = split_artifact(record.path.read_text(errors="replace"))
        head_and_def = pair
        if head_and_def is None:
            print(f"  SKIP {record.address_hex}: no marker+definition pair in "
                  f"{record.path.name}")
            continue
        head, definition = head_and_def
        method_match = DEFINITION.search(definition)
        if method_match is not None:
            d_klass = method_match.group("klass")
            method = method_match.group("method")
        else:
            # A FREE function - --tu mode. No class to declare it on and no
            # pending forwarder can exist for an unqualified name.
            d_klass, method = None, (record.name or "").split("@")[0].lstrip("?")
        tu = product_tu(d_klass, records) if not tu_arg else \
            SRC / tu_arg.name
        if tu is None:
            print(f"  BLOCKED {record.address_hex}: no product TU defines "
                  f"{d_klass}::* - model the class first")
            continue

        tu_text = tu.read_text()
        header = tu.with_suffix(".h")
        header_text = header.read_text() if header.exists() else ""
        pending_text = PENDING.read_text()

        # snapshots, so a failed build restores ALL FOUR edits
        snapshots = {
            "tu": tu_text,
            "header": header_text,
            "pending": pending_text,
            "artifact": record.path.read_text(),
        }

        def revert(where: str, error: str):
            nonlocal failures
            tu.write_text(snapshots["tu"])
            if header.exists():
                header.write_text(snapshots["header"])
            PENDING.write_text(snapshots["pending"])
            record.path.write_text(snapshots["artifact"])
            failures += 1
            print(f"  REVERTED {record.address_hex} ({where}): {error[:160]}")

        # THE RECONCILE STEP. Artifacts bind every fixed address they touch
        # as `static T *const g_HEX = (T *)0xADDR;` in their scaffold
        # preamble - which split_artifact throws away. Without those lines
        # the moved body cannot compile, and hand-copying them was the whole
        # of the Palette pass's whack-a-mole. Carry them verbatim into the
        # target TU instead; renaming them to modelled names is the class
        # pass's semantic half, done against a MEASURED body.
        full_artifact = snapshots["artifact"]
        # TYPEDEFS TRAVEL TOO. An artifact declares the Win32 entry points it
        # calls as its own function-pointer types - `typedef int (__stdcall
        # *IntersectRectFn)(RECT *, const RECT *, const RECT *);` - and the
        # scaffold preamble that holds them is thrown away with everything
        # else. Carrying only the `static ... = (...)0xADDR;` bindings homed
        # the body and left it referring to a type nothing declares: eleven
        # of Win's fifty-two failed on exactly that, one `undeclared
        # identifier` at a time.
        bindings = re.findall(
            r"^static .+?= \(.*?0x[0-9A-Fa-f]+;\n", full_artifact, re.M)
        fresh = [b for b in bindings
                 if not (b in tu_text or b in carried)]

        # TYPEDEFS ARE NOT CARRIED, AND TWO ATTEMPTS SAY WHY. An artifact
        # declares the Win32 entry points it calls as its own function-pointer
        # types, and a homed body that loses them fails on `undeclared
        # identifier`. Carrying them by exact string collides with the same
        # name spelled differently in the target TU: 1 homed became 0, with 36
        # `redefinition; different basic types`. Carrying them only when the
        # name is not already in the TU TEXT is no better, because the text
        # cannot see through `#include` - `HBITMAP` is declared in a Windows
        # header, so every artifact's copy of it looked fresh and redefined it.
        #
        # A correct carry has to react to the COMPILER rather than guess ahead
        # of it: build, and only on `undeclared identifier 'X'` go looking for
        # X's declaration. Until that exists, a body needing a typedef is
        # REVERTED and says so, which is a true report an agent can act on -
        # not a silent loss.
        carried_block = "".join(fresh)

        new_tu = tu_text.rstrip() + "\n\n" + (
            ("// Fixed-slot bindings carried from " + record.path.name + "\n"
             + carried_block + "\n") if carried_block else ""
        ) + head.strip() + "\n\n" + definition + "\n"
        tu.write_text(new_tu)

        if d_klass and header_text and not declared(header_text, d_klass, method):
            span = class_span(header_text, d_klass)
            if span is None:
                revert("header", f"no class {d_klass} in {header.name}")
                continue
            sig_head = definition.split("{")[0].strip()
            ret = re.match(r"([\w:<>,*&\s]+?)\b" + re.escape(d_klass) +
                           r"\s*::", sig_head)
            ret_type = (ret.group(1).strip() + " ") if ret else ""
            insertion = (f"\n public:\n  // homed from {record.path.name}\n"
                         f"  {ret_type}{method}({method_match.group('params')});\n")
            header.write_text(header_text[:span[0]] + insertion
                              + header_text[span[0]:])

        fwd = pending_forwarder_span(pending_text, d_klass, method) \
            if d_klass else None
        if fwd is not None:
            PENDING.write_text(
                pending_text[:fwd[0]] + pending_text[fwd[1]:])

        record.path.unlink()

        # THE CARRY REACTS TO THE COMPILER INSTEAD OF GUESSING AHEAD OF IT.
        # An artifact declares the Win32 entry points it calls as its own
        # function-pointer typedefs, and the scaffold preamble holding them is
        # thrown away with everything else - 42 of Win's 49 first-pass reverts
        # were a name the moved body could no longer see.
        #
        # Carrying typedefs speculatively does not work, and both wrong ways
        # were measured: by exact string it collides with the same name spelled
        # differently in the target TU (1 homed became 0, 36 `redefinition;
        # different basic types`), and skipping names already in the TU text is
        # no better because text cannot see through `#include` - `HBITMAP` lives
        # in a Windows header, so every artifact's copy looked fresh.
        #
        # Only the compiler knows what is missing. Build; if it says a name is
        # UNDECLARED, go find that name's declaration in the artifact and add
        # it. A name the compiler calls undeclared cannot be a redefinition, so
        # this direction is safe by construction where guessing was not.
        ok, error = build()
        for _ in range(8):
            if ok:
                break
            missing = re.search(
                r"error C2065: '(\w+)' : undeclared identifier|"
                r"error C2061: syntax error : identifier '(\w+)'", error)
            if not missing:
                break
            token = missing.group(1) or missing.group(2)
            found = re.search(
                rf"^(?:typedef|static|extern)[^\n;]*\b{re.escape(token)}\b"
                rf"[^\n;]*;\n", full_artifact, re.M)
            if not found:
                # A WHOLE CLASS, NOT JUST A LINE. Artifacts that dispatch
                # through a vtable slot carry their own `class VCall { ...
                # void slot004(); };` shim, and split_artifact drops it with
                # the rest of the preamble - 18 of Win's reverts are one of
                # those names. Take the definition entire, brace-matched,
                # since a class body spans lines where a typedef does not.
                head_ = re.search(rf"^(?:class|struct)\s+{re.escape(token)}\b"
                                  rf"[^\n{{]*\{{", full_artifact, re.M)
                if head_:
                    k = head_.end() - 1
                    depth = 0
                    while k < len(full_artifact):
                        if full_artifact[k] == "{":
                            depth += 1
                        elif full_artifact[k] == "}":
                            depth -= 1
                            if depth == 0:
                                break
                        k += 1
                    end = full_artifact.find("\n", k) + 1
                    found = re.match(r"(?s).*", full_artifact[head_.start():end])
                    class Found:
                        def __init__(self, s): self._s = s
                        def group(self, _n=0): return self._s
                    found = Found(full_artifact[head_.start():end])
            if not found or found.group(0) in tu.read_text():
                break
            fresh.append(found.group(0))
            tu.write_text(tu.read_text().replace(
                head.strip(), found.group(0) + "\n" + head.strip(), 1))
            ok, error = build()
        if not ok:
            revert("build", error or "link failed")
            continue
        # A CARRY IS ONLY REMEMBERED IF IT SURVIVED. `carried` used to be
        # updated before the build, and `revert` puts the file back - so a
        # body that carried `g_009b7b34` and was then reverted for an
        # unrelated reason left the set claiming that binding was in the TU
        # when it was not. Every later body needing it skipped the carry and
        # failed with `undeclared identifier`, which is how ONE failure
        # became a cascade: Win homed 1 of 52, and ten of the fifty reverts
        # were this and nothing else.
        for b in fresh:
            carried.add(b)
        kept += 1
        print(f"  HOMED {record.address_hex} -> {tu.name} "
              f"{'CLAIMED - gate before committing' if record.byte_exact else ''}")

    print(f"\n{kept} homed, {failures} reverted. "
          f"Next: bare osmx check; orphan_artifacts.py; lower the artifact "
          f"ceilings this commit made stale.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
