"""Graph-level late damage cannot escape batched constant freshness guards."""

import pytest

from match_recovery import linked_code_graph as graph
from match_recovery.linked_context import linked_build
from test_linked_code_graph import call_graph, direct_graph


@pytest.mark.parametrize("damage", ["physical", "interest", "witness"])
@pytest.mark.parametrize("exception", [False, True])
def test_late_absence_lookup_damage_refuses_whole_graph(tmp_path, monkeypatch, damage, exception):
    context = call_graph(tmp_path / "project")
    original = graph.verify_code_region
    captured = []

    def verify(*args, **kwargs):
        lookup, objects = kwargs["_constants"], kwargs["objects"]
        assert callable(lookup)
        if not captured:
            captured.append(lookup)

            def altered(names, **options):
                result = lookup(names, **options)
                assert all(value is None for value in result.values())
                if damage == "physical":
                    # This input is outside the rooted helper graph.
                    path = context.directory / "entry.obj"
                    path.write_bytes(path.read_bytes() + b"changed")
                elif damage == "interest":
                    objects._constant_interest.add("injected")
                else:
                    objects._constant_physical.clear()
                if exception:
                    raise ValueError("consumer refuses this node")
                return result

            kwargs["_constants"] = altered
        return original(*args, **kwargs)

    monkeypatch.setattr(graph, "verify_code_region", verify)
    with pytest.raises(ValueError, match="population changed|indices changed"):
        direct_graph(context)
    assert len(captured) == 1
    with pytest.raises(ValueError, match="expired"):
        captured[0](["_helper"])


@pytest.mark.parametrize("damage", ["file", "resolved"])
def test_enclosing_native_epoch_still_checks_lazy_caches(tmp_path, monkeypatch, damage):
    context = call_graph(tmp_path / "project")
    original = graph.verify_code_region
    damaged = []

    def verify(*args, **kwargs):
        result = original(*args, **kwargs)
        if not damaged:
            objects = kwargs["objects"]
            assert callable(kwargs["_constants"])
            if damage == "file":
                objects._files[str(context.directory / "main.obj")] += b"changed"
            else:
                objects._resolved["main.obj"]["owner"] = "forged.obj"
            damaged.append(True)
        return result

    monkeypatch.setattr(graph, "verify_code_region", verify)
    with pytest.raises(ValueError):
        with linked_build(context) as live:
            with pytest.raises(ValueError):
                live.code(context.graph_roots)
    assert damaged
