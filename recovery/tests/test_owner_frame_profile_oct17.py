import json
import pathlib
import re


ROOT = pathlib.Path(__file__).parents[2]
OWNERS = (
    "src/map_altitude.cpp",
    "src/wave.cpp",
)


def source_property_blocks(cmake):
    uncommented = re.sub(r"(?m)#.*$", "", cmake)
    for body in re.findall(r"set_source_files_properties\s*\((.*?)\)", uncommented, re.S):
        files, separator, properties = body.partition("PROPERTIES")
        if separator:
            sources = re.findall(r"src/[A-Za-z0-9_./-]+\.(?:cpp|c)", files)
            options = re.search(r'\bCOMPILE_OPTIONS\s+"([^"]*)"', properties)
            yield sources, options.group(1).split() if options else []


def test_owner_frame_policy_preserves_existing_measurement_profiles():
    cmake = (ROOT / "CMakeLists.txt").read_text()
    source_list = re.search(r"set\(OPENSMACX_SOURCES(.*?)\n\)", cmake, re.S).group(1)
    assert all(source in source_list.split() for source in OWNERS)

    blocks = list(source_property_blocks(cmake))
    # Its frame-preserving measurement profile does not authorize changing
    # the application recipe: that TU also emits frame-free Log definitions.
    assert not any("src/technology_queries.cpp" in sources for sources, _ in blocks)
    for owner in OWNERS:
        matches = [options for sources, options in blocks if owner in sources]
        assert matches == [["/Oy-"]], (owner, matches)

    project = json.loads((ROOT / "recovery/project.json").read_text())
    profiles = project["measurement"]["unit_flag_sets"]
    for owner in OWNERS:
        flags = profiles[owner]["flags"]
        assert flags.count("/Oy-") == 1, (owner, flags)
        assert flags.count("/Ob2") == 1, (owner, flags)

    for existing in ("src/interface_palette.cpp", "src/interface_fonts.cpp", "src/sound.cpp"):
        matches = [options for sources, options in blocks if existing in sources]
        assert matches == [["/Oy-"]], (existing, matches)
