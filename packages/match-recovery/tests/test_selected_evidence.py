"""Selected source records verify only their typed native evidence closure."""

import hashlib
import json
import struct

import pytest

from match_recovery.application_binary import verify_records
from match_recovery.generated_source import GeneratedBinding
from match_recovery.model import Origin, Result, Span
from match_recovery.object_source import ObjectBinding
from match_recovery.pe_coff import Coff
from match_recovery.shared_source import SharedFunctionBinding
from match_recovery.source import read_file
from test_binary import coff_fixture
from test_object_data import data_fixture


def image_fixture():
    raw = bytearray(2048)
    raw[:2] = b"MZ"
    struct.pack_into("<I", raw, 0x3C, 0x80)
    raw[0x80:0x84] = b"PE\0\0"
    struct.pack_into("<HHIIIHH", raw, 0x84, 0x14C, 3, 0, 0, 0, 224, 0)
    struct.pack_into("<H", raw, 0x98, 0x10B)
    struct.pack_into("<I", raw, 0x98 + 28, 0x400000)
    struct.pack_into("<I", raw, 0x98 + 92, 16)
    raw[0x178:0x180] = b".text\0\0\0"
    struct.pack_into("<IIII", raw, 0x180, 512, 0x1000, 512, 512)
    struct.pack_into("<I", raw, 0x178 + 36, 0x60000020)
    raw[0x1A0:0x1A8] = b".data\0\0\0"
    struct.pack_into("<IIII", raw, 0x1A8, 512, 0x2000, 512, 1024)
    struct.pack_into("<I", raw, 0x1A0 + 36, 0xC0000040)
    raw[0x1C8:0x1D0] = b".more\0\0\0"
    struct.pack_into("<IIII", raw, 0x1D0, 512, 0x3000, 512, 1536)
    struct.pack_into("<I", raw, 0x1C8 + 36, 0xC0000040)

    # Selected function loads _g; _g holds a pointer to _h. The second
    # original function anchors unrelated data evidence in the same project.
    raw[512:518] = b"\xA1\x00\x20\x40\x00\xC3"
    raw[528] = 0xC3
    struct.pack_into("<I", raw, 1024, 0x403000)
    struct.pack_into("<I", raw, 1028, 0x99)
    struct.pack_into("<I", raw, 1032, 0x55)
    struct.pack_into("<I", raw, 1536, 7)

    reloc = (struct.pack("<IIHH", 0x1000, 12, 0x3001, 0)
             + struct.pack("<IIHH", 0x2000, 12, 0x3000, 0))
    raw[768:768 + len(reloc)] = reloc
    struct.pack_into("<II", raw, 0x98 + 96 + 5 * 8, 0x1100, len(reloc))
    return bytes(raw)


def project(tmp_path):
    (tmp_path / "code").mkdir()
    (tmp_path / "data").mkdir()
    (tmp_path / "recovery").mkdir()
    image = image_fixture()
    pin = hashlib.sha256(image).hexdigest()
    (tmp_path / "image.exe").write_bytes(image)
    (tmp_path / "code/PIN").write_text("sha256 " + pin + "\n")
    (tmp_path / "code/caller.cc").write_text(
        "// ORIGINAL: 0x00401000 _f 0x00401000-0x00401006\n"
        "int f() { return 0; }\n"
        "// ORIGINAL: 0x00401010 _other 0x00401010-0x00401011\n"
        "int other() { return 0; }\n")
    (tmp_path / "data/global.cc").write_text(
        "// RECOVERY_OBJECT: v2 0x00402000 0x00402004 _g 0x00401000\n"
        "// RECOVERY_OBJECT: v2 0x00402004 0x00402008 _unused 0x00401010\n"
        "int g;\n")
    (tmp_path / "data/target.cc").write_text(
        "// RECOVERY_OBJECT: v2 0x00403000 0x00403004 _h 0x00401010\n"
        "int h;\n")
    (tmp_path / "data/unrelated.cc").write_text(
        "// RECOVERY_OBJECT: v2 0x00402008 0x0040200C _unrelated 0x00401010\n"
        "int unrelated;\n")
    (tmp_path / "recovery/project.json").write_text(json.dumps({
        "schema_version": 1,
        "source": {"image_pin": "code/PIN", "address_space": "pe:va",
                   "roots": ["code", "data"], "include_paths": ["code", "data"]},
        "measurement": {"backend": "pe-coff-i386", "compiler_profile": "synthetic",
                        "flag_sets": [["first"], ["second"]], "layout_flags": [],
                        "unit_extensions": [".cc"], "inputs": [], "input_roots": []},
    }))
    return tmp_path


