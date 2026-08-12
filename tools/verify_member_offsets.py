#!/usr/bin/env python3
"""Check every declared member boundary against the code that indexes past it.

`sizeof` is the only thing this tree checks about a layout, and sizeof is blind
in the direction that matters. Drop a member and append a pad of the same width
and the assertion still passes; every offset after the hole is wrong and nothing
says so. That is not hypothetical - it is the damage case
`test_verify_member_offsets.py` replays.

TWO EXACT SOURCES, NEITHER OF THEM A MODEL.

  declared   what `src/` actually compiles to. A probe translation unit per
             header, `#define private public` in front of it, and a table of
             `(unsigned long)&((C *)0)->m` and `sizeof(((C *)0)->m)` per member.
             VC6 constant-folds both into the object's data, so the numbers are
             read back out of the `.obj` beside a sentinel. Bases, virtual
             bases and embedded objects are all included because the real class
             is what gets asked - there is no offset arithmetic here to be
             wrong.
  observed   what the image does. The `[this + N]` accesses of a class's own
             thiscall methods, with the WIDTH of each access, collected the way
             `derive_access_bounds.reach` collects its maximum - same receiver
             tracking, same three restrictions, and the same consequence: it
             under-reports and never over-reports.

WHAT A DISAGREEMENT MEANS. An access is evidence about one contiguous scalar:
a `mov eax, [ecx+0x54]` says bytes 0x54..0x57 are one four-byte thing. So

  straddle   the access crosses a declared member's end. The declaration
             splits a field the code reads whole, or joins two the code reads
             apart. Either way the boundary is fiction.
  overrun    the access starts at or past `sizeof`. The class is SHORT - the
             size is too small, or a trailing member was never declared. This
             is the one that refutes a pinned size outright.
  typing     an x87 access names the member's TYPE. Integer code cannot reach
             a member with `fld`, so a four-byte one is a float and an
             eight-byte one a double. This is the only member typing in the
             tree read off the instruction rather than inferred from a name.
  unclaimed  the access lands in a gap no member covers. Not a violation:
             padding and anonymous storage are legitimate, and the member
             scanner can also just have missed a declaration.

`--check` fails on the first two, and on any header whose probe unit REFUSED -
a header that does not compile contributes no declared layout, so every access
into its classes falls through every test and the run would otherwise print OK
having measured nothing.

`--pins` turns the same evidence on the `static_assert(sizeof(X) == N)` lines,
which everything downstream believes and nothing checked. Where a class's own
code writes its last byte, an independent source agrees with the pin exactly;
where it stops short the pin is unconfirmed rather than wrong, and saying which
is which is the point.

The straddle test is deliberately silent about a uniform shift among members of
equal width - relabelling four dwords cannot be seen by it. It catches wrong
WIDTHS and missing members, which is where the errors in this tree actually
are, and it catches them without needing a recovered body to exercise them. A
shift is not beyond reach by other means: `ReportWin`'s Flic was found four
bytes early exactly this way, because the float the IDB's row swallowed showed
up as a typing finding on the object that was standing in its place.
"""

from __future__ import annotations

import argparse
import collections
import csv
import re
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import class_layouts  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC = REPO_ROOT / "src"
ACCESSES_CSV = REPO_ROOT / "docs" / "recovery" / "member-accesses.csv"

# The probe table is found in the object by looking for this, twice, with the
# payload between. A symbol-table walk would work too and would break on the
# first COMDAT the compiler decides to fold; a sentinel does not care where the
# data landed.
SENTINEL = 0xA5C30FF5
PROBE_STRIDE = 2          # each member contributes (offset, size)


# ---------------------------------------------------------------- declared ---

# A data member declaration, permissive on the type and strict on the shape.
MEMBER_RE = re.compile(
    r"^\s*(?:(?:public|private|protected)\s*:\s*)?"
    r"(?!return\b|typedef\b|using\b|friend\b|static\b|virtual\b|explicit\b)"
    r"(?P<type>(?:const\s+|volatile\s+|unsigned\s+|signed\s+|struct\s+|class\s+"
    r"|enum\s+|long\s+|short\s+)*[A-Za-z_]\w*(?:\s*::\s*\w+)*"
    r"(?:\s*\*+\s*|\s+))"
    r"(?P<name>\w+)\s*(?P<array>(?:\[[^\]]*\])*)\s*;",
    re.M)
