"""Source ownership can evolve without letting seed scaffolds drive new tasks."""

from dataclasses import replace
import json
import subprocess

import pytest

from match_recovery.application_binary import select_bindings
from match_recovery.model import Origin, Span
from match_recovery.ownership_source import class_hint, project_view
from match_recovery.partition import Candidate, partition, select_group
from match_recovery.task_source import create_task


PIN = "a" * 64


@pytest.fixture
def project(tmp_path):
    for directory in ("recovery", "code", "hints"):
        (tmp_path / directory).mkdir()
    (tmp_path / "code/identity").write_text("sha256 " + PIN + "\n")
    (tmp_path / "recovery/project.json").write_text(json.dumps({
        "schema_version": 1,
        "source": {"image_pin": "code/identity", "address_space": "pe:va",
                   "roots": ["code", "hints"], "seed_roots": ["hints"],
                   "include_paths": ["code"], "symbol_format": "msvc"}}))
    return tmp_path


def put(root, path, catalogue, emitted, *, end=0x401003):
    (root / path).write_text(
        f"// ORIGINAL: 0x00401000 {catalogue} 0x00401000-0x{end:08X} FILE\n"
        f"// symbol    {emitted}\n")


@pytest.mark.parametrize("emitted", ["?owner@Widget@@UAEPAV1@XZ", "?owner@Widget@@W7AEPAV1@XZ"])
def test_current_member_and_adjustor_use_the_same_class_group(project, emitted):
    put(project, "code/widget.cpp", "sub_401000", emitted)
    view, _ = project_view(project)
    entity, = select_group(view, "class:Widget")["entities"]
    assert entity["ownership"] == {"basis": "source_symbol", "owners": ["Widget"],
                                    "alternative_hints": [], "verified": False}
    candidate, = entity["candidates"]
    assert candidate["owner_hint_basis"] == "source_symbol"
    assert candidate["symbol"] == emitted


def test_new_member_owner_keeps_catalogue_and_seed_alternatives(project):
    put(project, "code/widget.cpp", "?owner@OldOwner@@QAEPAXXZ", "?owner@Widget@@UAEPAV1@XZ")
    put(project, "hints/attempt.cpp", "?owner@AnotherGuess@@QAEPAXXZ", "?method@Scaffold@@QAEPAXXZ")
    view, snapshot = project_view(project)
    entity, = select_group(view, "class:Widget")["entities"]
    assert entity["ownership"]["alternative_hints"] == ["AnotherGuess", "OldOwner"]
    assert not entity["conflicts"] and len(entity["candidates"]) == 2
    assert snapshot["records"][0]["catalogued_symbol"] == "?owner@OldOwner@@QAEPAXXZ"
    assert not any(group["selector"] == "class:Scaffold" for group in view["groups"])
    bindings, refused, targets = select_bindings(view, frozenset({entity["entity_id"]}))
    assert len(bindings) == 1 and not refused
    assert targets == {"?owner@Widget@@UAEPAV1@XZ": 0x401000}


@pytest.mark.parametrize("catalogue,owner", [("sub_401000", None), ("?owner@OldOwner@@QAEPAXXZ", "OldOwner")])
def test_seed_emitter_class_is_never_promoted_to_current_ownership(project, catalogue, owner):
    put(project, "hints/attempt.cpp", catalogue, "?method@Scaffold@@QAEPAXXZ")
    view, _ = project_view(project)
    group = select_group(view, "class:" + owner if owner else "unassigned")
    entity, = group["entities"]
    assert entity["ownership"]["basis"] == "hints"
    assert entity["candidates"][0]["owner_hint"] == owner


def test_free_redirect_does_not_erase_catalogue_class_hint(project):
    put(project, "code/redirect.cpp", "?owner@Widget@@QAEPAXXZ", "@redirect@4")
    view, _ = project_view(project)
    entity, = select_group(view, "class:Widget")["entities"]
    assert entity["ownership"]["basis"] == "hints"
    assert entity["candidates"][0]["owner_hint_basis"] == "catalogued_symbol"


def test_grouping_keeps_a_seed_match_obligation_under_the_same_original_identity(project):
    put(project, "code/widget.cpp", "sub_401000", "?owner@Widget@@UAEPAV1@XZ")
    put(project, "hints/attempt.cpp", "?owner@OldOwner@@QAEPAXXZ", "?method@Scaffold@@QAEPAXXZ")
    seed = project / "hints/attempt.cpp"
    seed.write_text(seed.read_text().replace(" FILE\n", " FILE BYTE_EXACT\n"))
    before = seed.read_bytes()
    view, snapshot = project_view(project)
    entity, = select_group(view, "class:Widget")["entities"]
    obligation, = snapshot["historical_obligations"]
    assert obligation["entity_id"] == entity["entity_id"]
    assert entity["ownership"]["alternative_hints"] == ["OldOwner"]
    assert seed.read_bytes() == before


