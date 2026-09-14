"""Independent physical MODI/SC and native-epoch damage controls."""

from copy import deepcopy
import struct

import pytest

from match_recovery import linked_context, pdb_contributions
from match_recovery.msf import SmallMsf
from test_linked_context import mutate
from test_linked_initializers import build, update_artifact
from test_msf import small_msf
from test_pdb_build_binding import add_bound_pdb
from test_pdb_contributions import bind, contribution, module, parts


def native_context(root):
    context = build(root)
    path = add_bound_pdb(context)
    msf = SmallMsf(path.read_bytes())
    streams = [msf.stream(index) for index in range(msf.stream_count)]
    mods = module(first=contribution())
    rows = contribution() + contribution(offset=32, size=4)
    header = bytearray(streams[3][:64])
    struct.pack_into("<II", header, 24, len(mods), 4 + len(rows))
    streams[3] = bytes(header) + mods + struct.pack("<I", 0xF12EBA2D) + rows + streams[3][64:]
    update_artifact(context, "app.pdb", small_msf(streams))
    return context


@pytest.mark.parametrize("cut", [0, 1, 4, 28, 32, 60, 63])
def test_incomplete_module_header_never_yields_a_partial_inventory(cut):
    with pytest.raises(ValueError):
        bind(*parts(module()[:cut], contribution()))


@pytest.mark.parametrize("names", [b"", b"name", b"name\0", b"name\0object",
                                   b"\0object\0", b"bad\x1fname\0obj\0", b"name\0bad\xff\0",
                                   b"x" * 4097 + b"\0obj\0"])
def test_names_require_two_bounded_supported_identities(names):
    raw = module()[:64] + names
    # Do not append zero padding: it could itself legitimately terminate a name.
    raw += b"X" * (-len(raw) % 4)
    with pytest.raises(ValueError):
        bind(*parts(raw))


def test_ignored_pointer_and_padding_fields_do_not_fabricate_semantics():
    mods = bytearray(module(name=b"x", object_name=b"", first=contribution()))
    for offset in (0, 52):
        struct.pack_into("<I", mods, offset, 0xFFFFFFFF)
    for offset in (6, 7, 22, 23, 50, 51, len(mods) - 1):
        mods[offset] = 0xA5
    struct.pack_into("<H", mods, 32, 1)  # Historical fWritten, not authority.
    record = bytearray(contribution())
    for offset in (2, 3, 18, 19):
        record[offset] = 0xA5
    result = bind(*parts(bytes(mods), bytes(record)))
    assert result.modules[0].first == result.contributions[0]
    assert result.report()["allocation_isolation"] == "unknown"


def test_nil_first_section_ignores_stale_non_region_fields_but_not_real_rows():
    first = contribution(section=0xFFFF, offset=-1, size=-1, module=0xFFFF,
                         flags=0xFFFFFFFF, data_crc=0xFFFFFFFF, relocation_crc=0xFFFFFFFF)
    result = bind(*parts(module(first=first)))
    assert result.modules[0].first is None
    assert len(result.contributions) == 1
    with pytest.raises(ValueError):
        bind(*parts(module(first=first), first))


@pytest.mark.parametrize("stream", [0, 1, 2, 3, 4, 5, 6, 100])
def test_module_cannot_alias_core_optional_or_missing_streams(stream):
    with pytest.raises(ValueError):
        bind(*parts(module(stream=stream)))


def test_two_modules_cannot_alias_one_present_stream():
    raw, streams = parts(module(stream=6) + module(stream=6))
    streams.append(b"")
    with pytest.raises(ValueError, match="aliased"):
        bind(raw, streams)


@pytest.mark.parametrize("offset", [12, 16, 20])
def test_module_cannot_alias_a_declared_dbi_symbol_stream(offset):
    raw, streams = parts(module(stream=6))
    streams.append(b"")
    dbi = bytearray(streams[3])
    struct.pack_into("<H", dbi, offset, 6)
    streams[3] = bytes(dbi)
    with pytest.raises(ValueError, match="aliased|reserved"):
        bind(raw, streams)


@pytest.mark.parametrize("lengths", [(-1, 0, 0), (0, -1, 0), (0, 0, -1),
                                     (1, 0, 0), (0, 1, 0), (0, 0, 1)])
def test_absent_module_stream_cannot_supply_declared_bytes(lengths):
    with pytest.raises(ValueError):
        bind(*parts(module(lengths=lengths)))


@pytest.mark.parametrize("lengths", [(5, 0, 0), (0, 5, 0), (0, 0, 5), (2, 2, 1),
                                     (0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF)])
def test_module_declared_regions_cannot_exceed_physical_stream(lengths):
    raw, streams = parts(module(stream=6, lengths=lengths))
    streams.append(bytes(4))
    with pytest.raises(ValueError, match="backing"):
        bind(raw, streams)


