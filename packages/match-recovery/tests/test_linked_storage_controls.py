"""Independent physical anchor/extent and live-epoch storage damage controls."""

from copy import deepcopy
import json
import struct

import pytest

from match_recovery import linked_context, linked_storage
from match_recovery.pe_coff import Coff
from test_link_file_evidence import stamp
from test_linked_initializers import Context
from test_linked_storage import OWNER, data_object, fixture, observe


def two_sections(storage=3):
    """One physical owner with an identically named anchor in two sections."""
    headers = b"".join(name.ljust(8, b"\0") + struct.pack(
        "<IIIIIIHHI", 0, 0, 8, 100 + index * 8, 0, 0, 0, 0, 0xC0300040)
        for index, name in enumerate((b".data", b".data2")))
    symbols = b"".join(b"_private" + struct.pack("<IhHBB", 4, number, 0, storage, 0)
                       for number in (1, 2))
    return struct.pack("<HHIIIHH", 0x14C, 2, 0, 116, 2, 0, 0) + headers + bytes(16) + symbols + struct.pack("<I", 4)


@pytest.mark.parametrize("storage", [2, 3])
@pytest.mark.parametrize("number", [1, 2])
def test_map_owner_and_name_cannot_choose_between_two_physical_sections(tmp_path, storage, number):
    raw = two_sections(storage)
    assert len(Coff(raw).sections) == 2
    value = fixture(tmp_path, raw=raw, placements=(("_private", 0x1014, storage),))
    with pytest.raises(ValueError, match="across selected object sections"):
        observe(value, number)


@pytest.mark.parametrize("storage", [2, 3])
@pytest.mark.parametrize("displacement", [0, 4])
@pytest.mark.parametrize("primary", ["map", "debug"])
def test_opposite_map_namespace_cannot_be_ignored(tmp_path, storage, displacement, primary):
    opposite = 3 if storage == 2 else 2
    placements = [("_private", 0x1014 + displacement, opposite)]
    if primary == "map":
        placements.append(("_private", 0x1014, storage))
    debug = [("_private", 0x1014, 1, 0, storage, b"")] if primary == "debug" else []
    value = fixture(tmp_path, raw=data_object((("_private", 4, storage),)),
                    placements=tuple(placements), debug=debug)
    with pytest.raises(ValueError, match="opposite map namespace"):
        observe(value)


def relocated(edges):
    raw = bytearray(data_object())
    struct.pack_into("<I", raw, 44, len(raw))
    struct.pack_into("<H", raw, 52, len(edges))
    for offset, kind in edges:
        raw.extend(struct.pack("<IIH", offset, 0, kind))
    return bytes(raw)


@pytest.mark.parametrize("edges", [((5, 6),), ((8, 20),), ((0xFFFFFFFF, 6),), ((0, 7),),
    ((0, 6), (0, 6)), ((0, 6), (3, 20)), ((4, 20), (1, 6))])
def test_relocation_writes_cannot_escape_or_overlap_the_declared_envelope(tmp_path, edges):
    raw = relocated(edges)
    assert len(Coff(raw).sections[0].relocations) == len(edges)
    with pytest.raises(ValueError, match="relocation"):
        observe(fixture(tmp_path, raw=raw))


@pytest.mark.parametrize("edges", [((0, 6),), ((4, 20),), ((0, 6), (4, 20)),
                                     ((0xFFFFFFFF, 0),), ((0, 6), (0, 0))])
def test_bounded_writes_and_explicit_noops_do_not_claim_verified_contents(tmp_path, edges):
    report = observe(fixture(tmp_path, raw=relocated(edges)))
    assert report["placement"] == "pass" and report["size"] == 8
    assert report["contents"] == report["allocation_isolation"] == "unknown"


def map_bytes(mapping):
    def row(item):
        flag = "f " if item.function else ""
        return f" {item.segment:04x}:{item.offset:08x} {item.symbol} {item.address:08x} {flag}{item.owner}\n"
    return (f"storage\n Preferred load address is {mapping.base:08x}\n"
        " Address         Publics by Value              Rva+Base     Lib:Object\n"
        + "".join(row(item) for item in mapping.publics)
        + f" entry point at {mapping.entry[0]:04x}:{mapping.entry[1]:08x}\n Static symbols\n"
        + "".join(row(item) for item in mapping.statics or ())).encode()


