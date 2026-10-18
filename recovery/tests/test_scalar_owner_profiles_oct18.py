"""Narrow CMake contract for the measured scalar-owner source profiles."""
from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[2]
OWNERS = {
    "src/byte_utilities.cpp",
    "src/shared_word.cpp",
    "src/span_math.cpp",
    "src/scale_adjustment.cpp",
}


def test_scalar_helper_owners_have_only_their_measured_frame_profile():
    cmake = (ROOT / "CMakeLists.txt").read_text()
    groups = []
    for match in re.finditer(
        r"set_source_files_properties\((.*?)\s+PROPERTIES\s+COMPILE_OPTIONS\s+\"([^\"]+)\"\s*\)",
        cmake,
        re.DOTALL,
    ):
        files = set(re.findall(r"src/[A-Za-z0-9_./-]+\.cpp", match.group(1)))
        groups.append((files, match.group(2)))

    owner_groups = [
        (files, options)
        for files, options in groups
        if files & OWNERS
    ]
    assert owner_groups == [(OWNERS, "/Oy-")]
    assert all("src/general.cpp" not in files for files, _ in groups)

    sources = re.search(r"set\(OPENSMACX_SOURCES\s+(.*?)\n\)", cmake, re.DOTALL)
    assert sources is not None
    source_rows = re.findall(r"src/[A-Za-z0-9_./-]+\.cpp", sources.group(1))
    assert source_rows.count("src/scale_adjustment.cpp") == 1
