"""Bounded direct-source controls for Console::terraform's reviewed gaps."""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "console.cpp"


def function_body(text: str) -> str:
    marker = "void Console::terraform(int vehicle_id, int requested_terraform)"
    start = text.index(marker)
    opening = text.index("{", start)
    depth = 0
    for offset in range(opening, len(text)):
        if text[offset] == "{":
            depth += 1
        elif text[offset] == "}":
            depth -= 1
            if depth == 0:
                return text[opening + 1 : offset]
    raise AssertionError("unterminated Console::terraform body")


def source_block(text: str, start: str, end: str) -> str:
    body = function_body(text)
    return body[body.index(start) : body.index(end, body.index(start))]


def compile_run(tmp_path: Path, source: str, compiler: str) -> subprocess.CompletedProcess[str]:
    cpp = tmp_path / "control.cpp"
    exe = tmp_path / "control"
    cpp.write_text(source)
    returncode = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        text=True,
        capture_output=True,
        check=False,
    )
    assert returncode.returncode == 0, returncode.stdout + returncode.stderr
    return subprocess.run([str(exe)], text=True, capture_output=True, check=False)


def compiler_paths() -> list[str]:
    found = [shutil.which(name) for name in ("g++", "clang++")]
    paths = [path for path in found if path]
    if len(paths) != 2:
        pytest.skip("both host GCC and Clang are required for this control")
    return paths


AUTO_START = "    int terraform_id = requested_terraform;"
AUTO_END = "    // Native auto-selection only rewrites the ordinary road/farm requests;"
STEPS_START = "        int steps = 1;"
STEPS_END = "        int terraform_offset;"


def auto_program(text: str) -> str:
    block = source_block(text, AUTO_START, AUTO_END)
    return r"""
struct Tile { unsigned int bit; };
struct TerraformEntry { int preq_tech; int preq_tech_sea; };
enum { TERRA_FARM=0, TERRA_SOIL_ENR=1, TERRA_ROAD=5, TERRA_MAGTUBE=6,
       TERRA_REMOVE_FUNGUS=10 };
enum { BIT_ROAD=4, BIT_FUNGUS=0x20, BIT_FARM=0x8000 };
static TerraformEntry Terraforming[20];
int choose(int requested_terraform, unsigned int bits, int altitude, int is_sea,
           int mag_land, int mag_sea, int soil_land, int soil_sea) {
    Tile storage; storage.bit = bits; Tile *tile = &storage;
    Terraforming[TERRA_MAGTUBE].preq_tech = mag_land;
    Terraforming[TERRA_MAGTUBE].preq_tech_sea = mag_sea;
    Terraforming[TERRA_SOIL_ENR].preq_tech = soil_land;
    Terraforming[TERRA_SOIL_ENR].preq_tech_sea = soil_sea;
""" + block + r"""
    return terraform_id;
}
int main() {
    if (choose(TERRA_ROAD, BIT_ROAD, 0x60, 0, -1, -2, -2, -2) != TERRA_MAGTUBE) return 1;
    if (choose(TERRA_ROAD, BIT_ROAD, 0x60, 1, -2, -1, -2, -2) != TERRA_MAGTUBE) return 2;
    if (choose(TERRA_ROAD, BIT_ROAD, 0x60, 0, -2, -2, -2, -2) != TERRA_ROAD) return 3;
    if (choose(TERRA_FARM, BIT_FUNGUS, 0x3f, 0, -2, -2, -2, -2) != TERRA_FARM) return 4;
    if (choose(TERRA_FARM, BIT_FUNGUS, 0x40, 0, -2, -2, -2, -2) != TERRA_REMOVE_FUNGUS) return 5;
    if (choose(TERRA_FARM, BIT_FUNGUS|BIT_FARM, 0x60, 0, -2, -2, -1, -2) != TERRA_REMOVE_FUNGUS) return 6;
    if (choose(TERRA_FARM, BIT_FARM, 0x5f, 0, -2, -2, -1, -2) != TERRA_FARM) return 7;
    if (choose(TERRA_FARM, BIT_FARM, 0x60, 0, -2, -2, -1, -2) != TERRA_SOIL_ENR) return 8;
    if (choose(TERRA_FARM, BIT_FARM, 0x60, 1, -2, -2, -2, -1) != TERRA_SOIL_ENR) return 9;
    if (choose(TERRA_FARM, BIT_FARM, 0x60, 0, -2, -2, -2, -2) != TERRA_FARM) return 10;
    return 0;
}
"""


