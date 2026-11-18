"""CANARY-PADS alignment from the first $SG literal a canary relocates against; synthetic objects only."""

import json
import struct
import subprocess

import pytest

from match_recovery.canary import (align_canaries, first_literal, format_residues, literal_binding_only,
                                   parse_block, parse_residues, rewrite_pads, rewrite_residues, solve)
from match_recovery.source.asm import AsmComparison
from match_recovery.source.model import Tier
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
    """A fake compiler whose literal counter is ``base`` plus the unit's pad count."""

    def __init__(self, base):
        self.base, self.overlays = base, 0

    def residue(self, text):
        return (self.base + parse_block(text)["count"]) % 64

    def emit(self, text):
        number = self.base + parse_block(text)["count"]
        return obj({FUNCTION: [number, number + 9]})

    def __call__(self, path, arguments, flags):
        return self.emit(path.read_text())

    def overlay(self, path, replacements, arguments, flags):
        self.overlays += 1
        (data,) = replacements.values()
        return self.emit(data.decode())


def image(compiler, project, exact):
    """A fake image judge: the canary is BYTE_EXACT when its residue is in ``exact``."""
    calls = []

    def judge(unit, data, canaries):
        text = (project / unit).read_text() if data is None else data.decode()
        calls.append(parse_block(text)["count"])
        return {canary["address"]: compiler.residue(text) in exact for canary in canaries}
    judge.calls = calls
    return judge


def run(project, compiler, exact, **options):
    return align_canaries(project, compiler=compiler, judge=image(compiler, project, exact), **options)


def test_exact_block_passes_untouched_and_flags_a_stale_set(project):
    compiler = Counting(64 * 900 + 9)  # two pads: residue 11, recorded set 10-12
    before = (project / "code/f.cc").read_bytes()
    report = run(project, compiler, {11}, write=True)
    unit = report["units"][0]
    assert report["result"] == "pass" and report["published"] == [] and "stale" not in unit
    assert (unit["canaries"][0]["residue"], unit["canaries"][0]["exact"], unit["needed"]) == (11, True, 2)
    report = run(project, compiler, {30, 11})  # exact here too
    assert report["result"] == "pass"
    report = run(project, Counting(64 * 900 + 20), {22})  # exact at residue 22, outside 10-12
    assert report["result"] == "pass" and "sweep" in report["units"][0]["stale"]
    assert (project / "code/f.cc").read_bytes() == before


def test_write_publishes_a_prediction_only_when_the_image_confirms_it(project):
    compiler = Counting(64 * 900 + 5)  # residue 7 with two pads; recorded set 10-12 needs 5 pads
    before = (project / "code/f.cc").read_bytes()
    report = run(project, compiler, {10, 11, 12})
    assert report["result"] == "fail" and report["units"][0]["needed"] == 5
    assert report["units"][0]["prediction_exact"] is True
    assert (project / "code/f.cc").read_bytes() == before
    report = run(project, compiler, {10, 11, 12}, write=True)
    assert report["result"] == "pass" and report["published"] == ["code/f.cc"]
    assert (project / "code/f.cc").read_text() == unit_text(5)


def test_write_refuses_a_stale_set_without_writing(project):
    compiler = Counting(64 * 900 + 5)  # recorded 10-12 predicts 5 pads, but only residue 40 is exact
    before = (project / "code/f.cc").read_bytes()
    report = run(project, compiler, {40}, write=True)
    unit = report["units"][0]
    assert report["result"] == "unknown" and report["published"] == []
    assert unit["prediction_exact"] is False and "--sweep" in unit["reason"]
    assert (project / "code/f.cc").read_bytes() == before


