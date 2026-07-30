#!/usr/bin/env python3
"""Generate recoveries for the two MSVC object-deletion thunk families.

Family A, the receiver-adjusting tail jump: two instructions, a fixed or
vtordisp-driven subtraction on `this` followed by a tail call that leaves the
arguments alone.

    sub ecx, A          ; or `sub ecx, [ecx-4]` for the vtordisp form
    jmp <target>

Family B, the compiler-generated scalar deleting destructor: run the complete
destructor, then hand the storage to the executable's `operator delete` only
when bit 0 of the incoming flags word asks, and always return the object.

    push esi / mov esi, ecx     ; with or without an ebp frame
    call <destructor>
    test byte ptr [ebp+8], 1    ; `[esp+8]` in the frameless form
    je over
    push esi / call <deleter> / add esp, 4
    over: mov eax, esi / pops / ret 4

The two families are generated together because they interlock: most Family A
thunks tail jump straight into a Family B body, so emitting them apart would
mean seaming a fixed address onto a function this same run recovers.

Nothing here is transcribed by hand, and three things are proven per row
rather than assumed.

Arity first, because it is the one wrong guess that silently corrupts a
caller's stack. Family B derives it from its own bytes: the body's `ret 4`
pops one slot and its guard tests `[ebp+8]` / `[esp+8]`, which
`delegates.parse_stack_slot` maps to parameter 0 - so the single stack slot is
the flags word, and that is checked, not assumed. Family A pops nothing at
all, so its whole stack contract belongs to the jump target and comes from
`callee_pop`; a thunk whose target pop cannot be determined is skipped and
named rather than guessed at.

The mangled names are no help and are actively misleading. 47 Family A thunks
are spelled `??3X@@SAXPAXI@Z`, which demangles as a caller-cleaned
`static void __cdecl X::operator delete(void *, unsigned int)`. The bytes say
otherwise: the receiver arrives in `ecx` and the callee pops four. Seven more
carry no name at all (`sub_60ce40` and friends), which is why a name-driven
scan misses exactly the rows a byte-driven one finds. Emitted symbols
therefore describe what the bytes do and never claim to be `operator delete`.

Second, every value is decoded twice. `decode_raw_adjust` and
`decode_raw_scalar_delete` read the ModRM, displacement and immediate bytes
directly; the capstone matchers read the printed mnemonics; the two builders
refuse to emit anything unless the readings agree. That is what keeps a
text-rendering quirk out of committed source - an off-by-one on the guarded
call would name 0x0064557E, the tail of `__alloca_probe`, instead of
`operator delete`.

Third, a forward goes through a fixed-address seam only when there is nothing
source-owned to call. A Family A target that this run emits as Family B is
called directly, and a destructor already recovered elsewhere is reached
through the binding the tree already has rather than through a second name for
the same address.

Emitted, deterministically ordered by address:
  <source-dir>/deleting_thunks.h / .cpp     the committed source pair
  <scratch>/deleting-thunk-tests.cpp        fragment for the leaf suite
  <scratch>/deleting-wire.txt               address/symbol redirect list

`--source-dir` defaults to the scratch directory so a run stages everything
outside the tree; point it at `src/` when the batch is ready to land.

Like the other bulk generators this compresses authoring, not verification:
the emitted bodies still go through the leaf suite, the mutation sweep and the
batch gate exactly as hand-written recoveries do. Each body keeps its own
`Original Offset:` comment and spells its adjustment and its flag mask as
inline literals for that reason - hoisting them into a shared table would read
as tidier and would leave the sweep with nothing in the body to perturb.

Reads proprietary bytes: local analysis only, never committed.
"""

from __future__ import annotations

import argparse
import re
import sys
import textwrap
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pefile  # noqa: E402

import analyze_delegates as delegates  # noqa: E402
import disasm  # noqa: E402
from generate_adjustor_thunks import (LICENSE, callee_pop,  # noqa: E402
                                      camel, declaration, entry_extent, snake)

# The file the emitted bodies land in. A row that is already source-complete
# somewhere ELSE belongs to another recovery and must not be emitted a second
# time; a row already recorded against this file is one an earlier run of this
# generator produced and must keep being emitted, or rerunning after the batch
# lands would silently drop it from the committed source.
GENERATED_SOURCE = "src/deleting_thunks.cpp"

# The executable's `??3@YAXPAX@Z` at 0x0064557F, which every Family B body
# hands the storage to. The tree already binds this address five times over;
# scroll.h's spelling is reused rather than adding a sixth name for it,
# and scroll.h is included here anyway for the recovered Scroll body below.
DELETER_ADDRESS = 0x0064557F
DELETER_NAME = "??3@YAXPAX@Z"
DELETER_SYMBOL = "ScrollOperatorDelete"
DELETER_INCLUDE = "scroll.h"

# Destructors a Family B body must reach through something already in the
# tree instead of a new seam: six recovered bodies, two classified original
# dependencies that already have a bound pointer, and one pure jump thunk.
# The address is part of the table so a catalogue drift is caught rather than
# absorbed - resolve_destructor requires it to equal the address decoded
# from the bytes. `{0}` is the receiver.
HANDLED_DESTRUCTORS = {
    "??1FlatButton@@QAE@XZ": (
        0x00406880, "flatbutton.h",
        "flat_button_destructor_redirect(\n"
        "        reinterpret_cast<FlatButton *>({0}), nullptr)"),
    "??1BaseButton@@QAE@XZ": (
        0x00607040, "basebutton.h",
        "base_button_destructor_redirect(\n"
        "        reinterpret_cast<BaseButton *>({0}), nullptr)"),
    "??1GraphicWin@@QAE@XZ": (
        0x005D4DD0, "graphicwin.h",
        "graphic_win_destructor_redirect(\n"
        "        reinterpret_cast<GraphicWin *>({0}), nullptr)"),
    "??1Buffer@@QAE@XZ": (
        0x005D7410, "buffer.h",
        "buffer_destructor_redirect(reinterpret_cast<Buffer *>({0}),\n"
        "                               nullptr)"),
    "??1PullDown@@QAE@XZ": (
        0x005F88A0, "pulldown.h",
        "pull_down_destructor_redirect(\n"
        "        reinterpret_cast<PullDown *>({0}), nullptr)"),
    "??1Ambience@@QAE@XZ": (
        0x004C7670, "ambience.h",
        "ambience_dtor_redirect(reinterpret_cast<Ambience *>({0}),\n"
        "                           nullptr)"),
    # The jump thunk at 0x004483C0 is five bytes of `jmp ??1Ambience`, so
    # calling the destructor is the same call with one fewer hop - the same
    # spelling tools/generate_atexit_thunks.py already uses for it.
    "j_??1Ambience@@QAE@XZ": (
        0x004483C0, "ambience.h",
        "ambience_dtor_redirect(reinterpret_cast<Ambience *>({0}),\n"
        "                           nullptr)"),
    # Two classified original dependencies. Neither body is source-owned yet,
    # but both already have a rebindable pointer at the same address, so
    # reusing them keeps one address to one name.
    "??1Win@@QAE@XZ": (0x005EBC90, "graphicwin.h",
                       "WinOriginalDestructor({0})"),
    "??1Effect@@QAE@XZ": (0x004482C0, "fx.h", "EffectElementTeardown({0})"),
}