# `void (__cdecl *callback1_)(int);` - real storage whose name sits inside the
# parentheses. Skipping these cost the coverage that mattered: `Time` declares
# two of them back to back, and its constructor writing 0x8 and 0xC then read
# as two holes in a class that has none.
# ANCHORED AT THE LINE START, and that is what tells a member from a
# PARAMETER. `void init(void(__cdecl *callback)(int), int param);` holds the
# same shape inside its parameter list, and an unanchored pattern read
# `callback` as a member of `Time` - `C2039`, taking the whole header's probe
# down with it.
FUNCTION_POINTER_RE = re.compile(
    r"^\s*(?:(?:public|private|protected)\s*:\s*)?"
    r"[\w\s:*&]+\(\s*(?:__\w+\s+)?\*+\s*(?P<name>\w+)\s*\)"
    r"\s*\([^;()]*\)\s*;", re.M)
# `struct PaletteInternal { ... } internal_[5];` - the name follows the closing
# brace, so a scan that drops brace blocks whole drops the member too.
AFTER_BRACE_RE = re.compile(r"^\s*\**\s*(?P<name>\w+)\s*(?:\[[^\]]*\])*\s*;")


def member_names(body: str) -> list:
    """[member name] in declaration order, for a class body.

    Nested type definitions are removed, and the member DECLARED BY one is
    kept. A member of a nested struct is not a member of the class, and asking
    for it on the wrong type is `C2039: is not a member of`, which fails the
    whole probe unit rather than that one member.
    """
    depth, kept, index = 0, [], 0
    for match in re.finditer(r"[{}]", body):
        if match.group() == "{":
            if depth == 0:
                kept.append(body[index:match.start()])
            depth += 1
        else:
            depth -= 1
            if depth == 0:
                index = match.end()
                tail = AFTER_BRACE_RE.match(body[index:])
                if tail:
                    kept.append(f"struct_tag {tail.group('name')};")
    if depth == 0:
        kept.append(body[index:])
    names = {}
    for chunk in kept:
        for pattern in (MEMBER_RE, FUNCTION_POINTER_RE):
            for found in pattern.finditer(chunk):
                name = found.group("name")
                if name not in names:
                    spelled = (found.groupdict().get("type")
                               or found.group(0).split("(")[0])
                    names[name] = " ".join(spelled.split())
    return names


NAMESPACE_RE = re.compile(r"^\s*namespace\b[^{;]*\{", re.M)


def without_namespaces(text: str) -> str:
    """The text with every `namespace X { ... }` block cut out.

    A class inside a namespace is not a class the image has. Every mangled
    name in `functions.csv` scopes its class directly under `@@`, so an
    in-namespace type can never be the receiver an access is attributed to -
    and naming one unqualified in the probe is `C2065`, which takes down the
    whole header's unit. `runtime_oracle.h` is the live case: test machinery
    under `namespace runtime_oracle`, refusing a header that has nothing to
    contribute either way.
    """
    while True:
        found = NAMESPACE_RE.search(text)
        if not found:
            return text
        depth, index = 1, found.end()
        while index < len(text) and depth:
            if text[index] == "{":
                depth += 1
            elif text[index] == "}":
                depth -= 1
            index += 1
        text = text[:found.start()] + text[index:]


def classes(src: Path = SRC) -> dict:
    """{class: (header, [own member names], [non-virtual base names])}.

    OWN members only. Asking a derived class for an inherited member does not
    compile here: `class BaseWin : GraphicWin` inherits PRIVATELY - that is
    what a `class` with no access keyword means - so `buffer_` is inaccessible
    from outside however public `GraphicWin` makes it, and `#define private
    public` cannot reach the inheritance specifier. 32 of 55 headers refused
    on exactly that. The base's members are measured on the BASE and shifted
    by its subobject offset instead, which is a C-style cast away.
    """
    bodies, home, bases = {}, {}, {}
    for header in sorted(src.glob("*.h")):
        text = without_namespaces(header.read_text(errors="ignore"))
        for name, spec, body in class_layouts.class_bodies(text):
            if name in bodies:
                continue
            bodies[name] = body
            home[name] = header.name
            bases[name] = [base for base, virtual
                           in (class_layouts.bases_of(spec) or [])
                           if not virtual]

    found = {}
    for name, body in bodies.items():
        wanted = [base for base in bases[name] if base in bodies]
        names = member_names(body)
        if names or wanted:
            found[name] = (home[name], names, wanted)
    return found