def steps_program(text: str) -> str:
    block = source_block(text, STEPS_START, STEPS_END)
    return r"""
enum { TERRA_RAISE_LAND=16, TERRA_LOWER_LAND=17 };
static const int RadiusRange[9] = {1,9,25,49,81,121,169,225,289};
struct Tile { unsigned char climate; };
int count_steps(int terraform_id, unsigned char climate) {
    Tile storage; storage.climate = climate; Tile *tile = &storage;
    (void)tile;
    const int altitude = 0;
    (void)altitude;
""" + block + r"""
    return steps;
}
int expected(int id, unsigned char climate) {
    int level = (climate >> 5);
    if (id == TERRA_RAISE_LAND) {
        ++level;
        if (level > 6) level = 6;
        return level > 3 ? RadiusRange[level - 3] : 1;
    }
    --level;
    if (level < 0) level = 0;
    return level < 3 ? RadiusRange[4 - level] : 1;
}
int main() {
    for (int climate = 0; climate <= 255; ++climate) {
        if (count_steps(TERRA_RAISE_LAND, (unsigned char)climate) !=
            expected(TERRA_RAISE_LAND, (unsigned char)climate)) return 1;
        if (count_steps(TERRA_LOWER_LAND, (unsigned char)climate) !=
            expected(TERRA_LOWER_LAND, (unsigned char)climate)) return 2;
    }
    return 0;
}
"""


@pytest.mark.parametrize("compiler", compiler_paths())
def test_auto_selection_is_extracted_from_actual_method(tmp_path: Path, compiler: str) -> None:
    result = compile_run(tmp_path, auto_program(SOURCE.read_text()), compiler)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler", compiler_paths())
def test_radius_step_counts_match_pinned_byte_domain(tmp_path: Path, compiler: str) -> None:
    result = compile_run(tmp_path, steps_program(SOURCE.read_text()), compiler)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler", compiler_paths())
def test_old_radius_indices_are_rejected(tmp_path: Path, compiler: str) -> None:
    text = SOURCE.read_text()
    old = text.replace("RadiusRange[level - 3]", "RadiusRange[level - 4]")
    old = old.replace("steps = RadiusRange[3];", "steps = RadiusRange[2];")
    old = old.replace("RadiusRange[4 - level]", "RadiusRange[3 - level]")
    assert old != text
    result = compile_run(tmp_path, steps_program(old), compiler)
    assert result.returncode != 0


@pytest.mark.parametrize("compiler", compiler_paths())
def test_step_path_reloads_climate_instead_of_using_entry_snapshot(
    tmp_path: Path, compiler: str
) -> None:
    text = SOURCE.read_text()
    current = steps_program(text)
    cached = current.replace("tile->climate", "altitude")
    assert cached != current
    result = compile_run(tmp_path, cached, compiler)
    assert result.returncode != 0


def test_final_conflict_bit_test_uses_the_reloaded_map_tile() -> None:
    body = function_body(SOURCE.read_text())
    begin = body.index("Map *const final_tile = map_loc(x, y);")
    end = body.index("if (NetDaemonNet.lock_state_", begin)
    final_scan = body[begin:end]
    def has_reload(scan: str) -> bool:
        return (
            "const int final_altitude = final_tile->climate & 0xE0;" in scan
            and "Terraforming[terraform_id].bit_incompatible & final_tile->bit" in scan
        )

    assert has_reload(final_scan)
    cached = final_scan.replace("final_tile->climate", "altitude")
    assert not has_reload(cached)


@pytest.mark.parametrize("compiler", compiler_paths())
def test_auto_selection_threshold_mutants_are_rejected(tmp_path: Path, compiler: str) -> None:
    text = SOURCE.read_text()
    original = auto_program(text)
    mutant = original.replace("altitude >= 0x40", "altitude > 0x40")
    assert mutant != original
    result = compile_run(tmp_path, mutant, compiler)
    assert result.returncode != 0

    mutant = original.replace(") >= -1)", ") > -1)")
    assert mutant != original
    result = compile_run(tmp_path, mutant, compiler)
    assert result.returncode != 0