# Family A jump targets that are already source-owned outside this run. The
# pop count is part of the table for the same reason the address is: it is
# cross-checked against `callee_pop` rather than trusted. `{0}` is the
# adjusted receiver, `{1}` the forwarded argument.
HANDLED_FORWARDS = {
    "??_GScroll@@UAEPAXI@Z": (
        0x00406F20, 4, "scroll.h",
        "scroll_scalar_dtor_redirect(\n"
        "        reinterpret_cast<Scroll *>({0}), nullptr, {1})"),
    # Not a recovery: adjustor_thunks.h already binds a seam to this exact
    # address under this exact name, because the vtordisp thunk family reaches
    # it too. Defining a second pointer here links as a duplicate symbol, and
    # the rule is one address to one name - so the existing one is reused.
    "??_GPlanWin@@UAEPAXI@Z": (
        0x0048BF20, 4, "adjustor_thunks.h",
        "PlanWinScalarDeleteTarget({0}, {1})"),
}

# Both families are found by shape, so only these four entry extents are ever
# read. Anything else is not one of the templates by construction.
TEMPLATE_EXTENTS = {8, 11, 30, 33}

SUB_VTORDISP = re.compile(r"^ecx, dword ptr \[ecx - 4\]$")
SUB_IMMEDIATE = re.compile(r"^ecx, (0x[0-9a-f]+|\d+)$")
TEST_FLAG = re.compile(r"^test byte ptr \[(ebp|esp) \+ (0x[0-9a-f]+|\d+)\], "
                       r"(0x[0-9a-f]+|\d+)$")

SCALAR_DELETE_NAME = re.compile(r"^\?\?_G(\w+)@@")
# The suffix after the class is deliberately not matched: `??3X@@SAXPAXI@Z`
# is the spelling that lies about this family's convention and arity, so the
# name contributes a class and nothing else.
STATIC_DELETE_NAME = re.compile(r"^\?\?3(\w+)@@")
QUALIFIED_NAME = re.compile(r"^(\w+)::(\w+)$")
UNNAMED = re.compile(r"^sub_[0-9a-fA-F]+$")

DTOR_NAME = re.compile(r"^(?:j_)?\?\?1(\w+)@@")
METHOD_NAME = re.compile(r"^\?(\w+)@(\w+)@@")

# The receiver sits this far into a poisoned arena, so the largest adjustment
# the family uses still lands inside it. Checked against the emitted rows.
TEST_ARENA_LEAD = 24576
TEST_ARENA_TAIL = 256

DTOR_TYPE = "func_deleting_dtor"
FORWARD_TYPES = {
    4: "func_deleting_forward",
    0: "func_deleting_forward_nullary",
}


def owns(row) -> bool:
    """True when this generator may emit the row.

    Both families are found by shape, and the shapes are not exclusive to the
    rows this tool owns: 93 virtual-base adjustor thunks decode as Family A
    and four hand-written scalar deleting destructors decode as Family B.
    Those already have committed bodies, so emitting them here would define
    the same recovery twice. Testing the recorded source file rather than a
    hardcoded address list means a row this generator itself recovered keeps
    being re-emitted on the next run."""
    if row["recovery_state"] == "unrecovered":
        return True
    return GENERATED_SOURCE in (row.get("source_locations") or "")


def family_of(data: bytes) -> str | None:
    """`adjust`, `scalar`, or None, from the entry bytes and their extent.

    The extent is the whole of what a decoder may read, so requiring it to
    equal the template length exactly is what proves there is no further code
    hiding behind a matched prologue."""
    if len(data) == 8 and (data[0:3] == b"\x2b\x49\xfc"
                           or data[0:2] == b"\x83\xe9"):
        return "adjust"
    if len(data) == 11 and data[0:2] == b"\x81\xe9":
        return "adjust"
    if len(data) == 33 and data[0:6] == b"\x55\x8b\xec\x56\x8b\xf1":
        return "scalar"
    if len(data) == 30 and data[0:3] == b"\x56\x8b\xf1":
        return "scalar"
    return None


def decode_raw_adjust(data: bytes, address: int):
    """(kind, value, target) read straight off a Family A encoding.

    The capstone path matches on printed mnemonics, which puts a text
    renderer between the bytes and the arithmetic that ends up in committed
    source. This reads the ModRM and immediate bytes instead, and
    build_adjust requires the two readings to agree. Returns None when the
    bytes are not a template, leaving the capstone path to report why."""
    if data[0:3] == b"\x2b\x49\xfc":
        # `sub ecx, [ecx-4]`: 2B /1, ecx as base and destination, disp8 -4.
        kind, value, offset = "vtordisp", 4, 3
    elif data[0:2] == b"\x81\xe9":
        # `sub ecx, imm32`: 81 /5.
        kind = "imm"
        value = int.from_bytes(data[2:6], "little", signed=True)
        offset = 6
    elif data[0:2] == b"\x83\xe9":
        # `sub ecx, imm8`: 83 /5, sign extended into the full register.
        kind = "imm"
        value = int.from_bytes(data[2:3], "little", signed=True)
        offset = 3
    else:
        return None
    # `jmp rel32`: E9, and nothing at all after it.
    if data[offset:offset + 1] != b"\xe9" or len(data) != offset + 5:
        return None
    relative = int.from_bytes(data[offset + 1:offset + 5], "little",
                              signed=True)
    return kind, value, (address + offset + 5 + relative) & 0xFFFFFFFF


