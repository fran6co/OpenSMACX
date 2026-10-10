"""Permanent source/history checks proposed with the narrow zlib migration."""

from dataclasses import fields
from pathlib import Path
import re
import subprocess
import os

from match_recovery.component_source import parse_component
from test_zlib_inflate_group_bindings_oct10 import EXPECTED as FOURTEEN_MEMBERS, UNITS
from match_recovery.source import read_text
from owner_transfer_test_support import assert_current_owner, assert_leaf_collection_retired


BASELINE = "948389543e746ff061ab04cb86e839fdc3600146"
REPO_ROOT = next(path for path in Path(__file__).resolve().parents if (path / ".git").exists())
ROOT = Path(os.environ.get("OPENSMACX_TEST_ROOT", REPO_ROOT)).resolve()
LEAF = Path("src/leaf_recoveries.cpp")
COMPONENT = Path("src/vendor/zlib.recovery.h")
ROLE = ("// RECOVERY_ROLE: SCAFFOLD Transitional leaf collection, not completed "
        "genuine owner recovery; preserve body evidence during owner migration.")
HISTORICAL_ROLE = ("// Historical source-role note: SCAFFOLD Transitional leaf collection, "
                   "not completed genuine owner recovery; preserve body evidence during owner migration.")
HISTORICAL_SYMBOL = "// Historical emitted symbol: "
EXPECTED = {
    0x00640A80: {
        "name": "sub_640a80", "span": (0x00640A80, 0x00640BAE),
        "symbol": "_adler32", "unit": "adler32.c",
        "old_symbol": "_sub_640a80", "annotation_symbol": "_adler32",
    },
    0x00644930: {
        "name": "sub_644930", "span": (0x00644930, 0x00644A62),
        "symbol": "_inflate_flush", "unit": "infutil.c",
        "old_symbol": "_sub_644930", "annotation_symbol": "_inflate_flush",
    },
    0x00642940: {
        "name": "_bi_reverse", "span": (0x00642940, 0x0064295F),
        "symbol": "_bi_reverse", "unit": "trees.c",
        "old_symbol": "?leaf_00642940_redirect@@YAIIH@Z", "annotation_symbol": "",
    },
    0x0063D420: {
        "name": "_putShortMSB", "span": (0x0063D420, 0x0063D44F),
        "symbol": "_putShortMSB", "unit": "deflate.c",
        "old_symbol": "?leaf_0063d420_redirect@@YAXPAXI@Z", "annotation_symbol": "",
    },
}
EXPECTED_FILES = {
    "adler32.c", "deflate.c", "deflate.h", "infblock.c", "infblock.h",
    "infcodes.c", "infcodes.h", "inffast.c", "inffast.h", "inflate.c",
    "inftrees.c", "inftrees.h", "infutil.c", "infutil.h", "trees.c",
    "uncompr.c", "zconf.h", "zlib.h", "zutil.c", "zutil.h",
}
RESTORED = {
    0x0063CF00: (0x0063CF25, "_deflateInit_", "deflate.c"),
    0x0063D4D0: (0x0063D55F, "_deflateEnd", "deflate.c"),
    0x0063DF90: (0x0063DFDD, "_inflateEnd", "inflate.c"),
    0x0063E0F0: (0x0063E10A, "_inflateInit_", "inflate.c"),
    0x0063D450: (0x0063D4C2, "_flush_pending", "deflate.c"),
}


def _git_text(path):
    result = subprocess.run(
        ["git", "show", f"{BASELINE}:{path.as_posix()}"],
        cwd=REPO_ROOT, check=True, capture_output=True, text=True,
    )
    return result.stdout


def _states(text, path):
    rows = read_text(text, path)
    assert len({row.address for row in rows}) == len(rows), "duplicate ORIGINAL identity"
    return {row.address: row for row in rows}


def _facts(state):
    omitted = {"path", "line", "region", "extract_error", "origin", "state", "symbol",
               "source_role", "source_role_reason"}
    facts = tuple((item.name, getattr(state, item.name)) for item in fields(state)
                  if item.name not in omitted)
    lessons = tuple((lesson.kind, lesson.key, lesson.prose,
                     tuple(line for _, line in lesson.lines))
                    for lesson in (state.origin or ()))
    return facts, lessons


