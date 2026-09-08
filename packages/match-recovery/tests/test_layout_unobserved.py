"""Bounded source-derived layout witnesses after ordinary compiler coverage."""

import json

import pytest

from match_recovery.cli import main
from match_recovery.layout_check import check_layout
from test_layout_check import add_unit
from test_layout_witness import Compiler, ABSENT
from test_symbol_check import project
from test_codeview import coff


@pytest.fixture
def unobserved_project(project):
    (project / "code/types.h").write_text("class Widget {\n int slot_; // 0x4\n};\n")
    path = project / "code/f.cc"
    path.write_text('#include "types.h"\n' + path.read_text())
    return project


def check(root, compiler, **kwargs):
    return check_layout(root, compiler=compiler, conventional_offsets=True, witness_unobserved=True, **kwargs)


def test_normal_objects_then_controlled_gap_witnesses_share_one_epoch(unobserved_project):
    root = unobserved_project
    compiler = Compiler(root)
    report = check(root, compiler)
    assert report["result"] == "pass" and report["counts"] == {"pass": 1}
    assert report["compile_attempts"] == 8  # two ordinary objects, three overlay controls per flag set
    assert len(compiler.calls) == 6
    assert report["unobserved_witnesses"]["hosts"]["Widget"]["unit"] == "code/f.cc"
    assert report["expectations"][0]["observed_definitions"] == 2
    assert report["structure_verification"] == "unknown" and not report["ready_for_collection"]
    assert {r["observation"] for r in report["coverage"]} == {"not_emitted", "witnessed"}


def test_direct_header_owner_precedes_smaller_indirect_host(unobserved_project):
    root = unobserved_project
    add_unit(root)
    (root / "code/indirect.h").write_text('#include "types.h"\n')
    (root / "code/g.cc").write_text('#include "indirect.h"\n')
    compiler = Compiler(root)
    report = check(root, compiler)
    assert report["result"] == "pass" and report["compile_attempts"] == 10
    assert all(path.name == "f.cc" for path, *_ in compiler.calls)


def test_observed_definition_is_never_replaced_by_a_better_probe(unobserved_project):
    class Observed(Compiler):
        def __call__(self, *_): return coff()
    compiler = Observed(unobserved_project)
    report = check(unobserved_project, compiler)
    assert report["result"] == "pass" and report["compile_attempts"] == 2
    assert not compiler.calls and not report["unobserved_witnesses"]["attempts"]


def test_only_the_unobserved_flag_set_is_probed(unobserved_project):
    class PartlyObserved(Compiler):
        def __call__(self, path, _, flags): return coff() if flags == ("first",) else ABSENT
    compiler = PartlyObserved(unobserved_project)
    report = check(unobserved_project, compiler)
    assert report["result"] == "pass" and report["compile_attempts"] == 5
    assert {flags for _, _, _, flags in compiler.calls} == {("second",)}


@pytest.mark.parametrize("damage", ["replica", "ignore", "invisible", "control", "layout"])
def test_failed_controls_do_not_fill_coverage(unobserved_project, damage):
    report = check(unobserved_project, Compiler(unobserved_project, damage=damage))
    assert report["result"] == "unknown" and report["counts"] == {"unknown": 1}
    assert all(row["result"] == "unknown" for row in report["unobserved_witnesses"]["attempts"])


def test_observed_bad_offset_in_a_probe_is_a_failure(unobserved_project):
    report = check(unobserved_project, Compiler(unobserved_project, offset=8))
    assert report["result"] == "fail" and report["counts"] == {"fail": 1}
    assert {row["actual_offset"] for row in report["observations"]} == {8}


def test_bad_ordinary_tu_stays_unknown_after_a_successful_neighbor_probe(unobserved_project):
    add_unit(unobserved_project)
    class BrokenSibling(Compiler):
        def __call__(self, path, *_): return b"broken" if path.name == "g.cc" else ABSENT
    report = check(unobserved_project, BrokenSibling(unobserved_project))
    assert report["result"] == "unknown" and report["expectations"][0]["observed_definitions"] == 2


def test_no_literal_include_host_is_unknown_not_permission_to_inject_a_header(unobserved_project):
    path = unobserved_project / "code/f.cc"
    path.write_text(path.read_text().replace('#include "types.h"', '#include HEADER_MACRO'))
    compiler = Compiler(unobserved_project)
    report = check(unobserved_project, compiler)
    assert report["result"] == "unknown" and not compiler.calls
    assert report["unobserved_witnesses"]["hosts"]["Widget"]["unit"] is None


def test_changed_header_refuses_mixed_epoch_witnesses(unobserved_project):
    class Mutating(Compiler):
        def overlay(self, *args):
            result = super().overlay(*args)
            path = self.root / "code/types.h"
            path.write_text(path.read_text() + "\n")
            return result
    with pytest.raises(ValueError, match="changed"):
        check(unobserved_project, Mutating(unobserved_project))


def test_native_cli_honors_project_layout_policy(unobserved_project, monkeypatch, capsys):
    import match_recovery.layout_check as module
    path = unobserved_project / "recovery/project.json"
    config = json.loads(path.read_text())
    config["layout"] = {"conventional_offsets": True, "witness_unobserved": True}
    path.write_text(json.dumps(config))
    monkeypatch.setattr(module, "Compiler", lambda root, **_: Compiler(root))
    assert main(["check-layout", "--root", str(unobserved_project), "--summary"]) == 0
    report = json.loads(capsys.readouterr().out)
    assert report["unobserved_witnesses"]["enabled"] and report["conventional_offsets"]["enabled"]


@pytest.mark.parametrize("policy", [{"conventional_offsets": "yes"}, {"unrecognized": True}, []])
def test_invalid_layout_policy_is_refused(unobserved_project, policy):
    path = unobserved_project / "recovery/project.json"
    config = json.loads(path.read_text()); config["layout"] = policy; path.write_text(json.dumps(config))
    with pytest.raises(ValueError, match="layout policy"):
        check_layout(unobserved_project)


def test_witness_modes_are_mutually_exclusive(unobserved_project):
    with pytest.raises(ValueError, match="mutually exclusive"):
        check(unobserved_project, Compiler(unobserved_project), witness_missing=True)
