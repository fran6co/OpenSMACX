"""One guarded placement batch, unchanged physical proofs and refusal paths."""

from contextlib import contextmanager

import pytest

from match_recovery import linked_data_contents as contents
from match_recovery import linked_data_anchors as anchors
from match_recovery.linked_context import linked_build
from test_linked_data_contents import fixture, OWNER
from test_linked_data_seed_anchors import fixture as seeded
from test_ordinary_allocation_coverage import fixture as application


def test_shared_owner_indexes_and_anchors_produce_identical_independent_proofs(tmp_path, monkeypatch):
    inputs = seeded(tmp_path / "inputs")
    expected = [contents.verify_data_contents(inputs, OWNER, number) for number in (1, 2, 1)]
    calls = {"batch": 0, "caller": 0, "data": 0}
    original_batch = contents._data_sections
    @contextmanager
    def count_batch(value):
        calls["batch"] += 1
        with original_batch(value) as observe:
            yield observe
    monkeypatch.setattr(contents, "_data_sections", count_batch)
    for key, name in (("caller", "collect_anchors"), ("data", "collect_data_anchors")):
        original = getattr(anchors, name)
        def count(*args, _key=key, _original=original, **kwargs):
            calls[_key] += 1
            return _original(*args, **kwargs)
        monkeypatch.setattr(anchors, name, count)
    with contents.data_contents_batch(inputs) as verify:
        observed = [verify(OWNER, number) for number in (1, 2, 1)]
        assert observed == expected
        observed[1]["envelope"]["data_evidence"]["sources"][0]["address"] = 0
        observed[1]["envelope"]["anchors"].clear()
        assert verify(OWNER, 2) == expected[1]
    assert calls == {"batch": 1, "caller": 1, "data": 1}
    with pytest.raises(ValueError, match="expired"):
        verify(OWNER, 1)


@pytest.mark.parametrize("unwind", [False, True])
def test_changed_physical_inputs_refuse_close_and_expire_even_on_unwind(tmp_path, unwind):
    inputs = fixture(tmp_path)
    with pytest.raises(ValueError, match="changed"):
        with contents.data_contents_batch(inputs) as verify:
            assert verify(OWNER, 1)["result"] == "pass"
            library = tmp_path / "runtime.lib"
            library.write_bytes(library.read_bytes() + b"changed after first request")
            if unwind:
                raise RuntimeError("another failure cannot bypass final freshness")
    with pytest.raises(ValueError, match="expired"):
        verify(OWNER, 1)


@pytest.mark.parametrize("used", [False, True])
@pytest.mark.parametrize("unwind", [False, True])
def test_normal_and_exceptional_closes_expire_used_and_unused_callables(tmp_path, used, unwind):
    inputs = fixture(tmp_path)
    def run():
        nonlocal escaped
        with contents.data_contents_batch(inputs) as verify:
            escaped = verify
            if used:
                assert verify(OWNER, 1)["result"] == "pass"
            if unwind:
                raise RuntimeError("stop")
    escaped = None
    if unwind:
        with pytest.raises(RuntimeError, match="stop"):
            run()
    else:
        run()
    with pytest.raises(ValueError, match="expired"):
        escaped(OWNER, 1)


def test_unused_batch_does_not_open_storage_indexes(tmp_path, monkeypatch):
    def unexpected(*args):
        pytest.fail("unused allocation batch opened placement indexes")
    monkeypatch.setattr(contents, "_data_sections", unexpected)
    with contents.data_contents_batch(fixture(tmp_path)):
        pass


def test_failed_lazy_entry_does_not_cache_a_partial_observer_or_refusal(tmp_path, monkeypatch):
    inputs = fixture(tmp_path)
    original = contents._data_sections
    attempts = 0
    @contextmanager
    def once(value):
        nonlocal attempts
        attempts += 1
        if attempts == 1:
            raise ValueError("entry refused")
        with original(value) as observe:
            yield observe
    monkeypatch.setattr(contents, "_data_sections", once)
    with contents.data_contents_batch(inputs) as verify:
        with pytest.raises(ValueError, match="entry refused"):
            verify(OWNER, 1)
        assert verify(OWNER, 1)["result"] == "pass"
    assert attempts == 2


def test_section_refusal_does_not_erase_another_sections_damage(tmp_path):
    inputs = fixture(tmp_path, contents=b"Xbcdefghijklmnop", target=False)
    with contents.data_contents_batch(inputs) as verify:
        with pytest.raises(ValueError, match="independent placement"):
            verify(OWNER, 2)
        assert verify(OWNER, 1)["result"] == "fail"


def test_allocation_batch_closes_outside_the_row_refusal_handler(tmp_path, monkeypatch):
    context = application(tmp_path / "project")
    original = contents._verify_placed_data
    changed = False
    def corrupt_after_measurement(*args, **kwargs):
        nonlocal changed
        report = original(*args, **kwargs)
        if not changed:
            changed = True
            path = context.objects[0]
            path.write_bytes(path.read_bytes() + b"changed after measurement")
        return report
    monkeypatch.setattr(contents, "_verify_placed_data", corrupt_after_measurement)
    with pytest.raises(ValueError, match="changed"):
        with linked_build(context) as live:
            with pytest.raises(ValueError, match="changed"):
                live.ordinary_allocations()


def test_batch_final_only_refusal_precedes_code_root_consumption(tmp_path, monkeypatch):
    from match_recovery import application_code_roots

    context = application(tmp_path / "project")
    original = contents._data_sections

    @contextmanager
    def final_only(inputs):
        with original(inputs) as observe:
            yield observe
        raise ValueError("batch final-only refusal")

    def unexpected(*args, **kwargs):
        pytest.fail("code roots consumed before batch freshness succeeded")

    monkeypatch.setattr(contents, "_data_sections", final_only)
    monkeypatch.setattr(application_code_roots, "combine_roots", unexpected)
    # Physical artifacts remain intact: only the batch finalizer refuses, so
    # an enclosing native freshness failure cannot accidentally satisfy this.
    with linked_build(context) as live:
        with pytest.raises(ValueError, match="batch final-only refusal"):
            live.ordinary_allocations()


@pytest.mark.parametrize("artifact", ["app.exe", "app.map", "app.pdb"])
def test_enclosing_native_epoch_still_guards_non_input_artifacts(tmp_path, artifact):
    context = application(tmp_path / "project")
    with pytest.raises(ValueError, match="changed"):
        with linked_build(context) as live:
            owner = next(row.owner for row in live.inputs.mapping.statics if row.symbol == "_anchor")
            with contents.data_contents_batch(live.inputs) as verify:
                assert verify(owner, 1)["result"] == "pass"
                path = context.directory / artifact
                path.write_bytes(path.read_bytes() + b"changed")
            live.check_current()
