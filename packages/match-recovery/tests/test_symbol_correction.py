"""Emitted-symbol corrections keep original identity; synthetic COFF only."""

import hashlib
import json
import struct
import subprocess

import pytest

from match_recovery.cli import main
from match_recovery.source import read_text
from match_recovery.source.mangled import signature_differences, signature_parts
from match_recovery.symbol_correction import correct_symbol

OLD = "?click@Win@@QAEHHH@Z"
NEW = "?click@Win@@QAEXHH@Z"


def coff(*names):
    """One executable section defining each name as a function at offset 0."""
    raw = b"\xc3" + b"\xcc" * 15
    strings, entries = b"", b""
    for name in names:
        encoded = name.encode()
        if len(encoded) <= 8:
            field = encoded.ljust(8, b"\0")
        else:
            field = struct.pack("<II", 0, 4 + len(strings))
            strings += encoded + b"\0"
        entries += field + struct.pack("<IhHBB", 0, 1, 0x20, 2, 0)
    header = struct.pack("<HHIIIHH", 0x14C, 1, 0, 60 + len(raw), len(names), 0, 0)
    section = b".text\0\0\0" + struct.pack("<IIIIIIHHI", 0, 0, len(raw), 60, 0, 0, 0, 0, 0x60501020)
    return header + section + raw + entries + struct.pack("<I", 4 + len(strings)) + strings


SOURCE = ("// ORIGINAL: 0x00401000 {old} 0x00401000-0x00401010 BYTE_EXACT\n"
          "// RECOVERY_BINDING: v1 {{\"reason\":\"fixture\",\"evidence\":[\"test\"]}}\n"
          "// TRIED: earlier lesson that must survive\n"
          "int Win::click(int a, int b) {{ return a; }}\n"
          "// ORIGINAL: 0x00401010 _other 0x00401010-0x00401020\n"
          "void other() {{}}\n")


@pytest.fixture
def project(tmp_path):
    for name in ("code", "recovery", "build"):
        (tmp_path / name).mkdir()
    (tmp_path / "code/PIN").write_text("sha256 " + "a" * 64 + "\n")
    (tmp_path / "code/f.cc").write_text(SOURCE.format(old=OLD))
    (tmp_path / "build/compile_commands.json").write_text(json.dumps([
        {"file": "code/f.cc", "directory": str(tmp_path), "arguments": ["fake", "code/f.cc"]}]))
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


def digest(project):
    return hashlib.sha256((project / "code/f.cc").read_bytes()).hexdigest()


def run(project, symbol=NEW, emitted=(NEW, "_other"), **extra):
    return correct_symbol(project, address=0x401000, source="code/f.cc", symbol=symbol,
                          source_sha256=digest(project), reason="image never sets EAX; caller ignores it",
                          compiler=lambda *_: coff(*emitted), **extra)


def test_correction_writes_only_the_symbol_fact_and_keeps_identity_and_claims(project):
    path = project / "code/f.cc"
    before = read_text(path.read_text(), path)
    report = run(project)
    text = path.read_text()
    lines = text.splitlines()
    assert lines[0] == f"// ORIGINAL: 0x00401000 {OLD} 0x00401000-0x00401010 BYTE_EXACT"
    assert lines[1].startswith("// RECOVERY_BINDING:") and lines[2] == f"// symbol    {NEW}"
    after = read_text(text, path)
    assert [(r.address, r.name, r.image_spans, r.byte_exact, r.semantic) for r in after] == \
        [(r.address, r.name, r.image_spans, r.byte_exact, r.semantic) for r in before]
    assert after[0].symbol == NEW and "earlier lesson" in text
    assert report["previous_symbol"] == OLD and report["original_name"] == OLD
    assert report["differences"] == {"readable": True, "parts": ["returns"], "identity": []}
    assert report["historical_claims"] == {"byte_exact": True, "instruction_shape": False}
    assert report["verification"] == "emission_only" and not report["ready_for_collection"]
    # A second correction rewrites the existing fact in place.
    third = "?click@Win@@SAXHH@Z"
    run(project, symbol=third, emitted=(third,))
    assert path.read_text().splitlines()[2] == f"// symbol    {third}"
    assert read_text(path.read_text(), path)[0].byte_exact