def _raw_block(text, address):
    marker = f"// ORIGINAL: 0x{address:08X} "
    assert text.count(marker) == 1, (hex(address), text.count(marker))
    at = text.index(marker)
    start = text.rfind("/*", 0, at)
    end = text.find("*/", at)
    assert start >= 0 and end >= 0, hex(address)
    return text[start:end + 2]


def test_zlib_original_records_move_with_full_catalogue_and_lesson_history():
    baseline_text = _git_text(LEAF)
    assert_leaf_collection_retired(ROOT)
    active_component = (ROOT / COMPONENT).read_text(encoding="utf-8")
    baseline = _states(baseline_text, LEAF)
    current_component = _states(active_component, COMPONENT)

    for address, expected in EXPECTED.items():
        assert_current_owner(ROOT, address, COMPONENT.as_posix())
        before = baseline[address]
        after = current_component[address]
        assert before.name == after.name == expected["name"]
        assert before.image_spans == after.image_spans == (expected["span"],)
        assert _facts(after) == _facts(before)
        assert before.symbol == expected["old_symbol"]
        assert after.symbol == expected["annotation_symbol"]
        old_block = _raw_block(baseline_text, address)
        symbol_match = re.search(r"^// symbol\s+([^\n]+)\n", old_block, flags=re.M)
        assert symbol_match and symbol_match.group(1) == expected["old_symbol"]
        expected_block = old_block.replace(ROLE, HISTORICAL_ROLE, 1)
        expected_block = expected_block.replace(
            symbol_match.group(0), HISTORICAL_SYMBOL + symbol_match.group(1) + "\n", 1,
        )
        actual_block = _raw_block(active_component, address)
        actual_block = re.sub(r"^// symbol\s+_[^\n]+\n", "", actual_block, flags=re.M)
        assert "Historical emitted symbol:" in actual_block
        assert actual_block == expected_block