# A base subobject's offset, and a member's offset inside a privately inherited
# base. Both go through a C-style cast, which is allowed to convert to an
# INACCESSIBLE base where `static_cast` is not - and both start from a non-null
# dummy, because `(Base *)(Derived *)0` is required to stay null and folds to 0
# rather than to the offset. VIRTUAL bases are excluded upstream: the
# adjustment there is read out of the vbtable at run time, so nothing folds.
DUMMY = 0x1000


def probe_unit(header: str, wanted: dict) -> str:
    """A translation unit whose data section states every member's offset."""
    lines = ["#define private public", "#define protected public",
             '#include "stdafx.h"', f'#include "{header}"',
             "extern \"C\" unsigned long probe_table[] = {",
             f"  0x{SENTINEL:08X}ul,"]
    for klass, (names, bases) in wanted.items():
        lines.append(f"  sizeof({klass}),")
        for member in names:
            lines.append(f"  (unsigned long)&((({klass} *)0)->{member}),")
            lines.append(f"  (unsigned long)sizeof((({klass} *)0)->{member}),")
        for base in bases:
            lines.append(f"  (unsigned long)({base} *)"
                         f"({klass} *)0x{DUMMY:X}ul - 0x{DUMMY:X}ul,")
    lines += [f"  0x{SENTINEL:08X}ul,", "};", ""]
    return "\n".join(lines)


def read_table(obj: bytes) -> list:
    """The dwords the probe unit published, between the two sentinels."""
    marker = struct.pack("<I", SENTINEL)
    start = obj.find(marker)
    if start < 0:
        return []
    end = obj.find(marker, start + 4)
    if end < 0:
        return []
    count = (end - start - 4) // 4
    return list(struct.unpack_from(f"<{count}I", obj, start + 4))


def declared(src: Path = SRC, work: Path = None) -> tuple:
    """({class: [(member, offset, size)]}, {class: sizeof}, [diagnostics]).

    One unit per header, all of them in one `cl` invocation. A header whose
    unit does not compile yields nothing and says so; it does not take the
    others down with it, which is the whole reason for the per-header split.
    """
    import byte_match as bm

    by_header = collections.defaultdict(dict)
    for klass, (header, names, bases) in classes(src).items():
        by_header[header][klass] = (names, bases)

    holder = None
    if work is None:
        holder = tempfile.TemporaryDirectory(prefix="member-offsets-")
        work = Path(holder.name)
    units = {}
    for header, wanted in by_header.items():
        units[Path(header).stem] = probe_unit(header, wanted)
    # The SAME include path the DLL build uses. `cmake/vc6-include` is where
    # this tree keeps the C99 headers VC6 predates, and a probe without it
    # refuses any header that spells `#include <stdint.h>` itself.
    shim = REPO_ROOT / "cmake" / "vc6-include"
    objects, diagnostics = bm.compile_batch(
        units, work, f"/c /nologo /W0 /I{src} /I{shim}")

    own, sizes, at, refused = {}, {}, {}, []
    for header, wanted in sorted(by_header.items()):
        stem = Path(header).stem
        obj = objects.get(stem)
        table = read_table(obj) if obj else []
        expected = sum(1 + PROBE_STRIDE * len(names) + len(bases)
                       for names, bases in wanted.values())
        if len(table) != expected:
            why = (diagnostics or {}).get(stem) or ""
            first = next((line for line in str(why).splitlines()
                          if "error" in line), "no usable table")
            refused.append(f"{header}: {first.strip()}")
            continue
        cursor = 0
        for klass, (names, bases) in wanted.items():
            sizes[klass] = table[cursor]
            cursor += 1
            rows = []
            for member, spelling in names.items():
                rows.append((member, table[cursor], table[cursor + 1],
                             spelling))
                cursor += PROBE_STRIDE
            own[klass] = rows
            for base in bases:
                at.setdefault(klass, {})[base] = table[cursor]
                cursor += 1
    if holder is not None:
        holder.cleanup()

    # A base's members belong to the derived object at the base's own offsets
    # plus where the subobject sits. Resolved bottom-up so a chain of any depth
    # composes; a base whose own probe refused contributes nothing rather than
    # contributing zeros.
    layouts, resolving = {}, set()

    def resolve(klass):
        if klass in layouts:
            return layouts[klass]
        if klass in resolving or klass not in own:
            return []
        resolving.add(klass)
        rows = list(own[klass])
        for base, offset in sorted(at.get(klass, {}).items()):
            for member, inner, size, spelling in resolve(base):
                rows.append((f"{base}::{member}", offset + inner, size,
                             spelling))
        resolving.discard(klass)
        layouts[klass] = sorted(rows, key=lambda row: row[1])
        return layouts[klass]

    for klass in own:
        resolve(klass)
    return layouts, sizes, refused