def decode_adjust(pe, row):
    """(kind, value, target) for a conforming Family A row, else a reason."""
    instructions = delegates.trailing_padding(
        delegates.decode(pe, int(row["address"], 16), entry_extent(row)))
    if len(instructions) != 2:
        return f"{len(instructions)} instructions, expected 2"
    first, last = instructions
    if first.mnemonic != "sub":
        return f"leading `{first.mnemonic} {first.op_str}`, expected a sub"
    immediate = SUB_IMMEDIATE.match(first.op_str)
    if SUB_VTORDISP.match(first.op_str):
        kind, value = "vtordisp", 4
    elif immediate:
        kind, value = "imm", int(immediate.group(1), 0)
    else:
        return f"unexpected `sub {first.op_str}`"
    if last.mnemonic != "jmp" or not last.op_str.startswith("0x"):
        return f"unexpected tail `{last.mnemonic} {last.op_str}`"
    return kind, value, int(last.op_str, 16)


def decode_raw_scalar_delete(data: bytes, address: int):
    """(form, destructor, deleter, mask, pop, parameter) off the encoding.

    Every offset the emitted body depends on is walked instruction by
    instruction rather than indexed by a constant. The guarded call is the
    one that matters: at 0x0064557E, one byte earlier, sits the tail of
    `__alloca_probe`, so an off-by-one here would name a plausible-looking
    function instead of `operator delete` and nothing downstream would
    notice. Walking gives +18 for the ebp form and +16 for the frameless
    one, with the instruction after them at +23 and +21."""
    if len(data) == 33 and data[0:6] == b"\x55\x8b\xec\x56\x8b\xf1":
        form, frame, depth, offset = "frame", True, 0, 6
    elif len(data) == 30 and data[0:3] == b"\x56\x8b\xf1":
        # No frame, so the one `push esi` is what moves the parameters.
        form, frame, depth, offset = "frameless", False, 1, 3
    else:
        return None
    if data[offset:offset + 1] != b"\xe8":
        return None
    destructor = address + offset + 5 + int.from_bytes(
        data[offset + 1:offset + 5], "little", signed=True)
    offset += 5
    # `test byte ptr [reg+disp8], imm8`: F6 /0. The esp form needs the SIB
    # byte 24 between the ModRM and the displacement.
    if data[offset:offset + 2] == b"\xf6\x45":
        slot, offset = data[offset + 2], offset + 3
    elif data[offset:offset + 3] == b"\xf6\x44\x24":
        slot, offset = data[offset + 3], offset + 4
    else:
        return None
    mask = data[offset]
    offset += 1
    if data[offset:offset + 1] != b"\x74":
        return None
    skipped = int.from_bytes(data[offset + 1:offset + 2], "little",
                             signed=True)
    offset += 2
    guarded = offset
    if data[offset:offset + 2] != b"\x56\xe8":
        return None
    deleter = address + offset + 6 + int.from_bytes(
        data[offset + 2:offset + 6], "little", signed=True)
    offset += 6
    # `add esp, 4`: the deleter is __cdecl and the thunk cleans after it.
    if data[offset:offset + 3] != b"\x83\xc4\x04":
        return None
    offset += 3
    # The branch has to clear exactly the guarded push/call/cleanup, or the
    # flag does not control what it appears to control.
    if guarded + skipped != offset:
        return None
    epilogue = b"\x8b\xc6\x5e\x5d" if frame else b"\x8b\xc6\x5e"
    if data[offset:offset + len(epilogue)] != epilogue:
        return None
    offset += len(epilogue)
    if data[offset:offset + 1] != b"\xc2" or offset + 3 != len(data):
        return None
    pop = int.from_bytes(data[offset + 1:offset + 3], "little")
    register = "ebp" if frame else "esp"
    parameter = delegates.parse_stack_slot(f"[{register} + {slot}]", frame,
                                           depth)
    return (form, destructor & 0xFFFFFFFF, deleter & 0xFFFFFFFF, mask, pop,
            parameter)


def decode_scalar_delete(pe, row):
    """The same six values from capstone, or a reason the row was skipped."""
    body = delegates.trailing_padding(
        delegates.decode(pe, int(row["address"], 16), entry_extent(row)))
    rendered = [f"{i.mnemonic} {i.op_str}".rstrip() for i in body]
    if rendered[:4] == ["push ebp", "mov ebp, esp", "push esi",
                        "mov esi, ecx"]:
        form, frame, depth, head = "frame", True, 0, 4
        epilogue = ["add esp, 4", "mov eax, esi", "pop esi", "pop ebp"]
    elif rendered[:2] == ["push esi", "mov esi, ecx"]:
        form, frame, depth, head = "frameless", False, 1, 2
        epilogue = ["add esp, 4", "mov eax, esi", "pop esi"]
    else:
        return "prologue is not a scalar deleting destructor's"
    if len(rendered) != head + 6 + len(epilogue):
        return f"{len(rendered)} instructions, expected " \
               f"{head + 6 + len(epilogue)}"
    destructor, guard, branch, push, free = rendered[head:head + 5]
    if rendered[head + 5:head + 5 + len(epilogue)] != epilogue:
        return "epilogue is not the cleanup, `mov eax, esi` and pops"
    if push != "push esi":
        return f"unexpected `{push}` ahead of the guarded call"
    if not destructor.startswith("call 0x") or not free.startswith("call 0x"):
        return "destructor or deleter is not a direct call"
    flags = TEST_FLAG.match(guard)
    if flags is None:
        return f"unexpected guard `{guard}`"
    if not branch.startswith("je 0x"):
        return f"unexpected branch `{branch}`"
    # The branch has to land past the cleanup, on the shared return path.
    if int(branch.split()[1], 16) != body[head + 6].address:
        return "the flag branch does not skip exactly the guarded free"
    exit_instruction = rendered[-1]
    if not exit_instruction.startswith("ret "):
        return f"unexpected exit `{exit_instruction}`"
    parameter = delegates.parse_stack_slot(
        f"[{flags.group(1)} + {flags.group(2)}]", frame, depth)
    return (form, int(destructor.split()[1], 16), int(free.split()[1], 16),
            int(flags.group(3), 0), int(exit_instruction.split()[1], 0),
            parameter)


def scalar_delete_symbol(name: str) -> str:
    """The emitted symbol for a Family B body, or '' when unnameable."""
    match = SCALAR_DELETE_NAME.match(name)
    if match:
        return f"scalar_delete_{snake(match.group(1))}"
    if UNNAMED.match(name):
        return f"scalar_delete_{name.lower()}"
    return ""


def adjust_symbol(name: str) -> str:
    """The emitted symbol for a Family A body, or '' when unnameable.

    Never `operator_delete`, whatever `??3X@@SAXPAXI@Z` claims: the bytes
    make this a receiver fixup in front of a tail call, and the emitted name
    has to survive being read by someone who trusts it."""
    match = STATIC_DELETE_NAME.match(name)
    if match:
        return f"adjust_this_{snake(match.group(1))}"
    match = QUALIFIED_NAME.match(name)
    if match:
        return f"adjust_this_{snake(match.group(1))}_{snake(match.group(2))}"
    if UNNAMED.match(name):
        return f"adjust_this_{name.lower()}"
    return ""


