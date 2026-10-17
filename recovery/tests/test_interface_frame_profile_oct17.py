"""Guard narrow frame-pointer profiles for interface-owned translation units."""
from __future__ import annotations

import json
import pathlib
import re


ROOT = pathlib.Path(__file__).resolve().parents[2]
NEW_OWNERS = ("src/datalink.cpp", "src/maininterface.cpp", "src/console.cpp")
WIN_OWNER = "src/win.cpp"
PREVIOUS_OWNERS = (
    "src/map_altitude.cpp",
    "src/wave.cpp",
    "src/voice.cpp",
    "src/wave_device.cpp",
    "src/midi.cpp",
    "src/sounddevice.cpp",
    "src/sound_loader.cpp",
    "src/interface_palette.cpp",
    "src/interface_fonts.cpp",
    "src/sound.cpp",
)
FRAME_PROFILE = ["/c", "/O2", "/Gy", "/GR-", "/Oy-", "/GX", "/Ob2"]
EXISTING_FLAG_SETS = [
    ["/c", "/O2", "/Gy", "/GR-", "/GX"],
    ["/c", "/O2", "/Gy", "/GR-", "/Oy-", "/GX"],
]
PREVIOUS_UNIT_PROFILES = {
    "src/map_altitude.cpp": {"flags": FRAME_PROFILE},
    "src/wave.cpp": {"flags": FRAME_PROFILE},
    "src/voice.cpp": None,
    "src/wave_device.cpp": None,
    "src/midi.cpp": None,
    "src/sounddevice.cpp": None,
    "src/sound_loader.cpp": None,
    "src/interface_palette.cpp": [1],
    "src/interface_fonts.cpp": [1],
    "src/sound.cpp": {"flags": FRAME_PROFILE},
}


def source_property_blocks(cmake: str):
    uncommented = re.sub(r"(?m)#.*$", "", cmake)
    for body in re.findall(r"set_source_files_properties\s*\((.*?)\)", uncommented, re.S):
        sources_text, separator, properties = body.partition("PROPERTIES")
        if separator:
            sources = re.findall(r"src/[A-Za-z0-9_./-]+\.(?:cpp|c)", sources_text)
            options = re.search(r'\bCOMPILE_OPTIONS\s+"([^"]*)"', properties)
            yield sources, options.group(1).split() if options else []


def test_interface_frame_profiles_are_narrow_and_preserve_existing_settings():
    cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    source_list_match = re.search(r"set\(OPENSMACX_SOURCES(.*?)\n\)", cmake, re.S)
    assert source_list_match is not None
    application_sources = source_list_match.group(1).split()
    assert all(owner in application_sources for owner in NEW_OWNERS)
    assert WIN_OWNER in application_sources

    blocks = list(source_property_blocks(cmake))
    for owner in NEW_OWNERS:
        matches = [options for sources, options in blocks if owner in sources]
        assert matches == [["/Oy-"]], (owner, matches)
    assert not any(WIN_OWNER in sources for sources, _ in blocks)

    # All previously established source properties remain singular and intact.
    for owner in PREVIOUS_OWNERS:
        matches = [options for sources, options in blocks if owner in sources]
        assert matches == [["/Oy-"]], (owner, matches)

    project = json.loads(
        (ROOT / "recovery/project.json").read_text(encoding="utf-8")
    )["measurement"]
    assert project["flag_sets"] == EXISTING_FLAG_SETS
    profiles = project["unit_flag_sets"]
    assert all(owner not in profiles for owner in NEW_OWNERS)
    assert WIN_OWNER not in profiles
    for owner, expected in PREVIOUS_UNIT_PROFILES.items():
        if expected is None:
            assert owner not in profiles, (owner, profiles.get(owner))
        else:
            assert profiles[owner] == expected, (owner, profiles.get(owner))

    # Technology's existing measured profile remains separate from CMake TU
    # policy and is not copied to Win or the new interface owners.
    assert "src/technology_queries.cpp" in application_sources
    assert not any(
        "src/technology_queries.cpp" in sources or WIN_OWNER in sources
        for sources, _ in blocks
    )
    assert profiles["src/technology_queries.cpp"] == {"flags": FRAME_PROFILE}
