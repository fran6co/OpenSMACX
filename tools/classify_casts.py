#!/usr/bin/env python3
"""Every `this`-relative cast in `src/`, and the header defect it implies.

A cast through `this` is not a style problem. It is a place where a recovered
body knew something about the object that the class declaration does not, and
had to spell it as an address instead of a name:

    ((SubInterface *)((char *)this + 0xA14))->release_iface_mode();

against what a declaration buys:

    sub_interface_.release_iface_mode();

Both compile to `lea ecx,[esi+0xa14]; call ...`, so the comparison cannot tell
them apart - which is what makes preferring the second safe rather than a
judgement call. This tool does not make that edit. It reads each cast against
the layout its class actually declares and says which of a small number of
things is wrong, so the edits can be worked in order of evidence.

WHAT IT REFUSES TO DO. It proposes nothing and edits nothing. A row is a
measurement: this cast, at this offset, against this layout, resolves to this.
Turning a row into a header change is a separate step with the ratchet as its
gate, because adding a DATA MEMBER moves every offset after it.

A REFUSAL IS A ROW, NEVER A DROPPED ROW. The interesting number here is how
much of the tree cannot be answered for, and a classifier that quietly emits
only what it understood reports a clean sheet by discarding the evidence.
"""

from __future__ import annotations

import argparse
import collections
import csv
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import class_layouts  # noqa: E402
import emit_hypothesis_layouts as hypothesis  # noqa: E402
import repair_source_locations as repair  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC = REPO_ROOT / "src"
FUNCTIONS = REPO_ROOT / "docs" / "recovery" / "functions.csv"
LEDGER = REPO_ROOT / ".opensmacx" / "byte-match.csv"
REPORT = REPO_ROOT / "docs" / "recovery" / "cast-classification.csv"

# Files whose casts are structural and answer to a generator, not to a class
# declaration: the redirect table, the thunks, the field accessors. Their
# casts are counted in the summary and never classified, because "the header
# is wrong" is not a statement about any of them.
GENERATED = re.compile(
    r"adjustor_thunks|delegation_thunks|atexit_thunks|init_thunks"
    r"|field_accessors|global_arith|deleting_thunks|nullsub_thunks"
    r"|guarded_teardowns|generated_signature_oracle|dllmain"
    r"|hypothesis_layouts")

FIELDS = ("file", "line", "address", "class", "tier", "operand", "offset",
          "width", "resolution", "member", "member_offset", "member_size",
          "commented_offset", "receiver_is_subobject", "confidence",
          "evidence")

# `uint8_t *const self = reinterpret_cast<uint8_t *>(this);` - the alias form.
# Offsets then appear on `self`, not on `this`, and 115 of the casts in this
# tree are written that way.
ALIAS = re.compile(
    r"^\s*(?:const\s+)?[\w:]+\s*\*+\s*(?:const\s+)?(?P<name>\w+)\s*=\s*"
    r"reinterpret_cast<[^>]*>\s*\(\s*this\s*\)\s*;")
CAST_HEAD = re.compile(r"reinterpret_cast<(?P<type>[^>]*)>\s*\(")
# `*reinterpret_cast<uint8_t **>(this)` - loading the vtable pointer at 0.
VTABLE_LOAD = re.compile(r"\*\s*reinterpret_cast<[^>]*\*\*>\s*\(\s*this\s*\)")
FIELD_NAME_OFFSET = re.compile(r"^field_([0-9A-Fa-f]+)_$")
COMMENTED_OFFSET = re.compile(r"//\s*(0x[0-9A-Fa-f]+)")


def balanced(text: str, open_at: int) -> int:
    """Index just past the `)` matching the `(` at `open_at`, or -1."""
    depth = 0
    for index in range(open_at, len(text)):
        if text[index] == "(":
            depth += 1
        elif text[index] == ")":
            depth -= 1
            if not depth:
                return index + 1
    return -1


def casts_in(text: str):
    """(line, cast type, operand, trailing) for each reinterpret_cast.

    The operand is read with a BALANCED scan rather than a regex. A regex for
    `\\(([^)]*)\\)` stops at the first `)`, and this tree's casts routinely
    contain a call - `reinterpret_cast<T *>(base(x) + 4)` - so it would report
    an operand of `base(x` and an offset of nothing.
    """
    for head in CAST_HEAD.finditer(text):
        end = balanced(text, head.end() - 1)
        if end < 0:
            continue
        before = text[:head.start()].rstrip()
        yield (text[:head.start()].count("\n") + 1,
               head.group("type").strip(),
               text[head.end():end - 1].strip(),
               text[end:end + 40],
               before.endswith("*"))


