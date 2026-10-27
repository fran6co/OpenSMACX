"""Source-derived controls for Spot's pinned signed count fields."""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def definition(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[start:index + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def actual_methods(source: str) -> str:
    signatures = (
        "void Spot::shutdown()",
        "Spot::Spot()",
        "void Spot::init(int count)",
        "void Spot::replace(",
        "int Spot::add(int position, int type, int left, int top, int length, int width)",
        "int Spot::add(int position, int type, RECT *rect)",
        "void Spot::kill_pos(",
        "void Spot::kill_specific(",
        "void Spot::kill_type(",
        "int Spot::check(int x, int y, int *spot_pos, int *spot_type)",
        "int Spot::check(int x, int y, int *spot_pos, int *spot_type, RECT *spot_rect)",
        "int Spot::get_rect(",
        "Spot::~Spot()",
    )
    return "\n\n".join(definition(source, signature) for signature in signatures)


def reverse_start_expressions(source: str) -> tuple[str, str]:
    kill_body = definition(source, "void Spot::kill_specific(")
    rect_body = definition(source, "int Spot::get_rect(")
    kill_match = re.search(r"for\s*\(int i = (.*?); i >= 0; i--\)", kill_body)
    rect_match = re.search(r"int offset = (.*?);", rect_body)
    assert kill_match and rect_match
    return kill_match.group(1), rect_match.group(1)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_spot_actual_owner_signed_fields_and_behavior(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")

    header = (ROOT / "src/spot.h").read_text()
    source = (ROOT / "src/spot.cpp").read_text()
    assert "int max_count_;" in header
    assert "int add_count_;" in header
    assert "static_cast<int>(static_cast<uint32_t>(add_count_) - 1u)" in source
    assert source.count("OPENSMACX_SPOT_MINIMAL_INIT") == 0

    fixture = (Path(__file__).with_name("spot_signed_counts_control.cpp")).read_text()
    assert fixture.count("// @@ACTUAL_SPOT_METHODS@@") == 1
    kill_start, rect_start = reverse_start_expressions(source)
    assert "static_cast<uint32_t>(add_count_) - 1u" in kill_start
    assert "static_cast<uint32_t>(add_count_) - 1u" in rect_start
    generated_source = fixture.replace("// @@ACTUAL_SPOT_METHODS@@", actual_methods(source))
    generated_source = generated_source.replace("@@KILL_SPECIFIC_START@@", kill_start)
    generated_source = generated_source.replace("@@GET_RECT_START@@", rect_start)
    cpp = tmp_path / "spot_signed_counts.cpp"
    cpp.write_text(generated_source)
    executable = tmp_path / f"spot-signed-counts-{compiler_name}"
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror=return-type",
         "-I", str(ROOT / "src"), str(cpp), "-o", str(executable)],
        capture_output=True, text=True, timeout=45, check=False,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(executable)], capture_output=True, text=True,
                         timeout=10, check=False)
    assert ran.returncode == 0, ran.stdout + ran.stderr

    # Damage the capacity member type but retain the real methods. With an
    # unsigned capacity, -1 promotes to UINT_MAX and add incorrectly accepts
    # slot zero; this must fail at the behavior assertion, not a spelling gate.
    mutant_dir = tmp_path / "unsigned-capacity-mutant"
    mutant_dir.mkdir()
    mutant_header = header.replace("int max_count_;", "uint32_t max_count_;", 1)
    assert mutant_header != header
    (mutant_dir / "spot.h").write_text(mutant_header)
    mutant_source = generated_source.replace(
        'static_assert(std::is_same<decltype(Spot::max_count_), int>::value,\n'
        '              "pinned Spot capacity is a signed int");\n', "")
    mutant_cpp = mutant_dir / "spot_signed_counts.cpp"
    mutant_cpp.write_text(mutant_source)
    mutant_binary = mutant_dir / "spot-signed-counts-mutant"
    mutant_built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror=return-type",
         "-I", str(ROOT / "src"), str(mutant_cpp), "-o", str(mutant_binary)],
        capture_output=True, text=True, timeout=45, check=False,
    )
    assert mutant_built.returncode == 0, mutant_built.stdout + mutant_built.stderr
    mutant_ran = subprocess.run([str(mutant_binary)], capture_output=True, text=True,
                                timeout=10, check=False)
    assert mutant_ran.returncode != 0, "unsigned capacity mutant unexpectedly passed"


def test_pinned_spot_count_accesses_are_signed():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin

    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", str(ROOT / ".opensmacx/game/terranx_original.exe")))
    if not image_path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    spans = (
        (0x5FA8A0, 0x5FA8F5), (0x5FA900, 0x5FA956),
        (0x5FA960, 0x5FA9C0), (0x5FA9C0, 0x5FAA04),
        (0x5FAA10, 0x5FAA89), (0x5FAA90, 0x5FAAFD),
        (0x5FAB00, 0x5FAB69), (0x5FAB70, 0x5FABFF),
        (0x5FAC00, 0x5FAC5B),
    )
    decoded = {}
    for start, end in spans:
        rows = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(
            image.read(Span(start, end)), start))
        cursor = start
        for row in rows:
            assert row.address == cursor
            cursor += row.size
            decoded[row.address] = (row.mnemonic, row.op_str)
        assert cursor == end

    # init(int) stores its input into +4; every mapped count consumer takes
    # signed branches on +8, and add compares +8 with +4 using JL.
    assert decoded[0x5FA8B9] == ("mov", "edi, dword ptr [esp + 0xc]")
    assert decoded[0x5FA8E6] == ("mov", "dword ptr [esi + 4], edi")
    assert decoded[0x5FA968][0] == "jl"
    assert decoded[0x5FA908][0] == "jle"
    assert decoded[0x5FAA18][0] == "js"
    assert decoded[0x5FAA98][0] == "js"
    assert decoded[0x5FAB07][0] == "js"
    assert decoded[0x5FAB77][0] == "js"
    assert decoded[0x5FAC06][0] == "js"
    assert decoded[0x5FA960] == ("mov", "eax, dword ptr [ecx + 8]")
    assert decoded[0x5FA963] == ("mov", "edx, dword ptr [ecx + 4]")

