"""Whole verified callers bind private data; fixup reports alone never do."""

from copy import deepcopy
from hashlib import sha256
import json
import struct

import pytest

from match_recovery import linked_context
from match_recovery.msf import SmallMsf
from match_recovery.pe_coff import Coff
from test_link_file_evidence import stamp
from test_linked_fixups import fixture as physical_fixture, private_object
from test_linked_initializers import Context, update_artifact
from test_linked_storage_controls import map_bytes
from test_msf import small_msf
from test_pdb_fixups import parts


def fixture(root, **options):
    """Native physical epoch; options are test_linked_fixups.fixture options."""
    root.mkdir()
    obj, objects, image, fixups, mapping = physical_fixture(root, **options)
    evidence = deepcopy(objects._constant_evidence)
    library = root / "runtime.lib"
    log = ("Start Pass1\n    Searching " + str(library) + ":\n"
           "Start Generating .exp file\nStart Pass1\nEnd Pass1\nEnd Generating .exp file\nEnd Pass1\n"
           "Start Pass2\napp.exp\napp.obj\nRUNTIME.lib(runtime.obj)\n"
           "RUNTIME.lib(DEMO.dll)\nRUNTIME.lib(DEMO.dll)\nEnd Pass2\n").encode()
    assert stamp(log)["sha256"] == evidence["link_log_sha256"]
    _, streams = parts(tuple((row.kind, 0, site, row.target_rva) for site, row in fixups.items()))
    streams[5] = image.data[0x178:0x1C8]
    pdb = small_msf(streams)
    assert sha256(pdb).hexdigest() == fixups.pdb_sha256
    raw_image = bytearray(image.data)
    # The older low-level fixture verifies a caller independently of the
    # application entry point; the native context also validates that map row.
    struct.pack_into("<I", raw_image, 0x98 + 16, 0x1000)
    directory = root / "observed"
    directory.mkdir()
    rows = []
    for name, kind, raw in [("app.exe", "pe32-i386", bytes(raw_image)), ("app.pdb", "msvc-pdb", pdb),
                            ("app.map", "msvc-map", map_bytes(mapping)), ("link.log", "msvc-link-log", log),
                            ("link.json", "link-input-files", json.dumps(evidence).encode())]:
        (directory / name).write_bytes(raw)
        rows.append({"path": name, "format": kind, "result": "pass", **stamp(raw)})
    context = Context(root, directory, {"result": "pass", "compile_link": {"result": "pass"}, "outputs": rows})
    context.objects = [root / "app.obj"]
    return context


def replace_object(context, raw):
    path = context.root / "app.obj"
    path.write_bytes(raw)
    evidence = json.loads((context.directory / "link.json").read_bytes())
    row = next(row for row in evidence["files"] if row["path"] == str(path))
    row["before"] = row["after"] = stamp(raw)
    update_artifact(context, "link.json", json.dumps(evidence).encode())


def replace_fixups(context, records):
    msf = SmallMsf((context.directory / "app.pdb").read_bytes())
    streams = [msf.stream(index) for index in range(msf.stream_count)]
    streams[4] = b"".join(struct.pack("<HHII", *row) for row in records)
    update_artifact(context, "app.pdb", small_msf(streams))


def replace_image(context, raw):
    # Preserve image/PDB section-header binding even for coherent header edits.
    msf = SmallMsf((context.directory / "app.pdb").read_bytes())
    streams = [msf.stream(index) for index in range(msf.stream_count)]
    streams[5] = bytes(raw[0x178:0x1C8])
    update_artifact(context, "app.exe", bytes(raw))
    update_artifact(context, "app.pdb", small_msf(streams))


def replace_map(context, text):
    update_artifact(context, "app.map", text.encode() if isinstance(text, str) else text)


def observe(context):
    with linked_context.linked_build(context) as live:
        return live.data_section("app.obj", 3)


def test_real_bundle_caller_proves_private_data_placement_without_data_map_label(tmp_path):
    context = fixture(tmp_path / "project")
    with linked_context.linked_build(context) as live:
        report = live.data_section("app.obj", 3)
        assert (report["placement"], report["address"], report["size"]) == ("pass", 0x402100, 16)
        assert report["input"] == {"path": str(context.root / "app.obj")}
        assert report["object_sha256"] == sha256(private_object()).hexdigest()
        assert report["anchors"]
        assert {row["referring_site"] for row in report["anchors"]} == {0x401006, 0x40100B}
        assert all(row["placement"] == "verified_caller_pdb_fixup" and row["object_index"] == 8
                   and row["object_section"] == 3 and row["object_offset"] == 0
                   and row["referring_symbol"] == "_f" and row["referring_object_index"] == 2
                   and row["referring_section"] == 1 and row["relocation_kind"] == "absolute32"
                   and len(row["caller_fingerprint"]) == 64 for row in report["anchors"])
        assert report["contents"] == report["allocation_isolation"] == "unknown"
        assert not report["ready_for_collection"]


