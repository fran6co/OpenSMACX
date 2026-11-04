"""Explicit measure CLI consumer for live weak original target qualification."""

from contextlib import contextmanager
from hashlib import sha256
import json
from types import SimpleNamespace

import pytest

from match_recovery.cli import main
from match_recovery.application_context import application_build as REAL_APPLICATION_BUILD
from test_recording import project
from test_linked_weak_code_references import _receipt_bind, weak_build


def _source(fallback="void _G() {}"):
    return ("// ORIGINAL: 0x00401000 _caller 0x00401000-0x00401010\n"
            "extern \"C\" void _caller() {}\n"
            "// ORIGINAL: 0x00401010 _G 0x00401010-0x00401020\n"
            "extern \"C\" " + fallback + "\n")


def _configure_source(root, raw_image):
    source = root / "engine/family.cc"
    source.write_text(_source())
    (root / "original/input.exe").write_bytes(raw_image)
    (root / "original/PIN").write_text("sha256 " + sha256(raw_image).hexdigest() + "\n")
    return source.read_bytes()


@pytest.fixture
def original_cli_project(project, tmp_path, monkeypatch):
    return _make_original_cli_project(project, tmp_path, monkeypatch)


def _make_original_cli_project(project, tmp_path, monkeypatch, *, g_reference=False,
                               damaged_target=False):
    worker_root = tmp_path / "physical"
    # Build a physical weak caller/fallback object and seed its pinned image
    # into the source project before asking the actual measure command to run.
    from test_linked_weak_code_references import weak_build as make_build
    current = make_build(worker_root, g_reference=g_reference, damaged_target=damaged_target)
    raw_image = (current.directory / "app.exe").read_bytes()
    source_bytes = _configure_source(project, raw_image)
    (worker_root / "engine").mkdir()
    (worker_root / "engine/family.cc").write_bytes(source_bytes)
    # Share the already-built native physical input tree; no compiler is run.
    current_source = worker_root / "engine/family.cc"
    assert current_source.read_bytes() == source_bytes
    selected_raw = (current.directory / "unit0.obj").read_bytes()
    _receipt_bind(current, {"raw": selected_raw, "sha256": sha256(selected_raw).hexdigest()},
                  unit="engine/family.cc")
    context_check = current.check_current

    def check_current():
        context_check()
        if current_source.read_bytes() != source_bytes:
            raise ValueError("synthetic application source changed")

    current.check_current = check_current

    @contextmanager
    def live_build(root, roots, files, *, versioned_files):
        assert files["engine/family.cc"] == source_bytes
        assert versioned_files == {}
        from match_recovery.linked_context import linked_build
        with linked_build(current):
            yield current

    from match_recovery import application_context, build_check
    monkeypatch.setattr(application_context, "application_build", live_build)
    monkeypatch.setattr(build_check.Build, "read", classmethod(
        lambda cls, root: SimpleNamespace(versioned_inputs=())))
    return project


def test_measure_cli_consumes_live_strict_original_weak_row(original_cli_project, capsys):
    project = original_cli_project
    code = main(["measure", "_caller", "--linked-current", "--root", str(project),
                 "--image", "original/input.exe"])
    captured = capsys.readouterr()
    assert captured.out, f"CLI returned {code}; stderr={captured.err!r}"
    report = json.loads(captured.out)
    assert code in (0, 3), report
    row, = report["linked_current"]["functions"]
    assert row["symbol"] == "_caller" and row["result"] == "pass", row
    current_root, = report["linked_current"]["current_root_regions"]
    assert current_root["result"] == "pass", current_root
    weak, = [item for item in row["relocations"] if item["storage_class"] == 105]
    assert weak["weak_original_target"]["fallback_result"] == "pass"
    assert report["measurements"][0]["linked_current_original_comparison"]["result"] == "pass"


def test_measure_cli_does_not_traverse_unselected_weak_callers(original_cli_project, capsys):
    project = original_cli_project
    code = main(["measure", "_G", "--linked-current", "--root", str(project),
                 "--image", "original/input.exe"])
    report = json.loads(capsys.readouterr().out)
    assert code in (0, 3), report
    linked = report["linked_current"]
    assert linked["current_code_graphs"] == []
    assert linked["current_code_graph"] == "unknown"
    assert linked["current_root_regions"] == []
    assert linked["current_root_regions_reason"] == "no selected weak-reference root was observed"
    assert linked["current_code_graph_reason"].startswith("transitive callees were not traversed")
    assert all(row["linked_current_code_graph"] == "unknown" for row in linked["functions"])


