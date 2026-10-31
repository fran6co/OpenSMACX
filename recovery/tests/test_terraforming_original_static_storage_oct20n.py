"""The real Terraforming aggregate begins with the pinned static bit fields."""
from __future__ import annotations

import os
from pathlib import Path
import re
import shutil
import struct
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
IMAGE_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
IMAGE_DEFAULT = ROOT / ".opensmacx/game/terranx_original.exe"
TERRAFORMING_SPAN = (0x00691878, 0x00691AF8)


def _balanced_block(source: str, start: int) -> str:
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError("unterminated source initializer")


def _source_initializer(source: str) -> str:
    match = re.search(
        r"RulesTerraforming\s+Terraforming\s*\[\s*MaxTerrainNum\s*\]\s*=\s*\{",
        source,
    )
    assert match, "the actual Terraforming global must have a static aggregate initializer"
    return _balanced_block(source, match.start())


def _enum_block(source: str) -> str:
    match = re.search(r"enum\s+TerrainBit\s*\{.*?\};", source, re.S)
    assert match, "the fixture must use the actual symbolic TerrainBit constants"
    return match.group(0)


def _fixture(initializer: str, *, damage: str | None = None) -> str:
    map_header = (ROOT / "src/map.h").read_text(encoding="utf-8")
    enum = _enum_block(map_header)
    if damage:
        if damage == "terrain-bit":
            before, after = "BIT_FARM,          BIT_FOREST", "0,                 BIT_FOREST"
        else:
            assert damage == "incompatible-bit"
            before = "BIT_MINE | BIT_SOLAR_TIDAL | BIT_FOREST | BIT_CONDENSER | BIT_ECH_MIRROR"
            after = "BIT_MINE | BIT_SOLAR_TIDAL | BIT_FOREST | BIT_CONDENSER"
        assert initializer.count(before) >= 1
        initializer = initializer.replace(before, after, 1)
    return f'''#include <stdint.h>
#include <stdio.h>
{enum}
typedef char *LPSTR;
struct RulesTerraforming {{
    LPSTR name;
    LPSTR name_sea;
    int prerequisite_tech[2];
    uint32_t bit;
    uint32_t bit_incompatible;
    int rate;
    LPSTR shortcuts;
}};
static const int MaxTerrainNum = 20;
{initializer};
int main() {{
    for (int i = 0; i < MaxTerrainNum; ++i) {{
        const RulesTerraforming &row = Terraforming[i];
        if (row.name || row.name_sea || row.prerequisite_tech[0] || row.prerequisite_tech[1] ||
            row.rate || row.shortcuts) return 2;
        printf("%08x %08x\\n", (unsigned int)row.bit,
               (unsigned int)row.bit_incompatible);
    }}
    return 0;
}}
'''


def _run_fixture(compiler: str, tmp_path: Path, initializer: str, damage: str | None = None) -> bytes:
    source = tmp_path / "terraforming.cpp"
    binary = tmp_path / "terraforming"
    source.write_text(_fixture(initializer, damage=damage), encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(binary)],
        capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, timeout=10)
    assert ran.returncode == 0, ran.stdout.decode(errors="replace") + ran.stderr.decode(errors="replace")
    return ran.stdout


def _serialized_rows(output: bytes) -> bytes:
    rows = [line.split() for line in output.decode("ascii").splitlines()]
    assert len(rows) == 20
    assert all(len(row) == 2 for row in rows)
    # The host fixture intentionally has 64-bit pointers. It validates the live
    # initializer's fields and constructs the original 32-bit record bytes from
    # those fields; it is not proof of the native compiler's layout or storage.
    return b"".join(
        struct.pack("<8I", 0, 0, 0, 0, int(bit, 16), int(incompatible, 16), 0, 0)
        for bit, incompatible in rows
    )


def test_actual_static_aggregate_matches_all_pinned_initial_record_bytes(tmp_path):
    source = (ROOT / "src/alpha.cpp").read_text(encoding="utf-8")
    initializer = _source_initializer(source)
    assert initializer.count("{ 0,") >= 20
    assert "Terraforming[i].bit = TerraformingBits[i][j]" in source
    assert "const uint32_t TerraformingBits[20][2]" in (ROOT / "src/terraforming.h").read_text()

    image_path = Path(os.environ.get("OPENSMACX_IMAGE", IMAGE_DEFAULT))
    if not image_path.is_file():
        pytest.skip("pinned original image unavailable")
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    image = PeImage(image_path.read_bytes(), IMAGE_SHA256)
    original = image.read_initial(Span(*TERRAFORMING_SPAN))
    assert len(original) == 20 * 0x20

    compilers = [path for name in ("g++", "clang++") if (path := shutil.which(name))]
    assert compilers, "at least one host C++ compiler is required for the source fixture"
    for compiler in compilers:
        output = _run_fixture(compiler, tmp_path, initializer)
        assert _serialized_rows(output) == original


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("damage", ["terrain-bit", "incompatible-bit"])
def test_changed_initialized_bit_is_detected(compiler_name, damage, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} is unavailable")
    source = (ROOT / "src/alpha.cpp").read_text(encoding="utf-8")
    initializer = _source_initializer(source)
    intact = _serialized_rows(_run_fixture(compiler, tmp_path, initializer))
    damaged = _serialized_rows(_run_fixture(compiler, tmp_path, initializer, damage=damage))
    assert intact != damaged
    offset = 0x10 if damage == "terrain-bit" else 0x54
    assert struct.unpack_from("<I", intact, offset)[0] != struct.unpack_from("<I", damaged, offset)[0]