@pytest.mark.parametrize("symbol", ["?helper@namespace_name@@YAXXZ", "??__EGlobal@@YAXXZ", "??_GWidget@@UAEPAXI@Z"])
def test_unhandled_special_or_nonmember_names_remain_unassigned(symbol):
    assert class_hint(symbol) is None


def test_unconfigured_symbol_format_does_not_infer_current_or_old_class(project):
    path = project / "recovery/project.json"
    config = json.loads(path.read_text())
    config["source"]["symbol_format"] = "none"
    path.write_text(json.dumps(config))
    put(project, "code/widget.cpp", "?owner@OldOwner@@QAEPAXXZ", "?owner@Widget@@UAEPAV1@XZ")
    view, _ = project_view(project)
    entity, = select_group(view, "source:code/widget.cpp")["entities"]
    assert entity["ownership"]["owners"] == []
    assert entity["ownership"]["alternative_hints"] == []


@pytest.mark.parametrize("damage", ["extent", "source", "owner"])
def test_current_member_hints_cannot_hide_ambiguous_mappings(project, damage):
    put(project, "code/widget.cpp", "sub_401000", "?owner@Widget@@UAEPAV1@XZ")
    if damage == "extent":
        put(project, "hints/attempt.cpp", "?owner@OldOwner@@QAEPAXXZ", "?f@Scaffold@@QAEPAXXZ", end=0x401004)
    else:
        put(project, "code/other.cpp", "sub_401000",
            "?owner@AnotherOwner@@UAEPAV1@XZ" if damage == "owner" else "?owner@Widget@@UAEPAV1@XZ")
    view, _ = project_view(project)
    entity, = select_group(view, "conflict")["entities"]
    bindings, refused, _ = select_bindings(view, frozenset({entity["entity_id"]}))
    assert not bindings and refused
    if damage == "owner":
        assert "conflicting owner hints" in entity["conflicts"]


def test_supplied_conflicting_hints_remain_conflicts_without_source_member_evidence():
    first = Candidate(Origin(PIN, "pe:va", 0x401000), (Span(0x401000, 0x401003),),
                      "code/a.cpp", 1, False, "_a", "First")
    group = select_group(partition((first, replace(first, owner_hint="Second", seed=True))), "conflict")
    assert group["entities"][0]["ownership"]["owners"] == ["First", "Second"]


@pytest.mark.parametrize("change", [{"owner_hint_basis": "invented"},
                                    {"owner_hint_basis": "source_symbol", "seed": True},
                                    {"owner_hint_basis": "source_symbol", "owner_hint": None},
                                    {"catalogued_owner_hint": ""}])
def test_invalid_ownership_hint_provenance_is_refused(change):
    candidate = Candidate(Origin(PIN, "pe:va", 0x401000), (), "code/a.cpp", 1, False, "_a", "First")
    with pytest.raises(ValueError):
        replace(candidate, **change)


def test_source_derived_task_includes_anonymous_named_member_and_generated_entry(project):
    put(project, "code/widget.cpp", "sub_401000", "?owner@Widget@@UAEPAV1@XZ")
    with (project / "code/widget.cpp").open("a") as output:
        output.write("// ORIGINAL: 0x00401010 sub_401010 0x00401010-0x00401018 FILE\n"
                     "// symbol    ?owner@Widget@@W7AEPAV1@XZ\n")
    (project / "code/widget.h").write_text("class Widget;\n")
    def git(*args):
        return subprocess.run(["git", "-c", "user.name=Fixture", "-c", "user.email=fixture@example.invalid",
                               "-c", "commit.gpgsign=false", "-c", "core.hooksPath=/dev/null", *args],
                              cwd=project, check=True, capture_output=True, text=True).stdout.strip()
    git("init", "-q", "--initial-branch=fixture")
    git("add", ".")
    git("commit", "-qm", "Synthetic recovered source identities")
    baseline = git("rev-parse", "HEAD")
    result = create_task(project, name="widget", group="class:Widget", author="coordinator",
                         intent="Recover the whole owner interface", evidence=("source:code/widget.h",), target="synthetic")
    assert result["task"]["base_revision"] == baseline
    assert {item["address"] for item in result["task"]["subjects"]} == {0x401000, 0x401010}
    assert result["task"]["editable_files"] == ("code/widget.cpp", "code/widget.h")
    assert result["verification"] == "not_measured"
