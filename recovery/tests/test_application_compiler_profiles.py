"""Guard application source profiles and the immutable measurement policy."""
from __future__ import annotations

import collections
import copy
import json
import pathlib
import re
import subprocess


ROOT = pathlib.Path(__file__).resolve().parents[2]
BASE_REVISION = "7270943e6988505e93cff9d32d126c129f70e8ab"
ALPHA_BASE_REVISION = "1b0f0231bd7dc49750fabd249ad11b21faf78dbe"
PRIOR_PROFILE_GROUPS = (
    (
        "src/game.cpp",
        "src/reportwin.cpp",
        "src/netwin.cpp",
    ),
    (
        "src/reportif.cpp",
        "src/statuswin.cpp",
        "src/prodpicker.cpp",
        "src/alphamenu.cpp",
    ),
    (
        "src/stringstruct.cpp",
        "src/councwin.cpp",
        "src/planwin.cpp",
    ),
    (
        "src/infowin.cpp",
        "src/alphasave.cpp",
        "src/famewin.cpp",
        "src/messagewin.cpp",
        "src/setupwin.cpp",
        "src/path.cpp",
        "src/dipedit.cpp",
        "src/picktech.cpp",
        "src/pickwin.cpp",
        "src/council.cpp",
    ),
    (
        "src/prefwin.cpp",
        "src/fx.cpp",
        "src/replaywin.cpp",
        "src/multidebug.cpp",
        "src/newtechwin.cpp",
        "src/quaylewin.cpp",
        "src/battlewin.cpp",
        "src/lock.cpp",
        "src/monuwin.cpp",
        "src/name_table.cpp",
    ),
)
PREVIOUS_OWNERS = tuple(owner for group in PRIOR_PROFILE_GROUPS for owner in group)
NEW_OWNERS = (
    "src/ambience.cpp",
    "src/deletionlist.cpp",
    "src/factionprofilewin.cpp",
    "src/interlude.cpp",
    "src/netmsg.cpp",
    "src/playerlock.cpp",
    "src/probe.cpp",
)
REJECTED_OWNERS = (
    "src/buffer.cpp", "src/alphamovie.cpp", "src/basepop.cpp",
    "src/terraforming.cpp", "src/newgame.cpp",
)
PREVIOUS_REJECTED_OWNERS = (
    "src/alphanet.cpp", "src/strings.cpp", "src/gamma.cpp", "src/credits.cpp",
)
NEW_PROFILE_COMMENT = (
    "# Preserve measured frame-based accesses in the core application cohort.\n"
    "# Mixed or non-improving owners retain their measured default recipes.\n"
)
NEW_PROFILE_BLOCK_TEXT = (
    "set_source_files_properties(\n"
    "    src/ambience.cpp\n"
    "    src/deletionlist.cpp\n"
    "    src/factionprofilewin.cpp\n"
    "    src/interlude.cpp\n"
    "    src/netmsg.cpp\n"
    "    src/playerlock.cpp\n"
    "    src/probe.cpp\n"
    '    PROPERTIES COMPILE_OPTIONS "/Oy-")'
)
NEW_PROFILE_BLOCK = (tuple(sorted(NEW_OWNERS)), ("/Oy-",))
ALPHA_PROFILE_COMMENT = (
    "# Preserve Alpha's measured frame-based application context.\n"
    "# Shared graphics language state and its setter have a separate library owner.\n"
)
ALPHA_PROFILE_BLOCK_TEXT = (
    'set_source_files_properties(src/alpha.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")'
)
ALPHA_PROFILE_INSERTION = ALPHA_PROFILE_COMMENT + ALPHA_PROFILE_BLOCK_TEXT + "\n"
ALPHA_PROFILE_BLOCK = (("src/alpha.cpp",), ("/Oy-",))
OLD_ALPHA_DEFAULT_COMMENT = (
    "# Keep Alpha and all other source recipes on their independently measured defaults.\n"
)
NEW_ALPHA_DEFAULT_COMMENT = (
    "# Keep all other source recipes on their independently measured defaults.\n"
)

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
    tuple(sorted(("src/netdaemon.cpp", "src/diplowin.cpp", "src/diplopop.cpp", "src/worldwin.cpp"))): ("/Oy-",),
    tuple(sorted(("src/map.cpp", "src/faction.cpp", "src/veh.cpp"))): ("/Oy-",),
    **{tuple(sorted(group)): ("/Oy-",) for group in PRIOR_PROFILE_GROUPS},
    tuple(sorted(NEW_OWNERS)): ("/Oy-",),
    ("src/alpha.cpp",): ("/Oy-",),
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
    if baseline.count(ALPHA_PROFILE_INSERTION) or candidate.count(ALPHA_PROFILE_INSERTION) != 1:
        return False
    if baseline.count(ALPHA_PROFILE_COMMENT) or candidate.count(ALPHA_PROFILE_COMMENT) != 1:
        return False
    if baseline.count(OLD_ALPHA_DEFAULT_COMMENT) != 1:
        return False
    if candidate.count(OLD_ALPHA_DEFAULT_COMMENT) or candidate.count(NEW_ALPHA_DEFAULT_COMMENT) != 1:
        return False

    projected = candidate.replace(insertion, "", 1)
    projected = projected.replace(ALPHA_PROFILE_INSERTION, "", 1)
    projected = projected.replace(NEW_ALPHA_DEFAULT_COMMENT, OLD_ALPHA_DEFAULT_COMMENT, 1)
    return projected == baseline


