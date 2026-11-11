"""Span corrections keep identity and claims and never orphan original bytes."""

import hashlib
import json
import subprocess
import sys

import pytest

from match_recovery.cli import main
from match_recovery.model import Span
from match_recovery.project import initialize
from match_recovery.source import read_text
from match_recovery.span_correction import correct_spans, parse_spans
from test_binary import pe_fixture
from test_image_survey import executable
from test_symbol_correction import coff

STUBS = ("// ORIGINAL: 0x00401000 sub_401000 0x00401000-0x00401010;0x00401100-0x00401110\n"
         "// RECOVERY_BINDING: v1 {\"evidence\": [\"fixture\"], \"reason\": \"fixture\"}\n"
         "// TRIED: earlier lesson that must survive\n"
         "void stub_a() {}\n"
         "// ORIGINAL: 0x00401010 sub_401010 0x00401010-0x00401020;0x00401100-0x00401110CLAIM\n"
         "void stub_b() {}\n")
SHARED = "// The shared routine.\nvoid shared() {}\n"
ASM = "; The shared routine.\n_shared PROC PUBLIC\n          ret\n_shared ENDP\n          END\n"
REASON = "Each stub ends in its own jmp to 0x401100; the seed attached the target as a chunk of both."
EVIDENCE = ["test-owned original fixture"]


@pytest.fixture
def project(tmp_path):
    image = tmp_path / "original.exe"
    image.write_bytes(executable(pe_fixture(b"\xc3" * 0x120)))
    initialize(tmp_path, "span-control", sys.executable, image=image)
    config_path = tmp_path / "recovery/project.json"
    config = json.loads(config_path.read_text())
    config["source"]["extensions"] = [".cpp", ".h", ".asm"]
    config["measurement"]["unit_extensions"] = [".cpp", ".asm"]
    config_path.write_text(json.dumps(config))
    (tmp_path / "src/stubs.cpp").write_text(STUBS.replace("CLAIM", ""))
    (tmp_path / "src/shared.cpp").write_text(SHARED)
    (tmp_path / "src/shared.asm").write_text(ASM)
    (tmp_path / ".gitignore").write_text("build/\noriginal.exe\n")
    for args in (("init",), ("config", "user.name", "Span fixture"), ("config", "user.email", "fixture@example.invalid"),
                 ("add", "."), ("commit", "-m", "fixture")):
        subprocess.run(["git", *args], cwd=tmp_path, check=True, capture_output=True)
    return tmp_path


def digest(project, name):
    return hashlib.sha256((project / name).read_bytes()).hexdigest()


def sources(project):
    return {path: path.read_bytes() for path in (project / "src").rglob("*") if path.is_file()}


def assign(project, host="src/shared.cpp", line=2, **extra):
    return {"address": 0x401100, "name": "sub_401100", "spans": [Span(0x401100, 0x401110)],
            "source": host, "line": line, "source_sha256": digest(project, host), **extra}


def run(project, corrections=None, **extra):
    corrections = corrections or {0x401000: [Span(0x401000, 0x401010)], 0x401010: [Span(0x401010, 0x401020)]}
    return correct_spans(project, image=project / "original.exe", source="src/stubs.cpp",
                         source_sha256=digest(project, "src/stubs.cpp"), corrections=corrections,
                         reason=REASON, evidence=EVIDENCE, **extra)


def test_shared_chunk_moves_to_its_own_mapping_and_only_markers_change(project):
    path = project / "src/stubs.cpp"
    before_lines = path.read_text().splitlines()
    before = read_text(path.read_text(), path)
    report = run(project, assign=assign(project))
    after_lines = path.read_text().splitlines()
    changed = [index for index, (left, right) in enumerate(zip(before_lines, after_lines)) if left != right]
    assert len(before_lines) == len(after_lines) and changed == [0, 4]
    after = read_text(path.read_text(), path)
    assert [row.image_spans for row in after] == [((0x401000, 0x401010),), ((0x401010, 0x401020),)]
    assert [(row.address, row.name, row.byte_exact, row.semantic, row.ruled_out) for row in after] == \
        [(row.address, row.name, row.byte_exact, row.semantic, row.ruled_out) for row in before]
    shared, = read_text((project / "src/shared.cpp").read_text(), project / "src/shared.cpp")
    assert shared.address == 0x401100 and shared.image_spans == ((0x401100, 0x401110),)
    assert not shared.byte_exact and not shared.semantic
    assert all(row["removed_owned_by"] == ["src/shared.cpp:0x00401100"] for row in report["corrections"])
    assert report["assignment"]["binding_added"] and not report["ready_for_collection"]


def test_removed_span_still_listed_by_another_mapping_is_owned(project):
    run(project, corrections={0x401000: [Span(0x401000, 0x401010)]})
    first, second = read_text((project / "src/stubs.cpp").read_text(), project / "src/stubs.cpp")
    assert first.image_spans == ((0x401000, 0x401010),)
    assert second.image_spans == ((0x401010, 0x401020), (0x401100, 0x401110))


