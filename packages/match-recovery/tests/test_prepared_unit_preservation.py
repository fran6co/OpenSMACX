"""Real private preparation and source histories, with synthetic COFF output.

No application compiler is launched. These tests exercise preservation plumbing,
not executable recovery, build/link acceptance or historical recipe inference.
"""

from dataclasses import replace
import json
import re

import pytest

from match_recovery.driver import working_state
from match_recovery.model import Origin
from match_recovery.preparation import prepare_project
from match_recovery.preservation import check_preservation
from match_recovery.tasks import TaskStore
from test_function_witness import object_bytes
from test_multispan import object_fixture
from test_prepared_context import prepared
from test_preservation import project
from test_task_source import git


GRAPH = "BuildGraph.txt"
NEW_UNIT = "code/new.cc"
NEW_SOURCE = b"// ORIGINAL: 0x00401020 _g 0x00401020-0x00401021\nint g() { return 1; }\n"
GENERATOR = '''import json,sys
from pathlib import Path
root, output = Path(sys.argv[1]), Path(sys.argv[2])
units = (root / "BuildGraph.txt").read_text().splitlines()
rows = []
for unit in units:
    source = root / unit
    if unit == "@generated.cc":
        source = output / "generated.cc"
        source.write_text("int generated() { return 1; }\\n")
    rows.append({"directory": str(root), "file": str(source), "arguments": ["compiler", str(source)]})
(output / "compile_commands.json").write_text(json.dumps(rows))
'''


def configure(root, *, initially_present=False, unbound=False):
    policy_path = root / "recovery/prepare.json"
    policy = json.loads(policy_path.read_text())
    policy["versioned_inputs"] = [GRAPH]
    policy_path.write_text(json.dumps(policy))
    (root / "recovery/generate.py").write_text(GENERATOR)
    units = ["code/caller.cc", "code/owner.cc"]
    if initially_present:
        (root / NEW_UNIT).write_bytes(NEW_SOURCE)
        units.append(NEW_UNIT)
    if unbound:
        path = root / "code/owner.cc"
        path.write_bytes(path.read_bytes() + b"// EXTRA_G\n")
    (root / GRAPH).write_text("\n".join(units) + "\n")
    assert prepare_project(root)["published"]
    git(root, "add", ".")
    git(root, "commit", "-m", "Synthetic explicit preparation baseline")
    base = git(root, "rev-parse", "HEAD")
    store = TaskStore(root)
    template = store.read("owner")
    store.create(replace(template, task_id="prepared-owner", base_revision=base,
                         subjects=(*template.subjects, Origin("a" * 64, "pe:va", 0x401020)),
                         editable_files=(*template.editable_files, NEW_UNIT, GRAPH)))
    git(root, "add", ".")
    git(root, "commit", "-m", "Authorize prepared unit task")
    return base


def change_presence(root, *, remove=False, extra=b"", graph_damage=None):
    units = ["code/caller.cc", "code/owner.cc"]
    if remove:
        (root / NEW_UNIT).unlink()
    else:
        (root / NEW_UNIT).write_bytes(NEW_SOURCE + extra)
        units.append(NEW_UNIT)
    if graph_damage == "omit":
        units = [unit for unit in units if unit != NEW_UNIT]
    elif graph_damage == "duplicate":
        units.append(units[0])
    elif graph_damage == "missing":
        units.append("code/missing.cc")
    (root / GRAPH).write_text("\n".join(units) + "\n")
    # This refreshes the present checkout's ordinary database too. Preservation
    # must still independently regenerate both private historical/current sets.
    assert prepare_project(root)["published"]


