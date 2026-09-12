"""Bind synthetic preservation trials to live native application membership.

No compiler is run; object correspondence remains distinct from linked effects.
"""

from copy import deepcopy
from dataclasses import replace
import json

import pytest

from match_recovery.build_graph import ApplicationGraph
from match_recovery.model import digest
from match_recovery.pe_coff import Coff
from match_recovery.source_replica import snapshot_digest
from test_binary import coff_fixture
from test_build_graph_commands import Context, stamp


FLAGS = ("/c", "/O2", "/Gy", "/GX")
INPUT = "b" * 64
UNIT = "src/a.cpp"


@pytest.fixture
def graph(tmp_path):
    root = tmp_path / "coordinator"
    root.mkdir()
    old = Context(tmp_path / "baseline", [UNIT, "src/b.cpp"])
    new = Context(tmp_path / "current", [UNIT, "src/b.cpp"])
    sources = {}
    for version, context in (("baseline", old), ("current", new)):
        source_map = {}
        for row in context.proof["compilations"]:
            source = context.root / row["source"]
            source.write_bytes(version.encode() + b" synthetic source " + source.name.encode())
            capture = row["compilation"]
            source_row = next(item for item in capture["inputs"] if item["path"] == str(source))
            source_row["before"] = source_row["after"] = stamp(source.read_bytes())
            output = context.directory / row["output"]
            output.write_bytes(coff_fixture())
            capture["output"]["after"] = stamp(output.read_bytes())
            source_map[source.relative_to(context.root).as_posix()] = source.read_bytes()
        sources[version] = source_map

    def fresh():
        try:
            old.check_current()
        finally:
            new.check_current()

    return ApplicationGraph(root, "owner", "1" * 40, ("src",), old, new, sources,
                            {"result": "unknown"}, {"task": {"task_id": "owner"}, "decisions": []}, fresh)


def trial(graph, *, version="current", flags=FLAGS, raw=None):
    from match_recovery.preservation_objects import ObjectTrial

    return ObjectTrial(raw=coff_fixture() if raw is None else raw, root=graph.root,
                       task_name=graph.task_name, baseline_revision=graph.baseline_revision,
                       authorization=digest(graph.authorization),
                       source_digest=snapshot_digest(graph.source_versions[version]),
                       unit=UNIT, version=version, flags=flags, input_fingerprint=INPUT)


def compare(graph, observation=None, obj=None, **changes):
    from match_recovery.preservation_objects import compare_application_object

    observation = trial(graph) if observation is None else observation
    arguments = {"unit": observation.unit, "version": observation.version,
                 "flags": observation.flags, "input_fingerprint": observation.input_fingerprint}
    arguments.update(changes)
    return compare_application_object(graph, observation, Coff(observation.raw) if obj is None else obj, **arguments)


@pytest.mark.parametrize("version", ["baseline", "current"])
def test_each_version_requires_exact_live_object_correspondence_not_acceptance(graph, version):
    observed = trial(graph, version=version)
    report = compare(graph, observed)
    assert report["result"] == "pass"
    assert report["linked_effects"] == "unknown" and not report["ready_for_collection"]
    assert graph.baseline.checks >= 2 and graph.current.checks >= 2
    encoded = json.dumps(report)
    assert observed.raw.hex() not in encoded
    assert "object_base64" not in encoded
    assert '"raw"' not in encoded


@pytest.mark.parametrize("raw", [coff_fixture(b"\x90\xc3"),
                                 coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 1, 20))])
def test_valid_different_trial_objects_stay_unknown(graph, raw):
    report = compare(graph, trial(graph, raw=raw))
    assert report["result"] == "unknown"
    assert report["linked_effects"] == "unknown" and not report["ready_for_collection"]


def test_relocation_identity_difference_is_not_masked_by_equal_code_bytes(graph):
    raw = coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 1, 20))
    current = graph.current
    row = current.proof["compilations"][0]
    output = current.directory / row["output"]
    output.write_bytes(raw)
    row["compilation"]["output"]["after"] = stamp(raw)
    assert compare(graph, trial(graph, raw=raw))["result"] == "pass"
    changed = coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 0, 20))
    assert compare(graph, trial(graph, raw=changed))["result"] == "unknown"


