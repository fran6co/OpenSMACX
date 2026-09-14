"""Captured legacy REF event grammar and framing, never native permission."""

from hashlib import sha256

import pytest

from match_recovery.msvc_discard import parse_discard_log
from match_recovery.msvc_icf import parse_icf_log
from test_link_log import LOG as NESTED
from test_msvc_icf import GROUP


ROWS = '''    Discarded ___no_lead_zeros
    Discarded ___alternate_form
    Discarded "void __cdecl unused(void)" (?unused@@YAXXZ) from sample.cpp.obj
    Discarded _unused from path\\other.obj
    Discarded ".idata$6" from kernel32.lib(KERNEL32.dll)
'''


def transcript(rows=ROWS, *, icf=True):
    return ("Start Pass1\nEnd Pass1\n" + rows
            + (GROUP + "ICF total savings: 16 bytes\n" if icf else "")
            + "Start Pass2\nsample.cpp.obj\npath\\other.obj\nEnd Pass2\n").encode()


@pytest.mark.parametrize("icf", [False, True])
def test_actual_forms_preserve_original_labels_and_unresolved_ownerless_events(icf):
    data = transcript(icf=icf)
    observed = parse_discard_log(data)
    assert observed["result"] == "pass" and observed["sha256"] == sha256(data).hexdigest()
    assert observed["row_count"] == 5 and observed["ownerless_count"] == 2
    rows = observed["rows"]
    assert [row["form"] for row in rows] == ["ownerless", "ownerless", "decorated", "plain", "quoted"]
    assert [row["symbol"] for row in rows[:2]] == ["___no_lead_zeros", "___alternate_form"]
    assert all(row["owner"] is None for row in rows[:2])
    assert rows[2]["symbol"] == "?unused@@YAXXZ" and rows[2]["display"] == "void __cdecl unused(void)"
    assert rows[2]["owner"] == {"label": "sample.cpp.obj", "library": None, "member": "sample.cpp.obj"}
    assert rows[3]["owner"]["member"] == "path\\other.obj"
    assert rows[4]["owner"] == {"label": "kernel32.lib(KERNEL32.dll)", "library": "kernel32.lib", "member": "KERNEL32.dll"}
    assert rows[4]["symbol"] == ".idata$6"  # Still a reported label, not a section identity.
    assert rows[2]["line"] == 5 and rows[2]["text"].startswith("Discarded ")
    assert observed["phase"]["executable_pass1_end_line"] == 2
    assert all(row["phase"] == "executable_ref" and row["input_identity"] == "unknown"
               and row["selection_verification"] == "unknown" for row in rows)
    assert observed["input_identity"] == observed["selection_verification"] == "unknown"
    assert not observed["ready_for_collection"] and not observed["persisted"]
    assert observed["icf_groups"] == parse_icf_log(data)["groups"]


def test_repeated_rows_keep_order_line_numbers_and_multiplicity():
    row = 'Discarded "_unused" from same.obj\n'
    report = parse_discard_log(transcript(row * 3))
    assert report["row_count"] == 3
    assert [item["symbol"] for item in report["rows"]] == ["_unused"] * 3
    assert [item["line"] for item in report["rows"]] == [3, 4, 5]


@pytest.mark.parametrize("suffix", ["", "ICF total savings: 0 bytes\n"])
def test_empty_observation_does_not_claim_retention_or_no_ref_elimination(suffix):
    report = parse_discard_log(transcript(suffix, icf=False))
    assert report["rows"] == [] and report["row_count"] == 0
    assert report["selection_verification"] == "unknown"


@pytest.mark.parametrize("separator", ["\n", "\r\n"])
def test_nested_export_generation_preserves_outer_executable_window(separator):
    text = NESTED.replace("Start Pass2", ROWS + "Start Pass2")
    report = parse_discard_log(text.replace("\n", separator).encode())
    assert report["row_count"] == 6  # Preserve the original fixture's existing discard too.
    assert report["rows"][0]["line"] > report["phase"]["executable_pass1_end_line"]


