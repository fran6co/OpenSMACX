"""Bounded source and pinned/native witnesses for Sound type dispatch.

The native checker consumes an already compiled COFF object. It verifies the
compiler-emitted table and its section-local destinations; it does not widen
the historical 128-byte function extent or claim linked-game acceptance.
"""
from __future__ import annotations

import hashlib
import os
from pathlib import Path
import resource
import shutil
import subprocess
import tempfile

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32, CS_GRP_JUMP
from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_OP_REG
from capstone.x86_const import X86_REG_EAX, X86_REG_EBP, X86_REG_ECX, X86_REG_EDX, X86_REG_ESP

from match_recovery.binary import RelocationKind
from match_recovery.compiler import Compiler
from match_recovery.measurement import Measurement
from match_recovery.model import Span
from match_recovery.pe_coff import Coff, PeImage
from test_spritelist_base_lifetime import extract_definition, repo_root


ROOT = repo_root()
GAME_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
GAME = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
METHOD_START = 0x004C61E0
METHOD_END = 0x004C6260
TABLE_START = 0x004C6260
TABLE_END = 0x004C627C
NATIVE_METHOD = "?set_type@Sound@@UAEXI@Z"
TYPE_EFFECTS = (0x04, 0x08, None, 0x10, 0x28, 0x100, 0x80)
PINNED_TARGETS = (0x004C6215, 0x004C6205, 0x004C6255, 0x004C61F5,
                  0x004C6225, 0x004C6235, 0x004C6245)


def _source_method() -> str:
    return extract_definition((ROOT / "src/sound_type.cpp").read_text(),
                              "void Sound::set_type(unsigned int type)")


def _host_program(method: str) -> str:
    return r'''#include <assert.h>
typedef unsigned int uint32_t;
class Sound {
public:
    uint32_t flags_40_;
    uint32_t type_;
    void set_type(unsigned int type);
};
''' + method + r'''
static unsigned int effect_for(unsigned int type) {
    switch (type) {
    case 1: return 0x04;
    case 2: return 0x08;
    case 4: return 0x10;
    case 5: return 0x28;
    case 6: return 0x100;
    case 7: return 0x80;
    default: return 0;
    }
}
static unsigned int stored_type(unsigned int type) {
    return (type == 1 || type == 2 || (type >= 4 && type <= 7)) ? type : 0;
}
int main() {
    const unsigned int inputs[] = { 0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 0xffffffffu };
    const unsigned int initial[] = { 0x80000001u, 0x00008400u, 0x7fffffffu,
                                     0x40010000u, 0x00000003u };
    for (unsigned int pass = 0; pass < sizeof(initial) / sizeof(initial[0]); ++pass) {
        for (unsigned int i = 0; i < sizeof(inputs) / sizeof(inputs[0]); ++i) {
            Sound sound;
            const unsigned int input = inputs[i];
            const unsigned int before = initial[pass];
            sound.flags_40_ = before;
            sound.type_ = 0xdeadbeefu;
            sound.set_type(input);
            assert(sound.type_ == stored_type(input));
            assert(sound.flags_40_ == (before | effect_for(input)));
        }
    }
    Sound repeated;
    repeated.flags_40_ = 0x40000000u;
    repeated.type_ = 0;
    repeated.set_type(4);
    repeated.set_type(1);
    assert(repeated.type_ == 1u);
    assert(repeated.flags_40_ == (0x40000000u | 0x10u | 0x04u));
    repeated.set_type(8);
    assert(repeated.type_ == 0u);
    assert(repeated.flags_40_ == (0x40000000u | 0x10u | 0x04u));
    return 0;
}
'''


