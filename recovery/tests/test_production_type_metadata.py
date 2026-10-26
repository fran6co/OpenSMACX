"""Guard the narrowly scoped production CodeView metadata recipes."""

from __future__ import annotations

from collections import Counter
import copy
import subprocess
from pathlib import Path

from match_recovery.measurement import Measurement
from test_inline_boundary_cohort_oct18 import (
    _effective_source_profiles,
    _global_compiler_link_policy,
    _load_json,
    _source_profiles,
)


ROOT = Path(__file__).resolve().parents[2]
BASE_REVISION = "8f5cf3d89838c277a1d0b22cf56b1bce951215fc"
METADATA_CHECKPOINT = "92d17311d7b80a7ef3adf8db08108ed837f4809c"
OWNERS = {"src/game.cpp", "src/netdaemon.cpp", "src/mapwin.cpp"}
TARGET_RECIPE = ["/c", "/O2", "/Gy", "/GR-", "/GX", "/Oy-", "/Ob2", "/Z7"]
GLOBAL_FLAG_SETS = [
    ["/c", "/O2", "/Gy", "/GR-", "/GX"],
    ["/c", "/O2", "/Gy", "/GR-", "/Oy-", "/GX"],
]


def _git_text(path: str, revision: str = BASE_REVISION) -> str:
    result = subprocess.run(
        ["git", "show", f"{revision}:{path}"],
        cwd=ROOT,
        check=True,
        capture_output=True,
        text=True,
    )
    return result.stdout


def _config(cmake: str, project: dict, baseline_cmake: str, baseline_project: dict) -> bool:
    if _global_compiler_link_policy(cmake) != _global_compiler_link_policy(baseline_cmake):
        return False

    base_profiles = _source_profiles(baseline_cmake)
    profiles = _source_profiles(cmake)
    base_effective = _effective_source_profiles(baseline_cmake)
    effective = _effective_source_profiles(cmake)
    for owner in OWNERS:
        if effective.get(owner, ()) != base_effective.get(owner, ()) + ("/Z7",):
            return False

    # The only newly introduced source-level /Z7 profile is the exact three-owner
    # append. Existing unrelated profiles remain unconstrained by this test.
    additions = Counter(profiles) - Counter(base_profiles)
    new_z7 = [row for row, count in additions.items() if count and "/Z7" in row[1]]
    if new_z7 != [(tuple(sorted(OWNERS)), ("/Z7",), True)]:
        # CMake's parser preserves source-list order, so compare set membership
        # separately rather than depending on the order of this owner block.
        if len(new_z7) != 1:
            return False
        files, flags, append = new_z7[0]
        if set(files) != OWNERS or flags != ("/Z7",) or not append:
            return False

    baseline_measurement = baseline_project.get("measurement", {})
    measurement = project.get("measurement", {})
    if {k: v for k, v in measurement.items() if k != "unit_flag_sets"} != {
        k: v for k, v in baseline_measurement.items() if k != "unit_flag_sets"
    }:
        return False
    if measurement.get("flag_sets") != GLOBAL_FLAG_SETS:
        return False
    if measurement.get("flag_sets") != baseline_measurement.get("flag_sets"):
        return False
    if measurement.get("layout_flags") != baseline_measurement.get("layout_flags"):
        return False

    candidate_units = measurement.get("unit_flag_sets", {})
    baseline_units = baseline_measurement.get("unit_flag_sets", {})
    for owner in OWNERS:
        owner_profile = candidate_units.get(owner)
        if not isinstance(owner_profile, dict) or owner_profile.get("flags") != TARGET_RECIPE:
            return False
    # No other owner gains an explicit per-unit /Z7 recipe. Unrelated sources
    # and future profiles are otherwise deliberately not frozen here.
    base_z7 = {
        name for name, row in baseline_units.items()
        if isinstance(row, dict) and "/Z7" in row.get("flags", [])
    }
    current_z7 = {
        name for name, row in candidate_units.items()
        if isinstance(row, dict) and "/Z7" in row.get("flags", [])
    }
    if current_z7 != base_z7 | OWNERS:
        return False
    return True


def _historical_candidate():
    # Preserve the measured three-owner migration as history. Later production
    # owners do not change that proof or its deliberately damaged controls.
    return (
        _git_text("CMakeLists.txt", METADATA_CHECKPOINT),
        _load_json(_git_text("recovery/project.json", METADATA_CHECKPOINT)),
    )


def _baseline():
    return (
        _git_text("CMakeLists.txt"),
        _load_json(_git_text("recovery/project.json")),
    )


def test_historical_metadata_migration_is_limited_to_three_measured_owners():
    cmake, project = _historical_candidate()
    baseline_cmake, baseline_project = _baseline()
    assert _config(cmake, project, baseline_cmake, baseline_project)

    settings = Measurement.read(ROOT)
    expected = tuple(TARGET_RECIPE)
    for owner in OWNERS:
        assert settings.flags_for(owner) == (expected,)
    assert settings.witness_grid(OWNERS) == {
        owner: [TARGET_RECIPE] for owner in OWNERS
    }


