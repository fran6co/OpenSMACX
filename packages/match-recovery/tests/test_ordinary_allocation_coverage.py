"""Closed physical ordinary allocation domains, never saved passing verdicts."""

from copy import deepcopy
import json
import struct

import pytest

from match_recovery.linked_context import linked_build
from match_recovery.model import digest
from test_common_request_coverage import append_section
from test_link_file_evidence import stamp
from test_linked_data_contents import object_bytes, SYMBOLS
from test_linked_initializers import update_artifact
from test_linked_modules import fixture as module_context, set_link, set_modules
from test_ordinary_contributions import set_contributions
from test_pdb_contributions import contribution, module


FIRST = b"abcdefghABCDEFGH"
SECOND = b"ABCDEFGH"


def fixture(root, *, overlap=False, missing_anchor=False, missing_sc=False, corrupt=False,
            extra_unplaced=False, empty_sc=False, pdb_data_flags=0xC0300040,
            commons=(), common_damage=False, adjust=None):
    """One receipt-bound input, code and two fully named ordinary sections.

    The selected ordinary entry has real code bytes and its own bound PDB SC.
    The allocation proof is not runtime or whole-application acceptance, nor
    attribution of every PE header/padding byte.
    """
    context = module_context(root)
    output = context.objects[0]
    raw = bytearray(append_section(object_bytes(FIRST, symbols=SYMBOLS[:3]),
                                   data=b"\xc3" + b"\xcc" * 15))
    raw[100:108] = b".text\0\0\0"
    struct.pack_into("<I", raw, 100 + 36, 0x60500020)
    at, count = struct.unpack_from("<II", raw, 8)
    end = at + 18 * count
    raw[end:end] = struct.pack("<8sIhHBB", b"_entry\0\0", 0, 3, 0x20, 2, 0)
    struct.pack_into("<I", raw, 12, count + 1)
    for name, size, offset, flags in commons:
        at, count = struct.unpack_from("<II", raw, 8)
        end = at + 18 * count
        raw[end:end] = struct.pack("<8sIhHBB", name.encode(), size, 0, 0, 2, 0)
        struct.pack_into("<I", raw, 12, count + 1)
    if extra_unplaced:
        raw = bytearray(append_section(raw, data=b"unplaced"))
        raw[140:148] = b".extra\0\0"
        at, count = struct.unpack_from("<II", raw, 8)
        end = at + 18 * count
        raw[end:end] = struct.pack("<8sIhHBB", b"_extra\0\0", 0, 4, 0, 3, 0)
        struct.pack_into("<I", raw, 12, count + 1)
    output.write_bytes(raw)
    context.objects = [output]
    compiled, = context.report["compile_link"]["compilations"]
    compiled["compilation"]["output"]["after"] = stamp(raw)
    evidence = json.loads((context.directory / "link.json").read_bytes())
    evidence["arguments"] = evidence["explicit_inputs"] = [str(output)]
    evidence["files"] = [{"path": str(path), "before": stamp(path.read_bytes()),
                          "after": stamp(path.read_bytes())} for path in (output, root / "native/linker")]
    evidence["library_searches"] = []
    evidence["generated_exports"] = []
    log = f"Start Pass1\nEnd Pass1\nStart Pass2\n{output.name}\nEnd Pass2\n"
    set_link(context, evidence, log)
    first, second = 0x20, 0x28 if overlap else 0x40
    mapping = ("allocation-domain\n Preferred load address is 00400000\n"
               " Address Publics by Value Rva+Base Lib:Object\n"
               f" 0001:000001e0 _entry 004011e0 f {output.name}\n"
               " entry point at 0001:000001e0\n Static symbols\n"
               f" 0002:{first + 4:08x} _anchor {0x402000 + first + 4:08x} {output.name}\n"
               f" 0002:{first + 8:08x} _inner {0x402000 + first + 8:08x} {output.name}\n"
               + ("" if missing_anchor else
                  f" 0002:{second + 4:08x} _target {0x402000 + second + 4:08x} {output.name}\n"))
    mapping = mapping.replace(" entry point", "".join(
        f" 0002:{offset:08x} {name} {0x402000 + offset:08x} <common>\n"
        for name, _, offset, _ in commons) + " entry point")
    update_artifact(context, "app.map", mapping.encode())
    image = bytearray((context.directory / "app.exe").read_bytes())
    image[0x200:0x210] = b"\xcc" * 16
    image[0x380:0x38C] = b"\xcc" * 12
    image[0x3E0:0x3F0] = b"\xc3" + b"\xcc" * 15
    image[0x400:0x600] = bytes(512)
    image[0x400 + first:0x400 + first + len(FIRST)] = FIRST
    image[0x400 + second:0x400 + second + len(SECOND)] = SECOND
    struct.pack_into("<II", image, 0x98 + 96 + 5 * 8, 0, 0)
    if corrupt:
        image[0x400 + first] ^= 1
    if common_damage:
        image[0x400 + commons[0][2]] = 1
    update_artifact(context, "app.exe", bytes(image))
    label = str(output).encode()
    set_modules(context, module(label, label) + (module(b"* Linker *", b"") if commons else b""))
    records = contribution(section=2, offset=first, size=16, flags=pdb_data_flags)
    if not missing_sc:
        records += contribution(section=2, offset=second, size=8, flags=pdb_data_flags)
    records += contribution(section=1, offset=0x1E0, size=16, flags=0x60500020)
    if empty_sc:
        records += contribution(section=2, offset=first + 4, size=0, flags=0xC0300040)
    for _, size, offset, flags in commons:
        if flags is not None:
            records += contribution(section=2, offset=offset, size=size, flags=flags, module=1)
    set_contributions(context, records)
    if adjust is not None:
        adjust(context)
    # All mutation above constructs a different physical fixture BEFORE its
    # live epoch. Changes after this point cannot refresh its captured proof.
    expected = digest(context.report)
    pins = {path: stamp(path.read_bytes()) for path in (output, root / "compiler", root / "src/unit.cpp",
            root / "recovery/project.json", root / "recovery/build.json", root / "native/linker")}
    original = context.check_current
    def fresh():
        original()
        if digest(context.report) != expected or any(not path.is_file() or stamp(path.read_bytes()) != wanted
                                                    for path, wanted in pins.items()):
            raise ValueError("ordinary allocation native input epoch changed")
    context.check_current = fresh
    return context


