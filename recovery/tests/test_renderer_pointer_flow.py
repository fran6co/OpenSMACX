"""Actual-source and native controls for the 27 borrowed Sprite slots.

The host controls compile extracted pointer-flow operations. Native controls
separately check copy operands, current types/layout and redraw slots. They
do not establish the unrelated remainder of BasePop::close or game acceptance.
"""
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_OP_REG

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import Mode, function_line, read_file
from match_recovery.source_inventory import image_pin

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
FIXTURE = HERE / "renderer_pointer_flow_harness.cpp"
sys.path.insert(0, str(ROOT / "recovery/tests"))
from test_game_logic_seed_batch_behavior import _definition


def mapped_body(relative, address, signature):
    records = [row for row in read_file(ROOT / relative) if row.address == address]
    assert len(records) == 1, "missing/duplicate mapped source body " + hex(address)
    record, = records
    assert record.mode == Mode.BODY and record.region
    assert signature in record.region
    return _definition(record.region, signature)


def pointer_sources():
    flat_h = (ROOT / "src/flatbutton.h").read_text()
    flat_cpp = (ROOT / "src/flatbutton.cpp").read_text()
    base_h = (ROOT / "src/basepop.h").read_text()
    base_cpp = (ROOT / "src/basepop.cpp").read_text()
    assert re.search(r"Sprite\s*\*\s*config\s*\[\s*27\s*\]", flat_h)
    assert re.search(r"Sprite\s*\*\s*FlatButtonDefaultsTable\s*\[\s*27\s*\]", flat_h)
    assert re.search(r"Sprite\s*\*\s*default_block_\s*\[\s*27\s*\]", base_h)
    assert re.search(r"Sprite\s*\*\s*BasePopDefaultBlock31B8\s*\[\s*27\s*\]", base_h)
    assert re.search(r"Sprite\s*\*\s*FlatButtonDefaultsTable\s*\[\s*27\s*\]", flat_cpp)
    assert re.search(r"Sprite\s*\*\s*BasePopDefaultBlock31B8\s*\[\s*27\s*\]", base_cpp)

    ctor = mapped_body("src/flatbutton.cpp", 0x00607CF0,
                       "FlatButton::FlatButton()")
    close = mapped_body("src/flatbutton.cpp", 0x00607DA0,
                        "void FlatButton::close()")
    setter = _definition(flat_h, "void set_slice_icons(")
    setter = setter.replace("void set_slice_icons(",
                            "void FlatButton::set_slice_icons(", 1)
    apply_line = function_line(ROOT / "src/basepop.h", "apply_button_config")
    assert apply_line > 0
    apply = _definition(base_h, "void apply_button_config(")
    apply = apply.replace("void apply_button_config(",
                          "void BasePop::apply_button_config(", 1)
    game_source = (ROOT / "src/game.cpp").read_text()
    assert function_line(ROOT / "src/game.cpp", "load_interface") > 0
    initializer = _definition(game_source, "void __cdecl load_interface(")
    expected_producer = {12: 18, 13: 21, 14: 24, 6: 19, 17: 26,
                         8: 25, 15: 20, 7: 22, 16: 23}
    for table in ("FlatButtonDefaultsTable", "BasePopDefaultBlock31B8"):
        writes = re.findall(
            rf"{table}\s*\[\s*(\d+)\s*\]\s*=\s*&g_BASEWIN_SPRITES\s*\[\s*(\d+)\s*\]",
            initializer)
        assert {int(slot): int(sprite) for slot, sprite in writes} == expected_producer
        table_lines = [line for line in initializer.splitlines() if table + "[" in line]
        assert all("reinterpret_cast" not in line for line in table_lines)
    return ctor, close, setter, apply


def compile_flow(ctor, close, setter, apply):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("GCC or Clang host compiler is required")
    source = FIXTURE.read_text()
    replacements = {
        "// ACTUAL_FLATBUTTON_CTOR": ctor,
        "// ACTUAL_FLATBUTTON_CLOSE": close,
        "// ACTUAL_FLATBUTTON_SLICE_SETTER": setter,
        "// ACTUAL_BASEPOP_APPLY_CONFIG": apply,
    }
    for marker, body in replacements.items():
        assert source.count(marker) == 1
        source = source.replace(marker, body)
    with tempfile.TemporaryDirectory(prefix="late-ui-pointer-flow-") as temp:
        cpp = Path(temp) / "pointer_flow.cpp"
        exe = Path(temp) / "pointer_flow"
        cpp.write_text(source)
        built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra",
                                str(cpp), "-o", str(exe)],
                               capture_output=True, text=True)
        assert built.returncode == 0, built.stdout + built.stderr
        return subprocess.run([str(exe)], capture_output=True, text=True)


def test_actual_flatbutton_ctor_close_and_basepop_apply_preserve_27_pointer_values():
    ctor, close, setter, apply = pointer_sources()
    run = compile_flow(ctor, close, setter, apply)
    assert run.returncode == 0, run.stdout + run.stderr


def test_pointer_flow_control_detects_final_slot_copy_damage():
    ctor, close, setter, apply = pointer_sources()
    original = "button->config[26] = default_block_[26];"
    assert apply.count(original) == 1
    damaged = apply.replace(original,
                            "button->config[26] = default_block_[25];", 1)
    run = compile_flow(ctor, close, setter, damaged)
    assert run.returncode != 0, "last Sprite pointer slot mutation escaped"