def aliases_of(body: str) -> set:
    """Names bound once to `this` in this body, so `name + N` is `this + N`.

    Single assignment only. A name that is reassigned, or whose address is
    taken, is dropped: it is no longer certain that an offset from it is an
    offset from `this`, and a wrong attribution here is worse than none.
    """
    found = collections.Counter()
    for line in body.splitlines():
        hit = ALIAS.match(line)
        if hit:
            found[hit.group("name")] += 1
    return {name for name, count in found.items()
            if count == 1 and f"&{name}" not in body}


# `this`, `self`, `self + 0xa14`, `bytes + 0x928 + (index - 1) * 0x19C`. The
# base and the FIRST constant displacement; a variable term after it means the
# constant is the base of an indexed run, which is recorded, not discarded.
OPERAND = re.compile(
    r"^\s*(?P<base>this|[A-Za-z_]\w*)\s*"
    r"(?:(?P<sign>[-+])\s*(?P<offset>0x[0-9A-Fa-f]+|\d+))?"
    r"(?P<rest>.*)$", re.S)


def read_operand(operand: str, trailing: str):
    """(base, offset, indexed) for a cast's operand.

    THE OFFSET IS USUALLY INSIDE THE OPERAND, not after the cast. This tree
    writes `reinterpret_cast<MCIVideo *>(self + 0xa14)`, and matching only a
    bare `this`/`self` classified 124 of 237 casts as having no displacement -
    which read as "unresolvable" when the offset was sitting right there. It
    also appears after the closing paren, as
    `reinterpret_cast<char *>(this) + 0xD4`, so both are tried.
    """
    hit = OPERAND.match(operand or "")
    if not hit:
        return None, None, False
    base = hit.group("base")
    offset = None
    if hit.group("offset") is not None:
        offset = int(hit.group("offset"), 0)
        if hit.group("sign") == "-":
            offset = -offset
    indexed = bool((hit.group("rest") or "").strip())
    if offset is None:
        after = re.match(r"\s*([-+])\s*(0x[0-9A-Fa-f]+|\d+)", trailing or "")
        if after:
            offset = int(after.group(2), 0)
            if after.group(1) == "-":
                offset = -offset
    return base, offset, indexed


def catalogued() -> dict:
    """{address: row} from functions.csv, keyed as an int."""
    rows = {}
    with FUNCTIONS.open(newline="", encoding="utf-8-sig") as handle:
        for row in csv.DictReader(handle):
            try:
                rows[int(row["address"], 16)] = row
            except (KeyError, ValueError):
                continue
    return rows


def tiers() -> dict:
    """{address: tier} from the byte-match ledger."""
    if not LEDGER.is_file():
        return {}
    with LEDGER.open(newline="", encoding="utf-8-sig") as handle:
        return {int(row["address"], 16): row.get("tier", "")
                for row in csv.DictReader(handle)
                if row.get("address", "").startswith("0x")}


def anchors(text: str) -> list:
    """[(line, address)] for every `Original Offset:` in a file, in order.

    THE ANCHOR IS THE TREE'S OWN, not a C++ parse. Scanning backwards for
    `Class::method(` attributes a cast to a class called `std`, because
    `    std::memcpy(reinterpret_cast<uint8_t *>(this) + 0x40100, ...)` at the
    start of a line matches any `Ident::ident(` pattern. Every recovered body
    carries an `Original Offset:` line, integrate_recovery requires one, and
    repair_source_locations keeps them true.
    """
    found = []
    for number, line in enumerate(text.splitlines(), start=1):
        hit = repair.OFFSET_LINE.match(line)
        if hit:
            found.append((number, int(hit.group(1), 16)))
    return found


def owning_class(mangled: str):
    """The class a mangled name scopes to, or None for a free function."""
    hit = hypothesis.SCOPE_RE.match(mangled or "")
    return hit.group(1) if hit else None


