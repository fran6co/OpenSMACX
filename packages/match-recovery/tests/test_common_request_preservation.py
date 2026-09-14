"""Driver routing only: opaque coverage hooks are intentionally controlled.

Physical proof and capability validity are exercised in
test_common_request_coverage. These tests retain real preservation history,
trial capture, compiler controls and application replica checks, but replace
generated authority/coverage and intercept their exact consumer arguments.
"""

from contextlib import contextmanager
from dataclasses import replace
import re

import pytest

from match_recovery import preservation
from match_recovery.coff_replica import object_signature
from match_recovery.pe_coff import Coff
from test_function_witness import object_bytes
from test_preservation import project, check
from test_preservation_graph_consumption import application
from test_prepared_context import prepared
from test_prepared_unit_preservation import compiler_calls, configure, change_presence, NEW_UNIT, check_unchanged


def route(monkeypatch, graph, unit, versions, *, failure=None):
    changes = [{"unit": unit, "version": version, "symbol": "_synthetic_init", "offset": 0}
               for version in versions]
    permission = {"generated_changes": changes}
    graph = replace(graph, _authorization={**graph.authorization, "permission": permission})
    monkeypatch.setattr(preservation, "generated_sources", lambda *args: changes)
    native_objects = preservation.application_objects
    native_compare = preservation.compare_outside_functions
    native_presence = preservation.compare_unit_presence
    captures, consumed, events = {}, [], []

    @contextmanager
    def objects(actual_graph):
        assert actual_graph is graph
        with native_objects(actual_graph) as compare:
            def startup(trial, obj, **fields):
                version = fields["version"]
                assert fields["unit"] == trial.unit == unit
                assert version == trial.version and version in versions
                assert fields["changes"] == [row for row in changes if row["version"] == version]
                assert tuple(fields["flags"]) == trial.flags
                assert fields["input_fingerprint"] == trial.input_fingerprint
                assert object_signature(obj) == object_signature(Coff(trial.raw))
                observed = compare(trial, obj, **{key: value for key, value in fields.items() if key != "changes"})
                assert observed["result"] == "pass"
                start, common = object(), object()
                captures[version] = (trial, obj, start, common)
                events.append(("startup", version))
                return start

            def common(trial, obj, **fields):
                version = fields["version"]
                original, original_obj, start, cap = captures[version]
                assert trial is original and obj is original_obj and fields["startup"] is start
                assert fields["unit"] == trial.unit and tuple(fields["flags"]) == trial.flags
                assert fields["input_fingerprint"] == trial.input_fingerprint
                events.append(("common", version))
                if failure == "producer" and version == versions[-1]:
                    raise ValueError("controlled common producer refusal")
                return cap

            compare.cover_startup = startup
            compare.cover_common_requests = common
            yield compare

    def consume(obj, version, start, cap):
        if cap is None:
            assert start is None
            return
        _, original, expected_start, expected_cap = captures[version]
        assert obj is original and start is expected_start and cap is expected_cap
        consumed.append(version)
        if failure == "consumer" and version == versions[-1]:
            raise ValueError("controlled common consumer refusal")

    def compare(old, new, *args, **kwargs):
        consume(old, "baseline", kwargs.pop("baseline_startup", None), kwargs.pop("baseline_common", None))
        consume(new, "current", kwargs.pop("current_startup", None), kwargs.pop("current_common", None))
        return native_compare(old, new, *args, **kwargs)

    def presence(obj, *args, **kwargs):
        consume(obj, kwargs["present"], kwargs.pop("startup", None), kwargs.pop("common", None))
        return native_presence(obj, *args, **kwargs)

    monkeypatch.setattr(preservation, "application_objects", objects)
    monkeypatch.setattr(preservation, "compare_outside_functions", compare)
    monkeypatch.setattr(preservation, "compare_unit_presence", presence)
    return graph, permission, captures, consumed, events


@pytest.mark.parametrize("failure", [None, "producer", "consumer"])
def test_driver_routes_exact_version_trials_and_propagates_common_refusal(project, monkeypatch, failure):
    graph, permission, captured, consumed, events = route(
        monkeypatch, application(project), "code/owner.cc", ("baseline", "current"), failure=failure)
    report = check(project, application=graph, permission=permission)
    assert report["result"] == ("unknown" if failure else "pass")
    assert set(captured) == {"baseline", "current"}
    assert events == [(kind, version) for kind in ("startup", "common") for version in ("baseline", "current")]
    assert consumed == ([] if failure == "producer" else ["baseline", "current"])
    row, = [row for row in report["units"] if row["unit"] == "code/owner.cc"]
    assert row["result"] == ("unknown" if failure else "pass")
    if failure:
        assert "controlled common " + failure + " refusal" in row["reason"]
    assert report["outside_scope_verification"] == "unknown" and not report["ready_for_collection"]


@pytest.mark.parametrize("remove", [False, True])
def test_added_or_removed_tu_routes_only_its_real_present_trial(prepared, compiler_calls, monkeypatch, remove):
    base = configure(prepared, initially_present=remove)
    change_presence(prepared, remove=remove)
    def emitted(name, files):
        symbols = re.findall(rb"// ORIGINAL: 0x[0-9A-Fa-f]+ (\S+)", files[name])
        return object_bytes([(symbol.decode(), b"\xc3", ()) for symbol in symbols])
    graph = application(prepared, task_name="prepared-owner", object_for=emitted)
    present = "baseline" if remove else "current"
    graph, permission, captured, consumed, events = route(monkeypatch, graph, NEW_UNIT, (present,))
    report = check_unchanged(prepared, base, application=graph, permission=permission)
    assert report["result"] == "pass" and compiler_calls
    assert set(captured) == {present} and consumed == [present]
    assert events == [("startup", present), ("common", present)]
    row, = [row for row in report["units"] if row["unit"] == NEW_UNIT]
    assert row["unit_presence"] == {"baseline": remove, "current": not remove}
