"""CANARY-PADS alignment from the first $SG literal a canary relocates against; synthetic objects only."""

import json
import struct
import subprocess

import pytest

from match_recovery.canary import (align_canaries, first_literal, format_residues, parse_block,
                                   parse_residues, rewrite_pads, solve)
from match_recovery.cli import main
from match_recovery.pe_coff import Coff


FUNCTION = "?f@@YAXXZ"


def obj(functions):
    """``{symbol: [$SG numbers relocated, in body order]}`` as a two-section COFF."""
    names = sorted({number for numbers in functions.values() for number in numbers})
    text, relocations, symbols = b"", [], []
    # Symbol table: each function, then each $SG label (no section definitions needed).
    first_label = len(functions)
    for name, numbers in functions.items():
        start = len(text)
        symbols.append((name, start, 1, 0x20, 2))
        for number in numbers:
            relocations.append((len(text) + 1, first_label + names.index(number), 6))
            text += b"\x68\0\0\0\0"
        text += b"\xC3"
    for index, number in enumerate(names):
        symbols.append((f"$SG{number}".encode().ljust(8, b"\0"), index * 4, 2, 0, 3))
    data = bytes(4 * len(names))
    strings = b""
    table = b""
    for name, value, section, kind, storage in symbols:
        if isinstance(name, str):
            raw = name.encode()
            if len(raw) > 8:
                field = struct.pack("<II", 0, 4 + len(strings))
                strings += raw + b"\0"
            else:
                field = raw.ljust(8, b"\0")
        else:
            field = name
        table += field + struct.pack("<IhHBB", value, section, kind, storage, 0)
    header_size, section_size = 20, 40
    text_at = header_size + 2 * section_size
    relocations_at = text_at + len(text)
    data_at = relocations_at + 10 * len(relocations)
    symbols_at = data_at + len(data)
    header = struct.pack("<HHIIIHH", 0x14C, 2, 0, symbols_at, len(symbols), 0, 0)
    sections = (b".text\0\0\0" + struct.pack("<IIIIIIHHI", 0, 0, len(text), text_at, relocations_at, 0,
                                             len(relocations), 0, 0x60000020)
                + b".data\0\0\0" + struct.pack("<IIIIIIHHI", 0, 0, len(data), data_at, 0, 0, 0, 0, 0xC0300040))
    body = b"".join(struct.pack("<IIH", *relocation) for relocation in relocations)
    return header + sections + text + body + data + table + struct.pack("<I", 4 + len(strings)) + strings


def unit_text(count, residues="10-12", kind="function", ending="\n"):
    pads = [f"void Pad{index}();" if kind == "function" else f"extern int Pad{index};" for index in range(count)]
    lines = ["#include \"x.h\"",
             f"// CANARY: 0x00401000 {FUNCTION} residues {residues}",
             "// Any other comment in the run is allowed.",
             f"// CANARY-PADS: {kind} Pad",
             *pads,
             "void f() {}", ""]
    return ending.join(lines)


def test_residue_lists_round_trip_and_refuse_bad_ranges():
    assert parse_residues("54-56,28-30,39") == frozenset({28, 29, 30, 39, 54, 55, 56})
    assert format_residues({28, 29, 30, 39, 54, 55, 56}) == "28-30,39,54-56"
    for bad in ("5-3", "64", "1,,2", "x"):
        with pytest.raises(ValueError):
            parse_residues(bad)


def test_block_parsing_counts_numbered_pads_and_refuses_strays():
    block = parse_block(unit_text(3))
    assert (block["kind"], block["prefix"], block["count"], block["line"]) == ("function", "Pad", 3, 4)
    assert [(c["symbol"], c["line"], sorted(c["residues"])) for c in block["canaries"]] == [(FUNCTION, 2, [10, 11, 12])]
    assert parse_block("void f() {}\n") is None
    with pytest.raises(ValueError, match="without a CANARY-PADS"):
        parse_block(f"// CANARY: 0x00401000 {FUNCTION} residues 1\nvoid f();\n")
    with pytest.raises(ValueError, match="not attached"):
        parse_block(f"// CANARY: 0x00401000 {FUNCTION} residues 1\n\n" + unit_text(1))
    with pytest.raises(ValueError, match="out of sequence"):
        parse_block(unit_text(2).replace("Pad1();", "Pad2();"))
    with pytest.raises(ValueError, match="more than one"):
        parse_block(unit_text(1) + unit_text(1))


def test_rewrite_replaces_only_pad_lines_and_keeps_crlf():
    text = unit_text(2, kind="extern", ending="\r\n")
    block = parse_block(text)
    result = rewrite_pads(text, block, 4)
    assert result == unit_text(4, kind="extern", ending="\r\n")
    assert rewrite_pads(result, parse_block(result), 0) == unit_text(0, kind="extern", ending="\r\n")


def test_first_literal_reads_the_body_relocation_order():
    data = Coff(obj({FUNCTION: [205, 203], "?g@@YAXXZ": [7]}))
    assert first_literal(data, FUNCTION) == 205
    assert first_literal(Coff(obj({FUNCTION: []})), FUNCTION) is None
    with pytest.raises(ValueError):
        first_literal(data, "?missing@@YAXXZ")


def test_solve_prefers_the_smallest_change_and_never_negative_counts():
    assert solve(7, [(54, frozenset({54}))]) == 7
    assert solve(7, [(52, frozenset({54, 55}))]) == 9
    assert solve(7, [(56, frozenset({54}))]) == 5
    assert solve(1, [(5, frozenset({0}))]) == 60  # 1 - 5 would be negative
    assert solve(3, [(10, frozenset({12})), (20, frozenset({21}))]) is None
    assert solve(3, [(10, frozenset({12, 13})), (20, frozenset({23}))]) == 6


