"""Guard application source profiles and the immutable measurement policy."""
from __future__ import annotations

import collections
import copy
import json
import pathlib
import re
import subprocess


ROOT = pathlib.Path(__file__).resolve().parents[2]
BASE_REVISION = "a14474869c6aa9cc5b577defbbe292208ae33e22"
NEW_OWNERS = (
    "src/netdaemon.cpp",
    "src/diplowin.cpp",
    "src/diplopop.cpp",
    "src/worldwin.cpp",
)
NEW_PROFILE_COMMENT = (
    "# Keep the controlled diplomacy/network/overview owners on their measured\n"
    "# frame-preserving source profiles; application defaults and link settings stay fixed.\n"
)
NEW_PROFILE_BLOCK_TEXT = (
    "set_source_files_properties(\n"
    "    src/netdaemon.cpp\n"
    "    src/diplowin.cpp\n"
    "    src/diplopop.cpp\n"
    "    src/worldwin.cpp\n"
    '    PROPERTIES COMPILE_OPTIONS "/Oy-")'
)
OLD_DESIGN_PROFILE_COMMENT = (
    "# Preserve the original frame-based stack accesses in these mapped owners;\n"
    "# leave the global compiler defaults and neighboring NetDaemon profile alone.\n"
)
NEW_DESIGN_PROFILE_COMMENT = (
    "# Preserve the original frame-based stack accesses in these mapped owners;\n"
    "# leave the global compiler defaults unchanged.\n"
)
NEW_PROFILE_BLOCK = (tuple(sorted(NEW_OWNERS)), ("/Oy-",))

FRAME_PROFILE = ["/c", "/O2", "/Gy", "/GR-", "/Oy-", "/GX", "/Ob2"]
EXPECTED_FLAG_SETS = [
    ["/c", "/O2", "/Gy", "/GR-", "/GX"],
    ["/c", "/O2", "/Gy", "/GR-", "/Oy-", "/GX"],
]
EXPECTED_CMAKE_PROFILES = {
    tuple(sorted(("src/map_altitude.cpp", "src/wave.cpp"))): ("/Oy-",),
    tuple(sorted(("src/voice.cpp", "src/wave_device.cpp"))): ("/Oy-",),
    tuple(sorted(("src/midi.cpp", "src/sounddevice.cpp", "src/sound_loader.cpp"))): ("/Oy-",),
    tuple(sorted(("src/datalink.cpp", "src/maininterface.cpp", "src/console.cpp"))): ("/Oy-",),
    tuple(sorted(("src/mapwin.cpp", "src/basewin.cpp"))): ("/Oy-",),
    tuple(sorted(("src/designwin.cpp", "src/tutwin.cpp", "src/xpops.cpp"))): ("/Oy-",),
    tuple(sorted(NEW_OWNERS)): ("/Oy-",),
    tuple(sorted(("src/interface_palette.cpp", "src/interface_fonts.cpp"))): ("/Oy-",),
    ("src/sound.cpp",): ("/Oy-",),
    ("src/caviar_cpu.cpp",): ("--frontend=intrinsics-i386-no-debug",),
}
EXISTING_MEASURED_PROFILES = {
    "src/technology_queries.cpp": {"flags": FRAME_PROFILE},
    "src/map_altitude.cpp": {"flags": FRAME_PROFILE},
    "src/wave.cpp": {"flags": FRAME_PROFILE},
    "src/sound.cpp": {"flags": FRAME_PROFILE},
    "src/interface_palette.cpp": [1],
    "src/interface_fonts.cpp": [1],
}


def baseline_text(relative_path: str) -> str:
    result = subprocess.run(
        ["git", "show", f"{BASE_REVISION}:{relative_path}"],
        cwd=ROOT,
        check=True,
        capture_output=True,
        text=True,
    )
    return result.stdout


def source_property_blocks(cmake: str):
    uncommented = re.sub(r"(?m)^\s*#.*$", "", cmake)
    for body in re.findall(r"set_source_files_properties\s*\((.*?)\)", uncommented, re.S):
        files, separator, properties = body.partition("PROPERTIES")
        if separator:
            sources = tuple(sorted(re.findall(r"src/[A-Za-z0-9_./-]+\.(?:cpp|c)", files)))
            match = re.search(r'\bCOMPILE_OPTIONS\s+"([^"]*)"', properties)
            yield sources, tuple(match.group(1).split() if match else ())


def non_profile_text_matches(baseline: str, candidate: str) -> bool:
    insertion = NEW_PROFILE_COMMENT + NEW_PROFILE_BLOCK_TEXT + "\n"
    if baseline.count(insertion) or candidate.count(insertion) != 1:
        return False
    if baseline.count(NEW_PROFILE_COMMENT) or candidate.count(NEW_PROFILE_COMMENT) != 1:
        return False

    has_old_comment = OLD_DESIGN_PROFILE_COMMENT in baseline
    has_new_comment = NEW_DESIGN_PROFILE_COMMENT in candidate
    if has_old_comment or has_new_comment:
        if baseline.count(OLD_DESIGN_PROFILE_COMMENT) != 1:
            return False
        if candidate.count(NEW_DESIGN_PROFILE_COMMENT) != 1:
            return False
        candidate = candidate.replace(
            NEW_DESIGN_PROFILE_COMMENT, OLD_DESIGN_PROFILE_COMMENT, 1
        )

    projected = candidate.replace(insertion, "", 1)
    return projected == baseline


