"""Permanent source/history checks proposed with the narrow zlib migration."""

from dataclasses import fields
from pathlib import Path
import re
import subprocess
import os

from match_recovery.component_source import parse_component
from match_recovery.source import read_text


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
    },
    0x00644930: {
        "name": "sub_644930", "span": (0x00644930, 0x00644A62),
        "symbol": "_inflate_flush", "unit": "infutil.c",
    },
}
EXPECTED_FILES = {
    "adler32.c", "deflate.c", "deflate.h", "infblock.c", "infblock.h",
    "infcodes.c", "infcodes.h", "inffast.c", "inffast.h", "inflate.c",
    "inftrees.c", "inftrees.h", "infutil.c", "infutil.h", "trees.c",
    "uncompr.c", "zconf.h", "zlib.h", "zutil.c", "zutil.h",
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
    active_leaf = (ROOT / LEAF).read_text(encoding="utf-8")
    active_component = (ROOT / COMPONENT).read_text(encoding="utf-8")
    baseline = _states(baseline_text, LEAF)
    current_leaf = _states(active_leaf, LEAF)
    current_component = _states(active_component, COMPONENT)

    for address, expected in EXPECTED.items():
        before = baseline[address]
        after = current_component[address]
        assert address not in current_leaf
        assert before.name == after.name == expected["name"]
        assert before.image_spans == after.image_spans == (expected["span"],)
        assert _facts(after) == _facts(before)
        assert before.symbol == f"_sub_{address:06x}"
        assert after.symbol == expected["symbol"]
        old_block = _raw_block(baseline_text, address)
        expected_block = old_block.replace(ROLE, HISTORICAL_ROLE, 1)
        expected_block = expected_block.replace(
            f"// symbol   _sub_{address:06x}",
            HISTORICAL_SYMBOL + f"_sub_{address:06x}", 1,
        )
        actual_block = _raw_block(active_component, address)
        actual_block = re.sub(r"^// symbol\s+_[^\n]+\n", "", actual_block, flags=re.M)
        assert "Historical emitted symbol:" in actual_block
        assert actual_block == expected_block


def test_zlib_component_keeps_exactly_the_two_reviewed_provider_bindings():
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
    assert set(manifest.units) == {"adler32.c", "infutil.c"}
    assert manifest.flags == ("/c", "/O2", "/Gy", "/GR-", "/GX")

    assert len(manifest.bindings) == 2, "do not revive the retired broad zlib mapping"
    for binding in manifest.bindings:
        expected = EXPECTED[binding.origin.address]
        assert binding.origin.image_sha256 == "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
        assert binding.origin.address_space == "pe:va"
        assert [(span.start, span.end) for span in binding.spans] == [expected["span"]]
        assert binding.symbol == expected["symbol"]
        assert binding.unit == expected["unit"]
        assert "COFF bytes/boundary/references pass" in binding.evidence
        assert "final-link selection remains unknown" in binding.evidence


def test_no_application_standin_or_declaration_remains_and_cmake_keeps_zlib():
    leaf = (ROOT / LEAF).read_text(encoding="utf-8")
    header = (ROOT / "src/leaf_recoveries.h").read_text(encoding="utf-8")
    cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    for name in ("sub_640a80", "sub_644930"):
        assert not re.search(rf"\b{re.escape(name)}\s*\(", leaf)
        assert not re.search(rf"\b{re.escape(name)}\s*\(", header)
    assert "src/vendor/zlib-1.0.2/adler32.c" in cmake
    assert "src/vendor/zlib-1.0.2/infutil.c" in cmake
    assert "target_link_libraries(OpenSMACX PRIVATE zlib" in cmake

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
