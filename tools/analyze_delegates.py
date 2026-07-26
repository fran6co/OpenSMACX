#!/usr/bin/env python3
"""Prove, from disassembly, that a function is a pure forwarder.

The shape classifier calls over a thousand of the remaining functions
`delegate`, the largest mechanically-shaped bucket left by a wide margin. But
that verdict comes from Ghidra's output, and Ghidra is wrong about exactly the
things a generated forwarder depends on: it drops the receiver on `__thiscall`
calls and tail jumps, and it normalises the callee-pop byte count that is the
only ground truth for arity. A generator that trusted it would emit bodies that
compile and are quietly wrong.

So this reads the bytes instead. It accepts a function only when the entire
body matches one of the rigid forms below, and reports precisely what the
forward consists of. Anything else - any branch, any store, any arithmetic,
any instruction outside the accepted set - is rejected and named. The point is
to be boring: a shape that is 99% right is worth nothing here, because the
whole value of the bucket is that the emitted body needs no human judgement.

Accepted forms
--------------

`frame-forward` / `flat-forward`, the common case, with or without an ebp
frame::

    [push ebp / mov ebp, esp]   ; optional frame
    push esi|edi|ebx            ; optional callee-saves, matched by pops
    mov <reg>, [ebp+N|esp+N]    ; load incoming parameters, any order
    push <reg|imm|ecx-as-this>  ; arguments, pushed right-to-left
    mov ecx, <imm32>            ; optional fixed receiver: a global singleton
    call <target>
    add esp, N                  ; iff the target is __cdecl
    pop <saves, reversed> [ebp]
    ret [K]

Frameless parameter slots are tracked against the running push depth, because
`[esp+8]` names a different parameter after every push - the exact arithmetic
Ghidra's output silently gets wrong.

`tail-alias`::

    jmp <target>                ; nothing else: same arguments, same receiver

`vtable-tail`, the destructor case::

    mov [ecx+off], <imm32>      ; republish one or more vtable pointers
    jmp <target>                ; tail call to the base destructor, this intact

`global-init`, the MSVC dynamic initializer (`??__E...`): construct a global
at a fixed address, optionally register its destructor with atexit.

Cross-validation
----------------

Where a mangled name can be decoded, the shape must also account for itself
arithmetically, in the same spirit as the callee-pop check that rejects
Win::on_redraw across the whole image:

- the source's own `ret K` must match its declared convention and arity;
- a parameter index read from the stack must exist in the declared list;
- an `add esp, N` after the call must clean exactly the arguments pushed;
- a named target must declare exactly the argument bytes that were pushed.

What it reports
---------------

For each accepted function: the target, the receiver (`this`, an incoming
parameter, or a fixed global), and the argument list as a vector of
`param:<index>` / `imm:<value>` / `this` entries. That vector is the
forwarding contract, and it is everything a generator needs.

This tool decides nothing about correctness. A proven shape says the body can
be written mechanically; the differential gate remains the only judge of
whether the result matches the original.

Reads proprietary bytes: local analysis only, never committed.
"""

from __future__ import annotations

import argparse
import csv
import json
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pefile  # noqa: E402
from capstone import CS_ARCH_X86, CS_MODE_32, Cs  # noqa: E402

import disasm  # noqa: E402
import find_constant_returns as const  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
FUNCTIONS_CSV = REPO_ROOT / "docs" / "recovery" / "functions.csv"

# Registers that may hold a loaded value on the way to a push.
SCRATCH = frozenset({"eax", "ecx", "edx", "ebx", "esi", "edi"})

# Registers the ABI preserves across a call; a push of one before it holds
# anything is a save, not an argument, and must come back off before ret.
CALLEE_SAVED = frozenset({"esi", "edi", "ebx"})