@pytest.mark.parametrize("position", [512, 527, 544, 559])
def test_any_root_or_associated_caller_byte_damage_cannot_supply_an_anchor(tmp_path, position):
    context = fixture(tmp_path / "project")
    raw = bytearray((context.directory / "app.exe").read_bytes())
    raw[position] ^= 1
    replace_image(context, raw)
    with pytest.raises(ValueError):
        observe(context)


@pytest.mark.parametrize("missing_site", [0x1001, 0x1006, 0x100B])
def test_incomplete_private_or_associated_fixup_evidence_cannot_place_storage(tmp_path, missing_site):
    context = fixture(tmp_path / "project")
    records = [(20, 0, 0x1001, 0x1020), (6, 0, 0x1006, 0x2100), (6, 0, 0x100B, 0x2100)]
    replace_fixups(context, [row for row in records if row[2] != missing_site])
    with pytest.raises(ValueError):
        observe(context)


def test_valid_linker_target_without_matching_actual_operand_cannot_place_data(tmp_path):
    context = fixture(tmp_path / "project")
    replace_fixups(context, [(20, 0, 0x1001, 0x1020), (6, 0, 0x1006, 0x2104), (6, 0, 0x100B, 0x2104)])
    with pytest.raises(ValueError):
        observe(context)


@pytest.mark.parametrize("options", [{"second_target": 0x2104}, {"duplicate": True},
    {"map_row": " 0002:00000100 private 00402100 other.obj"}])
def test_contradictory_targets_aliases_and_other_owner_map_rows_do_not_supply_anchors(tmp_path, options):
    with pytest.raises(ValueError):
        observe(fixture(tmp_path / "project", **options))


def test_already_mapped_target_does_not_receive_a_caller_derived_anchor(tmp_path):
    context = fixture(tmp_path / "project", map_row=" 0002:00000104 private 00402104 app.obj")
    report = observe(context)
    # Existing map placement is a separate proof path; this collector does not
    # certify contradictory caller operands or invent a private fallback.
    assert report["address"] == 0x402104
    assert all(row["placement"] != "verified_caller_pdb_fixup" for row in report["anchors"])
    assert report["caller_evidence"]["callers"] == []
    assert report["caller_evidence"]["caller_coverage"] == "unknown"


def test_other_mapped_symbol_must_agree_with_verified_private_section_base(tmp_path):
    from match_recovery.linked_data_anchors import collect_anchors
    context = fixture(tmp_path / "project")
    raw = bytearray(private_object())
    symbol_start, count = struct.unpack_from("<II", raw, 8)
    string_start = symbol_start + count * 18
    raw[string_start:string_start] = struct.pack("<8sIhHBB", b"anchor", 4, 3, 0, 3, 0)
    struct.pack_into("<I", raw, 12, count + 1)
    replace_object(context, bytes(raw))
    mapping = (context.directory / "app.map").read_bytes()
    replace_map(context, mapping + b" 0002:00000108 anchor 00402108 app.obj\n")
    with linked_context.linked_build(context) as live:
        selected = live.inputs.objects.resolve("app.obj")
        proof = collect_anchors(live.inputs, selected, Coff(selected["raw"]))
        assert proof["callers"][0]["result"] == "pass"
        assert {row["address"] - row["object_offset"] for row in proof["anchors"][3]} == {0x402100}
        with pytest.raises(ValueError, match="anchors disagree"):
            live.data_section("app.obj", 3)


def test_unverified_caller_refusal_remains_visible_in_collector_diagnostics(tmp_path):
    from match_recovery.linked_data_anchors import collect_anchors
    context = fixture(tmp_path / "project")
    raw = bytearray((context.directory / "app.exe").read_bytes())
    raw[527] ^= 1
    replace_image(context, raw)
    with linked_context.linked_build(context) as live:
        selected = live.inputs.objects.resolve("app.obj")
        report = collect_anchors(live.inputs, selected, Coff(selected["raw"]))
        assert report["anchors"] == {}
        assert report["callers"] and report["gaps"]
        assert all(row["result"] != "pass" for row in report["callers"])
        assert report["caller_coverage"] == report["target_contents"] == report["allocation_isolation"] == "unknown"