def test_measure_cli_keeps_fallback_original_reference_failure_unknown(tmp_path, project, monkeypatch, capsys):
    project = _make_original_cli_project(project, tmp_path, monkeypatch, g_reference=True)
    path = project / "original/input.exe"
    raw = bytearray(path.read_bytes())
    raw[512 + 0x11:512 + 0x15] = (0x4011E1 - 0x401015).to_bytes(4, "little", signed=True)
    path.write_bytes(raw)
    (project / "original/PIN").write_text("sha256 " + sha256(raw).hexdigest() + "\n")
    code = main(["measure", "_caller", "--linked-current", "--root", str(project),
                 "--image", "original/input.exe"])
    report = json.loads(capsys.readouterr().out)
    assert code == 3, report
    row, = report["linked_current"]["functions"]
    assert row["result"] == "unknown"
    assert "strictly compare PASS" in row["reason"]
    measurement, = report["measurements"]
    assert measurement["linked_current_result"] == "unknown"
    assert measurement["result"] == "unknown"
    assert measurement["legacy_result"] in ("pass", "unknown")
    assert report["counts"] == {"unknown": 1}


def test_measure_cli_preserves_original_caller_reference_fail(tmp_path, project, monkeypatch, capsys):
    project = _make_original_cli_project(project, tmp_path, monkeypatch)
    path = project / "original/input.exe"
    raw = bytearray(path.read_bytes())
    raw[512 + 1] = (raw[512 + 1] + 1) & 0xFF
    path.write_bytes(raw)
    (project / "original/PIN").write_text("sha256 " + sha256(raw).hexdigest() + "\n")
    code = main(["measure", "_caller", "--linked-current", "--root", str(project),
                 "--image", "original/input.exe"])
    report = json.loads(capsys.readouterr().out)
    assert code in (1, 3), report
    row, = report["linked_current"]["functions"]
    assert row["result"] == "fail", row
    weak, = [item for item in row["relocations"] if item["storage_class"] == 105]
    assert weak["result"] == "fail"
    measurement, = report["measurements"]
    assert measurement["linked_current_result"] == "fail"
    assert measurement["result"] == "fail"
    assert report["counts"] == {"fail": 1}


def test_measure_cli_current_root_reference_mismatch_cannot_pass(tmp_path, project, monkeypatch, capsys):
    project = _make_original_cli_project(project, tmp_path, monkeypatch, damaged_target=True)
    code = main(["measure", "_caller", "--linked-current", "--root", str(project),
                 "--image", "original/input.exe"])
    report = json.loads(capsys.readouterr().out)
    assert code in (1, 3), report
    root_region, = report["linked_current"]["current_root_regions"]
    assert root_region["result"] == "fail", root_region
    row, = report["linked_current"]["functions"]
    assert row["result"] == "fail", row
    measurement, = report["measurements"]
    assert measurement["legacy_result"] in ("pass", "unknown")
    assert measurement["linked_current_result"] == "fail"
    assert measurement["result"] == "fail"
    assert report["counts"] == {"fail": 1}


def test_measure_cli_root_budget_refusal_revokes_current_pass_capability(
        original_cli_project, monkeypatch, capsys):
    project = original_cli_project
    from match_recovery import linked_context
    from match_recovery.linked_context import LinkedBuild

    monkeypatch.setattr(linked_context, "SOURCE_REGION_MAX_BYTES", 1)
    observe = LinkedBuild._source_regions_in_scope

    def exhaust_after_first(self, roots, weak_scope):
        return observe(self, [*roots, *roots], weak_scope)

    monkeypatch.setattr(LinkedBuild, "_source_regions_in_scope", exhaust_after_first)
    code = main(["measure", "_caller", "--linked-current", "--root", str(project),
                 "--image", "original/input.exe"])
    report = json.loads(capsys.readouterr().out)
    assert code == 3, report
    root_regions = report["linked_current"]["current_root_regions"]
    assert len(root_regions) == 2
    assert all(item["result"] == "unknown" and item["blocks_original_qualification"] is True
               for item in root_regions), root_regions
    row, = report["linked_current"]["functions"]
    assert row["result"] == "unknown", row
    assert not any(item.get("weak_original_target") for item in row.get("relocations", ()))
    measurement, = report["measurements"]
    assert measurement["linked_current_result"] == "unknown"
    assert measurement["result"] == "unknown"
    assert report["counts"] == {"unknown": 1}


