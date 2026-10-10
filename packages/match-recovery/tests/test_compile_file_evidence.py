"""Portable real-file controls; no compiler execution or acceptance claim."""

from copy import deepcopy
import hashlib
import json
import os
from pathlib import Path

import pytest

from match_recovery.compile_file_evidence import parse_compile_files, verify_compile_files


def stamp(raw):
    return {"sha256": hashlib.sha256(raw).hexdigest(), "size": len(raw)}


def evidence(root):
    for name in ("unit.cc", "compiler", "header.h", "unit.obj"):
        (root / name).write_bytes(b"synthetic bytes: " + name.encode())
    return {"schema_version": 1, "source": str(root / "unit.cc"), "tool": str(root / "compiler"),
            "cwd": str(root), "arguments": ["opaque", "two words", "@literal-not-parsed"],
            "inputs": [{"path": str(root / name), "before": stamp((root / name).read_bytes()),
                        "after": stamp((root / name).read_bytes())} for name in ("unit.cc", "compiler", "header.h")],
            "output": {"path": str(root / "unit.obj"), "existed_before": False,
                       "after": stamp((root / "unit.obj").read_bytes())}, "exit_code": 0}


def verify(value, **kwargs):
    return verify_compile_files(json.dumps(value).encode(), **kwargs)


def pipeline_evidence(root, value=None):
    """Synthetic trusted capture, not execution or transform-semantic proof."""
    value = evidence(root) if value is None else value
    processor = root / "processor"
    processor.write_bytes(b"synthetic transform executable")
    tool = str(processor)
    fingerprint = stamp(processor.read_bytes())
    value["inputs"].append({"path": tool, "before": fingerprint, "after": fingerprint})
    raw = stamp(b"synthetic primary output before transform")
    value.update(schema_version=2, compiler_output={**value["output"], "after": raw},
        postprocess=[{"tool": tool, "cwd": value["cwd"],
            "argv": [tool, "opaque-transform", value["output"]["path"]], "inputs": [tool],
            "input_before": raw, "output_after": value["output"]["after"], "exit_code": 0}])
    return value


def test_pipeline_is_explicit_and_does_not_prove_transform_semantics(tmp_path):
    value = pipeline_evidence(tmp_path)
    report = verify(value)
    assert report["result"] == report["pipeline"]["result"] == "pass"
    assert report["output_provenance"] == "postprocessed"
    assert "pipeline output" in report["output"]["reason"]
    assert report["pipeline"]["compiler_output"] == value["compiler_output"]
    assert report["pipeline"]["transformation_semantics"] == "unknown"
    assert report["input_coverage"] == report["linked_selection"] == "unknown"
    assert not report["ready_for_collection"]


def test_multiple_pipeline_steps_and_explicit_empty_pipeline(tmp_path):
    value = pipeline_evidence(tmp_path)
    first = value["postprocess"][0]
    second = deepcopy(first)
    intermediate = stamp(b"synthetic intermediate")
    first["output_after"] = intermediate
    second["input_before"] = intermediate
    value["postprocess"].append(second)
    assert verify(value)["result"] == "pass"
    value["postprocess"] = []
    value["compiler_output"] = deepcopy(value["output"])
    report = verify(value)
    assert report["result"] == "pass" and report["output_provenance"] == "direct"
    assert report["pipeline"]["stages"] == []


@pytest.mark.parametrize("damage", ["primary", "between", "final", "failed_stage", "tool_during", "tool_after"])
def test_pipeline_discontinuity_or_execution_changes_cannot_pass(tmp_path, damage):
    value = pipeline_evidence(tmp_path)
    stage = value["postprocess"][0]
    if damage == "primary":
        value["compiler_output"]["after"] = stamp(b"different primary")
    elif damage == "between":
        second = deepcopy(stage)
        second["input_before"] = stamp(b"unrelated intermediate")
        value["postprocess"].append(second)
    elif damage == "final":
        stage["output_after"] = stamp(b"wrong final output")
    elif damage == "failed_stage":
        stage["exit_code"] = 1
    elif damage == "tool_during":
        value["inputs"][-1]["before"] = stamp(b"old processor")
    else:
        Path(stage["tool"]).write_bytes(b"replacement processor")
    assert verify(value)["result"] == "fail"


@pytest.mark.parametrize("damage", ["raw_missing", "raw_empty", "stage_input", "stage_output", "tool_missing"])
def test_unobserved_pipeline_states_remain_unknown(tmp_path, damage):
    value = pipeline_evidence(tmp_path)
    if damage == "raw_missing": value["compiler_output"]["after"] = None
    elif damage == "raw_empty":
        value["compiler_output"]["after"] = value["postprocess"][0]["input_before"] = stamp(b"")
    elif damage == "stage_input": value["postprocess"][0]["input_before"] = None
    elif damage == "stage_output": value["postprocess"][0]["output_after"] = None
    else: value["inputs"][-1]["before"] = None
    assert verify(value)["result"] == "unknown"


