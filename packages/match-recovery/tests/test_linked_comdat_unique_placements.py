"""Same-byte candidates need independent placement, not an absence inference."""

import json
import struct

import pytest

from match_recovery.linked_context import linked_build
from match_recovery.linked_comdat_selection import comdat_selection, _entry
from match_recovery.msf import SmallMsf
from test_linked_comdat_selection import fixture as ordinary_fixture
from test_linked_comdat_icf import object_keys, CODE
from test_linked_initializers import update_artifact
from test_linked_modules import set_link
from test_ordinary_contributions import set_contributions
from test_pdb_contributions import contribution
from test_msf import small_msf


def fixture(root, *, damage=None, fpo=False):
    """unit0 K@401000; dependency discarded K then unique P@401040."""
    a_rows = [(b"_K", 2, CODE)]
    if damage == "duplicate_key": a_rows.append((b"_P", 1, CODE))
    rows = [(b"_K", 2, b"\x31\xc0\xc3"), (b"_P", 2 if damage == "any_peer" else 1, CODE)]
    if damage == "hidden_peer": rows.append((b"_hidden", 2, CODE))
    b = object_keys(rows, fpo=fpo)
    if damage == "alias":
        data = bytearray(b)
        symbols, count = struct.unpack_from("<II", data, 8)
        extra = b"_alias\0\0" + struct.pack("<IhHBB", 0, 3 if fpo else 2, 0, 2, 0)
        b = bytes(data[:symbols + count * 18]) + extra + bytes(data[symbols + count * 18:])
        b = bytearray(b)
        struct.pack_into("<I", b, 12, count + 1)
        b = bytes(b)
    context = ordinary_fixture(root, winner=object_keys(a_rows, fpo=fpo), loser=b)
    evidence = json.loads((context.directory / "link.json").read_bytes())
    evidence["arguments"].extend(["/OPT:ICF", "/VERBOSE"])
    log = (context.directory / "link.log").read_text()
    events = {"ref_event": "Discarded _P from dependency.obj\n",
        "winner_ref": "Discarded _K from unit0.obj\n",
        "winner_section_ref": "Discarded .text from unit0.obj\n",
        "loser_ref": "Discarded _K from dependency.obj\n",
        "ownerless_ref": "Discarded _P\n", "section_ref": "Discarded .text from dependency.obj\n",
        "ownerless_section": "Discarded .text\n", "qualified_ref": "Discarded .text from wrong/dependency.obj\n",
        "malformed_ref": "Discarded bad line\n",
        "icf_event": "Selected symbol:\n_K from unit0.obj\nReplaced symbol(s):\n_P from dependency.obj\nICF total savings: 16 bytes\n",
        "section_icf": "Selected symbol:\n_K from unit0.obj\nReplaced symbol(s):\n.text from dependency.obj\nICF total savings: 16 bytes\n"}
    if damage in events: log = log.replace("Start Pass2", events[damage] + "Start Pass2")
    if damage == "qualified_final": log = log.replace("\ndependency.obj\n", "\nwrong/dependency.obj\n")
    set_link(context, evidence, log)
    offset = 0 if damage == "overlap" else 8 if damage == "partial_overlap" else 0x1f8 if damage == "cross_boundary" else 0x40
    owner = "unit0.obj" if damage == "wrong_owner" else "dependency.obj"
    row = f" 0001:{offset:08x} _P {0x401000 + offset:08x} f {owner}\n"
    mapping = ("separate\n Preferred load address is 00400000\n"
        " Address Publics by Value Rva+Base Lib:Object\n"
        " 0001:00000000 _K 00401000 f unit0.obj\n"
        + ("" if damage == "missing_map" else row)
        + (row if damage == "duplicate_map" else "")
        + " entry point at 0001:00000000\n Static symbols\n")
    update_artifact(context, "app.map", mapping.encode())
    image = bytearray((context.directory / "app.exe").read_bytes())
    image[0x200:0x210] = CODE
    image[0x200 + offset:0x210 + offset] = CODE
    if damage == "peer_bytes": image[0x200 + offset] ^= 1
    if damage in ("nonexecuting", "noncode"):
        flags = struct.unpack_from("<I", image, 0x178 + 36)[0]
        struct.pack_into("<I", image, 0x178 + 36, flags & ~(0x20000000 if damage == "nonexecuting" else 0x20))
    update_artifact(context, "app.exe", bytes(image))
    first = contribution(section=1, offset=0, size=16, flags=0x60501020, module=0)
    peer = contribution(section=1, offset=offset, size=16, flags=0x60501020,
                        module=0 if damage == "wrong_module" else 1)
    set_contributions(context, first + (b"" if damage == "missing_sc" else peer)
                      + (peer if damage == "duplicate_sc" else b""))
    if damage in ("nonexecuting", "noncode"):
        msf = SmallMsf((context.directory / "app.pdb").read_bytes())
        streams = [msf.stream(index) for index in range(msf.stream_count)]
        streams[5] = bytes(image[0x178:0x1c8])
        update_artifact(context, "app.pdb", small_msf(streams))
    if fpo:
        msf = SmallMsf((context.directory / "app.pdb").read_bytes())
        streams = [msf.stream(index) for index in range(msf.stream_count)]
        dbi = bytearray(streams[3])
        counts = struct.unpack_from("<8I", dbi, 24)
        struct.pack_into("<H", dbi, 64 + sum(counts[:5]) + counts[7], len(streams))
        streams[3] = bytes(dbi)
        records = [struct.pack("<IIIHH", address, 16, 3, 2, 0x0201) for address in (0x1000, 0x1000 + offset)]
        streams.append(b"".join(records[:1] if damage == "missing_fpo" else records))
        update_artifact(context, "app.pdb", small_msf(streams))
    return context