def sizes() -> dict:
    """{type: bytes} for what a member of that type occupies.

    Deliberately small and deliberately explicit. Anything absent makes the
    class unresolvable rather than being guessed at - a wrong width does not
    fail, it silently moves every offset after it, which is the one outcome
    this whole file exists to catch.
    """
    table = {"char": 1, "signed char": 1, "unsigned char": 1, "bool": 1,
             "int8_t": 1, "uint8_t": 1,
             "short": 2, "unsigned short": 2, "int16_t": 2, "uint16_t": 2,
             "int": 4, "unsigned": 4, "unsigned int": 4, "long": 4,
             "unsigned long": 4, "float": 4, "int32_t": 4, "uint32_t": 4,
             "size_t": 4, "intptr_t": 4, "uintptr_t": 4, "wchar_t": 2,
             "double": 8, "int64_t": 8, "uint64_t": 8}
    for name in class_layouts.WINDOWS_TYPEDEF:
        table.setdefault(name, 4)
    table["RECT"] = 16
    table["POINT"] = 8
    table["SIZE"] = 8
    return table


SIZES = sizes()


def array_count(array: str) -> int:
    """`[256]` -> 256, `[2][3]` -> 6, `[]` or an expression -> 0."""
    total = 1
    found = re.findall(r"\[([^\]]*)\]", array or "")
    if not found:
        return 1
    for bound in found:
        bound = bound.strip()
        if not re.fullmatch(r"0x[0-9A-Fa-f]+|\d+", bound):
            return 0
        total *= int(bound, 0)
    return total


def width_of(type_: str, name: str, array: str, depth=0):
    """Bytes a single member occupies, or None when it cannot be known."""
    bare = type_.replace("const", "").strip()
    count = array_count(array)
    if not count:
        return None
    if bare.endswith("*"):
        return 4 * count
    if bare in SIZES:
        return SIZES[bare] * count
    if depth > 4:
        return None
    nested = class_layouts._with_bases(bare)
    if nested is None:
        return None
    # A NESTED STRUCT IS LAID OUT, NOT SUMMED. Adding the raw member sizes
    # gave `Heap` 17 bytes against its own `static_assert(sizeof(Heap) ==
    # 0x14)`: `int8_t` then four 4-byte members is 1+4+4+4+4, and the real
    # object pads the int8_t to 4 and ends on a multiple of its alignment.
    #
    # The offsets AFTER such a member survived by luck - 17 rounds up to 20
    # under 4-byte alignment - which is why 1,113 name-encoded offsets all
    # agreed while this was wrong. What did not survive was the member's own
    # extent, and the image reading a dword at Heap+16 looked like an access
    # STRADDLING the end of it.
    total, alignment = 0, 1
    for member_type, member_name, member_array in nested:
        one = width_of(member_type, member_name, member_array, depth + 1)
        if one is None:
            return None
        member_alignment = alignment_of(member_type, member_array, depth + 1)
        alignment = max(alignment, member_alignment)
        total = align_to(total, member_alignment) + one
    return align_to(total, alignment) * count


def align_to(offset: int, alignment: int) -> int:
    """The next offset at or after `offset` that `alignment` divides."""
    if alignment < 2:
        return offset
    remainder = offset % alignment
    return offset if not remainder else offset + alignment - remainder


def alignment_of(type_: str, array: str, depth=0) -> int:
    """A member's alignment: its own size for a scalar, its widest member for
    a class, an array's element's, and never more than 8 under /Zp8."""
    bare = type_.replace("const", "").strip()
    if bare.endswith("*"):
        return 4
    # A STRUCT ALIGNS TO ITS WIDEST MEMBER, NOT TO ITS SIZE. Reading the size
    # here gave RECT - sixteen bytes of four `long` - an alignment of 8, which
    # pushed Win::outer_rect_ from 0x13C to 0x140 and every member after it
    # with it. Thirty of Win's members encode their own offset in their name
    # and all thirty disagreed by exactly four; that is what found this.
    if bare in class_layouts.WINDOWS_STRUCT:
        return min(8, max(alignment_of(member_type, member_array, depth + 1)
                          for member_type, _, member_array
                          in class_layouts.WINDOWS_STRUCT[bare]))
    if bare in SIZES:
        return min(8, SIZES[bare])
    if depth > 4:
        return 4
    nested = class_layouts._with_bases(bare)
    if not nested:
        return 4
    return min(8, max((alignment_of(t, a, depth + 1)
                       for t, _, a in nested), default=1))


