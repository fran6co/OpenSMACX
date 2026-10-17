"""Guard the narrow DesignWin, TutWin and XPops frame profile."""
from __future__ import annotations

import collections
import json
import pathlib
import re
import subprocess


ROOT = pathlib.Path(__file__).resolve().parents[2]
NEW_OWNERS = ("src/designwin.cpp", "src/tutwin.cpp", "src/xpops.cpp")
UNCHANGED_NEIGHBORS = ("src/netdaemon.cpp", "src/win.cpp")


def source_property_blocks(cmake: str):
    uncommented = re.sub(r"(?m)#.*$", "", cmake)
    for body in re.findall(r"set_source_files_properties\s*\((.*?)\)", uncommented, re.S):
        files, separator, properties = body.partition("PROPERTIES")
        if separator:
            sources = re.findall(r"src/[A-Za-z0-9_./-]+\.(?:cpp|c)", files)
            option = re.search(r'\bCOMPILE_OPTIONS\s+"([^"]*)"', properties)
            yield tuple(sorted(sources)), tuple(option.group(1).split() if option else ())


def baseline_text(revision: str, relative_path: str) -> str:
    result = subprocess.run(
        ["git", "show", f"{revision}:{relative_path}"],
        cwd=ROOT,
        check=True,
        capture_output=True,
        text=True,
    )
    return result.stdout


def is_exact_profile_extension(baseline, current, expected_addition) -> bool:
    expected = baseline.copy()
    expected[expected_addition] += 1
    return current == expected


def test_profile_guard_rejects_dropped_profiles_and_extra_owners():
    previous = (("src/previous.cpp",), ("/Oy-",))
    addition = (("src/designwin.cpp", "src/tutwin.cpp", "src/xpops.cpp"), ("/Oy-",))
    baseline = collections.Counter({previous: 1})
    expected = collections.Counter({previous: 1, addition: 1})

    assert is_exact_profile_extension(baseline, expected, addition)
    assert not is_exact_profile_extension(baseline, collections.Counter({addition: 1}), addition)
    extra_owner = (("src/extra.cpp",), ("/Oy-",))
    assert not is_exact_profile_extension(
        baseline, collections.Counter({previous: 1, addition: 1, extra_owner: 1}), addition
    )


def test_three_owner_frame_profile_is_narrow_and_preserves_measurement_policy():
    task = json.loads((ROOT / "recovery/tasks/design-tutorial-popup-frame-oct17.json").read_text())
    baseline_revision = task["base_revision"]
    baseline_cmake = baseline_text(baseline_revision, "CMakeLists.txt")
    current_cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")

    source_list = re.search(r"set\(OPENSMACX_SOURCES(.*?)\n\)", current_cmake, re.S)
    assert source_list is not None
    assert all(owner in source_list.group(1).split() for owner in NEW_OWNERS)

    baseline_blocks = collections.Counter(source_property_blocks(baseline_cmake))
    current_blocks = collections.Counter(source_property_blocks(current_cmake))
    expected_addition = (tuple(sorted(NEW_OWNERS)), ("/Oy-",))
    assert is_exact_profile_extension(baseline_blocks, current_blocks, expected_addition)
    for owner in NEW_OWNERS:
        matches = [options for sources, options in source_property_blocks(current_cmake) if owner in sources]
        assert matches == [("/Oy-",)], (owner, matches)
    for owner in UNCHANGED_NEIGHBORS:
        assert not any(owner in sources for sources, _ in source_property_blocks(current_cmake))

    baseline_project = json.loads(baseline_text(baseline_revision, "recovery/project.json"))
    current_project = json.loads((ROOT / "recovery/project.json").read_text(encoding="utf-8"))
    assert current_project["measurement"] == baseline_project["measurement"]
