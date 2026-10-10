"""Actual command membership controls, without a compiler or acceptance claim."""

from copy import deepcopy
import hashlib
import json
from pathlib import Path

import pytest

from match_recovery.build_graph_commands import compare_build_commands
from test_build_command_evidence import descriptor, STAGES


def stamp(data):
    return {"sha256": hashlib.sha256(data).hexdigest(), "size": len(data)}


class Context:
    def __init__(self, root, names, *, tool=None):
        self.root, self.directory = root, root / "build/fresh"
        self.directory.mkdir(parents=True)
        (root / "recovery").mkdir()
        (root / "recovery/project.json").write_text(json.dumps({"build": "recovery/build.json"}))
        outputs = [{"path": path, "format": kind} for path, kind in (
            ("commands.json", "compilation-database"), ("link.json", "link-input-files"), ("link.log", "msvc-link-log"))]
        self.policy = {"schema_version": 1, "artifacts": "build", "commands": [{"argv": ["unused", "{build}"], "timeout": 1}],
                       "outputs": outputs, "inputs": [], "input_roots": ["src"], "environment": [],
                       "compile_link": {"database": "commands.json", "directory": "objects", "suffix": ".compile.json",
                                        "source_extensions": [".cpp"], "link_files": "link.json", "link_log": "link.log",
                                        "command_stages": STAGES}}
        self.save_policy()
        tool = root / "compiler" if tool is None else tool
        tool.write_bytes(b"actual compiler identity")
        compiled, bound, objects = [], [], []
        for name in names:
            source, output = root / name, self.directory / (name + ".obj")
            source.parent.mkdir(parents=True, exist_ok=True)
            source.write_bytes(b"void function() {}")
            output.parent.mkdir(parents=True, exist_ok=True)
            output.write_bytes(b"actual object")
            args = [str(source), "/Fo" + str(output), "-O2", "-DKEEP"]
            observed_inputs = [{"path": str(path), "before": stamp(path.read_bytes()), "after": stamp(path.read_bytes()),
                                "result": "pass"} for path in (source, tool)]
            compilation = {"source": str(source), "tool": str(tool), "cwd": {"path": str(root), "result": "pass"},
                           "arguments": list(args), "inputs": observed_inputs, "exit_code": 0, "result": "pass",
                           "output": {"path": str(output), "existed_before": False, "after": stamp(output.read_bytes()), "result": "pass"}}
            compiled.append({"source": str(source), "output": str(output), "compilation": compilation,
                             "result": "pass", "link_correspondence": "pass"})
            bound.append({"source": str(source), "output": str(output), "arguments": list(args), "cwd": str(root),
                          "description": descriptor(args, STAGES["compile"], [(0, 0, "source"), (1, 3, "output_file")]),
                          "result": "pass"})
            objects.append(str(output))
        args = [*objects, "/OUT:" + str(self.directory / "application"), "alpha.lib", "beta.lib", "alpha.lib"]
        link = {"arguments": args, "cwd": str(self.directory), "result": "pass",
                "description": descriptor(args, STAGES["link"], [(i, 0, "input_file") for i in range(len(objects))]
                                           + [(len(objects), 5, "output_file")])}
        self.report = {"result": "pass", "compile_link": {"result": "pass", "source_membership": "pass",
                       "sources": [row["source"] for row in compiled], "missing_sources": [], "unexpected_sources": [],
                       "compilations": compiled, "link_files": {"result": "pass"},
                       "command_bindings": {"result": "pass", "compilations": bound, "link": link}}}
        self.checks = 0
        self.fail_at = None

    def save_policy(self):
        (self.root / "recovery/build.json").write_text(json.dumps(self.policy))

    def check_current(self):
        self.checks += 1
        if self.fail_at is not None and self.checks >= self.fail_at:
            raise ValueError("context changed")

    @property
    def proof(self):
        return self.report["compile_link"]

    @property
    def binding(self):
        return self.proof["command_bindings"]


@pytest.fixture
def pair(tmp_path):
    return Context(tmp_path / "before", ["src/a.cpp", "src/b.cpp"]), Context(tmp_path / "after", ["src/a.cpp", "src/b.cpp"])


def compare(pair, editable=()):
    return compare_build_commands(*pair, editable=editable)


