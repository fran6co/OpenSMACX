"""Exact transfer and inverse for the separately compiled string formatters."""
from __future__ import annotations

from collections import Counter
from pathlib import Path
import subprocess

import pytest

from match_recovery.source import read_text


ROOT = Path(__file__).resolve().parents[2]
BASE_REVISION = "f2f65f8f27a405b435b1c0d92159353a137177f3"
STRINGS_CPP = ROOT / "src/strings.cpp"
STRINGS_H = ROOT / "src/strings.h"
FORMATTERS_CPP = ROOT / "src/string_formatting.cpp"
FORMATTER_ADDRESSES = (
    "0x00500130",
    "0x00590510",
    "0x005905C0",
    "0x005906F0",
    "0x00590800",
    "0x00590B30",
    "0x00590910",
    "0x00590AA0",
    "0x00590A10",
)
FORMATTERS = (
    (
        b"// Append a caller-supplied string to the shared scratch buffer.\n",
        b"void __cdecl says(const char *text)",
    ),
    (
        b"// The five part-name formatters share the same label table, string table,\n",
        b"void __cdecl say_reactor(char *text, int reactor_id, int with_detail)",
    ),
    (
        b"// Superseded seed deleted; seed paths below are historical Git provenance.\n"
        b"// ORIGINAL: 0x005905C0",
        b"void __cdecl say_weapon(char *text, int weapon_id, int with_detail)",
    ),
    (
        b"// Superseded seed deleted; seed paths below are historical Git provenance.\n"
        b"// ORIGINAL: 0x005906F0",
        b"void __cdecl say_armor(char *text, int armor_id, int with_detail)",
    ),
    (
        b"// ORIGINAL: 0x00590800",
        b"void __cdecl say_chassis(char *text, int chassis_id, int with_detail)",
    ),
    (
        b"// ORIGINAL: 0x00590B30",
        b"void __cdecl say_techs(char *text, int tech_id, int with_detail)",
    ),
    (
        b"/*\nPurpose: Append a facility's name",
        b"void __cdecl say_facility(char *text, int facility_id, int with_cost)",
    ),
    (
        b"/*\nPurpose: Append an ability's name",
        b"void __cdecl say_ability(char *text, int ability_id, int with_detail)",
    ),
    (
        b"/*\nPurpose: Append a social category's name",
        b"void __cdecl say_society(char *text, int society_id, int with_model)",
    ),
)
RECORD_FIELDS = (
    "address", "mode", "state", "name", "image_spans", "symbol", "body",
    "context", "kind", "exclusion", "byte_exact", "semantic", "recipe",
    "levers", "ruled_out", "unrecoverable", "deferred", "source_role",
    "source_role_reason", "function_fragment",
)

PROFILE_BLOCK = (
    b"# Application formatters retain calls into the separately compiled string table.\n"
    b"# The Strings implementation keeps its independently measured default recipe.\n"
    b'set_source_files_properties(src/string_formatting.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")\n'
)
PROFILE_ANCHOR = (
    b"# Preserve measured frame-based accesses across the application owner batch.\n"
)
SOURCE_ROW = b"    src/strings.cpp\n"
FORMATTER_ROW = b"    src/string_formatting.cpp\n"


def _git_source(relative_path: str) -> bytes:
    return subprocess.run(
        ["git", "show", f"{BASE_REVISION}:{relative_path}"],
        cwd=ROOT,
        check=True,
        stdout=subprocess.PIPE,
    ).stdout


