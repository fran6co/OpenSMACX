"""Actual-source controls for rainfall live cloud reloads and entry order."""
from importlib.util import module_from_spec, spec_from_file_location
from pathlib import Path
import shutil

import pytest


ROOT = Path(__file__).resolve().parents[2]
MAINTAINED = ROOT / "recovery/tests/test_world_rainfall_behavior.py"
SPEC = spec_from_file_location("maintained_world_rainfall", MAINTAINED)
OWNER = module_from_spec(SPEC)
SPEC.loader.exec_module(OWNER)


def live_cloud_host():
    host = OWNER.HOST
    host = host.replace(
        "static int climate_calls;",
        "static int climate_calls;\nstatic int mutate_cloud_after_first_row;",
        1,
    )
    host = host.replace(
        "void do_all_non_input() { ++progress_calls; }",
        "void do_all_non_input() {\n"
        "    ++progress_calls;\n"
        "    if (mutate_cloud_after_first_row && progress_calls == 1)\n"
        "        MapCloudCover = 2;\n"
        "}",
        1,
    )
    host = host.replace(
        "    UnkBitfield1 = 0;\n}",
        "    UnkBitfield1 = 0;\n"
        "    mutate_cloud_after_first_row = 0;\n}",
        1,
    )
    scenario = r'''
    // Network/input work after row zero changes the shared climate setting.
    // The pinned next-row x+4 gate reloads it and emits scratch hint 0x80.
    reset_world();
    MapLongitudeBounds = 8;
    MapLongitude = 4;
    MapLatitudeBounds = 3;
    MapArea = 12;
    for (i = 0; i < 12; ++i) {
        Tiles[i].climate = 0x60;
        Tiles[i].unk_1 = 0;
    }
    Tiles[4].climate = 0x40; // y=1, logical x=1: altitude two
    MapCloudCover = 0;
    mutate_cloud_after_first_row = 1;
    world_rainfall();
    if (MapCloudCover != 2)
        failures += fail("first-row callback updates cloud-cover fixture");
    if ((Tiles[6].unk_1 & 0x80) == 0)
        failures += fail("later-row x+4 hint reloads live cloud cover");
'''
    anchor = "    if (failures != 0)\n        return 1;"
    assert host.count(anchor) == 1
    host = host.replace(anchor, scenario + "\n" + anchor, 1)
    assert host.count("mutate_cloud_after_first_row = 1;") == 1
    return host


def test_entry_resets_follow_pinned_store_order_once():
    source = OWNER.SOURCE.read_text()
    body = OWNER.extract(source, OWNER.OWNER_SIGNATURE)
    drowned = "TerraformDrownedBaseId = -1;"
    faction = "TerraformTileFaction = -1;"
    assert body.count(drowned) == 1
    assert body.count(faction) == 1
    assert body.index(drowned) < body.index(faction)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_source_observes_cloud_cover_change_after_row_callback(
        monkeypatch, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is unavailable")
    monkeypatch.setattr(OWNER, "HOST", live_cloud_host())
    result = OWNER.run(compiler)
    assert result.returncode == 0, result.stdout + result.stderr
    assert "later-row x+4 hint reloads live cloud cover" not in result.stderr


def snapshot_mutant_source(source):
    body = OWNER.extract(source, OWNER.OWNER_SIGNATURE)
    if "const int cloud_cover = static_cast<int>(MapCloudCover);" in body:
        # The current recovered source has the defect already; preserve this
        # exact product body as the stale-snapshot mutant.
        return source
    body = body.replace("MapCloudCover", "cloud_cover")
    body = body.replace(
        "void __cdecl world_rainfall() {",
        "void __cdecl world_rainfall() {\n"
        "    const int cloud_cover = static_cast<int>(MapCloudCover);",
        1,
    )
    old_body = OWNER.extract(source, OWNER.OWNER_SIGNATURE)
    assert source.count(old_body) == 1
    source = source.replace(old_body, body, 1)
    return source


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_old_entry_snapshot_mutant_is_caught(monkeypatch, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is unavailable")
    monkeypatch.setattr(OWNER, "HOST", live_cloud_host())
    source = OWNER.SOURCE.read_text()

    class MutatedSource:
        def read_text(self):
            return snapshot_mutant_source(source)

    monkeypatch.setattr(OWNER, "SOURCE", MutatedSource())
    result = OWNER.run(compiler)
    assert result.returncode != 0
    assert "FAIL: later-row x+4 hint reloads live cloud cover" in result.stderr
