"""Exact source/annotation continuity for Council and undo ownership.

Only the admitted moves and precise obsolete-comment cleanups are inverted;
this is not original-TU, native compiler, linked retention or runtime proof.
"""
from __future__ import annotations

from pathlib import Path
import subprocess

import pytest

from match_recovery.source import read_text


ROOT = Path(__file__).resolve().parents[2]
BASE_REVISION = "b24ef250e686245f6075d2593b1a0d159eda1827"
SCALE_OWNER_BASE_REVISION = "3ca9f8b067603d1e009932bc071c1ffa52185c3a"
LIVE_PATHS = {
    "general": ROOT / "src/general.cpp",
    "general_h": ROOT / "src/general.h",
    "council": ROOT / "src/council.cpp",
    "undo": ROOT / "src/scenario_undo.cpp",
    "scale_adjustment_cpp": ROOT / "src/scale_adjustment.cpp",
    "scale_adjustment_h": ROOT / "src/scale_adjustment.h",
}

STATE = b"uint32_t ScenEditorUndoPosition = 1; // 0x00690D7C\n"
UNDO_START = b"/*\nPurpose: Load a Scenario Editor undo "
UNDO_END = b"/*\nPurpose: Read the specified header from a file."
CLEAR_ALL_START = b"// ORIGINAL: 0x004243F0 "
CLEAR_ALL_END = b"// ORIGINAL: 0x005D2E00 "
CLEAR_ONE_START = b"// ORIGINAL: 0x004243D0 "
CLEAR_ONE_END = b"// ORIGINAL: 0x00589550 "
STATE_ANCHOR = b"// The default noun forms parse_set/parse_say/parse_says/get_noun fall back to,"
UNDO_INCLUDE_PREFIX = (
    b'#include "stdafx.h"\n'
    b'#include "general.h"\n'
    b'#include "game.h"\n'
    b'#include "alpha.h"\n'
    b'#include "strings.h"\n'
    b"#include <cstring>\n"
    b"#include <cstdlib>\n\n"
)

# Two narrowly authorized historical-comment migrations in the moved Council
# records. Inverse projection restores only these exact prose blocks; the
# implementation, ORIGINAL line, metadata, and retained compiler lessons are
# still taken from and checked in the actual owner sources.
CLEAR_ONE_CONTEXT_CURRENT = (
    b"//\n"
    b"// Historical byte-match context: this overload was first recorded before it\n"
    b"// had a compiled source owner. Its implementation and BYTE_EXACT record are\n"
    b"// now retained here in the Council owner.\n"
)
CLEAR_ONE_CONTEXT_HISTORICAL = (
    b"//\n"
    b"// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in\n"
    b"// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,\n"
    b"// and rewriting it in the tree's own style is a later phase. See README.md\n"
    b"// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.\n"
)
CLEAR_ALL_CONTEXT_CURRENT = (
    b"// Historical context: the source-form observations below came from earlier\n"
    b"// compiler-matching work on this preserved unit. They describe useful\n"
    b"// compiler-sensitive forms, not instructions to regenerate a translation\n"
    b"// unit or verification artifact.\n"
)
CLEAR_ALL_CONTEXT_HISTORICAL = (
    b"// Kept for COVERAGE. This directory IS on the ratchet: every file here\n"
    b"// carries an ORIGINAL marker, `decomp_status.py` compiles and measures\n"
    b"// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a\n"
    b"// quarter of the project's total. It is still in no build; the earlier\n"
    b"// header said \"on no ratchet\", which stopped being true when the map\n"
    b"// moved into src/ and was still being written into new files.\n"
    b"//\n"
    b"// address        0x004243F0\n"
    b"// measured tier  BYTE_EXACT\n"
    b"//\n"
    b"// The WHOLE unit as measured, scaffolding included: for the units\n"
    b"// that are byte-exact yet refuse extraction, the agent tuned the\n"
    b"// emitted scaffolding and the body alone will not reproduce the\n"
    b"// verdict. To resume, copy everything below back over\n"
    b"//   build/byte-match/004243f0/unit.cpp\n"
    b"// and score it with tools/agent_brief.py.\n"
    b"// GENERATED SKELETON - tools/emit_translation_unit.py\n"
    b"// subject: ?clear_council_notify@@YAXXZ  at 0x004243F0  (29 bytes)\n"
    b"//\n"
    b"// A VERIFICATION ARTIFACT, not product source: classes are opaque and\n"
    b"// globals are bound to fixed addresses, because both are byte-visible\n"
    b"// and both differ from the style src/ is written in.\n"
)