def cmake_profile_delta_is_exact(baseline: str, candidate: str) -> bool:
    expected = collections.Counter(source_property_blocks(baseline))
    expected[NEW_PROFILE_BLOCK] += 1
    return (
        collections.Counter(source_property_blocks(candidate)) == expected
        and non_profile_text_matches(baseline, candidate)
    )


def measurement_policy_is_unchanged(baseline: dict, candidate: dict) -> bool:
    return candidate == baseline


def test_profile_guard_rejects_policy_mutations():
    baseline = (
        'set_source_files_properties(src/previous.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")\n'
        'add_compile_options("/DDEFAULT=1")\n'
        'target_link_options(Game PRIVATE "/FIXED:NO")\n'
    )
    candidate = baseline + NEW_PROFILE_COMMENT + NEW_PROFILE_BLOCK_TEXT + "\n"
    assert cmake_profile_delta_is_exact(baseline, candidate)

    dropped_old_profile = candidate.replace(
        'set_source_files_properties(src/previous.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")\n', ""
    )
    assert not cmake_profile_delta_is_exact(baseline, dropped_old_profile)
    assert not cmake_profile_delta_is_exact(baseline, candidate + "\n" + NEW_PROFILE_BLOCK_TEXT)
    extra_profile = candidate + (
        '\nset_source_files_properties(src/extra.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")\n'
    )
    assert not cmake_profile_delta_is_exact(baseline, extra_profile)
    extra_source_property = candidate + (
        '\nset_source_files_properties(src/extra.cpp PROPERTIES COMPILE_DEFINITIONS "EXTRA")\n'
    )
    assert not cmake_profile_delta_is_exact(baseline, extra_source_property)
    changed_options = candidate.replace(
        NEW_PROFILE_BLOCK_TEXT,
        NEW_PROFILE_BLOCK_TEXT.replace('COMPILE_OPTIONS "/Oy-"', 'COMPILE_OPTIONS "/Oy- /Ob0"'),
    )
    assert not cmake_profile_delta_is_exact(baseline, changed_options)
    extra_owner = candidate.replace(
        "    src/worldwin.cpp\n", "    src/worldwin.cpp\n    src/other.cpp\n"
    )
    assert not cmake_profile_delta_is_exact(baseline, extra_owner)
    global_flag_change = candidate.replace(
        'add_compile_options("/DDEFAULT=1")', 'add_compile_options("/DDEFAULT=1" "/Oy-")'
    )
    assert not cmake_profile_delta_is_exact(baseline, global_flag_change)
    linker_change = candidate.replace('"/FIXED:NO"', '"/FIXED:NO" "/OPT:REF"')
    assert not cmake_profile_delta_is_exact(baseline, linker_change)

    baseline_measurement = {"flag_sets": [EXPECTED_FLAG_SETS[0]], "unit_flag_sets": {}}
    changed_measurement = copy.deepcopy(baseline_measurement)
    changed_measurement["flag_sets"][0].append("/Oy-")
    assert not measurement_policy_is_unchanged(baseline_measurement, changed_measurement)


def test_application_profiles_match_immutable_baseline_plus_four_owners():
    task = json.loads(
        (ROOT / "recovery/tasks/diplomacy-network-frame-oct17.json").read_text(
            encoding="utf-8"
        )
    )
    assert task["base_revision"] == BASE_REVISION
    baseline_cmake = baseline_text("CMakeLists.txt")
    current_cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    assert cmake_profile_delta_is_exact(baseline_cmake, current_cmake)

    source_list = re.search(r"set\(OPENSMACX_SOURCES(.*?)\n\)", current_cmake, re.S)
    assert source_list is not None
    application_sources = set(source_list.group(1).split())
    assert all(owner in application_sources for owner in NEW_OWNERS)
    assert "src/win.cpp" in application_sources
    assert "src/technology_queries.cpp" in application_sources

    actual_profiles = dict(source_property_blocks(current_cmake))
    assert actual_profiles == EXPECTED_CMAKE_PROFILES
    profiled_sources = {source for sources in actual_profiles for source in sources}
    assert "src/win.cpp" not in profiled_sources
    assert "src/technology_queries.cpp" not in profiled_sources

    baseline_project = json.loads(baseline_text("recovery/project.json"))
    current_project = json.loads(
        (ROOT / "recovery/project.json").read_text(encoding="utf-8")
    )
    baseline_measurement = baseline_project["measurement"]
    current_measurement = current_project["measurement"]
    assert measurement_policy_is_unchanged(baseline_measurement, current_measurement)
    assert current_measurement["flag_sets"] == EXPECTED_FLAG_SETS
    unit_profiles = current_measurement["unit_flag_sets"]
    for owner, expected in EXISTING_MEASURED_PROFILES.items():
        assert unit_profiles[owner] == expected, (owner, unit_profiles.get(owner))
    for owner in NEW_OWNERS + ("src/win.cpp",):
        assert owner not in unit_profiles, (owner, unit_profiles.get(owner))

