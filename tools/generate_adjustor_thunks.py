#!/usr/bin/env python3
"""Generate recoveries for the MSVC adjustor-thunk family.

Every class in this image that inherits a virtual base gets one compiler-
emitted `thunkN_Class::method` per overridden virtual: a three-instruction
receiver fixup in front of a tail jump to the real body.

    sub ecx, [ecx - D]      ; subtract the vtordisp stored ahead of `this`
    sub ecx, A              ; optional fixed subobject adjustment
    jmp <target>            ; tail call, arguments untouched

The family is zero-variance. Every catalogued candidate decodes to exactly
that, differing only in D, in whether A is present, and in which method it
forwards to - so nothing here is transcribed by hand. Two values are proven
per thunk rather than assumed: the instruction sequence is matched against the
template byte for byte (anything else is skipped and named), and the emitted
parameter list is checked against the callee-pop count of the jump target,
followed through any intervening `jmp` thunks. Arity is the one thing a wrong
guess would corrupt silently - the thunk pops nothing itself, so its stack
contract is entirely the target's - and it is the check that rejects a
plausible signature before it compiles.

The forward goes through one rebindable seam per DISTINCT target rather than
per thunk: 24 of these thunks share a target with another, and giving each
target a single seam is what lets the generated suite prove that two thunks
onto the same method really do adjust `this` differently.

Emitted, deterministically ordered by address:
  src/adjustor_thunks.h / src/adjustor_thunks.cpp   committed source
  <scratch>/adjustor-thunk-tests.cpp                fragment for the leaf suite
  <scratch>/adjustor-wire.txt                       address/symbol redirect list

Like the other bulk generators this compresses authoring, not verification:
the emitted bodies still go through the leaf suite, the mutation sweep, and
the batch gate exactly as hand-written recoveries do. Each body keeps its own
`Original Offset:` comment and spells D and A as inline literals for that
reason - hoisting them into a shared table would read as tidier and would
leave the sweep with nothing in the body to perturb.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pefile  # noqa: E402

import analyze_delegates as delegates  # noqa: E402
import disasm  # noqa: E402
import generate_atexit_thunks as atexit_gen  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
LICENSE = atexit_gen.LICENSE

# The whole mangled-signature grammar this family uses, keyed by the suffix
# after the class name. Pointer parameters are emitted as `void *`: the thunk
# never dereferences them, `void *` is ABI-identical in the slot, and it keeps
# a generated file free of includes that would only exist to name a type
# nothing here touches. `code` is the compact spelling used to build the
# per-signature typedef and test-table names.
SIGNATURES = {
    "QAEXXZ": ("void", [], "v"),
    "QAEXH@Z": ("void", ["int"], "v_i"),
    "QAEXHH@Z": ("void", ["int", "int"], "v_ii"),
    "QAEHH@Z": ("int", ["int"], "i_i"),
    "QAEHHH@Z": ("int", ["int", "int"], "i_ii"),
    "QAE@H@Z": ("void", ["int"], "v_i"),
    "UAEPAXI@Z": ("void *", ["unsigned int"], "p_u"),
    "QAEHPAUScroll@@@Z": ("int", ["void *"], "i_p"),
    "QAEXPAUBuffer@@H@Z": ("void", ["void *", "int"], "v_pi"),
    "QAEHPAUGraphicWin@@HHH@Z": ("int", ["void *", "int", "int", "int"],
                                 "i_piii"),
}

SUB_VTORDISP = re.compile(r"^ecx, dword ptr \[ecx - (0x[0-9a-f]+|\d+)\]$")
SUB_IMMEDIATE = re.compile(r"^ecx, (0x[0-9a-f]+|\d+)$")

# `?method@thunkN_Class@@sig`, plus the two compiler-generated members that
# spell the class where a method name would go.
THUNK_METHOD = re.compile(r"^\?(\w+)@thunk(\d+)_(\w+)@@(.+)$")
THUNK_DTOR = re.compile(r"^\?\?1thunk(\d+)_(\w+)@@(.+)$")
THUNK_SCALAR_DELETE = re.compile(r"^\?\?_Gthunk(\d+)_(\w+)@@(.+)$")

# Enough arena ahead of the receiver for the largest vtordisp displacement the
# family uses, so the generated suite can read one without running off a
# buffer. Checked against the emitted rows rather than trusted.
TEST_ARENA_LEAD = 2048
TEST_ARENA_TAIL = 256


def snake(name: str) -> str:
    """CamelCase or Camel_Case to snake_case, leaving snake_case alone."""
    text = re.sub(r"(.)([A-Z][a-z]+)", r"\1_\2", name)
    text = re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", text)
    return re.sub(r"__+", "_", text).strip("_").lower()


def declaration(prefix: str, parameters, suffix: str) -> list[str]:
    """A declaration, greedily wrapped under the paren at the column limit."""
    one_line = f"{prefix}({', '.join(parameters)}){suffix}"
    if len(one_line) <= 79:
        return [one_line]
    indent = " " * (len(prefix) + 1)
    lines = [f"{prefix}("]
    for index, parameter in enumerate(parameters):
        piece = parameter + (", " if index + 1 < len(parameters)
                             else f"){suffix}")
        if len(lines[-1]) + len(piece) <= 79 or lines[-1].endswith("("):
            lines[-1] += piece
        else:
            lines.append(indent + piece)
    return [line.rstrip() for line in lines]


def camel(name: str) -> str:
    return "".join(part.capitalize() for part in name.split("_") if part)


def parse_thunk_name(name: str):
    """(class, ordinal, method, signature suffix) or None."""
    match = THUNK_METHOD.match(name)
    if match:
        return match.group(3), int(match.group(2)), match.group(1), \
            match.group(4)
    match = THUNK_DTOR.match(name)
    if match:
        return match.group(2), int(match.group(1)), "dtor", match.group(3)
    match = THUNK_SCALAR_DELETE.match(name)
    if match:
        return match.group(2), int(match.group(1)), "scalar_delete", \
            match.group(3)
    return None


def target_symbol(name: str) -> str:
    """A seam name for a jump target, from its own mangled name."""
    match = re.match(r"^\?(\w+)@(\w+)@@", name)
    if match:
        return f"{match.group(2)}{camel(match.group(1))}Target"
    match = re.match(r"^\?\?1(\w+)@@", name)
    if match:
        return f"{match.group(1)}DtorTarget"
    match = re.match(r"^\?\?_G(\w+)@@", name)
    if match:
        return f"{match.group(1)}ScalarDeleteTarget"
    match = re.match(r"^j_\?\?1(\w+)@@", name)
    if match:
        return f"{match.group(1)}DtorTarget"
    return ""


def decode_raw_template(data: bytes, address: int):
    """(displacement, adjust, target) read straight off the encoding.

    The capstone path below matches on printed mnemonics, which puts a text
    renderer between the bytes and the emitted arithmetic. This decodes the
    same three instructions from the ModRM and immediate bytes instead, and
    collect() requires the two to agree - so a rendering quirk cannot become a
    wrong displacement in committed source. Returns None when the bytes are
    not the template, and lets the capstone path report why.
    """
    offset = 0
    # `sub ecx, [ecx+disp]`: 2B /1 with ecx as both base and destination, in
    # its disp8 (0x49) and disp32 (0x89) forms.
    if data[offset:offset + 2] == b"\x2b\x49":
        displacement = -int.from_bytes(data[offset + 2:offset + 3], "little",
                                       signed=True)
        offset += 3
    elif data[offset:offset + 2] == b"\x2b\x89":
        displacement = -int.from_bytes(data[offset + 2:offset + 6], "little",
                                       signed=True)
        offset += 6
    else:
        return None
    adjust = None
    # `sub ecx, imm32`: 81 /5.
    if data[offset:offset + 2] == b"\x81\xe9":
        adjust = int.from_bytes(data[offset + 2:offset + 6], "little",
                                signed=True)
        offset += 6
    # `jmp rel32`: E9.
    if data[offset:offset + 1] != b"\xe9":
        return None
    relative = int.from_bytes(data[offset + 1:offset + 5], "little",
                              signed=True)
    target = address + offset + 5 + relative
    return displacement, adjust, target & 0xFFFFFFFF


def entry_extent(row):
    """Bytes decodable from the entry address.

    NOT `size`: that column sums every span in `body_ranges`, and 416
    catalogued functions are split across two spans because MSVC parked an EH
    funclet in a separate region. Decoding `size` bytes from the entry of one
    of those runs off the end of the real body and into whatever follows it,
    so a `ret` belonging to the next function can be mistaken for this one's.
    `end_address` is exclusive and always ends the entry span, which is
    exactly the contiguous run a decoder may read. The two agree for every
    unsplit function, so this changes nothing for them."""
    try:
        extent = int(row["end_address"], 16) - int(row["address"], 16)
    except (KeyError, TypeError, ValueError):
        extent = 0
    return extent if extent > 0 else int(row["size"])


def decode_template(pe, row):
    """(displacement, adjust, target) for a conforming thunk, else a reason."""
    instructions = delegates.trailing_padding(
        delegates.decode(pe, int(row["address"], 16), entry_extent(row)))
    if not 2 <= len(instructions) <= 3:
        return f"{len(instructions)} instructions, expected 2 or 3"
    displacement = adjust = target = None
    for index, instruction in enumerate(instructions):
        last = index == len(instructions) - 1
        vtordisp = SUB_VTORDISP.match(instruction.op_str)
        immediate = SUB_IMMEDIATE.match(instruction.op_str)
        if index == 0 and instruction.mnemonic == "sub" and vtordisp:
            displacement = int(vtordisp.group(1), 0)
        elif index == 1 and instruction.mnemonic == "sub" and immediate \
                and not last:
            adjust = int(immediate.group(1), 0)
        elif last and instruction.mnemonic == "jmp" \
                and instruction.op_str.startswith("0x"):
            target = int(instruction.op_str, 16)
        else:
            return f"unexpected `{instruction.mnemonic} {instruction.op_str}`"
    if displacement is None or target is None:
        return "no vtordisp fixup or no tail jump"
    return displacement, adjust, target


def callee_pop(pe, functions, address, hops=4):
    """The target's `ret imm16`, following tail jumps; None if none is found."""
    seen = set()
    while address not in seen and hops > 0:
        seen.add(address)
        hops -= 1
        row = functions.get(address)
        if row is None or not row.get("size"):
            return None
        instructions = delegates.trailing_padding(
            delegates.decode(pe, address, entry_extent(row)))
        if not instructions:
            return None
        # A pure jump thunk forwards the same stack contract; keep walking.
        if len(instructions) == 1 and instructions[0].mnemonic == "jmp" \
                and instructions[0].op_str.startswith("0x"):
            address = int(instructions[0].op_str, 16)
            continue
        pops = {int(i.op_str, 0) if i.op_str else 0
                for i in instructions if i.mnemonic == "ret"}
        if pops:
            # A body that returns two different pop counts is not something to
            # guess about.
            return pops.pop() if len(pops) == 1 else None
        # No `ret` at all: every exit is a tail call, so the stack contract is
        # whatever they forward to. Follow the one direct tail jump. Sibling
        # indirect exits necessarily pop the same, or the shared frame could
        # not be returned through either way.
        last = instructions[-1]
        if last.mnemonic == "jmp" and last.op_str.startswith("0x"):
            address = int(last.op_str, 16)
            continue
        return None
    return None