def cmake_profile_delta_is_exact(baseline: str, candidate: str) -> bool:
    expected = collections.Counter(source_property_blocks(baseline))
    expected[NEW_PROFILE_BLOCK] += 1
    expected[ALPHA_PROFILE_BLOCK] += 1
    return (
        collections.Counter(source_property_blocks(candidate)) == expected
        and non_profile_text_matches(baseline, candidate)
    )


def measurement_policy_is_unchanged(baseline: dict, candidate: dict) -> bool:
    return candidate == baseline


def test_profile_guard_rejects_policy_mutations():
    baseline = (
        'set_source_files_properties(src/previous.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")\n'
        + OLD_ALPHA_DEFAULT_COMMENT
        + 'add_compile_options("/DDEFAULT=1")\n'
        'target_link_options(Game PRIVATE "/FIXED:NO")\n'
    )
    candidate = baseline.replace(OLD_ALPHA_DEFAULT_COMMENT, NEW_ALPHA_DEFAULT_COMMENT)
    candidate += NEW_PROFILE_COMMENT + NEW_PROFILE_BLOCK_TEXT + "\n"
    candidate += ALPHA_PROFILE_INSERTION
    assert cmake_profile_delta_is_exact(baseline, candidate)

    dropped_old_profile = candidate.replace(
        'set_source_files_properties(src/previous.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")\n', ""
    )
    assert not cmake_profile_delta_is_exact(baseline, dropped_old_profile)
    assert not cmake_profile_delta_is_exact(baseline, candidate + "\n" + NEW_PROFILE_BLOCK_TEXT)
    duplicate_alpha_profile = candidate + "\n" + ALPHA_PROFILE_BLOCK_TEXT + "\n"
    assert not cmake_profile_delta_is_exact(baseline, duplicate_alpha_profile)
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
        "    src/ambience.cpp\n", "    src/ambience.cpp\n    src/other.cpp\n"
    )
    assert not cmake_profile_delta_is_exact(baseline, extra_owner)
    global_flag_change = candidate.replace(
        'add_compile_options("/DDEFAULT=1")', 'add_compile_options("/DDEFAULT=1" "/Oy-")'
    )
    assert not cmake_profile_delta_is_exact(baseline, global_flag_change)
    linker_change = candidate.replace('"/FIXED:NO"', '"/FIXED:NO" "/OPT:REF"')
    assert not cmake_profile_delta_is_exact(baseline, linker_change)
    sprite_profile = candidate + (
        '\nset_source_files_properties(src/sprite.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")\n'
    )
    assert not cmake_profile_delta_is_exact(baseline, sprite_profile)
    network_profile = candidate + (
        '\nset_source_files_properties(src/net_class.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")\n'
    )
    assert not cmake_profile_delta_is_exact(baseline, network_profile)
    alphanet_profile = candidate + (
        '\nset_source_files_properties(src/alphanet.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")\n'
    )
    assert not cmake_profile_delta_is_exact(baseline, alphanet_profile)
    strings_profile = candidate + (
        '\nset_source_files_properties(src/strings.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")\n'
    )
    assert not cmake_profile_delta_is_exact(baseline, strings_profile)
    for rejected in (*REJECTED_OWNERS, *PREVIOUS_REJECTED_OWNERS, "src/base.cpp", "src/win.cpp", "src/technology_queries.cpp"):
        extra_profile = candidate + (
            f'\nset_source_files_properties({rejected} PROPERTIES COMPILE_OPTIONS "/Oy-")\n'
        )
        assert not cmake_profile_delta_is_exact(baseline, extra_profile)

    baseline_measurement = {"flag_sets": [EXPECTED_FLAG_SETS[0]], "unit_flag_sets": {}}
    changed_measurement = copy.deepcopy(baseline_measurement)
    changed_measurement["flag_sets"][0].append("/Oy-")
    assert not measurement_policy_is_unchanged(baseline_measurement, changed_measurement)


