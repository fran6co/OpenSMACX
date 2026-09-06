import json
from pathlib import Path
import subprocess
import sys

import pytest

from match_recovery.cli import main
from match_recovery.cpp_audit_rules import CPP_RECOVERY_V1
from match_recovery.source_audits import audit_source, code_lines


@pytest.fixture
def project(tmp_path):
    (tmp_path / "recovery").mkdir()
    (tmp_path / "code/seed/nested").mkdir(parents=True)
    (tmp_path / "code/dependency").mkdir()
    (tmp_path / "code/product").mkdir()
    (tmp_path / "code/a.cc").write_text("*g_00dead01 = 0;\n")
    (tmp_path / "code/product/a.cc").write_text("*g_00dead01 = 0;\n")
    (tmp_path / "code/a.hpp").write_text("int vtable;\nVCall();\n")
    for path in ("code/seed/a.cc", "code/seed/nested/a.cc", "code/dependency/a.cc"):
        (tmp_path / path).write_text("*g_00dead01 = 0;\n")
    (tmp_path / "recovery/project.json").write_text(json.dumps({
        "schema_version": 1,
        "source": {"image_pin": "code/PIN", "address_space": "test:address", "roots": ["code"],
                   "seed_roots": ["code/seed"], "extensions": [".cc", ".hpp"]},
        "audits": "recovery/quality.json"}))
    ceilings = dict.fromkeys(CPP_RECOVERY_V1, 0)
    ceilings.update({"anonymous fixed-address global": 2, "vtable-as-member": 1,
                     "seeds": 2, "top seeds": 1, "layout lines": 2, "manual slots": 1})
    config = {"ruleset": "cpp-recovery-v1", "unit_extensions": [".cc"], "interface_extensions": [".hpp"],
              "exclude_roots": ["code/dependency"], "ceilings": ceilings, "counters": [
                  {"name": "seeds", "kind": "files", "path": "code/seed", "extension": ".cc", "recursive": True},
                  {"name": "top seeds", "kind": "files", "path": "code/seed", "extension": ".cc", "recursive": False},
                  {"name": "layout lines", "kind": "lines", "path": "code/a.hpp"},
                  {"name": "manual slots", "kind": "text", "path": "code/a.hpp", "pattern": "vtable"}]}
    (tmp_path / "recovery/quality.json").write_text(json.dumps({"schema_version": 1, "audits": {"quality": config}}))
    return tmp_path


def modify_policy(root, change):
    path = root / "recovery/quality.json"
    value = json.loads(path.read_text())
    change(value["audits"]["quality"])
    path.write_text(json.dumps(value))


def test_native_audit_is_readonly_and_preserves_file_identity(project, capsys):
    before = {p.relative_to(project): p.read_bytes() for p in project.rglob("*") if p.is_file()}
    assert main(["audits", "check", "quality", "--root", str(project)]) == 0
    report = json.loads(capsys.readouterr().out)
    assert report["result"] == "pass" and report["structure_verification"] == "unknown"
    assert report["per_file"] == {"code/a.cc": {"anonymous fixed-address global": 1},
                                  "code/product/a.cc": {"anonymous fixed-address global": 1},
                                  "code/a.hpp": {"vtable-as-member": 1}}
    assert not report["persisted"] and report["lowerings"] == {}
    assert before == {p.relative_to(project): p.read_bytes() for p in project.rglob("*") if p.is_file()}


@pytest.mark.parametrize("count,status", [(1, "growth"), (3, "slack")])
def test_both_growth_and_slack_fail(project, capsys, count, status):
    modify_policy(project, lambda c: c["ceilings"].update({"anonymous fixed-address global": count}))
    assert main(["audits", "check", "quality", "--root", str(project), "--summary"]) == 1
    report = json.loads(capsys.readouterr().out)
    row = next(r for r in report["rules"] if r["name"] == "anonymous fixed-address global")
    assert row["status"] == status and row["count"] == 2
    assert report["lowerings"] == ({"anonymous fixed-address global": 2} if status == "slack" else {})
    assert "per_file" not in report
    assert json.loads((project / "recovery/quality.json").read_text())["audits"]["quality"]["ceilings"][row["name"]] == count


def test_damage_is_recomputed_without_mtime_or_cached_verdict(project):
    first = audit_source(project, "quality")
    (project / "code/a.cc").write_text("*g_00dead01 = g_00dead02;\n")
    second = audit_source(project, "quality")
    assert second["result"] == "fail"
    assert first["input_fingerprint"] != second["input_fingerprint"]
    assert second["per_file"]["code/a.cc"]["anonymous fixed-address global"] == 2


