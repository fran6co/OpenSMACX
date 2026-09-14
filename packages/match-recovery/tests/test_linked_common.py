"""Common storage observations use the original guarded parsed link epoch."""

from copy import deepcopy

import pytest

from match_recovery import common_allocation
from match_recovery import linked_context as native
from test_common_preservation import build


def expected(context):
    raw = lambda name: (context.directory / name).read_bytes()
    return common_allocation.verify_common_allocations(
        raw("link.json"), raw("link.log"), raw("app.exe"), raw("app.map"), ("_g",))


@pytest.mark.parametrize("damage", [None, "conflicting_size", "missing", "overlap", "policy", "highlow", "nonzero"])
def test_common_matches_bytes_api_and_reuses_parsed_inputs(tmp_path, monkeypatch, damage):
    context = build(tmp_path / "project", damage=damage)
    wrapped = expected(context)
    with native.linked_build(context) as live:
        original = common_allocation._verify_common_inputs
        calls = []
        def observed(*args):
            calls.append(args)
            return original(*args)
        def unexpected(*args, **kwargs):
            pytest.fail("common observation repeated native input parsing")
        monkeypatch.setattr(common_allocation, "_verify_common_inputs", observed)
        monkeypatch.setattr(common_allocation, "_input_inventory", unexpected)
        monkeypatch.setattr(common_allocation, "parse_link_files", unexpected)
        monkeypatch.setattr(common_allocation, "parse_link_map", unexpected)
        monkeypatch.setattr(common_allocation, "PeImage", unexpected)
        assert live.common(("_g",)) == wrapped
        assert live.common(("_g",)) == wrapped
        for call in calls:
            assert all(actual is getattr(live.inputs, name) for actual, name in zip(
                call[:5], ("evidence", "coverage", "records", "image", "mapping"), strict=True))
        assert wrapped["original_verification"] == wrapped["runtime_verification"] == "unknown"
        assert not wrapped["ready_for_collection"] and not wrapped["persisted"]


def mutate(context, live, damage):
    if damage in {"object", "archive", "image", "map_file", "receipt", "log", "tool"}:
        path = {"object": context.root / "app.obj", "archive": context.root / "runtime.lib",
                "image": context.directory / "app.exe", "map_file": context.directory / "app.map",
                "receipt": context.directory / "link.json", "log": context.directory / "link.log",
                "tool": context.root / "linker"}[damage]
        path.write_bytes(path.read_bytes() + b"changed")
    elif damage == "expired": context.alive = False
    elif damage == "parsed_image": live.inputs.image.base += 4096
    elif damage == "parsed_map": object.__setattr__(live.inputs.mapping, "base", 0)
    elif damage == "evidence": live.inputs.evidence["arguments"].append("/FORCE")
    elif damage == "coverage": live.inputs.coverage["result"] = "unknown"
    elif damage == "records": live.inputs.records.clear()
    elif damage == "artifacts": live.inputs.artifacts["msvc-map"] += b"changed"
    else: raise AssertionError(damage)


@pytest.mark.parametrize("damage", ["object", "archive", "image", "map_file", "receipt", "log", "tool",
                                    "expired", "parsed_image", "parsed_map", "evidence", "coverage", "records", "artifacts"])
@pytest.mark.parametrize("at_close", [False, True])
def test_common_refuses_physical_and_parsed_damage_during_use_and_close(tmp_path, damage, at_close):
    context = build(tmp_path / "project")
    with pytest.raises(ValueError):
        with native.linked_build(context) as live:
            assert live.common(("_g",))["result"] == "pass"
            mutate(context, live, damage)
            if not at_close:
                with pytest.raises(ValueError): live.common(("_g",))


@pytest.mark.parametrize("damage", ["archive", "image", "parsed_image", "coverage", "expired"])
def test_common_final_guard_rejects_damage_inside_observation(tmp_path, monkeypatch, damage):
    context = build(tmp_path / "project")
    original = common_allocation._verify_common_inputs
    with pytest.raises(ValueError):
        with native.linked_build(context) as live:
            def altered(*args):
                report = original(*args)
                assert report["result"] == "pass"
                mutate(context, live, damage)
                return report
            monkeypatch.setattr(common_allocation, "_verify_common_inputs", altered)
            with pytest.raises(ValueError): live.common(("_g",))


def test_common_returned_report_cannot_mutate_native_coverage(tmp_path):
    context = build(tmp_path / "project")
    with native.linked_build(context) as live:
        report = live.common(("_g",))
        saved = deepcopy(report)
        report["input_coverage"]["inputs"].clear()
        report["allocations"].clear()
        report["result"] = "forged"
        assert live.common(("_g",)) == saved


def test_common_cannot_outlive_the_parsed_epoch(tmp_path):
    context = build(tmp_path / "project")
    with native.linked_build(context) as live:
        assert live.common(("_g",))["result"] == "pass"
    assert context.alive
    with pytest.raises(ValueError, match="expired"):
        live.common(("_g",))


def test_invalid_request_still_checks_original_epoch_in_finally(tmp_path, monkeypatch):
    context = build(tmp_path / "project")
    with pytest.raises(ValueError, match="expired"):
        with native.linked_build(context) as live:
            def refused(*args):
                context.alive = False
                raise ValueError("invalid requested common")
            monkeypatch.setattr(common_allocation, "_verify_common_inputs", refused)
            with pytest.raises(ValueError, match="expired"):
                live.common(())
