"""Stale positional $SG literal symbols are re-identified, never guessed; synthetic bytes only."""

import hashlib
import json
import struct
import subprocess

import pytest

from match_recovery import literal_rebind
from match_recovery.cli import main
from match_recovery.literal_rebind import rebind_literals
from match_recovery.literal_source import parse_literals
from match_recovery.literal_writer import rewrite_literal_symbols
from match_recovery.source.asm import strict_row_reason


# Original .rdata at 0x402000: two identical unpooled "abc" copies, then others.
RDATA = b"abc\0" + b"abc\0" + b"hello\0\0\0" + b"zz\0\0" + b"miss\0\0\0\0"
ABC1, ABC2, HELLO, ZZ, MISS = 0x402000, 0x402004, 0x402008, 0x402010, 0x402014


def image(rdata=RDATA):
    data = bytearray(2048)
    data[:2] = b"MZ"
    struct.pack_into("<I", data, 0x3C, 0x80)
    data[0x80:0x84] = b"PE\0\0"
    struct.pack_into("<HHIIIHH", data, 0x84, 0x14C, 2, 0, 0, 0, 224, 0)
    struct.pack_into("<H", data, 0x98, 0x10B)
    struct.pack_into("<I", data, 0x98 + 28, 0x400000)
    struct.pack_into("<I", data, 0x98 + 92, 16)
    data[0x178:0x180] = b".text\0\0\0"
    struct.pack_into("<IIII", data, 0x180, 512, 0x1000, 512, 512)
    struct.pack_into("<I", data, 0x178 + 36, 0x60000020)
    data[0x1A0:0x1A8] = b".rdata\0\0"
    struct.pack_into("<IIII", data, 0x1A8, len(rdata), 0x2000, len(rdata), 1024)
    struct.pack_into("<I", data, 0x1A0 + 36, 0x40000040)
    data[512:528] = b"\x90" * 16
    data[1024:1024 + len(rdata)] = rdata
    return bytes(data)


def obj(*labels):
    """One ordinary .data section holding ``(number, contents)`` $SG labels, 4-aligned."""
    data, values = b"", []
    for number, contents in labels:
        values.append((number, len(data)))
        data += contents + bytes(-len(contents) % 4)
    base = 60
    symbols = (b".data\0\0\0" + struct.pack("<IhHBB", 0, 1, 0, 3, 1)
               + struct.pack("<IHHIHB3s", len(data), 0, 0, 0, 0, 0, bytes(3)))
    for number, value in values:
        symbols += f"$SG{number}".encode().ljust(8, b"\0") + struct.pack("<IhHBB", value, 1, 0, 3, 0)
    header = struct.pack("<HHIIIHH", 0x14C, 1, 0, base + len(data), 2 + len(values), 0, 0)
    section = b".data\0\0\0" + struct.pack("<IIIIIIHHI", 0, 0, len(data), base, 0, 0, 0, 0, 0xC0300040)
    return header + section + data + symbols + struct.pack("<I", 4)


def literal(start, end, symbol, merged=False):
    form = "v2 merged" if merged else "v1"
    return f"  // RECOVERY_LITERAL: {form} 0x{start:08X} 0x{end:08X} {symbol} 0x00401000\r\n"


@pytest.fixture
def project(tmp_path):
    for name in ("code", "recovery", "build"):
        (tmp_path / name).mkdir()
    raw = image()
    (tmp_path / "original.exe").write_bytes(raw)
    (tmp_path / "code/PIN").write_text("sha256 " + hashlib.sha256(raw).hexdigest() + "\n")
    (tmp_path / "code/f.cc").write_text("void f() {}\n")
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
    (tmp_path / ".gitignore").write_text("build/\noriginal.exe\n")
    for args in (("init",), ("add", "."),):
        subprocess.run(["git", *args], cwd=tmp_path, check=True, capture_output=True)
    return tmp_path


def source(project, *bindings, name="f.cc"):
    path = project / "code" / name
    path.write_bytes(("void f() {\r\n" + "".join(bindings) + "}\r\n").encode())
    return path


def run(project, *labels, write=False, **options):
    return rebind_literals(project, project / "original.exe", write=write,
                           compiler=lambda *_: obj(*labels), **options)


