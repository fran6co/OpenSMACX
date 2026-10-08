"""Actual-source relocation width controls; external doubles are not native proof."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

import pytest

from recovery.tests import test_goody_box as goody_fixture

OWNER = Path(__file__).resolve().parents[2] / "src" / "veh.cpp"
OLD = "x_distance = MapLongitude - x_distance;"
NEW = "x_distance = MapLongitudeBounds - x_distance;"


def _function(source: str, pattern: str, label: str) -> str:
    match = re.search(pattern, source, re.S)
    assert match, f"actual {label} helper missing"
    brace = source.find("{", match.start(), match.end())
    return goody_fixture._block(source, brace, match.start(), label)


def _candidate_source() -> tuple[str, str]:
    source = OWNER.read_text()
    namespace_source = goody_fixture._extract_namespace(source)
    patterns = (
        (r"\bstatic\s+Map\s*\*\s*tile_at\s*\([^)]*\)\s*\{", "tile_at"),
        (r"\bstatic\s+int\s+on_map\s*\([^)]*\)\s*\{", "on_map"),
        (r"\bstatic\s+int\s+relocation_random_below\s*\([^)]*\)\s*\{", "relocation_random_below"),
        (r"\bstatic\s+uint32_t\s+normalized_crt_mask_draw\s*\([^)]*\)\s*\{", "normalized_crt_mask_draw"),
        (r"\bstatic\s+int\s+relocate_vehicle\s*\([^)]*\)\s*\{", "relocate_vehicle"),
    )
    functions = [_function(namespace_source, pattern, label)
                 for pattern, label in patterns]
    namespace = "namespace goody_box_detail {\n" + "\n".join(functions) + "\n}"
    helper = functions[-1]
    assert helper.count(NEW) == 1, "expected corrected width expression"
    assert OLD not in helper
    return namespace, helper


MAIN = r'''
int main() {
    reset(0x60, 0, 0);
    MapLongitudeBounds = 100;
    MapLongitude = 50;
    MapLatitudeBounds = 100;
    MapIsFlat = 0;
    Vehs[0].x = 0; Vehs[0].y = 0;
    Vehs[0].proto_id = 0;
    VehPrototypes[0].chassis_id = 0;
    VehPrototypes[0].plan = 0;
    Chassis[0].triad = 0;
    MapTiles[0].region = 2;

    // Candidate 1 crosses the full-width seam. Native arithmetic uses
    // 100-96=4, so |dy|+dx=48 and radius=24 accepts it. A bad 50-96=-46
    // subtraction rounds to a close-site rejection and falls through.
    // Candidate 2 is a distinct non-seam fallback, also within radius.
    random_values[0] = 96; random_values[1] = 44;
    random_values[2] = 20; random_values[3] = 28;
    random_count = 4;
    MapTiles[44 * MapLongitude + (96 >> 1)].climate = 0x60;
    MapTiles[44 * MapLongitude + (96 >> 1)].region = 2;
    MapTiles[28 * MapLongitude + (20 >> 1)].climate = 0x60;
    MapTiles[28 * MapLongitude + (20 >> 1)].region = 2;

    assert(goody_box_detail::relocate_vehicle(0, 0, 0, 0, 0, 1) == 1);
    assert(random_cursor == 2);
    assert(last_stack_x == 96 && last_stack_y == 44);
    assert(strcmp(trace, "NVTFXWS") == 0);
    assert(Vehs[0].moves_expended == 0 && Vehs[0].order == 0);
    return 0;
}
'''


def _compilers():
    found = [shutil.which(name) for name in ("g++", "clang++")]
    found = [compiler for compiler in found if compiler]
    if not found:
        pytest.skip("neither GCC nor Clang is installed")
    return found


def _compile_helper(compiler, namespace, main, expected_run=0):
    # Reuse maintained external doubles, not the full owner's diagnostic
    # contract: this helper has neither of its two signed ranking comparisons.
    fixture = goody_fixture.FIXTURE_CPP.replace(
        "/* ACTUAL_GAME_STORAGE */",
        goody_fixture.current_game_storage_fixture(scenario=True))
    with tempfile.TemporaryDirectory(prefix="goody-width-control-") as temporary:
        source = Path(temporary) / "relocate.cpp"
        binary = Path(temporary) / "relocate"
        source.write_text(fixture + namespace + "\n" + main)
        compiled = subprocess.run(
            [compiler, "-std=c++14", "-Wall", "-Wextra", "-Werror",
             "-Wno-parentheses", "-fsanitize=undefined", "-fno-sanitize-recover=all",
             str(source), "-o", str(binary)], capture_output=True, text=True, timeout=30)
        assert compiled.returncode == 0, compiled.stderr
        result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
        if expected_run is None:
            assert result.returncode != 0, "half-width mutant unexpectedly passed"
        else:
            assert result.returncode == expected_run, result.stderr


def test_actual_relocator_uses_full_width_and_keeps_callback_path():
    namespace, helper = _candidate_source()
    assert NEW in helper
    for compiler in _compilers():
        _compile_helper(compiler, namespace, MAIN)


def test_half_stride_subtraction_mutant_changes_selected_candidate():
    namespace, helper = _candidate_source()
    assert helper.count(NEW) == 1
    mutant = helper.replace(NEW, OLD, 1)
    namespace = namespace.replace(helper, mutant, 1)
    for compiler in _compilers():
        _compile_helper(compiler, namespace, MAIN, expected_run=None)