def test_directory_names_are_not_implicit_exclusions(project):
    (project / "code/product/recovered").mkdir()
    (project / "code/product/recovered/b.cc").write_text("*g_00dead01 = 0;")
    assert "code/product/recovered/b.cc" in audit_source(project, "quality")["per_file"]


def test_comment_lens_keeps_dereferences_without_spanning_lines():
    text = "/* comment\n *g_00dead01\n */\n// g_00dead02\n*ptr = 0; // g_00dead03\n*g_00dead04 = 0;"
    assert list(code_lines(text)) == ["*ptr = 0; ", "*g_00dead04 = 0;"]
    assert sum(len(CPP_RECOVERY_V1["vtable-as-member"][0].findall(line))
               for line in code_lines("int\nvtable;")) == 0


@pytest.mark.parametrize("change", [
    lambda c: c.update(ruleset="unknown"),
    lambda c: c.update(unrecognized=True),
    lambda c: c.update(unit_extensions=[]),
    lambda c: c.update(unit_extensions=[".cc", ".cc"]),
    lambda c: c.update(interface_extensions=[".cc"]),
    lambda c: c.update(unit_extensions=[".cpp"]),
    lambda c: c.update(exclude_roots=["../outside"]),
    lambda c: c.update(exclude_roots=["unconfigured"]),
    lambda c: c["ceilings"].pop("vtable"),
    lambda c: c["ceilings"].update(vtable=True),
    lambda c: c["ceilings"].update(vtable=-1),
    lambda c: c["ceilings"].update(extra=0),
    lambda c: c["counters"].append(c["counters"][0]),
    lambda c: c["counters"][0].update(recursive=1),
    lambda c: c["counters"][0].update(extension=".unknown"),
    lambda c: c["counters"][0].update(kind="unknown"),
    lambda c: c["counters"][3].update(pattern="["),
    lambda c: c["counters"][3].update(path="elsewhere/a.cc"),
])
def test_malformed_policy_refused(project, change):
    modify_policy(project, change)
    with pytest.raises(ValueError):
        audit_source(project, "quality")


@pytest.mark.parametrize("missing", ["code/a.hpp", "code/seed", "code"])
def test_missing_required_input_is_unknown_not_zero(project, capsys, missing):
    (project / missing).rename(project / "parked")
    assert main(["audits", "check", "quality", "--root", str(project)]) == 3
    assert not capsys.readouterr().out


def test_external_symlink_refused(project, tmp_path_factory):
    outside = tmp_path_factory.mktemp("outside") / "unit.cc"
    outside.write_text("*g_00dead01 = 0;")
    (project / "code/external.cc").symlink_to(outside)
    with pytest.raises(ValueError, match="outside"):
        audit_source(project, "quality")


@pytest.mark.parametrize("target", ["product", "missing.cc"])
def test_directory_or_dangling_symlink_is_not_silently_skipped(project, target):
    (project / "code/alias").symlink_to(target)
    with pytest.raises(ValueError, match="symlink"):
        audit_source(project, "quality")


@pytest.mark.parametrize("damage", ["file", "policy", "added", "deleted", "project"])
def test_mid_scan_input_changes_refused(project, monkeypatch, damage):
    import match_recovery.source_audits as audits
    original = audits.code_lines
    changed = False
    def changing(text):
        nonlocal changed
        if not changed:
            changed = True
            if damage == "file":
                (project / "code/a.cc").write_text("new content")
            elif damage == "policy":
                modify_policy(project, lambda c: c["ceilings"].update(vtable=1))
            elif damage == "project":
                with (project / "recovery/project.json").open("a") as output:
                    output.write("\n")
            elif damage == "deleted":
                (project / "code/a.cc").unlink()
            else:
                (project / "code/new.cc").write_text("new content")
        yield from original(text)
    monkeypatch.setattr(audits, "code_lines", changing)
    with pytest.raises(ValueError, match="changed"):
        audit_source(project, "quality")


def test_no_optional_dependencies_or_project_python_needed(project):
    import match_recovery
    package_root = Path(match_recovery.__file__).parents[1]
    script = f"import sys; sys.path.insert(0, {str(package_root)!r}); from match_recovery.cli import main; sys.exit(main(sys.argv[1:]))"
    result = subprocess.run([sys.executable, "-I", "-S", "-c", script, "audits", "check", "quality",
                             "--root", str(project)], capture_output=True, text=True, cwd=project)
    assert result.returncode == 0, result.stderr
    assert json.loads(result.stdout)["result"] == "pass"


def test_module_command_used_by_gate_runs_from_another_project(project):
    result = subprocess.run([sys.executable, "-m", "match_recovery.cli", "audits", "check", "quality",
                             "--root", str(project)], capture_output=True, text=True, cwd=project)
    assert result.returncode == 0, result.stderr
    assert json.loads(result.stdout)["result"] == "pass"
