"""Invocation-local acceleration retains physical and native index guards."""

import struct

import pytest

from match_recovery import linked_constant_batch as native
from match_recovery import linked_objects
from match_recovery.linked_objects import SelectedObjects
from test_linked_constants import constant, observed, symbol_object


def test_many_absent_requests_scan_population_only_at_batch_boundaries(tmp_path, monkeypatch):
    objects = SelectedObjects(*observed(tmp_path))
    objects._constant_population()
    physical = linked_objects._physical
    calls = []

    def counted(path):
        calls.append(str(path))
        return physical(path)

    monkeypatch.setattr(linked_objects, "_physical", counted)
    monkeypatch.setattr(native, "_physical", counted)
    with native.constant_lookups(objects) as lookup:
        initial = len(calls)
        assert initial == len(objects._constant_physical)
        for index in range(100):
            assert lookup([f"ordinary_{index}", "another"]) == {f"ordinary_{index}": None, "another": None}
        assert len(calls) == initial
    assert len(calls) == initial * 2
    with pytest.raises(ValueError, match="expired"):
        lookup(["ordinary"])


def test_real_constant_and_required_lookup_keep_native_resolution(tmp_path):
    objects = SelectedObjects(*observed(tmp_path, members=[("constant.obj/", constant(23))]))
    with native.constant_lookups(objects) as lookup:
        result = lookup(["_K", "ordinary"])
        assert result["_K"]["value"] == 23 and result["ordinary"] is None
        result["_K"]["value"] = 99
        assert lookup(["_K"])["_K"]["value"] == 23
        with pytest.raises(ValueError, match="exactly one"):
            lookup(["ordinary"], optional=False)


def test_weak_path_to_constant_cannot_take_optional_absence_fast_path(tmp_path):
    weak = symbol_object([("_K", 0, 0, 0, 2, b""),
                          ("alias", 0, 0, 0, 105, struct.pack("<II10x", 0, 3))])
    objects = SelectedObjects(*observed(tmp_path, members=[("constant.obj/", constant()), ("weak.obj/", weak)]))
    with native.constant_lookups(objects) as lookup:
        with pytest.raises(ValueError, match="weak alias"):
            lookup(["alias"])


@pytest.mark.parametrize("names,optional", [([], True), (["a", "a"], True), (["white space"], True),
                                           (["a"], 1), ({"a"}, True), ([None], True), (["a"] * 4097, True)])
def test_absent_lookup_keeps_standalone_request_validation(tmp_path, names, optional):
    objects = SelectedObjects(*observed(tmp_path))
    with native.constant_lookups(objects) as lookup:
        with pytest.raises(ValueError):
            lookup(names, optional=optional)


@pytest.mark.parametrize("target", ["app.obj", "runtime.lib"])
@pytest.mark.parametrize("exception", [False, True])
def test_late_physical_input_change_refuses_even_when_consumer_raises(tmp_path, target, exception):
    objects = SelectedObjects(*observed(tmp_path))
    with pytest.raises(ValueError, match="input population changed"):
        with native.constant_lookups(objects) as lookup:
            assert lookup(["ordinary"]) == {"ordinary": None}
            path = tmp_path / target
            path.write_bytes(path.read_bytes() + b"changed")
            if exception:
                raise RuntimeError("consumer interruption")
    with pytest.raises(ValueError, match="expired"):
        lookup(["ordinary"])


@pytest.mark.parametrize("index", ["_constant_interest", "_constant_symbols", "_constant_physical",
                                   "_constant_inventory", "_owners"])
def test_native_index_mutation_cannot_refresh_optional_absence_epoch(tmp_path, index):
    objects = SelectedObjects(*observed(tmp_path))
    with pytest.raises(ValueError, match="indices changed"):
        with native.constant_lookups(objects) as lookup:
            getattr(objects, index).clear()
            # Frozen interest still sends real constants through strict lookup;
            # even ordinary absence cannot escape the final native index guard.
            assert lookup(["ordinary"]) == {"ordinary": None}


def test_changed_policy_cache_is_not_a_population_waiver(tmp_path):
    evidence, coverage, records = observed(tmp_path)
    evidence["arguments"].append("/FORCE")
    objects = SelectedObjects(evidence, coverage, records)
    with pytest.raises(ValueError, match="policy changed"):
        with native.constant_lookups(objects) as lookup:
            objects._constant_policy_gaps = []
            assert lookup(["ordinary"]) == {"ordinary": None}


def test_exception_expires_unchanged_batch_without_suppressing_original_error(tmp_path):
    objects = SelectedObjects(*observed(tmp_path))
    with pytest.raises(RuntimeError, match="consumer"):
        with native.constant_lookups(objects) as lookup:
            raise RuntimeError("consumer")
    with pytest.raises(ValueError, match="expired"):
        lookup(["ordinary"])


def test_nonnative_objects_get_no_acceleration():
    with native.constant_lookups(object()) as lookup:
        assert lookup is None