def target_symbol(name: str, address: int) -> str:
    """A seam name for a forward target, from its own catalogue name."""
    match = SCALAR_DELETE_NAME.match(name)
    if match:
        return f"{match.group(1)}ScalarDeleteTarget"
    match = DTOR_NAME.match(name)
    if match:
        return f"{match.group(1)}DtorTarget"
    match = METHOD_NAME.match(name)
    if match:
        return f"{match.group(2)}{camel(match.group(1))}Target"
    if UNNAMED.match(name):
        return f"Sub{address:08X}Target"
    return ""


def classify(pe):
    """{family: [(address, row, bytes)]}: candidate rows found by shape.

    This is only a prefilter on the prologue and the extent. Ownership is
    settled later, once both decoders have agreed the row really is a family
    member, so a body that merely starts like one is not reported as
    somebody else's recovery."""
    candidates = {"adjust": [], "scalar": []}
    for row in delegates.load_rows(None, None):
        extent = entry_extent(row)
        if extent not in TEMPLATE_EXTENTS:
            continue
        address = int(row["address"], 16)
        try:
            data = disasm.read_range(pe, address, extent)
        except ValueError:
            continue
        if len(data) != extent:
            continue
        family = family_of(data)
        if family is None:
            continue
        candidates[family].append((address, row, data))
    for entries in candidates.values():
        entries.sort(key=lambda entry: entry[0])
    return candidates


def resolve_destructor(functions, target):
    """(forward spelling, include, seam) for a Family B destructor call."""
    row = functions.get(target)
    if row is None:
        return f"destructor 0x{target:08X} not catalogued"
    handled = HANDLED_DESTRUCTORS.get(row["name"])
    if handled is not None:
        expected, include, spelling = handled
        if expected != target:
            raise SystemExit(
                f"{row['name']}: the handled table says 0x{expected:08X}, "
                f"the bytes say 0x{target:08X}")
        return spelling, include, None
    if row["recovery_state"] != "unrecovered":
        # Something else already owns this address. Binding a second
        # fixed-address seam onto it would work and would quietly compete
        # with the existing one, so the whole row is left alone instead.
        return f"destructor {row['name']} is {row['recovery_state']} " \
               f"and unmapped"
    seam = target_symbol(row["name"], target)
    if not seam:
        return f"cannot name destructor {row['name']}"
    return f"{seam}({{0}})", None, (seam, target, DTOR_TYPE)


def build_scalar(pe, functions, candidates):
    """(rows, leftovers, foreign): accepted Family B bodies, by address."""
    rows = []
    leftovers = []
    foreign = []
    for address, row, data in candidates:
        decoded = decode_scalar_delete(pe, row)
        if isinstance(decoded, str):
            if owns(row):
                leftovers.append((row["address"], row["name"], decoded))
            continue
        raw = decode_raw_scalar_delete(data, address)
        if raw != decoded:
            raise SystemExit(
                f"{row['name']}: capstone reads {decoded}, the encoding "
                f"reads {raw}; refusing to emit either")
        if not owns(row):
            foreign.append((row["address"], row["name"],
                            row.get("source_locations")
                            or row["recovery_state"]))
            continue
        form, destructor, deleter, mask, pop, parameter = decoded
        symbol = scalar_delete_symbol(row["name"])
        if not symbol:
            leftovers.append((row["address"], row["name"], "unnameable body"))
            continue
        # Arity, derived and then required: one popped slot, and a guard that
        # reads the parameter that slot belongs to.
        if pop != 4 or parameter != 0:
            leftovers.append(
                (row["address"], row["name"],
                 f"pops {pop} and guards parameter {parameter}"))
            continue
        if mask != 1:
            leftovers.append((row["address"], row["name"],
                              f"guard mask 0x{mask:02X}, expected 0x01"))
            continue
        if deleter != DELETER_ADDRESS:
            leftovers.append(
                (row["address"], row["name"],
                 f"deleter 0x{deleter:08X}, expected "
                 f"0x{DELETER_ADDRESS:08X}"))
            continue
        resolved = resolve_destructor(functions, destructor)
        if isinstance(resolved, str):
            leftovers.append((row["address"], row["name"], resolved))
            continue
        spelling, include, seam = resolved
        rows.append({
            "address": address,
            "name": row["name"],
            "symbol": symbol,
            "family": "scalar",
            "form": form,
            "destructor": destructor,
            "destructor_name": functions[destructor]["name"],
            "call": spelling,
            "include": include,
            "seam": seam,
        })
    return rows, leftovers, foreign


def resolve_forward(functions, target, pop, emitted):
    """(forward spelling, include, seam) for a Family A tail call."""
    row = functions.get(target)
    if row is None:
        return f"target 0x{target:08X} not catalogued"
    body = emitted.get(target)
    if body is not None:
        # This run recovers the target, so the forward is an ordinary call
        # rather than a fixed address. Its stack contract is known from the
        # target's own `ret 4`, so a disagreeing pop means one of the two
        # readings is wrong.
        if pop != 4:
            return f"target {row['name']} pops 4, this thunk's pop is {pop}"
        return f"{body['symbol']}({{0}}, nullptr, {{1}})", None, None
    handled = HANDLED_FORWARDS.get(row["name"])
    if handled is not None:
        expected, expected_pop, include, spelling = handled
        if expected != target:
            raise SystemExit(
                f"{row['name']}: the handled table says 0x{expected:08X}, "
                f"the bytes say 0x{target:08X}")
        if expected_pop != pop:
            return f"target {row['name']} pops {pop}, the handled table " \
                   f"says {expected_pop}"
        return spelling, include, None
    if row["recovery_state"] not in ("unrecovered", "original_dependency"):
        return f"target {row['name']} is {row['recovery_state']} and unmapped"
    seam = target_symbol(row["name"], target)
    if not seam:
        return f"cannot name target {row['name']}"
    kind = FORWARD_TYPES[pop]
    arguments = "{0}" if pop == 0 else "{0}, {1}"
    return f"{seam}({arguments})", None, (seam, target, kind)