def _function_block(source: bytes, anchor: bytes, signature: bytes) -> tuple[bytes, int, int]:
    assert source.count(anchor) == 1, f"expected one moved-block anchor {anchor!r}"
    start = source.index(anchor)
    assert source.count(signature) == 1, f"expected one definition {signature!r}"
    signature_at = source.index(signature, start)
    open_brace = source.index(b"{", signature_at)

    depth = 0
    state = "code"
    index = open_brace
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else 0
        if state == "code":
            if char == 47 and nxt == 42:  # /*
                state = "block"
                index += 2
                continue
            if char == 47 and nxt == 47:  # //
                state = "line"
                index += 2
                continue
            if char == 34:
                state = "string"
            elif char == 39:
                state = "char"
            elif char == 123:
                depth += 1
            elif char == 125:
                depth -= 1
                if depth == 0:
                    end = index + 1
                    return source[start:end], start, end
        elif state == "block":
            if char == 42 and nxt == 47:
                state = "code"
                index += 2
                continue
        elif state == "line":
            if char == 10:
                state = "code"
        else:
            if char == 92:
                index += 2
                continue
            if (state == "string" and char == 34) or (state == "char" and char == 39):
                state = "code"
        index += 1
    raise AssertionError(f"unclosed moved body: {signature!r}")


def _formatter_blocks(source: bytes) -> tuple[list[bytes], list[tuple[int, int]]]:
    blocks, spans = [], []
    for anchor, signature in FORMATTERS:
        block, start, end = _function_block(source, anchor, signature)
        blocks.append(block)
        spans.append((start, end))
    assert [block.split(b"// ORIGINAL: ", 1)[1].split(b" ", 1)[0]
            for block in blocks] == [address.encode() for address in FORMATTER_ADDRESSES]
    return blocks, spans


def _remove_spans(source: bytes, spans: list[tuple[int, int]]) -> bytes:
    projected = source
    for start, end in reversed(spans):
        # Each moved definition carries only its existing following separator.
        # All retained text, including declarations and includes, stays literal.
        separator = source[end:end + 2]
        assert separator == b"\n\n" or (separator == b"\n" and end + 1 == len(source))
        projected = projected[:start] + projected[end + len(separator):]
    return projected.rstrip(b"\n") + b"\n"


def _license(source: bytes) -> bytes:
    end_marker = b" */\n"
    assert source.startswith(b"/*\n * OpenSMACX - an open source clone")
    end = source.index(end_marker) + len(end_marker)
    return source[:end]


def _formatter_prologue(base_strings: bytes) -> bytes:
    return (
        _license(base_strings)
        + b'#include "stdafx.h"\n'
        + b'#include "strings.h"\n'
        + b'#include "alpha.h"\n'
        + b'#include "technology.h"\n'
        + b'#include "veh.h"\n'
        + b'#include "base.h"\n'
        + b'#include "faction.h"\n\n'
        + b"// Keep these application formatters separate from Strings::get's definition so\n"
        + b"// VC6's same-TU /Ob2 inliner cannot absorb the original get calls. Their shared\n"
        + b"// storage and Strings implementation remain owned by strings.cpp.\n\n"
    )


def _expected_cmake(base_cmake: bytes) -> bytes:
    assert base_cmake.count(SOURCE_ROW) == 1
    assert base_cmake.count(PROFILE_ANCHOR) == 1
    with_source = base_cmake.replace(SOURCE_ROW, SOURCE_ROW + FORMATTER_ROW, 1)
    return with_source.replace(PROFILE_ANCHOR, PROFILE_BLOCK + PROFILE_ANCHOR, 1)


def _inverse(strings_cpp: bytes, strings_h: bytes, formatters_cpp: bytes, cmake: bytes) -> None:
    base_strings = _git_source("src/strings.cpp")
    base_header = _git_source("src/strings.h")
    base_cmake = _git_source("CMakeLists.txt")
    expected_blocks, spans = _formatter_blocks(base_strings)

    assert strings_h == base_header, "unadmitted strings.h change"
    assert strings_cpp == _remove_spans(base_strings, spans), "unadmitted strings.cpp content/scope change"
    assert formatters_cpp == (
        _formatter_prologue(base_strings)
        + b"\n\n".join(expected_blocks)
        + b"\n"
    ), "formatter module differs from exact transferred blocks/prologue/boundaries"
    assert cmake == _expected_cmake(base_cmake), "unadmitted CMake source/profile/link change"

    before = read_text(base_strings.decode("utf-8"), STRINGS_CPP)
    after = read_text(strings_cpp.decode("utf-8"), STRINGS_CPP)
    after += read_text(formatters_cpp.decode("utf-8"), FORMATTERS_CPP)
    before_counts = Counter(record.address for record in before)
    after_counts = Counter(record.address for record in after)
    assert before_counts == after_counts, "moved/retained annotation identity multiplicity changed"
    before_rows = {record.address: record for record in before}
    after_rows = {record.address: record for record in after}
    for address_text in FORMATTER_ADDRESSES:
        address = int(address_text, 16)
        assert address in before_rows and address in after_rows
        old, new = before_rows[address], after_rows[address]
        for field in RECORD_FIELDS:
            assert getattr(new, field) == getattr(old, field), (address_text, field)