def test_unmapped_caller_does_not_infer_its_location_from_fixup_sites(tmp_path):
    context = fixture(tmp_path / "project")
    text = (context.directory / "app.map").read_text()
    text = "\n".join(line for line in text.splitlines() if " _f " not in line) + "\n"
    replace_map(context, text)
    with pytest.raises(ValueError):
        observe(context)


def test_native_private_anchor_report_is_defensive_and_handle_expires(tmp_path):
    context = fixture(tmp_path / "project")
    with linked_context.linked_build(context) as live:
        expected = live.data_section("app.obj", 3)
        changed = live.data_section("app.obj", 3)
        changed["anchors"].clear()
        changed["input"]["path"] = "forged"
        assert live.data_section("app.obj", 3) == expected
    assert context.alive
    with pytest.raises(ValueError, match="expired"):
        live.data_section("app.obj", 3)


def test_nonzero_private_symbol_offset_places_whole_section_not_symbol_suffix(tmp_path):
    context = fixture(tmp_path / "project", second_target=0x2104)
    raw_object = bytearray(private_object())
    symbols, = struct.unpack_from("<I", raw_object, 8)
    struct.pack_into("<I", raw_object, symbols + 8 * 18 + 8, 4)
    replace_object(context, bytes(raw_object))
    raw_image = bytearray((context.directory / "app.exe").read_bytes())
    struct.pack_into("<I", raw_image, 512 + 6, 0x402104)
    replace_image(context, raw_image)
    replace_fixups(context, [(20, 0, 0x1001, 0x1020), (6, 0, 0x1006, 0x2104), (6, 0, 0x100B, 0x2104)])
    report = observe(context)
    assert report["address"] == 0x402100 and report["size"] == 16
    assert all(row["address"] == 0x402104 and row["object_offset"] == 4 for row in report["anchors"])


def test_mutating_returned_batch_anchor_cannot_poison_cached_owner_evidence(tmp_path, monkeypatch):
    from match_recovery import linked_data_anchors
    from match_recovery.linked_storage import _data_sections
    context = fixture(tmp_path / "project")
    original = linked_data_anchors.collect_anchors
    calls = []
    def counted(*args, **kwargs):
        calls.append(args[1]["owner"])
        return original(*args, **kwargs)
    monkeypatch.setattr(linked_data_anchors, "collect_anchors", counted)
    with linked_context.linked_build(context) as live:
        with _data_sections(live.inputs) as data:
            first = data("app.obj", 3)
            expected = deepcopy(first)
            first["anchors"][0]["address"] = 0
            first["anchors"][0]["input"]["path"] = "forged"
            first["anchors"].pop()
            first["caller_evidence"]["callers"].clear()
            assert data("APP.OBJ", 3) == expected
    assert calls == ["app.obj"]


@pytest.mark.parametrize("damage", ["object", "selected_identity", "selected_hash", "selected_raw"])
def test_collector_requires_exact_selected_referring_object_identity(tmp_path, damage):
    from match_recovery.linked_data_anchors import collect_anchors
    context = fixture(tmp_path / "project")
    with linked_context.linked_build(context) as live:
        selected = live.inputs.objects.resolve("app.obj")
        obj = Coff(selected["raw"])
        if damage == "object": obj = Coff(private_object(duplicate=True))
        elif damage == "selected_identity": selected["identity"]["path"] = str(context.root / "app.exp")
        elif damage == "selected_hash": selected["sha256"] = "0" * 64
        else: selected["raw"] += b"changed"
        with pytest.raises(ValueError):
            collect_anchors(live.inputs, selected, obj)


def mutate(context, live, damage):
    if damage in {"pdb", "image", "map", "receipt", "object", "archive", "tool"}:
        path = {"pdb": context.directory / "app.pdb", "image": context.directory / "app.exe",
                "map": context.directory / "app.map", "receipt": context.directory / "link.json",
                "object": context.root / "app.obj", "archive": context.root / "runtime.lib",
                "tool": context.root / "linker"}[damage]
        path.write_bytes(path.read_bytes() + b"changed")
    elif damage == "parsed_image": live.inputs.image.base += 4096
    elif damage == "parsed_pdb": live.inputs.artifacts["msvc-pdb"] += b"changed"
    elif damage == "selected_cache": live.inputs.objects._resolved["app.obj"]["raw"] += b"changed"
    elif damage == "expired": context.alive = False
    else: raise AssertionError(damage)


@pytest.mark.parametrize("damage", ["pdb", "image", "map", "receipt", "object", "archive", "tool",
                                    "parsed_image", "parsed_pdb", "selected_cache", "expired"])