def name_convention_and_arity(mangled: str) -> tuple[str, int] | None:
    """(convention, parameter bytes) from a mangled name, or None.

    find_constant_returns.declared_arity reads the convention from the third
    character after `@@`, which is where member functions keep it - a free
    function's `@@YA` puts it second, so every free function decoded as None
    there. Both layouts are handled here; the parameter walk is the same one.
    """
    # The first `@@` ends the qualified name; later ones can appear inside
    # struct parameters (`PAUSprite@@`), so a greedy match would read the
    # signature out of the middle of the parameter list.
    if not mangled.startswith("?") or "@@" not in mangled:
        return None
    rest = mangled.split("@@", 1)[1]
    if not rest:
        return None
    if rest[0] == "Y":                       # free function: Y + convention
        if len(rest) < 2 or rest[1] not in "AGI":
            return None
        convention = {"A": "cdecl", "G": "stdcall", "I": "fastcall"}[rest[1]]
        rest = rest[2:]
    else:                                    # member: access + cv + convention
        if len(rest) < 3 or rest[2] not in "AEGI":
            return None
        convention = {"A": "cdecl", "E": "thiscall", "G": "stdcall",
                      "I": "fastcall"}[rest[2]]
        rest = rest[3:]
    if convention == "fastcall":
        return None                          # register args; out of scope

    item, index = const.token(rest, 0)       # the return type
    if item is None:
        return None
    total = 0
    while index < len(rest) and rest[index] not in "@Z":
        item, following = const.token(rest, index)
        if item is None:
            return None
        if item == "X":                      # void: no parameters at all
            break
        if item[0] in "PQRA" or item.startswith("_"):
            total += 4
        elif item in const.SIZES:
            total += const.SIZES[item]
        else:
            return None
        index = following
    return convention, total


@dataclass
class Argument:
    """One value the forwarder passes on."""
    kind: str            # "param" | "imm" | "this"
    value: int           # parameter index, or the literal; 0 for "this"

    def describe(self) -> str:
        if self.kind == "this":
            return "this"
        return (f"param:{self.value}" if self.kind == "param"
                else f"imm:0x{self.value & 0xFFFFFFFF:X}")


@dataclass
class Forward:
    address: int
    name: str
    size: int
    form: str                       # frame-forward | flat-forward | tail-alias
                                    # | vtable-tail | global-init
    target: int
    tail_call: bool                 # jmp rather than call
    receiver: str | None            # "this" | "param:N" | "global:0xADDR"
    arguments: list[Argument] = field(default_factory=list)
    popped: int = 0                 # callee-pop bytes from `ret K`
    caller_cleanup: int = 0         # `add esp, N` after the call
    vtable_writes: list[tuple[int, int]] = field(default_factory=list)

    def describe(self) -> str:
        args = ", ".join(argument.describe() for argument in self.arguments)
        receiver = f" this={self.receiver}" if self.receiver else ""
        tail = " tail" if self.tail_call else ""
        return (f"{self.form}{tail} -> 0x{self.target:08X}{receiver} "
                f"({args}) ret {self.popped}")


class Reject(Exception):
    """The body is not a provable forwarder. The message says why."""


def decode(pe, address: int, size: int):
    code = disasm.read_range(pe, address, size)
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    return list(engine.disasm(code, address))


def trailing_padding(instructions) -> list:
    """Drop the alignment nops between one function and the next."""
    end = len(instructions)
    while end and instructions[end - 1].mnemonic in ("nop", "int3"):
        end -= 1
    return instructions[:end]


def parse_stack_slot(operand: str, frame: bool, depth: int) -> int | None:
    """Map a `[ebp+N]` or `[esp+N]` load to a parameter index, or None.

    With a frame, `[ebp+8]` is parameter 0 regardless of pushes. Without one,
    `[esp]` starts at the return address and every push since entry moves the
    parameters four bytes further out - so the slot comes from the tracked
    depth, never from the displacement text alone.
    """
    text = operand.replace(" ", "")
    if frame:
        if "[ebp+" not in text:
            return None
        try:
            displacement = int(text.split("[ebp+")[1].rstrip("]"), 0)
        except ValueError:
            return None
        if displacement < 8 or displacement % 4:
            return None
        return (displacement - 8) // 4
    if "[esp+" in text:
        try:
            displacement = int(text.split("[esp+")[1].rstrip("]"), 0)
        except ValueError:
            return None
    elif text.endswith("[esp]"):
        displacement = 0
    else:
        return None
    adjusted = displacement - 4 - 4 * depth
    if adjusted < 0 or adjusted % 4:
        return None
    return adjusted // 4


def analyze_vtable_tail(instructions, row) -> Forward:
    """`mov [ecx+off], imm` repeated, then `jmp base` with `this` untouched."""
    writes = []
    for index, instruction in enumerate(instructions):
        mnemonic, operands = instruction.mnemonic, instruction.op_str
        if mnemonic == "mov" and operands.startswith("dword ptr [ecx"):
            destination, _, source = operands.partition(", ")
            text = destination.replace(" ", "")
            offset_text = text.split("[ecx")[1].rstrip("]")
            offset = 0 if not offset_text else int(offset_text.lstrip("+"), 0)
            if not source.startswith("0x"):
                raise Reject(f"vtable store of non-literal {source!r}")
            writes.append((offset, int(source, 16)))
            continue
        if mnemonic == "jmp" and operands.startswith("0x"):
            if index != len(instructions) - 1:
                raise Reject("jmp is not the final instruction")
            return Forward(
                address=int(row["address"], 16), name=row["name"],
                size=int(row["size"]), form="vtable-tail",
                target=int(operands, 16), tail_call=True,
                receiver="this", vtable_writes=writes)
        raise Reject(f"unexpected {mnemonic} {operands}".strip())
    raise Reject("no tail jump")