def _run_host(compiler: str, text: str, directory: Path, stem: str) -> subprocess.CompletedProcess:
    source = directory / (stem + ".cpp")
    binary = directory / stem
    source.write_text(text)
    built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                            str(source), "-o", str(binary)], capture_output=True, text=True,
                           timeout=45)
    assert built.returncode == 0, built.stdout + built.stderr

    def no_core_dumps():
        resource.setrlimit(resource.RLIMIT_CORE, (0, 0))

    return subprocess.run([str(binary)], capture_output=True, text=True,
                          preexec_fn=no_core_dumps, timeout=15)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_set_type_source_and_case_swap_damage_control(compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable for bounded C++98 host control")
    method = _source_method()
    assert method.startswith("void Sound::set_type(unsigned int type)")
    with tempfile.TemporaryDirectory(prefix="sound-type-dispatch-") as temp:
        directory = Path(temp)
        actual = _run_host(compiler, _host_program(method), directory, "actual")
        assert actual.returncode == 0, actual.stdout + actual.stderr

        marker = "case 0x7fffffff:"
        assert marker not in method
        assert method.count("case 1:") == 1 and method.count("case 4:") == 1
        swapped = (method.replace("case 1:", marker, 1)
                   .replace("case 4:", "case 1:", 1)
                   .replace(marker, "case 4:", 1))
        damaged = _run_host(compiler, _host_program(swapped), directory, "swapped_cases")
        assert damaged.returncode != 0, "reversing the type-1/type-4 table effects must fail the source control"


def _pinned_image() -> PeImage:
    if not GAME.is_file():
        pytest.skip(f"pinned original is unavailable: {GAME}")
    raw = GAME.read_bytes()
    digest = hashlib.sha256(raw).hexdigest()
    assert digest == GAME_SHA256, f"pinned executable digest mismatch: {GAME}"
    return PeImage(raw, digest)


def _decode_exact(image: PeImage, start: int, end: int):
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    code = image.read(Span(start, end))
    instructions = tuple(decoder.disasm(code, start))
    assert instructions and instructions[0].address == start
    assert instructions[-1].address + instructions[-1].size == end
    return instructions


def _memory_at(instruction, operand_index: int, displacement: int) -> bool:
    if len(instruction.operands) <= operand_index:
        return False
    operand = instruction.operands[operand_index]
    return (operand.type == X86_OP_MEM and operand.size == 4
            and operand.mem.base == X86_REG_ECX and operand.mem.index == 0
            and operand.mem.disp == displacement)


def _or_effect(instruction) -> int | None:
    if instruction.mnemonic != "or" or len(instruction.operands) != 2:
        return None
    destination, value = instruction.operands
    if destination.type != X86_OP_REG or value.type != X86_OP_IMM:
        return None
    register = instruction.reg_name(destination.reg).lower()
    if register == "edx":
        return value.imm & 0xFFFFFFFF
    if register == "dh":
        return (value.imm & 0xFF) << 8
    if register == "dl":
        return value.imm & 0xFF
    return None


def _assert_case_effect(instructions, effect: int | None, *, native: bool = False):
    # A bounded checker for these observed arms, not a general x86 emulator.
    # Account for the optional frame restore, but require the full dataflow:
    # input EAX -> type, loaded flags EDX -> OR -> flags, and no extra effects.
    pops = [item for item in instructions if item.mnemonic == "pop"]
    assert len(pops) <= 1
    assert all(len(item.operands) == 1 and item.operands[0].type == X86_OP_REG
               and item.operands[0].reg == X86_REG_EBP for item in pops)
    if pops:
        assert instructions[-2] is pops[0]
    operations = tuple(item for item in instructions if item.mnemonic != "pop")
    assert operations[-1].mnemonic == "ret"
    assert len(operations[-1].operands) == 1 and operations[-1].operands[0].imm == 4
    if effect is None:
        assert len(operations) == 2
    else:
        assert len(operations) == 5
        load, store_type, apply, store_flags, _ = operations
        assert load.mnemonic == "mov" and len(load.operands) == 2
        assert load.operands[0].type == X86_OP_REG and load.operands[0].reg == X86_REG_EDX
        assert _memory_at(load, 1, 0x40)
        assert store_type.mnemonic == "mov" and _memory_at(store_type, 0, 0x50)
        assert store_type.operands[1].type == X86_OP_REG and store_type.operands[1].reg == X86_REG_EAX
        assert _or_effect(apply) == effect
        assert store_flags.mnemonic == "mov" and _memory_at(store_flags, 0, 0x40)
        assert store_flags.operands[1].type == X86_OP_REG and store_flags.operands[1].reg == X86_REG_EDX
    returns = [item for item in instructions if item.mnemonic.startswith("ret")]
    assert returns, "case target must reach a return"
    type_stores = [index for index, item in enumerate(instructions)
                   if item.mnemonic == "mov" and _memory_at(item, 0, 0x50)]
    assert len(type_stores) == 1
    type_store = instructions[type_stores[0]]
    if effect is None:
        assert type_store.operands[1].type == X86_OP_IMM
        assert type_store.operands[1].imm == 0
        assert not any(item.mnemonic == "or" for item in instructions)
        assert not any(item.mnemonic == "mov" and _memory_at(item, 0, 0x40)
                       for item in instructions)
        return

    assert type_store.operands[1].type == X86_OP_REG
    flag_stores = [index for index, item in enumerate(instructions)
                   if item.mnemonic == "mov" and _memory_at(item, 0, 0x40)]
    assert len(flag_stores) == 1
    effects = [index for index, item in enumerate(instructions)
               if _or_effect(item) == effect]
    assert len(effects) == 1
    assert type_stores[0] < effects[0] < flag_stores[0], (
        "the original arm writes its type before applying/storing the flag effect"
    )
    if native:
        assert flag_stores[0] < next(index for index, item in enumerate(instructions)
                                     if item.mnemonic.startswith("ret"))


def test_pinned_selector_and_separate_seven_entry_table_effects():
    image = _pinned_image()
    selector = _decode_exact(image, METHOD_START, METHOD_END)
    table_bytes = image.read(Span(TABLE_START, TABLE_END))
    assert len(selector) and len(table_bytes) == 7 * 4

    assert any(item.mnemonic == "mov" and item.op_str.lower() == "eax, dword ptr [ebp + 8]"
               for item in selector)
    assert any(item.mnemonic == "lea" and item.op_str.lower() == "edx, [eax - 1]"
               for item in selector)
    assert any(item.mnemonic == "cmp" and item.op_str.lower() == "edx, 6"
               for item in selector)
    assert any(item.mnemonic == "ja" and int(item.op_str, 16) == PINNED_TARGETS[2]
               for item in selector)
    assert any(item.mnemonic == "jmp" and item.operands
               and item.operands[0].type == X86_OP_MEM
               and item.reg_name(item.operands[0].mem.index) == "edx"
               and item.operands[0].mem.scale == 4
               and item.operands[0].mem.disp == TABLE_START
               for item in selector)

    targets = tuple(int.from_bytes(table_bytes[index:index + 4], "little")
                    for index in range(0, len(table_bytes), 4))
    assert targets == PINNED_TARGETS
    for target, effect in zip(targets, TYPE_EFFECTS):
        block = _decode_exact(image, target,
                              min(METHOD_END, target + (0x10 if effect is not None else 0x0B)))
        # The exact bounded arm spans are naturally terminated by their ret;
        # decode only through that instruction if the selected span contains it.
        ret = next(item for item in block if item.mnemonic.startswith("ret"))
        block = tuple(item for item in block if item.address <= ret.address)
        _assert_case_effect(block, effect)


def _native_local_offset(obj: Coff, function_section: int, relocation,
                         section_bytes: bytes) -> int:
    assert relocation.kind is RelocationKind.ABSOLUTE32
    symbol = obj.symbols[int(relocation.target)]
    assert symbol.section == function_section, "jump-table binding must stay in the function's physical section"
    assert relocation.offset + 4 <= len(section_bytes)
    addend = int.from_bytes(section_bytes[relocation.offset:relocation.offset + 4], "little")
    offset = symbol.value + addend
    assert 0 <= offset < len(section_bytes)
    return offset


def validate_native_sound_dispatch(obj: Coff) -> tuple[int, ...]:
    """Validate Sound's compiled seven-entry switch table without local-label names.

    Coordinator-callable on each actual Sound COFF flag profile. All table and
    case destinations must resolve, after their COFF addends, within the same
    physical code section as the function. Returns the seven resolved offsets.
    """
    function = obj.function(NATIVE_METHOD)
    code, relocations, _ = obj.subject(function)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    instructions = tuple(decoder.disasm(code, 0))
    relocation_by_offset = {item.offset: item for item in relocations}
    jumps = []
    for item in instructions:
        if item.mnemonic != "jmp" or not item.operands or item.operands[0].type != X86_OP_MEM:
            continue
        operand = item.operands[0]
        if operand.mem.scale != 4 or operand.mem.index == 0:
            continue
        relocation = relocation_by_offset.get(item.address + item.size - 4)
        if relocation is not None and relocation.kind is RelocationKind.ABSOLUTE32:
            jumps.append((item, relocation))
    assert len(jumps) == 1, "expected one relocated indirect jump through the switch table"
    jump, table_relocation = jumps[0]
    table_start = _native_local_offset(obj, function.section, table_relocation, code)
    assert table_start % 4 == 0 and table_start + 7 * 4 <= len(code)

    table_entries = []
    for index in range(7):
        entry_offset = table_start + index * 4
        entry_relocations = [item for item in relocations if item.offset == entry_offset]
        assert len(entry_relocations) == 1, f"missing unique ABS32 table entry {index}"
        table_entries.append(_native_local_offset(obj, function.section,
                                                  entry_relocations[0], code))
    assert len(set(table_entries)) == 7

    # Confirm the unsigned out-of-range path lands on the same default arm as
    # the table's type-3 slot. No selector/table relationship is inferred from
    # compiler-generated local label spellings.
    jump_index = instructions.index(jump)
    assert jump.operands[0].mem.base == 0 and jump.operands[0].mem.index == X86_REG_EDX
    load, subtract, compare, branch = instructions[jump_index - 4:jump_index]
    assert load.mnemonic == "mov" and len(load.operands) == 2
    assert load.operands[0].type == X86_OP_REG and load.operands[0].reg == X86_REG_EAX
    argument = load.operands[1]
    assert argument.type == X86_OP_MEM and argument.size == 4 and argument.mem.index == 0
    assert (argument.mem.base, argument.mem.disp) in ((X86_REG_EBP, 8), (X86_REG_ESP, 4))
    assert subtract.mnemonic == "lea" and len(subtract.operands) == 2
    assert subtract.operands[0].type == X86_OP_REG and subtract.operands[0].reg == X86_REG_EDX
    assert (subtract.operands[1].type == X86_OP_MEM
            and subtract.operands[1].mem.base == X86_REG_EAX
            and subtract.operands[1].mem.index == 0 and subtract.operands[1].mem.disp == -1)
    assert compare.mnemonic == "cmp" and compare.operands[0].type == X86_OP_REG
    assert compare.operands[0].reg == X86_REG_EDX and compare.operands[1].imm == 6
    assert branch.mnemonic == "ja" and branch.operands[0].imm == table_entries[2]
    bounds = []
    for index, item in enumerate(instructions[:jump_index]):
        if item.mnemonic != "cmp" or len(item.operands) != 2:
            continue
        if item.operands[1].type != X86_OP_IMM or item.operands[1].imm != 6:
            continue
        for branch in instructions[index + 1:jump_index]:
            if (branch.group(CS_GRP_JUMP) and branch.mnemonic == "ja"
                    and branch.operands and branch.operands[0].type == X86_OP_IMM
                    and branch.operands[0].imm == table_entries[2]):
                bounds.append((index, instructions.index(branch)))
    assert bounds, "unsigned selector bound must route to the default arm"

    instruction_offsets = {item.address for item in instructions}
    assert all(target in instruction_offsets and target < table_start for target in table_entries), (
        "each relocated table destination must be an instruction entry in this section"
    )
    for target, effect in zip(table_entries, TYPE_EFFECTS):
        block_decoder = Cs(CS_ARCH_X86, CS_MODE_32)
        block_decoder.detail = True
        block = []
        for item in block_decoder.disasm(code[target:table_start], target):
            block.append(item)
            if item.mnemonic.startswith("ret"):
                break
            assert len(block) < 12, "switch case target did not reach a bounded return"
        _assert_case_effect(tuple(block), effect, native=True)
    return tuple(table_entries)


@pytest.mark.parametrize("profile_index", (0, 1))
def test_native_profiles_reject_the_original_recovery_dispatch_error(profile_index):
    """Compile real source and a source-overlay swap, not prebuilt fixtures."""
    from recovery.environment.wine import VC6_ROOT
    if (not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None
            or not (ROOT / "build/compile_commands.json").is_file()):
        pytest.skip("prepared VC6/Wine compiler is required")
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    flags = settings.flag_sets[profile_index] + settings.layout_flags
    unit = ROOT / "src/sound_type.cpp"
    actual = Coff(compiler(unit, (), flags))
    validate_native_sound_dispatch(actual)
    method = _source_method()
    marker = "case 0x7fffffff:"
    assert marker not in method
    swapped = (method.replace("case 1:", marker, 1)
               .replace("case 4:", "case 1:", 1)
               .replace(marker, "case 4:", 1))
    text = unit.read_text()
    assert text.count(method) == 1
    damaged = Coff(compiler.overlay(unit, {"src/sound_type.cpp": text.replace(method, swapped, 1).encode()},
                                    (), flags))
    with pytest.raises(AssertionError):
        validate_native_sound_dispatch(damaged)