def test_production_metadata_contract_rejects_missing_or_leaked_scope():
    cmake, project = _historical_candidate()
    baseline_cmake, baseline_project = _baseline()
    assert _config(cmake, project, baseline_cmake, baseline_project)

    bad_recipe = copy.deepcopy(project)
    bad_recipe["measurement"]["unit_flag_sets"]["src/game.cpp"]["flags"].remove("/Oy-")
    assert not _config(cmake, bad_recipe, baseline_cmake, baseline_project)

    bad_optimization = copy.deepcopy(project)
    bad_optimization["measurement"]["unit_flag_sets"]["src/netdaemon.cpp"]["flags"][1] = "/O1"
    assert not _config(cmake, bad_optimization, baseline_cmake, baseline_project)

    missing_measurement_owner = copy.deepcopy(project)
    del missing_measurement_owner["measurement"]["unit_flag_sets"]["src/mapwin.cpp"]
    assert not _config(cmake, missing_measurement_owner, baseline_cmake, baseline_project)

    malformed_owner = copy.deepcopy(project)
    malformed_owner["measurement"]["unit_flag_sets"]["src/game.cpp"] = [1]
    assert not _config(cmake, malformed_owner, baseline_cmake, baseline_project)

    leaked_measurement = copy.deepcopy(project)
    leaked_measurement["measurement"]["unit_flag_sets"]["src/terrain.cpp"] = {
        "flags": TARGET_RECIPE[:]
    }
    assert not _config(cmake, leaked_measurement, baseline_cmake, baseline_project)

    global_flags = copy.deepcopy(project)
    global_flags["measurement"]["flag_sets"][0].append("/Z7")
    assert not _config(cmake, global_flags, baseline_cmake, baseline_project)

    global_layout = copy.deepcopy(project)
    global_layout["measurement"]["layout_flags"] = []
    assert not _config(cmake, global_layout, baseline_cmake, baseline_project)

    # Prove that the CMake and measurement owner sets cannot silently diverge.
    source_rows = _source_profiles(cmake)
    target_block = next(
        row for row in source_rows if row[2] and set(row[0]) == OWNERS and row[1] == ("/Z7",)
    )
    missing_cmake_owner = cmake.replace(
        'set_property(SOURCE\n    src/game.cpp\n    src/netdaemon.cpp\n    src/mapwin.cpp\n'
        '    APPEND PROPERTY COMPILE_OPTIONS "/Z7")',
        'set_property(SOURCE\n    src/game.cpp\n    src/netdaemon.cpp\n'
        '    APPEND PROPERTY COMPILE_OPTIONS "/Z7")',
        1,
    )
    assert target_block in source_rows
    assert not _config(missing_cmake_owner, project, baseline_cmake, baseline_project)

    leaked_cmake = cmake.replace(
        "    src/mapwin.cpp\n    APPEND PROPERTY COMPILE_OPTIONS \"/Z7\")",
        "    src/mapwin.cpp\n    src/terrain.cpp\n    APPEND PROPERTY COMPILE_OPTIONS \"/Z7\")",
        1,
    )
    assert not _config(leaked_cmake, project, baseline_cmake, baseline_project)

    # Global compiler/link switches remain covered by the existing command parser.
    changed_global = cmake + '\ntarget_compile_options(OpenSMACX PRIVATE "/Z7")\n'
    assert not _config(changed_global, project, baseline_cmake, baseline_project)


CAVIAR_RECIPE = ["/c", "/O2", "/Gy", "/GR-", "/GX", "/Ob2", "/Z7"]


def _caviar_observation_policy(cmake, project):
    # This owner retains its frameless /Ob2 context. Metadata must agree
    # between ordinary production and standalone measurement; other source
    # owners may evolve under their own independently verified policies.
    measurement = project.get("measurement", {})
    return (
        _effective_source_profiles(cmake).get("src/caviar.cpp") == ("/Z7",)
        and measurement.get("unit_flag_sets", {}).get("src/caviar.cpp") ==
            {"flags": CAVIAR_RECIPE}
        and measurement.get("flag_sets") == GLOBAL_FLAG_SETS
        and measurement.get("layout_flags") == ["/Z7"]
    )


def test_current_caviar_type_observation_keeps_its_compiler_context():
    cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    project = _load_json((ROOT / "recovery/project.json").read_text(encoding="utf-8"))
    assert _caviar_observation_policy(cmake, project)
    assert Measurement.read(ROOT).flags_for("src/caviar.cpp") == (tuple(CAVIAR_RECIPE),)

    no_profile = copy.deepcopy(project)
    del no_profile["measurement"]["unit_flag_sets"]["src/caviar.cpp"]
    assert not _caviar_observation_policy(cmake, no_profile)
    changed_frame = copy.deepcopy(project)
    changed_frame["measurement"]["unit_flag_sets"]["src/caviar.cpp"]["flags"].append("/Oy-")
    assert not _caviar_observation_policy(cmake, changed_frame)
    changed_optimization = copy.deepcopy(project)
    changed_optimization["measurement"]["unit_flag_sets"]["src/caviar.cpp"]["flags"][1] = "/O1"
    assert not _caviar_observation_policy(cmake, changed_optimization)

    without_metadata = cmake.replace("    src/caviar.cpp\n    src/game.cpp\n",
                                    "    src/game.cpp\n", 1)
    assert without_metadata != cmake
    assert not _caviar_observation_policy(without_metadata, project)