# ---------------------------------------------------------------- observed ---

def accesses(exe: Path) -> dict:
    """{class: {(offset, width): evidence}} - every byte range read or written.

    `derive_access_bounds.reach` keeps only the furthest of these and throws the
    rest away. The receiver tracking is the same code path, so the guarantee is
    the same one its docstring earns: any write to a tracked register drops it,
    `lea` propagates but is not itself an access, and a negative displacement
    belongs to a base subobject rather than to this class.
    """
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    from capstone.x86 import X86_INS_LEA, X86_INS_MOV, X86_OP_MEM, X86_OP_REG

    import derive_access_bounds as bounds
    import derive_class_layout as layout

    image = layout.Image(exe)
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    engine.detail = True
    ceiling = bounds.object_ceiling(image)
    whole = bounds.WHOLE_REGISTER

    found, typed = {}, {}
    for klass, methods in sorted(bounds.receivers().items()):
        seen, floats = {}, {}
        for address, size, mangled in methods:
            start = address - image.code_start
            if start < 0 or start + size > len(image.code):
                continue
            carries = {"ecx": 0}
            for ins in engine.disasm(image.code[start:start + size], address):
                for operand in ([] if ins.id == X86_INS_LEA else ins.operands):
                    if operand.type != X86_OP_MEM or operand.mem.index != 0:
                        continue
                    base = whole.get(engine.reg_name(operand.mem.base) or "")
                    if base not in carries:
                        continue
                    offset = carries[base] + operand.mem.disp
                    width = operand.size or 4
                    if offset < 0 or (ceiling and offset + width > ceiling):
                        continue
                    seen.setdefault((offset, width),
                                    f"{mangled}: {ins.mnemonic} {ins.op_str}"
                                    f" at 0x{ins.address:08X}")
                    # THE X87 MNEMONIC IS A TYPE. Integer code cannot reach a
                    # member with `fld`/`fstp`, so a four-byte one is a float
                    # and an eight-byte one a double - the only member typing
                    # in this tree read straight off the instruction rather
                    # than inferred from a name or copied from a table.
                    if ins.mnemonic.startswith("f"):
                        floats.setdefault((offset, width),
                                          f"{mangled}: {ins.mnemonic} "
                                          f"{ins.op_str} at "
                                          f"0x{ins.address:08X}")
                gained = None
                operands = ins.operands
                if len(operands) == 2 and operands[0].type == X86_OP_REG:
                    destination = whole.get(engine.reg_name(operands[0].reg)
                                            or "")
                    if destination:
                        if (ins.id == X86_INS_MOV
                                and operands[1].type == X86_OP_REG):
                            source = whole.get(
                                engine.reg_name(operands[1].reg) or "")
                            if source in carries:
                                gained = (destination, carries[source])
                        elif (ins.id == X86_INS_LEA
                                and operands[1].type == X86_OP_MEM
                                and operands[1].mem.index == 0):
                            source = whole.get(
                                engine.reg_name(operands[1].mem.base) or "")
                            if source in carries:
                                gained = (destination,
                                          carries[source]
                                          + operands[1].mem.disp)
                for register in ins.regs_access()[1]:
                    carries.pop(whole.get(engine.reg_name(register) or ""),
                                None)
                if gained and gained[1] >= 0:
                    carries[gained[0]] = gained[1]
                if not carries:
                    break
        if seen:
            found[klass] = seen
        if floats:
            typed[klass] = floats
    return found, typed


# ---------------------------------------------------------------- findings ---

FLOAT_TYPES = {4: ("float", ("float",)), 8: ("double", ("double",))}


