"""Selected owners require physical final-pass identities, not map guesses."""

from copy import deepcopy
from hashlib import sha256
import json
import os
from pathlib import Path

import pytest

from match_recovery import linked_objects as native
from match_recovery.link_file_evidence import parse_link_files
from match_recovery.link_input_coverage import _input_inventory
from test_binary import coff_fixture
from test_coff_inventory import imported
from test_link_file_evidence import stamp
from test_link_input_coverage import inputs
from test_toolchain_components import archive


def inventory(value, log):
    raw = json.dumps(value).encode()
    coverage, records = _input_inventory(raw, log.encode())
    return parse_link_files(raw), coverage, records


@pytest.fixture
def observed(tmp_path):
    result = inventory(*inputs(tmp_path))
    assert result[1]["result"] == "pass"
    return result


def test_standalone_archive_and_generated_owners_resolve_without_crt_assumptions(observed, tmp_path):
    resolver = native.SelectedObjects(*observed)
    for requested, actual, expected in (
        ("APP.OBJ", "app.obj", {"path": str(tmp_path / "app.obj")}),
        ("runtime:RUNTIME.obj", "RUNTIME:runtime.obj", {"path": str(tmp_path / "runtime.lib"), "member": "runtime.obj", "occurrence": 0}),
        ("app.exp", "app.exp", {"path": str(tmp_path / "app.exp"), "generated": "msvc_export"}),
    ):
        result = resolver.resolve(requested)
        assert result["identity"] == expected and result["owner"] == actual
        assert sha256(result["raw"]).hexdigest() == result["sha256"]
        assert set(result) == {"identity", "owner", "sha256", "raw"}
        assert b".CRT$XCU" not in result["raw"]


@pytest.mark.parametrize("owner", ["missing.obj", "RUNTIME:DEMO.dll", "elsewhere/app.obj", "runtime:missing.obj", " app.obj"])
def test_missing_ambiguous_import_or_non_map_owners_refuse(observed, owner):
    with pytest.raises(ValueError): native.SelectedObjects(*observed).resolve(owner)


@pytest.mark.parametrize("damage", ["coverage", "gaps", "unsupported", "record_count", "input_count", "record_result",
                                   "missing_stamp", "changed_stamp", "duplicate_stamp", "bad_occurrence", "generated_existing"])
def test_incomplete_or_invalid_native_inventory_is_not_selection_evidence(observed, damage):
    evidence, coverage, records = deepcopy(observed)
    if damage == "coverage": coverage["result"] = "unknown"
    elif damage == "gaps": coverage["gaps"] = [{"reason": "missing input"}]
    elif damage == "unsupported": coverage["unsupported_records"] = [{"path": "unsupported"}]
    elif damage == "record_count": coverage["record_count"] -= 1
    elif damage == "input_count": coverage["input_count"] -= 1
    elif damage == "record_result": records[0]["result"] = "unknown"
    elif damage == "missing_stamp": evidence["files"][0]["after"] = None
    elif damage == "changed_stamp": evidence["files"][0]["after"] = stamp(b"different bytes")
    elif damage == "duplicate_stamp": evidence["files"].append(deepcopy(evidence["files"][0]))
    elif damage == "bad_occurrence": next(row for row in records if "member" in row)["occurrence"] = True
    else: evidence["generated_exports"][0]["existed_before"] = True
    with pytest.raises(ValueError): native.SelectedObjects(evidence, coverage, records)


@pytest.mark.parametrize("damage", ["selection", "candidate_coverage", "count", "paths", "duplicate_final", "wrong_path", "duplicate_record"])
def test_final_observation_and_complete_candidates_must_agree(observed, damage):
    evidence, coverage, records = deepcopy(observed)
    final = next(row for row in coverage["inputs"] if row["label"] == "app.obj")
    if damage == "selection": final["selection"] = "unknown"
    elif damage == "candidate_coverage": final["candidate_coverage"] = "unknown"
    elif damage == "count": final["candidate_count"] = 2
    elif damage == "paths": final["paths"].append(final["paths"][0])
    elif damage == "duplicate_final":
        coverage["inputs"].append({**deepcopy(final), "member": "APP.OBJ"})
        coverage["input_count"] += 1
    elif damage == "wrong_path": final["paths"] = [str(Path(final["paths"][0]).with_name("app.exp"))]
    else:
        records.append(deepcopy(next(row for row in records if row["path"].endswith("app.obj"))))
        coverage["record_count"] += 1
    with pytest.raises(ValueError): native.SelectedObjects(evidence, coverage, records).resolve("app.obj")


@pytest.mark.parametrize("damage", ["bytes", "symlink", "hardlink", "parent_alias", "missing"])
def test_physical_input_mutations_and_aliases_refuse_before_reading(observed, tmp_path, damage):
    resolver = native.SelectedObjects(*observed)
    target = tmp_path / "app.obj"
    if damage == "bytes": target.write_bytes(b"new contents")
    elif damage == "missing": target.unlink()
    elif damage in ("symlink", "hardlink"):
        replacement = tmp_path / "original.obj"
        target.rename(replacement)
        if damage == "symlink": target.symlink_to(replacement)
        else: os.link(replacement, target)
    else:
        replacement = tmp_path.with_name(tmp_path.name + "-physical")
        tmp_path.rename(replacement)
        tmp_path.symlink_to(replacement, target_is_directory=True)
    with pytest.raises((ValueError, OSError)): resolver.resolve("app.obj")