def test_common_commands_and_root_local_tool_identity_are_relocated_exactly(pair):
    original = [deepcopy(item.report) for item in pair]
    result = compare(pair)
    assert result["result"] == "pass"
    assert result["sources"] == {"common": ["src/a.cpp", "src/b.cpp"], "added": [], "removed": []}
    assert result["linked_effects"] == result["input_coverage"] == "unknown"
    assert not result["persisted"] and not result["ready_for_collection"]
    assert [item.report for item in pair] == original and [item.checks for item in pair] == [2, 2]


@pytest.mark.parametrize("side", [0, 1])
@pytest.mark.parametrize("evidence", ["provenance", "stages"])
def test_pipeline_commands_cannot_be_ignored_in_build_graph_comparison(pair, side, evidence):
    compilation = pair[side].proof["compilations"][0]["compilation"]
    if evidence == "provenance":
        compilation["output_provenance"] = "postprocessed"
    else:
        compilation["pipeline"] = {"stages": [{"argv": ["uncompared transform"]}]}
    with pytest.raises(ValueError, match="ordered pipeline command comparison"):
        compare(pair)
    assert pair[0].checks == pair[1].checks == 2


def test_only_added_removed_object_tokens_are_projected_with_explicit_evidence(tmp_path):
    old = Context(tmp_path / "before", ["src/old.cpp", "src/keep.cpp"])
    new = Context(tmp_path / "after", ["src/keep.cpp", "src/new.cpp"])
    result = compare((old, new), ["src/old.cpp", "src/new.cpp"])
    assert result["result"] == "pass" and len(result["compilations"]) == 1
    assert result["removed_operands"] == {
        "baseline": [{"argument": 0, "text": str(old.directory / "src/old.cpp.obj"), "output": str(old.directory / "src/old.cpp.obj")}],
        "current": [{"argument": 1, "text": str(new.directory / "src/new.cpp.obj"), "output": str(new.directory / "src/new.cpp.obj")}]}
    with pytest.raises(ValueError, match="editable"):
        compare((old, new), ["src/old.cpp"])


@pytest.mark.parametrize("damage", ["flag", "duplicate_flag", "tool_bytes", "tool_path", "stage", "cwd", "libraries", "common_object_order"])
def test_non_object_command_and_tool_differences_never_disappear(pair, damage):
    new = pair[1]
    bound, capture = new.binding["compilations"][0], new.proof["compilations"][0]["compilation"]
    if damage == "flag": bound["arguments"][-1] = "-DCHANGED"; capture["arguments"][-1] = "-DCHANGED"
    elif damage == "duplicate_flag": bound["arguments"].append("-O2"); capture["arguments"].append("-O2")
    elif damage == "tool_bytes":
        capture["inputs"][1]["before"] = capture["inputs"][1]["after"] = stamp(b"different compiler")
    elif damage == "tool_path":
        capture["tool"] = str(new.root / "another-compiler")
        capture["inputs"][1]["path"] = capture["tool"]
    elif damage == "stage":
        new.policy["compile_link"]["command_stages"] = {**STAGES, "compile": "other"}
        new.save_policy()
        for row in new.binding["compilations"]: row["description"]["stage"] = "other"
    elif damage == "cwd":
        bound["cwd"] = capture["cwd"]["path"] = str(new.directory)
    elif damage == "libraries": new.binding["link"]["arguments"][-2:] = ["alpha.lib", "beta.lib"]
    else:
        link = new.binding["link"]
        link["arguments"][0], link["arguments"][1] = link["arguments"][1], link["arguments"][0]
        for operand in link["description"]["operands"][:2]:
            operand["argument"] = 1 - operand["argument"]
    assert compare(pair)["result"] == "fail"


@pytest.mark.parametrize("damage", ["build", "correspondence", "binding", "source_membership", "missing_source", "duplicate_source",
                                   "duplicate_bound", "wrong_bound_output", "capture_arguments", "capture_source", "tool_missing",
                                   "tool_unknown", "tool_changed", "unsupported", "undeclared_stage", "missing_stages", "missing_link_object",
                                   "duplicate_link_object", "source_role", "output_role", "generated_source"])