def function_object(*, symbol="_g", opcode=0xA1):
    code = bytes((opcode, 0, 0, 0, 0, 0xC3))
    raw = bytearray(coff_fixture(code, relocation=(1, 1, 6)))
    symtab, = struct.unpack_from("<I", raw, 8)
    raw[symtab + 18:symtab + 26] = symbol.encode().ljust(8, b"\0")
    return bytes(raw)


def verify(root, compiler, *, records=None, flag_sets=(("first",),)):
    records = records or read_file(root / "code/caller.cc")[:1]
    return verify_records(root, records, root / "image.exe", flag_sets=flag_sets,
                          compiler=compiler, command_for=lambda _path: ())


def good_compiler(calls):
    def compile(path, command, flags):
        calls.append((path.relative_to(path.parents[1]).as_posix(), flags))
        if path.name == "caller.cc":
            return function_object()
        if path.name == "global.cc":
            return data_fixture(struct.pack("<I", 0), type_index=0x403,
                                relocations=((0, 1, 6),), definition=b"_g", reference=b"_h")
        if path.name == "target.cc":
            return data_fixture(struct.pack("<I", 7), definition=b"_h", reference=b"_unused")
        if path.name == "unrelated.cc":
            return data_fixture(struct.pack("<I", 9), definition=b"_unrel", reference=b"_other")
        raise AssertionError(f"unrelated or unexpected owner compiled: {path}")
    return compile


def test_selected_code_follows_transitive_pointer_target_but_ignores_unrelated_data(tmp_path):
    root = project(tmp_path)
    calls = []
    report = verify(root, good_compiler(calls))
    assert report["result"] == Result.PASS, report
    assert calls == [("code/caller.cc", ("first",)),
                     ("data/global.cc", ("first",)),
                     ("data/target.cc", ("first",))]
    assert {row["symbol"] for row in report["object_definitions"]} == {"_g", "_h"}
    assert report["compile_attempts"] == 3
    assert report["selected_evidence"]["trials"][0]["result"] == Result.PASS


def _unbound_marker(address, symbol, end):
    return f"// ORIGINAL: 0x{address:08X} {symbol} 0x{address:08X}-0x{end:08X} UNBOUND"


def _assert_unknown_coverage(report):
    assert report["result"] == Result.UNKNOWN, report
    assert report["selected_evidence"]["trials"]
    assert all(row["result"] == Result.UNKNOWN
               for row in report["selected_evidence"]["trials"]), report
    assert any("selected source identity" in row.get("reason", "")
               or "unfinished source placeholder" in row.get("reason", "")
               or "configured translation unit" in row.get("reason", "")
               or "could not be extracted" in row.get("reason", "")
               or "unbound original identity" in row.get("reason", "")
               for row in report["selected_evidence"]["checks"]), report


def test_all_unbound_selected_identities_are_unknown_without_compiling(tmp_path):
    root = project(tmp_path)
    path = root / "code/caller.cc"
    path.write_text(
        _unbound_marker(0x401000, "_f", 0x401006) + "\n"
        + _unbound_marker(0x401010, "_other", 0x401011) + "\n")
    records = read_file(path)
    calls = []
    report = verify(root, good_compiler(calls), records=records)
    _assert_unknown_coverage(report)
    assert not any(path == "code/caller.cc" for path, _flags in calls)
    assert report["selected_entities"] == 2