def test_sweep_measures_one_period_and_writes_count_and_sets(project):
    # Residue is (5 + pads) mod 64; the image is exact at residues 3 and 40-41,
    # i.e. 62 or 35-36 pads. From two pads, 35 is the nearest exact count.
    compiler = Counting(64 * 900 + 5)
    judge = image(compiler, project, {3, 40, 41})
    report = align_canaries(project, compiler=compiler, judge=judge, sweep=True)
    unit = report["units"][0]
    assert len(judge.calls) == 64 and sorted(judge.calls) == list(range(0, 64))
    assert unit["needed"] == 35
    assert (unit["canaries"][0]["recorded"], unit["canaries"][0]["measured"]) == ("10-12", "3,40-41")
    assert report["result"] == "fail"
    report = align_canaries(project, compiler=compiler, judge=image(compiler, project, {3, 40, 41}),
                            sweep=True, write=True)
    assert report["result"] == "pass" and report["published"] == ["code/f.cc"]
    assert (project / "code/f.cc").read_text() == unit_text(35, residues="3,40-41")


def test_sweep_without_an_exact_count_fails_without_writing(project):
    compiler = Counting(64 * 900 + 5)
    before = (project / "code/f.cc").read_bytes()
    report = align_canaries(project, compiler=compiler, judge=image(compiler, project, set()),
                            sweep=True, write=True)
    assert report["result"] == "fail" and report["published"] == []
    assert (project / "code/f.cc").read_bytes() == before


def test_missing_literal_block_or_image_is_refused(project):
    report = align_canaries(project, compiler=lambda *_: obj({FUNCTION: []}), judge=lambda *_: {})
    assert report["result"] == "unknown" and "no $SG" in report["units"][0]["reason"]
    with pytest.raises(ValueError, match="no CANARY-PADS block"):
        align_canaries(project, units=["code/g.cc"], compiler=lambda *_: obj({FUNCTION: [1]}))
    with pytest.raises(ValueError, match="not configured"):
        align_canaries(project, units=["code/h.cc"], compiler=lambda *_: obj({FUNCTION: [1]}))
    with pytest.raises(ValueError, match="image"):
        align_canaries(project, compiler=lambda *_: obj({FUNCTION: [1]}))


def test_rewrite_residues_replaces_only_the_named_line():
    text = unit_text(2, ending="\r\n")
    block = parse_block(text)
    assert rewrite_residues(text, block, {2: {3, 40, 41}}) == unit_text(2, residues="3,40-41", ending="\r\n")
    assert rewrite_residues(text, block, {}) == text


def test_cli_dispatches_the_canary_command(project, monkeypatch, capsys):
    import match_recovery.canary as module
    original = module.align_canaries
    compiler = Counting(64 * 900 + 9)
    monkeypatch.setattr(module, "align_canaries",
                        lambda root, **options: original(root, compiler=compiler,
                                                         judge=image(compiler, project, {11}),
                                                         **{k: v for k, v in options.items() if k != "image"}))
    assert main(["canary", "--root", str(project), "--image", "x.exe", "code/f.cc"]) == 0
    assert json.loads(capsys.readouterr().out)["result"] == "pass"


def test_only_stale_literal_bindings_count_as_exact():
    exact = {"available": True, "masked_verdict": str(Tier.BYTE_EXACT)}
    def case(text, payload=exact):
        return literal_binding_only(AsmComparison(verdict=Tier.REFUSED, diagnostic=text,
                                                  diagnostic_comparison=payload))
    advice = ("; bind each literal with `RECOVERY_LITERAL` from the image's operand, or run "
              "`recover rebind-literals --unit <this TU>` if an existing binding went stale")
    assert case("strict body proof unknown: unbound compiler artifact relocation(s): "
                "+0x18a $SG63514, +0x2b5 $SG63529" + advice)
    assert case("strict body proof failed: relocated target(s) differ: +0x10 $SG5; "
                "if a $SG literal binding went stale, run `recover rebind-literals --unit <this TU>`")
    assert not case("strict body proof unknown: unbound compiler artifact relocation(s): +0x10 ?f@@YAXXZ")
    assert not case("strict body proof failed: 4 non-relocation byte(s) differ at +0x46d; "
                    "relocated target(s) differ: +0x10 $SG5")
    assert not case("strict body proof unknown: unbound compiler artifact relocation(s): +0x10 $SG5",
                    {"available": True, "masked_verdict": str(Tier.MISMATCH)})
    assert not case("compiler failed")
