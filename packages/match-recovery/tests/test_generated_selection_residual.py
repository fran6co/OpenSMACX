"""Native selection is independent of the initializer-reachable closure.

These physical fixtures do not claim whole-change acceptance. In particular,
reachable dependency identities already pin entire object members; this gap is
about members pulled by ordinary code outside that closure.
"""

import json
import struct

import pytest

from match_recovery.generated_code_residual import _compare_contexts
from match_recovery.linked_context import linked_build
from test_binary import coff_fixture
from test_coff_inventory import imported
from test_common_request_coverage import common_context
from test_generated_storage_residual import configured_sources
from test_link_file_evidence import stamp
from test_linked_initializers import initializer_object, update_artifact
from test_slot_local_initializer_order import replace_object
from test_toolchain_components import archive


def sibling_object(*, changed=False):
    # One member has two actual independently named ordinary code definitions.
    # The externally called root is unchanged; the other definition differs.
    raw = bytearray(coff_fixture(b"\xc3" + b"\xcc" * 7 + (b"\x90\xc3" if changed else b"\xc3")))
    struct.pack_into("<I", raw, 56, 0x60500020)
    at, = struct.unpack_from("<I", raw, 8)
    raw[at:at + 8] = b"_sibling"
    raw[at + 18:at + 26] = b"_otherB\0"
    struct.pack_into("<IhH", raw, at + 26, 8, 1, 0x20)
    return bytes(raw)


def install_archive(context, *, present=True, changed=False, imports=(), entry_call=False, missing_code=False):
    member = sibling_object(changed=changed)
    members = [("sibling.obj/", member), *[("DEMO.dll/", raw) for raw in imports]]
    path = context.directory / "runtime.lib"
    path.write_bytes(archive(members))
    log = (context.directory / "link.log").read_text().replace(
        "Start Pass1\n", f"Start Pass1\n Searching {path}:\n")
    selected = (["sibling.obj"] if present else []) + ["DEMO.dll"] * len(imports)
    log = log.replace("End Pass2", "".join(f"RUNTIME.lib({name})\n" for name in selected) + "End Pass2")
    receipt = json.loads((context.directory / "link.json").read_bytes())
    receipt["files"].append({"path": str(path), "before": stamp(path.read_bytes()),
                             "after": stamp(path.read_bytes())})
    receipt["library_searches"].append({"spelling": str(path), "path": str(path)})
    receipt["link_log_sha256"] = stamp(log.encode())["sha256"]
    update_artifact(context, "link.log", log.encode())
    update_artifact(context, "link.json", json.dumps(receipt).encode())
    mapping = (context.directory / "app.map").read_text()
    image = bytearray((context.directory / "app.exe").read_bytes())
    if present:
        mapping = mapping.replace(" entry point", " 0001:00000140 _sibling 00401140 f runtime:sibling.obj\n"
                                  " 0001:00000148 _otherB 00401148 f runtime:sibling.obj\n entry point")
        image[0x340:0x350] = member[60:76]
    if entry_call:
        assert present
        raw = bytearray(coff_fixture(b"\xe8" + bytes(4) + b"\xc3", relocation=(1, 1, 20)))
        at, = struct.unpack_from("<I", raw, 8)
        raw[at:at + 8] = b"_dep\0\0\0\0"
        raw[at + 18:at + 26] = b"_sibling"
        struct.pack_into("<H", raw, at + 18 + 14, 0x20)
        replace_object(context, bytes(raw), index=1)
        image[0x3E0:0x3F0] = (b"\xe8" + struct.pack("<i", 0x401140 - 0x4011E5) + b"\xc3").ljust(16, b"\xcc")
    if missing_code:
        replace_object(context, initializer_object(b"\xe8" + bytes(4) + b"\xc3", reference="_absent"), index=0)
        image[0x200:0x210] = (b"\xe8" + struct.pack("<i", 0x401140 - 0x401005) + b"\xc3").ljust(16, b"\xcc")
    update_artifact(context, "app.map", mapping.encode())
    update_artifact(context, "app.exe", bytes(image))