def rows(report):
    return [(row["previous"], row["symbol"], row["status"])
            for unit in report["units"] for row in unit["bindings"]]


def test_current_bindings_are_reported_valid_and_untouched(project):
    path = source(project, literal(ABC1, ABC1 + 4, "$SG100"), literal(HELLO, HELLO + 6, "$SG102"))
    before = path.read_bytes()
    report = run(project, (100, b"abc\0"), (101, b"abc\0"), (102, b"hello\0"), write=True)
    assert rows(report) == [("$SG100", "$SG100", "valid"), ("$SG102", "$SG102", "valid")]
    assert report["result"] == "pass" and not report["persisted"] and report["units"][0]["shift"] == 0
    assert path.read_bytes() == before


def test_uniform_shift_resolves_duplicates_and_writes_only_symbol_tokens(project):
    path = source(project, literal(ABC1, ABC1 + 4, "$SG100"), literal(ABC2, ABC2 + 4, "$SG101"),
                  literal(HELLO, HELLO + 6, "$SG103"), literal(ZZ, ZZ + 3, "??_C@_02zz", merged=True))
    before = path.read_bytes()
    labels = ((104, b"abc\0"), (105, b"abc\0"), (106, b"other\0"), (107, b"hello\0"))
    report = run(project, *labels)
    assert rows(report) == [("$SG100", "$SG104", "rebind"), ("$SG101", "$SG105", "rebind"),
                            ("$SG103", "$SG107", "rebind")]
    assert report["result"] == "fail" and report["units"][0]["shift"] == 4
    assert path.read_bytes() == before, "dry run never writes"

    report = run(project, *labels, write=True)
    assert report["persisted"] and report["published"] == ["code/f.cc"] and report["result"] == "pass"
    after = path.read_bytes()
    assert after == (before.replace(b"$SG100", b"$SG104").replace(b"$SG101", b"$SG105")
                     .replace(b"$SG103", b"$SG107"))
    assert rows(run(project, *labels)) == [("$SG104", "$SG104", "valid"), ("$SG105", "$SG105", "valid"),
                                           ("$SG107", "$SG107", "valid")]


def test_content_valid_duplicate_still_follows_the_units_uniform_shift(project):
    # $SG101 still holds "abc" after the shift, but it is now the other copy.
    source(project, literal(ABC2, ABC2 + 4, "$SG101"), literal(HELLO, HELLO + 6, "$SG103"))
    report = run(project, (101, b"abc\0"), (102, b"abc\0"), (103, b"x\0"), (104, b"hello\0"))
    assert rows(report) == [("$SG101", "$SG102", "rebind"), ("$SG103", "$SG104", "rebind")]


def test_nonuniform_shift_uses_a_unique_order_preserving_assignment(project):
    source(project, literal(HELLO, HELLO + 6, "$SG100"), literal(ZZ, ZZ + 3, "$SG101"))
    report = run(project, (104, b"hello\0"), (105, b"x\0"), (110, b"zz\0"))
    assert rows(report) == [("$SG100", "$SG104", "rebind"), ("$SG101", "$SG110", "rebind")]
    assert {row["basis"] for row in report["units"][0]["bindings"]} == {"order_preserving"}


def test_reordered_definitions_rebind_by_unique_content_and_leave_duplicates(project):
    # The source's definitions were reordered: the fresh labels no longer rise
    # in the bindings' old order, so no order-preserving assignment exists.
    source(project, literal(HELLO, HELLO + 6, "$SG100"), literal(ZZ, ZZ + 3, "$SG101"))
    report = run(project, (104, b"zz\0"), (105, b"x\0"), (110, b"hello\0"))
    assert rows(report) == [("$SG100", "$SG110", "rebind"), ("$SG101", "$SG104", "rebind")]
    assert {row["basis"] for row in report["units"][0]["bindings"]} == {"unique_content"}

    # An identical duplicate is still not guessed, and a label two bindings
    # could claim identifies neither; the unique one beside them is rebound.
    source(project, literal(ABC1, ABC1 + 4, "$SG100"), literal(HELLO, HELLO + 6, "$SG101"))
    report = run(project, (200, b"hello\0"), (201, b"abc\0"), (202, b"abc\0"))
    assert rows(report) == [("$SG100", "$SG100", "unproven"), ("$SG101", "$SG200", "rebind")]
    reason = report["units"][0]["bindings"][0]["reason"]
    assert "existing binding order" in reason and reason.endswith("$SG201, $SG202")
    assert report["result"] == "unknown"