def _restore_admitted_council_context(data: bytes, *, require_both: bool = True) -> bytes:
    """Invert only the two reviewed obsolete-prose cleanups, exactly once."""
    one_count = data.count(CLEAR_ONE_CONTEXT_CURRENT)
    all_count = data.count(CLEAR_ALL_CONTEXT_CURRENT)
    assert one_count <= 1 and all_count <= 1
    if require_both:
        assert one_count == 1 and all_count == 1
    assert data.count(CLEAR_ONE_CONTEXT_HISTORICAL) == 0
    assert data.count(CLEAR_ALL_CONTEXT_HISTORICAL) == 0
    data = data.replace(CLEAR_ONE_CONTEXT_CURRENT, CLEAR_ONE_CONTEXT_HISTORICAL, 1)
    data = data.replace(CLEAR_ALL_CONTEXT_CURRENT, CLEAR_ALL_CONTEXT_HISTORICAL, 1)
    return data

# These are all source-record facts except physical location/region. Parser
# lesson offsets are checked separately while retaining their exact text.
CONTINUITY_FIELDS = (
    "address", "mode", "state", "name", "image_spans", "symbol", "body",
    "context", "kind", "exclusion", "byte_exact", "semantic", "recipe",
    "levers", "ruled_out", "unrecoverable", "deferred", "source_role",
    "source_role_reason", "function_fragment",
)


def _unique_section(data: bytes, start: bytes, end: bytes) -> bytes:
    assert data.count(start) == 1, f"expected one section start {start!r}"
    assert data.count(end) == 1, f"expected one section end {end!r}"
    left = data.index(start)
    right = data.index(end)
    assert left < right
    return data[left:right]


def _unique_insert(data: bytes, anchor: bytes, value: bytes) -> bytes:
    assert data.count(anchor) == 1, f"expected one inverse anchor {anchor!r}"
    return data.replace(anchor, value + anchor, 1)


SCALE_START = b"// Native592D70/592D90 prove four-byte accesses only."
SCALE_END = b"\n/*\n// ORIGINAL: 0x0063BAC0 "
SCALE_INSERT_ANCHOR = b"/*\n// ORIGINAL: 0x0063BAC0 "
SCALE_CPP_PROLOGUE = (
    b'#include "stdafx.h"\n'
    b'#include "scale_adjustment.h"\n\n'
    b"#include <cstring>\n\n"
)
SCALE_DECLARATIONS = (
    b"int __cdecl set_scale_adjustment(int value);\n"
    b"int __cdecl apply_scale_adjustment(int value);\n"
)
SCALE_HEADER = (
    b"#ifndef OPENSMACX_SCALE_ADJUSTMENT_H\n"
    b"#define OPENSMACX_SCALE_ADJUSTMENT_H\n\n"
    + SCALE_DECLARATIONS
    + b"\n#endif\n"
)


def _scale_frozen_sources() -> tuple[bytes, bytes, bytes]:
    general = _git_source_at(SCALE_OWNER_BASE_REVISION, "src/general.cpp")
    header = _git_source_at(SCALE_OWNER_BASE_REVISION, "src/general.h")
    assert header.count(SCALE_DECLARATIONS) == 1
    assert general.count(SCALE_START) == 1
    payload = _unique_section(general, SCALE_START, SCALE_END)
    assert payload.endswith(b"}\n")
    return general, header, payload


def project_scale_adjustment_general(
    general_bytes: bytes,
    scale_cpp: bytes | None,
    scale_h: bytes | None,
    general_h: bytes,
) -> bytes:
    """Restore only the checked scale-owner block, using actual moved bytes."""
    frozen_general, frozen_header, expected_payload = _scale_frozen_sources()
    assert frozen_general.count(SCALE_INSERT_ANCHOR) == 1
    if scale_cpp is None and scale_h is None:
        # A checkout that has not performed the move is valid only while the
        # complete accepted block and its original declarations remain in General.
        assert general_bytes.count(SCALE_START) == 1
        assert _unique_section(general_bytes, SCALE_START, SCALE_END) == expected_payload
        assert general_h == frozen_header
        return general_bytes

    assert scale_cpp is not None and scale_h is not None, "partial scale-owner move"
    assert scale_cpp == SCALE_CPP_PROLOGUE + expected_payload, "changed scale owner payload or boundary"
    assert scale_h == SCALE_HEADER, "changed scale owner API/header"
    expected_general_header = frozen_header.replace(
        SCALE_DECLARATIONS, b'#include "scale_adjustment.h"\n', 1
    )
    assert general_h == expected_general_header, "unadmitted General header change"
    assert general_bytes.count(SCALE_START) == 0, "scale payload remains in General"
    assert general_bytes.count(b"// RECOVERY_OBJECT: v1 0x00945AEC ?StoredScaleAdjustment@@3HA") == 0
    assert general_bytes.count(b"0x00592D70") == 0
    assert general_bytes.count(b"0x00592D90") == 0
    actual_payload = scale_cpp[len(SCALE_CPP_PROLOGUE):]
    assert actual_payload == expected_payload
    return _unique_insert(general_bytes, SCALE_INSERT_ANCHOR, actual_payload + b"\n")