def build_adjust(pe, functions, candidates, emitted):
    """(rows, leftovers, foreign): accepted Family A bodies, by address."""
    rows = []
    leftovers = []
    foreign = []
    for address, row, data in candidates:
        decoded = decode_adjust(pe, row)
        if isinstance(decoded, str):
            if owns(row):
                leftovers.append((row["address"], row["name"], decoded))
            continue
        raw = decode_raw_adjust(data, address)
        if raw != decoded:
            raise SystemExit(
                f"{row['name']}: capstone reads {decoded}, the encoding "
                f"reads {raw}; refusing to emit either")
        if not owns(row):
            foreign.append((row["address"], row["name"],
                            row.get("source_locations")
                            or row["recovery_state"]))
            continue
        kind, value, target = decoded
        symbol = adjust_symbol(row["name"])
        if not symbol:
            leftovers.append((row["address"], row["name"], "unnameable body"))
            continue
        # An uncatalogued target has no extent to decode and so no arity to
        # read; naming it here rather than letting the pop check report it
        # keeps the skip reason honest about which fact was missing.
        if target not in functions:
            leftovers.append((row["address"], row["name"],
                              f"target 0x{target:08X} not catalogued"))
            continue
        # The thunk pops nothing, so the entire stack contract is the
        # target's. Without it there is no signature to emit.
        pop = callee_pop(pe, functions, target)
        if pop is None:
            leftovers.append((row["address"], row["name"],
                              "target callee-pop not determinable"))
            continue
        if pop not in FORWARD_TYPES:
            leftovers.append((row["address"], row["name"],
                              f"target pops {pop}, unmodelled"))
            continue
        resolved = resolve_forward(functions, target, pop, emitted)
        if isinstance(resolved, str):
            leftovers.append((row["address"], row["name"], resolved))
            continue
        spelling, include, seam = resolved
        target_row = functions[target]
        rows.append({
            "address": address,
            "name": row["name"],
            "symbol": symbol,
            "family": "adjust",
            "kind": kind,
            "value": value,
            "target": target,
            "target_name": target_row["name"],
            "pop": pop,
            "call": spelling,
            "include": include,
            "seam": seam,
            "forwards_to": emitted.get(target),
        })
    return rows, leftovers, foreign


def seam_table(rows):
    """{name: (address, type)}, refusing any two-to-one binding."""
    by_name = {}
    by_address = {}
    for row in rows:
        if not row["seam"]:
            continue
        name, address, kind = row["seam"]
        existing = by_name.get(name)
        if existing is not None and existing != (address, kind):
            raise SystemExit(
                f"{name}: bound to 0x{existing[0]:08X}/{existing[1]} and "
                f"0x{address:08X}/{kind}")
        by_name[name] = (address, kind)
        # Deduping on the address rather than the name is what catches one
        # function reached under two spellings, which would otherwise become
        # two seams the tests rebind independently.
        seen = by_address.get(address)
        if seen is not None and seen != (name, kind):
            raise SystemExit(
                f"0x{address:08X}: bound as {seen[0]}/{seen[1]} and "
                f"{name}/{kind}")
        by_address[address] = (name, kind)
    refuse_seams_already_bound(by_address)
    return dict(sorted(by_name.items()))


def refuse_seams_already_bound(by_address):
    """Refuse to define a seam some other header already binds.

    Found the hard way: PlanWinScalarDeleteTarget is emitted by the adjustor
    generator too, because the vtordisp family reaches 0x0048BF20 as well, and
    defining it twice is a duplicate-symbol link failure a long way from its
    cause. Anything this trips on belongs in HANDLED_FORWARDS or
    HANDLED_DESTRUCTORS, reusing the existing spelling rather than adding a
    second name for one address."""
    declared = re.compile(r"extern\s+\w+\s+\*(\w+);\s*//\s*(0x[0-9A-Fa-f]{8})")
    for header in sorted(Path("src").glob("*.h")):
        if header.name == "deleting_thunks.h":
            continue
        for name, address in declared.findall(header.read_text()):
            hit = by_address.get(int(address, 16))
            if hit is not None:
                raise SystemExit(
                    f"{address} is already bound as {name} in {header}; "
                    f"this run would define it again as {hit[0]}. Reuse the "
                    f"existing symbol through HANDLED_FORWARDS or "
                    f"HANDLED_DESTRUCTORS instead.")


def parameters_of(row):
    """(declaration list, argument name) for an emitted body."""
    pop = 4 if row["family"] == "scalar" else row["pop"]
    if pop == 0:
        return ["void *self", "void *"], None
    return ["void *self", "void *", "unsigned int arg0"], "arg0"


def includes_of(rows):
    return sorted({row["include"] for row in rows if row["include"]}
                  | {DELETER_INCLUDE})


def render_header(rows, seams):
    out = [LICENSE.rstrip("\n"), "", "#pragma once", "", '''/*
 * Object-deletion thunks
 *
 * Two compiler-emitted families that only make sense together.
 *
 * `adjust_this_*` steps the receiver back from the subobject the caller
 * holds - by a fixed amount, or by the vtordisp stored immediately ahead of
 * it - and forwards to the real body with the arguments untouched. The
 * receiver arithmetic is the whole of the behaviour.
 *
 * `scalar_delete_*` is the compiler-generated scalar deleting destructor:
 * run the complete destructor, release the storage through the executable's
 * operator delete only when bit 0 of the flags asks, and always return the
 * object.
 *
 * Both take their receiver in ecx and pop their own stack argument. The
 * mangled names claim otherwise for most of the first family - do not
 * reintroduce them here.
 *
 * A forward that this file recovers is an ordinary call; a forward that is
 * source-owned elsewhere goes through the binding that already exists. Only
 * what neither covers gets a seam below, each defaulting to the original
 * address for the hybrid to redirect at run time.
 *
 * Generated by tools/generate_deleting_thunks.py - do not edit by hand.
 */''', ""]
    out.append(f"typedef void(__thiscall {DTOR_TYPE})(void *self);")
    used = {kind for _, kind in seams.values()}
    for pop, kind in sorted(FORWARD_TYPES.items(), reverse=True):
        if kind not in used:
            continue
        arguments = ["void *self"] + (["unsigned int arg0"] if pop else [])
        out.extend(declaration(f"typedef void *(__thiscall {kind})",
                               arguments, ";"))
    out.append("")
    for name, (address, kind) in seams.items():
        line = f"extern {kind} *{name};"
        comment = f"   // 0x{address:08X}"
        if len(line) + len(comment) <= 79:
            out.append(line + comment)
        else:
            out.append(f"// 0x{address:08X}")
            out.append(line)
    out.append("")
    for row in rows:
        declarations, _ = parameters_of(row)
        out.extend(declaration(f"void *__fastcall {row['symbol']}",
                               declarations, ";"))
    out.append("")
    return "\n".join(out)


