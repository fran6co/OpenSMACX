#!/usr/bin/env python3
"""Name a member after the method that exists to touch it.

Every other naming source in this tree is a LOOKUP: the IDA database and the
Thinker mod each recorded a name and this reads it back. This one is an
INFERENCE, and its provenance is better for it - nothing here depends on
somebody else having been right, only on the image.

The rule: if exactly one member is reached through `this` in a method whose own
name means something, that method is about that member.

    ?is_playing@MCIVideo@@QAEHXZ         reads [ecx+0]        -> playing_
    ?get_ms_length@Wave@@QAEHXZ          reads [ecx+0x60]     -> ms_length_
    ?set_scaling@Caviar@@QAEXM@Z         writes [ecx+0xd5]    -> scaling_
    ?basewin_hide@GAmbience@@QAEXXZ      writes 0 to [ecx+0x6c] -> basewin_

OFFSET 0 IS EXCLUDED. Measured against members this tree had already named by
hand, the inference agrees 6 times out of 6 above offset 0 and 6 of 9 including
it - and all three failures are at offset 0, where `[ecx+0]` is the vtable
pointer, a vbtable pointer, or a base subobject rather than a field of this
class. `ListBox`, `Interlude` and `SelectPartWin` each name that slot for what
it is; the method reaching it is reaching through it.

TRIVIAL ACCESSORS BARELY EXIST HERE, which is worth saying because it is the
obvious thing to look for. MSVC6 inlines them: of 611 thiscall members under 24
bytes, SIX are a pure getter or setter. The yield comes from the broader rule
above, and it is still small - tens of members, not hundreds. It is offered
because a behaviour-derived name is the best-attested kind available, not
because it is the biggest.

The name is derived conservatively. `get_`/`set_`/`is_` come off, because those
say what the method does rather than what the member is; anything else keeps
the method's own name. No attempt is made to turn `close` into `open_` or
`clear_lines` into `line_count_` - those readings are right and they are
reasoning about intent, which is not what this tool is for.
"""

from __future__ import annotations

import argparse
import collections
import csv
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from capstone import CS_ARCH_X86, CS_MODE_32, Cs  # noqa: E402
from capstone.x86 import X86_OP_MEM, X86_REG_ECX  # noqa: E402

import derive_access_bounds  # noqa: E402
import derive_class_layout  # noqa: E402


def _catalogue_rows():
    """Every catalogued row, from `src/`.

    `docs/recovery/functions.csv` is deleted: every `ORIGINAL:` annotation
    carries its own name, size, spans, prototype, kind, flags and call
    edges, and `emit.load_functions()` reads them back. This tool opened
    the CSV directly, so it broke the moment the store moved - which is
    how five layout gates went red at once.
    """
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parent))
    import emit_translation_unit as _emit
    return list(_emit.load_functions().values())

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
FUNCTIONS = REPO_ROOT / "docs" / "recovery" / "functions.csv"
OUTPUT = REPO_ROOT / "docs" / "recovery" / "behaviour-member-names.csv"

# `?name@Class@@...` - the method's own identifier.
METHOD_RE = re.compile(r"^\?(?P<name>[\w_]+)@[\w_]+@@")
# ONLY THE ACCESSOR FORMS. A first attempt accepted any informatively-named
# method that touched exactly one member, and it produced 58 rows of which most
# were wrong in the same way:
#
#     BaseButton::on_right_click  -> on_right_click_   at 0xA80
#     Console::clear_group        -> clear_group_      at 0x23D1C
#     BaseWin::delete_queue       -> delete_queue_     at 0x40B2C
#
# Those are verbs. An event handler is named for the EVENT and a command for
# what it DOES; that a handler reads one member says the member is involved,
# not that it is called `on_right_click`. `get_ms_length` is different in kind:
# the method exists to expose the member, so the member is the method's name
# with the accessor prefix taken off.
ACCESSOR_RE = re.compile(r"^(?:get|set|is|has)_(?P<subject>\w+)$")
UNINFORMATIVE = re.compile(r"^(UNK\w*|unk\w*|sub_\w+|nullsub\w*|\d+)$")
# Bodies longer than this stop being "a method about one member".
MAX_BODY = 200


def method_name(mangled: str) -> str:
    found = METHOD_RE.match(mangled or "")
    return found.group("name") if found else ""


