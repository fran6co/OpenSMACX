"""Physical ordinary seeds enter both builds without a referring code edge."""

from contextlib import ExitStack, contextmanager

import pytest

from match_recovery.generated_storage_residual import StorageClosure
from match_recovery.linked_code_ownership import identities
from match_recovery.linked_context import linked_build
from match_recovery.linked_data_ownership import section_producers
from test_generated_storage_residual import ordinary_context
from test_linked_data_ownership import fixture as data_context
from test_linked_initializers import update_artifact


def unreferenced(root, *, missing=False):
    def place(context):
        if not missing:
            mapping = (context.directory / "app.map").read_text()
            mapping += " 0002:00000084 _private 00402084 unit0.obj\n"
            update_artifact(context, "app.map", mapping.encode())
    return data_context(root, damage=place)


@contextmanager
def paired(old, new, *, limit=4096):
    with ExitStack() as stack:
        live = {version: stack.enter_context(linked_build(context))
                for version, context in (("baseline", old), ("current", new))}
        producers = {version: stack.enter_context(section_producers(native.context, native.inputs.objects))
                     for version, native in live.items()}
        yield StorageClosure(live, producers, limit=limit), live


def observed(closure, live, roots=()):
    graphs = {version: native.code(roots) for version, native in live.items()}
    assert all(graph["result"] == "pass" for graph in graphs.values())
    producers = {version: identities(native.context, graphs[version]["nodes"])
                 for version, native in live.items()}
    return closure.observe(graphs, producers)


@pytest.mark.parametrize("version", ["baseline", "current"])
def test_unreferenced_seed_is_observed_in_both_builds_and_retained_across_rounds(tmp_path, version):
    with paired(unreferenced(tmp_path / "old"), unreferenced(tmp_path / "new")) as (closure, live):
        assert observed(closure, live)["sections"] == 0
        closure.require_sections(version, iter([{"owner": "unit0.obj", "section": 1}]))
        result = observed(closure, live)
        assert result["result"] == result["input_consistency"] == "pass", result
        assert result["sections"] == 1
        for rows in result["dependencies"].values():
            row, = rows.values()
            assert (row["address"], row["size"]) == (0x402080, 8)
        closure.require_sections(version, [{"owner": "unit0.obj", "section": 1}])
        assert observed(closure, live) == result
        assert result["allocation_isolation"] == result["remaining_linked_effects"] == "unknown"
        assert not result["ready_for_collection"]


def test_absent_opposite_placement_is_unknown_and_seed_is_not_dropped(tmp_path):
    with paired(unreferenced(tmp_path / "old"), unreferenced(tmp_path / "new", missing=True)) as (closure, live):
        closure.require_sections("baseline", [{"owner": "unit0.obj", "section": 1}])
        result = observed(closure, live)
        assert result["result"] == "unknown", result
        assert len(closure.required) == 1
        assert len(result["dependencies"]["baseline"]) == 1
        assert observed(closure, live)["result"] == "unknown"


def test_seeded_callback_requires_native_code_expansion_in_both_builds(tmp_path):
    with paired(ordinary_context(tmp_path / "old", callback=True),
                ordinary_context(tmp_path / "new", callback=True)) as (closure, live):
        closure.require_sections("baseline", [{"owner": "dependency.obj", "section": 2}])
        first = observed(closure, live)
        assert first["result"] == first["code_coverage"] == "unknown", first
        identity, = first["required_code"].values()
        assert identity == {"kind": "compiled", "unit": "src/dependency.cpp", "symbol": "_dep"}
        second = observed(closure, live, [{"owner": "dependency.obj", "symbol": "_dep"}])
        assert second["result"] == second["code_coverage"] == "pass", second
        assert second["required_code"] == first["required_code"]


@pytest.mark.parametrize("row", [{"owner": "unit0.obj", "section": True},
                                 {"owner": "unit0.obj", "section": 0},
                                 {"owner": "", "section": 1},
                                 {"owner": "unit0.obj", "section": 1, "address": 0x402080},
                                 {"owner": "dependency.obj", "section": 1}])
def test_seed_identity_is_exact_and_ordinary_only(tmp_path, row):
    with paired(unreferenced(tmp_path / "old"), unreferenced(tmp_path / "new")) as (closure, live):
        with pytest.raises(ValueError):
            closure.require_sections("baseline", [row])
        assert not closure.required


def test_seed_batches_are_bounded_atomic_and_require_exact_version(tmp_path):
    with paired(unreferenced(tmp_path / "old"), unreferenced(tmp_path / "new"), limit=1) as (closure, live):
        seed = {"owner": "unit0.obj", "section": 1}
        with pytest.raises(ValueError, match="bound"):
            closure.require_sections("baseline", (dict(seed) for _ in range(2)))
        assert not closure.required
        with pytest.raises(ValueError, match="version"):
            closure.require_sections("old", [seed])
        closure.require_sections("current", [seed])
        assert len(closure.required) == 1
    with pytest.raises(ValueError, match="expired"):
        closure.require_sections("baseline", [seed])


def test_seed_iteration_cannot_hide_final_native_source_damage(tmp_path):
    old, new = unreferenced(tmp_path / "old"), unreferenced(tmp_path / "new")
    with pytest.raises(ValueError):
        with paired(old, new) as (closure, live):
            def damaged():
                yield {"owner": "unit0.obj", "section": 1}
                (old.root / "src/unit0.cpp").write_bytes(b"changed source")
            with pytest.raises(ValueError):
                closure.require_sections("baseline", damaged())
            assert not closure.required