def test_ambiguous_duplicates_and_missing_bytes_are_reported_not_guessed(project):
    path = source(project, literal(ABC1, ABC1 + 4, "$SG100"))
    before = path.read_bytes()
    report = run(project, (200, b"abc\0"), (201, b"abc\0"), write=True)
    row, = report["units"][0]["bindings"]
    assert row["status"] == "unproven" and "several uniform label shifts" in row["reason"]
    assert row["candidates"] == ["$SG200", "$SG201"]
    assert report["result"] == "unknown" and not report["persisted"] and path.read_bytes() == before

    source(project, literal(MISS, MISS + 5, "$SG100"), literal(HELLO, HELLO + 6, "$SG101"))
    report = run(project, (100, b"mist\0"), (102, b"hello\0"), write=True)
    assert rows(report) == [("$SG100", "$SG100", "unproven"), ("$SG101", "$SG102", "rebound")]
    assert "no fresh $SG label" in report["units"][0]["bindings"][0]["reason"]
    # Proven rebinds publish even when another binding stays unproven.
    assert b"$SG102" in (project / "code/f.cc").read_bytes()


def test_overlong_or_padded_label_extent_is_not_a_candidate(project):
    source(project, literal(ZZ, ZZ + 3, "$SG100"))
    # "zz\0" followed by nonzero storage before the next label is not the literal.
    report = run(project, (101, b"zz\0q"), (102, b"x\0"))
    assert rows(report) == [("$SG100", "$SG100", "unproven")]


def test_stale_pooled_names_are_rebound_by_content_and_layout(project):
    # The TU stopped pooling: the fresh object defines no ??_C@ symbol at all.
    # A pooled name binds one address, here the second of two identical copies.
    path = source(project, literal(ABC2, ABC2 + 4, "??_C@_03abc"), literal(HELLO, HELLO + 6, "??_C@_05hello"))
    before = path.read_bytes()
    labels = ((10, b"abc\0"), (11, b"abc\0"), (12, b"hello\0"))
    report = run(project, *labels)
    assert rows(report) == [("??_C@_03abc", "$SG11", "rebind"), ("??_C@_05hello", "$SG12", "rebind")]
    assert [row["basis"] for row in report["units"][0]["bindings"]] == ["unpooled_layout", "unpooled_content"]
    assert path.read_bytes() == before

    report = run(project, *labels, write=True)
    assert report["persisted"] and report["result"] == "pass"
    assert path.read_bytes() == before.replace(b"??_C@_03abc", b"$SG11").replace(b"??_C@_05hello", b"$SG12")
    assert rows(run(project, *labels)) == [("$SG11", "$SG11", "valid"), ("$SG12", "$SG12", "valid")]


def test_stale_pooled_duplicates_use_an_existing_static_binding_as_anchor(project):
    source(project, literal(ABC2, ABC2 + 4, "??_C@_03abc"), literal(HELLO, HELLO + 6, "$SG12"))
    report = run(project, (10, b"abc\0"), (11, b"abc\0"), (12, b"hello\0"))
    assert rows(report) == [("??_C@_03abc", "$SG11", "rebind"), ("$SG12", "$SG12", "valid")]


def test_stale_pooled_duplicates_without_a_layout_anchor_stay_unproven(project):
    path = source(project, literal(ABC1, ABC1 + 4, "??_C@_03abc"), literal(MISS, MISS + 5, "??_C@_04miss"))
    before = path.read_bytes()
    report = run(project, (10, b"abc\0"), (11, b"abc\0"), (12, b"x\0"), write=True)
    first, second = report["units"][0]["bindings"]
    assert first["status"] == "unproven" and "not separated by the layout" in first["reason"]
    assert first["candidates"] == ["$SG10", "$SG11"]
    assert second["status"] == "unproven" and "no fresh $SG label" in second["reason"]
    assert report["result"] == "unknown" and not report["persisted"] and path.read_bytes() == before