def layout_offsets(name: str):
    """[(offset, member, type, size)] for a class, or None.

    Python accumulates and refuses on anything it cannot total. It does NOT
    model alignment or padding, so an offset here is a HYPOTHESIS - which is
    why every row carries its confidence and why nothing acts on one without
    the compiler agreeing.
    """
    members = class_layouts._with_bases(name)
    if members is None:
        return None
    out, at = [], 0
    for type_, member, array in members:
        size = width_of(type_, member, array)
        if size is None:
            return None
        # ALIGNMENT, which the first version of this did not model and which
        # is not a rounding detail: without it `Dialog::field_BC_` computed to
        # 0xB9, three bytes low, and every member after it inherited the
        # error. A cast at 0xBC then resolved as "inside field_BC_" instead of
        # "exact", which is a different finding about a different defect.
        #
        # The rule is the C one under /Zp8: a member starts on a multiple of
        # its own alignment, an array aligns like its element, and nothing
        # aligns beyond 8.
        out.append((align_to(at, alignment_of(type_, array)), member, type_,
                    size))
        at = out[-1][0] + size
    return out


def commented_offsets(name: str) -> dict:
    """{member: offset} from the `// 0xNNN` comments and `field_<hex>_` names.

    A THIRD WITNESS, NEVER AN INPUT. class_layouts.statements() strips these
    deliberately, and trusting one once glued StringList into a single member.
    They are read here only so a disagreement with the computed offset can be
    reported as a finding of its own.
    """
    found = {}
    for header in SRC.glob("*.h"):
        text = header.read_text(errors="ignore")
        for class_name, _, body in class_layouts.class_bodies(text):
            if class_name != name:
                continue
            for line in body.splitlines():
                member = class_layouts.MEMBER.match(
                    re.sub(r"^\s*(?:public|private|protected)\s*:\s*", "",
                           line.strip()))
                if not member:
                    continue
                comment = COMMENTED_OFFSET.search(line)
                encoded = FIELD_NAME_OFFSET.match(member.group("name"))
                if comment:
                    found[member.group("name")] = int(comment.group(1), 16)
                elif encoded:
                    found[member.group("name")] = int(encoded.group(1), 16)
    return found


def resolve(offset, layout):
    """(resolution, member, member_offset, member_size) for one offset."""
    if layout is None:
        return "layout-unreadable", "", "", ""
    total = sum(size for _, _, _, size in layout)
    if offset >= total:
        return "past-declared-layout", "", "", ""
    for at, member, type_, size in layout:
        if offset == at:
            return "exact-member", member, at, size
        if at < offset < at + size:
            return "inside-declared-member", member, at, size
    return "unresolvable", "", "", ""