def collect(pe, functions):
    """(rows, leftovers): every catalogued adjustor thunk, proven and sorted."""
    rows = []
    leftovers = []
    # All states, not just unrecovered: regeneration must keep re-emitting the
    # thunks earlier runs recovered, or rerunning this after a new recovery
    # would silently drop them from the committed files.
    for row in delegates.load_rows(None, None):
        parsed = parse_thunk_name(row["name"])
        if parsed is None:
            continue
        class_name, ordinal, method, suffix = parsed
        address = int(row["address"], 16)
        if row["recovery_state"] not in ("unrecovered", "source_complete"):
            leftovers.append((row["address"], row["name"],
                              f"state {row['recovery_state']}"))
            continue
        decoded = decode_template(pe, row)
        if isinstance(decoded, str):
            leftovers.append((row["address"], row["name"], decoded))
            continue
        displacement, adjust, target = decoded
        raw = decode_raw_template(
            disasm.read_range(pe, address, entry_extent(row)), address)
        if raw != decoded:
            raise SystemExit(
                f"{row['name']}: capstone reads {decoded}, the encoding "
                f"reads {raw}; refusing to emit either")
        if suffix not in SIGNATURES:
            leftovers.append((row["address"], row["name"],
                              f"unmodelled signature {suffix}"))
            continue
        target_row = functions.get(target)
        if target_row is None:
            leftovers.append((row["address"], row["name"],
                              f"target 0x{target:08X} not catalogued"))
            continue
        seam = target_symbol(target_row["name"])
        if not seam:
            leftovers.append((row["address"], row["name"],
                              f"cannot name target {target_row['name']}"))
            continue
        returns, parameters, code = SIGNATURES[suffix]
        pop = callee_pop(pe, functions, target)
        # The thunk pops nothing itself, so its stack contract is the
        # target's. A mismatch means the modelled parameter list is wrong and
        # the redirect would corrupt the caller's stack.
        if pop is None:
            leftovers.append((row["address"], row["name"],
                              "target callee-pop not determinable"))
            continue
        if pop != 4 * len(parameters):
            leftovers.append(
                (row["address"], row["name"],
                 f"target pops {pop}, signature wants {4 * len(parameters)}"))
            continue
        rows.append({
            "address": address,
            "name": row["name"],
            "class_name": class_name,
            "ordinal": ordinal,
            "method": method,
            "symbol": f"adjust_{snake(class_name)}{ordinal}_{snake(method)}",
            "displacement": displacement,
            "adjust": adjust,
            "target": target,
            "target_name": target_row["name"],
            "seam": seam,
            "returns": returns,
            "parameters": parameters,
            "code": code,
        })
    rows.sort(key=lambda entry: entry["address"])
    return rows, leftovers


