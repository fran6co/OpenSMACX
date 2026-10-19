"""Exact inverse and record-closure controls for the header/RNG owner split."""
from __future__ import annotations

from pathlib import Path
import runpy
import subprocess

import pytest

from match_recovery.source import read_text


ROOT = Path(__file__).resolve().parents[2]
BASE_REVISION = "fcaa712ffd8249ca958c94046fd32c645ae67742"
OLD_HELPER_PATH = ROOT / "recovery/tests/test_general_owner_split_oct17.py"
OLD_HELPER = runpy.run_path(str(OLD_HELPER_PATH))

PATHS = {
    "general_cpp": ROOT / "src/general.cpp",
    "general_h": ROOT / "src/general.h",
    "header_io_cpp": ROOT / "src/header_io.cpp",
    "header_io_h": ROOT / "src/header_io.h",
    "crt_random_cpp": ROOT / "src/crt_random.cpp",
    "crt_random_h": ROOT / "src/crt_random.h",
    "number_parsing_cpp": ROOT / "src/number_parsing.cpp",
    "scale_adjustment_cpp": ROOT / "src/scale_adjustment.cpp",
}

REMOVED_INCLUDES = (
    b'#include "header_io.h"\n',
    b'#include "crt_random.h"\n',
)


def _baseline(relative: str) -> bytes:
    return subprocess.run(
        ["git", "show", f"{BASE_REVISION}:src/{relative}"],
        cwd=ROOT,
        check=True,
        stdout=subprocess.PIPE,
    ).stdout


def _contents(paths: dict[str, Path] = PATHS) -> dict[str, bytes]:
    return {name: path.read_bytes() for name, path in paths.items()}


def _project_general_header(general_h: bytes, sources: dict[str, bytes]) -> bytes:
    records = OLD_HELPER["_validate_header_rng_sources"](
        sources["header_io_cpp"],
        sources["header_io_h"],
        sources["crt_random_cpp"],
        sources["crt_random_h"],
    )
    assert general_h.count(REMOVED_INCLUDES[0]) == 1
    assert general_h.count(REMOVED_INCLUDES[1]) == 1
    result = general_h.replace(REMOVED_INCLUDES[0], b"", 1)
    result = result.replace(REMOVED_INCLUDES[1], b"", 1)
    # The signed API correction63e961419 is independently controlled by its
    # actual-source contract tests. Only its exact declaration is inverted
    # here to compare this older immutable owner publication.
    signed = b"int __cdecl bit_count_signed(int bitfield);\n"
    old_signed = b"uint32_t __cdecl bit_count_signed(int bitfield);\n"
    assert result.count(signed) == 1 and result.count(old_signed) == 0
    result = result.replace(signed, old_signed, 1)

    # Restore the old API declarations only after validating the new public
    # declarations and their closed headers above. The declaration spellings
    # here are the exact pre-migration General.h forms, not inferred ABI names.
    my_srand, rnd, header_pair = OLD_HELPER["HEADER_RNG_OLD_DECLARATIONS"]
    result = OLD_HELPER["_unique_insert"](
        result,
        b"int __cdecl fixed_div(int numerator, int denominator);\n",
        my_srand,
    )
    result = OLD_HELPER["_unique_insert"](
        result,
        b"void __cdecl danger(LPCSTR msg1, LPCSTR msg2, int num1, int num2, int num3);\n",
        rnd,
    )
    result = OLD_HELPER["_unique_insert"](
        result,
        b"void __cdecl sort(int count, int *id, int *value);\n",
        header_pair,
    )
    assert result.count(b"header_io.h") == 0
    assert result.count(b"crt_random.h") == 0
    assert records["header_check"] and records["header_write"]
    return result


def _project_general(general_cpp: bytes, sources: dict[str, bytes]) -> bytes:
    general_cpp = OLD_HELPER["project_scale_adjustment_general"](
        general_cpp, sources["scale_adjustment_cpp"], None, sources["general_h"]
    )
    return OLD_HELPER["_project_header_rng_general"](
        general_cpp,
        sources["header_io_cpp"],
        sources["header_io_h"],
        sources["crt_random_cpp"],
        sources["crt_random_h"],
        number_parsing_cpp=sources["number_parsing_cpp"],
    )


def _assert_exact_inverse(sources: dict[str, bytes]) -> None:
    general_cpp = _project_general(sources["general_cpp"], sources)
    general_h = _project_general_header(sources["general_h"], sources)
    assert general_cpp == _baseline("general.cpp")
    assert general_h == _baseline("general.h")


