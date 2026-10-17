"""Host controls for the shared graphics-language owner, not VC6 ABI or game acceptance."""

from pathlib import Path
import json
import hashlib
import os
import re
import shutil
import subprocess
import textwrap

import pytest

from match_recovery.model import Span
from match_recovery.object_source import read_objects
from match_recovery.pe_coff import PeImage
from match_recovery.source import read_file
from match_recovery.source.asm import Listing
from match_recovery.source.calls import call_sites
from match_recovery.source_inventory import image_pin, source_files


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
SETTER = (0x00627100, 0x0062710A)
GLOBAL = 0x009BC054


def test_shared_source_mapping_and_jackal_archive_membership():
    source = ROOT / "src/jackal/graphics_language.cpp"
    rows = [row for row in read_file(source) if row.address == SETTER[0]]
    assert len(rows) == 1
    setter, = rows
    assert setter.name == "?set_language@@YAXH@Z"
    assert setter.image_spans == (SETTER,)
    assert setter.byte_exact
    owners = [row for path in source_files(ROOT) for row in read_file(path)
              if row.address == SETTER[0]]
    assert len(owners) == 1 and owners[0].path == source
    assert not any(row.address == SETTER[0]
                   for row in read_file(ROOT / "src/alpha.cpp"))

    objects = [row for row in read_objects(ROOT, PIN)
               if row.origin.address == GLOBAL]
    assert len(objects) == 1
    language, = objects
    assert language.symbol == "?Language@@3IA"
    assert language.unit == "src/jackal/graphics_language.cpp"

    header = (ROOT / "src/jackal/graphics_language.h").read_text()
    assert "void __cdecl set_language(int language);" in header
    assert "inline" not in header and "__declspec" not in header
    implementation = source.read_text()
    assert "void __cdecl set_language(int language)" in implementation
    assert "Language = language;" in implementation
    cmake = (ROOT / "src/jackal/CMakeLists.txt").read_text()
    library = re.search(r"add_library\(jackal STATIC(.*?)\)", cmake, re.S)
    assert library is not None and "graphics_language.cpp" in library.group(1).split()
    build = json.loads((ROOT / "recovery/build.json").read_text())
    members = [row for row in build["compile_link"]["archive_members"]
               if row["object"] == "src/jackal/CMakeFiles/jackal.dir/graphics_language.cpp.obj"]
    assert members == [{
        "archive": "src/jackal/jackal.lib",
        "object": "src/jackal/CMakeFiles/jackal.dir/graphics_language.cpp.obj",
        "member": "src/jackal/CMakeFiles/jackal.dir/graphics_language.cpp.obj",
    }]

    alpha_h = (ROOT / "src/alpha.h").read_text()
    assert "set_language(int language) {" not in alpha_h
    assert "set_language(prefs_get(\"Language\", 0, false));" in (ROOT / "src/alpha.cpp").read_text()
    assert "set_language(prefs_get(\"Language\", 0, 0));" in (ROOT / "src/game.cpp").read_text()


def test_pinned_setter_bytes_and_original_caller_edges():
    image_path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file():
        pytest.skip("pinned original image is unavailable")
    assert image_pin(ROOT) == PIN
    contents = image_path.read_bytes()
    assert hashlib.sha256(contents).hexdigest() == PIN
    image = PeImage(contents, PIN)

    raw = image.read(Span(*SETTER))
    assert len(raw) == 10
    listing = Listing(raw, SETTER[0])
    assert sum(insn.size for insn in listing.instructions) == len(raw)
    assert listing.instructions[0].address == SETTER[0]
    assert listing.instructions[-1].address + listing.instructions[-1].size == SETTER[1]
    assert listing.mnemonics == ("mov", "mov", "ret")
    assert sum(instruction.size for instruction in listing.instructions) == len(raw)
    first, second, ret = listing.instructions
    from capstone.x86_const import X86_OP_MEM, X86_OP_REG, X86_REG_EAX, X86_REG_ESP
    assert first.operands[0].type == X86_OP_REG and first.operands[0].reg == X86_REG_EAX
    assert first.operands[1].type == X86_OP_MEM
    assert first.operands[1].mem.base == X86_REG_ESP and first.operands[1].mem.disp == 4
    assert second.operands[0].type == X86_OP_MEM and second.operands[0].mem.disp == GLOBAL
    assert second.operands[1].type == X86_OP_REG and second.operands[1].reg == X86_REG_EAX
    assert ret.mnemonic == "ret"

    callers = ((ROOT / "src/alpha.cpp", 0x0059DCF0, 0x0059DD5A),
               (ROOT / "src/game.cpp", 0x0058F040, 0x0058F0A1))
    for path, address, call_address in callers:
        record, = (row for row in read_file(path) if row.address == address)
        assert record.image_spans
        found = []
        for low, high in record.image_spans:
            caller_listing = Listing(image.read(Span(low, high)), low)
            assert sum(instruction.size for instruction in caller_listing.instructions) == high - low
            found.extend(call_sites(caller_listing))
        assert [(site.at, site.target, site.form) for site in found
                if site.at == call_address] == [(call_address, SETTER[0], "direct")]


HOST_DRIVER = textwrap.dedent(r"""
    #include "graphics_language.h"
    #include <limits.h>
    #include <stdint.h>
    #include <string.h>

    static_assert(sizeof(int) == sizeof(uint32_t), "host test requires 32-bit int");

    int main() {
        const int values[] = {0, -1, -7, INT_MIN, INT_MAX};
        for (unsigned i = 0; i < sizeof(values) / sizeof(values[0]); ++i) {
            const uint32_t expected = static_cast<uint32_t>(values[i]);
            Language = 0xA5A5A5A5u;
            set_language(values[i]);
            if (Language != expected) return 71;
        }
        return 0;
    }
""").lstrip()


def _compile_and_run(compiler, source, driver, output, include_dir):
    command = [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", "-pedantic", "-D__cdecl=", "-I", str(include_dir),
               str(source), str(driver), "-o", str(output)]
    built = subprocess.run(command, text=True, capture_output=True, timeout=45)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(output)], capture_output=True, timeout=15).returncode


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_host_compile_executes_real_setter_and_rejects_compiled_mutations(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    source = ROOT / "src/jackal/graphics_language.cpp"
    include_dir = ROOT / "src/jackal"
    driver = tmp_path / "driver.cpp"
    driver.write_text(HOST_DRIVER)

    # Positive-first: compile the checked-in implementation in a separate TU,
    # then execute signed edge cases and their exact uint32_t object bits.
    assert _compile_and_run(compiler, source, driver, tmp_path / "actual", include_dir) == 0

    original = source.read_text()
    assignment = "Language = language;"
    assert original.count(assignment) == 1
    wrong_value = tmp_path / "wrong_value.cpp"
    wrong_value.write_text(original.replace(assignment, "Language = language ^ 1;", 1))
    assert _compile_and_run(compiler, wrong_value, driver,
                            tmp_path / "wrong_value", include_dir) == 71

    wrong_storage = tmp_path / "wrong_storage.cpp"
    wrong_storage.write_text(original.replace(
        assignment,
        "static uint32_t OtherLanguage;\n    OtherLanguage = language;\n    (void)OtherLanguage;", 1))
    assert _compile_and_run(compiler, wrong_storage, driver,
                            tmp_path / "wrong_storage", include_dir) == 71