def analyze_global_init(instructions, row) -> Forward:
    """An MSVC dynamic initializer for a global object.

    Four variants appear across the image, and all four are the same
    statement: construct the object that lives at a fixed address, then
    register its destructor to run at exit.

        mov ecx, <global> / call <ctor>                     scalar
        push <arg> / mov ecx, <global> / call <ctor>        scalar + argument
        push <dtor> <ctor> <count> <size> <array>
            / call <vector ctor iterator>                   array
        [ push <dtor thunk> / call <atexit> / pop ecx ]     registration

    The tail-jump variant registers nothing.
    """
    arguments: list[Argument] = []
    receiver: str | None = None
    target: int | None = None
    tail_call = False
    registered: int | None = None
    index = 0

    while index < len(instructions):
        instruction = instructions[index]
        mnemonic, operands = instruction.mnemonic, instruction.op_str
        index += 1

        if target is None:
            if mnemonic == "push" and (operands.startswith("0x")
                                       or operands.lstrip("-").isdigit()):
                arguments.append(Argument("imm", int(operands, 0)))
                continue
            if mnemonic == "mov" and operands.startswith("ecx, 0x"):
                receiver = f"global:0x{int(operands.split(', ')[1], 16):08X}"
                continue
            if mnemonic in ("call", "jmp") and operands.startswith("0x"):
                target = int(operands, 16)
                tail_call = mnemonic == "jmp"
                continue
            raise Reject(f"unexpected {mnemonic} {operands}".strip())

        # After the constructor call: at most the atexit registration.
        if mnemonic == "push" and operands.startswith("0x"):
            registered = int(operands, 16)
            continue
        if mnemonic == "call" and operands.startswith("0x"):
            if registered is None:
                raise Reject("second call without a registered destructor")
            continue
        if mnemonic == "pop" and operands == "ecx":
            continue
        if mnemonic == "ret":
            if operands:
                raise Reject(f"dynamic initializer pops {operands}")
            break
        raise Reject(f"unexpected {mnemonic} {operands} after the call".strip())

    if target is None:
        raise Reject("no constructor call")
    arguments.reverse()
    forward = Forward(
        address=int(row["address"], 16), name=row["name"],
        size=int(row["size"]), form="global-init", target=target,
        tail_call=tail_call, receiver=receiver, arguments=arguments)
    if registered is not None:
        # The registered destructor thunk, recorded as the trailing entry so
        # the contract is complete without a separate field.
        forward.vtable_writes.append((-1, registered))
    return forward