def test_all_placeholders_are_unknown_without_compiling(tmp_path):
    root = project(tmp_path)
    path = root / "code/caller.cc"
    path.write_text(path.read_text().replace("int f() { return 0; }",
                                              "int f() {\n// BODY GOES HERE.\n}"))
    calls = []
    report = verify(root, good_compiler(calls))
    _assert_unknown_coverage(report)
    assert not any(path == "code/caller.cc" for path, _flags in calls)


def test_unsupported_source_owner_is_unknown_without_compiling(tmp_path):
    root = project(tmp_path)
    path = root / "code/unsupported.cpp"
    path.write_text("// ORIGINAL: 0x00401020 _unsupported 0x00401020-0x00401021\n"
                    "int unsupported() { return 0; }\n")
    calls = []
    report = verify(root, good_compiler(calls), records=read_file(path))
    _assert_unknown_coverage(report)
    assert calls == []


def test_missing_extractable_body_is_unknown_without_compiling(tmp_path):
    root = project(tmp_path)
    path = root / "code/caller.cc"
    path.write_text("// ORIGINAL: 0x00401000 _f 0x00401000-0x00401006\n")
    records = read_file(path)
    assert records[0].extract_error
    calls = []
    report = verify(root, good_compiler(calls), records=records)
    _assert_unknown_coverage(report)
    assert not any(path == "code/caller.cc" for path, _flags in calls)


def test_mixed_valid_and_refused_selection_cannot_aggregate_to_pass(tmp_path):
    root = project(tmp_path)
    path = root / "code/caller.cc"
    path.write_text(path.read_text().replace(
        "// ORIGINAL: 0x00401010 _other 0x00401010-0x00401011",
        _unbound_marker(0x401010, "_other", 0x401011)))
    calls = []
    report = verify(root, good_compiler(calls), records=read_file(path))
    _assert_unknown_coverage(report)
    assert any(path == "code/caller.cc" for path, _flags in calls)
    assert report["selected_entities"] == 2


@pytest.mark.parametrize("damage,expected", [("bytes", Result.FAIL),
                                              ("unknown_target", Result.UNKNOWN)])
def test_referenced_data_damage_or_unresolved_transitive_target_is_not_hidden(tmp_path, damage, expected):
    root = project(tmp_path)
    calls = []

    def compile(path, command, flags):
        calls.append(path.name)
        if path.name == "caller.cc":
            return function_object()
        if path.name == "global.cc":
            value = 4 if damage == "bytes" else 0
            return data_fixture(struct.pack("<I", value), type_index=0x403,
                                relocations=((0, 1, 6),), definition=b"_g", reference=b"_h")
        if path.name == "target.cc":
            return data_fixture(struct.pack("<I", 7), definition=(b"_missing" if damage == "unknown_target" else b"_h"),
                                reference=b"_unused")
        raise AssertionError(path)

    report = verify(root, compile)
    assert report["result"] == expected, report
    assert "target.cc" in calls


def test_ambiguous_object_owner_is_refused_before_any_partial_pass(tmp_path):
    root = project(tmp_path)
    (root / "data/other.cc").write_text(
        "// RECOVERY_OBJECT: v2 0x00403004 0x00403008 _h 0x00401010\nint other_h;\n")
    with pytest.raises(ValueError, match="duplicate original object symbol|current object owner|multiple original"):
        verify(root, good_compiler([]))


def test_unbounded_referenced_object_cannot_bank_a_relocation_only_proof(tmp_path):
    root = project(tmp_path)
    path = root / "data/global.cc"
    path.write_text(path.read_text().replace(
        "RECOVERY_OBJECT: v2 0x00402000 0x00402004 _g 0x00401000",
        "RECOVERY_OBJECT: v1 0x00402000 _g"))
    report = verify(root, good_compiler([]))
    assert report["result"] == Result.UNKNOWN, report
    assert any("no independently bounded original extent" in row.get("reason", "")
               for row in report["selected_evidence"]["checks"])