def test_stale_pooled_layout_refuses_a_displaced_duplicate(project):
    # An extra label between the copies breaks the displacement both anchors give.
    source(project, literal(ABC1, ABC1 + 4, "??_C@_03abc"), literal(HELLO, HELLO + 6, "??_C@_05hello"))
    report = run(project, (10, b"abc\0"), (11, b"pad\0"), (12, b"abc\0"), (13, b"wide\0\0\0\0"), (14, b"hello\0"))
    assert rows(report)[0][2] == "unproven" and rows(report)[1] == ("??_C@_05hello", "$SG14", "rebind")


def test_unit_selection_skips_other_tus_and_refuses_unknown_units(project):
    source(project, literal(ABC1, ABC1 + 4, "$SG100"))
    source(project, literal(HELLO, HELLO + 6, "$SG7"), name="g.cc")
    compiled = []

    def compiler(path, _includes, _flags):
        compiled.append(path.name)
        return obj((100, b"abc\0"), (7, b"hello\0"))
    report = rebind_literals(project, project / "original.exe", units=["code/g.cc"], compiler=compiler)
    assert [unit["unit"] for unit in report["units"]] == ["code/g.cc"] and compiled == ["g.cc"]
    report = rebind_literals(project, project / "original.exe", compiler=compiler)
    assert [unit["unit"] for unit in report["units"]] == ["code/f.cc", "code/g.cc"]
    with pytest.raises(ValueError, match="not configured build"):
        rebind_literals(project, project / "original.exe", units=["code/h.cc"], compiler=compiler)


def test_wrong_image_and_compile_failure_refuse(project):
    source(project, literal(ABC1, ABC1 + 4, "$SG100"))
    (project / "other.exe").write_bytes(image(RDATA[:-1] + b"!"))
    with pytest.raises(ValueError, match="pin"):
        rebind_literals(project, project / "other.exe", compiler=lambda *_: obj())

    def broken(*_):
        raise ValueError("compiler exploded")
    report = rebind_literals(project, project / "original.exe", compiler=broken, write=True)
    assert report["result"] == "unknown" and rows(report) == [("$SG100", "$SG100", "unproven")]


def test_cli_dry_run_reports_proposals(project, monkeypatch, capsys):
    source(project, literal(ABC1, ABC1 + 4, "$SG100"))
    monkeypatch.setattr(literal_rebind, "Compiler", lambda *_a, **_k: lambda *_: obj((5, b"x\0"), (6, b"abc\0")))
    code = main(["rebind-literals", "--root", str(project), "--image", str(project / "original.exe"),
                 "--unit", "code/f.cc", "--summary"])
    output = json.loads(capsys.readouterr().out)
    assert code == 1 and rows(output) == [("$SG100", "$SG6", "rebind")] and not output["persisted"]


PIN = "a" * 64


def test_symbol_rewrite_preserves_every_other_byte_and_refuses_stale_input():
    raw = ("x\r\n" + literal(ABC1, ABC1 + 4, "$SG1") + literal(ZZ, ZZ + 3, "$SG2", merged=True)).encode()
    parsed = parse_literals(raw.decode(), "code/f.cc", PIN, address_space="pe:va")
    result = rewrite_literal_symbols(raw, expected=parsed, changes={3: ("$SG2", "$SG9")},
                                     path="code/f.cc", pin=PIN, address_space="pe:va")
    assert result == raw.replace(b"$SG2", b"$SG9")
    for changes, expected in (({3: ("$SG3", "$SG9")}, parsed), ({1: ("$SG1", "$SG9")}, parsed),
                              ({2: ("$SG1", "$SG2")}, parsed),  # duplicate symbol in the TU
                              ({2: ("$SG1", "bad token")}, parsed), ({2: ("$SG1", "$SG9")}, parsed[:1])):
        with pytest.raises(ValueError):
            rewrite_literal_symbols(raw, expected=expected, changes=changes,
                                    path="code/f.cc", pin=PIN, address_space="pe:va")


def test_relocation_mismatch_on_a_static_label_hints_at_rebinding():
    row = {"result": "fail", "relocations": [{"result": "fail", "offset": 0x79, "symbol": "$SG66407"}]}
    assert "rebind-literals --unit" in strict_row_reason(row)
    row["relocations"][0]["symbol"] = "_g"
    assert "rebind-literals" not in strict_row_reason(row)


