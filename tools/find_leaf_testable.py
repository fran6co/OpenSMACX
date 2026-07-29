#!/usr/bin/env python3
"""List unrecovered functions that can be recovered AND verified as leaves.

The queue this produces is the answer to a question that cost a session to ask
properly: which functions are cheap to RECOVER is not the same as which are
cheap to VERIFY, and only the second one bounds the work.

`BattleWin::stop_timer` is the case that established it. Eight bytes,
`add ecx, 8` / `jmp Time::stop`, its offset already pinned by the recovered
destructor beside it, and its callee already `source_complete`. It still could
not be landed: `Time::stop` lives in `src/time.cpp` with `init`/`start`/`pulse`,
which reach fixed-address bindings into the original executable
(`HandleMain` at 0x009B7B28, `MsgStatus` at 0x009B7B9C), and that translation
unit is not linked into `recovery-leaf-tests`. Per the working rules a function
reaching absolute globals has to be exercised at its original address inside
the verified hybrid process, so an eight-byte body turned out to cost a staged
hybrid run.

So a candidate here must satisfy THREE conditions, and the third is the one
that is easy to forget:

  1. its own body touches no absolute global inside the image, makes no call
     this tool cannot account for, and is a FUNCTION rather than an unwind
     funclet;
  2. every function it calls is already `source_complete`;
  3. every function it calls has its RECOVERED IMPLEMENTATION in a translation
     unit compiled into `recovery-leaf-tests`.

(3) is answered by finding where each callee's implementation actually LIVES -
the `Original Offset:` comment that tools/mutate_and_verify.py already uses to
locate function bodies - and asking whether that file is compiled into the
target. The first version guessed the file from the mangled name, mapping
`?f@Class@@` to `src/class.cpp`, which worked only for class methods and
excluded every free function in the image: 349 of the 657 rejected candidates
under 64 bytes carried "no class in the mangled name", the largest single
reason, and none of them was rejected for a property of the code.

The file list is read out of THE BUILD - the target's own object directory -
rather than out of CMakeLists.txt. Parsing the source list from CMake was the first
attempt and it silently over-matched, collecting 122 sources where the target
has 100; the one false entry was `src/time.cpp`, which made this tool accept
BattleWin::stop_timer, the very function whose failure to link is why the tool
exists. A configured build is therefore a precondition.

The output is a work queue and a COUNT. The count is the point: it is what
makes "exhaust this closure" a goal that can be finished rather than a
direction to walk in, and it grows as recoveries land - each one can make its
callers eligible - so a falling count is real progress and a rising one is
information rather than a bug.

Usage:
    find_leaf_testable.py [--exe ...] [--functions ...] [--show-rejected]
"""

from __future__ import annotations

import argparse
import csv
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pefile  # noqa: E402
from find_constant_returns import declared_arity  # noqa: E402
from capstone import (CS_ARCH_X86, CS_GRP_CALL, CS_GRP_JUMP,  # noqa: E402
                      CS_MODE_32, Cs)
from capstone.x86 import (X86_OP_IMM, X86_OP_MEM, X86_OP_REG,  # noqa: E402
                          X86_REG_EBP)

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
FUNCTIONS_CSV = REPO_ROOT / "docs" / "recovery" / "functions.csv"

# A jump redirect writes five bytes over the target, so a body with less slack
# than that cannot be patched at all - the same rule find_constant_returns.py
# applies, and for the same reason.
REDIRECT_BYTES = 5


def leaf_test_sources(build_dir: Path) -> set[str]:
    """The src/*.cpp files actually compiled into recovery-leaf-tests.

    Taken from the BUILD, not from CMakeLists.txt, and that is the whole point.
    Parsing the target's source list out of CMake looked right and was wrong:
    the regex ran past the end of add_executable() and collected 122 sources
    where the target has 100, which let `src/time.cpp` in. That single false
    entry made the scanner accept BattleWin::stop_timer - the exact function
    whose failure to link is the reason this tool exists.

    The object directory cannot drift from the target, because ninja wrote it.
    A configured build is therefore a precondition, and saying so is better
    than guessing from source text that only resembles the answer.
    """
    for preset in ("mingw-i686-release", "mingw-i686-debug"):
        objects = build_dir / preset / "CMakeFiles" / "recovery-leaf-tests.dir" / "src"
        if objects.is_dir():
            found = {path.name[:-len(".cpp.obj")]
                     for path in objects.glob("*.cpp.obj")}
            if found:
                return found
    raise SystemExit(
        "no recovery-leaf-tests object directory under build/; configure and "
        "build a preset first - this tool reads the target's real source list "
        "from the build rather than parsing CMakeLists.txt, because a parse "
        "that over-matches silently accepts functions that cannot link")