@pytest.mark.parametrize("phase", ["before", "pass1", "nested_export", "between_export_passes", "pass2", "after"])
def test_discard_rows_outside_executable_ref_window_refuse(phase):
    row = "Discarded _unused from sample.obj\n"
    if phase == "before":
        text = row + NESTED
    elif phase == "pass1":
        text = NESTED.replace("Start Pass1\n", "Start Pass1\n" + row, 1)
    elif phase == "nested_export":
        text = NESTED.replace("End Generating .exp file", row + "End Generating .exp file")
    elif phase == "between_export_passes":
        text = NESTED.replace("Start Generating .exp file", "Start Generating .exp file\n" + row)
    elif phase == "pass2":
        text = NESTED.replace("End Pass2", row + "End Pass2")
    else:
        text = NESTED + row
    with pytest.raises(ValueError):
        parse_discard_log(text.encode())


@pytest.mark.parametrize("position", ["selected", "replaced", "after_icf"])
def test_ref_event_cannot_be_smuggled_through_icf_group_or_its_terminator(position):
    data = transcript()
    row = b"Discarded _unused from sample.obj\n"
    if position == "selected":
        data = data.replace(b"Replaced symbol(s):", row + b"Replaced symbol(s):")
    elif position == "replaced":
        data = data.replace(b"ICF total savings:", row + b"ICF total savings:")
    else:
        data = data.replace(b"Start Pass2", row + b"Start Pass2")
    with pytest.raises(ValueError):
        parse_discard_log(data)


@pytest.mark.parametrize("line", [
    "Discarded", "Discarded ", "Discarded:_unused", "discarded _unused from a.obj",
    "Discardedly _unused", "Discarded _unused from", "Discarded _unused from bad(nested(member)).lib",
    'Discarded "unclosed from sample.obj', 'Discarded "a declaration" from sample.obj',
    'Discarded "display" (name extra) from sample.obj', 'Discarded _unused from "sample.obj"',
    'Discarded _unused from a\tname.obj', 'Discarded _unused from a.obj trailing',
    'Discarded "_unused"', 'Discarded _unused extra',
])
def test_malformed_discard_prefix_and_ambiguous_target_never_yield_partial_success(line):
    with pytest.raises(ValueError):
        parse_discard_log(transcript(ROWS + line + "\n"))


@pytest.mark.parametrize("damage", [
    lambda data: data.replace(b"End Pass2\n", b""),
    lambda data: data.replace(b"End Pass1\n", b""),
    lambda data: data + data,
    lambda data: data.replace(b"Start Pass1", b"Start Pass3"),
    lambda data: data.replace(b"?unused@@YAXXZ", b"\x01unused"),
    lambda data: data.replace(b"?unused@@YAXXZ", b"\xffunused"),
    lambda data: data.replace(b"ICF total savings: 16 bytes\n", b""),
])
def test_truncated_unbalanced_or_unsupported_byte_transcript_refuses(damage):
    with pytest.raises(ValueError):
        parse_discard_log(damage(transcript()))


def test_each_bound_refuses_the_entire_observation_not_a_prefix(monkeypatch):
    from match_recovery import msvc_discard
    for name, limit in (("MAX_LOG_BYTES", 1), ("MAX_ROWS", 1), ("MAX_LINE_BYTES", 20)):
        with monkeypatch.context() as local:
            local.setattr(msvc_discard, name, limit)
            with pytest.raises(ValueError):
                parse_discard_log(transcript())
    with pytest.raises(ValueError):
        parse_discard_log(transcript("Discarded " + "x" * 4097 + "\n"))


@pytest.mark.parametrize("convert", [bytearray, memoryview, lambda value: value.decode("ascii")])
def test_mutable_or_nonbyte_inputs_cannot_supply_stable_transcript_observations(convert):
    with pytest.raises(ValueError, match="immutable"):
        parse_discard_log(convert(transcript()))


def test_physical_transcript_observation_remains_hash_bound_and_not_authority(tmp_path):
    path = tmp_path / "link.log"
    original = transcript()
    path.write_bytes(original)
    first = parse_discard_log(path.read_bytes())
    first["rows"].clear()
    assert parse_discard_log(path.read_bytes())["row_count"] == 5
    changed = original.replace(b"sample.cpp.obj", b"different.obj")
    path.write_bytes(changed)
    second = parse_discard_log(path.read_bytes())
    assert first["sha256"] != second["sha256"]
    assert second["selection_verification"] == "unknown"
    path.write_bytes(changed.removesuffix(b"End Pass2\n"))
    with pytest.raises(ValueError):
        parse_discard_log(path.read_bytes())