HEADER_RNG_OLD_STATUS = (
    b"Status: Complete with built in version of srand(). Revisit once more code is redirected to dll.\n"
)
HEADER_RNG_CURRENT_STATUS = b"Status: Complete\n"
HEADER_RNG_OLD_DECLARATIONS = (
    b"void __cdecl my_srand(int reseed);\n",
    b"uint32_t __cdecl rnd(int bounds, LPSTR UNUSED(input) input);\n",
    b"void __cdecl header_check(LPSTR header, FILE *file);\n"
    b"void __cdecl header_write(LPCSTR header, FILE *file);\n",
)
HEADER_RNG_API_DECLARATIONS = (
    b"void __cdecl header_check(char *header, FILE *file);",
    b"void __cdecl header_write(const char *header, FILE *file);",
    b"void __cdecl my_srand(int reseed);",
    b"unsigned int __cdecl rnd(int bounds, char *input);",
)


def _header_rng_license() -> bytes:
    return (
        b"/*\n"
        b" * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.\n"
        b" * Copyright (C) 2013-2021 Brendan Casey\n"
        b" *\n"
        b" * OpenSMACX is free software: you can redistribute it and / or modify\n"
        b" * it under the terms of the GNU General Public License as published by\n"
        b" * the Free Software Foundation, either version 3 of the License, or\n"
        b" * (at your option) any later version.\n"
        b" *\n"
        b" * OpenSMACX is distributed in the hope that it will be useful,\n"
        b" * but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
        b" * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
        b" * GNU General Public License for more details.\n"
        b" *\n"
        b" * You should have received a copy of the GNU General Public License\n"
        b" * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.\n"
        b" */\n"
    )


def _header_rng_record(data: bytes, address: str) -> bytes:
    marker = f"// ORIGINAL: {address} ".encode()
    assert data.count(marker) == 1, f"expected one owner record {address}"
    marker_at = data.index(marker)
    start = data.rfind(b"/*\nPurpose:", 0, marker_at)
    assert start >= 0
    next_start = data.find(b"\n/*\nPurpose:", marker_at)
    if next_start >= 0:
        end = next_start + 1
    else:
        end = len(data)
    return data[start:end]


def _validate_header_rng_sources(
    header_io_cpp: bytes,
    header_io_h: bytes,
    crt_random_cpp: bytes,
    crt_random_h: bytes,
) -> dict[str, bytes]:
    # Closed owner prologues and exactly the two authorized definitions in
    # each source file. The canonical comments are validated as APIs too.
    header_io_prologue = _header_rng_license() + b'#include "stdafx.h"\n#include "header_io.h"\n\n'
    random_prologue = (
        _header_rng_license()
        + b'#include "stdafx.h"\n#include "crt_random.h"\n#include "log.h"\n\n'
    )
    assert header_io_cpp.startswith(header_io_prologue)
    assert crt_random_cpp.startswith(random_prologue)
    header_check = _header_rng_record(header_io_cpp, "0x0057D1F0")
    header_write = _header_rng_record(header_io_cpp, "0x0057D240")
    my_srand = _header_rng_record(crt_random_cpp, "0x00538FB0")
    rnd = _header_rng_record(crt_random_cpp, "0x00579770")
    assert header_io_cpp == header_io_prologue + header_check + header_write
    assert crt_random_cpp == random_prologue + my_srand + rnd

    header_io_expected = (
        _header_rng_license()
        + b"#pragma once\n#include <stdio.h>\n\n"
        + b"// Shared game/map stream framing. The caller retains ownership of the FILE.\n"
        + b"// Headers include their terminating NUL and are followed by a 0x1A byte.\n"
        + HEADER_RNG_API_DECLARATIONS[0] + b"\n"
        + HEADER_RNG_API_DECLARATIONS[1] + b"\n"
    )
    crt_random_expected = (
        _header_rng_license()
        + b"#pragma once\n\n"
        + b"// These wrappers use the CRT stream, not the separate Random/Rand LCG.\n"
        + HEADER_RNG_API_DECLARATIONS[2] + b"\n"
        + HEADER_RNG_API_DECLARATIONS[3] + b"\n"
    )
    assert header_io_h == header_io_expected, "header I/O API/prologue is not closed"
    assert crt_random_h == crt_random_expected, "CRT RNG API/prologue is not closed"
    assert my_srand.count(HEADER_RNG_OLD_STATUS) == 0
    assert my_srand.count(HEADER_RNG_CURRENT_STATUS) == 1

    # The one reviewed stale status sentence is a comment-only migration. Its
    # exact old bytes are reinstated for the historical source projection.
    restored_my_srand = my_srand.replace(
        HEADER_RNG_CURRENT_STATUS, HEADER_RNG_OLD_STATUS, 1
    )
    assert restored_my_srand.count(HEADER_RNG_OLD_STATUS) == 1
    return {
        "my_srand": restored_my_srand,
        "rnd": rnd,
        "header_check": header_check,
        "header_write": header_write,
    }