@pytest.mark.parametrize("fpo", [False, True])
def test_unique_separately_placed_nodup_root_does_not_block_any_selection(tmp_path, fpo):
    context = fixture(tmp_path / "project", fpo=fpo)
    with linked_build(context) as native, comdat_selection(native) as selected:
        result = selected.loser("dependency.obj", 1)
        cert, = selected.certificates()
        peer, = selected.unique_placements()
        assert result["disposition"] == "discarded_any_duplicate"
        assert peer["owner"] == "dependency.obj" and peer["root"]["name"] == "_P"
        assert peer["address"] == 0x401040 and peer["size"] == 16
        assert peer["root"]["section"] == (3 if fpo else 2)
        assert peer["placement_id"] in cert["no_other_fold"]["unique_placements"]
        assert peer["proof"]["result"] == "pass"
        assert peer["outside_scope_verification"] == peer["allocation_isolation"] == "unknown"
        assert peer["source_authority"] == "not_transferred"
        assert selected.loser("dependency.obj", 3 if fpo else 2) is None


@pytest.mark.parametrize("damage", ["overlap", "partial_overlap", "wrong_module", "ref_event", "hidden_peer",
    "duplicate_key", "any_peer", "alias", "ownerless_ref", "section_ref", "ownerless_section",
    "qualified_ref", "malformed_ref", "icf_event", "section_icf", "qualified_final", "wrong_owner",
    "missing_map", "duplicate_map", "peer_bytes", "missing_sc", "duplicate_sc",
    "winner_ref", "winner_section_ref", "nonexecuting", "noncode", "cross_boundary"])
def test_unsupported_or_contradictory_peer_never_disappears(tmp_path, damage):
    context = fixture(tmp_path / "project", damage=damage)
    with pytest.raises(ValueError):
        with linked_build(context) as native, comdat_selection(native) as selected:
            selected.loser("dependency.obj", 1)


def test_peer_frame_is_verified_before_placement_can_be_used(tmp_path):
    context = fixture(tmp_path / "project", fpo=True, damage="missing_fpo")
    with pytest.raises(ValueError, match="metadata|FPO"):
        with linked_build(context) as native, comdat_selection(native) as selected:
            selected.loser("dependency.obj", 1)


def test_publication_waits_for_any_winner_proof_not_only_peer(tmp_path):
    context = fixture(tmp_path / "project")
    image = bytearray((context.directory / "app.exe").read_bytes())
    image[0x200] ^= 1
    update_artifact(context, "app.exe", bytes(image))
    with linked_build(context) as native, comdat_selection(native) as selected:
        with pytest.raises(ValueError, match="ANY winner code"):
            selected.loser("dependency.obj", 1)
        assert len(selected.unique_placements()) == 1
        assert selected.certificates() == []


def test_placement_reports_are_defensive_and_handle_expires(tmp_path):
    context = fixture(tmp_path / "project")
    with linked_build(context) as native, comdat_selection(native) as selected:
        selected.loser("dependency.obj", 1)
        selected.unique_placements()[0]["address"] = 0
        assert selected.unique_placements()[0]["address"] == 0x401040
    with pytest.raises(ValueError, match="expired"):
        selected.unique_placements()