def test_stream_global_reference_tail_is_not_mistaken_for_declared_symbol_bytes():
    raw, streams = parts(module(stream=6, lengths=(4, 2, 2)))
    streams.append(bytes(8) + struct.pack("<I", 4) + bytes(4))
    result = bind(raw, streams)
    assert (result.modules[0].symbol_bytes, result.modules[0].line_bytes,
            result.modules[0].c13_bytes) == (4, 2, 2)
    assert result.report()["physical_object_binding"] == "unknown"


@pytest.mark.parametrize("offset,value", [(32, 2), (32, 4), (32, 0x100), (56, 1), (60, 1)])
def test_ec_tsm_and_uninterpreted_module_flags_refuse(offset, value):
    mods = bytearray(module())
    struct.pack_into("<H" if offset == 32 else "<I", mods, offset, value)
    with pytest.raises(ValueError, match="unsupported"):
        bind(*parts(bytes(mods)))


@pytest.mark.parametrize("index", [1, 4, 5])
def test_required_pair_stream_absence_cannot_produce_contributions(index):
    raw, streams = parts()
    streams[index] = None
    with pytest.raises(ValueError):
        bind(raw, streams)


@pytest.mark.parametrize("damage", ["signature", "age", "section", "version", "partial_sc"])
def test_contributions_never_bypass_image_pair_and_record_binding(damage):
    raw, streams = parts()
    index = 1 if damage in {"signature", "age"} else 5 if damage == "section" else 3
    data = bytearray(streams[index])
    if damage in {"signature", "age"}:
        data[4 if damage == "signature" else 8] ^= 1
    elif damage == "section":
        data[12] ^= 1
    elif damage == "version":
        data[64 + len(module())] ^= 1
    else:
        data.pop(64 + len(module()) + 4)
        struct.pack_into("<I", data, 28, 31)
    streams[index] = bytes(data)
    with pytest.raises(ValueError):
        bind(raw, streams)


def test_native_observation_is_defensive_and_expires_with_its_handle(tmp_path):
    context = native_context(tmp_path / "project")
    with linked_context.linked_build(context) as live:
        expected = pdb_contributions.bound_contributions(
            live.inputs.image, live.inputs.artifacts["msvc-pdb"]).report()
        result = live.contributions()
        assert result == expected
        saved = deepcopy(result)
        result["modules"].clear()
        result["contributions"][0]["size"] = 999
        assert live.contributions() == saved
        assert result["result"] == "unknown" and not result["ready_for_collection"]
    assert context.alive
    with pytest.raises(ValueError, match="expired"):
        live.contributions()


def damage_live(context, live, damage):
    if damage == "pdb":
        path = context.directory / "app.pdb"
        path.write_bytes(path.read_bytes() + b"changed")
    elif damage == "parsed_pdb":
        live.inputs.artifacts["msvc-pdb"] += b"changed"
    else:
        mutate(context, live, damage)


@pytest.mark.parametrize("damage", ["pdb", "parsed_pdb", "object", "image_file", "map_file",
                                    "receipt_file", "tool", "image_base", "records", "expired"])
@pytest.mark.parametrize("at_close", [False, True])
def test_native_before_and_close_guards_refuse_changed_evidence(tmp_path, damage, at_close):
    context = native_context(tmp_path / "project")
    with pytest.raises(ValueError):
        with linked_context.linked_build(context) as live:
            assert live.contributions()["enumeration"] == "pass"
            damage_live(context, live, damage)
            if not at_close:
                with pytest.raises(ValueError):
                    live.contributions()


@pytest.mark.parametrize("damage", ["pdb", "parsed_pdb", "object", "image_base", "expired"])
def test_native_final_guard_refuses_damage_inside_successful_observer(tmp_path, monkeypatch, damage):
    context = native_context(tmp_path / "project")
    original = pdb_contributions.bound_contributions
    with pytest.raises(ValueError):
        with linked_context.linked_build(context) as live:
            def changed(*args, **kwargs):
                result = original(*args, **kwargs)
                damage_live(context, live, damage)
                return result
            monkeypatch.setattr(pdb_contributions, "bound_contributions", changed)
            with pytest.raises(ValueError):
                live.contributions()


def test_native_final_guard_also_runs_after_parser_refusal(tmp_path, monkeypatch):
    context = native_context(tmp_path / "project")
    with pytest.raises(ValueError, match="expired"):
        with linked_context.linked_build(context) as live:
            def refused(*args, **kwargs):
                context.alive = False
                raise ValueError("unsupported contribution format")
            monkeypatch.setattr(pdb_contributions, "bound_contributions", refused)
            with pytest.raises(ValueError, match="expired"):
                live.contributions()
