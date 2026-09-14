"""Physical live link epochs, not serialized observation authority."""

from copy import deepcopy
import json

import pytest

from match_recovery import linked_context as native
from match_recovery.linked_initializers import observe_build_initializers
from test_binary import coff_fixture
from test_linked_initializers import build, stamp, update_artifact
from test_toolchain_components import archive


ROOTS = [{"owner": "unit0.obj", "symbol": "_$E1"}]


def test_initializers_match_ordinary_observation_and_reroot_reuses_native_inputs(tmp_path, monkeypatch):
    context = build(tmp_path / "project")
    expected = observe_build_initializers(context)
    observed = []
    original = native.observe_code_graph
    def inspect(objects, mapping, image, roots, **kwargs):
        observed.append((objects, mapping, image))
        return original(objects, mapping, image, roots, **kwargs)
    monkeypatch.setattr(native, "observe_code_graph", inspect)
    with native.linked_build(context) as live:
        result = live.initializers()
        assert result == {key: expected[key] for key in result}
        assert result["result"] == "pass"
        one = live.code(ROOTS)
        two = live.code([{"owner": "dependency.obj", "symbol": "_dep"}])
        assert one["result"] == two["result"] == "pass"
        assert one["nodes"][0]["symbol"] == "_$E1"
        assert two["nodes"][0]["symbol"] == "_dep"
        assert observed[-1] == observed[-2]
        assert observed[-1][0] is live.inputs.objects
        assert observed[-1][1] is live.inputs.mapping
        assert observed[-1][2] is live.inputs.image
        assert one["remaining_linked_effects"] == "unknown" and not one["ready_for_collection"]
    assert context.checks > 5


@pytest.mark.parametrize("operation", ["check", "code", "initializers"])
def test_handle_expires_even_when_original_build_remains_live(tmp_path, operation):
    context = build(tmp_path / "project")
    with native.linked_build(context) as live:
        assert live.code(ROOTS)["result"] == "pass"
    assert context.alive
    with pytest.raises(ValueError, match="expired"):
        {"check": live.check_current, "code": lambda: live.code(ROOTS), "initializers": live.initializers}[operation]()


def mutate(context, live, damage):
    if damage in {"object", "dependency", "image_file", "map_file", "log_file", "receipt_file", "tool"}:
        path = {"object": context.objects[0], "dependency": context.objects[-1],
                "image_file": context.directory / "app.exe", "map_file": context.directory / "app.map",
                "log_file": context.directory / "link.log", "receipt_file": context.directory / "link.json",
                "tool": context.root / "linker"}[damage]
        path.write_bytes(path.read_bytes() + b"changed")
    elif damage == "expired": context.alive = False
    elif damage == "image_base": live.inputs.image.base += 4096
    elif damage == "image_relocations": live.inputs.image.relocations = ()
    elif damage == "artifact_bytes": live.inputs.artifacts["pe32-i386"] += b"changed"
    elif damage == "evidence": live.inputs.evidence["arguments"].append("/FORCE")
    elif damage == "coverage": live.inputs.coverage["result"] = "unknown"
    elif damage == "records": live.inputs.records.pop()
    elif damage == "map": object.__setattr__(live.inputs.mapping, "base", 0)
    elif damage == "selected_inventory": live.inputs.objects._constant_inventory.pop()
    elif damage == "selected_bool_zero":
        record = live.inputs.objects._constant_inventory[0]
        assert record["optional_header_size"] == 0
        record["optional_header_size"] = False
    elif damage == "map_bool_zero":
        row = live.inputs.mapping.statics[0]
        assert row.offset == 0
        object.__setattr__(row, "offset", False)
    elif damage == "selected_owner": live.inputs.objects._owners["unit0.obj"][0]["candidate_count"] = 2
    else: raise AssertionError(damage)


@pytest.mark.parametrize("damage", ["object", "dependency", "image_file", "map_file", "log_file", "receipt_file",
                                    "expired", "image_base", "image_relocations", "artifact_bytes", "evidence", "coverage", "records", "map",
                                    "selected_bool_zero", "map_bool_zero"])
@pytest.mark.parametrize("at_close", [False, True])
def test_physical_and_parsed_mutations_refuse_during_use_and_close(tmp_path, damage, at_close):
    context = build(tmp_path / "project")
    with pytest.raises(ValueError):
        with native.linked_build(context) as live:
            assert live.code(ROOTS)["result"] == "pass"
            mutate(context, live, damage)
            if not at_close:
                with pytest.raises(ValueError): live.code(ROOTS)


@pytest.mark.parametrize("damage", ["object", "image_file", "image_base"])
def test_mutation_inside_observer_is_rejected_by_mandatory_final_guard(tmp_path, monkeypatch, damage):
    context = build(tmp_path / "project")
    original = native.observe_code_graph
    with pytest.raises(ValueError):
        with native.linked_build(context) as live:
            def altered(*args, **kwargs):
                result = original(*args, **kwargs)
                assert result["result"] == "pass"
                mutate(context, live, damage)
                return result
            monkeypatch.setattr(native, "observe_code_graph", altered)
            with pytest.raises(ValueError): live.code(ROOTS)


