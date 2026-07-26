#!/usr/bin/env python3
"""Prove, from disassembly, that a function is a pure forwarder.

The shape classifier calls 1,052 of the remaining functions `delegate`, which
makes them the largest mechanically-shaped bucket left by a wide margin. But
that verdict comes from Ghidra's output, and Ghidra is wrong about exactly the
things a generated forwarder depends on: it drops the receiver on `__thiscall`
calls and tail jumps, and it normalises the callee-pop byte count that is the
only ground truth for arity. A generator that trusted it would emit bodies that
compile and are quietly wrong.

So this reads the bytes instead. It accepts a function only when the entire
body matches one of two rigid forms, and reports precisely what the forward
consists of. Anything else - any branch, any store, any arithmetic, any
instruction not in the accepted set - is rejected and named. The point is to be
boring: a shape that is 99% right is worth nothing here, because the whole
value of the bucket is that the emitted body needs no human judgement.

Accepted forms
--------------

`frame-forward`, the common case::

    push ebp / mov ebp, esp
    mov <reg>, [ebp+N]          ; load incoming parameters, any order
    push <reg|imm>              ; arguments, pushed right-to-left
    mov ecx, <imm32>            ; optional fixed receiver: a global singleton
    call <target>
    add esp, N                  ; iff the target is __cdecl
    pop ebp
    ret [K]

`vtable-tail`, the destructor case::

    mov [ecx+off], <imm32>      ; republish one or more vtable pointers
    jmp <target>                ; tail call to the base destructor, this intact

What it reports
---------------

For each accepted function: the target, the receiver (an incoming parameter, a
fixed global, or none), and the argument list as a vector of either
`param:<index>` or `imm:<value>`. That vector is the forwarding contract, and it
is everything a generator needs.

This tool decides nothing about correctness. A proven shape says the body can be
written mechanically; the differential gate remains the only judge of whether
the result matches the original.

Reads proprietary bytes: local analysis only, never committed.
"""

from __future__ import annotations

import argparse
import csv
import json
import sys
from dataclasses import dataclass, field
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pefile  # noqa: E402
from capstone import CS_ARCH_X86, CS_MODE_32, Cs  # noqa: E402

import disasm  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
FUNCTIONS_CSV = REPO_ROOT / "docs" / "recovery" / "functions.csv"

# Registers that may hold a loaded parameter on the way to a push. Anything
# outside this set means the body is doing something a forwarder does not.
SCRATCH = frozenset({"eax", "ecx", "edx", "ebx", "esi", "edi"})


@dataclass
class Argument:
    """One value the forwarder passes on."""
    kind: str            # "param" | "imm"
    value: int           # parameter index, or the literal

    def describe(self) -> str:
        return (f"param:{self.value}" if self.kind == "param"
                else f"imm:0x{self.value & 0xFFFFFFFF:X}")


@dataclass
class Forward:
    address: int
    name: str
    size: int
    form: str                       # "frame-forward" | "vtable-tail"
    target: int
    tail_call: bool                 # jmp rather than call
    receiver: str | None            # "param:0" | "global:0xADDR" | None
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


def parse_frame_slot(operand: str) -> int | None:
    """`dword ptr [ebp + 0xc]` -> parameter index 1."""
    text = operand.replace(" ", "")
    if "[ebp+" not in text:
        return None
    try:
        displacement = int(text.split("[ebp+")[1].rstrip("]"), 0)
    except ValueError:
        return None
    # [ebp+0] is the saved ebp and [ebp+4] the return address; parameters
    # start at [ebp+8] and are int-sized.
    if displacement < 8 or displacement % 4:
        return None
    return (displacement - 8) // 4


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
                receiver="param:0", vtable_writes=writes)
        raise Reject(f"unexpected {mnemonic} {operands}".strip())
    raise Reject("no tail jump")


