"""Complete native-log ICF grammar controls, not ownership assertions."""

from hashlib import sha256

import pytest

from match_recovery.msvc_icf import parse_icf_log
from test_link_log import LOG as NESTED_LOG


GROUP = '''    Selected symbol:
        "void __cdecl first(void)" (?first@@YAXXZ) from original.cpp.obj
    Replaced symbol(s):
        "void __cdecl second(void)" (?second@@YAXXZ) from OTHER.lib(unit.obj)
        _third from path\\third.obj
'''


def transcript(body=GROUP, *, summary="    ICF total savings: 16 bytes\n"):
    return ("Start Pass1\nEnd Pass1\n    Discarded _unused from other.obj\n"
            + body + summary + "\nStart Pass2\noriginal.cpp.obj\nOTHER.lib(unit.obj)\nEnd Pass2\n").encode()


def test_exact_original_names_owners_order_and_report_scope_are_preserved():
    data = transcript()
    report = parse_icf_log(data)
    assert report["result"] == "pass" and report["sha256"] == sha256(data).hexdigest()
    assert report["group_count"] == 1 and report["member_count"] == 3 and report["savings_bytes"] == 16
    group, = report["groups"]
    assert group["line"] == 4
    assert group["selected"] == {"symbol": "?first@@YAXXZ", "display": "void __cdecl first(void)",
        "form": "decorated", "owner": {"label": "original.cpp.obj", "library": None, "member": "original.cpp.obj"},
        "line": 5, "text": '"void __cdecl first(void)" (?first@@YAXXZ) from original.cpp.obj'}
    assert [row["symbol"] for row in group["replaced"]] == ["?second@@YAXXZ", "_third"]
    assert group["replaced"][0]["owner"] == {"label": "OTHER.lib(unit.obj)", "library": "OTHER.lib", "member": "unit.obj"}
    assert group["replaced"][1]["owner"]["member"] == "path\\third.obj"
    assert report["input_identity"] == report["selection_verification"] == "unknown"
    assert not report["ready_for_collection"] and not report["persisted"]


def test_repeated_names_rows_and_groups_are_not_coalesced_or_given_ownership():
    group = '''Selected symbol:
".text" from RUNTIME.lib(one.obj)
Replaced symbol(s):
".text" from RUNTIME.lib(two.obj)
".text" from RUNTIME.lib(two.obj)
'''
    report = parse_icf_log(transcript(group + "\n" + group))
    assert report["group_count"] == 2 and report["member_count"] == 6
    for item in report["groups"]:
        assert item["selected"]["symbol"] == ".text" and item["selected"]["form"] == "quoted"
        assert [row["symbol"] for row in item["replaced"]] == [".text", ".text"]
        assert [row["owner"]["member"] for row in item["replaced"]] == ["two.obj", "two.obj"]


@pytest.mark.parametrize("separator", ["\n", "\r\n"])
def test_nested_export_library_pass_is_not_executable_icf_phase(separator):
    text = NESTED_LOG.replace("Start Pass2", GROUP + "ICF total savings: 16 bytes\nStart Pass2")
    report = parse_icf_log(text.replace("\n", separator).encode())
    assert report["group_count"] == 1


@pytest.mark.parametrize("summary", ["", "ICF total savings: 0 bytes\n"])
def test_no_groups_is_an_empty_observation_not_a_no_folding_proof(summary):
    report = parse_icf_log(transcript("", summary=summary))
    assert report["groups"] == [] and report["selection_verification"] == "unknown"


@pytest.mark.parametrize("body", [
    "Selected symbol:\n", "Selected symbol:\n_f from a.obj\n", "Replaced symbol(s):\n_g from a.obj\n",
    "Selected symbol:\nReplaced symbol(s):\n_g from a.obj\n",
    "Selected symbol:\n_f from a.obj\nReplaced symbol(s):\n",
    "Selected symbol:\n_f from a.obj\n_g from b.obj\nReplaced symbol(s):\n_h from c.obj\n",
    GROUP.replace("Replaced symbol(s):", "Replaced symbols:"), GROUP.replace("Selected symbol:", "selected symbol:"),
    GROUP.replace("?first@@YAXXZ", "?first@@YAXXZ extra"),
    GROUP.replace('"void __cdecl first(void)" (?first@@YAXXZ)', '"void __cdecl first(void)"'),
    GROUP.replace("original.cpp.obj", "bad(nested(member)).lib"),
    GROUP.replace("original.cpp.obj", "bad\tname.obj"),
    GROUP.replace("original.cpp.obj", '"quoted.obj"'),
    GROUP + "unrecognized diagnostic\n", GROUP + "Replaced symbol(s):\n_g from a.obj\n",
    GROUP + "Selected symbol:\n", '_orphan from original.cpp.obj\n' + GROUP,
    '".text" from RUNTIME.lib(one.obj)\n' + GROUP,
])
def test_malformed_partial_ambiguous_or_orphan_groups_refuse(body):
    with pytest.raises(ValueError):
        parse_icf_log(transcript(body))


@pytest.mark.parametrize("summary", ["", "ICF total savings: -1 bytes\n", "ICF total savings: 01 bytes\n",
    "ICF total savings: 1 byte\n", "ICF total savings: 1 bytes extra\n", "icf total savings: 1 bytes\n",
    "ICF total savings: 1000000000000 bytes\n", "ICF total savings: 1 bytes\nICF total savings: 1 bytes\n",
    "ICF total savings: 1 bytes\n" + GROUP, "ICF total savings: 1 bytes\n_g from a.obj\n"])
def test_summary_is_unique_complete_and_terminates_groups(summary):
    with pytest.raises(ValueError):
        parse_icf_log(transcript(summary=summary))


@pytest.mark.parametrize("phase", ["before", "pass1", "export", "after"])
def test_group_markers_outside_executable_optimization_window_refuse(phase):
    body = GROUP + "ICF total savings: 16 bytes\n"
    if phase == "before":
        text = body + NESTED_LOG
    elif phase == "pass1":
        text = NESTED_LOG.replace("Start Pass1\n", "Start Pass1\n" + body, 1)
    elif phase == "export":
        text = NESTED_LOG.replace("End Generating .exp file", body + "End Generating .exp file")
    else:
        text = NESTED_LOG + body
    with pytest.raises(ValueError):
        parse_icf_log(text.encode())


@pytest.mark.parametrize("damage", [lambda data: data.replace(b"End Pass2\n", b""), lambda data: data + data,
    lambda data: data.replace(b"End Pass1", b"End Pass3"), lambda data: data.replace(b"_third", b"\x01third"),
    lambda data: data.replace(b"_third", b"\xffthird")])
def test_complete_native_framing_is_required_before_any_group_is_returned(damage):
    with pytest.raises(ValueError):
        parse_icf_log(damage(transcript()))


def test_positive_savings_without_any_group_cannot_claim_a_complete_observation():
    with pytest.raises(ValueError, match="lack reported groups"):
        parse_icf_log(transcript(""))


def test_bounds_refuse_instead_of_truncating_observations(monkeypatch):
    import match_recovery.msvc_icf as module
    for name, limit in (("MAX_LOG_BYTES", 1), ("MAX_ROWS", 2), ("MAX_GROUPS", 0)):
        with monkeypatch.context() as local:
            local.setattr(module, name, limit)
            with pytest.raises(ValueError):
                parse_icf_log(transcript())


def test_mutable_input_is_not_a_stable_log_observation():
    with pytest.raises(ValueError, match="immutable"):
        parse_icf_log(bytearray(transcript()))