def _records(contents: dict[str, bytes], virtual_paths: dict[str, Path]):
    parsed = []
    for key, content in contents.items():
        parsed.extend(read_text(content.decode("utf-8"), virtual_paths[key]))
    records = {record.address: record for record in parsed}
    assert len(records) == len(parsed), "duplicate parsed address across old/new owners"
    return records


def _lesson_content(record):
    if record.origin is None:
        return None
    return tuple(
        (lesson.kind, lesson.key, lesson.prose,
         tuple(text for _, text in lesson.lines))
        for lesson in record.origin
    )


def _assert_identity_closure(sources: dict[str, bytes]) -> None:
    old = _records(
        {"general_cpp": _baseline("general.cpp"), "general_h": _baseline("general.h")},
        {"general_cpp": ROOT / "src/general.cpp", "general_h": ROOT / "src/general.h"},
    )
    projected_sources = dict(sources)
    projected_sources["general_cpp"] = OLD_HELPER["_restore_admitted_signed_bit_count"](
        sources["general_cpp"]
    )
    current = _records(projected_sources, PATHS)
    assert len(old) == 89, f"expected the pinned 89-record General source closure, got {len(old)}"
    assert current.keys() == old.keys()
    for address, prior in old.items():
        present = current[address]
        for field in OLD_HELPER["CONTINUITY_FIELDS"]:
            assert getattr(present, field) == getattr(prior, field), (hex(address), field)
        assert _lesson_content(present) == _lesson_content(prior), (hex(address), "lesson history")


def test_actual_owner_blocks_and_public_apis_project_exactly_to_fcaa_general():
    sources = _contents()
    _assert_exact_inverse(sources)
    _assert_identity_closure(sources)


@pytest.mark.parametrize(
    "filename,mutate",
    [
        ("crt_random_cpp", lambda b: b.replace(b"srand(reseed);", b"srand(reseed + 1);", 1)),
        ("crt_random_cpp", lambda b: b.replace(b"return rand() % bounds;", b"return rand() % (bounds - 1);", 1)),
        ("crt_random_cpp", lambda b: b.replace(b"0x00538FB0", b"0x00538FB1", 1)),
        ("crt_random_cpp", lambda b: b.replace(b"Status: Complete\n", b"Status: Pending\n", 1)),
        ("crt_random_cpp", lambda b: b.replace(b"BYTE_EXACT", b"", 1)),
        ("crt_random_cpp", lambda b: b"// unadmitted owner prefix\n" + b),
        ("header_io_cpp", lambda b: b.replace(b"fputc(0x1A, file);", b"fputc(0, file);", 1)),
        ("header_io_cpp", lambda b: b.replace(b"*header++ = header_chr;", b"*header++ = 0;", 1)),
        ("header_io_cpp", lambda b: b.replace(b"0x0057D1F0", b"0x0057D1F1", 1)),
        ("header_io_h", lambda b: b.replace(b"char *header", b"int *header", 1)),
        ("header_io_h", lambda b: b + b"// unadmitted API suffix\n"),
        ("crt_random_h", lambda b: b.replace(b"unsigned int __cdecl rnd", b"int __cdecl rnd", 1)),
        ("crt_random_h", lambda b: b"// unadmitted API prefix\n" + b),
        ("general_h", lambda b: b.replace(b'#include "header_io.h"\n', b"", 1)),
        ("general_h", lambda b: b.replace(b'#include "crt_random.h"', b'#include "other_api.h"', 1)),
        ("general_cpp", lambda b: b + b"\n// unadmitted General suffix\n"),
        ("header_io_cpp", lambda b: b + b"\n// unadmitted owner suffix\n"),
    ],
)
def test_inverse_rejects_moved_body_claim_api_and_boundary_damage(filename, mutate):
    sources = _contents()
    changed = mutate(sources[filename])
    assert changed != sources[filename]
    sources[filename] = changed
    with pytest.raises(AssertionError):
        _assert_exact_inverse(sources)


def test_chained_projection_runs_before_prior_council_undo_inverse():
    sources = _contents()
    scale_restored = OLD_HELPER["project_scale_adjustment_general"](
        sources["general_cpp"], sources["scale_adjustment_cpp"], None, sources["general_h"]
    )
    restored_general = OLD_HELPER["project_header_rng_general"](scale_restored)
    assert restored_general == _baseline("general.cpp")
    previous = OLD_HELPER["_actual_split"](OLD_HELPER["LIVE_PATHS"])
    previous["general"] = sources["general_cpp"]
    projected = OLD_HELPER["inverse_split"](previous)
    assert projected == OLD_HELPER["_baseline_sources"]()
