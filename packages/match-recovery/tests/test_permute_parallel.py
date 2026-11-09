"""Batched/concurrent permute scheduling, compiler slot leasing and dead budget probes."""

import json
import operator
import os

import pytest

from match_recovery import permute
from match_recovery.permute import (Body, Score, budget_probes, insert_probe, mutation_sites, probe_calls,
                                    probe_helpers, probe_positions, probe_variants, search)


def compact(text):
    return " ".join(t.text for t in permute.tokenize(text) if t.kind not in ("ws", "comment"))


class FakeScorer:
    def __init__(self, wanted):
        self.wanted, self.calls, self.batches = wanted, [], []

    def __call__(self, text):
        self.calls.append(text)
        key = compact(text)
        distance = sum(1 for a, b in zip(key.split(), self.wanted.split()) if a != b) + abs(
            len(key.split()) - len(self.wanted.split()))
        return Score(distance, distance == 0)

    def many(self, texts):
        self.batches.append(len(texts))
        # Reverse evaluation order: the search must not depend on completion order.
        return list(reversed([self(text) for text in reversed(texts)]))


BODY = "x = a + b; y = c * d; z = e | f; w = g & h;"


@pytest.mark.parametrize("operators", [None, ["commute", "flip_comparison"]])
def test_batch_one_with_a_batch_scorer_is_the_serial_search(operators):
    body = BODY if operators is None else "x = a + b; if (c < d) y = 1;"
    serial = FakeScorer("never matches")
    expected = search(body, serial, iterations=60, seed=4, operators=operators)
    batched = FakeScorer("never matches")
    result = search(body, batched, iterations=60, seed=4, operators=operators,
                    batch_scorer=batched.many, batch=1)
    assert result.best_text == expected.best_text
    assert (result.compiles, result.cache_hits, result.accepted, result.iterations) == (
        expected.compiles, expected.cache_hits, expected.accepted, expected.iterations)
    assert batched.calls == serial.calls
    if operators:
        assert expected.cache_hits > 0


def test_batched_search_is_reproducible_and_bounded_by_the_batch():
    def run():
        scorer = FakeScorer("never matches")
        result = search(BODY, scorer, iterations=50, seed=9, batch_scorer=scorer.many, batch=4)
        return result.best_text, result.iterations, result.compiles, scorer.calls
    first, second = run(), run()
    assert first == second
    scorer = FakeScorer("never matches")
    result = search(BODY, scorer, iterations=50, seed=9, batch_scorer=scorer.many, batch=4, minimize=False)
    assert max(scorer.batches) <= 4 and result.iterations == 50
    assert result.compiles - 1 + result.cache_hits + result.no_sites == 50     # every iteration accounted
    assert result.compiles == sum(scorer.batches)


def test_batched_search_reaches_exact_and_minimizes_in_batches():
    scorer = FakeScorer("x = b + a ; y = d * c ; z = e | f ; w = g & h ;")
    result = search(BODY, scorer, iterations=400, seed=3, batch_scorer=scorer.many, batch=5)
    assert result.best.exact and result.stop_reason == "exact"
    assert compact(result.best_text) == "x = b + a ; y = d * c ; z = e | f ; w = g & h ;"


def test_batch_must_be_positive():
    with pytest.raises(ValueError, match="batch"):
        search("x = a;", lambda text: Score(1, False), iterations=1, batch=0)


# ------------------------------------------------------------- budget probes

PROBE_BODY = """
    int x = f(a);
    if (x) {
        g(x);
    }
    h();
"""


def test_probe_positions_cover_statement_boundaries_and_block_ends():
    body = Body(PROBE_BODY)
    labels = [label for _offset, label in probe_positions(body)]
    assert len(labels) == 6     # before decl, if, g, h; the inner block end; the body end
    assert any("before `int x = f(a);`" in label for label in labels)
    assert sum("block end" in label for label in labels) == 2


def test_probe_insertion_parses_and_derives_helpers():
    offset = PROBE_BODY.index("h();")
    text = insert_probe(PROBE_BODY, offset, 3)
    assert "permute_budget_probe_3(2); /* stand-in: dead budget probe */\n    h();" in text
    Body(text)
    assert probe_calls(text) == [(offset, offset + len("permute_budget_probe_3(2); "
                                                     "/* stand-in: dead budget probe */\n    "), 3, "flat")]
    helpers = probe_helpers(text)
    assert "static int permute_budget_probe_3(int x)" in helpers
    assert helpers.count("if (x == ") == 3 and "stand-in" in helpers
    nested = insert_probe(PROBE_BODY, offset, 2, "nested")
    helpers = probe_helpers(nested)
    assert "permute_budget_probe_nested_2(int x)" in helpers and "permute_budget_probe_2(int x)" in helpers
    assert probe_helpers(PROBE_BODY) == ""


