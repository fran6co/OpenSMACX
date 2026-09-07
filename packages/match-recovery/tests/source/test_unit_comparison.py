"""One compiler object per TU/trial, with independent subject outcomes."""

from dataclasses import replace
import struct

import pytest

from match_recovery.source import read_file, Tier
from match_recovery.source import asm


def coff(functions):
    """One synthetic section per function, as required by the legacy comparer."""
    raw = 20 + 40 * len(functions)
    symbols_at = raw + sum(len(code) for symbol, code in functions)
    header = struct.pack("<HHIIIHH", 0x14c, len(functions), 0, symbols_at, len(functions), 0, 0)
    sections, bodies, symbols = b"", b"", b""
    strings = bytearray(4)
    for index, (symbol, code) in enumerate(functions, 1):
        sections += struct.pack("<8sIIIIIIHHI", b".text", 0, 0, len(code), raw + len(bodies),
                                0, 0, 0, 0, 0x60000020)
        symbols += b"\0" * 4 + struct.pack("<IIhHBB", len(strings), 0, index, 0, 2, 0)
        strings.extend(symbol.encode() + b"\0")
        bodies += code
    struct.pack_into("<I", strings, 0, len(strings))
    return header + sections + bodies + symbols + strings


@pytest.fixture
def unit(tmp_path, monkeypatch):
    path = tmp_path / "family.cpp"
    path.write_text("// ORIGINAL: 0x00401000 _a 0x00401000-0x00401001\nvoid a() {}\n"
                    "// ORIGINAL: 0x00402000 _b 0x00402000-0x00402003\nvoid b() {}\n")
    rows = read_file(path)
    bodies = {rows[0].address: b"\xc3", rows[1].address: b"\x33\xc0\xc3"}
    monkeypatch.setattr(asm, "span_refusal", lambda *a, **k: None)
    monkeypatch.setattr(asm, "original_asm", lambda row, *a, **k:
                        asm.Listing(code=bodies[row.address], base=row.address))
    first = coff([("_a", b"\xc3"), ("_b", b"\xb8\x01\x00\x00\x00\xc3")])
    second = coff([("_a", b"\x33\xc0\xc3"), ("_b", b"\x33\xc0\xc3")])
    calls = []
    def compiler(source, command, flags):
        calls.append((source, command, flags))
        return first if flags == ("first",) else second
    return path, rows, compiler, calls


def measure(unit, **kwargs):
    path, rows, compiler, _ = unit
    return asm.compare_unit(rows, "image", path, ["compiler"], ("first", "second"),
                            compiler=compiler, **kwargs)


def test_grouped_and_individual_measurements_agree_with_fewer_compiles(unit):
    path, rows, compiler, calls = unit
    individual = [asm.compare_record(row, "image", ["compiler"], ("first", "second"),
                                      compiler=compiler) for row in rows]
    assert len(calls) == 3
    calls.clear()
    before = path.read_bytes(), path.stat().st_mtime_ns
    grouped = measure(unit)
    assert [value for row, value in grouped.results] == individual
    assert [value.flags for row, value in grouped.results] == ["first", "second"]
    assert grouped.compile_attempts == len(calls) == 2 and grouped.reused_attempts == 1
    assert before == (path.read_bytes(), path.stat().st_mtime_ns)


def test_objects_are_not_reused_between_calls(unit):
    measure(unit)
    measure(unit)
    assert len(unit[3]) == 4


def test_missing_symbol_does_not_discard_measured_siblings(unit):
    path, rows, compiler, calls = unit
    missing = replace(rows[0], name="_absent", line=rows[0].line + 100)
    result = asm.compare_unit([missing, *rows], "image", path, ["compiler"], ("first", "second"), compiler=compiler)
    assert isinstance(result.results[0][1], str) and "absent" in result.results[0][1]
    assert all(value.verdict == Tier.BYTE_EXACT for row, value in result.results[1:])
    assert result.compile_attempts == len(calls) == 2