@pytest.mark.parametrize("at_close", [False, True])
def test_private_anchor_placement_refuses_changed_live_inputs(tmp_path, damage, at_close):
    context = fixture(tmp_path / "project")
    with pytest.raises(ValueError):
        with linked_context.linked_build(context) as live:
            assert live.data_section("app.obj", 3)["placement"] == "pass"
            mutate(context, live, damage)
            if not at_close:
                with pytest.raises(ValueError):
                    live.data_section("app.obj", 3)


@pytest.mark.parametrize("damage", ["pdb", "object", "parsed_image", "selected_cache", "expired"])
def test_native_final_guard_refuses_mutation_inside_successful_collector(tmp_path, monkeypatch, damage):
    from match_recovery import linked_data_anchors
    context = fixture(tmp_path / "project")
    original = linked_data_anchors.collect_anchors
    with pytest.raises(ValueError):
        with linked_context.linked_build(context) as live:
            def changed(*args, **kwargs):
                result = original(*args, **kwargs)
                mutate(context, live, damage)
                return result
            monkeypatch.setattr(linked_data_anchors, "collect_anchors", changed)
            with pytest.raises(ValueError):
                live.data_section("app.obj", 3)


def test_native_final_guard_runs_even_when_the_collector_raises(tmp_path, monkeypatch):
    from match_recovery import linked_data_anchors
    context = fixture(tmp_path / "project")
    with pytest.raises(ValueError, match="expired"):
        with linked_context.linked_build(context) as live:
            def refused(*args, **kwargs):
                context.alive = False
                raise ValueError("unsupported caller")
            monkeypatch.setattr(linked_data_anchors, "collect_anchors", refused)
            with pytest.raises(ValueError, match="expired"):
                live.data_section("app.obj", 3)


def two_callers(root, *, second_address=0x402100, damage_second=False):
    """Promote the physical child to an independently mapped code COMDAT."""
    context = fixture(root)
    raw = bytearray(private_object())
    symbol_start, = struct.unpack_from("<I", raw, 8)
    child_data, = struct.unpack_from("<I", raw, 60 + 20)
    raw[child_data:child_data + 16] = b"\x68" + bytes(4) + b"\xc3" + b"\xcc" * 10
    # Place the new relocation immediately before the old symbol table.
    raw[symbol_start:symbol_start] = struct.pack("<IIH", 1, 8, 6)
    struct.pack_into("<I", raw, 60 + 24, symbol_start)
    struct.pack_into("<H", raw, 60 + 32, 1)
    symbol_start += 10
    struct.pack_into("<I", raw, 8, symbol_start)
    auxiliary = symbol_start + 4 * 18
    struct.pack_into("<H", raw, auxiliary + 4, 1)
    struct.pack_into("<HB", raw, auxiliary + 12, 0, 2)
    entry = symbol_start + 5 * 18
    raw[entry:entry + 8] = b"_g".ljust(8, b"\0")
    struct.pack_into("<HB", raw, entry + 14, 32, 2)
    replace_object(context, bytes(raw))
    mapping = (context.directory / "app.map").read_text()
    mapping = mapping.replace(" entry point", " 0001:00000020 _g 00401020 f app.obj\n entry point")
    replace_map(context, mapping)
    image = bytearray((context.directory / "app.exe").read_bytes())
    image[544:560] = b"\x68" + struct.pack("<I", second_address) + b"\xc3" + b"\xcc" * 10
    if damage_second:
        image[549] ^= 1
    relocations = (struct.pack("<II4H", 0x1000, 16, 0x3006, 0x300B, 0x3021, 0)
                   + struct.pack("<IIHH", 0x2000, 12, 0x3004, 0))
    image[896:896 + len(relocations)] = relocations
    struct.pack_into("<II", image, 0x98 + 96 + 5 * 8, 0x1180, len(relocations))
    replace_image(context, image)
    replace_fixups(context, [(20, 0, 0x1001, 0x1020), (6, 0, 0x1006, 0x2100),
                            (6, 0, 0x100B, 0x2100), (6, 0, 0x1021, second_address - 0x400000)])
    return context


