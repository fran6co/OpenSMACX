from __future__ import annotations

import shutil
import subprocess
from pathlib import Path
import os


ROOT = Path(__file__).resolve().parent
SRC = ROOT.parents[1] / "src"


def _function(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated function {signature}")


def test_huffman_host_links_actual_product_delta_helpers(tmp_path: Path) -> None:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        return
    decoder = (SRC / "movie_decoder_huffman.cpp").read_text()
    executable = _compile_huffman_host(compiler, decoder, tmp_path, "huffman_host")
    subprocess.run([str(executable)], check=True)


def _compile_huffman_host(
    compiler: str, decoder_source: str, tmp_path: Path, stem: str
) -> Path:
    general = (SRC / "general.cpp").read_text()
    helper_4x = _function(general, 'extern "C" void __fastcall sub_5d2d20(')
    helper_2x = _function(general, 'void __fastcall sub_5d2e00(')
    product = tmp_path / "mapped_delta_helpers.cpp"
    product.write_text(
        "#define __fastcall\n"
        + helper_4x
        + "\n"
        + helper_2x
        + "\n"
    )
    decoder_path = tmp_path / f"{stem}_decoder.cpp"
    decoder_path.write_text(decoder_source)
    binary = tmp_path / stem
    subprocess.run(
        [
            compiler,
            "-std=c++17",
            "-D__fastcall=",
            "-Wall",
            "-Wextra",
            "-Werror",
            "-pedantic",
            "-iquote",
            str(SRC),
            str(decoder_path),
            str(ROOT / "movie_decoder_huffman_host.cpp"),
            str(product),
            "-o",
            str(binary),
        ],
        check=True,
    )
    return binary


def test_dc_and_ac_window_mutants_fail_actual_huffman_host(tmp_path: Path) -> None:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        return
    actual = (SRC / "movie_decoder_huffman.cpp").read_text()
    mutants = (
        ("state.coefficients[0] = state.fill_value;",
         "static_cast<void>(state.fill_value);"),
        ("std::memset(state.coefficients + 1, 0, 63 * sizeof(*state.coefficients));",
         "std::memset(state.coefficients, 0, 63 * sizeof(*state.coefficients));"),
        ("std::memset(state.coefficients + 1, 0, 63 * sizeof(*state.coefficients));",
         "std::memset(state.coefficients + 1, 0, 62 * sizeof(*state.coefficients));"),
    )
    for index, (before, after) in enumerate(mutants):
        assert actual.count(before) == 1
        mutated = actual.replace(before, after, 1)
        executable = _compile_huffman_host(
            compiler, mutated, tmp_path, f"huffman_mutant_{index}"
        )
        result = subprocess.run([str(executable)], check=False)
        assert result.returncode != 0, f"mutant {index} survived host assertions"


def test_pinned_native_dc_store_precedes_refill_and_clears_ac_slots() -> None:
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_OP_REG, X86_REG_EAX, X86_REG_EBX
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", str(SRC.parent / ".opensmacx/game/terranx_original.exe")
    ))
    if not image_path.is_file():
        import pytest
        pytest.skip("pinned original unavailable")
    image = PeImage(
        image_path.read_bytes(),
        "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf",
    )
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    start, end = 0x5CE85C, 0x5CEA09
    instructions = list(decoder.disasm(image.read(Span(start, end)), start))
    assert sum(instruction.size for instruction in instructions) == end - start
    by_address = {instruction.address: instruction for instruction in instructions}

    dc_store = by_address[0x5CE884]
    assert dc_store.mnemonic == "mov"
    assert dc_store.operands[0].type == X86_OP_MEM
    assert dc_store.operands[0].mem.disp == 0x9C314C
    assert dc_store.operands[1].type == X86_OP_REG
    assert dc_store.operands[1].reg == X86_REG_EAX
    first_refill = by_address[0x5CE896]
    assert first_refill.mnemonic == "mov"
    assert first_refill.operands[0].type == X86_OP_REG
    assert first_refill.operands[1].type == X86_OP_MEM
    assert first_refill.address > dc_store.address

    clear_base = by_address[0x5CE879]
    assert clear_base.mnemonic == "mov"
    assert clear_base.operands[0].type == X86_OP_REG
    assert clear_base.operands[0].reg == X86_REG_EBX
    assert clear_base.operands[1].type == X86_OP_IMM
    assert clear_base.operands[1].imm == 0x9C3150
    zero_eax = by_address[0x5CE8AA]
    assert zero_eax.mnemonic == "xor"
    assert zero_eax.op_str == "eax, eax"
    loop_count = by_address[0x5CE8AC]
    assert loop_count.mnemonic == "mov" and loop_count.operands[1].imm == 7
    dc_shift = by_address[0x5CE876]
    assert dc_shift.mnemonic == "sar" and dc_shift.operands[0].reg == X86_REG_EAX
    assert dc_shift.operands[1].type == X86_OP_IMM and dc_shift.operands[1].imm == 24
    dc_product = by_address[0x5CE87E]
    assert dc_product.mnemonic == "imul" and len(dc_product.operands) == 1
    assert dc_product.operands[0].type == X86_OP_MEM
    assert dc_product.operands[0].mem.disp == 0x9C454C
    zero_stores = [instruction for instruction in instructions
                   if 0x5CE8B1 <= instruction.address < 0x5CE8CB]
    assert len(zero_stores) == 9
    assert [instruction.address for instruction in zero_stores] == [
        0x5CE8B1, 0x5CE8B3, 0x5CE8B6, 0x5CE8B9, 0x5CE8BC,
        0x5CE8BF, 0x5CE8C2, 0x5CE8C5, 0x5CE8C8,
    ]
    for index, instruction in enumerate(zero_stores):
        assert instruction.mnemonic == "mov"
        assert instruction.operands[0].type == X86_OP_MEM
        assert instruction.operands[0].mem.base == X86_REG_EBX
        assert instruction.operands[0].mem.disp == index * 4
        assert instruction.operands[1].type == X86_OP_REG
        assert instruction.operands[1].reg == X86_REG_EAX
    loop_step = by_address[0x5CE8CB]
    assert loop_step.mnemonic == "add" and loop_step.operands[1].imm == 0x24
    loop_branch = by_address[0x5CE8CF]
    assert loop_branch.mnemonic == "jne"
    assert loop_branch.operands[0].imm == 0x5CE8B1
