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
REMAINING_BASE_REVISION = "02518228188b32f92a5314df56a218f415e72cd8"
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
REMAINING_OWNERS = (
    "src/base.cpp",
    "src/multiwin.cpp",
    "src/socialwin.cpp",
    "src/technology.cpp",
    "src/technology_queries.cpp",
    "src/uv2player.cpp",
)
REJECTED_OWNERS = (
    "src/buffer.cpp", "src/alphamovie.cpp", "src/basepop.cpp",
    "src/terraforming.cpp", "src/newgame.cpp",
)
PREVIOUS_REJECTED_OWNERS = (
    "src/strings.cpp", "src/gamma.cpp", "src/credits.cpp",
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
REMAINING_PROFILE_COMMENT = (
    "# Preserve measured frame-based accesses in the remaining application owners.\n"
    "# Mixed generic/application owners retain their independently measured defaults.\n"
)
REMAINING_PROFILE_BLOCK_TEXT = (
    "set_source_files_properties(\n"
    "    src/base.cpp\n"
    "    src/multiwin.cpp\n"
    "    src/socialwin.cpp\n"
    "    src/technology.cpp\n"
    "    src/technology_queries.cpp\n"
    "    src/uv2player.cpp\n"
    '    PROPERTIES COMPILE_OPTIONS "/Oy-")'
)
REMAINING_PROFILE_INSERTION = (
    REMAINING_PROFILE_COMMENT + REMAINING_PROFILE_BLOCK_TEXT + "\n"
)
REMAINING_PROFILE_BLOCK = (tuple(sorted(REMAINING_OWNERS)), ("/Oy-",))
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
ALPHANET_SOURCE_BEFORE = "    src/alphanet.cpp\n    src/ambience.cpp\n"
ALPHANET_SOURCE_AFTER = (
    "    src/alphanet.cpp\n    src/alphanet_records.cpp\n    src/ambience.cpp\n"
)
ALPHANET_PROFILE_COMMENT = (
    "# Preserve the measured frame-based AlphaNet controller context.\n"
    "# Its record and lifecycle owner retains the default application profile.\n"
)
ALPHANET_PROFILE_BLOCK_TEXT = (
    'set_source_files_properties(src/alphanet.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")'
)
ALPHANET_PROFILE_INSERTION = (
    ALPHANET_PROFILE_COMMENT + ALPHANET_PROFILE_BLOCK_TEXT + "\n"
)
ALPHANET_PROFILE_BLOCK = (("src/alphanet.cpp",), ("/Oy-",))
VIDEO_SOUND_BASE_REVISION = "95e6bb71f2bfb0f2d9859c2b06f8cdc9c595e9b4"
VIDEO_SOUND_SOURCE_BEFORE = "    src/video.cpp\n"
VIDEO_SOUND_SOURCE_AFTER = "    src/video.cpp\n    src/video_sound.cpp\n"
VIDEO_SOUND_PROFILE_COMMENT = (
    "# Preserve the measured frame-based context of the Sound-derived video owner.\n"
    "# Video and queue methods retain their independently measured default recipe.\n"
)
VIDEO_SOUND_PROFILE_BLOCK_TEXT = (
    'set_source_files_properties(src/video_sound.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")'
)
VIDEO_SOUND_PROFILE_INSERTION = (
    VIDEO_SOUND_PROFILE_COMMENT + VIDEO_SOUND_PROFILE_BLOCK_TEXT + "\n"
)
VIDEO_SOUND_PROFILE_BLOCK = (("src/video_sound.cpp",), ("/Oy-",))

UNDO_BASE_REVISION = "b24ef250e686245f6075d2593b1a0d159eda1827"
UNDO_SOURCE_BEFORE = "    src/general.cpp\n"
UNDO_SOURCE_AFTER = "    src/general.cpp\n    src/scenario_undo.cpp\n"
UNDO_PROFILE_COMMENT = (
    "# Preserve the measured frame-based context of the scenario-editor undo owner.\n"
    "# General's unrelated helpers retain their independently measured default recipe.\n"
)
UNDO_PROFILE_BLOCK_TEXT = (
    'set_source_files_properties(src/scenario_undo.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")'
)
UNDO_PROFILE_INSERTION = "\n" + UNDO_PROFILE_COMMENT + UNDO_PROFILE_BLOCK_TEXT + "\n"
UNDO_PROFILE_BLOCK = (("src/scenario_undo.cpp",), ("/Oy-",))

HEADER_RNG_SOURCE_BEFORE = "    src/scenario_undo.cpp\n"
HEADER_RNG_SOURCE_AFTER = (
    "    src/scenario_undo.cpp\n    src/header_io.cpp\n    src/crt_random.cpp\n"
)
HEADER_RNG_PROFILE_INSERTION = (
    "# Preserve the measured frame-based contexts of the shared stream wrappers.\n"
    "# General's unrelated helpers retain their independently measured default recipe.\n"
    "set_source_files_properties(src/header_io.cpp src/crt_random.cpp\n"
    '    PROPERTIES COMPILE_OPTIONS "/Oy-")\n'
)
HEADER_RNG_PROFILE_BLOCK = (("src/crt_random.cpp", "src/header_io.cpp"), ("/Oy-",))
VEHICLE_PROFILE_COMMENT = (
    "# Preserve the mapped frame-based vehicle owners after their source split.\n"
    "# Their original translation-unit names are unknown; other veh.cpp code stays\n"
    "# on its existing owner profile.\n"
)
VEHICLE_PROFILE_BLOCK_TEXT = (
    "set_source_files_properties(\n"
    "    src/vehicle_cost.cpp\n"
    "    src/vehicle_orders.cpp\n"
    '    PROPERTIES COMPILE_OPTIONS "/Oy-")'
)
VEHICLE_PROFILE_INSERTION = VEHICLE_PROFILE_COMMENT + VEHICLE_PROFILE_BLOCK_TEXT + "\n"
VEHICLE_PROFILE_BLOCK = (("src/vehicle_cost.cpp", "src/vehicle_orders.cpp"), ("/Oy-",))
TUTWIN_SOURCE_BEFORE = "    src/tutwin.cpp\n    src/popmenu.cpp\n"
TUTWIN_SOURCE_AFTER = "    src/tutwin.cpp\n    src/tutwin_callouts.cpp\n    src/popmenu.cpp\n"
TUTWIN_PROFILE_BEFORE = (
    "set_source_files_properties(\n"
    "    src/designwin.cpp\n    src/tutwin.cpp\n    src/xpops.cpp\n"
    '    PROPERTIES COMPILE_OPTIONS "/Oy-")\n'
)
TUTWIN_PROFILE_AFTER = TUTWIN_PROFILE_BEFORE.replace(
    "    src/tutwin.cpp\n", "    src/tutwin.cpp\n    src/tutwin_callouts.cpp\n", 1
)

FRAME_PROFILE = ["/c", "/O2", "/Gy", "/GR-", "/Oy-", "/GX", "/Ob2"]
EXPECTED_FLAG_SETS = [
    ["/c", "/O2", "/Gy", "/GR-", "/GX"],
    ["/c", "/O2", "/Gy", "/GR-", "/Oy-", "/GX"],
]
EXPECTED_CMAKE_PROFILES = {
    tuple(sorted(("src/map_altitude.cpp", "src/wave.cpp"))): ("/Oy-",),
    tuple(sorted(("src/midi.cpp", "src/sounddevice.cpp"))): ("/Oy-",),
    tuple(sorted(("src/datalink.cpp", "src/datalink_entries.cpp", "src/maininterface.cpp",
                  "src/console.cpp", "src/console_commands.cpp"))): ("/Oy-",),
    tuple(sorted(("src/mapwin.cpp", "src/basewin.cpp"))): ("/Oy-",),
    tuple(sorted(("src/designwin.cpp", "src/tutwin.cpp", "src/xpops.cpp"))): ("/Oy-",),
    tuple(sorted(("src/netdaemon.cpp", "src/diplowin.cpp", "src/diplopop.cpp", "src/worldwin.cpp"))): ("/Oy-",),
    tuple(sorted(("src/map.cpp", "src/map_grid.cpp", "src/movement_queries.cpp",
                  "src/map_sites.cpp", "src/territory.cpp", "src/world.cpp",
                  "src/faction.cpp", "src/faction_turn.cpp", "src/faction_treaty.cpp",
                  "src/faction_diplomacy.cpp", "src/faction_dialogue.cpp",
                  "src/faction_network.cpp", "src/veh.cpp", "src/vehicle_render.cpp",
                  "src/veh_queries.cpp", "src/prototype_history.cpp"))): ("/Oy-",),
    **{tuple(sorted(group)): ("/Oy-",) for group in PRIOR_PROFILE_GROUPS},
    tuple(sorted(NEW_OWNERS)): ("/Oy-",),
    tuple(sorted(REMAINING_OWNERS)): ("/Oy-",),
    ("src/alpha.cpp",): ("/Oy-",),
    ("src/scenario_undo.cpp",): ("/Oy-",),
    ("src/crt_random.cpp", "src/header_io.cpp"): ("/Oy-",),
    ("src/vehicle_cost.cpp", "src/vehicle_orders.cpp"): ("/Oy-",),
    tuple(sorted(("src/interface_palette.cpp", "src/interface_fonts.cpp"))): ("/Oy-",),
    ("src/sound.cpp", "src/sound_type.cpp"): ("/Oy-",),
    ("src/caviar_cpu.cpp",): ("--frontend=intrinsics-i386-no-debug",),
}
EXISTING_MEASURED_PROFILES = {
    "src/technology_queries.cpp": {"flags": FRAME_PROFILE},
    "src/map_altitude.cpp": {"flags": FRAME_PROFILE},
    "src/wave.cpp": {"flags": FRAME_PROFILE},
    "src/sound.cpp": {"flags": FRAME_PROFILE},
    "src/sound_type.cpp": {"flags": FRAME_PROFILE},
    "src/interface_palette.cpp": [1],
    "src/interface_fonts.cpp": [1],
}
MEDIA_DIAGNOSTIC_OWNERS = (
    "src/video.cpp", "src/video_queue.cpp", "src/alphamovie.cpp", "src/mci_video.cpp",
)
MEDIA_DIAGNOSTIC_FLAGS = ["/c", "/O2", "/Gy", "/GR-", "/GX", "/Ob2"]


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
    for before, after in ((TUTWIN_SOURCE_BEFORE, TUTWIN_SOURCE_AFTER),
                          (TUTWIN_PROFILE_BEFORE, TUTWIN_PROFILE_AFTER)):
        if baseline.count(before) != 1 or baseline.count(after) or candidate.count(after) != 1:
            return False
    insertion = NEW_PROFILE_COMMENT + NEW_PROFILE_BLOCK_TEXT + "\n"
    if baseline.count(insertion) or candidate.count(insertion) != 1:
        return False
    if baseline.count(NEW_PROFILE_COMMENT) or candidate.count(NEW_PROFILE_COMMENT) != 1:
        return False
    if baseline.count(REMAINING_PROFILE_INSERTION) or candidate.count(REMAINING_PROFILE_INSERTION) != 1:
        return False
    if baseline.count(REMAINING_PROFILE_COMMENT) or candidate.count(REMAINING_PROFILE_COMMENT) != 1:
        return False
    if baseline.count(ALPHA_PROFILE_INSERTION) or candidate.count(ALPHA_PROFILE_INSERTION) != 1:
        return False
    if baseline.count(ALPHA_PROFILE_COMMENT) or candidate.count(ALPHA_PROFILE_COMMENT) != 1:
        return False
    if baseline.count(OLD_ALPHA_DEFAULT_COMMENT) != 1:
        return False
    if candidate.count(OLD_ALPHA_DEFAULT_COMMENT) or candidate.count(NEW_ALPHA_DEFAULT_COMMENT) != 1:
        return False
    if baseline.count(VIDEO_SOUND_PROFILE_INSERTION) or candidate.count(VIDEO_SOUND_PROFILE_INSERTION) != 1:
        return False
    if baseline.count(VIDEO_SOUND_SOURCE_BEFORE) != 1 or candidate.count(VIDEO_SOUND_SOURCE_AFTER) != 1:
        return False
    if baseline.count(UNDO_PROFILE_INSERTION) or candidate.count(UNDO_PROFILE_INSERTION) != 1:
        return False
    if baseline.count(UNDO_SOURCE_BEFORE) != 1 or candidate.count(UNDO_SOURCE_AFTER) != 1:
        return False
    if baseline.count(HEADER_RNG_PROFILE_INSERTION) or candidate.count(HEADER_RNG_PROFILE_INSERTION) != 1:
        return False
    if baseline.count(HEADER_RNG_SOURCE_BEFORE) or candidate.count(HEADER_RNG_SOURCE_AFTER) != 1:
        return False
    if baseline.count(VEHICLE_PROFILE_INSERTION) or candidate.count(VEHICLE_PROFILE_INSERTION) != 1:
        return False
    if baseline.count(ALPHANET_SOURCE_BEFORE) != 1 or candidate.count(ALPHANET_SOURCE_AFTER) != 1:
        return False
    if baseline.count(ALPHANET_PROFILE_INSERTION) or candidate.count(ALPHANET_PROFILE_INSERTION) != 1:
        return False

    projected = candidate.replace(TUTWIN_SOURCE_AFTER, TUTWIN_SOURCE_BEFORE, 1)
    projected = projected.replace(TUTWIN_PROFILE_AFTER, TUTWIN_PROFILE_BEFORE, 1)
    projected = projected.replace(VEHICLE_PROFILE_INSERTION, "", 1)
    projected = projected.replace(HEADER_RNG_PROFILE_INSERTION, "", 1)
    projected = projected.replace(HEADER_RNG_SOURCE_AFTER, HEADER_RNG_SOURCE_BEFORE, 1)
    projected = projected.replace(insertion, "", 1)
    projected = projected.replace(REMAINING_PROFILE_INSERTION, "", 1)
    projected = projected.replace(ALPHA_PROFILE_INSERTION, "", 1)
    projected = projected.replace(VIDEO_SOUND_PROFILE_INSERTION, "", 1)
    projected = projected.replace(VIDEO_SOUND_SOURCE_AFTER, VIDEO_SOUND_SOURCE_BEFORE, 1)
    projected = projected.replace(UNDO_PROFILE_INSERTION, "", 1)
    projected = projected.replace(UNDO_SOURCE_AFTER, UNDO_SOURCE_BEFORE, 1)
    projected = projected.replace(ALPHANET_SOURCE_AFTER, ALPHANET_SOURCE_BEFORE, 1)
    projected = projected.replace(NEW_ALPHA_DEFAULT_COMMENT, OLD_ALPHA_DEFAULT_COMMENT, 1)
    projected = projected.replace(ALPHANET_PROFILE_INSERTION, "", 1)
    return projected == baseline


def cmake_profile_delta_is_exact(baseline: str, candidate: str) -> bool:
    expected = collections.Counter(source_property_blocks(baseline))
    old_group, = source_property_blocks(TUTWIN_PROFILE_BEFORE)
    new_group, = source_property_blocks(TUTWIN_PROFILE_AFTER)
    if expected[old_group] != 1:
        return False
    del expected[old_group]
    expected[new_group] += 1
    expected[NEW_PROFILE_BLOCK] += 1
    expected[REMAINING_PROFILE_BLOCK] += 1
    expected[ALPHA_PROFILE_BLOCK] += 1
    expected[ALPHANET_PROFILE_BLOCK] += 1
    expected[VIDEO_SOUND_PROFILE_BLOCK] += 1
    expected[UNDO_PROFILE_BLOCK] += 1
    expected[HEADER_RNG_PROFILE_BLOCK] += 1
    expected[VEHICLE_PROFILE_BLOCK] += 1
    return (
        collections.Counter(source_property_blocks(candidate)) == expected
        and non_profile_text_matches(baseline, candidate)
    )


def measurement_policy_is_unchanged(baseline: dict, candidate: dict) -> bool:
    return candidate == baseline


def source_append_option_blocks(cmake: str):
    """Read this test's narrow SOURCE/APPEND profile form, not all CMake."""
    uncommented = re.sub(r"(?m)^\s*#.*$", "", cmake)
    for body in re.findall(r"set_property\s*\(\s*SOURCE(.*?)\)", uncommented, re.S):
        sources, separator, properties = body.partition("APPEND PROPERTY")
        if not separator:
            continue
        source_names = tuple(re.findall(r"src/[A-Za-z0-9_./-]+\.(?:cpp|c)", sources))
        match = re.fullmatch(r'\s*COMPILE_OPTIONS\s+"([^"]*)"\s*', properties)
        if match:
            yield source_names, tuple(match.group(1).split())


def effective_source_profiles(blocks):
    """Flatten owner facts while refusing contradictory source properties."""
    profiles = {}
    for sources, options in blocks:
        for source in sources:
            if source in profiles and profiles[source] != options:
                raise AssertionError(
                    f"conflicting COMPILE_OPTIONS for {source}: "
                    f"{profiles[source]} vs {options}"
                )
            profiles[source] = options
    return profiles


def cmake_command_blocks(cmake: str, command: str):
    """Extract balanced named command calls for the live global-policy check."""
    uncommented = re.sub(r"(?m)^\s*#.*$", "", cmake)
    for match in re.finditer(rf"\b{re.escape(command)}\s*\(", uncommented):
        start = match.end()
        depth = 1
        quoted = False
        escaped = False
        for index in range(start, len(uncommented)):
            char = uncommented[index]
            if escaped:
                escaped = False
            elif char == "\\" and quoted:
                escaped = True
            elif char == '"':
                quoted = not quoted
            elif not quoted and char == "(":
                depth += 1
            elif not quoted and char == ")":
                depth -= 1
                if depth == 0:
                    yield " ".join(uncommented[start:index].split())
                    break


def application_global_policy(cmake: str):
    """Return only global/app compiler and link commands, excluding TU rows."""
    result = []
    for command in ("add_compile_options", "target_compile_options",
                    "target_compile_definitions", "target_link_options",
                    "target_link_libraries", "set_target_properties"):
        for body in cmake_command_blocks(cmake, command):
            if command == "add_compile_options" or body.split(maxsplit=1)[0] == "OpenSMACX":
                result.append((command, body))
    return tuple(result)


def test_profile_guard_rejects_policy_mutations():
    baseline = (
        'set_source_files_properties(src/previous.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")\n'
        + OLD_ALPHA_DEFAULT_COMMENT
        + 'set(OPENSMACX_SOURCES\n    src/video.cpp\n    src/general.cpp\n'
        + ALPHANET_SOURCE_BEFORE
        + TUTWIN_SOURCE_BEFORE + ')\n'
        + TUTWIN_PROFILE_BEFORE
        + 'add_compile_options("/DDEFAULT=1")\n'
        'target_link_options(Game PRIVATE "/FIXED:NO")\n'
    )
    candidate = baseline.replace(OLD_ALPHA_DEFAULT_COMMENT, NEW_ALPHA_DEFAULT_COMMENT)
    candidate += NEW_PROFILE_COMMENT + NEW_PROFILE_BLOCK_TEXT + "\n"
    candidate += REMAINING_PROFILE_INSERTION
    candidate += ALPHA_PROFILE_INSERTION
    candidate += ALPHANET_PROFILE_INSERTION
    candidate = candidate.replace(VIDEO_SOUND_SOURCE_BEFORE, VIDEO_SOUND_SOURCE_AFTER, 1)
    candidate = candidate.replace(ALPHANET_SOURCE_BEFORE, ALPHANET_SOURCE_AFTER, 1)
    candidate += VIDEO_SOUND_PROFILE_INSERTION
    candidate = candidate.replace(UNDO_SOURCE_BEFORE, UNDO_SOURCE_AFTER, 1)
    candidate += UNDO_PROFILE_INSERTION
    candidate = candidate.replace(HEADER_RNG_SOURCE_BEFORE, HEADER_RNG_SOURCE_AFTER, 1)
    candidate += HEADER_RNG_PROFILE_INSERTION
    candidate += VEHICLE_PROFILE_INSERTION
    candidate = candidate.replace(TUTWIN_SOURCE_BEFORE, TUTWIN_SOURCE_AFTER, 1)
    candidate = candidate.replace(TUTWIN_PROFILE_BEFORE, TUTWIN_PROFILE_AFTER, 1)
    assert cmake_profile_delta_is_exact(baseline, candidate)

    for damaged in (
        candidate.replace(TUTWIN_SOURCE_AFTER, TUTWIN_SOURCE_BEFORE, 1),
        candidate.replace(TUTWIN_SOURCE_AFTER, TUTWIN_SOURCE_AFTER + "    src/tutwin_callouts.cpp\n", 1),
        candidate.replace(TUTWIN_PROFILE_AFTER, TUTWIN_PROFILE_BEFORE, 1),
        candidate.replace(TUTWIN_PROFILE_AFTER, TUTWIN_PROFILE_AFTER.replace("/Oy-", "/Oy- /Ob0"), 1),
        candidate.replace(TUTWIN_PROFILE_AFTER, TUTWIN_PROFILE_AFTER.replace("src/tutwin_callouts.cpp", "src/basewin.cpp"), 1),
        candidate.replace(HEADER_RNG_PROFILE_INSERTION, "", 1),
        candidate + HEADER_RNG_PROFILE_INSERTION,
        candidate.replace(HEADER_RNG_SOURCE_AFTER, HEADER_RNG_SOURCE_BEFORE, 1),
        candidate.replace(HEADER_RNG_SOURCE_AFTER,
                          HEADER_RNG_SOURCE_AFTER + "    src/header_io.cpp\n", 1),
        candidate.replace(HEADER_RNG_SOURCE_AFTER,
                          HEADER_RNG_SOURCE_AFTER + "    src/crt_random.cpp\n", 1),
        candidate.replace(HEADER_RNG_PROFILE_INSERTION,
                          HEADER_RNG_PROFILE_INSERTION.replace("/Oy-", "/Oy- /Ob0"), 1),
        candidate.replace(HEADER_RNG_PROFILE_INSERTION,
                          HEADER_RNG_PROFILE_INSERTION.replace("src/header_io.cpp", "src/general.cpp"), 1),
        candidate.replace(VEHICLE_PROFILE_INSERTION, "", 1),
        candidate + VEHICLE_PROFILE_INSERTION,
        candidate.replace(VEHICLE_PROFILE_BLOCK_TEXT,
                          VEHICLE_PROFILE_BLOCK_TEXT.replace("src/vehicle_cost.cpp", "src/veh.cpp"), 1),
        candidate.replace(VEHICLE_PROFILE_BLOCK_TEXT,
                          VEHICLE_PROFILE_BLOCK_TEXT.replace("/Oy-", "/Oy- /Ob0"), 1),
    ):
        assert not cmake_profile_delta_is_exact(baseline, damaged)

    assert not cmake_profile_delta_is_exact(
        baseline, candidate.replace(UNDO_PROFILE_INSERTION, "", 1))
    assert not cmake_profile_delta_is_exact(
        baseline, candidate + UNDO_PROFILE_INSERTION)
    assert not cmake_profile_delta_is_exact(
        baseline, candidate.replace(UNDO_SOURCE_AFTER, UNDO_SOURCE_BEFORE, 1))
    assert not cmake_profile_delta_is_exact(
        baseline, candidate.replace(UNDO_SOURCE_AFTER,
                                    UNDO_SOURCE_AFTER + "    src/scenario_undo.cpp\n", 1))
    assert not cmake_profile_delta_is_exact(
        baseline, candidate.replace(UNDO_PROFILE_BLOCK_TEXT,
                                    UNDO_PROFILE_BLOCK_TEXT.replace("src/scenario_undo.cpp", "src/general.cpp"), 1))
    assert not cmake_profile_delta_is_exact(
        baseline, candidate.replace(UNDO_PROFILE_BLOCK_TEXT,
                                    UNDO_PROFILE_BLOCK_TEXT.replace("/Oy-", "/Oy- /Ob0"), 1))

    assert not cmake_profile_delta_is_exact(
        baseline, candidate.replace(VIDEO_SOUND_PROFILE_INSERTION, "", 1))
    assert not cmake_profile_delta_is_exact(
        baseline, candidate + VIDEO_SOUND_PROFILE_INSERTION)
    assert not cmake_profile_delta_is_exact(
        baseline, candidate.replace(VIDEO_SOUND_SOURCE_AFTER, VIDEO_SOUND_SOURCE_BEFORE, 1))
    assert not cmake_profile_delta_is_exact(
        baseline, candidate.replace(VIDEO_SOUND_SOURCE_AFTER,
                                    VIDEO_SOUND_SOURCE_AFTER + "    src/video_sound.cpp\n", 1))
    assert not cmake_profile_delta_is_exact(
        baseline, candidate.replace(VIDEO_SOUND_PROFILE_BLOCK_TEXT,
                                    VIDEO_SOUND_PROFILE_BLOCK_TEXT.replace("src/video_sound.cpp", "src/video.cpp"), 1))
    assert not cmake_profile_delta_is_exact(
        baseline, candidate.replace(VIDEO_SOUND_PROFILE_BLOCK_TEXT,
                                    VIDEO_SOUND_PROFILE_BLOCK_TEXT.replace("/Oy-", "/Oy- /Ob0"), 1))

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
    missing_remaining = candidate.replace(REMAINING_PROFILE_INSERTION, "")
    assert not cmake_profile_delta_is_exact(baseline, missing_remaining)
    duplicate_remaining = candidate + "\n" + REMAINING_PROFILE_BLOCK_TEXT + "\n"
    assert not cmake_profile_delta_is_exact(baseline, duplicate_remaining)
    changed_remaining_options = candidate.replace(
        REMAINING_PROFILE_BLOCK_TEXT,
        REMAINING_PROFILE_BLOCK_TEXT.replace('COMPILE_OPTIONS "/Oy-"', 'COMPILE_OPTIONS "/Oy- /Ob0"'),
    )
    assert not cmake_profile_delta_is_exact(baseline, changed_remaining_options)
    extra_remaining_owner = candidate.replace(
        "    src/base.cpp\n", "    src/base.cpp\n    src/polygon.cpp\n"
    )
    assert not cmake_profile_delta_is_exact(baseline, extra_remaining_owner)
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
    duplicate_alphanet_profile = candidate + "\n" + ALPHANET_PROFILE_BLOCK_TEXT + "\n"
    assert not cmake_profile_delta_is_exact(baseline, duplicate_alphanet_profile)
    strings_profile = candidate + (
        '\nset_source_files_properties(src/strings.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")\n'
    )
    assert not cmake_profile_delta_is_exact(baseline, strings_profile)
    for rejected in (
        *REJECTED_OWNERS,
        *PREVIOUS_REJECTED_OWNERS,
        "src/base.cpp",
        "src/win.cpp",
        "src/technology_queries.cpp",
        "src/polygon.cpp",
        "src/texture.cpp",
        "src/general.cpp",
        "src/popup.cpp",
    ):
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
    remaining_task = json.loads(
        (ROOT / "recovery/tasks/remaining-owner-frames-oct17.json").read_text(
            encoding="utf-8"
        )
    )
    assert remaining_task["base_revision"] == REMAINING_BASE_REVISION
    assert remaining_task["editable_files"] == [
        "CMakeLists.txt",
        "recovery/tests/test_application_compiler_profiles.py",
    ]
    video_sound_task = json.loads(
        (ROOT / "recovery/tasks/video-sound-owner-oct17.json").read_text(encoding="utf-8")
    )
    assert video_sound_task["base_revision"] == VIDEO_SOUND_BASE_REVISION
    undo_task = json.loads(
        (ROOT / "recovery/tasks/general-owner-split-oct17.json").read_text(encoding="utf-8")
    )
    assert undo_task["base_revision"] == UNDO_BASE_REVISION
    baseline_cmake = baseline_text("CMakeLists.txt")
    current_cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    assert application_global_policy(current_cmake) == application_global_policy(baseline_cmake)

    source_list = re.search(r"set\(OPENSMACX_SOURCES(.*?)\n\)", current_cmake, re.S)
    assert source_list is not None
    source_rows = re.findall(r"src/[A-Za-z0-9_./-]+\.(?:cpp|c)", source_list.group(1))
    application_sources = set(source_rows)
    assert all(owner in application_sources for owner in NEW_OWNERS + REMAINING_OWNERS + PREVIOUS_OWNERS)
    assert all(owner in application_sources for owner in REJECTED_OWNERS)
    assert all(owner in application_sources for owner in PREVIOUS_REJECTED_OWNERS)
    assert source_rows[source_rows.index("src/alphanet.cpp") + 1] == "src/alphanet_records.cpp"
    assert "src/sprite.cpp" in application_sources
    assert "src/net_class.cpp" in application_sources
    assert "src/alpha.cpp" in application_sources
    assert "src/win.cpp" in application_sources
    assert "src/video.cpp" in application_sources
    assert "src/video_queue.cpp" in application_sources
    assert "src/alphamovie.cpp" in application_sources
    assert "src/mci_video.cpp" in application_sources
    assert source_rows[source_rows.index("src/video.cpp") + 1] == "src/video_queue.cpp"
    assert source_rows[source_rows.index("src/alphamovie.cpp") + 1] == "src/mci_video.cpp"
    assert "src/video_sound.cpp" not in application_sources
    assert "src/voice.cpp" not in application_sources
    assert "src/sound.cpp" in application_sources
    assert "src/scenario_undo.cpp" in application_sources
    assert "src/header_io.cpp" in application_sources
    assert "src/crt_random.cpp" in application_sources
    assert "src/vehicle_cost.cpp" in application_sources
    assert "src/vehicle_orders.cpp" in application_sources
    assert "src/tutwin.cpp" in application_sources
    assert "src/tutwin_callouts.cpp" not in application_sources
    assert "src/world.cpp" in application_sources
    assert source_rows[source_rows.index("src/map.cpp") + 1] == "src/world.cpp"
    inline_owners = ("src/pulldown.cpp", "src/dipedit.cpp")
    assert all(owner in application_sources for owner in inline_owners)

    actual_profiles = effective_source_profiles(source_property_blocks(current_cmake))
    expected_profiles = effective_source_profiles(EXPECTED_CMAKE_PROFILES.items())
    for owner, options in expected_profiles.items():
        assert actual_profiles.get(owner) == options, (owner, actual_profiles.get(owner))
    assert actual_profiles.get("src/newgame.cpp") == ("/Oy-",)
    profiled_sources = set(actual_profiles)
    assert "src/alpha.cpp" in profiled_sources
    assert actual_profiles["src/alpha.cpp"] == ("/Oy-",)
    assert actual_profiles["src/alphanet.cpp"] == ("/Oy-",)
    assert "src/alphanet_records.cpp" not in profiled_sources
    assert all(owner in profiled_sources for owner in REMAINING_OWNERS)
    assert actual_profiles["src/sound.cpp"] == ("/Oy-",)
    assert actual_profiles["src/scenario_undo.cpp"] == ("/Oy-",)
    assert actual_profiles["src/crt_random.cpp"] == ("/Oy-",)
    assert actual_profiles["src/header_io.cpp"] == ("/Oy-",)
    assert actual_profiles["src/vehicle_cost.cpp"] == ("/Oy-",)
    assert actual_profiles["src/vehicle_orders.cpp"] == ("/Oy-",)
    assert actual_profiles["src/world.cpp"] == actual_profiles["src/map.cpp"] == ("/Oy-",)
    assert "src/video.cpp" not in profiled_sources
    assert "src/alphamovie.cpp" not in profiled_sources
    assert "src/general.cpp" not in profiled_sources
    assert "src/sprite.cpp" not in profiled_sources
    assert "src/net_class.cpp" not in profiled_sources
    appended_profiles = tuple(source_append_option_blocks(current_cmake))
    assert appended_profiles.count((inline_owners, ("$<$<NOT:$<CONFIG:Debug>>:/Ob1>",))) == 1
    appended_effective_profiles = effective_source_profiles(appended_profiles)
    append_counts = collections.Counter(
        owner for owners, _ in appended_profiles for owner in owners
    )
    assert all(append_counts[owner] == 1 for owner in inline_owners)
    assert all(appended_effective_profiles[owner] == ("$<$<NOT:$<CONFIG:Debug>>:/Ob1>",) for owner in inline_owners)
    assert actual_profiles["src/dipedit.cpp"] == ("/Oy-",)
    assert all(owner not in profiled_sources for owner in inline_owners
               if owner != "src/dipedit.cpp")
    denied_owners = ((set(REJECTED_OWNERS) | set(PREVIOUS_REJECTED_OWNERS))
                     - set(inline_owners) - set(MEDIA_DIAGNOSTIC_OWNERS)
                     - {"src/newgame.cpp"})
    all_cmake_profile_sources = profiled_sources | {
        owner for owners, _ in appended_profiles for owner in owners
    }
    assert all(owner not in all_cmake_profile_sources for owner in MEDIA_DIAGNOSTIC_OWNERS)
    assert all(owner not in all_cmake_profile_sources for owner in denied_owners)

    baseline_project = json.loads(baseline_text("recovery/project.json"))
    current_project = json.loads(
        (ROOT / "recovery/project.json").read_text(encoding="utf-8")
    )
    baseline_measurement = baseline_project["measurement"]
    current_measurement = current_project["measurement"]
    for key in ("backend", "compiler_profile", "flag_sets", "layout_flags"):
        assert current_measurement[key] == baseline_measurement[key], key
    assert current_measurement["flag_sets"] == EXPECTED_FLAG_SETS
    unit_profiles = current_measurement["unit_flag_sets"]
    assert "src/newgame.cpp" not in unit_profiles
    for owner, expected in EXISTING_MEASURED_PROFILES.items():
        assert unit_profiles[owner] == expected, (owner, unit_profiles.get(owner))
    for owner in NEW_OWNERS + tuple(
        owner for owner in REMAINING_OWNERS if owner != "src/technology_queries.cpp"
    ) + tuple(denied_owners) + (
        "src/win.cpp", "src/alpha.cpp",
        "src/scenario_undo.cpp",
        "src/header_io.cpp", "src/crt_random.cpp",
        "src/vehicle_cost.cpp", "src/vehicle_orders.cpp",
    ):
        assert owner not in unit_profiles, (owner, unit_profiles.get(owner))
    base_profile = ["/c", "/O2", "/Gy", "/GR-", "/GX", "/Ob1"]
    frame_profile = ["/c", "/O2", "/Gy", "/GR-", "/Oy-", "/GX", "/Ob1"]
    for owner in inline_owners:
        expected = frame_profile if owner == "src/dipedit.cpp" else base_profile
        assert unit_profiles[owner] == {"flags": expected}, (owner, unit_profiles.get(owner))
    for owner in MEDIA_DIAGNOSTIC_OWNERS:
        assert unit_profiles[owner] == {"flags": MEDIA_DIAGNOSTIC_FLAGS}, (
            owner, unit_profiles.get(owner)
        )
