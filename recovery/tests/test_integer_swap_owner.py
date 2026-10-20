"""Validate the mapped integer swap owner and its original call boundaries."""

from pathlib import Path
import os
import shutil
import subprocess

import pytest

from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
SWAP_SIGNATURE = "void __cdecl swap(int *var1, int *var2)"
BYTE_SWAP_SIGNATURE = "void __cdecl swap(uint8_t *var1, uint8_t *var2)"


def _definition(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for index in range(opening, len(text)):
        depth += (text[index] == "{") - (text[index] == "}")
        if depth == 0:
            return text[start:index + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def test_integer_swap_is_owned_out_of_line_and_adapters_are_gone():
    header = (ROOT / "src/general.h").read_text(encoding="utf-8")
    owner = (ROOT / "src/general.cpp").read_text(encoding="utf-8")
    base = (ROOT / "src/base.cpp").read_text(encoding="utf-8")
    scroll = (ROOT / "src/scroll.cpp").read_text(encoding="utf-8")
    console = (ROOT / "src/console_commands.cpp").read_text(encoding="utf-8")

    assert header.count(SWAP_SIGNATURE + ";") == 1
    assert "MEASURED inline void __cdecl swap(int *var1" not in header
    assert "MEASURED void __cdecl swap(uint8_t *var1, uint8_t *var2);" in header
    assert "MEASURED inline void __cdecl swap(uint8_t *var1" not in header
    assert "MEASURED void __cdecl swap(uint8_t *var1, uint8_t *var2) {" in owner
    assert "MEASURED inline void __cdecl swap(uint8_t *var1" not in owner
    byte_body = _definition(owner, BYTE_SWAP_SIGNATURE)
    assert owner.count(BYTE_SWAP_SIGNATURE + " {") == 1
    assert byte_body.count("^=") == 3
    assert "if (" not in byte_body and "std::swap" not in byte_body
    body = _definition(owner, SWAP_SIGNATURE)
    assert owner.count(SWAP_SIGNATURE + " {") == 1
    assert body.count("^=") == 3
    assert "if (" not in body and "std::swap" not in body
    assert "static void __cdecl swap_call(" not in base
    assert "swap_call(&base_" not in base
    assert "static void __cdecl range_swap_call(" not in scroll
    assert "range_swap_call(&minimum" not in scroll
    assert "swap(&base_energy[i], &base_energy[i + 1])" in base
    assert "swap(&base_id[i], &base_id[i + 1])" in base
    assert "swap(&minimum, &maximum)" in scroll
    diplo = (ROOT / "src/diplowin.cpp").read_text(encoding="utf-8")
    monu = (ROOT / "src/monuwin.cpp").read_text(encoding="utf-8")
    assert "swap(&low_faction, &high_faction);" in diplo
    assert "const int temporary = low_faction;" not in diplo
    assert "swap(&order[i - 1], &order[j]);" in monu
    assert "const int displaced = order[i - 1];" not in monu
    assert "const int bounds = GameDataStateStorage.fields_.base_current_count_ - 1;" in console
    numeric = console[console.index("if (sorting) {"):console.index("for (int candidate =")]
    assert numeric.index("swap(&scores[j], &scores[j + 1]);") < numeric.index("swap(&ids[j], &ids[j + 1]);")
    assert "for (int j = 0; j < GameDataStateStorage.fields_.base_current_count_ - 1; ++j)" in numeric
    assert "_stricmp(Bases[ids[j]].name_string" in numeric
    assert "swap(&ids[j], &ids[j + 1]);" in numeric

    mapped, = [row for row in read_file(ROOT / "src/general.cpp")
               if row.address == 0x00628A50]
    assert mapped.image_spans == ((0x00628A50, 0x00628A73),)
    assert mapped.extract_error == ""
    assert "body      src/general.cpp" in mapped.region
    assert SWAP_SIGNATURE in mapped.region

    source_facts = (ROOT / "src/general.cpp").read_text(encoding="utf-8")
    for address in ("0x005A58E0", "0x00422F00"):
        start = source_facts.index("// ORIGINAL: " + address)
        next_record = source_facts.find("// ORIGINAL: ", start + 1)
        record = source_facts[start: next_record if next_record >= 0 else len(source_facts)]
        assert "// body      src/general.h" in record
    assert "// BODY IN general.h" in source_facts


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_swap_bodies_and_xor_alias_contract(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    owner = (ROOT / "src/general.cpp").read_text(encoding="utf-8")
    body = _definition(owner, SWAP_SIGNATURE)
    byte_body = _definition(owner, BYTE_SWAP_SIGNATURE)
    program = r'''#include <cassert>
#include <climits>
#include <cstdint>
#define __cdecl
INT_BODY
BYTE_BODY
int main() {
    int left = 17, right = -91;
    swap(&left, &right);
    assert(left == -91 && right == 17);

    int low = INT_MIN, high = INT_MAX;
    swap(&low, &high);
    assert(low == INT_MAX && high == INT_MIN);

    int same = 0x13579BDF;
    swap(&same, &same);
    assert(same == 0); // Preserve the original XOR self-alias behavior.

    int rows[4] = { 3, 30, 1, 10 };
    if (rows[0] > rows[2]) {
        swap(&rows[0], &rows[2]);
        swap(&rows[1], &rows[3]);
    }
    assert(rows[0] == 1 && rows[1] == 10 &&
           rows[2] == 3 && rows[3] == 30);

    uint8_t first = 0xA5, second = 0x3C;
    swap(&first, &second);
    assert(first == 0x3C && second == 0xA5);

    uint8_t same_byte = 0xD7;
    swap(&same_byte, &same_byte);
    assert(same_byte == 0); // Preserve the byte overload's original XOR alias effect.
}
'''.replace("INT_BODY", body).replace("BYTE_BODY", byte_body)
    generated = tmp_path / "integer_swap.cpp"
    executable = tmp_path / f"integer-swap-{compiler_name}"
    generated.write_text(program, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(generated), "-o", str(executable)],
        capture_output=True, text=True, timeout=30, check=False,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(executable)], capture_output=True, text=True,
                         timeout=10, check=False)
    assert ran.returncode == 0, ran.stdout + ran.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_purge_trailing_body_and_changed_loop_control_is_detected(
        compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    owner = (ROOT / "src/general.cpp").read_text(encoding="utf-8")
    body = _definition(owner, "void __cdecl purge_trailing(LPSTR input)")
    assert "MEASURED void __cdecl purge_trailing(LPSTR input) {" in owner
    assert "MEASURED inline void __cdecl purge_trailing" not in owner
    program = r'''#include <cassert>
#include <cstring>
#define MEASURED
#define __cdecl
typedef char *LPSTR;
BODY
int main() {
    char trailing[] = "keep  interior   ";
    purge_trailing(trailing);
    assert(std::strcmp(trailing, "keep  interior") == 0);

    char all_spaces[] = "   ";
    purge_trailing(all_spaces);
    assert(all_spaces[0] == 0);

    char empty[] = "";
    purge_trailing(empty);
    assert(empty[0] == 0);

    char unchanged[] = "no trailing";
    purge_trailing(unchanged);
    assert(std::strcmp(unchanged, "no trailing") == 0);
}
'''

    def compile_run(source, stem):
        generated = tmp_path / f"{stem}.cpp"
        executable = tmp_path / stem
        generated.write_text(source, encoding="utf-8")
        built = subprocess.run(
            [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
             str(generated), "-o", str(executable)],
            capture_output=True, text=True, timeout=30, check=False,
        )
        assert built.returncode == 0, built.stdout + built.stderr
        return subprocess.run([str(executable)], capture_output=True, text=True,
                              timeout=10, check=False)

    actual = compile_run(program.replace("BODY", body), "purge-trailing-actual")
    assert actual.returncode == 0, actual.stdout + actual.stderr

    damaged = body.replace("*end == ' '", "*end != ' '", 1)
    assert damaged != body
    mutant = compile_run(program.replace("BODY", damaged), "purge-trailing-mutant")
    assert mutant.returncode != 0, "changed loop predicate passed the body-effect control"


def test_pinned_integer_swap_body_and_original_direct_call_sites():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from capstone.x86 import X86_OP_IMM
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin

    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True

    owner, = [row for row in read_file(ROOT / "src/general.cpp")
              if row.address == 0x00628A50]
    low, high = owner.image_spans[0]
    instructions = list(decoder.disasm(image.read(Span(low, high)), low))
    cursor = low
    for instruction in instructions:
        assert instruction.address == cursor
        cursor += instruction.size
    assert cursor == high
    assert [(i.address, i.mnemonic) for i in instructions if i.mnemonic == "xor"] == [
        (0x00628A5D, "xor"), (0x00628A65, "xor"), (0x00628A6E, "xor")]

    expected = {
        0x004EB51A, 0x004EB52F, 0x006059DD,
        0x004D708B, 0x004D70A0, 0x004D7108,
        0x005100E3, 0x005100F9,
        0x00444AE2, 0x0047618F,
    }
    for address in expected:
        instruction = next(row for row in decoder.disasm(
            image.read(Span(address, address + 5)), address)
            if row.address == address)
        assert instruction.mnemonic == "call"
        assert instruction.operands[0].type == X86_OP_IMM
        assert instruction.operands[0].imm == 0x00628A50