def test_mutable_returned_report_does_not_modify_live_evidence(tmp_path):
    context = build(tmp_path / "project")
    with native.linked_build(context) as live:
        first = live.initializers()
        saved = deepcopy(first)
        first["result"] = "forged"
        first["entries"].clear()
        assert live.initializers() == saved
        code = live.code(ROOTS)
        code["nodes"].clear()
        assert len(live.code(ROOTS)["nodes"]) == 1


def test_expired_original_context_cannot_open_a_linked_epoch(tmp_path):
    context = build(tmp_path / "project")
    context.alive = False
    with pytest.raises(ValueError, match="expired"):
        with native.linked_build(context): pytest.fail("expired build was admitted")


def test_ordinary_exception_still_expires_the_handle(tmp_path):
    context = build(tmp_path / "project")
    with pytest.raises(RuntimeError, match="consumer failed"):
        with native.linked_build(context) as live:
            raise RuntimeError("consumer failed")
    with pytest.raises(ValueError, match="expired"): live.check_current()


@pytest.mark.parametrize("damage", ["tool", "selected_inventory", "selected_owner"])
def test_complete_input_and_selected_parse_state_remain_guarded_until_close(tmp_path, damage):
    context = build(tmp_path / "project")
    with pytest.raises(ValueError):
        with native.linked_build(context) as live:
            assert live.code(ROOTS)["result"] == "pass"
            mutate(context, live, damage)
            with pytest.raises(ValueError): live.check_current()


@pytest.mark.parametrize("damage", ["resolved_owner", "resolved_identity", "resolved_bytes", "resolved_hash",
                                    "file_bytes", "policy_gaps", "unknown_file", "unknown_archive",
                                    "unknown_resolved", "unknown_state"])
@pytest.mark.parametrize("at_close", [False, True])
def test_lazy_caches_cannot_replace_physical_selection_evidence(tmp_path, damage, at_close):
    context = build(tmp_path / "project")
    with pytest.raises(ValueError):
        with native.linked_build(context) as live:
            assert live.code(ROOTS)["result"] == "pass"
            objects = live.inputs.objects
            resolved = objects._resolved["unit0.obj"]
            if damage == "resolved_owner": resolved["owner"] = "different.obj"
            elif damage == "resolved_identity": resolved["identity"]["path"] = str(context.objects[-1])
            elif damage == "resolved_bytes": resolved["raw"] += b"changed"
            elif damage == "resolved_hash": resolved["sha256"] = "a" * 64
            elif damage == "file_bytes": objects._files[str(context.objects[0])] += b"changed"
            elif damage == "policy_gaps": objects._constant_policy_gaps = [{"option": "/FORCE"}]
            elif damage == "unknown_file": objects._files[str(context.directory / "unknown.obj")] = b"not observed"
            elif damage == "unknown_archive": objects._archives[str(context.directory / "unknown.lib")] = object()
            elif damage == "unknown_resolved": objects._resolved["unknown.obj"] = deepcopy(resolved)
            elif damage == "unknown_state": objects.unobserved_authority = True
            else: raise AssertionError(damage)
            if not at_close:
                with pytest.raises(ValueError): live.code(ROOTS)


def archive_context(root):
    context = build(root)
    path = context.directory / "extra.lib"
    path.write_bytes(archive([("extra.obj/", coff_fixture())]))
    log = (context.directory / "link.log").read_text().replace(
        "Start Pass1\n", f"Start Pass1\n Searching {path}:\n").replace(
        "End Pass2\n", "EXTRA.lib(extra.obj)\nEnd Pass2\n")
    receipt = json.loads((context.directory / "link.json").read_text())
    receipt["files"].append({"path": str(path), "before": stamp(path.read_bytes()), "after": stamp(path.read_bytes())})
    receipt["library_searches"].append({"spelling": str(path), "path": str(path)})
    receipt["link_log_sha256"] = stamp(log.encode())["sha256"]
    update_artifact(context, "link.log", log.encode())
    update_artifact(context, "link.json", json.dumps(receipt).encode())
    return context, path


def test_lazily_resolved_physical_archive_is_supported_without_refreshing_epoch(tmp_path):
    context, path = archive_context(tmp_path / "project")
    with native.linked_build(context) as live:
        assert not live.inputs.objects._archives
        selected = live.inputs.objects.resolve("extra:extra.obj")
        assert selected["identity"] == {"path": str(path), "member": "extra.obj", "occurrence": 0}
        live.check_current()
        assert live.code(ROOTS)["result"] == "pass"


@pytest.mark.parametrize("damage", ["member_bytes", "member_added", "member_removed", "archive_object", "physical_bytes"])
def test_physical_archive_and_its_lazy_member_cache_remain_pinned(tmp_path, damage):
    context, path = archive_context(tmp_path / "project")
    with pytest.raises(ValueError):
        with native.linked_build(context) as live:
            live.inputs.objects.resolve("extra:extra.obj")
            live.check_current()
            cached = live.inputs.objects._archives[str(path)]
            if damage == "member_bytes": cached.members["extra.obj"][0] += b"changed"
            elif damage == "member_added": cached.members["forged.obj"] = [coff_fixture()]
            elif damage == "member_removed": cached.members.pop("extra.obj")
            elif damage == "archive_object": live.inputs.objects._archives[str(path)] = object()
            else: path.write_bytes(path.read_bytes() + b"changed")
            with pytest.raises(ValueError): live.check_current()
