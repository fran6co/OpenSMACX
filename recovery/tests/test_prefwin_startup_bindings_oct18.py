"""Exact scoped inverse and damage controls for PrefWin CRT-entry bindings."""
from __future__ import annotations

from pathlib import Path
import subprocess

import pytest

from match_recovery.source import read_text


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "CMakeLists.txt").is_file())
BASE_REVISION = "75b8ac5c72a535bfa012883d3ba5aa5c34d4fa2a"
GEOMETRY = ROOT / "src/prefwin_geometry.cpp"
PROJECT = ROOT / "recovery/project.json"
HEADER = ROOT / "src/prefwin_geometry.h"
CMAKE = ROOT / "CMakeLists.txt"
ADDRESSES = (
    0x48D550, 0x48D560, 0x48D570, 0x48D590, 0x48D5B0, 0x48D5D0,
    0x48D5F0, 0x48D610, 0x48D630, 0x48D650, 0x48D670, 0x48D690,
    0x48D6B0, 0x48D6D0, 0x48D6F0, 0x48D710, 0x48D730, 0x48D750,
    0x48D760, 0x48D770, 0x48D790, 0x48D7B0, 0x48D7D0, 0x48D7F0,
    0x48D810, 0x48D820,
)
OLD_RATIONALE = (
    b"// The /Gy measurement recipes emit an arithmetic helper followed by a\n"
    b"// tail-jump wrapper registered in .CRT$XCU. Bind the original arithmetic body\n"
    b"// to the helper, not the wrapper. The ordinary non-/Gy application recipe\n"
    b"// numbers its direct initializer differently; these bindings alone do not\n"
    b"// establish linked startup correspondence or historical TU boundaries.\n"
)
NEW_RATIONALE = (
    b"// The original CRT slots 0x0068257C..0x006825E0 select these 26 arithmetic\n"
    b"// initializers directly, in declaration order. Bind the production /Ob2\n"
    b"// CRT-selected entries, not the separate helper names removed by inlining.\n"
    b"// The explicit measurement context retains that selection with /Gy as well.\n"
    b"// The entries at 0x0048D610, 0x0048D790 and 0x0048D7F0 still differ in input\n"
    b"// load order despite masked-byte equality. These bindings do not establish\n"
    b"// whole-program startup order, linked retention or historical TU boundaries.\n"
)
FLAG_BLOCK = (
    b'      "src/prefwin_geometry.cpp": {\n'
    b'        "flags": ["/c", "/O2", "/Gy", "/GR-", "/GX", "/Ob2"]\n'
    b"      },\n"
)
PROJECT_ANCHOR = b'      "src/technology_queries.cpp": {\n'
UNCHANGED_PATHS = ("src/prefwin_geometry.h", "CMakeLists.txt")
PRESERVED_FIELDS = (
    "mode", "state", "name", "image_spans", "body", "context", "kind",
    "exclusion", "byte_exact", "semantic", "recipe", "levers", "ruled_out",
    "unrecoverable", "deferred", "source_role", "source_role_reason",
    "function_fragment",
)


def _base(relative: str) -> bytes:
    return subprocess.run(
        ["git", "show", f"{BASE_REVISION}:{relative}"], cwd=ROOT,
        check=True, stdout=subprocess.PIPE,
    ).stdout


def _project_geometry(base: bytes) -> bytes:
    assert base.count(OLD_RATIONALE) == 1
    projected = base.replace(OLD_RATIONALE, NEW_RATIONALE, 1)
    for index in range(26):
        old = f"// symbol    _$E{1 + 3 * index}\n".encode()
        new = f"// symbol    _$E{2 + 3 * index}\n".encode()
        assert projected.count(old) == 1, (index, old)
        projected = projected.replace(old, new, 1)
    return projected


def _project_configuration(base: bytes) -> bytes:
    assert base.count(PROJECT_ANCHOR) == 1
    assert base.count(FLAG_BLOCK) == 0
    return base.replace(PROJECT_ANCHOR, FLAG_BLOCK + PROJECT_ANCHOR, 1)


def _assert_inverse(geometry: bytes, project: bytes, header: bytes, cmake: bytes) -> None:
    base_geometry = _base("src/prefwin_geometry.cpp")
    base_project = _base("recovery/project.json")
    assert geometry == _project_geometry(base_geometry), "unadmitted geometry source delta"
    assert project == _project_configuration(base_project), "unadmitted compiler-profile delta"
    assert header == _base("src/prefwin_geometry.h"), "geometry header changed"
    assert cmake == _base("CMakeLists.txt"), "application CMake changed"

    before = {row.address: row for row in read_text(
        base_geometry.decode("utf-8"), Path("src/prefwin_geometry.cpp"))
        if row.address in ADDRESSES}
    after = {row.address: row for row in read_text(
        geometry.decode("utf-8"), Path("src/prefwin_geometry.cpp"))
        if row.address in ADDRESSES}
    assert list(before) == list(ADDRESSES) and list(after) == list(ADDRESSES)
    for index, address in enumerate(ADDRESSES):
        old, new = before[address], after[address]
        for field in PRESERVED_FIELDS:
            assert getattr(new, field) == getattr(old, field), (hex(address), field)
        assert new.symbol == f"_$E{2 + 3 * index}"


def _actual() -> tuple[bytes, bytes, bytes, bytes]:
    return GEOMETRY.read_bytes(), PROJECT.read_bytes(), HEADER.read_bytes(), CMAKE.read_bytes()


def test_prefwin_binding_update_is_exact_against_immutable_task_base() -> None:
    _assert_inverse(*_actual())


@pytest.mark.parametrize(
    "path_index,mutate",
    [
        (0, lambda data: data.replace(b"// symbol    _$E2\n", b"// symbol    _$E3\n", 1)),
        (0, lambda data: data.replace(b"// ORIGINAL: 0x0048D550 ", b"// ORIGINAL: 0x0048D551 ", 1)),
        (0, lambda data: data.replace(b"BYTE_EXACT", b"", 1)),
        (0, lambda data: data.replace(b"LEVER:", b"TRIED:", 1)),
        (0, lambda data: data.replace(b"0x00686B6C 0x00686B70", b"0x00686B70 0x00686B74", 1)),
        (0, lambda data: data.replace(b"+ 23u", b"+ 24u", 1)),
        (1, lambda data: data.replace(b'"/GX", "/Ob2"', b'"/Oy-", "/GX", "/Ob2"', 1)),
        (1, lambda data: data.replace(b'"/Ob2"]', b'"/Ob1"]', 1)),
        (2, lambda data: data + b"\n// changed geometry interface\n"),
        (3, lambda data: data.replace(b"src/prefwin_geometry.cpp", b"src/prefwin_other.cpp", 1)),
    ],
)
def test_prefwin_binding_inverse_rejects_unapproved_damages(path_index, mutate) -> None:
    sources = list(_actual())
    original = sources[path_index]
    sources[path_index] = mutate(original)
    assert sources[path_index] != original
    with pytest.raises(AssertionError):
        _assert_inverse(*sources)