@pytest.mark.parametrize("damage", ["v1", "no_primary", "no_stages", "primary_extra", "primary_path",
    "primary_preexisting", "primary_bool", "stage_extra", "stages_type", "stage_bool", "argv_tool",
    "argv_empty", "argv_nul", "tool_unobserved", "cwd_other", "files_duplicate", "files_unobserved", "files_no_tool"])
def test_malformed_pipeline_evidence_refuses(tmp_path, damage):
    value = pipeline_evidence(tmp_path)
    stage = value["postprocess"][0]
    if damage == "v1": value["schema_version"] = 1
    elif damage == "no_primary": del value["compiler_output"]
    elif damage == "no_stages": del value["postprocess"]
    elif damage == "primary_extra": value["compiler_output"]["extra"] = True
    elif damage == "primary_path": value["compiler_output"]["path"] = str(tmp_path / "other.obj")
    elif damage == "primary_preexisting": value["compiler_output"]["existed_before"] = True
    elif damage == "primary_bool": value["compiler_output"]["existed_before"] = 0
    elif damage == "stage_extra": stage["permission"] = True
    elif damage == "stages_type": value["postprocess"] = {}
    elif damage == "stage_bool": stage["exit_code"] = False
    elif damage == "argv_tool": stage["argv"][0] = value["tool"]
    elif damage == "argv_empty": stage["argv"] = []
    elif damage == "argv_nul": stage["argv"].append("bad\0argument")
    elif damage == "tool_unobserved": stage["tool"] = stage["argv"][0] = str(tmp_path / "unobserved")
    elif damage == "cwd_other": stage["cwd"] = str(tmp_path / "other")
    elif damage == "files_duplicate": stage["inputs"].append(stage["tool"])
    elif damage == "files_unobserved": stage["inputs"].append(str(tmp_path / "unobserved"))
    else: stage["inputs"] = [value["source"]]
    with pytest.raises(ValueError):
        parse_compile_files(json.dumps(value).encode())


def test_fresh_observations_are_not_complete_input_or_acceptance_proof(tmp_path):
    value = evidence(tmp_path)
    report = verify(value, object_path=tmp_path / "unit.obj")
    assert report["result"] == "pass"
    assert all(row["result"] == "pass" for row in report["inputs"])
    assert report["output"]["result"] == report["cwd"]["result"] == "pass"
    assert report["arguments"] == value["arguments"]
    assert report["input_coverage"] == report["linked_selection"] == "unknown"
    assert not report["persisted"] and not report["ready_for_collection"]


@pytest.mark.parametrize("index", [0, 1, 2])
@pytest.mark.parametrize("damage", ["during", "after", "size", "restored_mtime"])
def test_all_declared_inputs_are_required_and_fresh(tmp_path, index, damage):
    value = evidence(tmp_path)
    row = value["inputs"][index]
    path = Path(row["path"])
    if damage == "during":
        row["before"] = stamp(b"old content")
    elif damage == "size":
        row["before"]["size"] += 1
        row["after"]["size"] += 1
    else:
        previous = path.stat()
        path.write_bytes(b"X" * previous.st_size)
        if damage == "restored_mtime":
            os.utime(path, ns=(previous.st_atime_ns, previous.st_mtime_ns))
    report = verify(value)
    assert report["result"] == report["inputs"][index]["result"] == "fail"


@pytest.mark.parametrize("damage", ["bytes", "size"])
def test_output_content_mismatch_fails(tmp_path, damage):
    value = evidence(tmp_path)
    if damage == "bytes":
        (tmp_path / "unit.obj").write_bytes(b"different object")
    else:
        value["output"]["after"]["size"] += 1
    report = verify(value)
    assert report["result"] == report["output"]["result"] == "fail"


@pytest.mark.parametrize("which,damage", [(which, damage) for which in ("input", "output", "cwd")
                                         for damage in ("missing", "symlink", "parent_alias", "wrong_kind", "hardlink")
                                         if (which, damage) != ("cwd", "hardlink")])