FIXED_COS_PRIOR = (
    b"int __cdecl cos(int a1, int a2) {\n"
    b"    uint32_t wrapped = static_cast<uint32_t>(a1) + 0x3fffffffU;\n"
    b"    int32_t biased_angle;\n"
    b"    std::memcpy(&biased_angle, &wrapped, sizeof(biased_angle));\n"
    b"    return sin(biased_angle, a2);\n"
    b"}\n"
)
FIXED_COS_CURRENT = (
    b"int __cdecl cos(int a1, int a2) {\n"
    b"    return sin(static_cast<int>(static_cast<uint32_t>(a1) + 0x3fffffffU), a2);\n"
    b"}\n"
)

# These three source-owned associated-data records are an authorized addition
# to the pilot's live General source. Inverse projection removes only these
# exact comments at their exact source seams; all original function records,
# bodies and historical baselines remain checked by the existing projection.
ASSOCIATED_PREFIX = b"// RECOVERY_ASSOCIATED:"
STOI_PURPOSE = b"/*\nPurpose: Converts a binary, hex or decimal string to an integer.\n"
STOI_TARGET_ASSOCIATION = b"// RECOVERY_ASSOCIATED: v1 switch_targets 0x00628950 0x00628A00 0x00628A10 0x00628976\n"
STOI_SELECTOR_ASSOCIATION = b"// RECOVERY_ASSOCIATED: v1 switch_selector 0x00628950 0x00628A10 0x00628A47 0x00628970\n"
STOI_ASSOCIATIONS = STOI_TARGET_ASSOCIATION + STOI_SELECTOR_ASSOCIATION
HSV_ORIGINAL = b"// ORIGINAL: 0x00628B70 ?HSV_to_RGB@@YAXPAUHSV@@PAUPALETTEENTRY@@@Z"
HSV_ASSOCIATION = b"// RECOVERY_ASSOCIATED: v1 switch_targets 0x00628B70 0x00628D94 0x00628DAC 0x00628C65\n"


def _strip_switch_associations(general_bytes: bytes) -> bytes:
    """Invert only the three approved records at their unique live seams."""
    assert general_bytes.count(ASSOCIATED_PREFIX) == 3
    assert general_bytes.count(STOI_ASSOCIATIONS) == 1
    assert general_bytes.count(HSV_ASSOCIATION) == 1
    assert general_bytes.count(STOI_ASSOCIATIONS + STOI_PURPOSE) == 1
    assert general_bytes.count(HSV_ASSOCIATION + HSV_ORIGINAL) == 1
    projected = general_bytes.replace(STOI_ASSOCIATIONS, b"", 1)
    projected = projected.replace(HSV_ASSOCIATION, b"", 1)
    assert ASSOCIATED_PREFIX not in projected
    return projected


def _restore_admitted_fixed_cos(general_bytes: bytes) -> bytes:
    """Undo only the reviewed whole body; never replace an arbitrary function."""
    assert general_bytes.count(b"int __cdecl cos(int a1, int a2) {") == 1
    old_count = general_bytes.count(FIXED_COS_PRIOR)
    new_count = general_bytes.count(FIXED_COS_CURRENT)
    assert (old_count, new_count) in ((1, 0), (0, 1)), "unadmitted fixed cosine body"
    return general_bytes.replace(FIXED_COS_CURRENT, FIXED_COS_PRIOR, 1)