@pytest.mark.parametrize("target", ["cache", "object", "physical"])
def test_completed_placement_batch_rechecks_all_derived_and_physical_inputs(tmp_path, target):
    context = fixture(tmp_path / "project")
    with pytest.raises(ValueError):
        with linked_build(context) as native, comdat_selection(native) as selected:
            selected.loser("dependency.obj", 1)
            if target == "cache": _entry(selected)["unique_placements"].clear()
            elif target == "object":
                state = _entry(selected)
                key = next(key for key, obj in state["objects"].items() if any(s.name == "_P" for s in obj.symbols.values()))
                state["objects"][key].symbols.clear()
            else: context.objects[1].write_bytes(object_keys([(b"_P", 1, CODE)]))


def test_cached_root_proof_does_not_cache_disjointness_for_another_query(monkeypatch, tmp_path):
    from match_recovery import linked_comdat_selection as module
    original = module.verify_code_region
    calls = []
    def counted(*args, **kwargs):
        calls.append(args[1].name)
        return original(*args, **kwargs)
    monkeypatch.setattr(module, "verify_code_region", counted)
    context = fixture(tmp_path / "project")
    with linked_build(context) as native, comdat_selection(native) as selected:
        selected.loser("dependency.obj", 1)
        state = _entry(selected)
        certificate, = selected.certificates()
        _, obj, number = module._query(state, "unit0.obj", 1)
        # This exercises the internal per-query interval algebra only; the
        # hypothetical address never becomes a MAP row or publication scope.
        with pytest.raises(ValueError, match="not separately placed"):
            module._no_other_fold(state, obj, obj.function("_K"), certificate["candidates"], 0x401040)
        assert len(selected.unique_placements()) == 1
    assert calls == ["_P", "_K"]


def test_exact_peer_refusal_is_memoized_and_guarded(monkeypatch, tmp_path):
    from match_recovery import linked_comdat_selection as module
    original = module._unique_placement_uncached
    calls = []
    def counted(*args, **kwargs):
        calls.append(args[1:4])
        return original(*args, **kwargs)
    monkeypatch.setattr(module, "_unique_placement_uncached", counted)
    context = fixture(tmp_path / "project", damage="any_peer")
    with pytest.raises(ValueError, match="indices changed"):
        with linked_build(context) as native, comdat_selection(native) as selected:
            state = _entry(selected)
            _, obj, _ = module._query(state, "dependency.obj", 2)
            from match_recovery.model import digest
            from hashlib import sha256
            identity = digest({"path": str(context.objects[1])})
            stamp = sha256(obj.raw_data).hexdigest()
            for _ in range(2):
                with pytest.raises(ValueError, match="not a unique independent NODUP"):
                    module._unique_placement(state, identity, stamp, 2)
            assert len(calls) == 1
            state["unique_refusals"].clear()


def test_positive_ref_for_distinct_any_loser_is_not_a_winner_contradiction(tmp_path):
    context = fixture(tmp_path / "project", damage="loser_ref")
    with linked_build(context) as native, comdat_selection(native) as selected:
        assert selected.loser("dependency.obj", 1) is not None
        assert len(selected.unique_placements()) == 1


@pytest.mark.parametrize("owner", ["unit0.obj", "dependency.obj"])
def test_prefix_only_any_route_also_reconciles_positive_winner_ref(tmp_path, owner):
    context = ordinary_fixture(tmp_path / "project")
    evidence = json.loads((context.directory / "link.json").read_bytes())
    log = (context.directory / "link.log").read_text().replace("Start Pass2",
        f"Discarded _f from {owner}\nStart Pass2")
    set_link(context, evidence, log)
    with linked_build(context) as native, comdat_selection(native) as selected:
        if owner == "unit0.obj":
            with pytest.raises(ValueError, match="winner has contradictory"):
                selected.loser("dependency.obj", 1)
            assert selected.certificates() == []
        else:
            assert selected.loser("dependency.obj", 1) is not None
            assert selected.unique_placements() == []


def test_nonexecuting_peer_is_refused_by_placement_after_real_code_proof(monkeypatch, tmp_path):
    from match_recovery import linked_comdat_selection as module
    original = module.verify_code_region
    proofs = []
    def observed(*args, **kwargs):
        proof = original(*args, **kwargs)
        proofs.append(proof)
        return proof
    monkeypatch.setattr(module, "verify_code_region", observed)
    context = fixture(tmp_path / "project", damage="nonexecuting")
    with pytest.raises(ValueError, match="not wholly within executable PE code storage"):
        with linked_build(context) as native, comdat_selection(native) as selected:
            selected.loser("dependency.obj", 1)
    assert len(proofs) == 1 and proofs[0]["result"] == "pass" and proofs[0]["symbol"] == "_P"