@pytest.fixture
def compiler_calls(monkeypatch, prepared):
    calls = []

    class TreeCompiler:
        def __init__(self, root, **kwargs):
            self.root = root

        def __call__(self, source, command, flags):
            assert source.is_file(), "a missing TU must never be resurrected for compilation"
            name = source.relative_to(self.root).as_posix()
            graph = (self.root / GRAPH).read_text().splitlines()
            assert graph.count(name) == 1, "compilation requires the fresh version's unique recipe"
            contents = source.read_bytes()
            calls.append({"root": self.root, "unit": name, "graph": graph,
                          "contents": contents, "command": command, "flags": flags})
            for raw in (contents, (self.root / "code/model.h").read_bytes()):
                if raw.startswith(b"#error "):
                    raise ValueError(raw.splitlines()[0].decode())
            symbols = re.findall(rb"// ORIGINAL: 0x[0-9A-Fa-f]+ (\S+)", contents)
            functions = [(symbol.decode(), b"\xc3", ()) for symbol in symbols]
            if b"BROKEN_REPLICA" in contents and self.root != prepared:
                functions = [(symbol, b"\x90" + body, relocations) for symbol, body, relocations in functions]
            if b"EXTRA_G" in contents:
                functions.append(("_g", b"\xc3", ()))
            if b"EXTRA_HELPER" in contents:
                functions.append(("_helper", b"\xc3", ()))
            if b"EXTRA_DATA" in contents:
                assert functions == [("_g", b"\xc3", ())]
                return object_fixture(extra=(b".data", b"unowned-data", (), 0, 2, 0xC0501040)).replace(b"_f", b"_g")
            return object_bytes(functions)

    # Keep the native prepared path: do not pass compiler= to check_preservation.
    # Only the configured compiler protocol's object producer is synthetic.
    monkeypatch.setattr("match_recovery.preservation.Compiler", TreeCompiler)
    monkeypatch.setattr("match_recovery.prepared_context.Compiler", TreeCompiler)
    return calls


def check_unchanged(root, base, **kwargs):
    store = TaskStore(root)
    task, decisions, state = store.read("prepared-owner"), store.decisions("prepared-owner"), working_state(root)
    source = {path.relative_to(root).as_posix(): path.read_bytes() for path in (root / "code").rglob("*") if path.is_file()}
    graph = (root / GRAPH).read_bytes()
    report = check_preservation(root, "prepared-owner", **kwargs)
    assert report["base_revision"] == task.base_revision == base
    assert store.read("prepared-owner") == task and store.decisions("prepared-owner") == decisions
    assert working_state(root) == state
    assert {path.relative_to(root).as_posix(): path.read_bytes() for path in (root / "code").rglob("*") if path.is_file()} == source
    assert (root / GRAPH).read_bytes() == graph
    assert not report["ready_for_collection"] and not report["scope_complete"] and not report["persisted"]
    return report


@pytest.mark.parametrize("remove", [False, True])
def test_new_and_removed_tus_use_fresh_versioned_recipe_membership(prepared, compiler_calls, remove):
    base = configure(prepared, initially_present=remove)
    change_presence(prepared, remove=remove)
    report = check_unchanged(prepared, base)
    assert report["result"] == "pass", report
    present, absent = ("baseline", "current") if remove else ("current", "baseline")
    assert NEW_UNIT in report["recipe_membership"][present]
    assert NEW_UNIT not in report["recipe_membership"][absent]
    assert report["versioned_build_inputs"] == report["changed_build_inputs"] == [GRAPH]
    assert report["unsupported_file_set_changes"] == []
    unit = next(row for row in report["units"] if row["unit"] == NEW_UNIT)
    assert unit["unit_presence"] == {"baseline": remove, "current": not remove}
    assert unit["result"] == unit["replica"] == "pass"
    control, = report["controls"][0]["files"]
    assert control == {"path": NEW_UNIT, "unit": NEW_UNIT, "tree": present, "result": "pass"}
    new_calls = [call for call in compiler_calls if call["unit"] == NEW_UNIT]
    assert len(new_calls) >= 3 and all(NEW_UNIT in call["graph"] for call in new_calls)
    damaged = [call for call in new_calls if call["contents"].startswith(b"#error MATCH_RECOVERY_")]
    assert len(damaged) == 1
    assert damaged[0]["root"] != prepared
    assert any(call["root"] != prepared and NEW_UNIT not in call["graph"] for call in compiler_calls)