def test_budget_probe_operator_is_opt_in_and_round_trips():
    assert not any(site.operator == "budget_probe" for site in mutation_sites(PROBE_BODY))
    sites = mutation_sites(PROBE_BODY, operators=["budget_probe"])
    assert sites and all(site.operator == "budget_probe" for site in sites)
    inserted = sites[0].apply(PROBE_BODY)
    (start, end, size, form), = probe_calls(inserted)
    again = budget_probes(Body(inserted))
    removal = [site for site in again if site.description.startswith("remove")]
    assert removal and compact(removal[0].apply(inserted)) == compact(PROBE_BODY)
    assert removal[0].apply(inserted) == PROBE_BODY
    resized = [site for site in again if site.description.startswith("resize")][0].apply(inserted)
    assert len(probe_calls(resized)) == 1 and probe_calls(resized)[0][2] != size
    two = budget_probes(Body(inserted), limit=1)
    assert not any(site.description.startswith(("flat probe", "nested probe")) for site in two)


def test_probe_variants_enumerate_positions_sizes_and_forms():
    variants = probe_variants(PROBE_BODY, sizes=(1, 4), forms=("flat",))
    assert len(variants) == 6 * 2
    assert all(len(probe_calls(text)) == 1 for _description, text in variants)
    assert len({text for _description, text in variants}) == len(variants)


# --------------------------------------------------------------- slot pool

def test_slot_pool_runs_each_slot_in_its_own_process_and_environment(tmp_path):
    from match_recovery.source_permute import SlotPool
    slots = [("a", {"PERMUTE_TEST_SLOT": "one"}), ("b", {"PERMUTE_TEST_SLOT": "two"}),
             ("c", {"PERMUTE_TEST_SLOT": "three"})]
    pool = SlotPool(tmp_path, "default", slots)
    try:
        values = pool.run(os.getenv, [("PERMUTE_TEST_SLOT",)] * 12)
        assert set(values) <= {"one", "two", "three"} and len(values) == 12
        pinned = pool.run(os.getenv, [("PERMUTE_TEST_SLOT",)] * 3, slot=1)
        assert pinned == ["two"] * 3
        pids = {pool.run(os.getpid, [()], slot=number)[0] for number in range(3)}
        assert len(pids) == 3 and os.getpid() not in pids
        assert os.environ.get("PERMUTE_TEST_SLOT") is None
        with pytest.raises(ZeroDivisionError):
            pool.run(operator.truediv, [(1, 1), (1, 0)])
    finally:
        pool.close()


def project(tmp_path, variables=("VC6_PREFIX",)):
    (tmp_path / "recovery").mkdir()
    (tmp_path / "recovery/project.json").write_text(json.dumps(
        {"schema_version": 1, "compiler": {"environment": list(variables)}}))
    return tmp_path


def test_checked_slots_require_declared_distinct_environments(tmp_path):
    from match_recovery.source_permute import _checked_slots
    root = project(tmp_path)
    good = [("a", {"VC6_PREFIX": "/p/a"}), ("b", {"VC6_PREFIX": "/p/b"})]
    assert _checked_slots(root, good) == good
    with pytest.raises(ValueError, match="distinct"):
        _checked_slots(root, [("a", {"VC6_PREFIX": "/p/a"}), ("b", {"VC6_PREFIX": "/p/a"})])
    with pytest.raises(ValueError, match="declared"):
        _checked_slots(root, [("a", {"UNDECLARED": "x"})])
    with pytest.raises(ValueError, match="same environment"):
        _checked_slots(root, [("a", {"VC6_PREFIX": "/p/a"}), ("b", {"VC6_PREFIX": "/p/b", "PATH": "/x"})])


def test_slot_option_parsing():
    from match_recovery.project_commands import _slot_environment
    assert _slot_environment("VC6_PREFIX=/a,PATH=/b") == {"VC6_PREFIX": "/a", "PATH": "/b"}
    for bad in ("VC6_PREFIX", "=x", "A=1,A=2", "A="):
        with pytest.raises(ValueError):
            _slot_environment(bad)


# --------------------------------------------------------- multi-slot leases

def pool_configuration(tmp_path, count):
    directories = []
    for index in range(count):
        directory = (tmp_path / f"slot-{index}")
        directory.mkdir()
        directories.append(directory.resolve())
    return directories, {"schema_version": 1, "slots": [
        {"name": f"slot-{index}", "directory": str(directory), "environment": {"VC6_PREFIX": "{resource}"}}
        for index, directory in enumerate(directories)]}