@pytest.mark.parametrize("conflicting", [False, True])
def test_two_complete_verified_callers_must_agree_on_private_section_base(tmp_path, conflicting):
    from match_recovery.linked_code_region import verify_code_region
    context = two_callers(tmp_path / "project", second_address=0x402104 if conflicting else 0x402100)
    with linked_context.linked_build(context) as live:
        obj = Coff(live.inputs.objects.resolve("app.obj")["raw"])
        for name in ("_f", "_g"):
            proof = verify_code_region(obj, obj.function(name), "app.obj", live.inputs.mapping,
                live.inputs.image, objects=live.inputs.objects, fixups=live.inputs.fixups)
            assert proof["result"] == "pass"
        if conflicting:
            with pytest.raises(ValueError, match="anchors disagree"):
                live.data_section("app.obj", 3)
        else:
            report = live.data_section("app.obj", 3)
            assert report["address"] == 0x402100
            assert {row["referring_symbol"] for row in report["anchors"]} == {"_f", "_g"}
            assert all(row["result"] == "pass" for row in report["caller_evidence"]["callers"])


def test_one_verified_caller_can_place_storage_while_other_failed_caller_remains_a_gap(tmp_path):
    from match_recovery.linked_code_region import verify_code_region
    context = two_callers(tmp_path / "project", damage_second=True)
    with linked_context.linked_build(context) as live:
        obj = Coff(live.inputs.objects.resolve("app.obj")["raw"])
        proofs = {name: verify_code_region(obj, obj.function(name), "app.obj", live.inputs.mapping,
            live.inputs.image, objects=live.inputs.objects, fixups=live.inputs.fixups) for name in ("_f", "_g")}
        assert proofs["_f"]["result"] == "pass" and proofs["_g"]["result"] == "fail"
        report = live.data_section("app.obj", 3)
        assert (report["address"], report["size"]) == (0x402100, 16)
        assert {row["referring_symbol"] for row in report["anchors"]} == {"_f"}
        evidence = report["caller_evidence"]
        assert any(row["symbol"] == "_g" and row["result"] == "fail" for row in evidence["gaps"])
        assert evidence["caller_coverage"] == evidence["allocation_isolation"] == "unknown"


def add_icf_candidate(context, spelling):
    # A physical, receipt-bound diagnostic group with an intentionally
    # unresolved peer. It proves only candidate exclusion, never ICF acceptance.
    group = ("Selected symbol:\n" + spelling + " from app.obj\nReplaced symbol(s):\n"
             "_unproved from app.exp\nICF total savings: 16 bytes\n")
    log = (context.directory / "link.log").read_text().replace("Start Pass2\n", group + "Start Pass2\n")
    evidence = json.loads((context.directory / "link.json").read_bytes())
    evidence["link_log_sha256"] = sha256(log.encode()).hexdigest()
    update_artifact(context, "link.log", log.encode())
    update_artifact(context, "link.json", json.dumps(evidence).encode())


@pytest.mark.parametrize("spelling", ["_f", '".text"'])
def test_symbol_or_section_icf_candidate_cannot_supply_private_data_anchors(tmp_path, spelling):
    from match_recovery.linked_data_anchors import collect_anchors
    context = fixture(tmp_path / "project")
    add_icf_candidate(context, spelling)
    with linked_context.linked_build(context) as live:
        assert ("app.obj", "_f") in live.inputs.folding.candidate_keys()
        with pytest.raises(ValueError):
            live.inputs.folding.group("app.obj", "_f")
        selected = live.inputs.objects.resolve("app.obj")
        report = collect_anchors(live.inputs, selected, Coff(selected["raw"]))
        assert report["anchors"] == {}
        assert any(row["symbol"] == "_f" and "folded caller" in row["reason"] for row in report["gaps"])
        assert report["caller_coverage"] == "unknown"
        with pytest.raises(ValueError, match="no independent placement anchor"):
            live.data_section("app.obj", 3)


@pytest.mark.parametrize("spelling", ["_g", '".text$x"'])
def test_independent_caller_still_places_data_while_icf_peer_remains_excluded(tmp_path, spelling):
    context = two_callers(tmp_path / "project")
    add_icf_candidate(context, spelling)
    with linked_context.linked_build(context) as live:
        candidates = live.inputs.folding.candidate_keys()
        assert ("app.obj", "_g") in candidates and ("app.obj", "_f") not in candidates
        with pytest.raises(ValueError):
            live.inputs.folding.group("app.obj", "_g")
        report = live.data_section("app.obj", 3)
        assert (report["placement"], report["address"], report["size"]) == ("pass", 0x402100, 16)
        assert {row["referring_symbol"] for row in report["anchors"]} == {"_f"}
        evidence = report["caller_evidence"]
        assert any(row["symbol"] == "_g" and "folded caller" in row["reason"] for row in evidence["gaps"])
        assert evidence["caller_coverage"] == evidence["allocation_isolation"] == "unknown"