def _project_header_rng_general(
    general_bytes: bytes,
    header_io_cpp: bytes,
    header_io_h: bytes,
    crt_random_cpp: bytes,
    crt_random_h: bytes,
) -> bytes:
    general_bytes = _strip_switch_associations(general_bytes)
    general_bytes = _restore_admitted_fixed_cos(general_bytes)
    records = _validate_header_rng_sources(
        header_io_cpp, header_io_h, crt_random_cpp, crt_random_h
    )
    general_bytes = _unique_insert(
        general_bytes,
        b"// Current structural interpretation supersedes the seed-history binding's\n",
        records["my_srand"],
    )
    general_bytes = _unique_insert(
        general_bytes,
        b"/*\nPurpose: Create a debug error pop-up then write all the parameters to the log file.\n",
        records["rnd"],
    )
    general_bytes = _unique_insert(
        general_bytes,
        b"/*\nPurpose: For the count, sort both id and value arrays by the least to greatest value (ascending).\n",
        records["header_check"] + records["header_write"],
    )
    return general_bytes


def project_header_rng_general(general_bytes: bytes) -> bytes:
    """Inverse the header/RNG move using only its actual source owners/APIs."""
    live = {
        "header_io_cpp": ROOT / "src/header_io.cpp",
        "header_io_h": ROOT / "src/header_io.h",
        "crt_random_cpp": ROOT / "src/crt_random.cpp",
        "crt_random_h": ROOT / "src/crt_random.h",
    }
    sources = {key: path.read_bytes() for key, path in live.items()}
    return _project_header_rng_general(
        general_bytes,
        sources["header_io_cpp"],
        sources["header_io_h"],
        sources["crt_random_cpp"],
        sources["crt_random_h"],
    )


def _git_source(relative_path: str) -> bytes:
    return _git_source_at(BASE_REVISION, relative_path)


def _git_source_at(revision: str, relative_path: str) -> bytes:
    return subprocess.run(
        ["git", "show", f"{revision}:{relative_path}"],
        cwd=ROOT,
        check=True,
        stdout=subprocess.PIPE,
    ).stdout


def _baseline_sources() -> dict[str, bytes]:
    return {
        "general": _git_source("src/general.cpp"),
        "council": _git_source("src/council.cpp"),
    }


def _actual_split(paths: dict[str, Path]) -> dict[str, bytes]:
    optional = {"scale_adjustment_cpp", "scale_adjustment_h"}
    return {
        key: path.read_bytes() if path.is_file() else None
        for key, path in paths.items()
        if path.is_file() or key in optional
    }


def inverse_split(parts: dict[str, bytes]) -> dict[str, bytes]:
    """Rebuild original general/council bytes from actual split contents."""
    assert set(parts) == {
        "general", "general_h", "council", "undo",
        "scale_adjustment_cpp", "scale_adjustment_h",
    }
    general = project_scale_adjustment_general(
        parts["general"], parts["scale_adjustment_cpp"],
        parts["scale_adjustment_h"], parts["general_h"],
    )
    general = project_header_rng_general(general)
    council = parts["council"]
    undo_owner = parts["undo"]
    baseline = _baseline_sources()

    # Recover each payload from its new owner, never from the baseline.
    clear_one_actual = _unique_section(council, CLEAR_ONE_START, CLEAR_ALL_START)
    assert council.count(CLEAR_ALL_START) == 1
    clear_all_actual = council[council.index(CLEAR_ALL_START):]
    # The current Council blocks are the source of the restored bytes. Only
    # the exact admitted prose cleanups are projected back to their former
    # historical comments for comparison with the immutable source.
    clear_one = _restore_admitted_council_context(clear_one_actual, require_both=False)
    clear_all = _restore_admitted_council_context(clear_all_actual, require_both=False)
    # The terminal moved record has no following record in its new owner.
    # Its exact terminal bytes are bounded by its own immutable source span,
    # while still being taken from the actual Council suffix for projection.
    assert clear_all == _unique_section(
        baseline["general"], CLEAR_ALL_START, CLEAR_ALL_END
    )
    assert STATE.count(b"\n") == 1
    assert undo_owner.count(STATE) == 1
    assert general.count(STATE) == 0
    assert general.count(CLEAR_ALL_START) == 0
    assert general.count(CLEAR_ONE_START) == 0
    assert general.count(UNDO_START) == 0

    # The destination must be exactly the old Council file plus its admitted
    # separator and two moved records; this rejects extra suffixes and keeps
    # duplicate moved blocks in the retained core, where baseline comparison
    # below will reject them.
    council_suffix = b"\n" + clear_one_actual + clear_all_actual
    assert council.endswith(council_suffix), "unadmitted Council suffix or reordered payload"
    council_core = council[:-len(council_suffix)]

    # The new owner is intentionally closed: old General heading, the
    # authorized standard prologue, one moved state row, and the complete
    # moved function block, with no extra prefix/suffix or historical-TU
    # assertion.
    old_general = baseline["general"]
    include_at = old_general.index(b'#include "stdafx.h"')
    heading = old_general[:include_at]
    undo_prefix = heading + UNDO_INCLUDE_PREFIX + STATE + b"\n"
    assert undo_owner.startswith(undo_prefix), "unexpected undo-owner heading/prologue/state"
    undo_payload = undo_owner[len(undo_prefix):]
    assert undo_payload.startswith(UNDO_START)
    assert undo_payload.count(UNDO_START) == 1
    assert undo_payload.endswith(b"\n")
    assert b"ORIGINAL TU" not in undo_owner and b"original translation unit" not in undo_owner.lower()

    # Reinsert payloads at unique old-file seams. These anchors constrain
    # placement while the inserted bytes still come from the actual owners.
    restored_general = general
    restored_general = _unique_insert(restored_general, STATE_ANCHOR, STATE)
    restored_general = _unique_insert(restored_general, CLEAR_ALL_END, clear_all)
    restored_general = _unique_insert(restored_general, CLEAR_ONE_END, clear_one)
    restored_general = _unique_insert(restored_general, UNDO_END, undo_payload)

    # Refuse altered, omitted, duplicated, reordered, or unadmitted content in
    # either reconstructed file. The immutable source is an exact oracle, not
    # the source of the reconstructed bytes.
    assert restored_general == old_general
    assert council_core == baseline["council"]
    return {"general": restored_general, "council": council_core}