def render_source(rows, seams):
    out = [LICENSE.rstrip("\n"), "", '#include "stdafx.h"',
           '#include "deleting_thunks.h"']
    for include in includes_of(rows):
        out.append(f'#include "{include}"')
    out.extend(["", '''/*
Verification note: the generated leaf suite proves that each scalar deleting
body runs its destructor exactly once on the receiver it was given, releases
the storage on bit 0 of the flags and on nothing else, and returns the
object; and that each receiver fixup subtracts the amount recorded for it
before forwarding its argument and its result untouched. It does NOT
independently confirm that those amounts are what the original subtracted,
because the suite's expectation comes from the same decoding the bodies do.
What pins the model is the acceptance check in
tools/generate_deleting_thunks.py: a row is emitted only when its bytes match
one of the templates exactly under two independent decoders, and only when
the popped byte count agrees between the thunk and what it calls.

Fourteen bodies are outside the leaf suite's reach and are covered only by
the batch gate: the nine whose destructor is already source-owned, and the
five fixups that forward into them. Driving those from a leaf fixture would
run a real destructor over a synthetic object.

The check that would close the remaining gap is a differential oracle running
each original thunk against its recovered twin inside the hybrid process with
the destructor patched to record its receiver. That is not built here.
*/''', ""])
    out.append("// Each seam defaults to the original body the thunk")
    out.append("// reached; the hybrid redirects it to a recovered one at run")
    out.append("// time, and the leaf suite rebinds it to a recorder.")
    for name, (address, kind) in seams.items():
        out.append(f"{kind} *{name} =")
        out.append(f"    ({kind} *)0x{address:08X};")
    out.append("")
    for row in rows:
        out.extend(render_body(row))
        out.append("")
    return "\n".join(out)


def render_body(row):
    declarations, argument = parameters_of(row)
    if row["family"] == "scalar":
        purpose = (f"The compiler-generated scalar deleting destructor at "
                   f"{row['name']}: run the complete destructor, then "
                   f"release the storage through the executable's operator "
                   f"delete only when bit 0 of the flags asks.")
    elif row["kind"] == "vtordisp":
        purpose = (f"Step the receiver back by the vtordisp stored ahead of "
                   f"it to the one {row['target_name']} expects, then "
                   f"forward unchanged.")
    else:
        purpose = (f"Step the receiver back to the subobject "
                   f"{row['target_name']} expects, then forward unchanged.")
    out = ["/*"]
    out.extend(textwrap.wrap(purpose, width=79, initial_indent="Purpose: ",
                             subsequent_indent=" " * 9,
                             break_long_words=False,
                             break_on_hyphens=False))
    out.append(f"Original Offset: {row['address']:08X}")
    out.append("Return Value: the object pointer" if row["family"] == "scalar"
               else "Return Value: the forwarded call's")
    out.append("Status: Complete")
    out.append("*/")
    out.extend(declaration(f"void *__fastcall {row['symbol']}",
                           declarations, " {"))
    if row["family"] == "scalar":
        out.append("    " + row["call"].format("self") + ";")
        out.append(f"    if ({argument} & 1) {{")
        out.append(f"        {DELETER_SYMBOL}(self);")
        out.append("    }")
        out.append("    return self;")
    else:
        out.append("    uint8_t *const object = static_cast<uint8_t *>(self);")
        if row["kind"] == "vtordisp":
            out.append("    const int32_t vtordisp =")
            out.append("        *reinterpret_cast<const int32_t *>("
                       "object - 4);")
            receiver = "object - vtordisp"
        else:
            receiver = f"object - 0x{row['value']:X}"
        statement = "    return " + row["call"].format(receiver, argument)
        if max(len(line) for line in statement.split("\n")) > 79:
            # Spelling the adjusted receiver out first is the only thing that
            # shortens the call without hiding the literal in a table.
            out.append(f"    uint8_t *const receiver = {receiver};")
            statement = "    return " + row["call"].format("receiver",
                                                           argument)
        out.append(statement + ";")
    out.append("}")
    return out


def test_tables(rows):
    """(scalar cases, forward-through-a-body cases, seamed fixup cases).

    A body is only put in the suite when a fixture can observe it without
    running an original destructor over synthetic storage."""
    seamed = {row["address"]: row for row in rows
              if row["family"] == "scalar" and row["seam"]}
    scalar = [row for row in rows if row["address"] in seamed]
    through = [row for row in rows
               if row["family"] == "adjust" and row["forwards_to"]
               and row["forwards_to"]["address"] in seamed]
    direct = [row for row in rows if row["family"] == "adjust" and row["seam"]]
    return scalar, through, direct