def test_application_profiles_match_immutable_baseline_plus_core_cohort():
    task = json.loads(
        (ROOT / "recovery/tasks/core-owner-and-win-flags-oct17.json").read_text(
            encoding="utf-8"
        )
    )
    assert task["base_revision"] == BASE_REVISION
    alpha_task = json.loads(
        (ROOT / "recovery/tasks/alpha-graphics-language-oct17.json").read_text(
            encoding="utf-8"
        )
    )
    assert alpha_task["base_revision"] == ALPHA_BASE_REVISION
    baseline_cmake = baseline_text("CMakeLists.txt")
    current_cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    assert cmake_profile_delta_is_exact(baseline_cmake, current_cmake)

    source_list = re.search(r"set\(OPENSMACX_SOURCES(.*?)\n\)", current_cmake, re.S)
    assert source_list is not None
    application_sources = set(source_list.group(1).split())
    assert all(owner in application_sources for owner in NEW_OWNERS + PREVIOUS_OWNERS)
    assert all(owner in application_sources for owner in REJECTED_OWNERS)
    assert all(owner in application_sources for owner in PREVIOUS_REJECTED_OWNERS)
    assert "src/base.cpp" in application_sources
    assert "src/sprite.cpp" in application_sources
    assert "src/net_class.cpp" in application_sources
    assert "src/alpha.cpp" in application_sources
    assert "src/win.cpp" in application_sources
    assert "src/technology_queries.cpp" in application_sources

    actual_profiles = dict(source_property_blocks(current_cmake))
    assert actual_profiles == EXPECTED_CMAKE_PROFILES
    profiled_sources = {source for sources in actual_profiles for source in sources}
    assert "src/alpha.cpp" in profiled_sources
    assert actual_profiles[("src/alpha.cpp",)] == ("/Oy-",)
    assert "src/win.cpp" not in profiled_sources
    assert "src/technology_queries.cpp" not in profiled_sources
    assert "src/sprite.cpp" not in profiled_sources
    assert "src/net_class.cpp" not in profiled_sources
    assert all(owner not in profiled_sources for owner in REJECTED_OWNERS)
    assert all(owner not in profiled_sources for owner in PREVIOUS_REJECTED_OWNERS)
    assert "src/base.cpp" not in profiled_sources

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
    for owner in NEW_OWNERS + REJECTED_OWNERS + PREVIOUS_REJECTED_OWNERS + ("src/base.cpp", "src/win.cpp", "src/alpha.cpp"):
        assert owner not in unit_profiles, (owner, unit_profiles.get(owner))