@pytest.mark.parametrize("damage", ["omit", "duplicate", "missing"])
def test_missing_or_duplicate_fresh_recipes_are_unknown_before_compile(prepared, compiler_calls, damage):
    base = configure(prepared)
    change_presence(prepared, graph_damage=damage)
    report = check_unchanged(prepared, base)
    assert report["result"] == "unknown" and report["preparation_error"]
    assert report["unsupported_file_set_changes"] == [NEW_UNIT]
    assert report["compile_attempts"] == 0 and compiler_calls == []


@pytest.mark.parametrize("extra", [b"// EXTRA_HELPER\n", b"// EXTRA_DATA\n"])
def test_added_tu_cannot_hide_independent_unowned_function_or_data(prepared, compiler_calls, extra):
    base = configure(prepared)
    change_presence(prepared, extra=extra)
    report = check_unchanged(prepared, base)
    assert report["result"] == "fail", report
    unit = next(row for row in report["units"] if row["unit"] == NEW_UNIT)
    assert unit["result"] == "fail" and unit["replica"] == "pass"
    assert unit["changed_sections"]
    assert all(control["result"] == "pass" for control in report["controls"])


def test_filtered_unit_selection_never_claims_complete_preservation(prepared, compiler_calls):
    base = configure(prepared)
    change_presence(prepared)
    report = check_unchanged(prepared, base, units=(NEW_UNIT,))
    assert report["result"] == "unknown"
    assert report["unmeasured_units"] == ["code/caller.cc", "code/owner.cc"]
    assert report["units"][0]["result"] == "pass"


def test_existing_unbound_baseline_definition_stays_protected_in_added_tu(prepared, compiler_calls):
    base = configure(prepared, unbound=True)
    change_presence(prepared)
    report = check_unchanged(prepared, base)
    assert report["result"] == "fail", report
    unit = next(row for row in report["units"] if row["unit"] == NEW_UNIT)
    assert unit["result"] == "fail"
    assert unit["newly_bound_protected_functions"] == {"_g": Origin("a" * 64, "pe:va", 0x401020).entity_id}
    assert any(row["symbol"] == "_g" for row in unit["symbol_changes"])
    assert any(call["unit"] == "code/owner.cc" and b"EXTRA_G" in call["contents"] for call in compiler_calls)
    baseline_calls = [call for call in compiler_calls if NEW_UNIT not in call["graph"]]
    for unit in ("code/caller.cc", "code/owner.cc"):
        assert len([call for call in baseline_calls if call["unit"] == unit]) == 2


def test_graph_only_generated_tu_cannot_be_intersected_out_of_coverage(prepared, compiler_calls):
    base = configure(prepared)
    graph = prepared / GRAPH
    graph.write_bytes(graph.read_bytes() + b"@generated.cc\n")
    assert prepare_project(prepared)["published"]
    assert list((prepared / "build/preparation").glob("build-*/generated.cc"))
    report = check_unchanged(prepared, base)
    assert report["result"] == "unknown"
    assert report["changed_files"] == report["file_set_changes"] == []
    assert report["changed_build_inputs"] == [GRAPH]
    assert "recipe membership differs from source TU coverage" in report["preparation_error"]
    assert report["compile_attempts"] == 0 and compiler_calls == []