def member_name(method: str) -> str:
    """The member an accessor's name implies, or '' when it implies none."""
    found = ACCESSOR_RE.match(method or "")
    if not found:
        return ""
    subject = found.group("subject")
    if UNINFORMATIVE.match(subject):
        return ""
    return f"{subject}_"


def touched(image, engine, address: int, size: int) -> set:
    """Every `[ecx+N]` offset this body reaches before ECX stops being `this`.

    Deliberately NOT the register-following analysis in
    `derive_access_bounds.reach` - that one is looking for the furthest byte
    and is right to chase `this` through copies. Here a single unambiguous
    member is the whole point, so the moment the receiver leaves ECX the
    evidence stops being about one member.
    """
    start = address - image.code_start
    if start < 0 or start + size > len(image.code):
        return set()
    offsets = set()
    for instruction in engine.disasm(image.code[start:start + size], address):
        written = {engine.reg_name(register)
                   for register in instruction.regs_access()[1]}
        follows_pointer = bool(written & {"ecx", "cx", "cl", "ch"})
        for operand in instruction.operands:
            if (operand.type != X86_OP_MEM
                    or operand.mem.base != X86_REG_ECX
                    or operand.mem.index != 0
                    or operand.mem.disp <= 0):     # offset 0 is the vtable
                continue
            # THE DELEGATING ACCESSOR. `mov ecx, [ecx+0x14]` loads a POINTER
            # out of the member and dispatches through it, so the accessor's
            # subject belongs to the object at the far end and the member it
            # touched is the pointer. Measured on the two cases this rule was
            # about to name wrongly:
            #
            #   ?is_disabled@Midi_Device@@QAEHXZ   mov ecx,[ecx+0x14]; jmp [eax+0x5c]
            #   ?get_buffer_size@VoiceRx@@QAEHXZ   mov ecx,[ecx+0x3c]; jmp [eax+0x90]
            #
            # 0x14 and 0x3C are the held device, not `disabled_` or
            # `buffer_size_`.
            if follows_pointer:
                continue
            offsets.add((operand.mem.disp, operand.size or 4))
        if follows_pointer:
            break
    return offsets


def infer(exe: Path) -> list:
    """[(class, offset, name, size, method)] - one row per inference."""
    image = derive_class_layout.Image(exe)
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    engine.detail = True

    proposals = collections.defaultdict(list)
    # `src/` is the catalogue's store; the export is deleted.
    for row in _catalogue_rows():
        mangled = row.get("name") or ""
        klass = derive_access_bounds.receiver_scope(mangled)
        if not klass:
            continue
        name = member_name(method_name(mangled))
        if not name:
            continue
        try:
            address, size = int(row["address"], 16), int(row["size"])
        except (ValueError, KeyError, TypeError):
            continue
        if not 0 < size <= MAX_BODY:
            continue
        offsets = touched(image, engine, address, size)
        if len(offsets) != 1:
            # Two members is not evidence about either; zero is silence.
            continue
        offset, width = offsets.pop()
        proposals[(klass, offset)].append((name, width, mangled))

    rows = []
    for (klass, offset), found in sorted(proposals.items()):
        names = {name for name, _, _ in found}
        if len(names) != 1:
            # Two differently-named methods claiming the same member. Both may
            # be right about the offset and they cannot both name it, so
            # neither does.
            continue
        name, width, mangled = found[0]
        rows.append((klass, offset, name, width, mangled))
    return rows


def render(rows: list) -> str:
    import io

    buffer = io.StringIO()
    writer = csv.writer(buffer, lineterminator="\n")
    writer.writerow(["class", "offset", "name", "size", "evidence"])
    for klass, offset, name, width, mangled in sorted(rows):
        writer.writerow([klass, f"0x{offset:X}", name, width,
                         f"the only member {mangled} touches"])
    return buffer.getvalue()


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--out", type=Path, default=OUTPUT)
    parser.add_argument("--check", action="store_true",
                        help="fail if the committed CSV is stale")
    args = parser.parse_args(argv)

    if not args.exe.is_file():
        print(f"SKIP: {args.exe} is absent. Every name here is read out of "
              f"the image.")
        return 0

    rows = infer(args.exe)
    text = render(rows)

    if args.check:
        current = args.out.read_text() if args.out.is_file() else ""
        if current != text:
            print(f"{args.out.name} is stale; regenerate it", file=sys.stderr)
            return 1
        print(f"behaviour-derived names: {len(rows)} member(s) (up to date)")
        return 0

    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(text)
    print(f"{len(rows)} member(s) named by the method that touches them "
          f"-> {args.out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