def test_incomplete_inconsistent_or_unowned_evidence_refuses(pair, damage):
    new = pair[1]
    bound = new.binding["compilations"][0]
    capture = new.proof["compilations"][0]["compilation"]
    if damage == "build": new.report["result"] = "unknown"
    elif damage == "correspondence": new.proof["result"] = "unknown"
    elif damage == "binding": new.binding["result"] = "unknown"
    elif damage == "source_membership": new.proof["source_membership"] = "unknown"
    elif damage == "missing_source": new.proof["sources"].pop()
    elif damage == "duplicate_source": new.proof["sources"].append(new.proof["sources"][0])
    elif damage == "duplicate_bound": new.binding["compilations"].append(deepcopy(bound))
    elif damage == "wrong_bound_output": bound["output"] = str(new.directory / "other.obj")
    elif damage == "capture_arguments": capture["arguments"][-1] = "-DFORGED"
    elif damage == "capture_source": capture["source"] = str(new.root / "other.cpp")
    elif damage == "tool_missing": capture["inputs"].pop()
    elif damage == "tool_unknown": capture["inputs"][1]["after"] = None
    elif damage == "tool_changed": capture["inputs"][1]["after"] = stamp(b"different")
    elif damage == "unsupported": bound["description"]["unsupported"] = ["opaque response file"]
    elif damage == "undeclared_stage": bound["description"]["stage"] = "other"
    elif damage == "missing_stages": del new.policy["compile_link"]["command_stages"]; new.save_policy()
    elif damage == "source_role": bound["description"]["operands"][0]["kind"] = "input_file"
    elif damage == "output_role": bound["description"]["operands"][1]["kind"] = "input_file"
    elif damage == "generated_source":
        source = str(new.directory / "generated.cpp")
        Path(source).write_bytes(b"generated")
        new.proof["sources"][0] = source
        new.proof["compilations"][0]["source"] = source
        bound["source"] = source
    else:
        link = new.binding["link"]
        if damage == "missing_link_object": link["description"]["operands"].pop(0)
        else:
            index = len(link["arguments"])
            link["arguments"].append(link["arguments"][0])
            link["description"]["operands"].append({**link["description"]["operands"][0], "argument": index})
    with pytest.raises(ValueError): compare(pair)
    assert [item.checks for item in pair] == [2, 2]


@pytest.mark.parametrize("embedded", [True, False])
def test_added_object_cannot_delete_a_flag_or_another_operand(tmp_path, embedded):
    old, new = Context(tmp_path / "before", ["src/a.cpp"]), Context(tmp_path / "after", ["src/a.cpp", "src/new.cpp"])
    link = new.binding["link"]
    operand = link["description"]["operands"][1]
    if embedded:
        link["arguments"][1] = "/OBJECT:" + link["arguments"][1]
        operand.update(start=8, end=len(link["arguments"][1]))
    else:
        original = link["arguments"][1]
        link["arguments"][1] += ",/another/input"
        link["description"]["operands"].append({"argument": 1, "start": len(original) + 1,
            "end": len(link["arguments"][1]), "path": "/another/input", "kind": "input_file", "codec": 0})
    with pytest.raises(ValueError, match="complete link argument"):
        compare((old, new), ["src/new.cpp"])


@pytest.mark.parametrize("side", [0, 1])
@pytest.mark.parametrize("fail_at", [1, 2])
def test_both_context_epochs_checked_on_entry_exit_and_errors(pair, side, fail_at):
    pair[side].fail_at = fail_at
    with pytest.raises(ValueError, match="context changed"):
        compare(pair)
    assert [item.checks for item in pair] == [2, 2]


def test_same_external_compiler_path_and_bytes_are_not_rebased(tmp_path):
    tool = tmp_path / "external-compiler"
    old = Context(tmp_path / "before", ["src/a.cpp"], tool=tool)
    new = Context(tmp_path / "after", ["src/a.cpp"], tool=tool)
    assert compare((old, new))["result"] == "pass"


@pytest.mark.parametrize("editable", ["src/a.cpp", ["../outside.cpp"], ["/absolute.cpp"]])
def test_editable_paths_are_explicit_normalized_relative_authority(pair, editable):
    with pytest.raises(ValueError): compare(pair, editable)


@pytest.mark.parametrize("damage", ["source_type", "missing_description", "missing_capture", "missing_link"])
def test_malformed_evidence_is_a_refusal_with_final_freshness(pair, damage):
    new = pair[1]
    if damage == "source_type": new.proof["sources"][0] = []
    elif damage == "missing_description": del new.binding["compilations"][0]["description"]
    elif damage == "missing_capture": del new.proof["compilations"][0]["compilation"]
    else: del new.binding["link"]
    with pytest.raises(ValueError): compare(pair)
    assert [item.checks for item in pair] == [2, 2]