@pytest.mark.parametrize("retry,expected", [(lambda error: False, 1), (None, 2)])
def test_compiler_failures_are_reused_only_within_the_same_unit(unit, retry, expected):
    path, rows, _, calls = unit
    def failure(*args):
        calls.append(args)
        raise ValueError("synthetic compilation failure")
    result = asm.compare_unit(rows, "image", path, [], ("first", "second"), compiler=failure, retry_compile=retry)
    assert len(calls) == expected and result.compile_attempts == expected
    assert result.reused_attempts == expected
    assert all(value.verdict == Tier.NO_COMPILE for row, value in result.results)
    assert all(value.diagnostic == "synthetic compilation failure" for row, value in result.results)


def test_a_later_flag_can_emit_a_symbol_missing_from_the_first(unit):
    path, rows, _, calls = unit
    def compiler(*args):
        calls.append(args)
        return coff([("_a", b"\xc3")] + ([] if args[-1] == ("first",) else [("_b", b"\x33\xc0\xc3")]))
    result = asm.compare_unit(rows, "image", path, [], ("first", "second"), compiler=compiler)
    assert len(calls) == 2 and all(value.verdict == Tier.BYTE_EXACT for row, value in result.results)


def test_refusals_and_unresolved_originals_do_not_force_extra_compiles(unit, monkeypatch):
    path, rows, compiler, calls = unit
    monkeypatch.setattr(asm, "span_refusal", lambda row, *a: Tier.REFUSED if row == rows[0] else None)
    result = measure(unit)
    assert result.results[0][1].verdict == Tier.REFUSED
    assert result.results[1][1].verdict == Tier.BYTE_EXACT and len(calls) == 2
    monkeypatch.setattr(asm, "span_refusal", lambda *a: Tier.REFUSED)
    calls.clear()
    assert measure(unit).compile_attempts == 0 and not calls


def test_refusal_uses_the_callers_whole_source_span_inventory(unit, monkeypatch):
    _, rows, _, calls = unit
    shared = frozenset(rows[0].image_spans)
    seen = []
    def refusal(row, image, spans):
        seen.append(spans)
        return Tier.SHARED_TAIL if row.image_spans[0] in spans else None
    monkeypatch.setattr(asm, "span_refusal", refusal)
    result = measure(unit, shared=shared)
    assert result.results[0][1].verdict == Tier.SHARED_TAIL
    assert seen == [shared, shared] and len(calls) == 2


@pytest.mark.parametrize("damage", ["candidate", "marker", "delete"])
def test_source_changes_during_compilation_refuse_the_entire_unit(unit, tmp_path, damage):
    path, rows, compiler, _ = unit
    candidate = tmp_path / "candidate.cpp"
    candidate.write_bytes(path.read_bytes())
    def changing(*args):
        value = compiler(*args)
        if damage == "delete":
            candidate.unlink(missing_ok=True)
        else:
            (candidate if damage == "candidate" else path).write_text("changed source")
        return value
    with pytest.raises((OSError, ValueError)):
        asm.compare_unit(rows, "image", candidate, [], ("first", "second"), compiler=changing)


@pytest.mark.parametrize("selection", ["empty", "duplicate", "other-host"])
def test_ambiguous_unit_selection_fails_before_compilation(unit, tmp_path, selection):
    path, rows, compiler, calls = unit
    selected = [] if selection == "empty" else [rows[0], rows[0]] if selection == "duplicate" else [
        rows[0], replace(rows[1], path=tmp_path / "other.cpp")]
    with pytest.raises(ValueError):
        asm.compare_unit(selected, "image", path, [], ("first",), compiler=compiler)
    assert not calls


def test_two_marker_locations_for_one_address_are_not_collapsed(unit):
    path, rows, compiler, calls = unit
    selected = [rows[0], replace(rows[0], line=rows[0].line + 100)]
    result = asm.compare_unit(selected, "image", path, [], ("first",), compiler=compiler)
    assert [row.line for row, value in result.results] == [row.line for row in selected]
    assert len(calls) == 1 and len(result.results) == 2