@pytest.mark.parametrize("symbol,flag", [
    ("?press@Win@@QAEXHH@Z", "qualified"),       # a different function name
    ("?click@Win@@QAEXH@Z", "parameters"),       # an argument removed
    ("?click@Win@@UAEXHH@Z", "kind"),            # virtual is not storage
    ("?click@Win@@AAEXHH@Z", "access"),
    ("?click@Win@@QBEXHH@Z", "this_cv"),
])
def test_identity_changes_need_the_explicit_flag(project, symbol, flag):
    before = (project / "code/f.cc").read_bytes()
    with pytest.raises(ValueError, match=flag):
        run(project, symbol=symbol, emitted=(symbol,))
    assert (project / "code/f.cc").read_bytes() == before
    report = run(project, symbol=symbol, emitted=(symbol,), allow_identity_change=True)
    assert flag in report["differences"]["identity"] and report["identity_change_allowed"]


@pytest.mark.parametrize("emitted", [(), (OLD,), (OLD, NEW), ("?click@Win@@QAEXHHH@Z",)])
def test_correction_requires_fresh_emission_of_only_the_new_symbol(project, emitted):
    before = (project / "code/f.cc").read_bytes()
    with pytest.raises(ValueError, match="no configured compilation"):
        run(project, emitted=emitted)
    assert (project / "code/f.cc").read_bytes() == before


@pytest.mark.parametrize("damage", ["stale", "address", "unchanged", "collision", "reason", "unbound"])
def test_refusals_leave_source_untouched(project, damage):
    path = project / "code/f.cc"
    if damage == "unbound":
        path.write_text("// ORIGINAL: 0x00401000 sub_401000 0x00401000-0x00401010 UNBOUND\n")
    before = path.read_bytes()
    arguments = dict(address=0x401000, source="code/f.cc", symbol=NEW, source_sha256=digest(project),
                     reason="evidence", compiler=lambda *_: coff(NEW))
    if damage == "stale":
        arguments["source_sha256"] = "b" * 64
    elif damage == "address":
        arguments["address"] = 0x401004
    elif damage == "unchanged":
        arguments["symbol"] = OLD
    elif damage == "collision":
        arguments["symbol"] = "_other"
        arguments["allow_identity_change"] = True
    elif damage == "reason":
        arguments["reason"] = "two\nlines"
    with pytest.raises(ValueError):
        correct_symbol(project, **arguments)
    assert path.read_bytes() == before


def test_cli_reports_refusal_without_writing(project, capsys):
    before = (project / "code/f.cc").read_bytes()
    code = main(["correct-symbol", "0x401000", "--root", str(project), "--in", "code/f.cc",
                 "--to", "?press@Win@@QAEXHH@Z", "--source-sha256", digest(project), "--reason", "r"])
    assert code == 3 and "allow-identity-change" in capsys.readouterr().err
    assert (project / "code/f.cc").read_bytes() == before


def test_signature_parts_read_storage_convention_and_return():
    assert signature_parts("?update_screen@Win@@SAXPAUtagRECT@@PAV1@@Z") == {
        "form": "c++", "qualified": "Win::update_screen", "access": "public", "kind": "static",
        "this_cv": None, "convention": "__cdecl", "returns": "X", "return_neutral": True,
        "parameters": "PAUtagRECT@@PAV1@@Z"}
    # Nonstatic thiscall to static stdcall is a storage/convention correction.
    assert signature_differences("?f@Win@@QAGXXZ", "?f@Win@@SAXXZ")["identity"] == []
    # A written-out class name in a return enters the name table: arguments
    # spelled identically could name different types, so this is refused.
    assert "parameters" in signature_differences("?f@A@@QAEPAVB@@PAV2@@Z", "?f@A@@QAEXPAV2@@Z")["identity"]
    assert signature_differences("_f", "_f@8")["identity"] == []
    assert signature_differences("_f@4", "_f@8")["identity"] == ["parameters"]
    for unreadable in ("?f@?$T@H@@QAEXXZ", "?f@A@@QBXXZ", "", "?f@A@@ZAEXXZ"):
        assert signature_parts(unreadable) is None
    assert signature_differences("?f@A@@QAEXXZ", "?f@?$T@H@@QAEXXZ")["identity"] == ["unreadable decoration"]