def selected_context(root, **options):
    return common_context(root, witness=False, damage=lambda context: install_archive(context, **options))[0]


def assert_closures_pass(result):
    assert result["result"] == "pass", result
    assert result["storage_residual"]["result"] == "pass", result
    assert result["preserved_producers"] == 1  # Only the unchanged generated root.


def test_unchanged_physical_selected_family_does_not_claim_global_retention(tmp_path):
    result = _compare_contexts(selected_context(tmp_path / "old", entry_call=True),
                               selected_context(tmp_path / "new", entry_call=True), [])
    assert_closures_pass(result)
    selection = result["input_selection"]
    assert selection["dependency_families"] == "pass", selection
    assert selection["result"] == "unknown"  # Not all retained sections/COMDAT choices.


def test_unrelated_sibling_change_is_visible_despite_exact_initializer_closure(tmp_path):
    before = selected_context(tmp_path / "old", entry_call=True)
    after = selected_context(tmp_path / "new", entry_call=True, changed=True)
    # Independently check the ordinary entry really calls the selected member;
    # it is not among the generated startup roots under comparison.
    for context in (before, after):
        with linked_build(context) as live:
            code = live.code([{"owner": "dependency.obj", "symbol": "_dep"},
                              {"owner": "runtime:sibling.obj", "symbol": "_otherB"}])
            assert code["result"] == "pass", code
    result = _compare_contexts(before, after, [])
    assert_closures_pass(result)
    assert result["input_selection"]["dependency_families"] == "fail", result


@pytest.mark.parametrize("added", [False, True])
def test_added_or_removed_selected_family_is_not_erased_by_unreachability(tmp_path, added):
    before = selected_context(tmp_path / "old", present=not added, entry_call=not added)
    after = selected_context(tmp_path / "new", present=added, entry_call=added)
    with linked_build(after if added else before) as live:
        assert live.code([{"owner": "dependency.obj", "symbol": "_dep"}])["result"] == "pass"
    result = _compare_contexts(before, after, [])
    assert_closures_pass(result)
    assert result["input_selection"]["dependency_families"] == "fail", result


@pytest.mark.parametrize("duplicate", [False, True])
def test_ambiguous_short_import_families_do_not_choose_a_candidate(tmp_path, duplicate):
    candidates = (imported(), imported() if duplicate else imported("_Other@8"))
    before = selected_context(tmp_path / "old", imports=candidates)
    after = selected_context(tmp_path / "new", imports=candidates)
    from match_recovery.linked_input_selection import observe_input_selection
    for context in (before, after):
        with linked_build(context) as live:
            observed = observe_input_selection(context, live.inputs.objects)
            ambiguous = [row for row in observed["families"] if row["selection"] == "unknown"]
            assert len(ambiguous) == 2
            for family in ambiguous:
                assert len(family["candidates"]) == 2
                assert {row["input"]["occurrence"] for row in family["candidates"]} == {0, 1}
            assert observed["selected_input_identity"] == "unknown"
    result = _compare_contexts(before, after, [])
    selection = result["input_selection"]
    assert selection["result"] == "unknown"
    # Equality of the complete ambiguity is not proof which import was retained.
    assert selection["dependency_families"] == "pass"
    assert all(len(rows) == 2 for rows in selection["ambiguous_families"].values())
    assert selection["retained_section_selection"] == "unknown"


def test_selection_change_survives_later_unresolvable_code_context(tmp_path):
    result = _compare_contexts(selected_context(tmp_path / "old", present=False),
                               selected_context(tmp_path / "new", missing_code=True), [])
    assert result["result"] == "unknown", result
    assert result["input_selection"]["dependency_families"] == "fail", result
    assert "no unique independent map placement" in result["reason"]