def analyze_frame_forward(instructions, row) -> Forward:
    """Prologue, parameter loads, pushes, one call, epilogue - nothing else."""
    if len(instructions) < 4:
        raise Reject("too short for a frame forward")
    if (instructions[0].mnemonic, instructions[0].op_str) != ("push", "ebp"):
        raise Reject("no frame prologue")
    if (instructions[1].mnemonic, instructions[1].op_str) != ("mov", "ebp, esp"):
        raise Reject("no frame prologue")

    held: dict[str, Argument] = {}   # register -> what it currently holds
    pushed: list[Argument] = []
    receiver: str | None = None
    target: int | None = None
    tail_call = False
    caller_cleanup = 0
    popped = 0
    saw_epilogue = False

    for instruction in instructions[2:]:
        mnemonic, operands = instruction.mnemonic, instruction.op_str

        if target is None:
            if mnemonic == "mov":
                destination, _, source = operands.partition(", ")
                if destination not in SCRATCH:
                    raise Reject(f"store to {destination!r} before the call")
                slot = parse_frame_slot(source)
                if slot is not None:
                    held[destination] = Argument("param", slot)
                    continue
                if source.startswith("0x"):
                    # A literal into ecx immediately before the call is a fixed
                    # receiver - a global singleton - not an argument.
                    held[destination] = Argument("imm", int(source, 16))
                    continue
                raise Reject(f"mov {destination}, {source} is not a parameter load")
            if mnemonic == "push":
                if operands in SCRATCH:
                    if operands not in held:
                        raise Reject(f"push of undefined {operands}")
                    pushed.append(held[operands])
                    continue
                if operands.startswith("0x") or operands.lstrip("-").isdigit():
                    pushed.append(Argument("imm", int(operands, 0)))
                    continue
                raise Reject(f"push {operands} is neither a held value nor a literal")
            if mnemonic in ("call", "jmp") and operands.startswith("0x"):
                target = int(operands, 16)
                tail_call = mnemonic == "jmp"
                if "ecx" in held and held["ecx"].kind == "imm":
                    receiver = f"global:0x{held['ecx'].value:08X}"
                elif "ecx" in held and held["ecx"].kind == "param":
                    receiver = f"param:{held['ecx'].value}"
                continue
            raise Reject(f"unexpected {mnemonic} {operands}".strip())

        # After the call: cleanup and epilogue only.
        if mnemonic == "add" and operands.startswith("esp, "):
            caller_cleanup = int(operands.split(", ")[1], 0)
            continue
        if mnemonic == "pop" and operands == "ebp":
            saw_epilogue = True
            continue
        if mnemonic == "ret":
            popped = int(operands, 0) if operands else 0
            break
        raise Reject(f"unexpected {mnemonic} {operands} after the call".strip())

    if target is None:
        raise Reject("no call")
    if not saw_epilogue and not tail_call:
        raise Reject("no frame epilogue")

    # Arguments are pushed right-to-left, so the source order is the reverse.
    pushed.reverse()
    # A receiver held in ecx is not one of them.
    return Forward(
        address=int(row["address"], 16), name=row["name"],
        size=int(row["size"]), form="frame-forward", target=target,
        tail_call=tail_call, receiver=receiver, arguments=pushed,
        popped=popped, caller_cleanup=caller_cleanup)


def analyze_global_init(instructions, row) -> Forward:
    """An MSVC dynamic initializer for a global object.

    393 of the remaining functions are these, 283 of them byte-identical in
    shape, which makes it the most regular family left in the image. Four
    variants appear, and all four are the same statement: construct the object
    that lives at a fixed address, then register its destructor to run at exit.

        mov ecx, <global> / call <ctor>                     scalar
        push <arg> / mov ecx, <global> / call <ctor>        scalar with an argument
        push <dtor> <ctor> <count> <size> <array>
            / call <vector ctor iterator>                   array
        [ push <dtor thunk> / call <atexit> / pop ecx ]     optional registration
        ret

    The 10-byte variant tail-jumps to the constructor and registers nothing.
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

        # After the constructor call: at most the atexit registration, then ret.
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
        # Record the registered destructor thunk as the trailing argument, so
        # the contract is complete without a separate field.
        forward.vtable_writes.append((-1, registered))
    return forward


def analyze(pe, row) -> Forward:
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
    return analyze_frame_forward(instructions, row)


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

    accepted: list[Forward] = []
    rejected: list[tuple[str, str]] = []
    for row in load_rows(addresses, args.recovery_state or None):
        try:
            accepted.append(analyze(pe, row))
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
