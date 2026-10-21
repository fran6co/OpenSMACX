"""Exact scoped inverse and damage controls for PrefWin CRT-entry bindings."""
from __future__ import annotations

import json
from dataclasses import replace
from pathlib import Path
import re
import subprocess

import pytest

from match_recovery.source import read_text
from match_recovery.source.writer import write_bytes


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
    b"// These bindings do not establish whole-program startup order, linked\n"
    b"// retention or historical TU boundaries.\n"
)
INITIALIZER_TRANSITIONS = (
    (
        b"int32_t PrefWinGeometryValue_85789C = static_cast<int32_t>(static_cast<uint32_t>(PrefWinButtonWidth) + static_cast<uint32_t>(PrefWinGeometryConstant_686B74) + static_cast<uint32_t>(PrefWinGeometryValue_857888));",
        b"int32_t PrefWinGeometryValue_85789C = static_cast<int32_t>(static_cast<uint32_t>(PrefWinGeometryValue_857888) + static_cast<uint32_t>(PrefWinButtonWidth) + static_cast<uint32_t>(PrefWinGeometryConstant_686B74));",
    ),
    (
        b"int32_t PrefWinGeometryValue_8578CC = static_cast<int32_t>(static_cast<uint32_t>(PrefWinToggleWidth) + static_cast<uint32_t>(PrefWinGeometryConstant_686B74) + static_cast<uint32_t>(PrefWinGeometryValue_8578B4));",
        b"int32_t PrefWinGeometryValue_8578CC = static_cast<int32_t>(static_cast<uint32_t>(PrefWinGeometryValue_8578B4) + static_cast<uint32_t>(PrefWinToggleWidth) + static_cast<uint32_t>(PrefWinGeometryConstant_686B74));",
    ),
    (
        b"int32_t PrefWinGeometryValue_8577E0 = static_cast<int32_t>(static_cast<uint32_t>(PrefWinGeometryValue_8578B0) - static_cast<uint32_t>(PrefWinGeometryConstant_686B78) - static_cast<uint32_t>(PrefWinGeometryValue_8577DC));",
        b"int32_t PrefWinGeometryValue_8577E0 = static_cast<int32_t>(static_cast<uint32_t>(PrefWinGeometryValue_8578B0) - static_cast<uint32_t>(PrefWinGeometryValue_8577DC) - static_cast<uint32_t>(PrefWinGeometryConstant_686B78));",
    ),
)
RECORDED_ADDRESSES = {0x48D610, 0x48D7F0}
SELECTED_FLAGS = ["/c", "/O2", "/Gy", "/GR-", "/GX", "/Ob2"]
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
    for old, new in INITIALIZER_TRANSITIONS:
        assert projected.count(old) == 1, old
        projected = projected.replace(old, new, 1)
    # Generate expected claim changes through the same native source writer
    # used for recorded annotations; do not hand-author claim tokens.
    old_records = [row for row in read_text(
        projected.decode("utf-8"), Path("src/prefwin_geometry.cpp"))
        if row.address in RECORDED_ADDRESSES]
    assert {row.address for row in old_records} == RECORDED_ADDRESSES
    updated = [replace(row, byte_exact=True, semantic=False) for row in old_records]
    projected = write_bytes(projected, updated)
    return projected


def _project_without_unit_profiles(project: bytes) -> dict:
    value = json.loads(project)
    value["measurement"].pop("unit_flag_sets")
    return value


def _without_source_profiles(cmake: bytes) -> bytes:
    # Owner-local source recipes and PrefWin's local source rows are checked
    # below. Normalize the broader source inventory so unrelated later owner
    # additions do not become part of this binding test's scope. Keep global
    # compiler, target, and linker policy literal.
    cmake = re.sub(rb"(?m)^[ \t]*(?:#[^\n]*)?(?:\n|$)", b"", cmake)
    pattern = re.compile(
        rb"(?ms)^[ \t]*(set_source_files_properties|set_property)\((.*?)\)[ \t]*(?:\n|$)"
    )
    ranges = []
    for match in pattern.finditer(cmake):
        if match.group(1) == b"set_property" and not re.match(
            rb"\s*SOURCE(?:\s|$)", match.group(2)
        ):
            continue
        ranges.append((match.start(), match.end()))
    normalized = cmake
    for start, end in reversed(ranges):
        normalized = normalized[:start] + normalized[end:]
    normalized, count = re.subn(
        rb"(?ms)^set\(OPENSMACX_SOURCES\n.*?^\)",
        b"set(OPENSMACX_SOURCES\n    <other application source rows>\n)",
        normalized,
    )
    assert count == 1, "expected one application source inventory"
    return normalized


def _has_source_profile(cmake: bytes, source_path: bytes) -> bool:
    pattern = re.compile(
        rb"(?ms)^[ \t]*(set_source_files_properties|set_property)\((.*?)\)[ \t]*(?:\n|$)"
    )
    for match in pattern.finditer(cmake):
        command, body = match.group(1), match.group(2)
        if command == b"set_source_files_properties" and source_path in body:
            return True
        if command == b"set_property" and re.match(rb"\s*SOURCE(?:\s|$)", body):
            property_at = re.search(rb"\bPROPERTY\s+", body)
            if property_at is not None and source_path in body[:property_at.start()]:
                return True
    return False


def _source_rows(cmake: bytes) -> list[bytes]:
    match = re.search(rb"(?ms)^set\(OPENSMACX_SOURCES\n(.*?)^\)", cmake)
    assert match is not None
    return re.findall(rb"^\s+(src/[^\s]+)\s*$", match.group(1), re.M)


