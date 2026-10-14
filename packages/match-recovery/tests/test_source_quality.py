import json

from match_recovery.cli import main
from match_recovery.source_quality import _findings, quality_report


def project(tmp_path, body):
    (tmp_path / "recovery").mkdir()
    (tmp_path / "src").mkdir()
    (tmp_path / "original").mkdir()
    (tmp_path / "original/PIN").write_text("sha256 " + "a" * 64 + "\n")
    (tmp_path / "recovery/project.json").write_text(json.dumps({"schema_version": 1, "source": {
        "image_pin": "original/PIN", "address_space": "test:va", "roots": ["src"]}}))
    (tmp_path / "src/functions.cpp").write_text(body)
    return tmp_path


def test_lexer_ignores_comments_and_literals_but_reports_c_and_cpp_casts():
    findings = _findings('''void f() {
      // reinterpret_cast<Foo*>(0x00401234)
      const char* s = "(Foo*)0x00401234";
      const char* raw = R"tag(
        (LPBYTE)0x00401234 reinterpret_cast<Foo*>(value)
      )tag";
      const char* raw2 = R"(
        (int)pointer 0x00401234
      )";
      foo(this);
      auto p = reinterpret_cast<Foo*>(value);
      auto q = (Foo*)value;
      auto i = (int)pointer;
      auto t = (LPBYTE)buffer;
      auto lower = (byte_t*)buffer;
    }
''', path="unit.cpp", first_line=40)
    assert [item["kind"] for item in findings] == ["cast or reinterpretation boundary"] * 4 + [
        "ambiguous C-style cast or parenthesized expression"]
    assert all(item["path"] == "unit.cpp" and item["line"] >= 40 for item in findings)
    assert all("foo(this)" not in item["text"] for item in findings)


def test_scans_each_body_without_leaking_findings_to_neighbor():
    source = '''// ORIGINAL: 0x00401000 _a 0x00401000-0x00401010 BYTE_EXACT
void a() { auto p = reinterpret_cast<int*>(0x00401000); }
// ORIGINAL: 0x00401010 _b 0x00401010-0x00401020
void b() { return; }
'''
    from pathlib import Path
    import tempfile
    with tempfile.TemporaryDirectory() as directory:
        report = quality_report(project(Path(directory), source))
    a = next(row for row in report["functions"] if "_a" in row["catalogued_symbols"])
    b = next(row for row in report["functions"] if "_b" in row["catalogued_symbols"])
    assert a["status"] == "needs_investigation", report
    assert a["historical_byte_claim"] and a["current_exactness"] == "not_measured"
    assert a["findings"][0]["line"] == 2
    assert a["findings"][0]["path"] == "src/functions.cpp"
    assert b["status"] == "unreviewed" and not b["findings"]
    assert b["scan_observation"] == "no_patterns_found"
    assert report["source_revision"] is None or len(report["source_revision"]) == 40
    assert len(report["image_sha256"]) == 64 and len(report["source_fingerprint"]) == 64
    assert report["historical_byte_claims_by_status"]["needs_investigation"] == {"claimed": 1, "unclaimed": 0}
    assert report["historical_byte_claims_by_status"]["unreviewed"] == {"claimed": 0, "unclaimed": 1}


def test_native_placeholder_classification_and_source_line(tmp_path):
    root = project(tmp_path, '''// ORIGINAL: 0x00401000 _p 0x00401000-0x00401010
void p() {
// BODY GOES HERE.
return PLACEHOLDER;
}
''')
    row = quality_report(root)["functions"][0]
    assert row["status"] == "reconstruction_debt"
    finding, = [item for item in row["findings"] if item["kind"] == "native placeholder classification"]
    assert finding["path"] == "src/functions.cpp" and finding["line"] == 3


def test_file_mapping_resolves_its_function_but_never_scans_neighbor(tmp_path):
    root = project(tmp_path, '''// ORIGINAL: 0x00401000 _authored 0x00401000-0x00401010 FILE
void authored() { auto p = reinterpret_cast<int*>(value); }
// ORIGINAL: 0x00401010 _$E26 0x00401010-0x00401020 FILE
void neighboring() { auto p = reinterpret_cast<int*>(value); }
''')
    rows = quality_report(root)["functions"]
    authored = next(row for row in rows if "_authored" in row["catalogued_symbols"])
    generated = next(row for row in rows if "_$E26" in row["catalogued_symbols"])
    assert authored["status"] == "needs_investigation"
    assert authored["body_resolution"] == "unique symbol-resolved definition"
    assert authored["findings"][0]["line"] == 2
    assert generated["status"] == "unreviewed" and not generated["findings"]


def test_msvc_adjustor_file_does_not_inherit_override_body(tmp_path):
    root = project(tmp_path, '''// ORIGINAL: 0x00401000 ?on_left_click@thunk1_MapWin@@QAEXHH@Z 0x00401000-0x00401008 FILE
// symbol    ?on_left_click@MapWin@@$4PPPPPPPM@A@AEXHH@Z
// kind game
void MapWin::on_left_click(int, int) { auto p = reinterpret_cast<int*>(value); }
''')
    # Exercise the MSVC name reader used by real projects.
    config = json.loads((root / "recovery/project.json").read_text())
    config["source"]["symbol_format"] = "msvc"
    (root / "recovery/project.json").write_text(json.dumps(config))
    row, = quality_report(root)["functions"]
    assert row["status"] == "unreviewed" and row["findings"] == []
    assert "adjustor" in row["body_resolution"]
    assert row["scan_observation"] == "unresolved"


def test_file_mapping_unreviewed_duplicate_identity_and_cli_summary(tmp_path, capsys):
    source = '''// ORIGINAL: 0x00401000 _a 0x00401000-0x00401010 FILE
void a() { auto p = reinterpret_cast<int*>(0x00401000); }
// ORIGINAL: 0x00401000 _a 0x00401000-0x00401010 FILE
void b() {}
'''
    root = project(tmp_path, source)
    report = quality_report(root)
    assert report["identity_count"] == 1
    assert report["functions"][0]["status"] == "unreviewed"
    assert report["functions"][0]["scan_observation"] == "unresolved"
    assert main(["quality", "--root", str(root), "--summary"]) == 0
    summary = json.loads(capsys.readouterr().out)
    assert summary["result"] == "advisory"
    assert "findings" not in summary["functions"][0]
    assert summary["functions"][0]["current_exactness"] == "not_measured"
