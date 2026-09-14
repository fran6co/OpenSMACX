"""Physical ordinary section envelopes, never label-distance allocation guesses."""

from hashlib import sha256
import struct
from types import SimpleNamespace

import pytest

from match_recovery.linked_objects import SelectedObjects
from match_recovery.linked_storage import observe_data_section
from match_recovery.msvc_link_map import parse_link_map
from match_recovery.pe_coff import PeImage
from test_linked_constants import symbol_object
from test_linked_private_symbols import observed, local_object
from test_pe_debug import embedded_image


OWNER = "RUNTIME:private.obj"


def data_object(symbols=(("_private", 4, 3),), *, size=8, flags=0xC0300040,
                zero_fill=False, name=b".data"):
    raw = bytearray(symbol_object([(name, offset, 1, 0, storage, b"")
                                   for name, offset, storage in symbols], directives=bytes(8)))
    raw[20:28] = name.ljust(8, b"\0")
    struct.pack_into("<I", raw, 36, size)
    struct.pack_into("<I", raw, 56, flags)
    if zero_fill:
        struct.pack_into("<I", raw, 40, 0)
    return bytes(raw)


def fixture(root, *, raw=None, placements=(("_private", 0x1014, 3),), debug=None,
            extra=(), selected=True, image_flags=0xC0000040):
    evidence, coverage, records = observed(root, raw=data_object() if raw is None else raw,
                                          extra=extra, selected=selected)
    pe = bytearray(embedded_image([] if debug is None else debug))
    struct.pack_into("<I", pe, 0x98 + 16, 0x1000)
    struct.pack_into("<I", pe, 0x178 + 36, image_flags)
    image = PeImage(bytes(pe), sha256(pe).hexdigest())
    publics, statics = [" 0001:00000000 _f 00401000 f app.obj\n"], []
    for name, rva, storage in placements:
        row = f" 0001:{rva - 0x1000:08x} {name} {0x400000 + rva:08x} {OWNER}\n"
        (publics if storage == 2 else statics).append(row)
    mapping = parse_link_map(("storage\n Preferred load address is 00400000\n"
        " Address         Publics by Value              Rva+Base     Lib:Object\n" + "".join(publics)
        + " entry point at 0001:00000000\n Static symbols\n" + "".join(statics)).encode())
    return SimpleNamespace(objects=SelectedObjects(evidence, coverage, records),
                           evidence=evidence, mapping=mapping, image=image)


def observe(inputs, number=1):
    return observe_data_section(inputs, OWNER, number)


@pytest.mark.parametrize("storage", [2, 3])
@pytest.mark.parametrize("anchor", ["map", "debug", "both"])
def test_nonzero_anchor_preserves_whole_extent_and_provenance(tmp_path, storage, anchor):
    row = ("_private", 0x1014, 1, 0, storage, b"")
    value = fixture(tmp_path, raw=data_object((("_private", 4, storage),)),
                    placements=(("_private", 0x1014, storage),) if anchor != "debug" else (),
                    debug=[row] if anchor != "map" else [])
    report = observe(value)
    assert report["result"] == report["placement"] == "pass"
    assert (report["address"], report["size"]) == (0x401010, 8)
    assert report["object_section"] == report["image_section"] == 1
    assert report["input"] == {"path": str(tmp_path / "runtime.lib"), "member": "private.obj", "occurrence": 0}
    assert report["object_sha256"] == sha256(data_object((("_private", 4, storage),))).hexdigest()
    assert len(report["anchors"]) == (2 if anchor == "both" else 1)
    assert report["symbols"] == [{"index": 0, "name": "_private", "offset": 4, "storage": storage}]
    for dimension in ("contents", "allocation_isolation", "retained_data_coverage", "remaining_linked_effects"):
        assert report[dimension] == "unknown"
    assert not report["persisted"] and not report["ready_for_collection"]


def test_symbols_do_not_truncate_an_earlier_section_prefix_or_tail(tmp_path):
    specs = (("_first", 4, 2), ("_last", 6, 3))
    value = fixture(tmp_path, raw=data_object(specs),
                    placements=(("_first", 0x1014, 2), ("_last", 0x1016, 3)))
    report = observe(value)
    assert report["address"] == 0x401010 and report["size"] == 8
    assert len(report["symbols"]) == 2


def test_undefined_external_uses_do_not_invent_another_definition(tmp_path):
    reference = symbol_object([("_private", 0, 0, 0, 2, b"")])
    value = fixture(tmp_path, raw=data_object((("_private", 4, 2),)), placements=(),
                    debug=[("_private", 0x1014, 1, 0, 2, b"")], extra=[("reference.obj/", reference)])
    assert observe(value)["address"] == 0x401010


@pytest.mark.parametrize("declaration", [(4, 0, 0, 2), (0, -1, 0, 2), (0, 0, 0x20, 3)])
def test_nonordinary_references_cannot_hide_competing_external_identities(tmp_path, declaration):
    other = symbol_object([("_private", *declaration, b"")])
    value = fixture(tmp_path, raw=data_object((("_private", 4, 2),)), placements=(),
                    debug=[("_private", 0x1014, 1, 0, 2, b"")], extra=[("reference.obj/", other)])
    with pytest.raises(ValueError):
        observe(value)


