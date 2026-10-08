"""Build orchestration controls; collector doubles do not establish acceptance."""

from copy import deepcopy
import hashlib
import json

import pytest

from match_recovery.build_check import Build, check_build
from test_build_check import project, command, write_config
from test_link_file_evidence import evidence as link_evidence
from test_link_log import LOG


CONFIG = {"database": "compile_commands.json", "source_extensions": [".c"],
          "directory": "compile-files", "suffix": ".json", "link_files": "link-files.json", "link_log": "link.log",
          "archive_members": []}
OUTPUTS = [{"path": CONFIG[key], "format": format_name} for key, format_name in (
    ("database", "compilation-database"), ("link_files", "link-input-files"), ("link_log", "msvc-link-log"))]


def configure(root):
    artifacts = {"compile_commands.json": json.dumps([{
        "directory": str(root), "file": str(root / "code/main.c"),
        "arguments": ["synthetic-compiler-not-executed", "code/main.c"]}]),
        "link-files.json": json.dumps(link_evidence(root)), "link.log": LOG}
    script = ("import json,sys; from pathlib import Path; directory=Path(sys.argv[2]); "
              "[(directory/name).write_text(data) for name,data in json.loads(sys.argv[1]).items()]; "
              "(directory/'compile-files').mkdir()")
    write_config(root, compile_link=deepcopy(CONFIG), outputs=OUTPUTS,
                 commands=[command(script, json.dumps(artifacts), "{build}")])
    return artifacts


def test_absent_or_null_compile_link_preserves_unconfigured_behavior(project, monkeypatch):
    from match_recovery import build_compile_evidence
    monkeypatch.setattr(build_compile_evidence, "verify_build_compilations",
                        lambda *a, **k: pytest.fail("unconfigured build must not collect compilation evidence"))
    assert Build.read(project).compile_link is None
    plain = check_build(project)
    assert plain["result"] == "pass" and "compile_link" not in plain
    write_config(project, compile_link=None)
    assert Build.read(project).compile_link is None
    explicit = check_build(project)
    assert explicit["result"] == "pass" and "compile_link" not in explicit


def test_build_uses_validated_compile_link_policy_and_pins_it(project):
    configure(project)
    settings = Build.read(project)
    assert settings.compile_link == CONFIG
    before = settings.snapshot(project)
    write_config(project, compile_link={**CONFIG, "suffix": ".receipt.json"})
    assert Build.read(project).snapshot(project) != before


def test_omitted_archive_bindings_normalize_to_empty_policy(project):
    configure(project)
    before = Build.read(project).snapshot(project)
    policy = {key: value for key, value in CONFIG.items() if key != "archive_members"}
    write_config(project, compile_link=policy)
    assert Build.read(project).compile_link == CONFIG
    # Equivalent parsed policy does not erase the actual configuration edit.
    assert Build.read(project).snapshot(project) != before


@pytest.mark.parametrize("damage", ["extra", "missing", "not_object", "database", "source_extensions",
                                     "directory", "suffix", "link_files", "link_log", "output_format"])
def test_invalid_compile_link_configuration_refuses_in_build_parser(project, damage):
    configure(project)
    value = deepcopy(CONFIG)
    if damage == "extra": value["trust"] = True
    elif damage == "missing": del value["directory"]
    elif damage == "not_object": value = []
    elif damage == "source_extensions": value[damage] = []
    elif damage == "suffix": value[damage] = "../receipt"
    elif damage == "output_format":
        write_config(project, outputs=[{**row, "format": "file"} for row in OUTPUTS])
    else: value[damage] = "../outside"
    write_config(project, compile_link=value)
    with pytest.raises(ValueError):
        Build.read(project)


