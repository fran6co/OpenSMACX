"""Diagnostic-only comparisons beside refused or unscored strict verdicts."""

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]

from match_recovery.source import read_text  # noqa: E402
from match_recovery.source.asm import (compare_unit, resolve_fragment_pairs,  # noqa: E402
                                       shared_spans)
from match_recovery.diff_html import render_diff  # noqa: E402
sys.path.insert(0, str(ROOT / "packages/match-recovery/tests"))
from test_binary import coff_fixture, pe_fixture  # noqa: E402


def fragment_records(path):
    text = (
        "// ORIGINAL: 0x00401000 parent 0x00401000-0x00401001 FILE\n"
        "// symbol _f\n"
        "// ORIGINAL: 0x00401001 tail 0x00401001-0x00401002 FILE BYTE_EXACT\n"
        '// RECOVERY_FRAGMENT: v1 {"parent_address":"0x00401000",'
        '"parent_symbol":"_f","reason":"adjacent terminal byte",'
        '"evidence":["pinned fixture"]}\n')
    return read_text(text, path)


def shared_records(path):
    text = ("// ORIGINAL: 0x00401000 first 0x00401000-0x00401002 FILE\n"
            "// symbol _f\n"
            "// ORIGINAL: 0x00401001 second 0x00401000-0x00401002 FILE\n"
            "// symbol _g\n")
    return read_text(text, path)


def fixture(tmp_path):
    source = tmp_path / "fixture.cpp"
    source.write_text("// fixture TU\n")
    image = tmp_path / "fixture.exe"
    image.write_bytes(pe_fixture(b"\x90\xc3"))
    return source, image


def test_refused_fragment_child_carries_non_authoritative_slice_comparison(tmp_path):
    source, image = fixture(tmp_path)
    parent, child = fragment_records(source)
    pairs, refused = resolve_fragment_pairs([parent, child], [parent, child])
    assert not refused
    compiler = lambda *_: coff_fixture(b"\x91\xc3")
    plain = compare_unit([parent, child], image, source, [], ((),), compiler=compiler,
                         fragment_pairs=pairs)
    assert all(value.diagnostic_comparison is None for _row, value in plain.results)
    rows = compare_unit([parent, child], image, source, [], ((),), compiler=compiler,
                        fragment_pairs=pairs, diagnostics=True)
    by_address = {row.address: value for row, value in rows.results}
    assert by_address[parent.address].verdict.value == "MISMATCH"
    assert by_address[parent.address].diagnostic_comparison is None
    child_value = by_address[child.address]
    assert child_value.verdict.value == "REFUSED"
    diagnostic = child_value.diagnostic_comparison
    assert diagnostic["authoritative"] is False and diagnostic["claimable"] is False
    assert diagnostic["masked_verdict"] == "BYTE_EXACT"
    assert diagnostic["similarity"] == 1.0
    assert "parent masked verdict MISMATCH" in diagnostic["notes"][0]


def test_shared_tail_reports_diagnostic_only_when_requested(tmp_path):
    source, image = fixture(tmp_path)
    first, second = shared_records(source)
    shared = shared_spans([first, second])
    calls = []

    def compiler(*args):
        calls.append(args)
        return coff_fixture(b"\x40\xc3")

    plain = compare_unit([first], image, source, [], ((),), shared, compiler=compiler)
    assert plain.results[0][1].verdict.value == "SHARED_TAIL"
    assert plain.results[0][1].diagnostic_comparison is None
    assert calls == []
    rows = compare_unit([first], image, source, [], ((),), shared, compiler=compiler,
                        diagnostics=True)
    value = rows.results[0][1]
    assert value.verdict.value == "SHARED_TAIL"
    diagnostic = value.diagnostic_comparison
    assert diagnostic["available"] is True and diagnostic["authoritative"] is False
    assert diagnostic["masked_verdict"] == "MISMATCH"
    assert diagnostic["original_instructions"] == diagnostic["compiled_instructions"] == 2
    assert diagnostic["matching_instructions"] == 1 and diagnostic["first_divergence"] == 0
    assert len(calls) == 1


def test_diagnostic_reports_unavailable_body_without_changing_verdict(tmp_path):
    source, image = fixture(tmp_path)
    first, second = shared_records(source)
    rows = compare_unit([second], image, source, [], ((),), shared_spans([first, second]),
                        compiler=lambda *_: coff_fixture(b"\x90\xc3"), diagnostics=True)
    value = rows.results[0][1]
    assert value.verdict.value == "SHARED_TAIL"
    assert value.diagnostic_comparison["available"] is False
    assert "_g" in value.diagnostic_comparison["reason"]


def test_diff_page_carries_verdict_notice():
    page = render_diff({"title": "f", "source_path": "src/f.cpp", "source_lines": [],
                        "regions": [], "notice": "Strict verdict: REFUSED • DIAGNOSTIC ONLY"})
    assert "Strict verdict: REFUSED" in page and 'id="verdict"' in page