def test_native_basepop_close_copy_operands_are_same_index_column_walks():
    image_path = Path(__import__("os").environ.get(
        "OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file():
        pytest.skip("pinned native executable is unavailable")
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    code = image.read(Span(0x00600F83, 0x00600FDE))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    instructions = list(decoder.disasm(code, 0x00600F83))
    assert instructions and instructions[-1].mnemonic == "jl"
    assert instructions[-1].op_str == "0x600f83"
    loop = instructions[:-2]  # exclude cmp/jl; retain all data transfers/adds

    # Execute only the decoded register/addressing form symbolically. The
    # BasePop object is represented by its disassembled ECX anchor; no C++
    # mock layout participates in this evidence.
    source_indices, destination_indices = [], []
    eax = 0
    ecx_object_offset = 0x31C4
    table_va = 0x009B8D10
    for _column in range(3):
        for op in loop:
            if op.mnemonic == "add" and op.operands[0].type == X86_OP_REG:
                target = op.reg_name(op.operands[0].reg)
                if op.operands[1].type == X86_OP_IMM:
                    if target == "eax":
                        eax += op.operands[1].imm
                    elif target == "ecx":
                        ecx_object_offset += op.operands[1].imm
                continue
            if op.mnemonic != "mov" or len(op.operands) != 2:
                continue
            dst, src = op.operands
            if dst.type == X86_OP_REG and op.reg_name(dst.reg) == "edx" and src.type == X86_OP_MEM:
                assert op.reg_name(src.mem.base) == "eax"
                address = eax + src.mem.disp
                assert address >= table_va
                source_indices.append((address - table_va) // 4)
            elif dst.type == X86_OP_MEM and src.type == X86_OP_REG:
                assert op.reg_name(src.reg) == "edx"
                assert op.reg_name(dst.mem.base) == "ecx"
                address = ecx_object_offset + dst.mem.disp
                assert address >= 0x31B8
                destination_indices.append((address - 0x31B8) // 4)

    expected = [column + row * 3 for column in range(3) for row in range(9)]
    assert source_indices == expected
    assert destination_indices == expected
    assert sorted(source_indices) == list(range(27))
    assert sorted(destination_indices) == list(range(27))


def test_source_close_copy_is_pointer_typed_and_not_the_old_shifted_source_index():
    source = (ROOT / "src/basepop.cpp").read_text()
    assert function_line(ROOT / "src/basepop.cpp", "BasePop::close") > 0
    body = _definition(source, "void BasePop::close()")
    assert "BasePopDefaultBlock31B8" in body
    assert "BasePopDefaultBlock31B8[i / 4 + 2]" not in body
    assert "BasePopDefaultBlock31B8[i / 4 + 5]" not in body
    assert_copy_column_same_index(body)
    # The unrelated cleanup/reset body remains deliberately outside these
    # assertions; only the 27 borrowed sprite-copy source indexes are scoped.


def assert_copy_column_same_index(body):
    copy = re.search(
        r"for\s*\(int column = 0; column < 3; \+\+column\)\s*\{\s*"
        r"for\s*\(int group = 0; group < 9; \+\+group\)\s*\{\s*"
        r"default_block_\[([^]]+)\]\s*=\s*"
        r"BasePopDefaultBlock31B8\[([^]]+)\]\s*;",
        body)
    assert copy, "BasePop close must retain the bounded 3-column x 9-group copy"
    assert copy.group(1).strip() == copy.group(2).strip()


def test_copy_column_control_rejects_source_index_shift():
    source = (ROOT / "src/basepop.cpp").read_text()
    body = _definition(source, "void BasePop::close()")
    wrong = body.replace(
        "BasePopDefaultBlock31B8[group * 3 + column]",
        "BasePopDefaultBlock31B8[group * 3 + column + 1]", 1)
    assert wrong != body
    with pytest.raises(AssertionError):
        assert_copy_column_same_index(wrong)


def test_native_layout_types_and_primary_redraw_dispatch():
    """Fresh physical COFF and original table words, not host mock layouts."""
    import os
    import struct
    from match_recovery.codeview import class_layout
    from match_recovery.compiler import Compiler
    from match_recovery.measurement import Measurement
    from match_recovery.pe_coff import Coff
    from recovery.environment.wine import VC6_ROOT

    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file() or not (VC6_ROOT / "BIN/CL.EXE").is_file() or not shutil.which("wine32"):
        pytest.skip("pinned image, VC6 and Wine32 required")
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    owners = (
        ("FlatButton", "src/flatbutton.cpp", 0xB4C, "config", 0xAE0,
         0x669754, 0x607E50),
        ("BasePop", "src/basepop.cpp", 0x3230, "default_block_", 0x31B8,
         None, None),
        ("PullDown", "src/pulldown.cpp", 0xF40, None, None,
         0x66FF40, 0x5F98B0),
    )
    for flags in settings.flag_sets:
        for owner, unit, size, member, offset, table_va, target_va in owners:
            obj = Coff(compiler(ROOT / unit, (), flags + settings.layout_flags))
            layout = class_layout(obj, owner)
            assert layout["size"] == size
            if member:
                assert layout["members"][member] == offset
                array = layout["member_types"][member]
                assert (array["kind"], array["count"], array["size"], array["element_size"]) == ("array", 27, 108, 4)
                pointer = array["target"]
                assert pointer["kind"] == "pointer" and pointer["width"] == 4
                assert pointer["target"]["class"] == "Sprite"
            if table_va is not None:
                table, = (s for s in obj.symbols.values()
                          if s.name == f"??_7{owner}@@6BWin@@@" and s.section > 0)
                section = obj.sections[table.section - 1]
                slots = struct.unpack("<90I", image.read(Span(table_va, table_va + 360)))
                slot, = (index for index, value in enumerate(slots) if value == target_va)
                relocated = {offset: obj.symbols[target].name
                             for offset, target, kind in section.relocations if kind == 6}
                symbol = f"?on_redraw@{owner}@@UAEXXZ"
                assert relocated[table.value + slot * 4] == symbol
                assert any(s.name == symbol and s.section > 0 for s in obj.symbols.values())