def test_assembler_assignment_uses_semicolon_comments_and_checks_emission(project):
    host = "src/shared.asm"
    report = run(project, assign=assign(project, host=host, symbol="_shared"),
                 compiler=lambda *_: coff("_shared"))
    text = (project / host).read_text()
    lines = text.splitlines()
    assert lines[1].startswith("; ORIGINAL: 0x00401100 sub_401100 0x00401100-0x00401110")
    assert lines[2].startswith("; RECOVERY_BINDING: v1 ") and lines[3] == "; symbol    _shared"
    assert lines[4] == "_shared PROC PUBLIC"
    record, = read_text(text, project / host)
    assert record.symbol == "_shared" and record.image_spans == ((0x401100, 0x401110),)
    assert report["assignment"]["emission"]["trials"][-1]["result"] == "pass"


REFUSALS = {"unowned": "unowned", "primary": "primary span must begin", "reordered": "primary span must begin",
            "overlap": "overlaps another owner", "stale": "digest is stale", "unchanged": "already lists",
            "missing": "expected one mapping", "unbacked": "backed", "claimed": "historical BYTE_EXACT",
            "claimed_primary": "only drop whole secondary spans", "emission": "defines _shared",
            "assign_overlap": "overlap another binding"}


@pytest.mark.parametrize("damage", sorted(REFUSALS))
def test_refusals_do_not_change_source(project, damage):
    corrections, extra = None, {}
    if damage == "unowned":
        corrections = {0x401000: [Span(0x401000, 0x401010)], 0x401010: [Span(0x401010, 0x401020)]}
    elif damage == "primary":
        corrections = {0x401000: [Span(0x401002, 0x401010)]}
    elif damage == "reordered":
        corrections = {0x401000: [Span(0x401100, 0x401110), Span(0x401000, 0x401010)]}
    elif damage == "overlap":
        corrections = {0x401000: [Span(0x401000, 0x401018), Span(0x401100, 0x401110)]}
    elif damage == "unchanged":
        corrections = {0x401000: [Span(0x401000, 0x401010), Span(0x401100, 0x401110)]}
    elif damage == "missing":
        corrections = {0x401200: [Span(0x401200, 0x401210)]}
    elif damage == "unbacked":
        corrections = {0x401000: [Span(0x401000, 0x401010), Span(0x401100, 0x401110), Span(0x500000, 0x500010)]}
    elif damage in ("claimed", "claimed_primary"):
        (project / "src/stubs.cpp").write_text(STUBS.replace("CLAIM", " BYTE_EXACT"))
        extra["assign"] = assign(project)
        if damage == "claimed_primary":
            extra["allow_claimed"] = True
            corrections = {0x401010: [Span(0x401010, 0x401018)]}
        else:
            corrections = {0x401010: [Span(0x401010, 0x401020)]}
    elif damage == "emission":
        extra = {"assign": assign(project, host="src/shared.asm", symbol="_shared"),
                 "compiler": lambda *_: coff("_other")}
    elif damage == "assign_overlap":
        extra["assign"] = {**assign(project), "spans": [Span(0x401100, 0x401118)]}
        (project / "src/other.cpp").write_text("// ORIGINAL: 0x00401114 sub_401114 0x00401114-0x00401118\n"
                                               "void other() {}\n")
    before = sources(project)
    with pytest.raises(ValueError, match=REFUSALS[damage]):
        if damage == "stale":
            correct_spans(project, image=project / "original.exe", source="src/stubs.cpp", source_sha256="a" * 64,
                          corrections={0x401000: [Span(0x401000, 0x401010)]}, reason=REASON, evidence=EVIDENCE)
        else:
            run(project, corrections=corrections, **extra)
    assert sources(project) == before


def test_claimed_mapping_may_drop_an_owned_secondary_span_only_when_allowed(project):
    (project / "src/stubs.cpp").write_text(STUBS.replace("CLAIM", " BYTE_EXACT"))
    run(project, corrections={0x401010: [Span(0x401010, 0x401020)]}, allow_claimed=True)
    _, second = read_text((project / "src/stubs.cpp").read_text(), project / "src/stubs.cpp")
    assert second.byte_exact and second.image_spans == ((0x401010, 0x401020),)


def test_parse_spans_accepts_semicolon_or_comma_lists():
    assert parse_spans("0x10-0x20;0x30-0x40") == parse_spans("0x10-0x20,0x30-0x40") == (Span(0x10, 0x20), Span(0x30, 0x40))
    with pytest.raises(ValueError):
        parse_spans("0x10")


def test_cli_correction_with_assignment(project, capsys):
    code = main(["correct-spans", "--root", str(project), "--image", "original.exe", "--in", "src/stubs.cpp",
                 "--set", "0x401000=0x401000-0x401010", "--set", "0x401010=0x401010-0x401020",
                 "--source-sha256", digest(project, "src/stubs.cpp"), "--reason", REASON, "--evidence", EVIDENCE[0],
                 "--assign-address", "0x401100", "--assign-in", "src/shared.cpp", "--assign-line", "2",
                 "--assign-name", "sub_401100", "--assign-span", "0x401100-0x401110",
                 "--assign-sha256", digest(project, "src/shared.cpp")])
    assert code == 0
    report = json.loads(capsys.readouterr().out)
    assert [row["spans"] for row in report["corrections"]] == [["0x00401000-0x00401010"], ["0x00401010-0x00401020"]]
    assert report["assignment"]["address"] == "0x00401100"