def test_receipt_owner_resolution_skips_unrelated_ambiguous_import():
    from match_recovery.application_binary import _source_receipt_object_owners

    class Objects:
        _owners = {
            "unit.cpp": [{"owner": "unit.obj", "paths": ("/build/unit.obj",)}],
            "user32": [
                {"owner": "user32:USER32.dll", "paths": ("/sdk/USER32.dll",)},
                {"owner": "user32:USER32.dll", "paths": ("/sdk/USER32.dll",)},
            ],
        }

        def __init__(self):
            self.calls = []

        def resolve(self, owner):
            self.calls.append(owner)
            if owner == "user32:USER32.dll":
                raise ValueError("missing or ambiguous final-pass object owner: " + owner)
            return {"identity": {"path": "/build/unit.obj"}, "sha256": "a" * 64,
                    "raw": b"COFF"}

    objects = Objects()
    owners = _source_receipt_object_owners(objects, {"/build/unit.obj"})
    assert set(owners) == {"/build/unit.obj"}
    assert objects.calls == ["unit.obj"]


def test_receipt_owner_resolution_still_refuses_needed_ambiguous_import():
    from match_recovery.application_binary import _source_receipt_object_owners

    class Objects:
        _owners = {"user32": [{"owner": "user32:USER32.dll", "paths": ("/sdk/USER32.dll",)}]}

        def resolve(self, owner):
            raise ValueError("missing or ambiguous final-pass object owner: " + owner)

    with pytest.raises(ValueError, match="missing or ambiguous final-pass object owner"):
        _source_receipt_object_owners(Objects(), {"/sdk/USER32.dll"})


def test_archive_receipt_owner_resolution_binds_exact_native_member_and_object():
    from hashlib import sha256
    from match_recovery.application_binary import (_source_receipt_object_matches,
                                                    _source_receipt_object_owners)

    raw = b"physical archive member bytes"

    class Objects:
        _owners = {"jackal:checkbox.cpp.obj": [{
            "owner": "jackal:checkbox.cpp.obj",
            "member": "checkbox.cpp.obj",
            "paths": ("/build/jackal.lib",),
        }]}

        def resolve(self, owner):
            assert owner == "jackal:checkbox.cpp.obj"
            return {"identity": {"path": "/build/jackal.lib",
                                 "member": "src/jackal/checkbox.cpp.obj",
                                 "occurrence": 0},
                    "sha256": sha256(raw).hexdigest(), "raw": raw, "owner": owner}

    receipt = {"archive": "/build/jackal.lib",
               "member": "src/jackal/checkbox.cpp.obj",
               "object": "/build/checkbox.cpp.obj", "source": "/src/checkbox.cpp"}
    owners = _source_receipt_object_owners(Objects(), set(), [receipt])
    owner, selected = owners["/build/checkbox.cpp.obj"][0]
    assert owner == "jackal:checkbox.cpp.obj"
    assert selected["identity"] == {"path": receipt["archive"],
                                    "member": receipt["member"], "occurrence": 0}
    assert _source_receipt_object_matches(
        selected, {"sha256": sha256(raw).hexdigest(), "size": len(raw)})
    assert not _source_receipt_object_matches(
        selected, {"sha256": "0" * 64, "size": len(raw)})


def test_archive_receipt_owner_resolution_uses_physical_native_archive_fixture(tmp_path):
    from match_recovery.application_binary import _source_receipt_object_owners
    from match_recovery.linked_objects import SelectedObjects
    from test_linked_objects import library_inputs
    from test_binary import coff_fixture

    member = "checkbox.obj"
    evidence = library_inputs(tmp_path, [(member + "/", coff_fixture())], [member])
    objects = SelectedObjects(*evidence)
    archive = str(tmp_path / "runtime.lib")
    receipt = {"archive": archive, "member": member,
               "object": str(tmp_path / "checkbox.cpp.obj"),
               "source": str(tmp_path / "checkbox.cpp")}
    owners = _source_receipt_object_owners(objects, set(), [receipt])
    owner, selected = owners[receipt["object"]][0]
    assert owner.casefold() == "runtime:checkbox.obj"
    assert selected["identity"] == {"path": archive, "member": member, "occurrence": 0}
    assert selected["raw"] == coff_fixture()


@pytest.mark.parametrize("receipt", [
    {"archive": "/build/other.lib", "member": "src/jackal/checkbox.cpp.obj",
     "object": "/build/checkbox.cpp.obj", "source": "/src/checkbox.cpp"},
    {"archive": "/build/jackal.lib", "member": "other/checkbox.cpp.obj",
     "object": "/build/checkbox.cpp.obj", "source": "/src/checkbox.cpp"},
])
def test_archive_receipt_owner_resolution_refuses_wrong_archive_or_member(receipt):
    from match_recovery.application_binary import _source_receipt_object_owners

    class Objects:
        _owners = {"jackal:checkbox.cpp.obj": [{
            "owner": "jackal:checkbox.cpp.obj", "member": "checkbox.cpp.obj",
            "paths": ("/build/jackal.lib",),
        }]}

        def resolve(self, owner):
            return {"identity": {"path": "/build/jackal.lib",
                                 "member": "src/jackal/checkbox.cpp.obj",
                                 "occurrence": 0}, "sha256": "a" * 64,
                    "raw": b"bytes", "owner": owner}

    assert _source_receipt_object_owners(Objects(), set(), [receipt]) == {}