@pytest.mark.parametrize("field,value", [
    ("task_name", "different-task"), ("baseline_revision", "2" * 40),
    ("authorization", "3" * 64), ("source_digest", "4" * 64),
    ("unit", "src/b.cpp"), ("version", "baseline"),
    ("flags", ("/c", "/Od")), ("input_fingerprint", "5" * 64),
])
def test_trial_provenance_cannot_be_reused_for_another_observation(graph, field, value):
    original = trial(graph)
    altered = replace(original, **{field: value})
    with pytest.raises(ValueError):
        compare(graph, altered, unit=UNIT, version="current", flags=FLAGS, input_fingerprint=INPUT)


def test_another_original_root_cannot_borrow_the_graph(graph, tmp_path):
    root = tmp_path / "other-root"
    root.mkdir()
    with pytest.raises(ValueError):
        compare(graph, replace(trial(graph), root=root))


@pytest.mark.parametrize("field,value", [("unit", "src/b.cpp"), ("version", "baseline"),
                                       ("flags", ("/c", "/O2", "/Gy", "/Oy-", "/GX")),
                                       ("input_fingerprint", "6" * 64)])
def test_consumer_request_cannot_change_trial_identity(graph, field, value):
    with pytest.raises(ValueError):
        compare(graph, **{field: value})


def test_source_digest_covers_whole_source_version_not_only_the_compiled_unit(graph):
    observed = trial(graph)
    sources = graph.source_versions
    sources["current"]["src/b.cpp"] += b" changed neighbor"
    changed_graph = replace(graph, _sources=sources)
    with pytest.raises(ValueError):
        compare(changed_graph, observed)


@pytest.mark.parametrize("damage", ["code", "symbol", "raw"])
def test_mutable_parsed_coff_cannot_diverge_from_the_recorded_raw_object(graph, damage):
    observed = trial(graph)
    obj = Coff(observed.raw)
    if damage == "code":
        obj.sections = (replace(obj.sections[0], data=b"\x90" + obj.sections[0].data[1:]),)
    elif damage == "symbol":
        obj.symbols[0] = replace(obj.symbols[0], name="_different")
    else:
        observed = replace(observed, raw=coff_fixture(b"\x90\xc3"))
    with pytest.raises(ValueError):
        compare(graph, observed, obj)


@pytest.mark.parametrize("damage", ["duplicate_source", "missing_source", "duplicate_compilation", "missing_compilation",
                                  "duplicate_link", "missing_link", "receipt_digest", "receipt_size", "object_changed"])
def test_incomplete_native_member_or_output_receipt_never_passes(graph, damage):
    current = graph.current
    row = current.proof["compilations"][0]
    if damage == "duplicate_source":
        current.proof["sources"].append(current.proof["sources"][0])
    elif damage == "missing_source":
        current.proof["sources"].pop(0)
    elif damage == "duplicate_compilation":
        current.proof["compilations"].append(deepcopy(row))
    elif damage == "missing_compilation":
        current.proof["compilations"].pop(0)
    elif damage in {"duplicate_link", "missing_link"}:
        link = current.binding["link"]
        if damage == "missing_link":
            link["description"]["operands"].pop(0)
        else:
            index = len(link["arguments"])
            link["arguments"].append(link["arguments"][0])
            link["description"]["operands"].append({**link["description"]["operands"][0], "argument": index})
    elif damage == "receipt_digest":
        row["compilation"]["output"]["after"]["sha256"] = "7" * 64
    elif damage == "receipt_size":
        row["compilation"]["output"]["after"]["size"] += 1
    else:
        (current.directory / row["output"]).write_bytes(coff_fixture(b"\x90\xc3"))
    with pytest.raises(ValueError):
        compare(graph)
    assert graph.baseline.checks >= 2 and graph.current.checks >= 2


@pytest.mark.parametrize("side", ["baseline", "current"])
@pytest.mark.parametrize("when", [1, 2])
def test_both_live_contexts_are_rechecked_even_when_one_refuses(graph, side, when):
    getattr(graph, side).fail_at = when
    with pytest.raises(ValueError, match="context changed"):
        compare(graph)
    assert graph.baseline.checks >= 2 and graph.current.checks >= 2