@pytest.fixture
def project(tmp_path):
    for name in ("code", "recovery", "build"):
        (tmp_path / name).mkdir()
    (tmp_path / "code/PIN").write_text("sha256 " + "0" * 64 + "\n")
    (tmp_path / "code/f.cc").write_text(unit_text(2))
    (tmp_path / "code/g.cc").write_text("void g() {}\n")
    (tmp_path / "build/compile_commands.json").write_text(json.dumps([
        {"file": f"code/{name}", "directory": str(tmp_path), "arguments": ["fake", f"code/{name}"]}
        for name in ("f.cc", "g.cc")]))
    config = {"schema_version": 1,
              "source": {"image_pin": "code/PIN", "address_space": "pe:va", "roots": ["code"],
                         "seed_roots": ["code/seeds"], "include_paths": ["code"]},
              "mappings": {"compile_commands": "build/compile_commands.json", "forwarder_macros": []},
              "measurement": {"backend": "pe-coff-i386", "compiler_profile": "test",
                              "flag_sets": [["first"]], "layout_flags": [],
                              "unit_extensions": [".cc"], "inputs": [], "input_roots": []}}
    (tmp_path / "recovery/project.json").write_text(json.dumps(config))
    (tmp_path / ".gitignore").write_text("build/\n")
    for args in (("init",), ("add", "."),):
        subprocess.run(["git", *args], cwd=tmp_path, check=True, capture_output=True)
    return tmp_path


class Counting:
    """A fake compiler whose literal counter is ``base`` plus the unit's pad count.

    ``good`` residues compile the canary to its exact body; others to a variant.
    """

    def __init__(self, base, good=None):
        self.base, self.good, self.overlays = base, good, 0

    def emit(self, text):
        number = self.base + parse_block(text)["count"]
        body = [number, number + 9]
        if self.good is not None and number % 64 not in self.good:
            body = body + [number + 10]
        return obj({FUNCTION: body})

    def __call__(self, path, arguments, flags):
        return self.emit(path.read_text())

    def overlay(self, path, replacements, arguments, flags):
        self.overlays += 1
        (data,) = replacements.values()
        return self.emit(data.decode())


def counting(project, base):
    return Counting(base)


def test_aligned_block_passes_and_is_untouched(project):
    before = (project / "code/f.cc").read_bytes()
    report = align_canaries(project, compiler=counting(project, 64 * 900 + 9), write=True)
    assert report["result"] == "pass" and report["published"] == []
    row = report["units"][0]["canaries"][0]
    assert (row["residue"], row["aligned"], report["units"][0]["needed"]) == (11, True, 2)
    assert (project / "code/f.cc").read_bytes() == before


def test_dry_run_reports_the_needed_count_and_write_verifies(project):
    compiler = counting(project, 64 * 900 + 5)  # residue 7 with two pads: needs 5 more
    before = (project / "code/f.cc").read_bytes()
    report = align_canaries(project, units=["code/f.cc"], compiler=compiler)
    assert report["result"] == "fail" and report["units"][0]["needed"] == 5
    assert (project / "code/f.cc").read_bytes() == before
    report = align_canaries(project, units=["code/f.cc"], compiler=compiler, write=True)
    assert report["result"] == "pass" and report["published"] == ["code/f.cc"]
    assert (project / "code/f.cc").read_text() == unit_text(5)
    assert report["units"][0]["canaries_after"][0]["aligned"] is True


def test_write_refuses_when_the_overlay_misses(project):
    class Fixed(Counting):
        def emit(self, text):
            return obj({FUNCTION: [64 * 900 + 1]})  # pads do not move this counter
    before = (project / "code/f.cc").read_bytes()
    compiler = Fixed(0)
    report = align_canaries(project, compiler=compiler, write=True)
    unit = report["units"][0]
    assert report["result"] == "unknown" and report["published"] == [] and compiler.overlays == 1
    assert unit["canaries_after"][0]["aligned"] is False
    assert (project / "code/f.cc").read_bytes() == before


def test_calibrate_reports_the_residues_that_keep_current_bytes(project):
    # Two pads put the counter at residue 11 (good); the exact body recurs at 10-12 and 40.
    compiler = Counting(64 * 900 + 9, good={10, 11, 12, 40})
    before = (project / "code/f.cc").read_bytes()
    report = align_canaries(project, calibrate=True, compiler=compiler)
    row = report["units"][0]["canaries"][0]
    assert (row["recorded"], row["calibrated"]) == ("10-12", "10-12,40")
    assert report["result"] == "fail" and compiler.overlays == 64
    assert (project / "code/f.cc").read_bytes() == before
    with pytest.raises(ValueError, match="never writes"):
        align_canaries(project, calibrate=True, write=True, compiler=compiler)


def test_missing_literal_or_block_is_refused(project):
    report = align_canaries(project, compiler=lambda *_: obj({FUNCTION: []}))
    assert report["result"] == "unknown" and "no $SG" in report["units"][0]["reason"]
    with pytest.raises(ValueError, match="no CANARY-PADS block"):
        align_canaries(project, units=["code/g.cc"], compiler=lambda *_: obj({FUNCTION: [1]}))
    with pytest.raises(ValueError, match="not configured"):
        align_canaries(project, units=["code/h.cc"], compiler=lambda *_: obj({FUNCTION: [1]}))


def test_cli_dispatches_the_canary_command(project, monkeypatch, capsys):
    import match_recovery.canary as module
    original = module.align_canaries
    monkeypatch.setattr(module, "align_canaries",
                        lambda root, **options: original(root, compiler=counting(project, 64 * 900 + 9), **options))
    assert main(["canary", "--root", str(project), "code/f.cc"]) == 0
    assert json.loads(capsys.readouterr().out)["result"] == "pass"