def test_archive_receipt_owner_resolution_keeps_missing_proof_and_required_ambiguity_closed():
    from match_recovery.application_binary import _source_receipt_object_owners

    class Objects:
        _owners = {"jackal:checkbox.cpp.obj": [{
            "owner": "jackal:checkbox.cpp.obj", "member": "checkbox.cpp.obj",
            "paths": ("/build/jackal.lib",),
        }]}

        def resolve(self, owner):
            raise ValueError("missing or ambiguous final-pass object owner: " + owner)

    assert _source_receipt_object_owners(Objects(), set()) == {}
    receipt = {"archive": "/build/jackal.lib",
               "member": "src/jackal/checkbox.cpp.obj",
               "object": "/build/checkbox.cpp.obj", "source": "/src/checkbox.cpp"}
    with pytest.raises(ValueError, match="missing or ambiguous final-pass object owner"):
        _source_receipt_object_owners(Objects(), set(), [receipt])


def test_archive_receipt_requires_passing_compile_link_and_unique_source_command():
    from match_recovery.application_binary import _passing_archive_source_receipts

    row = {"source": "/src/checkbox.cpp", "output": "/build/checkbox.cpp.obj",
           "result": "pass", "link_correspondence": "pass",
           "archive_correspondence": {"archive": "/build/jackal.lib",
                                      "member": "src/jackal/checkbox.cpp.obj",
                                      "object": "/build/checkbox.cpp.obj", "result": "pass"}}
    good_command = {"source": row["source"], "result": "pass"}
    assert _passing_archive_source_receipts([row], {row["output"]: [good_command]}) == [{
        "archive": "/build/jackal.lib", "member": "src/jackal/checkbox.cpp.obj",
        "object": row["output"], "source": row["source"]}]
    assert _passing_archive_source_receipts([row], {row["output"]: [
        {"source": "/src/different.cpp", "result": "pass"}]}) == []
    assert _passing_archive_source_receipts([row], {row["output"]: [
        {"source": row["source"], "result": "unknown"}]}) == []
    assert _passing_archive_source_receipts([row], {row["output"]: [good_command, good_command]}) == []
    missing_proof = {**row, "archive_correspondence": {**row["archive_correspondence"],
                                                       "result": "unknown"}}
    assert _passing_archive_source_receipts(
        [missing_proof], {row["output"]: [good_command]}) == []


def test_advisory_fold_candidate_without_complete_group_does_not_refuse_root():
    from match_recovery.linked_context import _selected_root_fold_group

    class Folding:
        def __init__(self):
            self.calls = []

        def for_map(self, owner, symbol, *, optional):
            self.calls.append(("for_map", owner, symbol, optional))
            return None

        def group(self, owner, symbol, *, optional):
            self.calls.append(("group", owner, symbol, optional))
            return None

    folding = Folding()
    candidates = {("unit.obj", "_caller")}
    assert _selected_root_fold_group(folding, "unit.obj", "_caller", candidates) is None
    assert [call[0] for call in folding.calls] == ["for_map", "group"]


def test_linked_current_passes_byte_tree_to_real_application_build(
        original_cli_project, monkeypatch):
    project = original_cli_project
    from match_recovery import application_context
    from match_recovery.build_check import Build
    from match_recovery.ownership_source import project_view

    monkeypatch.setattr(application_context, "application_build", REAL_APPLICATION_BUILD)
    _, snapshot = project_view(project)
    selected = {row["entity_id"] for row in snapshot["records"]}
    reads = 0

    class ReachedBuildRead(Exception):
        pass

    def stop_after_source_validation(root):
        nonlocal reads
        reads += 1
        if reads == 1:
            return type("BuildInputs", (), {"versioned_inputs": ()})()
        raise ReachedBuildRead

    monkeypatch.setattr(Build, "read", classmethod(lambda cls, root: stop_after_source_validation(root)))
    from match_recovery.application_binary import verify_linked_selection

    with pytest.raises(ReachedBuildRead):
        verify_linked_selection(project, selected, project / "original/input.exe")
    # verify_linked_selection reads project policy once. The second read occurs
    # inside the real application_build, after its validate_tree byte check.
    assert reads == 2