def render_tests(rows, seams):
    scalar, through, direct = test_tables(rows)
    lead = max([row["value"] for row in rows
                if row["family"] == "adjust" and row["kind"] == "imm"] or [0])
    if lead + 4 >= TEST_ARENA_LEAD:
        raise SystemExit(f"adjustment {lead} exceeds the test arena lead")
    out = ["namespace {", "",
           "// The receiver sits inside a poisoned arena with room ahead of",
           "// it for the largest adjustment the family uses and for the",
           "// vtordisp the two-instruction form reads. Every four-byte",
           "// window holds a distinct value, so a body that reads its",
           "// vtordisp from the wrong place cannot happen to read an equal",
           "// one. Adjusted receivers are only ever compared, never",
           "// dereferenced, so they may land anywhere.",
           f"constexpr size_t DeletingArenaLead = {TEST_ARENA_LEAD};",
           f"constexpr size_t DeletingArenaTail = {TEST_ARENA_TAIL};",
           "uint8_t g_deleting_arena[DeletingArenaLead + DeletingArenaTail];",
           "uint8_t *deleting_receiver() {",
           "    for (size_t index = 0; index < sizeof(g_deleting_arena);"
           " ++index) {",
           "        g_deleting_arena[index] = static_cast<uint8_t>("
           "index * 7u + 1u);",
           "    }",
           "    return g_deleting_arena + DeletingArenaLead;",
           "}",
           "",
           "void *g_deleting_dtor_seen;",
           "int g_deleting_dtor_calls;",
           "void *g_deleting_free_seen;",
           "int g_deleting_free_calls;",
           "void *g_deleting_forward_seen;",
           "unsigned int g_deleting_forward_arg;",
           "int g_deleting_forward_calls;",
           "",
           "void __cdecl observe_deleting_free(void *block) {",
           "    g_deleting_free_seen = block;",
           "    ++g_deleting_free_calls;",
           "}",
           "",
           "#if defined(__GNUC__)",
           "#pragma GCC diagnostic push",
           '#pragma GCC diagnostic ignored "-Wattributes"',
           "#endif",
           "void __thiscall observe_deleting_dtor(void *self) {",
           "    g_deleting_dtor_seen = self;",
           "    ++g_deleting_dtor_calls;",
           "}"]
    codes = {kind for _, kind in seams.values()}
    if FORWARD_TYPES[4] in codes:
        out.extend([
            f"void *__thiscall observe_deleting_forward(void *self,",
            "                                          unsigned int arg0) {",
            "    g_deleting_forward_seen = self;",
            "    g_deleting_forward_arg = arg0;",
            "    ++g_deleting_forward_calls;",
            "    return &g_deleting_forward_calls;",
            "}"])
    if FORWARD_TYPES[0] in codes:
        out.extend([
            "void *__thiscall observe_deleting_forward_nullary(void *self) {",
            "    g_deleting_forward_seen = self;",
            "    ++g_deleting_forward_calls;",
            "    return &g_deleting_forward_calls;",
            "}"])
    out.extend(["#if defined(__GNUC__)", "#pragma GCC diagnostic pop",
                "#endif", ""])

    out.extend(["typedef void *(__fastcall *DeletingBody)(void *, void *,",
                "                                         unsigned int);"])
    if any(row["family"] == "adjust" and row["seam"] and not row["pop"]
           for row in rows):
        # A fixup onto a target that pops nothing has a different function
        # type, and a table that spelled it as the common one would only be
        # wrong at the call.
        out.append("typedef void *(__fastcall *DeletingBodyNullary)(void *,")
        out.append("                                                void *);")
    out.append("")
    out.append("struct DeletingCase {")
    out.append("    DeletingBody body;")
    out.append(f"    {DTOR_TYPE} **slot;")
    out.append("};")
    out.append("const DeletingCase g_deleting_cases[] = {")
    for row in scalar:
        out.extend(table_entry(f"&{row['symbol']}", f"&{row['seam'][0]}"))
    out.append("};")
    out.append("")
    out.append("// The fixup is observed through the destructor its target")
    out.append("// runs: with bit 0 set, the recorder and the free both see")
    out.append("// the adjusted receiver the fixup computed.")
    out.append("struct DeletingAdjustCase {")
    out.append("    DeletingBody body;")
    out.append(f"    {DTOR_TYPE} **slot;")
    out.append("    int adjust;")
    out.append("    int vtordisp;")
    out.append("};")
    out.append("const DeletingAdjustCase g_deleting_adjust_cases[] = {")
    for row in through:
        out.extend(table_entry(
            f"&{row['symbol']}", f"&{row['forwards_to']['seam'][0]}",
            "0" if row["kind"] == "vtordisp" else f"0x{row['value']:X}",
            "1" if row["kind"] == "vtordisp" else "0"))
    out.append("};")
    out.append("")
    for pop, kind in sorted(FORWARD_TYPES.items()):
        entries = [row for row in direct if row["pop"] == pop]
        if not entries:
            continue
        suffix = "" if pop else "_nullary"
        out.append(f"struct DeletingSeamCase{suffix} {{")
        out.append(f"    DeletingBody{'' if pop else 'Nullary'} body;")
        out.append(f"    {kind} **slot;")
        out.append("    int adjust;")
        out.append("    int vtordisp;")
        out.append("};")
        out.append(f"const DeletingSeamCase{suffix} "
                   f"g_deleting_seam_cases{suffix}[] = {{")
        for row in entries:
            out.extend(table_entry(
                f"&{row['symbol']}", f"&{row['seam'][0]}",
                "0" if row["kind"] == "vtordisp" else f"0x{row['value']:X}",
                "1" if row["kind"] == "vtordisp" else "0"))
        out.append("};")
        out.append("")
    out.append("}  // namespace")
    out.append("")
    out.extend(render_test_body(direct))
    return "\n".join(out)


def table_entry(*fields):
    """One brace-initialised row, wrapped only when it has to be."""
    entry = "    {" + ", ".join(fields) + "},"
    if len(entry) <= 79:
        return [entry]
    lines = ["    {" + fields[0] + ","]
    for index, field in enumerate(fields[1:]):
        piece = field + ("," if index + 2 < len(fields) else "},")
        if len(lines[-1]) + len(piece) + 1 <= 79:
            lines[-1] += " " + piece
        else:
            lines.append("     " + piece)
    return lines


def render_test_body(direct):
    out = ["void test_deleting_thunks() {",
           "    func_operator_delete *const saved_free ="
           " ScrollOperatorDelete;",
           "    ScrollOperatorDelete = &observe_deleting_free;",
           "    for (const DeletingCase &entry : g_deleting_cases) {",
           "        // Every flag value, so a body that freed on any nonzero",
           "        // word rather than on bit 0 is caught by mode 2.",
           "        for (unsigned int mode = 0; mode < 4; ++mode) {",
           f"            {DTOR_TYPE} *const saved = *entry.slot;",
           "            *entry.slot = &observe_deleting_dtor;",
           "            void *const object = deleting_receiver();",
           "            g_deleting_dtor_calls = 0;",
           "            g_deleting_dtor_seen = nullptr;",
           "            g_deleting_free_calls = 0;",
           "            g_deleting_free_seen = nullptr;",
           "            void *const result = entry.body(object, nullptr,"
           " mode);",
           "            expect(g_deleting_dtor_calls == 1);",
           "            expect(g_deleting_dtor_seen == object);",
           "            expect(g_deleting_free_calls =="
           " static_cast<int>(mode & 1));",
           "            expect(g_deleting_free_seen ==",
           "                   ((mode & 1) ? object : nullptr));",
           "            expect(result == object);",
           "            *entry.slot = saved;",
           "        }",
           "    }",
           "    for (const DeletingAdjustCase &entry :"
           " g_deleting_adjust_cases) {",
           f"        {DTOR_TYPE} *const saved = *entry.slot;",
           "        *entry.slot = &observe_deleting_dtor;",
           "        uint8_t *const object = deleting_receiver();",
           "        const int32_t vtordisp = entry.vtordisp",
           "            ? *reinterpret_cast<const int32_t *>(object - 4) : 0;",
           "        void *const expected = object - entry.adjust - vtordisp;",
           "        g_deleting_dtor_calls = 0;",
           "        g_deleting_dtor_seen = nullptr;",
           "        g_deleting_free_calls = 0;",
           "        g_deleting_free_seen = nullptr;",
           "        void *const result = entry.body(object, nullptr, 1u);",
           "        expect(g_deleting_dtor_calls == 1);",
           "        expect(g_deleting_dtor_seen == expected);",
           "        expect(g_deleting_free_calls == 1);",
           "        expect(g_deleting_free_seen == expected);",
           "        expect(result == expected);",
           "        *entry.slot = saved;",
           "    }"]
    for pop, kind in sorted(FORWARD_TYPES.items()):
        if not any(row["pop"] == pop for row in direct):
            continue
        suffix = "" if pop else "_nullary"
        out.extend([
            f"    for (const DeletingSeamCase{suffix} &entry :",
            f"             g_deleting_seam_cases{suffix}) {{",
            f"        {kind} *const saved = *entry.slot;",
            f"        *entry.slot = &observe_deleting_forward{suffix};",
            "        uint8_t *const object = deleting_receiver();",
            "        const int32_t vtordisp = entry.vtordisp",
            "            ? *reinterpret_cast<const int32_t *>(object - 4)",
            "            : 0;",
            "        void *const expected = object - entry.adjust - vtordisp;",
            "        g_deleting_forward_calls = 0;",
            "        g_deleting_forward_seen = nullptr;",
            "        g_deleting_forward_arg = 0;",
            "        void *const result = entry.body(object, nullptr"
            + (", 0x2A2Au);" if pop else ");"),
            "        expect(g_deleting_forward_calls == 1);",
            "        expect(g_deleting_forward_seen == expected);",
            "        expect(result == &g_deleting_forward_calls);"])
        if pop:
            out.append("        expect(g_deleting_forward_arg == 0x2A2Au);")
        out.extend(["        *entry.slot = saved;", "    }"])
    out.extend(["    ScrollOperatorDelete = saved_free;", "}", ""])
    return out