# Evidence-function relocations: the original f (0x401000) pushes ABC2 at body offset 1.
def code_image(target=ABC2, *, shared=False):
    """``shared``: g (0x401010) pushes the same literal at its offset 1 too."""
    data = bytearray(image())
    struct.pack_into("<II", data, 0x98 + 96 + 5 * 8, 0x1100, 12)  # base relocation directory
    data[512:517] = b"\x68" + struct.pack("<I", target)
    data[528:533] = b"\x68" + struct.pack("<I", target)
    struct.pack_into("<IIHH", data, 512 + 0x100, 0x1000, 12, 0x3001, 0x3011 if shared else 0)
    return bytes(data)


def code_obj(labels, references, *, other=()):
    """``labels`` as in ``obj``; .text holds ``?f@@YAXXZ`` then ``?g@@YAXXZ`` at 0x10.

    ``references`` / ``other`` are (body offset, label number) DIR32 sites in f / g.
    """
    data, values = b"", []
    for number, contents in labels:
        values.append((number, len(data)))
        data += contents + bytes(-len(contents) % 4)
    text = bytes(32)  # DIR32 addends are zero
    base = 20 + 2 * 40
    names = [".data", *(f"$SG{number}" for number, _ in labels), "?f@@YAXXZ", "?g@@YAXXZ"]
    strings, table = b"", {}
    for name in names:
        if len(name) > 8:
            table[name] = 4 + len(strings)
            strings += name.encode() + b"\0"

    def entry(name, value, section, storage, aux=0):
        raw = name.encode().ljust(8, b"\0") if name not in table else struct.pack("<II", 0, table[name])
        return raw + struct.pack("<IhHBB", value, section, 0x20 if section == 2 else 0, storage, aux)
    symbols = entry(".data", 0, 1, 3, 1) + struct.pack("<IHHIHB3s", len(data), 0, 0, 0, 0, 0, bytes(3))
    index = {}
    for number, value in values:
        index[number] = 2 + len(index)
        symbols += entry(f"$SG{number}", value, 1, 3)
    symbols += entry("?f@@YAXXZ", 0, 2, 2) + entry("?g@@YAXXZ", 0x10, 2, 2)
    sites = [(offset, number) for offset, number in references] + [(0x10 + offset, number) for offset, number in other]
    relocations = b"".join(struct.pack("<IIH", offset, index[number], 6) for offset, number in sites)
    text_at, relocations_at = base + len(data), base + len(data) + len(text)
    symbols_at = relocations_at + len(relocations)
    header = struct.pack("<HHIIIHH", 0x14C, 2, 0, symbols_at, 4 + len(values), 0, 0)
    sections = (b".data\0\0\0" + struct.pack("<IIIIIIHHI", 0, 0, len(data), base, 0, 0, 0, 0, 0xC0300040)
                + b".text\0\0\0" + struct.pack("<IIIIIIHHI", 0, 0, len(text), text_at, relocations_at, 0,
                                               len(sites), 0, 0x60500020))
    return (header + sections + data + text + relocations + symbols
            + struct.pack("<I", 4 + len(strings)) + strings)


@pytest.fixture
def code_project(project):
    raw = code_image()
    (project / "original.exe").write_bytes(raw)
    (project / "code/PIN").write_text("sha256 " + hashlib.sha256(raw).hexdigest() + "\n")
    return project


def context_source(project, symbol):
    path = project / "code/f.cc"
    path.write_bytes(("// ORIGINAL: 0x00401000 ?f@@YAXXZ 0x00401000-0x00401010\r\nvoid f() {\r\n"
                      + literal(ABC2, ABC2 + 4, symbol) + "}\r\n").encode())
    return path


def code_run(project, labels, references, write=False, **options):
    return rebind_literals(project, project / "original.exe", write=write,
                           compiler=lambda *_: code_obj(labels, references, **options))


ABCS = ((100, b"abc\0"), (101, b"abc\0"), (102, b"hello\0"))