def test_missing_or_aliased_paths_never_pass(tmp_path, which, damage):
    value = evidence(tmp_path)
    row = value["inputs"][0] if which == "input" else value["output"]
    path = Path(row["path"])
    if which == "cwd":
        path = tmp_path / "working"
        path.mkdir()
        value["cwd"] = str(path)
    if damage == "parent_alias":
        alias = tmp_path / "alias"
        alias.symlink_to(tmp_path, target_is_directory=True)
        spelling = str(alias / path.name)
        if which == "cwd":
            value["cwd"] = spelling
        else:
            row["path"] = spelling
            if which == "input":
                value["source"] = spelling
    elif damage == "hardlink":
        os.link(path, tmp_path / "another-name")
    else:
        if which == "cwd":
            path.rmdir()
        else:
            path.unlink()
        if damage == "symlink":
            target = tmp_path / "replacement"
            target.mkdir() if which == "cwd" else target.write_bytes(b"replacement")
            path.symlink_to(target)
        elif damage == "wrong_kind":
            path.write_bytes(b"not a directory") if which == "cwd" else path.mkdir()
    assert verify(value)["result"] == "unknown"


@pytest.mark.parametrize("field", ["before", "after"])
def test_missing_input_stamp_stays_unknown(tmp_path, field):
    value = evidence(tmp_path)
    value["inputs"][0][field] = None
    assert verify(value)["result"] == "unknown"


@pytest.mark.parametrize("damage", ["null", "preexisting", "empty"])
def test_unproved_output_stays_unknown(tmp_path, damage):
    value = evidence(tmp_path)
    if damage == "null":
        value["output"]["after"] = None
    elif damage == "preexisting":
        value["output"]["existed_before"] = True
    else:
        (tmp_path / "unit.obj").write_bytes(b"")
        value["output"]["after"] = stamp(b"")
    assert verify(value)["result"] == "unknown"


@pytest.mark.parametrize("code", [-9, 1, 3, 255])
def test_failed_compiler_cannot_pass_even_with_nonempty_object(tmp_path, code):
    value = evidence(tmp_path)
    value["exit_code"] = code
    assert verify(value)["result"] == "fail"


def test_exact_requested_object_path_not_matching_basename(tmp_path):
    value = evidence(tmp_path)
    for wrong in (Path("unit.obj"), tmp_path / "other" / "unit.obj", tmp_path / "other.obj"):
        with pytest.raises(ValueError, match="different object"):
            verify(value, object_path=wrong)


@pytest.mark.parametrize("damage", ["extra", "version", "exit_bool", "empty_args", "arg_nul", "input_shape",
                                     "duplicate", "missing_source", "missing_tool", "relative", "dotdot",
                                     "unnormalized", "stamp_size_bool", "stamp_digest", "stamp_extra",
                                     "output_shape", "output_bool", "output_input"])
def test_malformed_identity_refuses_before_file_reads(tmp_path, damage):
    value = evidence(tmp_path)
    if damage == "extra": value["trusted"] = True
    elif damage == "version": value["schema_version"] = True
    elif damage == "exit_bool": value["exit_code"] = False
    elif damage == "empty_args": value["arguments"] = []
    elif damage == "arg_nul": value["arguments"] = ["bad\0word"]
    elif damage == "input_shape": value["inputs"][0]["extra"] = 1
    elif damage == "duplicate": value["inputs"].append(deepcopy(value["inputs"][0]))
    elif damage == "missing_source": value["inputs"].pop(0)
    elif damage == "missing_tool": value["inputs"].pop(1)
    elif damage == "relative": value["cwd"] = "relative"
    elif damage == "dotdot": value["cwd"] += "/../elsewhere"
    elif damage == "unnormalized": value["cwd"] += "/./"
    elif damage == "stamp_size_bool": value["inputs"][0]["before"]["size"] = True
    elif damage == "stamp_digest": value["inputs"][0]["before"]["sha256"] = "bad"
    elif damage == "stamp_extra": value["inputs"][0]["before"]["mtime"] = 42
    elif damage == "output_shape": value["output"]["before"] = None
    elif damage == "output_bool": value["output"]["existed_before"] = 0
    else: value["output"]["path"] = value["source"]
    with pytest.raises(ValueError):
        parse_compile_files(json.dumps(value).encode())


def test_parser_does_not_require_files_to_exist_and_rejects_duplicate_json_keys(tmp_path):
    value = evidence(tmp_path)
    for path in tmp_path.iterdir():
        path.unlink()
    encoded = json.dumps(value).encode()
    assert parse_compile_files(encoded) == value
    with pytest.raises(ValueError, match="duplicate JSON key"):
        parse_compile_files(encoded[:-1] + b', "exit_code": 0}')


def test_file_change_during_fresh_hash_never_passes(tmp_path, monkeypatch):
    value = evidence(tmp_path)
    original = hashlib.file_digest
    calls = []
    def mutate(stream, algorithm):
        result = original(stream, algorithm)
        if not calls:
            (tmp_path / "unit.cc").write_bytes(b"mutated during read")
        calls.append(True)
        return result
    monkeypatch.setattr(hashlib, "file_digest", mutate)
    assert verify(value)["result"] == "unknown"