@pytest.mark.parametrize("damage", ["auxiliary", "type", "machine", "import", "weak"])
def test_external_debug_identity_keeps_opaque_undefined_and_non_symbol_competitors(tmp_path, damage):
    from test_coff_inventory import imported
    if damage == "import":
        other = imported("_private")
    elif damage == "weak":
        other = symbol_object([("_private", 0, 0, 0, 2, b""),
            ("alias", 0, 0, 0, 105, struct.pack("<II10x", 0, 3))])
    else:
        other = symbol_object([("_private", 0, 0, 0x20 if damage == "type" else 0, 2,
                                bytes(18) if damage == "auxiliary" else b"")],
                              machine=0 if damage == "machine" else 0x14C)
    value = fixture(tmp_path, raw=data_object((("_private", 4, 2),)), placements=(),
                    debug=[("_private", 0x1014, 1, 0, 2, b"")], extra=[("reference.obj/", other)])
    with pytest.raises(ValueError):
        observe(value)


@pytest.mark.parametrize("offset", [0, 4])
def test_zero_fill_extent_is_the_coff_request_not_empty_raw_bytes(tmp_path, offset):
    raw = data_object((("_private", offset, 3),), flags=0xC0300080, zero_fill=True)
    report = observe(fixture(tmp_path, raw=raw, placements=(("_private", 0x1010 + offset, 3),)))
    assert report["zero_fill"] and report["size"] == 8 and report["address"] == 0x401010


@pytest.mark.parametrize("storage", [2, 3])
def test_conflicting_map_and_embedded_debug_cannot_choose_the_preferred_anchor(tmp_path, storage):
    value = fixture(tmp_path, raw=data_object((("_private", 4, storage),)),
                    placements=(("_private", 0x1014, storage),),
                    debug=[("_private", 0x1018, 1, 0, storage, b"")])
    with pytest.raises(ValueError, match="disagree"):
        observe(value)


def test_two_owner_qualified_names_must_agree(tmp_path):
    value = fixture(tmp_path, raw=data_object((("a", 0, 2), ("b", 4, 3))),
                    placements=(("a", 0x1010, 2), ("b", 0x1018, 3)))
    with pytest.raises(ValueError, match="disagree"):
        observe(value)


@pytest.mark.parametrize("damage", ["no_anchor", "unselected", "duplicate_member", "duplicate_private", "duplicate_debug",
                                    "policy", "symbol_extent", "within_name", "zero", "large",
                                    "backed_bss", "unbacked_data", "seam", "unaligned", "code_image", "readonly_image"])
def test_incomplete_or_incompatible_envelopes_refuse(tmp_path, damage):
    options = {}
    if damage == "no_anchor": options["placements"] = ()
    elif damage == "unselected": options["selected"] = False
    elif damage == "duplicate_member": options["extra"] = [("private.obj/", data_object())]
    elif damage == "duplicate_private":
        options.update(extra=[("other.obj/", local_object())], debug=[("_private", 0x1014, 1, 0, 3, b"")])
    elif damage == "duplicate_debug": options["debug"] = [("_private", 0x1014, 1, 0, 3, b"")] * 2
    elif damage == "symbol_extent": options["raw"] = data_object((("_private", 8, 3),))
    elif damage == "within_name": options["raw"] = data_object((("_private", 0, 3), ("_private", 4, 3)))
    elif damage == "zero": options["raw"] = data_object((("_private", 0, 3),), size=0)
    elif damage == "large": options["raw"] = data_object(size=16 * 1024 * 1024 + 1, flags=0xC0300080, zero_fill=True)
    elif damage == "backed_bss": options["raw"] = data_object(flags=0xC0300080)
    elif damage == "unbacked_data": options["raw"] = data_object(zero_fill=True)
    elif damage == "seam": options["placements"] = (("_private", 0x11FF, 3),)
    elif damage == "unaligned": options["placements"] = (("_private", 0x1015, 3),)
    elif damage == "code_image": options["image_flags"] = 0xE0000040
    elif damage == "readonly_image": options["image_flags"] = 0x40000040
    value = fixture(tmp_path, **options)
    if damage == "policy": value.evidence["arguments"].append("/FORCE")
    with pytest.raises(ValueError):
        observe(value)


@pytest.mark.parametrize("flags", [0xC0301040, 0xC0300840, 0xC2300040, 0xC03000C0,
                                    0xE0300040, 0xC0300020, 0x00300040, 0xC0000040])
def test_comdat_special_forms_and_unsupported_flags_are_not_ordinary_data(tmp_path, flags):
    with pytest.raises(ValueError):
        observe(fixture(tmp_path, raw=data_object(flags=flags)))


@pytest.mark.parametrize("name", [b".tls", b".tls$ZZ", b".idata$5", b".debug$T", b".edata", b".rsrc", b"/15"])
def test_special_linker_storage_requires_its_own_proof(tmp_path, name):
    with pytest.raises(ValueError):
        observe(fixture(tmp_path, raw=data_object(name=name)))


@pytest.mark.parametrize("number", [True, False, 0, 2, -1, "1", None])
def test_section_identity_is_exact_and_bounded(tmp_path, number):
    with pytest.raises(ValueError):
        observe(fixture(tmp_path), number)


@pytest.mark.parametrize("when", ["before", "during"])
def test_physical_population_remains_fresh_through_observation(tmp_path, monkeypatch, when):
    value = fixture(tmp_path)
    if when == "before":
        (tmp_path / "runtime.lib").write_bytes(b"changed")
    else:
        original = value.objects.resolve
        def changed(owner):
            result = original(owner)
            (tmp_path / "app.obj").write_bytes(b"unrelated changed input")
            return result
        monkeypatch.setattr(value.objects, "resolve", changed)
    with pytest.raises(ValueError):
        observe(value)
