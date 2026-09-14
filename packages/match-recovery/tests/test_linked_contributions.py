"""Fresh common/PDB extent correspondence; no isolation or projection waiver."""

from copy import deepcopy
import struct

import pytest

from match_recovery import linked_contributions, pdb_contributions
from match_recovery.linked_context import linked_build
from test_common_preservation import build
from test_link_file_evidence import stamp
from test_linked_initializers import update_artifact
from test_linked_common import mutate
from test_msf import small_msf
from test_pdb_contributions import parts, module, contribution
from test_pdb_fixups import DIRECTORY, DEBUG, SIGNATURE


def context(root, *, records=None, name=b"* Linker *", object_name=b"", damage=None):
    value = build(root, damage=damage)
    raw = bytearray((value.directory / "app.exe").read_bytes())
    payload = struct.pack("<4sIII", b"NB10", 0, SIGNATURE, 1) + b"app.pdb\0"
    overlay = len(raw)
    raw.extend(payload)
    struct.pack_into("<I", raw, 0x98 + 60, 0x200)
    struct.pack_into("<I", raw, 0x98 + 92, 16)
    struct.pack_into("<II", raw, DIRECTORY, 0x1100, 28)
    struct.pack_into("<IIHHIIII", raw, DEBUG, 0, 0, 0, 0, 2, len(payload), 0, overlay)
    row = contribution(section=2, offset=0, size=4, flags=0xC0102080)
    _, streams = parts(module(name, object_name), row if records is None else records)
    streams[4] = b""  # Explicitly present empty fixup stream.
    streams[5] = bytes(raw[0x178:0x1C8])
    pdb = small_msf(streams)
    update_artifact(value, "app.exe", bytes(raw))
    (value.directory / "app.pdb").write_bytes(pdb)
    value.report["outputs"].append({"path": "app.pdb", "format": "msvc-pdb", "result": "pass", **stamp(pdb)})
    value.pin_outputs()
    return value


def test_native_common_extent_has_independent_full_table_correspondence(tmp_path):
    with linked_build(context(tmp_path / "project")) as live:
        census = live.contributions()
        assert census["enumeration"] == "pass" and census["result"] == "unknown"
        assert census["modules"][0]["first"] is None
        report = live.common_contributions(("_g",))
        row, = report["contributions"]
        assert row["correspondence"] == "pass"
        actual, = row["intersections"]
        assert actual["address"] == 0x402000 and actual["size"] == 4 and actual["index"] == 0
        assert actual["flags"] == 0xC0102080
        assert report["contribution_count"] == report["module_count"] == 1
        for field in ("result", "allocation_isolation", "retained_storage_coverage", "physical_object_binding"):
            assert report[field] == "unknown"
        assert not report["ready_for_collection"] and not report["persisted"]
        automatic = live.common_intersections(("_g",))
        assert automatic["pdb_contributions"] == report
        assert automatic["allocation_isolation"] == "unknown"


def test_invalid_contribution_table_remains_a_default_check_gap(tmp_path):
    value = context(tmp_path / "project")
    # Coherent physical PDB, invalid SC-version metadata. The existing PDB
    # fixup binding legitimately does not interpret this optional substream.
    from match_recovery.msf import SmallMsf
    original = SmallMsf((value.directory / "app.pdb").read_bytes())
    streams = [original.stream(index) for index in range(original.stream_count)]
    header = bytearray(streams[3])
    start = 64 + struct.unpack_from("<I", header, 24)[0]
    struct.pack_into("<I", header, start, 0)
    streams[3] = bytes(header)
    update_artifact(value, "app.pdb", small_msf(streams))
    with linked_build(value) as live:
        report = live.common_intersections(("_g",))
        assert report["pdb_contributions"]["result"] == "unknown"
        assert any("PDB common contribution evidence refused" in row["reason"] for row in report["gaps"])
        assert report["allocation_isolation"] == "unknown"


@pytest.mark.parametrize("records", [b"", contribution(section=2, offset=4, size=4),
    contribution(section=2, offset=0, size=8, flags=0xC0102080),
    contribution(section=2, offset=0, size=4, flags=0xE0102080),
    contribution(section=2, offset=0, size=4, flags=0xC0102080) * 2,
    contribution(section=2, offset=0, size=4, flags=0xC0102080) + contribution(section=2, offset=2, size=2)])
def test_missing_broader_executable_and_overlapping_contributions_never_prove_common_identity(tmp_path, records):
    with linked_build(context(tmp_path / "project", records=records)) as live:
        report = live.common_contributions(("_g",))
        assert report["contributions"][0]["correspondence"] == "unknown"
        assert report["result"] == report["allocation_isolation"] == "unknown"


@pytest.mark.parametrize("name,object_name", [(b"ordinary.obj", b"ordinary.obj"),
                                           (b"* Linker *", b"actual.obj")])
def test_coordinates_cannot_turn_an_ordinary_module_into_the_linker_label(tmp_path, name, object_name):
    with linked_build(context(tmp_path / "project", name=name, object_name=object_name)) as live:
        assert live.common_contributions(("_g",))["contributions"][0]["correspondence"] == "unknown"


@pytest.mark.parametrize("damage", ["conflicting_size", "missing", "overlap", "policy", "nonzero"])
def test_pdb_extent_cannot_repair_failed_common_provenance(tmp_path, damage):
    with linked_build(context(tmp_path / "project", damage=damage)) as live:
        report = live.common_contributions(("_g",))
        assert report["common_allocations"]["result"] != "pass"
        assert report["contributions"][0]["correspondence"] == "unknown"


@pytest.mark.parametrize("method", ["contributions", "common_contributions"])
def test_native_reports_are_defensive_and_context_bound(tmp_path, method):
    with linked_build(context(tmp_path / "project")) as live:
        call = lambda: getattr(live, method)(*(("_g",),) if method == "common_contributions" else ())
        value = call()
        saved = deepcopy(value)
        value["contributions"].clear()
        value["result"] = "forged"
        assert call() == saved
    with pytest.raises(ValueError, match="expired"): call()


@pytest.mark.parametrize("damage", ["object", "archive", "image", "map_file", "receipt", "log", "tool",
                                    "expired", "parsed_image", "parsed_map", "evidence", "coverage", "records", "artifacts", "pdb"])
@pytest.mark.parametrize("method", ["contributions", "common_contributions"])
def test_native_input_mutation_refuses_both_observers(tmp_path, damage, method):
    value = context(tmp_path / "project")
    with pytest.raises(ValueError):
        with linked_build(value) as live:
            call = lambda: getattr(live, method)(*(("_g",),) if method == "common_contributions" else ())
            call()
            if damage == "pdb":
                path = value.directory / "app.pdb"
                path.write_bytes(path.read_bytes() + b"changed")
            else:
                mutate(value, live, damage)
            with pytest.raises(ValueError): call()


@pytest.mark.parametrize("method", ["contributions", "common_contributions"])
def test_final_guard_runs_even_after_an_observer_returns(tmp_path, monkeypatch, method):
    value = context(tmp_path / "project")
    target = pdb_contributions if method == "contributions" else linked_contributions
    name = "bound_contributions" if method == "contributions" else "common_contributions"
    original = getattr(target, name)
    with pytest.raises(ValueError, match="expired"):
        with linked_build(value) as live:
            def altered(*args, **kwargs):
                result = original(*args, **kwargs)
                value.alive = False
                return result
            monkeypatch.setattr(target, name, altered)
            with pytest.raises(ValueError, match="expired"):
                getattr(live, method)(*(("_g",),) if method == "common_contributions" else ())