@pytest.mark.parametrize("extra", [b"", b"// EXTRA_HELPER\n", b"// EXTRA_DATA\n"])
def test_added_unit_diagnostic_keeps_raw_scoped_and_unmapped_evidence_without_baseline_compiles(
        prepared, compiler_calls, monkeypatch, extra):
    base = configure(prepared)
    change_presence(prepared, extra=extra)

    def forbidden(*args, **kwargs):
        pytest.fail("raw added-TU diagnostics cannot subtract compiler-witnessed scope")

    monkeypatch.setattr("match_recovery.inline_ownership.observe_copies", forbidden)
    report = check_unchanged(prepared, base, units=(NEW_UNIT,), diagnostic=True)
    assert report["result"] == "unknown" and report["diagnostic_only"]
    row, = report["units"]
    assert row["result"] == "unknown" and row["replica"] == "pass"
    assert row["diagnostic_basis"] == "raw_present_object"
    assert row["baseline_definition_coverage"] == row["scope_subtraction"] == "omitted_for_diagnostics"
    assert "compiled_comparison" not in row
    assert row["baseline_scoped_functions"] == row["current_scoped_functions"] == {}
    assert row["newly_bound_protected_functions"] == {}
    symbols = {symbol["symbol"] for symbol in row["symbol_changes"]}
    assert "_g" in symbols  # Even authorized source scope stays visible here.
    assert any(binding["symbol"] == "_g" and binding["task_scope"] for binding in row["affected_source_bindings"])
    if b"EXTRA_HELPER" in extra:
        assert "_helper" in symbols
    if b"EXTRA_DATA" in extra:
        assert any(section["name"] == ".data" for section in row["changed_sections"])
    assert report["compile_attempts"] == len(compiler_calls) == 2
    assert all(call["unit"] == NEW_UNIT and NEW_UNIT in call["graph"] for call in compiler_calls)
    assert compiler_calls[0]["root"] == prepared and compiler_calls[1]["root"] != prepared
    assert not any(call["contents"].startswith(b"#error") for call in compiler_calls)


def test_added_unit_diagnostic_cannot_publish_raw_evidence_from_a_failed_replica(prepared, compiler_calls):
    base = configure(prepared)
    change_presence(prepared, extra=b"// BROKEN_REPLICA\n")
    report = check_unchanged(prepared, base, units=(NEW_UNIT,), diagnostic=True)
    row, = report["units"]
    assert report["result"] == row["result"] == "unknown"
    assert "current replica does not agree" in row["reason"]
    assert "compiled_comparison" not in row and "replica" not in row
    assert "changed_sections" not in row and "symbol_changes" not in row
    assert report["compile_attempts"] == len(compiler_calls) == 2
    assert all(call["unit"] == NEW_UNIT for call in compiler_calls)


@pytest.mark.parametrize("diagnostic", [False, True])
def test_removed_only_filtered_unit_does_not_scan_unrelated_baseline_definitions(prepared, compiler_calls, diagnostic):
    base = configure(prepared, initially_present=True)
    change_presence(prepared, remove=True)
    report = check_unchanged(prepared, base, units=(NEW_UNIT,), diagnostic=diagnostic)
    row, = report["units"]
    assert report["result"] == "unknown"  # Other existing units remain unmeasured.
    assert row["result"] == ("unknown" if diagnostic else "pass")
    assert row["compiled_comparison"] == row["replica"] == "pass"
    assert row["unit_presence"] == {"baseline": True, "current": False}
    assert "diagnostic_basis" not in row
    assert all(call["unit"] == NEW_UNIT and NEW_UNIT in call["graph"] for call in compiler_calls)
    assert report["compile_attempts"] == len(compiler_calls) == (2 if diagnostic else 3)


@pytest.mark.parametrize("diagnostic", [False, True])
def test_filtered_out_added_unit_does_not_trigger_global_baseline_scan(prepared, compiler_calls, diagnostic):
    base = configure(prepared)
    change_presence(prepared)
    report = check_unchanged(prepared, base, units=("code/owner.cc",), diagnostic=diagnostic)
    row, = report["units"]
    assert report["result"] == "unknown" and NEW_UNIT in report["unmeasured_units"]
    assert row["unit"] == "code/owner.cc"
    assert row["compiled_comparison"] == row["replica"] == "pass"
    assert "diagnostic_basis" not in row
    assert all(call["unit"] == "code/owner.cc" for call in compiler_calls)
    assert any(NEW_UNIT not in call["graph"] for call in compiler_calls)