ORIGINAL_OFFSET = re.compile(r"Original Offset:\s*([0-9A-Fa-f]{6,8})")


def implementation_files(source_dir: Path) -> dict[int, str]:
    """address -> the stem of the .cpp its recovered body lives in.

    Read from the `Original Offset:` comments the recovery convention already
    requires, which is the same anchor tools/mutate_and_verify.py locates
    bodies by. This replaces guessing a filename from the mangled name: it is
    correct for free functions, correct for classes whose file is not named
    after them - Time's recovered half lives in time_recovery.cpp - and it
    cannot drift, because a body that moved took its comment with it.
    """
    found: dict[int, str] = {}
    for path in sorted(source_dir.glob("*.cpp")):
        for match in ORIGINAL_OFFSET.finditer(path.read_text(encoding="utf-8",
                                                             errors="replace")):
            found[int(match.group(1), 16)] = path.stem
    return found


def load_rows(functions_csv: Path) -> list[dict]:
    with functions_csv.open() as handle:
        return list(csv.DictReader(handle))


def body_bytes(pe: pefile.PE, address: int, length: int) -> bytes:
    base = pe.OPTIONAL_HEADER.ImageBase
    for section in pe.sections:
        begin = base + section.VirtualAddress
        end = begin + max(section.Misc_VirtualSize, section.SizeOfRawData)
        if begin <= address < end:
            offset = section.PointerToRawData + (address - begin)
            return pe.__data__[offset:offset + length]
    return b""


def image_span(pe: pefile.PE) -> tuple[int, int]:
    base = pe.OPTIONAL_HEADER.ImageBase
    end = base
    for section in pe.sections:
        end = max(end, base + section.VirtualAddress
                  + max(section.Misc_VirtualSize, section.SizeOfRawData))
    return base, end


def inherits_a_frame(instructions) -> bool:
    """True when the body reads EBP without establishing its own frame.

    MSVC emits EH unwind funclets into the cold 0x0065xxxx region as bodies
    that begin `mov ecx,[ebp-0x10]` and tail-jump into a destructor. They have
    no prologue: EBP belongs to the frame being unwound, and the funclet is
    entered by the runtime with that frame still live. They are not callable on
    their own, so recovering one produces a function that reads whatever
    happens to be below the caller's EBP.

    The catalogue lists them as ordinary functions and they pass every other
    condition here - no globals, one direct tail call to a source_complete
    destructor - so nothing else rejects them. Fifteen were in the first forty
    candidates of the enlarged closure.
    """
    for one in instructions:
        if one.mnemonic == "push" and one.op_str == "ebp":
            return False            # establishes its own frame
        for operand in one.operands:
            if (operand.type == X86_OP_MEM
                    and operand.mem.base == X86_REG_EBP):
                return True         # reads a frame it never set up
    return False


# Instructions a recovered body could not contain. Privileged or
# machine-state-only: there is no C++ that means them, and inline assembly
# would fault in user mode rather than work.
INEXPRESSIBLE = frozenset({
    "out", "outsb", "outsw", "outsd", "in", "insb", "insw", "insd",
    "cli", "sti", "hlt", "iret", "iretd", "clts", "invd", "wbinvd",
    "lgdt", "lidt", "lldt", "ltr", "lmsw", "rdmsr", "wrmsr", "arpl",
})


def propagate_image_taint(one, tainted, low, high) -> None:
    """Track which registers currently hold an address inside the image.

    Deliberately small. An image address enters a register as an immediate and
    spreads only by being copied or offset: `mov edx,0x94cea0 / lea
    eax,[edx-0x488]` is how ?clear_monuments@@YAXXZ reaches its table, so `lea`
    has to carry it. Anything else that writes a register CLEARS it - this
    would rather stop tracking than guess, since a false taint rejects a
    recoverable function.
    """
    operands = one.operands
    if not operands or operands[0].type != X86_OP_REG:
        return
    destination = operands[0].reg
    if one.mnemonic in ("mov", "lea", "add") and len(operands) == 2:
        source = operands[1]
        if source.type == X86_OP_IMM and low <= source.imm < high:
            tainted.add(destination)
            return
        if source.type == X86_OP_REG and source.reg in tainted:
            tainted.add(destination)
            return
        if (source.type == X86_OP_MEM
                and (source.mem.base in tainted
                     or source.mem.index in tainted)):
            tainted.add(destination)
            return
        if one.mnemonic == "add" and destination in tainted:
            return          # offsetting an image address keeps it one
    tainted.discard(destination)