def seam_table(rows):
    """{seam: (address, code)}, rejecting one target seen at two signatures."""
    seams = {}
    for row in rows:
        existing = seams.get(row["seam"])
        entry = (row["target"], row["code"])
        if existing is not None and existing != entry:
            raise SystemExit(
                f"{row['seam']}: bound to 0x{existing[0]:08X}/{existing[1]} "
                f"and 0x{row['target']:08X}/{row['code']}")
        seams[row["seam"]] = entry
    return dict(sorted(seams.items()))


def typedef_name(code: str) -> str:
    return f"func_adjustor_{code}"


def signature_of(row):
    """(parameter declarations, argument names) for an emitted thunk."""
    declarations = [f"{kind}arg{index}" if kind.endswith("*")
                    else f"{kind} arg{index}"
                    for index, kind in enumerate(row["parameters"])]
    arguments = [f"arg{index}" for index in range(len(row["parameters"]))]
    return declarations, arguments


def used_codes(rows):
    codes = {}
    for row in rows:
        codes[row["code"]] = row
    return dict(sorted(codes.items()))


def render_header(rows, seams):
    out = [LICENSE.rstrip("\n"), "", "#pragma once", "", '''/*
 * Virtual-base adjustor thunks
 *
 * One compiler-emitted thunk per virtual a class overrides through a virtual
 * base. Each subtracts the vtordisp stored at a fixed negative displacement
 * from `this`, optionally subtracts a further fixed subobject offset, and tail
 * calls the real method with the arguments untouched. The receiver arithmetic
 * is the whole of the behaviour, so it is what the suite pins.
 *
 * The forward goes through one rebindable seam per distinct target rather
 * than one per thunk: several of these thunks share a target, and a single
 * seam per target is what lets the suite prove that two thunks onto the same
 * method adjust `this` differently. Each seam defaults to the original
 * address; the hybrid redirects those to recovered bodies at run time.
 *
 * Generated by tools/generate_adjustor_thunks.py - do not edit by hand.
 */''', ""]
    for code, row in used_codes(rows).items():
        parameters = ", ".join(["void *"] + row["parameters"])
        out.append(f"typedef {row['returns']}(__thiscall "
                   f"{typedef_name(code)})({parameters});")
    out.append("")
    for seam, (address, code) in seams.items():
        out.append(f"extern {typedef_name(code)} *{seam};"
                   f"   // 0x{address:08X}")
    out.append("")
    for row in rows:
        declarations, _ = signature_of(row)
        out.extend(declaration(
            f"{row['returns']} __fastcall {row['symbol']}",
            ["void *self", "void *"] + declarations, ";"))
    out.append("")
    return "\n".join(out)


