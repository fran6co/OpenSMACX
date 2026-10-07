"""Actual storage, ABI declarations and build inputs after owner migration."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
OWNERS = {
    "BufferStrHeight": ("buffer", "int", "", "009B3A9C"),
    "FilefindPath": ("general", "Filefind", "", "009B8198"),
    "ParseStrPlurality": ("general", "int", "[10]", "009BB570"),
    "ParseNumTable": ("general", "int", "[10]", "009BB598"),
    "ParseStrGender": ("general", "int", "[10]", "009BB5C0"),
    "ParseStrBuffer": ("general", "char256", "[10]", "009BB5E8"),
    "HandleMain": ("win", "HWND", "", "009B7B28"),
    "UnkBitfield1": ("game", "uint32_t", "", "0090D91C"),
}


def test_owner_storage_and_historical_bindings_are_unique():
    sources = {p.name: p.read_text() for p in (ROOT / "src").glob("*.cpp")}
    for name, (owner, ctype, suffix, address) in OWNERS.items():
        pattern = r"^" + re.escape(ctype + " " + name + suffix) + r";[^\n]*$"
        definitions = [(unit, line) for unit, text in sources.items()
                       for line in re.findall(pattern, text, re.M)]
        assert len(definitions) == 1, (name, definitions)
        assert definitions[0][0] == owner + ".cpp"
        assert "0x" + address in definitions[0][1]
        assert ("extern " + ctype + " " + name + suffix + ";") in (
            ROOT / "src" / (owner + ".h")).read_text()
    # Only these three objects already had source bindings. Moving storage
    # must preserve them, not fabricate bindings for the other five globals.
    for name in ("BufferStrHeight", "FilefindPath", "HandleMain"):
        owner, _, _, address = OWNERS[name]
        matches = [(unit, line) for unit, text in sources.items()
                   for line in text.splitlines()
                   if line.startswith("// RECOVERY_OBJECT: v1 0x" + address + " ")]
        assert len(matches) == 1
        assert matches[0][0] == owner + ".cpp"
        assert "?" + name + "@@" in matches[0][1]


def test_no_retired_build_or_include_inputs():
    for name in ("temp.cpp", "temp.h"):
        assert not (ROOT / "src" / name).exists()
    assert "src/temp.cpp" not in (ROOT / "CMakeLists.txt").read_text()
    for path in (ROOT / "src").rglob("*"):
        if path.suffix in (".cpp", ".h"):
            assert not re.search(r'^\s*#\s*include\s*[<"]temp\.h[>"]',
                                 path.read_text(), re.M), path


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_parse_and_path_storage_layout(tmp_path, compiler):
    compiler = shutil.which(compiler)
    if compiler is None:
        pytest.skip("host compiler unavailable")
    header = (ROOT / "src/general.h").read_text()
    filefind, = re.findall(r"struct Filefind\s*\{[^}]*\};", header, re.S)
    char256, = re.findall(r"typedef struct char256\s*\{[^}]*\}\s*char256;", header, re.S)
    text = (ROOT / "src/general.cpp").read_text()
    definitions = []
    for name in ("FilefindPath", "ParseStrPlurality", "ParseNumTable", "ParseStrGender", "ParseStrBuffer"):
        _, ctype, suffix, _ = OWNERS[name]
        line, = re.findall(r"^" + re.escape(ctype + " " + name + suffix) + r";[^\n]*$", text, re.M)
        definitions.append(line)
    fixture = '#include <cassert>\n#include <cstddef>\n' + filefind + '\n' + char256 + '\n'
    fixture += '\n'.join(definitions) + r'''
static_assert(sizeof(Filefind) == 0x500, "five path buffers");
static_assert(offsetof(Filefind, last_path) == 0x200, "native member offset");
static_assert(sizeof(char256) == 0x100, "parse slot stride");
static_assert(sizeof(ParseStrBuffer) == 0xA00, "ten parse slots");
int main() {
    for (int i = 0; i < 10; ++i) {
        assert(ParseNumTable[i] == 0 && ParseStrPlurality[i] == 0 && ParseStrGender[i] == 0);
        for (int j = 0; j < 256; ++j) assert(ParseStrBuffer[i].str[j] == 0);
    }
    assert(FilefindPath.last_path[0] == 0);
}
'''
    path = tmp_path / "storage.cpp"
    path.write_text(fixture)
    binary = tmp_path / "storage"
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(path), "-o", str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