def classify(instructions, address: int, size: int, span: tuple[int, int]):
    """(callees, reasons, bindings) for one body.

    `bindings` is separate from `reasons` on purpose. An absolute IMMEDIATE -
    `cmp ecx, 0x9156b0`, comparing `this` against a known global object - does
    not disqualify a function from being leaf-tested, because nothing is
    dereferenced and nothing can fault. But recovering it hardcodes an address
    of the original image, which is a fixed data binding, and the release rule
    requires zero of those. So it stays in the queue and is FLAGGED: it needs a
    row in docs/recovery-binding-classifications.csv, and whoever picks it up
    should know that before writing it rather than at the gate.

    An absolute memory operand is different and is a rejection: it is a read or
    write through an address that is simply not mapped in a standalone test
    executable.
    """
    low, high = span
    callees: set[int] = set()
    reasons: list[str] = []
    bindings: list[int] = []
    inside = range(address, address + size)
    # Registers currently holding an address INSIDE the image. An absolute
    # address does not have to appear as a memory operand's displacement to be
    # an absolute global: `mov edi,0x90db24 / rep stosd [edi]` writes eleven
    # dwords to one, and the displacement check never sees it because the
    # operand has a base register. Seven candidates reached image memory that
    # way and were being reported as merely "needs classification".
    tainted: set[int] = set()
    for one in instructions:
        mnemonic = one.mnemonic
        operands = one.operands
        if mnemonic in INEXPRESSIBLE:
            # Nothing recovers this. `?set_palette@...` at 0x005d4240 uploads a
            # VGA palette with `cli`, a loop of `out dx,al` to ports 0x3c8 and
            # 0x3c9, then `sti` - privileged port I/O with no C++ expression at
            # all, which would fault in user mode even if one were written.
            #
            # The other three conditions are about whether a body can be TESTED
            # in isolation; this one is about whether it can be WRITTEN. A
            # candidate that cannot be is not work, and leaving it in the queue
            # makes the count a target that can never reach zero.
            reasons.append(f"no C++ expression for `{mnemonic}`")
            continue
        # An absolute memory operand is a global. `opensmacx_at` aside, a
        # recovered body reaching one is a fixed-address binding, which is the
        # thing that puts a function in the hybrid oracle rather than here.
        # A memory operand reached through a tainted register is an absolute
        # global, whatever its displacement says.
        for operand in operands:
            if operand.type != X86_OP_MEM:
                continue
            for register in (operand.mem.base, operand.mem.index):
                if register and register in tainted:
                    reasons.append(
                        "absolute global reached through a register")
                    break

        for operand in operands:
            # No BASE register and a displacement inside the image is an
            # absolute address, whether or not an index is scaled onto it.
            # Requiring index == 0 as well missed exactly the shape that
            # matters most: Dialog::set_def_dialog_text_color writes four
            # global arrays as `[eax*4 + 0x6970ac]`, and the scanner offered it
            # as leaf-testable. A fixture would have written through an
            # unmapped address rather than failing an assertion.
            if (operand.type == X86_OP_MEM and operand.mem.base == 0
                    and low <= operand.mem.disp < high):
                reasons.append(f"absolute global {operand.mem.disp:#010x}")
            # An address used as a VALUE, not dereferenced. Testable, but it is
            # a fixed data binding and has to be declared as one.
            #
            # Control-flow targets are excluded by GROUP rather than by
            # mnemonic. Excluding just "call" and "jmp" by name left every
            # conditional branch in: `je 0x405049` inside a function starting
            # at 0x405020 was reported as a fixed-address binding, which is a
            # label. Four of eighteen queue entries carried that noise, and a
            # flag that cries wolf is worse than no flag.
            if (operand.type == X86_OP_IMM
                    and not one.group(CS_GRP_JUMP)
                    and not one.group(CS_GRP_CALL)
                    and low <= operand.imm < high):
                bindings.append(operand.imm)
        if mnemonic == "call":
            if len(operands) == 1 and operands[0].type == X86_OP_IMM:
                callees.add(operands[0].imm)
            else:
                reasons.append("indirect call")
        elif mnemonic == "jmp":
            if len(operands) == 1 and operands[0].type == X86_OP_IMM:
                if operands[0].imm not in inside:
                    callees.add(operands[0].imm)
            else:
                # `jmp [eax+0xE8]` is a virtual dispatch: the callee is not
                # knowable statically, so condition (2) cannot be decided.
                reasons.append("indirect jump")
        propagate_image_taint(one, tainted, low, high)
    return callees, reasons, bindings


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--functions", type=Path, default=FUNCTIONS_CSV)
    parser.add_argument("--build-dir", type=Path,
                        default=REPO_ROOT / "build")
    parser.add_argument("--max-size", type=int, default=64,
                        help="skip bodies larger than this many bytes")
    parser.add_argument("--show-rejected", action="store_true")
    args = parser.parse_args(argv)

    sources = leaf_test_sources(args.build_dir)

    rows = load_rows(args.functions)
    state = {int(row["address"], 16): row["recovery_state"] for row in rows}
    names = {int(row["address"], 16): row["name"] for row in rows}
    ordered = sorted(state)

    pe = pefile.PE(str(args.exe), fast_load=True)
    span = image_span(pe)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True

    homes = implementation_files(REPO_ROOT / "src")

    accepted, rejected = [], []
    for row in rows:
        if row["recovery_state"] != "unrecovered":
            continue
        size = int(row["size"] or 0)
        if not size or size > args.max_size:
            continue
        address = int(row["address"], 16)
        why: list[str] = []
        # No requirement on the CANDIDATE's own home: it has none yet, and its
        # implementation can be written into any translation unit the target
        # already compiles. The constraint is on its CALLEES, below.

        data = body_bytes(pe, address, size)
        if len(data) < size:
            why.append("body not readable from the image")
            rejected.append((address, row["name"], why))
            continue
        instructions = list(decoder.disasm(data, address))
        if sum(one.size for one in instructions) < size:
            why.append("does not fully decode")
        callees, reasons, bindings = classify(instructions, address, size, span)
        why.extend(reasons)
        if inherits_a_frame(instructions):
            why.append("reads EBP without a prologue - an unwind funclet, not "
                       "an independently callable function")

        # THE ARITY MUST AGREE WITH THE STACK CLEANUP, and this is the check
        # find_constant_returns.py already applies for the same reason.
        #
        # A recovered body replaces the original through a jump patch, so a
        # declaration that pops a different number of bytes than the original
        # corrupts its CALLER - the failure the working rules call the worst in
        # this project, because the crash lands somewhere unrelated.
        #
        # ?on_redraw@Win@@QAEHXZ is the case: the name declares no parameters
        # and the body is `xor eax,eax / ret 8`. Every other on_redraw in the
        # image is QAEXXZ, and InfoWin's is a one-byte `ret` popping nothing,
        # so it cannot even share a vtable slot with this one. There are no
        # direct callers to settle it from - it is reached only virtually - so
        # either the catalogued name or the body is wrong and nothing here can
        # say which. Declaring it either way is a guess that a passing test
        # would hide.
        arity = declared_arity(row["name"])
        if arity is not None and instructions:
            last = instructions[-1]
            if last.mnemonic == "ret":
                popped = (last.operands[0].imm
                          if last.operands and last.operands[0].type == X86_OP_IMM
                          else 0)
                if popped != arity[1]:
                    why.append(f"declares {arity[1]} bytes of arguments and "
                               f"cleans {popped}")

        # The patch slack rule: a five-byte jump has to fit.
        position = ordered.index(address) if address in state else -1
        if 0 <= position < len(ordered) - 1:
            slack = ordered[position + 1] - address
            if slack < REDIRECT_BYTES:
                why.append(f"only {slack}B of slack")

        for callee in sorted(callees):
            if state.get(callee) != "source_complete":
                why.append(f"callee {callee:#010x} is "
                           f"{state.get(callee, 'uncatalogued')}")
                continue
            home = homes.get(callee)
            if home is None:
                why.append(f"callee {names.get(callee, hex(callee))} is "
                           f"source_complete but no Original Offset comment "
                           f"says where it lives")
            elif home not in sources:
                # Condition (3), and the one BattleWin::stop_timer failed:
                # Time::stop's body is in time.cpp, which the target does not
                # compile, even though time_recovery.cpp is.
                why.append(f"callee {names.get(callee, hex(callee))} lives in "
                           f"src/{home}.cpp, not compiled into "
                           f"recovery-leaf-tests")

        if why:
            rejected.append((address, row["name"], why))
        else:
            accepted.append((size, address, row["name"], bindings))

    accepted.sort()
    print(f"{len(accepted)} leaf-testable candidates "
          f"({sum(size for size, _, _, _ in accepted)} bytes)\n")
    for size, address, name, bindings in accepted:
        note = ""
        if bindings:
            note = ("   [fixed-address binding: "
                    + ", ".join(f"{one:#010x}" for one in sorted(set(bindings)))
                    + " - needs classification]")
        print(f"  {address:#010x} {size:4d} B  {name}{note}")
    if args.show_rejected:
        print(f"\n{len(rejected)} rejected:")
        for address, name, why in sorted(rejected):
            print(f"  {address:#010x} {name[:52]}")
            for reason in dict.fromkeys(why):
                print(f"        {reason}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
