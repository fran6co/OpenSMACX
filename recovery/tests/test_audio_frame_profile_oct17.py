"""Guard the narrow Voice/Wave_Device frame-pointer build profile."""
from __future__ import annotations

import json
import pathlib
import re


ROOT = pathlib.Path(__file__).resolve().parents[2]
OWNERS = ("src/voice.cpp", "src/wave_device.cpp")
ESTABLISHED = (
    "src/map_altitude.cpp",
    "src/wave.cpp",
    "src/interface_palette.cpp",
    "src/interface_fonts.cpp",
    "src/sound.cpp",
)
MEASURED_PROFILE = ["/c", "/O2", "/Gy", "/GR-", "/Oy-", "/GX", "/Ob2"]
ESTABLISHED_FLAG_SETS = [
    ["/c", "/O2", "/Gy", "/GR-", "/GX"],
    ["/c", "/O2", "/Gy", "/GR-", "/Oy-", "/GX"],
]
ESTABLISHED_UNIT_PROFILES = {
    "src/map_altitude.cpp": {"flags": MEASURED_PROFILE},
    "src/wave.cpp": {"flags": MEASURED_PROFILE},
    "src/interface_palette.cpp": [1],
    "src/interface_fonts.cpp": [1],
    "src/sound.cpp": {"flags": MEASURED_PROFILE},
}


def source_property_blocks(cmake: str):
    uncommented = re.sub(r"(?m)#.*$", "", cmake)
    for body in re.findall(r"set_source_files_properties\s*\((.*?)\)", uncommented, re.S):
        sources_text, separator, properties = body.partition("PROPERTIES")
        if separator:
            sources = re.findall(r"src/[A-Za-z0-9_./-]+\.(?:cpp|c)", sources_text)
            options = re.search(r'\bCOMPILE_OPTIONS\s+"([^"]*)"', properties)
            yield sources, options.group(1).split() if options else []


def test_audio_frame_profile_is_source_specific_and_preserves_existing_profiles():
    cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    source_list_match = re.search(r"set\(OPENSMACX_SOURCES(.*?)\n\)", cmake, re.S)
    assert source_list_match is not None
    application_sources = source_list_match.group(1).split()
    assert all(owner in application_sources for owner in OWNERS)

    blocks = list(source_property_blocks(cmake))
    for owner in OWNERS:
        matches = [options for sources, options in blocks if owner in sources]
        assert matches == [["/Oy-"]], (owner, matches)

    # Existing source-specific profiles remain singular and unchanged.
    for owner in ESTABLISHED:
        matches = [options for sources, options in blocks if owner in sources]
        assert matches == [["/Oy-"]], (owner, matches)

    # Technology keeps its existing measured profile, but does not inherit a
    # source-specific application option from this frame-pointer recovery.
    assert "src/technology_queries.cpp" in application_sources
    assert not any("src/technology_queries.cpp" in sources for sources, _ in blocks)

    project = json.loads((ROOT / "recovery/project.json").read_text(encoding="utf-8"))
    measurement = project["measurement"]
    assert measurement["flag_sets"] == ESTABLISHED_FLAG_SETS
    profiles = measurement["unit_flag_sets"]
    assert all(owner not in profiles for owner in OWNERS)

    # Existing owner profiles are untouched; these are not permission to
    # globalize `/Oy-` or adopt the technology unit's separate frame setting.
    for owner in ESTABLISHED:
        assert profiles[owner] == ESTABLISHED_UNIT_PROFILES[owner], (owner, profiles[owner])
    assert profiles["src/technology_queries.cpp"]["flags"] == MEASURED_PROFILE