def classify() -> list:
    """One row per `this`-relative cast in hand-written `src/`."""
    functions, tier_of = catalogued(), tiers()
    rows, layouts, comments = [], {}, {}

    for path in sorted(SRC.glob("*.cpp")):
        if GENERATED.search(path.name):
            continue
        text = path.read_text(errors="ignore")
        marks = anchors(text)
        alias = aliases_of(text)
        # A body that reaches BACKWARDS from `this` is standing on a
        # sub-object, so `this` is not the declared class and no offset in it
        # resolves against that class's layout. Detected per body, and it
        # disqualifies every cast in that body rather than just the negative
        # one - src/basewin.cpp is the case: `-0xA14` is container_of and
        # `+0x40100` in the same function is interface-relative.
        subobject_bodies = set()
        for line, cast_type, operand, trailing, _deref in casts_in(text):
            base, offset, _ = read_operand(operand, trailing)
            if base in ({"this"} | alias) and (offset or 0) < 0:
                anchored = [a for n, a in marks if n <= line]
                if anchored:
                    subobject_bodies.add(anchored[-1])

        for line, cast_type, operand, trailing, deref in casts_in(text):
            base, offset, indexed = read_operand(operand, trailing)
            if base not in ({"this"} | alias):
                continue
            bare_operand = base
            anchored = [a for n, a in marks if n <= line]
            address = anchored[-1] if anchored else None
            row = {field: "" for field in FIELDS}
            row.update(file=f"src/{path.name}", line=line,
                       operand=bare_operand, width=cast_type)

            if address is None:
                row.update(resolution="outside-any-catalogued-body",
                           confidence="derived",
                           evidence="no `Original Offset:` precedes this line")
                rows.append(row)
                continue

            row["address"] = f"0x{address:08X}"
            row["tier"] = tier_of.get(address, "")
            name = (functions.get(address) or {}).get("name", "")
            klass = owning_class(name)
            row["offset"] = "" if offset is None else f"{offset:#x}"

            if deref and "**" in cast_type and base == "this":
                # `*reinterpret_cast<uint8_t **>(this) + 0x80` reads the
                # VTABLE POINTER and then indexes the VTABLE. The 0x80 is a
                # slot, not a member offset, and resolving it against the
                # class layout reported six phantom "missing member at
                # Wave+0x80" findings before this told them apart.
                row.update(
                    resolution="vtable-slot" if offset else "vtable-load",
                    confidence="proved",
                    evidence=("indexes the vtable, not the object; the offset "
                              "is a slot" if offset else
                              "loads the vtable pointer at offset 0; the "
                              "original installs its own vtables by hand"))
                rows.append(row)
                continue

            if klass is None:
                row.update(resolution="unresolvable", confidence="derived",
                           evidence=f"{name or 'no catalogued name'} scopes to "
                                    f"no class")
                rows.append(row)
                continue
            row["class"] = klass

            if address in subobject_bodies:
                row.update(resolution="receiver-is-subobject",
                           receiver_is_subobject="1", confidence="proved",
                           evidence="a negative offset in this body proves "
                                    "`this` is not the declared class")
                rows.append(row)
                continue

            if offset is None:
                binding = ALIAS.match(
                    text.splitlines()[line - 1] if line <= len(
                        text.splitlines()) else "")
                if binding or base == "this":
                    row.update(
                        resolution="alias-binding" if binding
                        else "whole-object-cast",
                        confidence="proved",
                        evidence=("binds the alias later offsets are measured "
                                  "from; carries no displacement itself")
                        if binding else
                        ("reinterprets the whole object, no displacement - a "
                         "base or sub-object view, not a member access"))
                else:
                    row.update(resolution="unresolvable", confidence="derived",
                               evidence="alias used with no constant "
                                        "displacement")
                rows.append(row)
                continue
            if offset < 0:
                row.update(resolution="negative-offset",
                           receiver_is_subobject="1", confidence="proved",
                           evidence="container_of; C++ has no member before "
                                    "`this` and no declaration expresses one")
                rows.append(row)
                continue

            if klass not in layouts:
                layouts[klass] = layout_offsets(klass)
                comments[klass] = commented_offsets(klass)
            layout = layouts[klass]
            resolution, member, at, size = resolve(offset, layout)
            if indexed and resolution == "exact-member":
                resolution = "exact-member"   # base of an indexed run
            row.update(resolution=resolution, member=member,
                       member_offset="" if at == "" else f"{at:#x}",
                       member_size=size)
            commented = comments[klass].get(member)
            if commented is not None:
                row["commented_offset"] = f"{commented:#x}"
                if at != "" and commented != at:
                    row["evidence"] = (
                        f"the comment says {commented:#x}, the accumulated "
                        f"layout says {at:#x} - one of them is wrong")
            if resolution == "layout-unreadable":
                row["confidence"] = "hypothesis"
                row["evidence"] = ("class_layouts cannot total this class, so "
                                   "no offset in it can be resolved")
            elif resolution == "exact-member":
                row["confidence"] = ("proved" if row["tier"] == "BYTE_EXACT"
                                     else "derived")
                row["resolution"] = "exact-member-typeable"
            else:
                row["confidence"] = "derived"
            rows.append(row)
    return rows


def render(rows: list) -> str:
    from io import StringIO
    out = StringIO()
    writer = csv.DictWriter(out, fieldnames=FIELDS, lineterminator="\n")
    writer.writeheader()
    for row in sorted(rows, key=lambda r: (r["file"], int(r["line"]))):
        writer.writerow(row)
    return out.getvalue()


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true",
                        help="fail if the committed classification is stale")
    parser.add_argument("--report", type=Path, default=REPORT)
    arguments = parser.parse_args(argv)

    rows = classify()
    text = render(rows)
    if arguments.check:
        current = (arguments.report.read_text()
                   if arguments.report.is_file() else "")
        if current != text:
            print("cast-classification.csv is stale; regenerate it",
                  file=sys.stderr)
            return 1
        print(f"cast classification: {len(rows)} cast(s) (up to date)")
        return 0

    arguments.report.parent.mkdir(parents=True, exist_ok=True)
    arguments.report.write_text(text)
    counts = collections.Counter(row["resolution"] for row in rows)
    print(f"{len(rows)} this-relative cast(s) -> {arguments.report}")
    for resolution, count in counts.most_common():
        print(f"  {count:4}  {resolution}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