def library_inputs(root, members, finals):
    value, _ = inputs(root)
    library = root / "runtime.lib"
    library.write_bytes(archive(members))
    row = next(row for row in value["files"] if row["path"] == str(library))
    row["before"] = row["after"] = stamp(library.read_bytes())
    value["generated_exports"] = []
    log = (f"Start Pass1\n Searching {library}:\nEnd Pass1\nStart Pass2\napp.obj\n"
           + "".join(f"RUNTIME.lib({name})\n" for name in finals) + "End Pass2\n")
    value["link_log_sha256"] = stamp(log.encode())["sha256"]
    return inventory(value, log)


@pytest.mark.parametrize("members", [
    [("same.obj/", coff_fixture()), ("same.obj/", coff_fixture())],
    [("a/same.obj/", coff_fixture()), ("b/same.obj/", coff_fixture())],
])
def test_actual_archive_collisions_cannot_be_hidden_by_forged_inventory_subset(tmp_path, members):
    evidence, coverage, records = library_inputs(tmp_path, members, ["same.obj"])
    assert coverage["result"] == "pass"
    with pytest.raises(ValueError): native.SelectedObjects(evidence, coverage, records).resolve("RUNTIME:same.obj")
    # Removing an inconvenient occurrence from metadata does not remove it
    # from the independently stamp-checked physical archive.
    candidates = [row for row in records if "member" in row]
    records.remove(candidates[-1])
    coverage["record_count"] -= 1
    final = coverage["inputs"][-1]
    final.update(candidate_count=1, selection="pass")
    with pytest.raises(ValueError, match="physical archive member identity"):
        native.SelectedObjects(evidence, coverage, records).resolve("RUNTIME:same.obj")


def test_unique_short_import_is_not_returned_as_an_ordinary_coff_object(tmp_path):
    evidence, coverage, records = library_inputs(tmp_path, [("DEMO.dll/", imported())], ["DEMO.dll"])
    assert coverage["result"] == "pass" and coverage["inputs"][-1]["selection"] == "pass"
    with pytest.raises(ValueError, match="ordinary COFF"):
        native.SelectedObjects(evidence, coverage, records).resolve("RUNTIME:DEMO.dll")


def test_loaded_archive_and_files_are_cached_once_but_returned_metadata_is_defensive(tmp_path, monkeypatch):
    observed = library_inputs(tmp_path, [("first.obj/", coff_fixture()), ("second.obj/", coff_fixture(b"\xc3"))],
                              ["first.obj", "second.obj"])
    reads, parses = [], []
    read, parse = native._read, native.CoffArchive
    def reading(path, expected):
        reads.append(path)
        return read(path, expected)
    def parsing(raw):
        parses.append(True)
        return parse(raw)
    monkeypatch.setattr(native, "_read", reading)
    monkeypatch.setattr(native, "CoffArchive", parsing)
    resolver = native.SelectedObjects(*observed)
    original = resolver.resolve("runtime:first.obj")
    resolver.resolve("runtime:second.obj")
    changed = resolver.resolve("RUNTIME:FIRST.OBJ")
    changed["identity"]["member"] = "forged.obj"
    changed["raw"] = b"forged"
    assert resolver.resolve("runtime:first.obj") == original
    assert reads == [str(tmp_path / "runtime.lib")] and parses == [True]


def test_caller_metadata_changes_cannot_refresh_the_selected_epoch(observed):
    resolver = native.SelectedObjects(*observed)
    observed[0]["files"].clear()
    observed[1]["inputs"].clear()
    observed[2].clear()
    assert resolver.resolve("app.obj")["owner"] == "app.obj"


@pytest.mark.parametrize("damage", ["record_sha", "member", "occurrence"])
def test_selected_member_inventory_cannot_substitute_another_physical_record(observed, damage):
    evidence, coverage, records = deepcopy(observed)
    row = next(row for row in records if row.get("member") == "runtime.obj")
    if damage == "record_sha": row["sha256"] = "0" * 64
    elif damage == "member": row["member"] = "directory/runtime.obj"
    else: row["occurrence"] = 1
    with pytest.raises(ValueError): native.SelectedObjects(evidence, coverage, records).resolve("runtime:runtime.obj")


@pytest.mark.parametrize("damage", ["label", "library_search", "generated_identity", "bool_count"])
def test_final_labels_search_spellings_and_generated_provenance_cannot_be_reinterpreted(observed, damage):
    evidence, coverage, records = deepcopy(observed)
    if damage == "label": coverage["inputs"][0]["member"] = "different.exp"
    elif damage == "library_search": evidence["library_searches"][0]["spelling"] = "other.lib"
    elif damage == "generated_identity": del next(row for row in records if "generated" in row)["generated"]
    else: next(row for row in coverage["inputs"] if row["member"] == "runtime.obj")["candidate_count"] = True
    with pytest.raises(ValueError):
        native.SelectedObjects(evidence, coverage, records).resolve("runtime:runtime.obj")