def test_resource_environments_lease_distinct_slots_and_release(tmp_path):
    from match_recovery.resources import ACTIVE, INHERITED, resource_environments
    (tmp_path / "root").mkdir()
    root = project(tmp_path / "root")
    directories, configuration = pool_configuration(tmp_path, 3)
    with resource_environments(configuration, [root], 2) as leased:
        assert [name for name, _ in leased] == ["slot-0", "slot-1"]
        assert [environment["VC6_PREFIX"] for _, environment in leased] == [str(d) for d in directories[:2]]
        assert all(environment[INHERITED] == "1" for _, environment in leased)
        assert INHERITED not in os.environ
        assert (directories[0] / ACTIVE).exists() and (directories[1] / ACTIVE).exists()
        assert not (directories[2] / ACTIVE).exists()
    assert not any((directory / ACTIVE).exists() for directory in directories)


def test_resource_environments_skip_quarantine_and_refuse_too_few(tmp_path):
    from match_recovery.resources import ACTIVE, resource_environments
    (tmp_path / "root").mkdir()
    root = project(tmp_path / "root")
    directories, configuration = pool_configuration(tmp_path, 3)
    (directories[0] / ACTIVE).write_text("{}")         # interrupted owner: quarantined, never reclaimed
    with resource_environments(configuration, [root], 2) as leased:
        assert [name for name, _ in leased] == ["slot-1", "slot-2"]
    with pytest.raises(TimeoutError, match="fewer than 3"):
        with resource_environments(configuration, [root], 3):
            pass
    assert (directories[0] / ACTIVE).read_text() == "{}"
    assert not (directories[1] / ACTIVE).exists() and not (directories[2] / ACTIVE).exists()
    with pytest.raises(ValueError, match="exceeds"):
        with resource_environments(configuration, [root], 4):
            pass


def test_resource_environments_refuse_nested_allocation(tmp_path, monkeypatch):
    from match_recovery.resources import INHERITED, resource_environments
    (tmp_path / "root").mkdir()
    root = project(tmp_path / "root")
    _directories, configuration = pool_configuration(tmp_path, 2)
    monkeypatch.setenv(INHERITED, "1")
    with pytest.raises(ValueError, match="nested"):
        with resource_environments(configuration, [root], 1):
            pass


def test_interrupted_multi_lease_keeps_quarantine_markers(tmp_path):
    from match_recovery.resources import ACTIVE, resource_environments
    (tmp_path / "root").mkdir()
    root = project(tmp_path / "root")
    directories, configuration = pool_configuration(tmp_path, 2)
    with pytest.raises(KeyboardInterrupt):
        with resource_environments(configuration, [root], 2):
            raise KeyboardInterrupt
    assert all((directory / ACTIVE).exists() for directory in directories)


def test_probe_helpers_land_at_file_scope_before_marker_comments():
    from match_recovery.source_permute import _helper_offset
    prefix = ("int a;\n#include <x.h>\nvoid f() {\n}  /* trailing\n comment */\n\n/*\nPurpose:\n"
              "// ORIGINAL: 0x1 f\n*/\n// LEVER: x\nvoid Win::g(int x) {")
    at = _helper_offset(prefix)
    assert prefix[at:].startswith("\n/*\nPurpose:")
    assert _helper_offset("// ORIGINAL: 0x1 g\nvoid g() {") == 0


def test_candidate_deck_expands_bodies_and_anchored_replacements():
    from match_recovery.source_permute import candidate_variants
    body = "\n    int x = a + b;\n    return x;\n"
    rows = candidate_variants({"schema_version": 1, "candidates": {
        "whole": {"body": "\n    return a + b;\n"},
        "edit": {"replace": [{"before": "a + b", "after": "b + a"}]}}}, body)
    assert rows == [("whole", "\n    return a + b;\n"), ("edit", "\n    int x = b + a;\n    return x;\n")]
    for deck, message in (
            ({"schema_version": 2, "candidates": {"x": {"body": ""}}}, "schema_version 1"),
            ({"schema_version": 1, "candidates": {"x": {"replace": [{"before": "x", "after": "y"}]}}}, "exactly once"),
            ({"schema_version": 1, "candidates": {"x": {"body": "// ORIGINAL: 0x1"}}}, "claim text"),
            ({"schema_version": 1, "candidates": {"x": {"body": "a", "replace": []}}}, "exactly one of")):
        with pytest.raises(ValueError, match=message):
            candidate_variants(deck, body)