def _lesson_content(record) -> tuple | None:
    if record.origin is None:
        return None
    return tuple(
        (lesson.kind, lesson.key, lesson.prose,
         tuple(text for _, text in lesson.lines))
        for lesson in record.origin
    )


def _records(contents: dict[str, bytes], paths: dict[str, Path]) -> dict[int, object]:
    parsed = []
    for key, content in contents.items():
        virtual_path = paths[key]
        parsed.extend(read_text(content.decode("utf-8"), virtual_path))
    rows = {row.address: row for row in parsed}
    assert len(rows) == len(parsed), "duplicate native parsed source identity"
    return rows


def assert_parser_continuity(parts: dict[str, bytes], paths: dict[str, Path]) -> None:
    baseline = _baseline_sources()
    before = _records(
        {"general": baseline["general"], "council": baseline["council"]},
        {"general": ROOT / "src/general.cpp", "council": ROOT / "src/council.cpp"},
    )
    parser_parts = {key: parts[key] for key in ("general", "council", "undo")}
    parser_parts["general"] = project_scale_adjustment_general(
        parts["general"], parts["scale_adjustment_cpp"],
        parts["scale_adjustment_h"], parts["general_h"],
    )
    parser_parts["general"] = project_header_rng_general(parser_parts["general"])
    parser_parts["council"] = _restore_admitted_council_context(parts["council"])
    after = _records(parser_parts, paths)
    assert after.keys() == before.keys()
    for address in sorted(before):
        old, new = before[address], after[address]
        for name in CONTINUITY_FIELDS:
            assert getattr(new, name) == getattr(old, name), (hex(address), name)
        assert _lesson_content(new) == _lesson_content(old), (hex(address), "lesson history")


def _parts_from(paths: dict[str, Path]) -> dict[str, bytes]:
    parts = _actual_split(paths)
    projected = inverse_split(parts)
    assert_parser_continuity(parts, paths)
    return projected


def test_live_inverse_reconstructs_immutable_general_and_council():
    projected = _parts_from(LIVE_PATHS)
    assert projected == _baseline_sources()


def test_native_parser_owner_set_round_trips_without_unmapped_source_changes():
    paths = LIVE_PATHS
    projected = _parts_from(paths)
    assert projected == _baseline_sources()


def test_scale_owner_projection_is_exact_and_accepts_pre_move_source():
    frozen_general, frozen_header, payload = _scale_frozen_sources()
    moved_general = frozen_general.replace(payload + b"\n", b"", 1)
    moved_header = frozen_header.replace(
        SCALE_DECLARATIONS, b'#include "scale_adjustment.h"\n', 1
    )
    assert project_scale_adjustment_general(
        moved_general, SCALE_CPP_PROLOGUE + payload, SCALE_HEADER, moved_header
    ) == frozen_general
    assert project_scale_adjustment_general(
        frozen_general, None, None, frozen_header
    ) == frozen_general