def test_every_flag_trial_requires_its_own_observed_full_object(graph):
    flags = (*FLAGS, "/Oy-")
    first = trial(graph)
    assert compare(graph, first)["result"] == "pass"
    with pytest.raises(ValueError):
        compare(graph, first, flags=flags)
    second = trial(graph, flags=flags, raw=coff_fixture(b"\x90\xc3"))
    assert compare(graph, second)["result"] == "unknown"
    # Different flags may agree only through a separately bound full object,
    # never by silently treating the prior trial's PASS as this trial's result.
    independently_equal = trial(graph, flags=flags)
    assert compare(graph, independently_equal)["result"] == "pass"


def test_changed_live_graph_authorization_cannot_reuse_an_existing_trial(graph):
    observed = trial(graph)
    authorization = graph.authorization
    authorization["decisions"].append({"kind": "different authorization"})
    changed = replace(graph, _authorization=authorization)
    with pytest.raises(ValueError, match="authorization"):
        compare(changed, observed)


def test_unit_must_exist_in_the_bound_source_version(graph):
    observed = trial(graph)
    sources = graph.source_versions
    del sources["current"][UNIT]
    changed = replace(graph, _sources=sources)
    observed = replace(observed, source_digest=snapshot_digest(sources["current"]))
    with pytest.raises(ValueError, match="source version"):
        compare(changed, observed)


def test_source_presence_does_not_invent_missing_application_recipe(graph, tmp_path):
    context = Context(tmp_path / "different-membership", ["src/b.cpp"])
    changed = replace(graph, current=context)
    with pytest.raises(ValueError, match="no unique application compilation"):
        compare(changed, trial(changed))


def test_a_serialized_report_is_not_an_object_trial_or_reusable_authority(graph):
    from match_recovery.preservation_objects import compare_application_object

    report = compare(graph)
    assert report["result"] == "pass"
    with pytest.raises(ValueError, match="identity or authorization"):
        compare_application_object(graph, json.loads(json.dumps(report)), Coff(coff_fixture()),
                                   unit=UNIT, version="current", flags=FLAGS, input_fingerprint=INPUT)


@pytest.mark.parametrize("count", [1, 20])
def test_batch_indexes_each_version_once_and_has_constant_full_freshness_checks(graph, monkeypatch, count):
    from match_recovery import preservation_objects as native

    calls, original = [], native._context
    def index(context):
        calls.append(context)
        return original(context)
    monkeypatch.setattr(native, "_context", index)
    observations = [trial(graph, version=version) for version in ("baseline", "current")]
    with native.application_objects(graph) as run:
        assert calls == [graph.baseline, graph.current]
        assert graph.baseline.checks == graph.current.checks == 2
        for _ in range(count):
            for observed in observations:
                report = run(observed, Coff(observed.raw), unit=observed.unit, version=observed.version,
                             flags=observed.flags, input_fingerprint=observed.input_fingerprint)
                assert report["result"] == "pass" and report["linked_effects"] == "unknown"
        assert calls == [graph.baseline, graph.current]
        assert graph.baseline.checks == graph.current.checks == 2
    assert graph.baseline.checks == graph.current.checks == 3


def test_batch_callable_expires_after_normal_close(graph):
    from match_recovery.preservation_objects import application_objects

    observed = trial(graph)
    with application_objects(graph) as run:
        assert run(observed, Coff(observed.raw), unit=UNIT, version="current", flags=FLAGS,
                   input_fingerprint=INPUT)["result"] == "pass"
    with pytest.raises(ValueError, match="expired"):
        run(observed, Coff(observed.raw), unit=UNIT, version="current", flags=FLAGS, input_fingerprint=INPUT)


@pytest.mark.parametrize("side", ["baseline", "current"])
def test_failed_final_graph_guard_prevents_returning_a_provisional_batch_result(graph, side):
    from match_recovery.preservation_objects import application_objects

    observed = trial(graph)
    getattr(graph, side).fail_at = 3
    provisional, returned = [], []
    def operation():
        with application_objects(graph) as run:
            result = run(observed, Coff(observed.raw), unit=UNIT, version="current", flags=FLAGS,
                         input_fingerprint=INPUT)
            provisional.append(result)
            return result
    with pytest.raises(ValueError, match="context changed"):
        returned.append(operation())
    assert provisional[0]["result"] == "pass" and returned == []
    assert graph.baseline.checks == graph.current.checks == 3


