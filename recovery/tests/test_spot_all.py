"""Actual mapped spot_all body controls, not native ABI/game acceptance."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from recovery.tests.test_pact_ascent import definition

ROOT = Path(__file__).resolve().parents[2]
SIGNATURE = "void __cdecl spot_all("


def actual_owner():
    product = (ROOT / "src/veh.cpp").read_text(encoding="utf-8")
    start = product.index("\n" + SIGNATURE) + 1
    owner = definition(product[start:], re.escape(SIGNATURE))
    assert "SpotAllSourceContext" not in owner
    assert "inspect_spot_all_source(" not in owner
    assert "scan_spot_all_" not in owner
    assert len(owner) > 10000
    return owner


def _compile_and_run(tmp_path, compiler, fixture):
    source = tmp_path / "spot_all_control.cpp"
    binary = tmp_path / "spot_all_control"
    source.write_text(fixture, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
         "-I", str(Path(__file__).parent), str(source), "-o", str(binary)],
        capture_output=True, text=True, timeout=45)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=15)


@pytest.mark.parametrize("name", ["g++", "clang++"])
def test_actual_spot_all_and_damage(tmp_path, name):
    compiler = shutil.which(name)
    if not compiler:
        pytest.skip(name + " unavailable")
    fixture = Path(__file__).with_name("spot_all_control.cpp").read_text(
        encoding="utf-8")
    assert fixture.count("// @@ACTUAL_SPOT_ALL_OWNER@@") == 1
    owner = actual_owner()
    template = fixture.replace("// @@ACTUAL_SPOT_ALL_OWNER@@", owner)
    good = _compile_and_run(tmp_path, compiler, template)
    assert good.returncode == 0, (good.stdout, good.stderr)

    mutants = (
        ("WorldClimateWorldWin.GraphicWin::redraw();",
         "WorldClimateWorldWin.on_redraw();"),
        ("(PlayersData[source_faction_id].diplo_treaties[sensor_faction] &\n"
         "             7u) == 0",
         "(PlayersData[source_faction_id].diplo_treaties[sensor_faction] &\n"
         "             1u) == 0"),
        ("spot_tile(x, y, source_faction_id);",
         "spot_tile(source_x, source_y, source_faction_id);"),
    )
    for old, new in mutants:
        assert owner.count(old) == 1, (old, owner.count(old))
        damaged = owner.replace(old, new, 1)
        mutant_fixture = fixture.replace("// @@ACTUAL_SPOT_ALL_OWNER@@", damaged)
        result = _compile_and_run(tmp_path, compiler, mutant_fixture)
        assert result.returncode != 0, (old, result.stdout, result.stderr)