@pytest.mark.parametrize(
    "target,mutate",
    [
        ("scale_adjustment_cpp", lambda b: b.replace(b"StoredScaleAdjustment = value;", b"StoredScaleAdjustment = 1;", 1)),
        ("scale_adjustment_cpp", lambda b: b.replace(b"int StoredScaleAdjustment = 0;", b"int StoredScaleAdjustment = 1;", 1)),
        ("scale_adjustment_cpp", lambda b: b"// extra prefix\n" + b),
        ("scale_adjustment_cpp", lambda b: b + b"\n// extra suffix\n"),
        ("scale_adjustment_h", lambda b: b.replace(b"apply_scale_adjustment", b"alter_scale_adjustment", 1)),
        ("scale_adjustment_h", lambda b: b + b"\n// extra header bytes\n"),
        ("general_h", lambda b: b + b"\n// unadmitted General header change\n"),
        ("general_h", lambda b: b.replace(b'#include "scale_adjustment.h"', b'#include "changed_scale_header.h"', 1)),
        ("general_h", lambda b: b.replace(b'#include "scale_adjustment.h"\n', b"", 1)),
    ],
)
def test_scale_owner_projection_rejects_payload_object_and_boundary_damage(target, mutate):
    frozen_general, frozen_header, payload = _scale_frozen_sources()
    moved_general = frozen_general.replace(payload + b"\n", b"", 1)
    moved_header = frozen_header.replace(
        SCALE_DECLARATIONS, b'#include "scale_adjustment.h"\n', 1
    )
    sources = {
        "scale_adjustment_cpp": SCALE_CPP_PROLOGUE + payload,
        "scale_adjustment_h": SCALE_HEADER,
        "general_h": moved_header,
    }
    before = sources[target]
    sources[target] = mutate(before)
    assert sources[target] != before
    with pytest.raises(AssertionError):
        project_scale_adjustment_general(
            moved_general, sources["scale_adjustment_cpp"],
            sources["scale_adjustment_h"], sources["general_h"],
        )


@pytest.mark.parametrize(
    "target,mutate",
    [
        ("council", lambda b: b.replace(CLEAR_ONE_START, b"// removed marker ", 1)),
        ("council", lambda b: b.replace(b"CouncilNotifyPluralSlots[a1] = 0;", b"CouncilNotifyPluralSlots[a1] = 1;", 1)),
        ("council", lambda b: b.replace(CLEAR_ONE_CONTEXT_CURRENT, CLEAR_ONE_CONTEXT_CURRENT.replace(b"Historical", b"Changed", 1), 1)),
        ("council", lambda b: b.replace(CLEAR_ONE_CONTEXT_CURRENT, CLEAR_ONE_CONTEXT_CURRENT + CLEAR_ONE_CONTEXT_CURRENT, 1)),
        ("council", lambda b: b.replace(CLEAR_ALL_CONTEXT_CURRENT, CLEAR_ALL_CONTEXT_CURRENT.replace(b"Historical", b"Changed", 1), 1)),
        ("council", lambda b: b.replace(b"0x004243D0-0x004243E8 BYTE_EXACT", b"0x004243D0-0x004243E8", 1)),
        ("council", lambda b: b + _unique_section(b, CLEAR_ONE_START, CLEAR_ALL_START)),
        ("council", lambda b: b + b"\n// unadmitted council suffix\n"),
        ("general", lambda b: b + b"\n// unadmitted appended source\n"),
        ("general", lambda b: b.replace(STATE_ANCHOR, b"// changed retained anchor\n" + STATE_ANCHOR, 1)),
        ("undo", lambda b: b.replace(STATE, b"", 1)),
        ("undo", lambda b: b.replace(STATE, STATE + STATE, 1)),
        ("undo", lambda b: b.replace(UNDO_START, b"/* modified marker\nPurpose: Load a Scenario Editor undo ", 1)),
        ("undo", lambda b: b.replace(b"load_daemon(StringTemp, false);", b"load_daemon(StringTemp, true);", 1)),
        ("undo", lambda b: b"// unadmitted prefix\n" + b),
        ("undo", lambda b: b + b"\n// unadmitted suffix\n"),
    ],
)
def test_inverse_rejects_payload_annotation_state_and_boundary_damage(target, mutate):
    parts = _actual_split(LIVE_PATHS)
    changed = mutate(parts[target])
    assert changed != parts[target]
    parts[target] = changed
    with pytest.raises(AssertionError):
        inverse_split(parts)