@pytest.mark.parametrize("result", ["pass", "fail", "unknown"])
def test_collector_result_governs_build_and_receives_exact_live_artifact_hashes(project, monkeypatch, result):
    from match_recovery import build_compile_evidence
    artifacts = configure(project)
    calls = []
    observation = {"result": result, "test_only": "orchestration double, not compilation proof"}
    def collect(root, directory, policy, *, outputs):
        assert root == project and directory.is_dir() and policy == CONFIG
        assert (directory / CONFIG["directory"]).is_dir()
        assert {row["path"] for row in outputs} == set(artifacts)
        for row in outputs:
            data = (directory / row["path"]).read_bytes()
            assert row["result"] == "pass"
            assert row["sha256"] == hashlib.sha256(data).hexdigest()
            assert row["size"] == len(data)
        calls.append(directory)
        return observation
    monkeypatch.setattr(build_compile_evidence, "verify_build_compilations", collect)
    report = check_build(project)
    assert report["result"] == result and report["compile_link"] == observation
    assert len(calls) == 1 and not calls[0].exists()
    assert all(row["result"] == "pass" for row in report["outputs"])
    assert not report["ready_for_collection"] and not report["persisted"]


@pytest.mark.parametrize("failure", ["value_error", "os_error", "missing_result", "invalid_result", "not_object"])
def test_unavailable_or_malformed_collector_cannot_borrow_successful_build(project, monkeypatch, failure):
    from match_recovery import build_compile_evidence
    configure(project)
    def collect(*args, **kwargs):
        if failure == "value_error": raise ValueError("unmeasurable compilation evidence")
        if failure == "os_error": raise OSError("missing compilation input")
        return {"missing_result": {}, "invalid_result": {"result": True}, "not_object": []}[failure]
    monkeypatch.setattr(build_compile_evidence, "verify_build_compilations", collect)
    report = check_build(project)
    assert report["result"] == report["compile_link"]["result"] == "unknown"
    assert report["compile_link"]["verification"] == "not_measured"
    assert report["compile_link"]["reason"]
    assert all(row["result"] == "pass" for row in report["outputs"])


@pytest.mark.parametrize("failure", ["command", "missing_artifact", "invalid_artifact"])
def test_failed_build_never_collects_evidence_or_reuses_previous_result(project, monkeypatch, failure):
    from match_recovery import build_compile_evidence
    configure(project)
    calls = []
    def collect(*args, **kwargs):
        calls.append(True)
        return {"result": "pass", "old_result": "must not leak into later run"}
    monkeypatch.setattr(build_compile_evidence, "verify_build_compilations", collect)
    assert check_build(project)["compile_link"]["result"] == "pass"
    configuration = json.loads((project / "recovery/build.json").read_text())
    if failure == "command":
        commands = [*configuration["commands"], command("raise SystemExit(1)")]
    else:
        action = "unlink()" if failure == "missing_artifact" else "write_text('not valid JSON')"
        commands = [*configuration["commands"], command(
            "from pathlib import Path; import sys; (Path(sys.argv[1])/'compile_commands.json')." + action, "{build}")]
    write_config(project, commands=commands)
    report = check_build(project)
    assert len(calls) == 1
    assert report["result"] == ("fail" if failure == "command" else "unknown")
    assert report["compile_link"]["result"] == "unknown"
    assert report["compile_link"]["verification"] == "not_measured" and "old_result" not in report["compile_link"]


@pytest.mark.parametrize("target", ["source", "policy"])
def test_collector_is_inside_build_freshness_boundary_and_mutation_is_preserved(project, monkeypatch, target):
    from match_recovery import build_compile_evidence
    configure(project)
    path = project / ("code/main.c" if target == "source" else "recovery/build.json")
    before = path.read_bytes()
    def collect(*args, **kwargs):
        path.write_bytes(before + b"\n")
        return {"result": "pass"}
    monkeypatch.setattr(build_compile_evidence, "verify_build_compilations", collect)
    with pytest.raises(ValueError, match="build changed source, policy or Git state"):
        check_build(project)
    assert path.read_bytes() == before + b"\n"