def test_evidence_function_relocation_resolves_several_shifts(code_project):
    # Stale $SG104: shifts -4 and -3 both fit the duplicates; f's own site names $SG101.
    path = context_source(code_project, "$SG104")
    before = path.read_bytes()
    report = code_run(code_project, ABCS, [(1, 101)], other=[(1, 100)], write=True)
    row, = report["units"][0]["bindings"]
    assert (row["status"], row["symbol"]) == ("rebound", "$SG101")
    assert row["context"] == "$SG101 by context_relocation 0x00401000" and row["candidates"] == ["$SG100", "$SG101"]
    assert path.read_bytes() == before.replace(b"$SG104", b"$SG101")
    row, = code_run(code_project, ABCS, [(1, 101)], other=[(1, 100)])["units"][0]["bindings"]
    assert (row["status"], row["symbol"]) == ("valid", "$SG101")


def test_evidence_function_relocation_moves_a_content_valid_wrong_copy(code_project):
    # $SG100 holds the bytes, but f emits its literal as $SG101: shift 0 no longer wins.
    context_source(code_project, "$SG100")
    report = code_run(code_project, ABCS, [(1, 101)], other=[(1, 100)])
    assert rows(report) == [("$SG100", "$SG101", "rebind")]


def test_evidence_function_reference_away_from_the_site_still_decides(code_project):
    # f is not byte-exact (its push moved), but it references only one candidate.
    context_source(code_project, "$SG104")
    report = code_run(code_project, ABCS, [(5, 100)], other=[(1, 101)])
    row, = report["units"][0]["bindings"]
    assert (row["status"], row["symbol"], row["context"]) == ("rebind", "$SG100", "$SG100 by context_reference 0x00401000")


def test_evidence_function_that_cannot_separate_duplicates_is_refused(code_project):
    path = context_source(code_project, "$SG104")
    before = path.read_bytes()
    # f references both copies away from the original site: no choice.
    report = code_run(code_project, ABCS, [(5, 100), (9, 101)], write=True)
    row, = report["units"][0]["bindings"]
    assert row["status"] == "unproven" and "several uniform label shifts" in row["reason"]
    assert "?f@@YAXXZ relocations do not separate the candidates: $SG100, $SG101" == row["context"]
    assert report["result"] == "unknown" and not report["persisted"] and path.read_bytes() == before
    # A relocation at the very site to a label with different bytes is never a candidate.
    labels = ((100, b"abc\0"), (101, b"abc\0"), (102, b"abd\0"))
    row, = code_run(code_project, labels, [(1, 102)], write=True)["units"][0]["bindings"]
    assert row["status"] == "unproven" and row["candidates"] == ["$SG100", "$SG101"]
    assert "none referenced" in row["context"] and path.read_bytes() == before


def test_evidence_and_enclosing_functions_must_agree(code_project):
    # A shared original literal: the binding sits in g, its evidence is f, and
    # both original bodies push it, but each compiled body has its own copy.
    raw = code_image(shared=True)
    (code_project / "original.exe").write_bytes(raw)
    (code_project / "code/PIN").write_text("sha256 " + hashlib.sha256(raw).hexdigest() + "\n")
    path = code_project / "code/f.cc"
    path.write_bytes(("// ORIGINAL: 0x00401000 ?f@@YAXXZ 0x00401000-0x00401010\r\nvoid f() {\r\n}\r\n"
                      "// ORIGINAL: 0x00401010 ?g@@YAXXZ 0x00401010-0x00401020\r\nvoid g() {\r\n"
                      + literal(ABC2, ABC2 + 4, "$SG100") + "}\r\n").encode())
    report = code_run(code_project, ABCS, [(1, 101)], other=[(1, 100)])
    row, = report["units"][0]["bindings"]
    assert (row["status"], row["symbol"]) == ("valid", "$SG100"), "no narrowing: shift 0 still holds"
    assert row["context"].startswith("context functions disagree: ")
    # When both compiled bodies reference the same copy, they agree.
    row, = code_run(code_project, ABCS, [(1, 101)], other=[(1, 101)])["units"][0]["bindings"]
    assert (row["status"], row["symbol"]) == ("rebind", "$SG101")
    assert row["context"] == "$SG101 by context_relocation 0x00401000; context_relocation 0x00401010"