def test_unknown_coff_data_extent_cannot_hide_a_selected_dependency(tmp_path):
    root = project(tmp_path)

    def compile(path, _command, _flags):
        if path.name == "caller.cc":
            return function_object()
        if path.name == "global.cc":
            return data_fixture(struct.pack("<I", 0), type_index=0xFFFF,
                                relocations=((0, 1, 6),), definition=b"_g", reference=b"_h")
        return data_fixture(struct.pack("<I", 7), definition=b"_h", reference=b"_unused")

    report = verify(root, compile)
    assert report["result"] == Result.UNKNOWN, report


def test_unknown_code_reference_stays_unknown_even_with_other_valid_evidence(tmp_path):
    root = project(tmp_path)
    report = verify(root, lambda path, _command, _flags:
                    function_object(symbol="_missing") if path.name == "caller.cc"
                    else good_compiler([])(path, (), ("first",)))
    assert report["result"] == Result.UNKNOWN, report
    assert report["functions"][0]["result"] == Result.UNKNOWN


def test_distinct_flags_cannot_combine_body_and_transitive_data_proofs(tmp_path):
    root = project(tmp_path)

    def compile(path, _command, flags):
        if path.name == "caller.cc":
            return function_object(opcode=0xA1 if flags == ("first",) else 0xA3)
        if path.name == "global.cc":
            value = 4 if flags == ("first",) else 0
            return data_fixture(struct.pack("<I", value), type_index=0x403,
                                relocations=((0, 1, 6),), definition=b"_g", reference=b"_h")
        return data_fixture(struct.pack("<I", 7), definition=b"_h", reference=b"_unused")

    report = verify(root, compile, flag_sets=(("first",), ("second",)))
    assert report["result"] == Result.FAIL, report
    assert [row["result"] for row in report["selected_evidence"]["trials"]] == [Result.FAIL, Result.FAIL]
    assert all(row["result"] != Result.PASS for row in report["selected_evidence"]["trials"])


def test_selected_extent_rejects_relocation_crossing_its_boundary():
    from match_recovery.selected_evidence import _relocation_names

    class Section:
        relocations = ((2, 1, 6),)

    class Object:
        sections = (Section(),)
        symbols = {1: type("Symbol", (), {"name": "_edge"})()}

    with pytest.raises(ValueError, match="cuts through"):
        _relocation_names("data/owner.cc", Object(), (1,), span=(0, 4))


def test_selected_generated_and_shared_references_schedule_only_their_typed_owners():
    from match_recovery.bound_objects import FunctionBinding
    from match_recovery.selected_evidence import select_evidence

    pin = "a" * 64
    origin = Origin(pin, "pe:va", 0x401000)
    function = FunctionBinding(origin, (Span(0x401000, 0x401006),), "_f", "code/caller.cc")
    shared_origin = Origin(pin, "pe:va", 0x401100)
    shared = SharedFunctionBinding(shared_origin, Span(0x401100, 0x401106), "_shared",
                                   origin, "data/shared.cc", 1)
    generated = GeneratedBinding(Origin(pin, "pe:va", 0x402000), None,
                                 "??_7Widget@@6B@", origin, "data/widget.cc", 1)

    class Symbol:
        def __init__(self, name, section=1, storage=2):
            self.name, self.section, self.storage = name, section, storage

    class Section:
        relocations = ((1, 2, 6), (2, 3, 6))

    class Object:
        sections = (Section(),)
        symbols = {1: Symbol("_f"), 2: Symbol("_shared", 0),
                   3: Symbol("??_7Widget@@6B@", 0)}

        def function(self, name):
            assert name == "_f"
            return self.symbols[1]

        @staticmethod
        def code_sections(_symbol):
            return (1,)

    selection = select_evidence((function,), {origin.entity_id},
                                {"code/caller.cc": Object()}, (), (generated,), (),
                                (shared,), ())
    assert selection.units == {"code/caller.cc", "data/shared.cc", "data/widget.cc"}
    assert selection.shared == (shared,)
    assert selection.generated == (generated,)