def _assert_inverse(geometry: bytes, project: bytes, header: bytes, cmake: bytes) -> None:
    base_geometry = _base("src/prefwin_geometry.cpp")
    base_project = _base("recovery/project.json")
    base_cmake = _base("CMakeLists.txt")
    assert geometry == _project_geometry(base_geometry), "unadmitted geometry source delta"
    assert _project_without_unit_profiles(project) == _project_without_unit_profiles(base_project), (
        "global compiler/measurement policy changed"
    )
    project_value = json.loads(project)
    assert project_value["measurement"]["unit_flag_sets"].get(
        "src/prefwin_geometry.cpp", {}
    ).get("flags") == SELECTED_FLAGS, "PrefWin selected recipe changed"
    assert header == _base("src/prefwin_geometry.h"), "geometry header changed"
    assert _without_source_profiles(cmake) == _without_source_profiles(base_cmake), (
        "global application compiler/link/source policy changed"
    )
    source_rows = _source_rows(cmake)
    assert source_rows.count(b"src/prefwin_geometry.cpp") == 1
    prefwin_index = source_rows.index(b"src/prefwin_geometry.cpp")
    assert source_rows[prefwin_index - 1] == b"src/prefwin.cpp"
    assert source_rows[prefwin_index + 1] == b"src/quaylewin.cpp"
    assert not _has_source_profile(cmake, b"src/prefwin_geometry.cpp"), (
        "PrefWin must retain its measured default application profile"
    )

    before = {row.address: row for row in read_text(
        base_geometry.decode("utf-8"), Path("src/prefwin_geometry.cpp"))
        if row.address in ADDRESSES}
    after = {row.address: row for row in read_text(
        geometry.decode("utf-8"), Path("src/prefwin_geometry.cpp"))
        if row.address in ADDRESSES}
    assert list(before) == list(ADDRESSES) and list(after) == list(ADDRESSES)
    for index, address in enumerate(ADDRESSES):
        old, new = before[address], after[address]
        expected = (replace(old, byte_exact=True, semantic=False)
                    if address in RECORDED_ADDRESSES else old)
        for field in PRESERVED_FIELDS:
            assert getattr(new, field) == getattr(expected, field), (hex(address), field)
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
        (1, lambda data: data.replace(
            b'"src/prefwin_geometry.cpp": {\n        "flags": ["/c", "/O2", "/Gy", "/GR-", "/GX", "/Ob2"]',
            b'"src/prefwin_geometry.cpp": {\n        "flags": ["/c", "/O2", "/Gy", "/GR-", "/Oy-", "/GX", "/Ob2"]', 1)),
        (1, lambda data: data.replace(
            b'"src/prefwin_geometry.cpp": {\n        "flags": ["/c", "/O2", "/Gy", "/GR-", "/GX", "/Ob2"]',
            b'"src/prefwin_geometry.cpp": {\n        "flags": ["/c", "/O2", "/Gy", "/GR-", "/GX", "/Ob1"]', 1)),
        (2, lambda data: data + b"\n// changed geometry interface\n"),
        (3, lambda data: data.replace(b"src/prefwin_geometry.cpp", b"src/prefwin_other.cpp", 1)),
        (3, lambda data: data + b'\nset_property(SOURCE src/prefwin_geometry.cpp APPEND PROPERTY COMPILE_OPTIONS "/Ob1")\n'),
        (3, lambda data: data.replace(b"add_executable(OpenSMACX", b'target_compile_options(OpenSMACX PRIVATE "/Ob1")\nadd_executable(OpenSMACX', 1)),
    ],
)
def test_prefwin_binding_inverse_rejects_unapproved_damages(path_index, mutate) -> None:
    sources = list(_actual())
    original = sources[path_index]
    sources[path_index] = mutate(original)
    assert sources[path_index] != original
    with pytest.raises(AssertionError):
        _assert_inverse(*sources)


@pytest.mark.parametrize("old, new", INITIALIZER_TRANSITIONS)
def test_prefwin_exact_initializer_inverse_rejects_reverted_order(old, new) -> None:
    geometry, project, header, cmake = _actual()
    assert geometry.count(new) == 1
    reverted = geometry.replace(new, old, 1)
    with pytest.raises(AssertionError):
        _assert_inverse(reverted, project, header, cmake)


@pytest.mark.parametrize("address", sorted(RECORDED_ADDRESSES))
def test_prefwin_recorded_claim_inverse_rejects_semantic_reversion(address) -> None:
    geometry, project, header, cmake = _actual()
    record = next(row for row in read_text(
        geometry.decode("utf-8"), Path("src/prefwin_geometry.cpp"))
        if row.address == address)
    assert record.byte_exact and not record.semantic
    old_claim = replace(record, byte_exact=False, semantic=True)
    reverted = write_bytes(geometry, [old_claim])
    with pytest.raises(AssertionError):
        _assert_inverse(reverted, project, header, cmake)


def test_prefwin_configuration_allows_unrelated_source_local_properties() -> None:
    sources = list(_actual())
    sources[3] += (
        b'\n# Independent later owner profile.\n'
        b'set_property(SOURCE src/unrelated_owner.cpp APPEND PROPERTY COMPILE_OPTIONS "/Ob1")\n'
        b'set_source_files_properties(src/another_owner.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")\n'
    )
    _assert_inverse(*sources)
