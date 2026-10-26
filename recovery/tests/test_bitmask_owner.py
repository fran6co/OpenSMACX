"""Guard bitmask's shared owner and its alias/output contract."""

from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest

from match_recovery.source import read_file
from owner_transfer_test_support import current_record_by_address


ROOT = Path(__file__).resolve().parents[2]
BITMASK_SIGNATURE = "void __cdecl bitmask(int input, int *offset, int *mask)"
BUILT_SIGNATURE = "inline int __cdecl has_fac_built(int facility_id, int base_id)"
GUARDED_SIGNATURE = "int __cdecl has_fac_built(int facility_id)"


def _definition(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for index in range(opening, len(text)):
        depth += (text[index] == "{") - (text[index] == "}")
        if depth == 0:
            return text[start:index + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def test_bitmask_owner_and_two_argument_facility_predicate_are_direct():
    header = (ROOT / "src/general.h").read_text(encoding="utf-8")
    mapped = current_record_by_address(ROOT, 0x0050BA00)
    owner = mapped.path.read_text(encoding="utf-8")
    base_header = (ROOT / "src/base.h").read_text(encoding="utf-8")
    base_source = (ROOT / "src/base.cpp").read_text(encoding="utf-8")
    sources = [
        (ROOT / f"src/{name}").read_text(encoding="utf-8")
        for name in (
            "base.cpp", "console.cpp", "diplopop.cpp", "game.cpp",
            "map.cpp", "path.cpp", "popup.cpp", "probe.cpp",
            "terraforming.cpp", "veh.cpp",
        )
    ]

    assert header.count(BITMASK_SIGNATURE + ";") == 1
    assert "MEASURED inline " + BITMASK_SIGNATURE not in header
    source_text = header + owner + base_header + "".join(sources)
    code = re.sub(r"/\*.*?\*/|//[^\n]*", "", source_text, flags=re.S)
    assert "bitmask_call" not in code
    assert owner.count(BITMASK_SIGNATURE + " {") == 1
    bitmask = _definition(owner, BITMASK_SIGNATURE)
    assert (bitmask.index("const int shift = input & 7;")
            < bitmask.index("*offset = input / 8;")
            < bitmask.index("*mask = 1 << shift;"))
    assert "input >> 3" not in bitmask
    assert "if (" not in bitmask

    code = re.sub(r"/\*.*?\*/|//[^\n]*", "", base_header + "".join(sources), flags=re.S)
    assert "has_fac_built_call" not in code
    assert "static __forceinline int has_fac_built" not in base_header
    assert BUILT_SIGNATURE in base_header
    assert base_header.count(GUARDED_SIGNATURE + ";") == 1
    assert "inline " + GUARDED_SIGNATURE not in base_header
    assert "inline bool __cdecl has_fac_built(int facility_id, int base_id)" not in base_header
    assert "facility_id >= FacilityRepStart" in _definition(base_source, GUARDED_SIGNATURE)
    two_arg = _definition(base_header, BUILT_SIGNATURE)
    assert "return (Bases[base_id].facilities_built[offset] & mask) != 0;" in two_arg
    assert "bitmask(facility_id, &offset, &mask);" in two_arg

    assert mapped.image_spans == ((0x0050BA00, 0x0050BA27),)
    assert mapped.extract_error == ""
    assert BITMASK_SIGNATURE in mapped.region

    build = _definition(base_source, "int __cdecl base_build(")
    guarded = _definition(base_source, GUARDED_SIGNATURE)
    maint_start = base_source.index("void __cdecl base_maint()")
    assert base_source.count(GUARDED_SIGNATURE + " {") == 1
    assert base_source.index(build) + len(build) < base_source.index(guarded) < maint_start
    assert guarded == ("int __cdecl has_fac_built(int facility_id) {\n"
                      "    int base_id = BaseIDCurrentSelected;\n"
                      "    if (facility_id >= FacilityRepStart) {\n"
                      "        return false;\n"
                      "    }\n"
                      "    int mask;\n"
                      "    bitmask(facility_id, &facility_id, &mask);\n"
                      "    return (Bases[base_id].facilities_built[facility_id] & mask) != 0;\n"
                      "}")

    # The coordinator's native writer updates this path after source transport.
    # Keep this assertion explicit; the pre-writer worker snapshot is expected
    # to fail here rather than silently accepting the stale header binding.
    record, = (row for row in read_file(ROOT / "src/base.cpp")
               if row.address == 0x00500290)
    assert record.body == "src/base.cpp"
    assert record.image_spans == ((0x00500290, 0x005002DE),)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_bitmask_and_facility_predicate_alias_contract(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    mapped = current_record_by_address(ROOT, 0x0050BA00)
    owner = mapped.path.read_text(encoding="utf-8")
    base_header = (ROOT / "src/base.h").read_text(encoding="utf-8")
    base_source = (ROOT / "src/base.cpp").read_text(encoding="utf-8")
    bitmask = _definition(owner, BITMASK_SIGNATURE)
    two_arg = _definition(base_header, BUILT_SIGNATURE)
    one_arg = _definition(base_source, GUARDED_SIGNATURE)
    program = r'''#include <cassert>
#include <climits>
#include <stdint.h>
#include <type_traits>
#define __cdecl
struct BaseStub { uint8_t facilities_built[12]; };
BaseStub Bases[2] = {};
int BaseIDCurrentSelected = 0;
const int FacilityRepStart = 0x41;
BODY_BITMASK
BODY_TWO_ARG
BODY_ONE_ARG
static_assert(std::is_same<decltype(has_fac_built(1, 0)), int>::value,
              "two-argument predicate preserves its integer contract");
static_assert(std::is_same<decltype(has_fac_built(1)), int>::value,
              "one-argument guarded overload preserves int return ABI");
int main() {
    struct Case { int input, offset, mask; };
    const Case cases[] = {
        { INT_MIN, INT_MIN / 8, 1 },
        { INT_MAX, INT_MAX / 8, 128 },
        { -9, -1, 128 }, { -8, -1, 1 }, { -1, 0, 128 },
        { 0, 0, 1 }, { 7, 0, 128 }, { 8, 1, 1 },
    };
    for (const Case &item : cases) {
        int offset = 99, mask = 99;
        bitmask(item.input, &offset, &mask);
        assert(offset == item.offset && mask == item.mask);
    }

    // The input is by value even when its caller slot is also the offset out.
    int aliased_input = -9, mask = 0;
    bitmask(aliased_input, &aliased_input, &mask);
    assert(aliased_input == -1 && mask == 128);

    // Ordered writes are observable when both output pointers alias.
    int same_output = 0;
    bitmask(-9, &same_output, &same_output);
    assert(same_output == 128);

    Bases[0].facilities_built[0] = 0x81;
    assert(has_fac_built(0, 0));
    assert(has_fac_built(7, 0));
    assert(!has_fac_built(1, 0));
    Bases[0].facilities_built[8] = 2;
    assert(has_fac_built(0x41, 0)); // Two-arg predicate remains unguarded.
    BaseIDCurrentSelected = 0;
    assert(!has_fac_built(0x41)); // One-arg overload keeps its guard.
    assert(has_fac_built(7));
    Bases[0].facilities_built[2] = 0x08;
    assert(has_fac_built(19) == 1);
    assert(has_fac_built(18) == 0);
}
'''.replace("BODY_BITMASK", bitmask).replace("BODY_TWO_ARG", two_arg).replace(
        "BODY_ONE_ARG", one_arg)
    generated = tmp_path / "bitmask_owner.cpp"
    executable = tmp_path / f"bitmask-owner-{compiler_name}"
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


def test_pinned_bitmask_owner_and_direct_caller():
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
    mapped = current_record_by_address(ROOT, 0x0050BA00)
    low, high = mapped.image_spans[0]
    instructions = list(decoder.disasm(image.read(Span(low, high)), low))
    cursor = low
    for instruction in instructions:
        assert instruction.address == cursor
        cursor += instruction.size
    assert cursor == high
    assert [(ins.address, ins.mnemonic) for ins in instructions] == [
        (0x0050BA00, "push"), (0x0050BA01, "mov"),
        (0x0050BA03, "mov"), (0x0050BA06, "mov"),
        (0x0050BA08, "and"), (0x0050BA0B, "cdq"),
        (0x0050BA0C, "and"), (0x0050BA0F, "add"),
        (0x0050BA11, "mov"), (0x0050BA14, "sar"),
        (0x0050BA17, "mov"), (0x0050BA19, "mov"),
        (0x0050BA1E, "shl"), (0x0050BA20, "mov"),
        (0x0050BA23, "mov"), (0x0050BA25, "pop"),
        (0x0050BA26, "ret"),
    ]
    assert instructions[10].address == 0x0050BA17  # offset store precedes mask.
    assert instructions[14].address == 0x0050BA23

    caller_address = 0x00461341  # MainMenu::check direct-call witness.
    caller = next(ins for ins in decoder.disasm(
        image.read(Span(caller_address, caller_address + 5)), caller_address)
        if ins.address == caller_address)
    assert caller.mnemonic == "call"
    assert caller.operands[0].type == X86_OP_IMM
    assert caller.operands[0].imm == 0x0050BA00