def _actual_sources() -> tuple[bytes, bytes, bytes, bytes]:
    return (
        STRINGS_CPP.read_bytes(),
        STRINGS_H.read_bytes(),
        FORMATTERS_CPP.read_bytes(),
        (ROOT / "CMakeLists.txt").read_bytes(),
    )


def test_formatter_owner_is_an_exact_scoped_transfer_from_immutable_base() -> None:
    _inverse(*_actual_sources())


@pytest.mark.parametrize(
    "which,mutate",
    [
        (2, lambda data: data.replace(b"// ORIGINAL: 0x00500130", b"// ORIGINAL: 0x00500131", 1)),
        (2, lambda data: data.replace(b"// RECOVERY_BINDING: v1 {", b"// binding removed {", 1)),
        (2, lambda data: data.replace(b"void __cdecl says(", b"void __cdecl Strings::says(", 1)),
        (2, lambda data: data.replace(b"StringTemp, text", b"StringTemp, \"\"", 1)),
        (2, lambda data: data + b"\n// unadmitted formatter suffix\n"),
        (2, lambda data: b"// unadmitted formatter prefix\n" + data),
        (0, lambda data: data.replace(b"char StringTemp[1032];", b"char StringTemp[1033];", 1)),
        (0, lambda data: data.replace(b"char StringTemp[1032];", b"char StringTemp[1032] = {0};", 1)),
        (2, lambda data: data.replace(b"// Keep these application formatters", b"// RECOVERY_OBJECT: v1 0x009B86A0 ?StringTemp@@3PADA\n// Keep these application formatters", 1)),
        (3, lambda data: data.replace(b"src/string_formatting.cpp\n", b"", 1)),
        (3, lambda data: data.replace(b"set_source_files_properties(src/string_formatting.cpp", b"set_source_files_properties(src/strings.cpp", 1)),
        (3, lambda data: data.replace(b"add_executable(OpenSMACX", b"add_compile_options(\"/Oy-\")\nadd_executable(OpenSMACX", 1)),
    ],
)
def test_formatter_inverse_rejects_payload_storage_and_profile_damage(which, mutate) -> None:
    sources = list(_actual_sources())
    before = sources[which]
    sources[which] = mutate(before)
    assert sources[which] != before
    with pytest.raises(AssertionError):
        _inverse(*sources)


def test_formatter_inverse_rejects_duplicate_block_and_header_changes() -> None:
    sources = list(_actual_sources())
    first, = [block for block, (anchor, _) in zip(_formatter_blocks(_git_source("src/strings.cpp"))[0], FORMATTERS)
              if anchor.startswith(b"// Append")]
    sources[2] += b"\n\n" + first + b"\n"
    with pytest.raises(AssertionError):
        _inverse(*sources)


def test_formatter_inverse_rejects_moving_shared_storage_into_formatter_tu() -> None:
    sources = list(_actual_sources())
    storage = b"char StringTemp[1032];   // 0x009B86A0\n"
    assert sources[0].count(storage) == 1
    sources[0] = sources[0].replace(storage, b"", 1)
    sources[2] = sources[2].replace(b"// Keep these application formatters", storage + b"// Keep these application formatters", 1)
    with pytest.raises(AssertionError):
        _inverse(*sources)

    sources = list(_actual_sources())
    sources[1] += b"\n// unadmitted header change\n"
    with pytest.raises(AssertionError):
        _inverse(*sources)