def render_source(rows, seams):
    out = [LICENSE.rstrip("\n"), "", '#include "stdafx.h"',
           '#include "adjustor_thunks.h"', "", '''/*
Verification note: the generated leaf suite proves that each body reads its
vtordisp from the displacement recorded for it, subtracts the adjustment
recorded for it, and forwards its arguments and its result untouched. It does
NOT independently confirm that `this - vtordisp - adjust` is what the original
computed, because the suite's expectation is derived from the same table the
bodies are. What pins the model is the acceptance check in
tools/generate_adjustor_thunks.py: a thunk is emitted only when its bytes are
exactly `sub ecx, [ecx - D]` / optional `sub ecx, A` / `jmp target`, and only
when the target's own callee-pop count agrees with the emitted parameter list.
A three-instruction body matched instruction for instruction leaves nothing
between the bytes and the transcription for a leaf fixture to catch.

The check that would close the remaining gap is a differential oracle running
each original thunk against its recovered twin with the target patched to
record the receiver. That needs the hybrid process, not the leaf suite, and it
is not built here.
*/''', ""]
    out.append("// Each seam defaults to the original method the thunk tail")
    out.append("// jumps to; the hybrid redirects it to the recovered body at")
    out.append("// run time, and the leaf suite rebinds it to a recorder.")
    for seam, (address, code) in seams.items():
        out.append(f"{typedef_name(code)} *{seam} =")
        out.append(f"    ({typedef_name(code)} *)0x{address:08X};")
    out.append("")
    for row in rows:
        declarations, arguments = signature_of(row)
        subject = f"{row['class_name']}::{row['method']}"
        out.append("/*")
        out.append(f"Purpose: Adjust the receiver from {row['class_name']}'s "
                   f"thunk{row['ordinal']} subobject to the one")
        out.append(f"         {subject} expects, then forward unchanged.")
        out.append(f"Original Offset: {row['address']:08X}")
        out.append("Return Value: n/a" if row["returns"] == "void"
                   else "Return Value: the forwarded call's")
        out.append("Status: Complete")
        out.append("*/")
        out.extend(declaration(
            f"{row['returns']} __fastcall {row['symbol']}",
            ["void *self", "void *"] + declarations, " {"))
        out.append("    uint8_t *const object = static_cast<uint8_t *>(self);")
        out.append("    const int32_t vtordisp =")
        out.append("        *reinterpret_cast<const int32_t *>(object - "
                   f"{row['displacement']});")
        receiver = "object - vtordisp"
        if row["adjust"] is not None:
            receiver += f" - {row['adjust']}"
        call = f"{row['seam']}({receiver}"
        call += ("".join(f", {name}" for name in arguments)) + ");"
        keyword = "    " if row["returns"] == "void" else "    return "
        out.append(f"{keyword}{call}")
        out.append("}")
        out.append("")
    return "\n".join(out)


