"""Guard the source-specific frame profile for Midi and sound-device owners."""
from __future__ import annotations

import json
import pathlib
import re


ROOT = pathlib.Path(__file__).resolve().parents[2]
NEW_OWNERS = ("src/midi.cpp", "src/sounddevice.cpp", "src/sound_loader.cpp")
ESTABLISHED_OWNERS = (
    "src/map_altitude.cpp",
    "src/wave.cpp",
    "src/voice.cpp",
    "src/wave_device.cpp",
    "src/interface_palette.cpp",
    "src/interface_fonts.cpp",
    "src/sound.cpp",
)
FRAME_PROFILE = ["/c", "/O2", "/Gy", "/GR-", "/Oy-", "/GX", "/Ob2"]
EXISTING_FLAG_SETS = [
    ["/c", "/O2", "/Gy", "/GR-", "/GX"],
    ["/c", "/O2", "/Gy", "/GR-", "/Oy-", "/GX"],
]
EXISTING_UNIT_PROFILES = {
    "src/map_altitude.cpp": {"flags": FRAME_PROFILE},
    "src/wave.cpp": {"flags": FRAME_PROFILE},
    "src/voice.cpp": None,
    "src/wave_device.cpp": None,
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


def test_device_frame_profile_is_narrow_and_preserves_existing_settings():
    cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    source_list_match = re.search(r"set\(OPENSMACX_SOURCES(.*?)\n\)", cmake, re.S)
    assert source_list_match is not None
    application_sources = source_list_match.group(1).split()
    assert all(owner in application_sources for owner in NEW_OWNERS)

    blocks = list(source_property_blocks(cmake))
    for owner in NEW_OWNERS:
        matches = [options for sources, options in blocks if owner in sources]
        assert matches == [["/Oy-"]], (owner, matches)

    # Retain the earlier terrain, voice/wave, interface and Sound profiles
    # exactly once; do not adopt Technology's source-specific profile here.
    for owner in ESTABLISHED_OWNERS:
        matches = [options for sources, options in blocks if owner in sources]
        assert matches == [["/Oy-"]], (owner, matches)
    assert "src/technology_queries.cpp" in application_sources
    assert not any("src/technology_queries.cpp" in sources for sources, _ in blocks)

    measurement = json.loads(
        (ROOT / "recovery/project.json").read_text(encoding="utf-8")
    )["measurement"]
    assert measurement["flag_sets"] == EXISTING_FLAG_SETS
    profiles = measurement["unit_flag_sets"]
    assert all(owner not in profiles for owner in NEW_OWNERS)
    for owner, expected in EXISTING_UNIT_PROFILES.items():
        if expected is None:
            assert owner not in profiles, (owner, profiles.get(owner))
        else:
            assert profiles[owner] == expected, (owner, profiles.get(owner))

    # Technology retains its preexisting measured frame profile, but is not
    # added to a CMake application-source profile by this recovery.
    assert profiles["src/technology_queries.cpp"] == {"flags": FRAME_PROFILE}