def test_batch_captures_original_receipt_even_if_report_and_object_are_later_replaced(graph):
    from match_recovery.preservation_objects import application_objects

    with application_objects(graph) as run:
        observed = trial(graph)
        assert run(observed, Coff(observed.raw), unit=UNIT, version="current", flags=FLAGS,
                   input_fingerprint=INPUT)["result"] == "pass"
        raw = coff_fixture(b"\x90\xc3")
        row = graph.current.proof["compilations"][0]
        (graph.current.directory / row["output"]).write_bytes(raw)
        row["compilation"]["output"]["after"] = stamp(raw)
        with pytest.raises(ValueError, match="fresh compiler receipt"):
            run(trial(graph, raw=raw), Coff(raw), unit=UNIT, version="current", flags=FLAGS,
                input_fingerprint=INPUT)


def test_output_change_after_comparison_is_refused_by_mandatory_batch_exit(graph):
    from match_recovery.preservation_objects import application_objects

    row = graph.current.proof["compilations"][0]
    output = graph.current.directory / row["output"]
    before = output.read_bytes()
    original_guard = graph.check_current
    def guard():
        original_guard()
        if output.read_bytes() != before:
            raise ValueError("live application artifact changed")
    guarded = replace(graph, check_current=guard)
    with pytest.raises(ValueError, match="artifact changed"):
        with application_objects(guarded) as run:
            observed = trial(guarded)
            assert run(observed, Coff(observed.raw), unit=UNIT, version="current", flags=FLAGS,
                       input_fingerprint=INPUT)["result"] == "pass"
            output.write_bytes(coff_fixture(b"\x90\xc3"))
    assert output.read_bytes() != before  # Never restore over the newer edit.
    with pytest.raises(ValueError, match="expired"):
        run(observed, Coff(observed.raw), unit=UNIT, version="current", flags=FLAGS, input_fingerprint=INPUT)


@pytest.mark.parametrize("damage", ["symlink", "hardlink", "removed"])
def test_each_batch_lookup_revalidates_physical_object_identity(graph, damage):
    from match_recovery.preservation_objects import application_objects

    observed = trial(graph)
    with application_objects(graph) as run:
        assert run(observed, Coff(observed.raw), unit=UNIT, version="current", flags=FLAGS,
                   input_fingerprint=INPUT)["result"] == "pass"
        row = graph.current.proof["compilations"][0]
        output = graph.current.directory / row["output"]
        target = output.with_suffix(".same-bytes")
        output.rename(target)
        if damage == "symlink":
            output.symlink_to(target)
        elif damage == "hardlink":
            output.hardlink_to(target)
        with pytest.raises(ValueError, match="physical file"):
            run(observed, Coff(observed.raw), unit=UNIT, version="current", flags=FLAGS,
                input_fingerprint=INPUT)


def test_signature_memo_does_not_skip_validation_of_mutable_parsed_coff(graph):
    from match_recovery.preservation_objects import application_objects

    observed = trial(graph)
    obj = Coff(observed.raw)
    with application_objects(graph) as run:
        assert run(observed, obj, unit=UNIT, version="current", flags=FLAGS, input_fingerprint=INPUT)["result"] == "pass"
        obj.sections = (replace(obj.sections[0], data=b"\x90" + obj.sections[0].data[1:]),)
        with pytest.raises(ValueError, match="captured compiler output"):
            run(observed, obj, unit=UNIT, version="current", flags=FLAGS, input_fingerprint=INPUT)


def test_unrelated_caller_exception_is_preserved_while_batch_expires(graph):
    from match_recovery.preservation_objects import application_objects

    error = KeyError("caller failure")
    with pytest.raises(KeyError) as caught:
        with application_objects(graph) as run:
            raise error
    assert caught.value is error
    assert graph.baseline.checks == graph.current.checks == 3
    observed = trial(graph)
    with pytest.raises(ValueError, match="expired"):
        run(observed, Coff(observed.raw), unit=UNIT, version="current", flags=FLAGS, input_fingerprint=INPUT)