def native_context(root):
    root.mkdir()
    inputs = fixture(root)
    directory = root / "product"
    directory.mkdir()
    log = (f"Start Pass1\n Searching {root / 'runtime.lib'}:\nEnd Pass1\nStart Pass2\napp.obj\n"
           "RUNTIME.lib(private.obj)\nEnd Pass2\n").encode()
    assert stamp(log)["sha256"] == inputs.evidence["link_log_sha256"]
    artifacts = [("app.exe", "pe32-i386", inputs.image.data),
                 ("app.map", "msvc-map", map_bytes(inputs.mapping)),
                 ("link.log", "msvc-link-log", log),
                 ("link.json", "link-input-files", json.dumps(inputs.evidence).encode())]
    rows = []
    for name, kind, raw in artifacts:
        (directory / name).write_bytes(raw)
        rows.append({"path": name, "format": kind, "result": "pass", **stamp(raw)})
    context = Context(root, directory, {"result": "pass", "compile_link": {"result": "pass"}, "outputs": rows})
    return context, inputs


def mutate(context, live, damage):
    if damage in {"archive", "object", "tool", "image", "map", "log", "receipt"}:
        path = {"archive": context.root / "runtime.lib", "object": context.root / "app.obj",
                "tool": context.root / "linker", "image": context.directory / "app.exe",
                "map": context.directory / "app.map", "log": context.directory / "link.log",
                "receipt": context.directory / "link.json"}[damage]
        path.write_bytes(path.read_bytes() + b"changed")
    elif damage == "expired": context.alive = False
    elif damage == "parsed_image": live.inputs.image.base += 4096
    elif damage == "parsed_map": object.__setattr__(live.inputs.mapping, "base", 0)
    elif damage == "evidence": live.inputs.evidence["arguments"].append("/FORCE")
    elif damage == "records": live.inputs.records.clear()
    elif damage == "cache": live.inputs.objects._resolved[OWNER.casefold()]["raw"] += b"changed"
    else: raise AssertionError(damage)


def test_native_context_matches_direct_observation_and_returns_defensive_diagnostics(tmp_path):
    context, inputs = native_context(tmp_path / "project")
    expected = observe(inputs)
    with linked_context.linked_build(context) as live:
        report = live.data_section(OWNER, 1)
        assert report == expected
        saved = deepcopy(report)
        report["input"]["path"] = "forged"
        report["anchors"].clear()
        report["symbols"].clear()
        report["result"] = "forged"
        assert live.data_section(OWNER, 1) == saved
    assert context.alive
    with pytest.raises(ValueError, match="expired"):
        live.data_section(OWNER, 1)


@pytest.mark.parametrize("damage", ["archive", "object", "tool", "image", "map", "log", "receipt", "expired",
                                    "parsed_image", "parsed_map", "evidence", "records", "cache"])
@pytest.mark.parametrize("at_close", [False, True])
def test_live_envelope_rejects_physical_and_parsed_mutation(tmp_path, damage, at_close):
    context, _ = native_context(tmp_path / "project")
    with pytest.raises(ValueError):
        with linked_context.linked_build(context) as live:
            assert live.data_section(OWNER, 1)["placement"] == "pass"
            mutate(context, live, damage)
            if not at_close:
                with pytest.raises(ValueError): live.data_section(OWNER, 1)


@pytest.mark.parametrize("damage", ["archive", "object", "image", "parsed_image", "cache", "expired"])
def test_native_final_guard_rejects_mutation_inside_observer(tmp_path, monkeypatch, damage):
    context, _ = native_context(tmp_path / "project")
    original = linked_storage.observe_data_section
    with pytest.raises(ValueError):
        with linked_context.linked_build(context) as live:
            def altered(*args):
                report = original(*args)
                assert report["placement"] == "pass"
                mutate(context, live, damage)
                return report
            monkeypatch.setattr(linked_storage, "observe_data_section", altered)
            with pytest.raises(ValueError): live.data_section(OWNER, 1)


def test_observation_refusal_still_runs_the_native_final_guard(tmp_path, monkeypatch):
    context, _ = native_context(tmp_path / "project")
    with pytest.raises(ValueError, match="expired"):
        with linked_context.linked_build(context) as live:
            def refused(*args):
                context.alive = False
                raise ValueError("unsupported observation")
            monkeypatch.setattr(linked_storage, "observe_data_section", refused)
            with pytest.raises(ValueError, match="expired"):
                live.data_section(OWNER, 1)