def findings(layouts: dict, sizes: dict, observed: dict,
             typed: dict = None) -> list:
    """[(kind, class, offset, width, detail, evidence)], worst kind first."""
    out = []
    for klass, touched in sorted(observed.items()):
        rows = layouts.get(klass)
        if rows is None:
            continue
        spans = sorted((offset, offset + size, name)
                       for name, offset, size, _ in rows if size)
        by_start = {offset: (name, spelling)
                    for name, offset, size, spelling in rows if size}
        total = sizes.get(klass, 0)
        for (offset, width), evidence in sorted(touched.items()):
            if total and offset >= total:
                out.append(("overrun", klass, offset, width,
                            f"sizeof is 0x{total:X}", evidence))
                continue
            for start, end, name in spans:
                if start <= offset < end:
                    if offset + width > end:
                        out.append(("straddle", klass, offset, width,
                                    f"{name} ends at 0x{end:X}", evidence))
                    break
            else:
                out.append(("unclaimed", klass, offset, width,
                            "no member covers it", evidence))
        # An x87 access names the member's TYPE. Only reported where a member
        # starts exactly there and is not already spelled floating - a float
        # read out of the middle of an array is the array's business.
        for (offset, width), evidence in sorted((typed or {})
                                                .get(klass, {}).items()):
            wanted = FLOAT_TYPES.get(width)
            if not wanted or offset not in by_start:
                continue
            name, spelled = by_start[offset]
            if spelled.strip() in wanted[1]:
                continue
            out.append(("typing", klass, offset, width,
                        f"{name} is spelled `{spelled.strip()}` and read "
                        f"as {wanted[0]}", evidence))
    order = {"overrun": 0, "straddle": 1, "typing": 2, "unclaimed": 3}
    return sorted(out, key=lambda row: (order[row[0]], row[1], row[2]))


def audit_pins(sizes: dict, observed: dict, src: Path = SRC) -> dict:
    """Which `static_assert(sizeof(X) == N)` the image itself corroborates.

    A pinned size is believed by everything downstream and, until this, was
    checked by nothing except its own compile. `derive_access_bounds --check`
    proves the weaker property that no bound EXCEEDS a pin; the interesting
    question is the other one - does the class's own code reach its last byte?

    Where it does, an independent source agrees with the pin exactly: the
    instruction stream, which cannot have been copied from whoever wrote the
    assertion. Where it stops short the pin is not refuted, only unconfirmed
    by this route, and saying which is which is the point. The verdicts:

      reached      an access ends exactly at `sizeof` - corroborated
      padded       it ends within 8 bytes, the tail alignment MSVC adds
      short        the class's own methods never reach the end
      silent       no thiscall method of this class touches `this` at all,
                   which is what an array-element type looks like: its size
                   comes from the stride of the arrays that hold it
    """
    pinned = {}
    for header in sorted(src.glob("*.h")):
        for found in re.finditer(
                r"static_assert\(\s*sizeof\((\w+)\)\s*==\s*"
                r"(0x[0-9A-Fa-f]+|\d+)", header.read_text(errors="ignore")):
            pinned[found.group(1)] = (int(found.group(2), 0), header.name)

    out = {}
    for klass, (size, header) in sorted(pinned.items()):
        reach = max((offset + width
                     for offset, width in observed.get(klass, {})), default=0)
        if not reach:
            verdict = "silent"
        elif reach > size:
            # Refuted, and `--check` has already failed on the overrun that
            # says so; repeated here because a pin is where a reader looks.
            # TESTED BEFORE `padded`, which is a difference of at most eight
            # bytes and read a refutation four bytes over as tail alignment.
            verdict = "REFUTED"
        elif reach == size:
            verdict = "reached"
        elif size - reach <= 8:
            verdict = "padded"
        else:
            verdict = "short"
        # `sizes` is what src/ actually compiles to. A pin the compiler does
        # not agree with cannot happen - the assertion would not build - but
        # asking costs nothing and a mismatch would mean the probe is reading
        # a different class than the assertion does.
        compiled = sizes.get(klass)
        out[klass] = (verdict, size, reach, header,
                      compiled if compiled != size else None)
    return out


def render(observed: dict, typed: dict) -> str:
    """The published view. `x87` marks the accesses that name a type."""
    import io

    buffer = io.StringIO()
    writer = csv.writer(buffer, lineterminator="\n")
    writer.writerow(["class", "offset", "width", "x87", "evidence"])
    for klass in sorted(observed):
        for (offset, width), evidence in sorted(observed[klass].items()):
            writer.writerow([klass, f"0x{offset:X}", width,
                             int((offset, width) in typed.get(klass, {})),
                             evidence])
    return buffer.getvalue()