def render_tests(rows, seams):
    lead = max(row["displacement"] for row in rows)
    if lead >= TEST_ARENA_LEAD:
        raise SystemExit(f"displacement {lead} exceeds the test arena lead")
    out = ["namespace {", "",
           "// The receiver sits inside a poisoned arena with room ahead of it",
           "// for the largest vtordisp displacement the family uses. Every",
           "// four-byte window in the arena holds a distinct value, so a body",
           "// that reads its vtordisp from the wrong displacement cannot",
           "// happen to read an equal one. The adjusted receiver is only ever",
           "// compared, never dereferenced, so it may land anywhere.",
           f"constexpr size_t AdjustorArenaLead = {TEST_ARENA_LEAD};",
           f"constexpr size_t AdjustorArenaTail = {TEST_ARENA_TAIL};",
           "uint8_t g_adjustor_arena[AdjustorArenaLead + AdjustorArenaTail];",
           "uint8_t *adjustor_receiver() {",
           "    for (size_t index = 0; index < sizeof(g_adjustor_arena);"
           " ++index) {",
           "        g_adjustor_arena[index] = static_cast<uint8_t>("
           "index * 7u + 1u);",
           "    }",
           "    return g_adjustor_arena + AdjustorArenaLead;",
           "}",
           "",
           "void *g_adjustor_seen;",
           "int g_adjustor_calls;",
           "int g_adjustor_args[4];",
           "",
           "#if defined(__GNUC__)",
           "#pragma GCC diagnostic push",
           '#pragma GCC diagnostic ignored "-Wattributes"',
           "#endif"]
    for code, row in used_codes(rows).items():
        declarations, arguments = signature_of(row)
        returns = row["returns"]
        out.extend(declaration(f"{returns} __thiscall observe_adjustor_{code}",
                               ["void *self"] + declarations, " {"))
        out.append("    g_adjustor_seen = self;")
        out.append("    ++g_adjustor_calls;")
        for index, name in enumerate(arguments):
            value = (f"reinterpret_cast<intptr_t>({name})"
                     if row["parameters"][index] == "void *" else name)
            out.append(f"    g_adjustor_args[{index}] = "
                       f"static_cast<int>({value});")
        if returns == "int":
            out.append("    return 0x5A5A;")
        elif returns == "void *":
            out.append("    return &g_adjustor_calls;")
        out.append("}")
    out.append("#if defined(__GNUC__)")
    out.append("#pragma GCC diagnostic pop")
    out.append("#endif")
    out.append("")

    by_code = {}
    for row in rows:
        by_code.setdefault(row["code"], []).append(row)
    for code, entries in sorted(by_code.items()):
        sample = entries[0]
        out.append(f"struct AdjustorCase_{code} {{")
        out.extend("    " + line for line in declaration(
            f"{sample['returns']} (__fastcall *thunk)",
            ["void *", "void *"] + sample["parameters"], ";"))
        out.append(f"    {typedef_name(code)} **slot;")
        out.append("    int displacement;")
        out.append("    int adjust;")
        out.append("};")
        out.append(f"const AdjustorCase_{code} g_adjustor_cases_{code}[] = {{")
        for row in entries:
            adjust = 0 if row["adjust"] is None else row["adjust"]
            entry = (f"    {{&{row['symbol']}, &{row['seam']}, "
                     f"{row['displacement']}, {adjust}}},")
            if len(entry) <= 79:
                out.append(entry)
            else:
                out.append(f"    {{&{row['symbol']},")
                out.append(f"     &{row['seam']}, {row['displacement']}, "
                           f"{adjust}}},")
        out.append("};")
        out.append("")
    out.append("}  // namespace")
    out.append("")
    out.append("void test_adjustor_thunks() {")
    for code, entries in sorted(by_code.items()):
        sample = entries[0]
        _, arguments = signature_of(sample)
        literals = []
        for index, kind in enumerate(sample["parameters"]):
            literal = f"0x1{index}1{index}"
            literals.append(f"reinterpret_cast<void *>({literal})"
                            if kind == "void *" else literal)
        out.append(f"    for (const AdjustorCase_{code} &entry :"
                   f" g_adjustor_cases_{code}) {{")
        out.append(f"        {typedef_name(code)} *const saved = *entry.slot;")
        out.append(f"        *entry.slot = &observe_adjustor_{code};")
        out.append("        uint8_t *const self = adjustor_receiver();")
        out.append("        const int32_t vtordisp =")
        out.append("            *reinterpret_cast<const int32_t *>("
                   "self - entry.displacement);")
        out.append("        g_adjustor_calls = 0;")
        out.append("        g_adjustor_seen = nullptr;")
        # The return value is carried back too, so a body that forwarded but
        # dropped the result would not pass.
        returns = sample["returns"]
        prefix = ("        entry.thunk" if returns == "void"
                  else f"        {returns} const result = entry.thunk")
        out.extend(declaration(prefix, ["self", "nullptr"] + literals, ";"))
        out.append("        expect(g_adjustor_calls == 1);")
        out.append("        expect(g_adjustor_seen == "
                   "self - vtordisp - entry.adjust);")
        if returns == "int":
            out.append("        expect(result == 0x5A5A);")
        elif returns == "void *":
            out.append("        expect(result == &g_adjustor_calls);")
        for index, kind in enumerate(sample["parameters"]):
            literal = f"0x1{index}1{index}"
            out.append(f"        expect(g_adjustor_args[{index}] == "
                       f"{literal});")
        out.append("        *entry.slot = saved;")
        out.append("    }")
    out.append("}")
    out.append("")
    return "\n".join(out)


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--exe", type=Path, default=disasm.DEFAULT_EXE)
    parser.add_argument("--scratch-dir", type=Path, required=True)
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    pe = pefile.PE(str(args.exe), fast_load=True)
    functions = {int(row["address"], 16): row
                 for row in delegates.load_rows(None, None)}

    rows, leftovers = collect(pe, functions)
    if not rows:
        raise SystemExit("no adjustor thunks accepted")
    seams = seam_table(rows)

    symbols = [row["symbol"] for row in rows]
    if len(set(symbols)) != len(symbols):
        duplicates = sorted({s for s in symbols if symbols.count(s) > 1})
        raise SystemExit(f"duplicate symbols; refusing to emit: {duplicates}")

    shapes = {}
    for row in rows:
        shapes.setdefault((row["displacement"], row["adjust"]), []).append(row)
    print(f"thunks accepted: {len(rows)} over {len(seams)} target seams")
    for (displacement, adjust), entries in sorted(
            shapes.items(), key=lambda item: -len(item[1])):
        label = f"[ecx-{displacement}]" + (f" -{adjust}" if adjust else "")
        print(f"  {label:20} {len(entries):4}")
    if leftovers:
        print(f"left alone: {len(leftovers)}")
        for address, name, reason in leftovers[:12]:
            print(f"  {address} {name[:44]:46} {reason[:44]}")

    if args.dry_run:
        return 0

    (REPO_ROOT / "src" / "adjustor_thunks.h").write_text(
        render_header(rows, seams))
    (REPO_ROOT / "src" / "adjustor_thunks.cpp").write_text(
        render_source(rows, seams))
    args.scratch_dir.mkdir(parents=True, exist_ok=True)
    (args.scratch_dir / "adjustor-thunk-tests.cpp").write_text(
        render_tests(rows, seams))
    wire = "\n".join(f"0x{row['address']:08X} {row['symbol']}"
                     for row in rows) + "\n"
    (args.scratch_dir / "adjustor-wire.txt").write_text(wire)
    print("emitted src/adjustor_thunks.{h,cpp}, test fragment, wire list")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