def sections(report):
    return {row["object_section"]: row for row in report["sections"]}


def test_closed_physical_ordinary_domain_proves_retention_and_static_isolation(tmp_path):
    with linked_build(fixture(tmp_path / "project")) as live:
        report = live.ordinary_allocations()
        assert report["result"] == "pass", report
        rows = sections(report)
        assert set(rows) == {1, 2, 3}
        assert all(row["retention"] == row["allocation_isolation"] == row["input_consistency"] == "pass"
                   for row in rows.values()), report
        assert report["remaining_linked_effects"] == "unknown"
        assert not report["ready_for_collection"]


def test_overlapping_distinct_sections_do_not_gain_isolation_from_matching_bytes(tmp_path):
    with linked_build(fixture(tmp_path / "project", overlap=True)) as live:
        report = live.ordinary_allocations()
        assert report["result"] != "pass", report
        assert all(row["allocation_isolation"] != "pass" for row in sections(report).values()), report
        # The shared bytes really satisfy both section payloads. This is an
        # allocation/placement counterexample, not manufactured byte damage.
        assert all(row["input_consistency"] == "pass" for row in sections(report).values()), report


@pytest.mark.parametrize("damage", ["missing_anchor", "missing_sc", "extra_unplaced"])
def test_unobserved_contributor_cannot_disappear_from_closed_domain(tmp_path, damage):
    with linked_build(fixture(tmp_path / "project", **{damage: True})) as live:
        report = live.ordinary_allocations()
        assert report["result"] == "unknown", report
        rows = sections(report)
        assert set(rows) == ({1, 2, 3, 4} if damage == "extra_unplaced" else {1, 2, 3})
        assert all(row["allocation_isolation"] == "unknown" for row in rows.values()), report
        if damage != "extra_unplaced":
            assert rows[2]["retention"] != "pass"


def test_actual_data_damage_remains_physical_failure(tmp_path):
    with linked_build(fixture(tmp_path / "project", corrupt=True)) as live:
        report = live.ordinary_allocations()
        assert report["result"] == "fail", report
        assert sections(report)[1]["input_consistency"] == "fail"


def test_retained_payload_does_not_certify_unexplained_producer_characteristics(tmp_path):
    with linked_build(fixture(tmp_path / "project", pdb_data_flags=0xC0302040)) as live:
        report = live.ordinary_allocations()
        for index in (1, 2):
            row = sections(report)[index]
            assert row["retention"] == row["input_consistency"] == "pass"
            assert row["flags"] == 0xC0300040 and row["pdb_flags"] == 0xC0302040
            assert row["characteristics_agreement"] == row["allocation_isolation"] == "unknown"
        assert report["result"] == report["allocation_domain"] == report["allocation_isolation"] == "unknown"
        assert any("characteristics remain unexplained" in row["reason"] for row in report["gaps"])


