"""Atomic per-key refusal caching against physical native fixture artifacts."""

import pytest

from match_recovery import linked_comdat_selection as selection
from match_recovery.linked_context import linked_build
from test_linked_comdat_selection import fixture, independent_sibling


def test_fifty_repeated_queries_prove_refused_key_once_with_same_reason(tmp_path, monkeypatch):
    context = fixture(tmp_path, winner=independent_sibling())
    original = selection._certificate_uncached
    calls = []
    def counted(state, name):
        calls.append(name)
        return original(state, name)
    monkeypatch.setattr(selection, "_certificate_uncached", counted)
    with linked_build(context) as native, selection.any_selection(native) as handle:
        reasons = []
        for _ in range(50):
            with pytest.raises(ValueError) as error:
                handle.loser("dependency.obj", 1)
            reasons.append(str(error.value))
        assert calls == ["_f"]
        assert reasons == ["ANY winner has a possible distinct-section ICF competitor"] * 50
        assert handle.certificates() == []
        assert selection._entry(handle)["refusals"] == {"_f": reasons[0]}


@pytest.mark.parametrize("mode", ["change", "clear", "insert", "replace"])
@pytest.mark.parametrize("exceptional", [False, True])
def test_refusal_cache_mutation_fails_final_even_during_exceptional_exit(tmp_path, mode, exceptional):
    context = fixture(tmp_path, winner=independent_sibling())
    with pytest.raises(ValueError, match="derived selection indices changed"):
        with linked_build(context) as native, selection.any_selection(native) as handle:
            with pytest.raises(ValueError, match="ICF competitor"):
                handle.loser("dependency.obj", 1)
            state = selection._entry(handle)
            if mode == "change":
                state["refusals"]["_f"] = "forged refusal"
            elif mode == "clear":
                state["refusals"].clear()
            elif mode == "insert":
                state["refusals"]["_other"] = "forged refusal"
            else:
                state["refusals"] = []
            if exceptional:
                raise RuntimeError("unrelated consumer error")
    with pytest.raises(ValueError, match="expired"):
        handle.loser("dependency.obj", 1)


def test_new_native_batch_rechecks_refusal_and_expires_old_handle(tmp_path, monkeypatch):
    context = fixture(tmp_path, winner=independent_sibling())
    original = selection._certificate_uncached
    calls = []
    def counted(state, name):
        calls.append(name)
        return original(state, name)
    monkeypatch.setattr(selection, "_certificate_uncached", counted)
    for _ in range(2):
        with linked_build(context) as native, selection.any_selection(native) as handle:
            assert selection._entry(handle)["refusals"] == {}
            for _ in range(3):
                with pytest.raises(ValueError, match="ICF competitor"):
                    handle.loser("dependency.obj", 1)
        with pytest.raises(ValueError, match="expired"):
            handle.loser("dependency.obj", 1)
    assert calls == ["_f", "_f"]


@pytest.mark.parametrize("error", [OSError("temporary read failure"), RuntimeError("unexpected failure"),
                                  TypeError("unexpected type"),
                                  UnicodeDecodeError("ascii", b"\xff", 0, 1, "non-ascii")])
def test_io_arbitrary_errors_and_specialized_valueerror_subclasses_are_not_cached(tmp_path, monkeypatch, error):
    context = fixture(tmp_path)
    original = selection._certificate_uncached
    calls = []
    def flaky(state, name):
        calls.append(name)
        if len(calls) <= 2:
            raise error
        return original(state, name)
    monkeypatch.setattr(selection, "_certificate_uncached", flaky)
    with linked_build(context) as native, selection.any_selection(native) as handle:
        for _ in range(2):
            with pytest.raises(type(error)) as caught:
                handle.loser("dependency.obj", 1)
            assert caught.value is error
            assert selection._entry(handle)["refusals"] == {}
        assert handle.loser("dependency.obj", 1)["disposition"] == "discarded_any_duplicate"
        assert calls == ["_f", "_f", "_f"]


def test_refused_key_is_exact_case_sensitive_and_does_not_poison_other_key(monkeypatch):
    state = {"certificates": {}, "refusals": {}, "refusal_witnesses": {}}
    calls = []
    def prove(state, name):
        calls.append(name)
        if name == "_f":
            raise ValueError("refused original key")
        return {"key": name}
    monkeypatch.setattr(selection, "_certificate_uncached", prove)
    for _ in range(2):
        with pytest.raises(ValueError, match="refused original key"):
            selection._certificate(state, "_f")
    assert selection._certificate(state, "_F") == {"key": "_F"}
    assert calls == ["_f", "_F"]


def test_cached_refusal_still_requires_final_physical_integrity(tmp_path):
    context = fixture(tmp_path, winner=independent_sibling())
    with pytest.raises(ValueError, match="changed"):
        with linked_build(context) as native, selection.any_selection(native) as handle:
            for _ in range(3):
                with pytest.raises(ValueError, match="ICF competitor"):
                    handle.loser("dependency.obj", 1)
            path = context.directory / "app.exe"
            path.write_bytes(path.read_bytes() + b"damage")