def test_inverse_rejects_duplicate_undo_body_and_extra_council_prefix():
    parts = _actual_split(LIVE_PATHS)
    undo_prefix_len = len(
        _baseline_sources()["general"][:_baseline_sources()["general"].index(b'#include "stdafx.h"')]
        + UNDO_INCLUDE_PREFIX + STATE + b"\n"
    )
    payload = parts["undo"][undo_prefix_len:]
    parts["undo"] += payload
    with pytest.raises(AssertionError):
        inverse_split(parts)

    parts = _actual_split(LIVE_PATHS)
    parts["council"] = b"// unadmitted prefix\n" + parts["council"]
    with pytest.raises(AssertionError):
        inverse_split(parts)


@pytest.mark.parametrize("damage", ["wrong-phase", "missing-body", "duplicate-body"])
def test_inverse_rejects_unadmitted_fixed_cos_changes(damage):
    parts = _actual_split(LIVE_PATHS)
    assert parts["general"].count(FIXED_COS_CURRENT) == 1
    changed = {
        "wrong-phase": FIXED_COS_CURRENT.replace(b"0x3fffffffU", b"0x40000000U"),
        "missing-body": b"",
        "duplicate-body": FIXED_COS_CURRENT + FIXED_COS_CURRENT,
    }[damage]
    parts["general"] = parts["general"].replace(FIXED_COS_CURRENT, changed, 1)
    with pytest.raises(AssertionError):
        inverse_split(parts)


@pytest.mark.parametrize("mutate", [
    lambda data: data.replace(STOI_ASSOCIATIONS, b"", 1)
                    .replace(HSV_ASSOCIATION, b"", 1),
    lambda data: data.replace(
        b"// RECOVERY_ASSOCIATED: v1 switch_targets 0x00628950 0x00628A00 0x00628A10 0x00628976\n",
        b"", 1),
    lambda data: data.replace(
        b"// RECOVERY_ASSOCIATED: v1 switch_selector 0x00628950 0x00628A10 0x00628A47 0x00628970\n",
        b"", 1),
    lambda data: data.replace(HSV_ASSOCIATION, b"", 1),
    lambda data: data.replace(STOI_ASSOCIATIONS,
                               STOI_ASSOCIATIONS + STOI_ASSOCIATIONS, 1),
    lambda data: data.replace(HSV_ASSOCIATION,
                               HSV_ASSOCIATION + HSV_ASSOCIATION, 1),
    lambda data: data.replace(
        STOI_TARGET_ASSOCIATION,
        STOI_TARGET_ASSOCIATION.replace(b"0x00628A00", b"0x00628A04", 1), 1),
    lambda data: data.replace(
        STOI_TARGET_ASSOCIATION,
        STOI_TARGET_ASSOCIATION.replace(b"0x00628950", b"0x00628951", 1), 1),
    lambda data: data.replace(
        STOI_SELECTOR_ASSOCIATION,
        STOI_SELECTOR_ASSOCIATION.replace(b"switch_selector", b"switch_targets", 1), 1),
    lambda data: data.replace(
        STOI_TARGET_ASSOCIATION,
        STOI_TARGET_ASSOCIATION.replace(b"0x00628976", b"0x00628975", 1), 1),
    lambda data: data.replace(
        HSV_ASSOCIATION,
        HSV_ASSOCIATION.replace(b"0x00628D94", b"0x00628D98", 1), 1),
    lambda data: data.replace(
        HSV_ASSOCIATION,
        HSV_ASSOCIATION.replace(b"0x00628B70", b"0x00628B71", 1), 1),
    lambda data: data.replace(
        HSV_ASSOCIATION,
        HSV_ASSOCIATION.replace(b"0x00628C65", b"0x00628C64", 1), 1),
])
def test_inverse_rejects_associated_record_population_and_interpretation_damage(mutate):
    parts = _actual_split(LIVE_PATHS)
    changed = mutate(parts["general"])
    assert changed != parts["general"]
    parts["general"] = changed
    with pytest.raises(AssertionError):
        inverse_split(parts)


@pytest.mark.parametrize("which", ["stoi", "hsv"])
def test_inverse_rejects_associated_record_at_wrong_source_seam(which):
    parts = _actual_split(LIVE_PATHS)
    marker = STOI_ASSOCIATIONS if which == "stoi" else HSV_ASSOCIATION
    anchor = STOI_PURPOSE if which == "stoi" else HSV_ORIGINAL
    general = parts["general"]
    assert general.count(marker + anchor) == 1
    moved = general.replace(marker + anchor, anchor, 1)
    moved = _unique_insert(moved, b'#include "stdafx.h"\n', marker)
    parts["general"] = moved
    with pytest.raises(AssertionError):
        inverse_split(parts)
