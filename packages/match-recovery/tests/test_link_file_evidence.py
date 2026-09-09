"""Synthetic compiler-boundary observations with real file-change controls."""

from copy import deepcopy
import hashlib
import json

import pytest

from match_recovery.link_file_evidence import parse_link_files, verify_link_files
from match_recovery.msvc_link_log import parse_link_log
from test_link_log import LOG


def stamp(raw):
    return {"sha256": hashlib.sha256(raw).hexdigest(), "size": len(raw)}


def evidence(root, log=LOG):
    tool, library, other, obj = [root / name for name in ("linker", "runtime.lib", "other.lib", "app.obj")]
    for path in (tool, library, other, obj):
        if not path.exists():
            path.write_bytes(b"synthetic input " + path.name.encode())
    return {"schema_version": 1, "link_log_sha256": hashlib.sha256(log.encode()).hexdigest(),
            "cwd": str(root), "arguments": [str(obj)], "tool": str(tool), "explicit_inputs": [str(obj)],
            "files": [{"path": str(path), "before": stamp(path.read_bytes()), "after": stamp(path.read_bytes())}
                      for path in (tool, library, other, obj)],
            "library_searches": [{"spelling": row["path"], "path": str(other if "OTHER" in row["path"] else library)}
                                 for row in parse_link_log(log.encode())["searched_libraries"]], "capture_gaps": []}


def verify(value, log=LOG):
    return verify_link_files(json.dumps(value).encode(), log.encode())


def test_stable_file_observations_do_not_become_complete_input_or_storage_proof(tmp_path):
    report = verify(evidence(tmp_path))
    assert report["result"] == "pass"
    assert report["input_coverage"] == report["allocation_verification"] == "unknown"
    assert not report["ready_for_collection"]
    assert [row["phase"] for row in report["library_searches"]] == ["executable", "export_library"]


@pytest.mark.parametrize("damage", ["during", "after", "size"])
def test_changed_required_files_fail_even_when_the_link_log_agrees(tmp_path, damage):
    value = evidence(tmp_path)
    row = value["files"][1]
    if damage == "during":
        row["before"] = stamp(b"different original file")
    elif damage == "after":
        (tmp_path / "runtime.lib").write_bytes(b"changed after link")
    else:
        row["before"]["size"] += 1
        row["after"]["size"] += 1
    assert verify(value)["result"] == "fail"


@pytest.mark.parametrize("damage", ["missing_before", "missing_after", "missing_file", "symlink", "capture_gap", "unresolved_path"])
def test_incomplete_or_aliased_file_capture_stays_unknown(tmp_path, damage):
    value = evidence(tmp_path)
    path = tmp_path / "runtime.lib"
    if damage.startswith("missing_") and damage != "missing_file":
        value["files"][1][damage.removeprefix("missing_")] = None
    elif damage == "missing_file":
        path.unlink()
    elif damage == "symlink":
        target = tmp_path / "same-contents"
        path.rename(target)
        path.symlink_to(target)
    elif damage == "capture_gap":
        value["capture_gaps"].append("unsupported compiler input")
    else:
        value["library_searches"][0]["path"] = None
    assert verify(value)["result"] == "unknown"


@pytest.mark.parametrize("damage", ["duplicate", "schema", "size", "digest", "relative", "dotdot", "tool", "searched_file", "omitted_search", "wrong_log"])
def test_malformed_mixed_or_incomplete_search_identity_refuses(tmp_path, damage):
    value = evidence(tmp_path)
    if damage == "duplicate":
        value["files"].append(deepcopy(value["files"][0]))
    elif damage == "schema":
        value["schema_version"] = True
    elif damage == "size":
        value["files"][0]["before"]["size"] = True
    elif damage == "digest":
        value["files"][0]["before"]["sha256"] = "invalid"
    elif damage == "relative":
        value["cwd"] = "relative"
    elif damage == "dotdot":
        value["cwd"] += "/../elsewhere"
    elif damage == "tool":
        value["files"].pop(0)
    elif damage == "searched_file":
        value["files"].pop(1)
    elif damage == "omitted_search":
        value["library_searches"].pop()
    else:
        value["link_log_sha256"] = "0" * 64
    with pytest.raises(ValueError):
        verify(value)


def test_unused_candidates_are_not_misreported_as_selected_input_changes(tmp_path):
    value = evidence(tmp_path)
    value["files"].append({"path": str(tmp_path / "unused.lib"), "before": stamp(b"one"), "after": stamp(b"two")})
    report = verify(value)
    assert report["result"] == "pass" and report["files"][-1]["result"] == "fail"


def test_output_format_validation_reads_no_arbitrary_input_paths(tmp_path):
    value = evidence(tmp_path)
    (tmp_path / "runtime.lib").unlink()
    assert parse_link_files(json.dumps(value).encode()) == value