@pytest.mark.parametrize("flags", [0xC0402040, 0xC0301040, 0x40302040, 0xC0302080])
def test_contradictory_or_unsupported_pdb_characteristics_cannot_prove_retention(tmp_path, flags):
    with linked_build(fixture(tmp_path / "project", pdb_data_flags=flags)) as live:
        report = live.ordinary_allocations()
        assert all(sections(report)[index]["retention"] == "unknown" for index in (1, 2))
        assert report["result"] == report["allocation_domain"] == "unknown"


@pytest.mark.parametrize("damage", ["corrupt", "overlap", "missing_sc"])
def test_unexplained_characteristics_do_not_hide_damage_or_ambiguous_placement(tmp_path, damage):
    with linked_build(fixture(tmp_path / "project", pdb_data_flags=0xC0302040,
                              **{damage: True})) as live:
        report = live.ordinary_allocations()
        assert report["result"] == ("unknown" if damage == "missing_sc" else "fail")
        if damage == "corrupt":
            assert sections(report)[1]["input_consistency"] == "fail"
        else:
            assert sections(report)[2]["retention"] == "unknown"


def test_empty_contribution_inside_real_allocation_is_retained_but_not_an_overlap(tmp_path):
    with linked_build(fixture(tmp_path / "project", empty_sc=True)) as live:
        report = live.ordinary_allocations()
        assert report["result"] == "unknown" and report["allocation_domain"] == "unknown"
        assert report["empty_contributions"] == report["unmatched_contributions"] == [3]
        assert report["contributions"][3]["size"] == 0
        assert all(row["retention"] == row["input_consistency"] == "pass" for row in sections(report).values())
        assert all(row["overlaps"] == [] for row in sections(report).values())
        assert report["allocation_isolation"] == "unknown"


@pytest.mark.parametrize("artifact", ["app.exe", "app.map", "app.pdb", "unit0.obj", "link.log", "link.json"])
def test_changed_physical_artifact_refuses_during_use_and_final_close(tmp_path, artifact):
    context = fixture(tmp_path / "project")
    with pytest.raises(ValueError, match="changed"):
        with linked_build(context) as live:
            assert live.ordinary_allocations()["result"] == "pass"
            path = context.directory / artifact
            path.write_bytes(path.read_bytes() + b"changed after native observation")
            with pytest.raises(ValueError, match="changed"):
                live.ordinary_allocations()


def test_native_observation_is_defensive_and_handle_expires(tmp_path):
    with linked_build(fixture(tmp_path / "project")) as live:
        observed = live.ordinary_allocations()
        saved = deepcopy(observed)
        observed["sections"].clear()
        assert live.ordinary_allocations() == saved
    with pytest.raises(ValueError, match="expired"):
        live.ordinary_allocations()


def test_authentic_icf_code_coalescing_is_not_an_ordinary_allocation_counterexample(monkeypatch, tmp_path):
    import test_generated_residual_folded_roots as folded
    original = folded.fold_dependency

    def bind_contributions(context, **options):
        original(context, **options)
        set_modules(context, b"".join(module(str(path).encode(), str(path).encode())
                                      for path in context.objects))
        set_contributions(context, b"".join((
            contribution(section=1, offset=0, size=16, flags=0x60501020, module=0),
            contribution(section=2, offset=4, size=4, flags=0xC0300040, module=0),
            contribution(section=1, offset=0x20, size=16, flags=0x60501020, module=1),
            contribution(section=2, offset=8, size=4, flags=0xC0300040, module=1),
            contribution(section=1, offset=0x20, size=16, flags=0x60501020, module=2))))

    # Modify physical construction only, before the existing fixture seals
    # source/command/artifact epochs. No native verifier verdict is replaced.
    monkeypatch.setattr(folded, "fold_dependency", bind_contributions)
    context = folded.context(monkeypatch, tmp_path / "project")
    with linked_build(context) as live:
        initializers = live.initializers()
        assert initializers["result"] == initializers["code_dependencies"]["result"] == "pass"
        report = live.ordinary_allocations()
        assert report["code"]["result"] == "pass", report
        sharing = [row for row in report["sections"] if row["classification"] == "code"
                   and row.get("envelope", {}).get("address") == 0x401020]
        assert len(sharing) == 2
        assert len({row["input"]["path"] for row in sharing}) == 2
        assert all(row["input_consistency"] == "pass" for row in sharing)
        assert all(row["allocation_isolation"] == "unknown" for row in sharing)
        assert report["result"] == report["allocation_isolation"] == "unknown", report
        assert report["remaining_linked_effects"] == "unknown"