def analyze_forward(instructions, row, functions=None) -> Forward:
    """The general forwarder: optional frame, saves, loads, pushes, one call.

    Framed and frameless bodies share this walk; the only difference is where
    parameters live, which `parse_stack_slot` resolves against the frame flag
    and the running push depth.
    """
    own = name_convention_and_arity(row["name"])
    own_convention = own[0] if own else None
    own_bytes = own[1] if own else None
    is_thiscall_source = own_convention == "thiscall"

    index = 0
    frame = False
    if (len(instructions) >= 2
            and (instructions[0].mnemonic, instructions[0].op_str) == ("push", "ebp")
            and (instructions[1].mnemonic, instructions[1].op_str) == ("mov", "ebp, esp")):
        frame = True
        index = 2

    # A single jmp is an alias: same stack, same receiver. It can only be
    # synthesised into a call when the own parameter list is known.
    if (index == 0 and len(instructions) == 1
            and instructions[0].mnemonic == "jmp"
            and instructions[0].op_str.startswith("0x")):
        if own_bytes is None:
            raise Reject("tail alias with undecodable own arity")
        target_address = int(instructions[0].op_str, 16)
        check_target_arity(functions, target_address, own_bytes,
                           expect_callee_pop=own_convention != "cdecl")
        return Forward(
            address=int(row["address"], 16), name=row["name"],
            size=int(row["size"]), form="tail-alias",
            target=target_address, tail_call=True,
            receiver="this" if is_thiscall_source else None,
            arguments=[Argument("param", slot)
                       for slot in range(own_bytes // 4)],
            popped=0)

    held: dict[str, Argument] = {}
    if is_thiscall_source:
        held["ecx"] = Argument("this", 0)
    saves: list[str] = []            # callee-saves, in push order
    depth = 0                        # every push since entry, saves included
    pushed: list[Argument] = []
    receiver: str | None = None
    target: int | None = None
    tail_call = False
    caller_cleanup: int | None = None
    popped = 0
    pops: list[str] = []
    saw_frame_pop = False
    saw_ret = False

    while index < len(instructions):
        instruction = instructions[index]
        mnemonic, operands = instruction.mnemonic, instruction.op_str
        index += 1

        if target is None:
            if mnemonic == "mov":
                destination, _, source = operands.partition(", ")
                if destination not in SCRATCH:
                    raise Reject(f"store to {destination!r} before the call")
                slot = parse_stack_slot(source, frame, depth)
                if slot is not None:
                    if own_bytes is not None and slot >= own_bytes // 4:
                        raise Reject(
                            f"parameter {slot} outside the declared list")
                    held[destination] = Argument("param", slot)
                    continue
                if source.startswith("0x"):
                    held[destination] = Argument("imm", int(source, 16))
                    continue
                if source in held:            # register copy keeps the value
                    held[destination] = held[source]
                    continue
                raise Reject(f"mov {destination}, {source} is not a "
                             f"parameter load")
            if mnemonic == "push":
                if operands in held:
                    pushed.append(held[operands])
                    depth += 1
                    continue
                if operands in CALLEE_SAVED:
                    # Not holding anything: a callee-save. It must come back
                    # off in reverse order, which the epilogue walk checks.
                    if pushed:
                        raise Reject(f"save of {operands} after arguments")
                    saves.append(operands)
                    depth += 1
                    continue
                if operands.startswith("0x") or operands.lstrip("-").isdigit():
                    pushed.append(Argument("imm", int(operands, 0)))
                    depth += 1
                    continue
                raise Reject(f"push {operands} is neither a held value nor "
                             f"a literal")
            if mnemonic in ("call", "jmp") and operands.startswith("0x"):
                if mnemonic == "jmp" and (pushed or saves or frame):
                    raise Reject("tail jump over a live stack")
                target = int(operands, 16)
                tail_call = mnemonic == "jmp"
                if "ecx" in held:
                    holder = held["ecx"]
                    if holder.kind == "this":
                        receiver = "this"
                    elif holder.kind == "imm":
                        receiver = f"global:0x{holder.value:08X}"
                    else:
                        receiver = f"param:{holder.value}"
                continue
            raise Reject(f"unexpected {mnemonic} {operands}".strip())

        # After the call: cleanup and epilogue only.
        if mnemonic == "add" and operands.startswith("esp, "):
            caller_cleanup = int(operands.split(", ")[1], 0)
            continue
        if mnemonic == "pop":
            if operands == "ebp":
                saw_frame_pop = True
            else:
                pops.append(operands)
            continue
        if mnemonic == "ret":
            popped = int(operands, 0) if operands else 0
            saw_ret = True
            break
        raise Reject(f"unexpected {mnemonic} {operands} after the call".strip())

    if target is None:
        raise Reject("no call")
    if not tail_call:
        if not saw_ret:
            raise Reject("no ret")
        if frame and not saw_frame_pop:
            raise Reject("no frame epilogue")
        if pops != list(reversed(saves)):
            raise Reject("callee-saves not restored in reverse order")

    # The shape must account for itself arithmetically wherever a mangled
    # name can be decoded; these are the checks that reject a plausible-
    # looking body whose numbers do not add up.
    argument_bytes = 4 * len(pushed)
    if caller_cleanup is not None and caller_cleanup != argument_bytes:
        raise Reject(f"cleans {caller_cleanup} bytes but pushed "
                     f"{argument_bytes}")
    if own_convention == "cdecl" and popped:
        raise Reject(f"cdecl body pops {popped} bytes")
    if own_convention in ("stdcall", "thiscall") and own_bytes is not None \
            and not tail_call and popped != own_bytes:
        raise Reject(f"{own_convention} body pops {popped} of {own_bytes}")
    check_target_arity(functions, target, argument_bytes,
                       expect_callee_pop=caller_cleanup is None)

    pushed.reverse()                 # right-to-left pushes -> source order
    return Forward(
        address=int(row["address"], 16), name=row["name"],
        size=int(row["size"]),
        form="frame-forward" if frame else "flat-forward",
        target=target, tail_call=tail_call, receiver=receiver,
        arguments=pushed, popped=popped,
        caller_cleanup=caller_cleanup or 0)


def check_target_arity(functions, target: int, argument_bytes: int,
                       expect_callee_pop: bool) -> None:
    """A named target must declare exactly the bytes that were pushed."""
    if not functions:
        return
    row = functions.get(target)
    if not row:
        return
    decoded = name_convention_and_arity(row.get("name", ""))
    if not decoded:
        return
    convention, declared = decoded
    if declared != argument_bytes:
        raise Reject(f"target declares {declared} argument bytes, "
                     f"{argument_bytes} pushed")
    if convention == "cdecl" and expect_callee_pop and argument_bytes:
        raise Reject("cdecl target but no caller cleanup")
    if convention in ("stdcall", "thiscall") and not expect_callee_pop:
        raise Reject(f"{convention} target but caller cleans the stack")


def analyze(pe, row, functions=None) -> Forward:
    size = int(row["size"])
    if not 1 <= size <= 256:
        raise Reject(f"size {size} outside the forwarder range")
    instructions = trailing_padding(decode(pe, int(row["address"], 16), size))
    if not instructions:
        raise Reject("no decodable instructions")
    # An incomplete decode means capstone gave up part way, which is never a
    # forwarder and must not be analysed as a truncated one.
    consumed = sum(len(instruction.bytes) for instruction in instructions)
    trailing = size - consumed
    if trailing < 0 or trailing > 15:
        raise Reject(f"decoded {consumed} of {size} bytes")

    # The mangled name is what identifies a dynamic initializer; `??__E` is
    # MSVC's prefix for one, and no other function carries it.
    if row["name"].startswith("??__E"):
        return analyze_global_init(instructions, row)
    if instructions[0].mnemonic == "mov" and \
            instructions[0].op_str.startswith("dword ptr [ecx"):
        return analyze_vtable_tail(instructions, row)
    return analyze_forward(instructions, row, functions)


def load_rows(addresses: set[int] | None, state: str | None):
    with FUNCTIONS_CSV.open() as handle:
        for row in csv.DictReader(handle):
            try:
                address = int(row["address"], 16)
            except ValueError:
                continue
            if addresses is not None and address not in addresses:
                continue
            if state and row.get("recovery_state") != state:
                continue
            if not row.get("size"):
                continue
            yield row


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--exe", type=Path, default=disasm.DEFAULT_EXE)
    parser.add_argument("--address", action="append", default=[],
                        help="restrict to these addresses (repeatable)")
    parser.add_argument("--recovery-state", default="unrecovered")
    parser.add_argument("--limit", type=int, default=0)
    parser.add_argument("--json", type=Path,
                        help="write the accepted forwards here")
    parser.add_argument("--show-rejections", action="store_true")
    args = parser.parse_args()

    addresses = {int(value, 16) for value in args.address} or None
    pe = pefile.PE(str(args.exe), fast_load=True)
    functions = {int(row["address"], 16): row
                 for row in load_rows(None, None)}

    accepted: list[Forward] = []
    rejected: list[tuple[str, str]] = []
    for row in load_rows(addresses, args.recovery_state or None):
        try:
            accepted.append(analyze(pe, row, functions))
        except Reject as reason:
            rejected.append((row["address"], str(reason)))
        except Exception as error:                     # noqa: BLE001
            rejected.append((row["address"], f"analysis error: {error}"))
        if args.limit and len(accepted) >= args.limit:
            break

    forms = {}
    for forward in accepted:
        forms[forward.form] = forms.get(forward.form, 0) + 1
    print(f"proven forwarders: {len(accepted)} "
          f"(rejected {len(rejected)})")
    for form, count in sorted(forms.items(), key=lambda item: -item[1]):
        print(f"  {form:16} {count:5}")

    if args.show_rejections:
        reasons = {}
        for _, reason in rejected:
            key = reason.split(" 0x")[0][:60]
            reasons[key] = reasons.get(key, 0) + 1
        print("\nrejection reasons:")
        for reason, count in sorted(reasons.items(), key=lambda i: -i[1])[:15]:
            print(f"  {count:5}  {reason}")

    if args.json:
        payload = [
            {
                "address": f"0x{forward.address:08X}",
                "name": forward.name,
                "size": forward.size,
                "form": forward.form,
                "target": f"0x{forward.target:08X}",
                "tail_call": forward.tail_call,
                "receiver": forward.receiver,
                "arguments": [
                    {"kind": argument.kind, "value": argument.value}
                    for argument in forward.arguments
                ],
                "popped": forward.popped,
                "caller_cleanup": forward.caller_cleanup,
                "vtable_writes": [
                    {"offset": offset, "value": f"0x{value:08X}"}
                    for offset, value in forward.vtable_writes
                ],
            }
            for forward in accepted
        ]
        args.json.write_text(json.dumps(payload, indent=2) + "\n")
        print(f"\nwrote {args.json}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