def report(rows, seams, leftovers, foreign):
    scalar = [row for row in rows if row["family"] == "scalar"]
    adjust = [row for row in rows if row["family"] == "adjust"]
    print(f"scalar deleting destructors: {len(scalar)} "
          f"({sum(1 for r in scalar if r['form'] == 'frame')} framed, "
          f"{sum(1 for r in scalar if r['form'] == 'frameless')} frameless)")
    print(f"  destructors called directly:"
          f" {sum(1 for r in scalar if not r['seam'])}")
    print(f"  destructors through a new seam:"
          f" {sum(1 for r in scalar if r['seam'])}")
    print(f"receiver fixups: {len(adjust)}")
    print(f"  forwarding into a generated body:"
          f" {sum(1 for r in adjust if r['forwards_to'])}")
    existing = sum(1 for r in adjust if not r["forwards_to"]
                   and not r["seam"])
    print(f"  forwarding to an existing recovery: {existing}")
    print(f"  forwarding through a new seam:"
          f" {sum(1 for r in adjust if r['seam'])}")
    shapes = {}
    for row in adjust:
        label = ("vtordisp" if row["kind"] == "vtordisp"
                 else f"-0x{row['value']:X}")
        shapes[label] = shapes.get(label, 0) + 1
    for label, count in sorted(shapes.items(), key=lambda item: -item[1]):
        print(f"    {label:12} {count:4}")
    print(f"seams: {len(seams)}")
    scalar_test, through, direct = test_tables(rows)
    print(f"leaf-suite coverage: {len(scalar_test)} bodies, "
          f"{len(through) + len(direct)} fixups; "
          f"{len(rows) - len(scalar_test) - len(through) - len(direct)} "
          f"left to the batch gate")
    if leftovers:
        print(f"left alone: {len(leftovers)}")
        for address, name, reason in leftovers:
            print(f"  {address} {name[:34]:36} {reason[:48]}")
    if foreign:
        print(f"already recovered elsewhere, not re-emitted: {len(foreign)}")
        files = {}
        for _, _, where in foreign:
            key = where.split(":")[0]
            files[key] = files.get(key, 0) + 1
        for key, count in sorted(files.items()):
            print(f"  {key:44} {count:4}")


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--exe", type=Path, default=disasm.DEFAULT_EXE)
    parser.add_argument("--scratch-dir", type=Path, required=True)
    parser.add_argument("--source-dir", type=Path, default=None,
                        help="where the .h/.cpp pair lands; defaults to the "
                             "scratch directory so a run stages nothing "
                             "into the tree")
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    pe = pefile.PE(str(args.exe), fast_load=True)
    functions = {int(row["address"], 16): row
                 for row in delegates.load_rows(None, None)}
    if functions.get(DELETER_ADDRESS, {}).get("name") != DELETER_NAME:
        raise SystemExit(
            f"0x{DELETER_ADDRESS:08X} is not {DELETER_NAME}; the shared "
            f"deleter binding cannot be trusted")

    candidates = classify(pe)
    # Family B first: a Family A row can only resolve its tail call to an
    # ordinary function call once the body it jumps into is known to exist.
    scalar, scalar_left, scalar_foreign = build_scalar(
        pe, functions, candidates["scalar"])
    emitted = {row["address"]: row for row in scalar}
    adjust, adjust_left, adjust_foreign = build_adjust(
        pe, functions, candidates["adjust"], emitted)
    rows = sorted(scalar + adjust, key=lambda row: row["address"])
    leftovers = sorted(scalar_left + adjust_left)
    foreign = sorted(scalar_foreign + adjust_foreign)
    if not rows:
        raise SystemExit("no deleting thunks accepted")

    symbols = [row["symbol"] for row in rows]
    if len(set(symbols)) != len(symbols):
        duplicates = sorted({s for s in symbols if symbols.count(s) > 1})
        raise SystemExit(f"duplicate symbols; refusing to emit: {duplicates}")
    seams = seam_table(rows)

    report(rows, seams, leftovers, foreign)
    if args.dry_run:
        return 0

    source_dir = args.source_dir or args.scratch_dir
    source_dir.mkdir(parents=True, exist_ok=True)
    args.scratch_dir.mkdir(parents=True, exist_ok=True)
    (source_dir / "deleting_thunks.h").write_text(render_header(rows, seams))
    (source_dir / "deleting_thunks.cpp").write_text(
        render_source(rows, seams))
    (args.scratch_dir / "deleting-thunk-tests.cpp").write_text(
        render_tests(rows, seams))
    wire = "\n".join(f"0x{row['address']:08X} {row['symbol']}"
                     for row in rows) + "\n"
    (args.scratch_dir / "deleting-wire.txt").write_text(wire)
    print(f"emitted {source_dir}/deleting_thunks.{{h,cpp}}, test fragment, "
          f"wire list")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