def test_zlib_component_keeps_reviewed_provider_bindings_with_witnessed_additions():
    manifest = parse_component((ROOT / COMPONENT).read_text(encoding="utf-8"))
    assert manifest.component.component_id == "zlib"
    assert manifest.component.ownership.value == "third_party"
    assert manifest.component.delivery.value == "upstream_source"
    assert manifest.component.version == "1.0.2"
    assert manifest.source_root == "src/vendor/zlib-1.0.2"
    assert manifest.archive_url == "https://zlib.net/fossils/zlib-1.0.2.tar.gz"
    assert manifest.archive_sha256 == "517f7df278536bf9b91bb14104fcb4b13ce7b3c791d174195fec1618621b24e6"
    assert manifest.license_file == "zlib.h"
    provenance = (ROOT / "src/vendor/zlib-1.0.2/PROVENANCE.md").read_text(encoding="utf-8")
    assert "# zlib 1.0.2, vendored verbatim" in provenance
    assert "https://zlib.net/fossils/zlib-1.0.2.tar.gz" in provenance
    assert "517f7df278536bf9b91bb14104fcb4b13ce7b3c791d174195fec1618621b24e6" in provenance
    assert "license notice in `zlib.h`" in provenance
    assert {item.path for item in manifest.files} == EXPECTED_FILES
    assert set(manifest.units) == UNITS
    assert manifest.flags == ("/c", "/O2", "/Gy", "/GR-", "/GX")

    additions = {0x00642960, 0x006429F0, 0x00642A70}
    from test_zlib_ten_member_bindings_oct10 import EXPECTED as TEN_MEMBERS
    from test_zlib_deflate_tree_group_bindings_oct10 import EXPECTED as SIXTEEN_MEMBERS
    assert len(manifest.bindings) == 52, "only independently witnessed additions are allowed"
    assert {binding.origin.address for binding in manifest.bindings} == set(EXPECTED) | set(RESTORED) | additions | set(TEN_MEMBERS) | set(SIXTEEN_MEMBERS) | set(FOURTEEN_MEMBERS)
    for binding in manifest.bindings:
        witnessed = TEN_MEMBERS | SIXTEEN_MEMBERS | FOURTEEN_MEMBERS
        if binding.origin.address in witnessed:
            end, symbol, unit = witnessed[binding.origin.address]
            assert [(span.start, span.end) for span in binding.spans] == [(binding.origin.address, end)]
            assert (binding.symbol, binding.unit) == (symbol, unit)
            continue
        if binding.origin.address in additions:
            # Exact new envelopes, spans and provider symbols have their own
            # continuity/roundtrip controls; historical rows below stay intact.
            assert binding.unit == "trees.c"
            continue
        if binding.origin.address in RESTORED:
            end, symbol, unit = RESTORED[binding.origin.address]
            assert [(span.start, span.end) for span in binding.spans] == [(binding.origin.address, end)]
            assert (binding.symbol, binding.unit) == (symbol, unit)
            assert "Source correspondence only" in binding.evidence
            assert "retained selection" in binding.evidence
            continue
        expected = EXPECTED[binding.origin.address]
        assert binding.origin.image_sha256 == "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
        assert binding.origin.address_space == "pe:va"
        assert [(span.start, span.end) for span in binding.spans] == [expected["span"]]
        assert binding.symbol == expected["symbol"]
        assert binding.unit == expected["unit"]
        if binding.origin.address in (0x00640A80, 0x00644930):
            assert "COFF bytes/boundary/references pass" in binding.evidence
            assert "final-link selection remains unknown" in binding.evidence
        elif binding.origin.address == 0x00642940:
            assert "trees.c" in binding.evidence
            assert "tr_static_init" in binding.evidence and "gen_codes" in binding.evidence
            assert "legal Huffman lengths 1..15" in binding.evidence
            assert "This records source ownership only, not a current byte claim" in binding.evidence
            assert "Component-binary bytes, boundary, and references require a fresh configured component-verifier measurement" in binding.evidence
            assert "Application retained-selection status is UNKNOWN" in binding.evidence
            assert "Whole-library version uniqueness and runtime equivalence are unproved" in binding.evidence
            assert "expected one defined COFF function" not in binding.evidence
        elif binding.origin.address == 0x0063D420:
            assert "deflate.c" in binding.evidence
            assert "zlib header" in binding.evidence and "Adler-32 trailer" in binding.evidence
            assert "pending_buf at +0x08 and pending at +0x10" in binding.evidence
            assert "This records source ownership only, not a current byte claim" in binding.evidence
            assert "Component-binary bytes, boundary, and references require a fresh configured component-verifier measurement" in binding.evidence
            assert "Application retained-selection status is UNKNOWN" in binding.evidence
            assert "Whole-library version uniqueness and runtime equivalence are unproved" in binding.evidence
            assert "expected one defined COFF function" not in binding.evidence
        else:
            raise AssertionError(f"unreviewed zlib binding 0x{binding.origin.address:08X}")


def test_no_application_standin_or_declaration_remains_and_cmake_keeps_zlib():
    assert_leaf_collection_retired(ROOT)
    component = (ROOT / COMPONENT).read_text(encoding="utf-8")
    for address in EXPECTED:
        assert_current_owner(ROOT, address, COMPONENT.as_posix())
    assert not (ROOT / "src/leaf_recoveries.h").exists()
    cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    for name in ("sub_640a80", "sub_644930"):
        assert not re.search(rf"\b{re.escape(name)}\s*\(", component)
    assert "src/vendor/zlib-1.0.2/adler32.c" in cmake
    assert "src/vendor/zlib-1.0.2/infutil.c" in cmake
    assert "src/vendor/zlib-1.0.2/deflate.c" in cmake
    assert "src/vendor/zlib-1.0.2/trees.c" in cmake
    link = re.search(r"target_link_libraries\(OpenSMACX PRIVATE ([^)]+)\)", cmake)
    assert link and "zlib" in link.group(1).split()

    for path in (ROOT / "src").rglob("*"):
        if (not path.is_file() or path.suffix not in (".cpp", ".h", ".c")
                or path.is_relative_to(ROOT / "src/vendor/zlib-1.0.2")):
            continue
        content = path.read_text(encoding="utf-8", errors="replace")
        for signature in (
            r'extern\s+"C"\s+unsigned\s+int\s+__cdecl\s+sub_640a80',
            r'extern\s+"C"\s+int\s+__cdecl\s+sub_644930',
        ):
            assert not re.search(signature + r"\s*\([^;{}]*\)\s*\{", content, re.S), path