def load_accesses(path: Path = ACCESSES_CSV) -> tuple:
    found, typed = collections.defaultdict(dict), collections.defaultdict(dict)
    if not path.is_file():
        return found, typed
    with path.open(newline="", encoding="utf-8-sig") as handle:
        for row in csv.DictReader(handle):
            key = (int(row["offset"], 16), int(row["width"]))
            found[row["class"]][key] = row["evidence"]
            if row.get("x87") == "1":
                typed[row["class"]][key] = row["evidence"]
    return found, typed


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path)
    parser.add_argument("--publish", action="store_true",
                        help="rewrite docs/recovery/member-accesses.csv")
    parser.add_argument("--check", action="store_true",
                        help="fail on any overrun or straddle")
    parser.add_argument("--class", dest="only", help="report one class")
    parser.add_argument("--pins", action="store_true",
                        help="which pinned sizes the image corroborates")
    parser.add_argument("--kind", choices=("overrun", "straddle", "typing",
                                          "unclaimed"))
    parser.add_argument("--src", type=Path, default=SRC)
    parser.add_argument("--accesses", type=Path, default=ACCESSES_CSV,
                        help="read the observed accesses from here instead of "
                             "deriving them (the offline and test route)")
    args = parser.parse_args(argv)

    import derive_class_layout as layout

    exe = args.exe or layout.DEFAULT_EXE
    if exe.is_file() and args.accesses == ACCESSES_CSV:
        observed, typed = accesses(exe)
        if args.publish:
            ACCESSES_CSV.write_text(render(observed, typed))
            print(f"wrote {ACCESSES_CSV.relative_to(REPO_ROOT)}: "
                  f"{sum(len(v) for v in observed.values())} accesses")
    else:
        observed, typed = load_accesses(args.accesses)
        print(f"reading observed accesses from {args.accesses}")

    import byte_match as bm

    reason = bm.available()
    if reason:
        print(f"cannot read declared offsets: {reason}")
        return 0

    layouts, sizes, refused = declared(args.src)
    for line in refused:
        print(f"  refused {line}")

    if args.pins:
        audit = audit_pins(sizes, observed, args.src)
        tally = collections.Counter(row[0] for row in audit.values())
        print(f"pinned sizes: {len(audit)}  " + "  ".join(
            f"{name} {count}" for name, count in sorted(tally.items())))
        for klass, (verdict, size, reach, header, mismatch) in audit.items():
            note = f"  COMPILES TO 0x{mismatch:X}" if mismatch else ""
            print(f"  {verdict:<8} {klass:<20} 0x{size:X}"
                  f"  image reaches 0x{reach:X}  ({header}){note}")
        return 1 if tally["REFUTED"] else 0

    rows = findings(layouts, sizes, observed, typed)
    # COUNTED BEFORE FILTERING. `--kind unclaimed` printing `overrun 0` because
    # the filter had already removed them reads as a clean tree.
    counted = collections.Counter(row[0] for row in rows)
    if args.only:
        rows = [row for row in rows if row[1] == args.only]
    if args.kind:
        rows = [row for row in rows if row[0] == args.kind]
    print(f"classes with declared members: {len(layouts)}; "
          f"with observed accesses: {len(observed)}")
    print(f"overrun {counted['overrun']}  straddle {counted['straddle']}  "
          f"typing {counted['typing']}  "
          f"unclaimed {counted['unclaimed']}")
    for kind, klass, offset, width, detail, evidence in rows:
        if kind == "unclaimed" and not args.kind:
            continue
        print(f"  {kind:<9} {klass}+0x{offset:X} w{width}: {detail}\n"
              f"            {evidence}")

    if args.check:
        bad = counted["overrun"] + counted["straddle"]
        if bad:
            print(f"FAIL: {bad} declared boundaries the image contradicts")
            return 1
        # A REFUSAL IS NOT A PASS. A header whose probe unit does not compile
        # contributes no declared layout, so every access into its classes
        # falls through every test and the run prints OK having measured
        # nothing. The damage case found this the first time it ran: a class
        # split into a byte and three pad bytes reported clean because its own
        # header had refused.
        if refused:
            print(f"FAIL: {len(refused)} header(s) could not be measured")
            return 1
        print("OK: no declared boundary is contradicted by the image")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
