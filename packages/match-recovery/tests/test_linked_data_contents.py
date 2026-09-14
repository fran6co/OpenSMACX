"""Physical ordinary data correspondence; no game or allocation acceptance."""

from dataclasses import replace
from hashlib import sha256
import struct
from types import SimpleNamespace

import pytest

from match_recovery.linked_data_contents import verify_data_contents
from match_recovery.linked_objects import SelectedObjects
from match_recovery.msvc_link_map import parse_link_map
from match_recovery.pe_coff import PeImage
from test_binary import pe_fixture
from test_linked_constants import symbol_object, constant
from test_linked_objects import library_inputs


OWNER = "RUNTIME:data.obj"
DATA = b"abcdefghijklmnop"
SYMBOLS = [("_anchor", 4, 1, 0, 3, b""), ("_inner", 8, 1, 0, 3, b""),
           ("_target", 4, 2, 0, 3, b""), ("_public", 0, 0, 0, 2, b"")]


def object_bytes(data=DATA, *, references=(), zero=False, symbols=SYMBOLS, flags=0xC0300040,
                 name=b".data"):
    specs = [(name, b"" if zero else data, 16 if zero else len(data),
              0xC0300080 if zero else flags, references),
             (b".rdata", b"ABCDEFGH", 8, 0xC0300040, ())]
    payload, headers = bytearray(), []
    start = 20 + len(specs) * 40
    for section_name, raw, size, characteristics, relocations in specs:
        raw_at = start + len(payload) if raw else 0
        payload.extend(raw)
        reloc_at = start + len(payload) if relocations else 0
        payload.extend(b"".join(struct.pack("<IIH", *row) for row in relocations))
        headers.append(struct.pack("<8sIIIIIIHHI", section_name, 0, 0, size, raw_at, reloc_at, 0,
                                   len(relocations), 0, characteristics))
    table = symbol_object(symbols)
    count, = struct.unpack_from("<I", table, 12)
    return (struct.pack("<HHIIIHH", 0x14C, len(specs), 0, start + len(payload), count, 0, 0)
            + b"".join(headers) + payload + table[20:])


def fixture(root, *, raw=None, contents=DATA, absolute=(), target=True, extra=(), finals=("data.obj",)):
    raw = object_bytes() if raw is None else raw
    evidence, coverage, records = library_inputs(root, [("data.obj/", raw), *extra], finals)
    image = bytearray(pe_fixture())
    struct.pack_into("<I", image, 0x98 + 16, 0x1000)
    struct.pack_into("<I", image, 0x178 + 36, 0xC0000040)
    image[0x210:0x210 + len(contents)] = contents
    image[0x230:0x238] = b"ABCDEFGH"
    if absolute:
        entries = [0x3000 + offset for offset in absolute]
        entries += [0] * (len(entries) % 2)
        block = struct.pack("<II", 0x1000, 8 + 2 * len(entries)) + struct.pack("<" + "H" * len(entries), *entries)
        image[0x380:0x380 + len(block)] = block
        struct.pack_into("<II", image, 0x98 + 96 + 5 * 8, 0x1180, len(block))
    mapping = (" Preferred load address is 00400000\n Address Publics by Value Rva+Base Lib:Object\n"
               " 0001:00000000 _f 00401000 f app.obj\n"
               " 0001:00000050 _public 00401050 other.obj\n"
               " entry point at 0001:00000000\n Static symbols\n"
               f" 0001:00000014 _anchor 00401014 {OWNER}\n"
               f" 0001:00000018 _inner 00401018 {OWNER}\n"
               + (f" 0001:00000034 _target 00401034 {OWNER}\n" if target else ""))
    return SimpleNamespace(objects=SelectedObjects(evidence, coverage, records), evidence=evidence,
                           mapping=parse_link_map(mapping.encode()),
                           image=PeImage(bytes(image), sha256(image).hexdigest()))


def observe(inputs):
    return verify_data_contents(inputs, OWNER, 1)


def pointer(address, tail=DATA[4:]):
    return struct.pack("<I", address) + tail


def test_entire_ordinary_section_is_checked_without_target_or_isolation_claim(tmp_path):
    value = observe(fixture(tmp_path))
    assert value["result"] == value["input_consistency"] == "pass"
    assert value["contents"]["alignment_fill"] == 0
    assert value["envelope"]["address"] == 0x401010 and value["envelope"]["size"] == 16
    assert value["raw_sha256"] == sha256(DATA).hexdigest()
    assert value["references"] == [] and value["target_contents"] == value["allocation_isolation"] == "unknown"
    assert not value["persisted"] and not value["ready_for_collection"]


@pytest.mark.parametrize("offset", [0, 3, 4, 15])
def test_changed_prefix_anchor_and_tail_bytes_fail(tmp_path, offset):
    changed = bytearray(DATA)
    changed[offset] ^= 1
    value = observe(fixture(tmp_path, contents=bytes(changed)))
    assert value["result"] == value["input_consistency"] == "fail"
    assert value["contents"]["differing_offsets"] == [offset]


@pytest.mark.parametrize("changed", [False, True])
def test_bss_checks_complete_declared_initial_zero_storage(tmp_path, changed):
    image = bytes(15) + bytes([int(changed)])
    value = observe(fixture(tmp_path, raw=object_bytes(zero=True), contents=image))
    assert value["result"] == ("fail" if changed else "pass")
    assert value["raw_sha256"] == sha256(bytes(16)).hexdigest()


def test_bss_virtual_tail_is_read_as_initial_zero_fill_not_missing_raw_code(tmp_path):
    inputs = fixture(tmp_path, raw=object_bytes(zero=True), contents=bytes(16))
    raw = bytearray(inputs.image.data)
    struct.pack_into("<I", raw, 0x178 + 8, 0x240)
    inputs.image = PeImage(bytes(raw), sha256(raw).hexdigest())
    inputs.mapping = replace(inputs.mapping, statics=tuple(
        replace(row, address=row.address + 0x200, offset=row.offset + 0x200)
        if row.symbol in {"_anchor", "_inner"} else row for row in inputs.mapping.statics))
    result = observe(inputs)
    assert result["result"] == "pass" and result["envelope"]["address"] == 0x401210


@pytest.mark.parametrize("target_index,address,selection", [(1, 0x401018, "pass"), (2, 0x401034, "pass"),
                                                           (3, 0x401050, "unknown")])
def test_self_private_and_public_map_references_retain_different_evidence(tmp_path, target_index, address, selection):
    value = observe(fixture(tmp_path, raw=object_bytes(pointer(0), references=((0, target_index, 6),)),
                            contents=pointer(address), absolute=(0x10,)))
    assert value["result"] == "pass"
    ref, = value["references"]
    assert ref["object_target"]["index"] == target_index and ref["address"] == address
    assert ref["kind"] == "absolute32" and ref["addend"] == 0
    assert ref["target_selection"] == selection and ref["target_contents"] == "unknown"
    if target_index == 2:
        assert ref["target_envelope"]["object_section"] == 2
        assert ref["identity"]["object_offset"] == 4


@pytest.mark.parametrize("kind", [6, 20])
def test_signed_addend_is_candidate_owned_and_strictly_compared(tmp_path, kind):
    expected = 0x401034 - 2 - (0x401014 if kind == 20 else 0)
    raw = object_bytes(pointer(0xFFFFFFFE), references=((0, 2, kind),))
    value = observe(fixture(tmp_path, raw=raw, contents=pointer(expected & 0xFFFFFFFF),
                            absolute=(0x10,) if kind == 6 else ()))
    assert value["result"] == "pass" and value["references"][0]["addend"] == -2


@pytest.mark.parametrize("damage", ["target", "addend", "missing_highlow", "extra_highlow", "missing_coff"])
def test_pointer_and_rebase_damage_cannot_be_masked(tmp_path, damage):
    raw = object_bytes(pointer(1 if damage == "addend" else 0), references=() if damage == "missing_coff" else ((0, 2, 6),))
    linked = pointer(0x401035 if damage == "target" else 0x401034)
    absolute = () if damage == "missing_highlow" else (0x10, 0x18) if damage == "extra_highlow" else (0x10,)
    value = observe(fixture(tmp_path, raw=raw, contents=linked, absolute=absolute))
    assert value["result"] == value["input_consistency"] == "fail"


def test_unplaced_private_target_refuses_even_when_linked_operand_looks_correct(tmp_path):
    with pytest.raises(ValueError, match="independent placement"):
        observe(fixture(tmp_path, raw=object_bytes(pointer(0), references=((0, 2, 6),)),
                        contents=pointer(0x401034), absolute=(0x10,), target=False))


@pytest.mark.parametrize("value", [0, 17, 0xFFFFFFFF])
def test_selected_external_constant_is_not_an_image_address(tmp_path, value):
    symbols = [*SYMBOLS, ("_K", 0, 0, 0, 2, b"")]
    raw = object_bytes(pointer(0), references=((0, 4, 6),), symbols=symbols)
    result = observe(fixture(tmp_path, raw=raw, contents=pointer(value),
                             extra=(("constant.obj/", constant(value)),), finals=("data.obj", "constant.obj")))
    assert result["result"] == "pass"
    ref, = result["references"]
    assert ref["constant_binding"]["value"] == value and "address" not in ref
    assert ref["target_selection"] == "pass"


def test_rebased_absolute_constant_fails(tmp_path):
    symbols = [*SYMBOLS, ("_K", 0, 0, 0, 2, b"")]
    raw = object_bytes(pointer(0), references=((0, 4, 6),), symbols=symbols)
    result = observe(fixture(tmp_path, raw=raw, contents=pointer(7), absolute=(0x10,),
                             extra=(("constant.obj/", constant(7)),), finals=("data.obj", "constant.obj")))
    assert result["result"] == "fail" and result["contents"]["unexpected_constant_base_relocations"] == [0]


def test_relative_absolute_constant_is_not_upgraded_to_supported_binding(tmp_path):
    symbols = [*SYMBOLS, ("_K", 0, 0, 0, 2, b"")]
    raw = object_bytes(pointer(0), references=((0, 4, 20),), symbols=symbols)
    result = observe(fixture(tmp_path, raw=raw, contents=pointer(0),
                             extra=(("constant.obj/", constant(7)),), finals=("data.obj", "constant.obj")))
    assert result["result"] == result["input_consistency"] == "unknown"


def test_noop_coff_relocation_cannot_explain_a_real_loader_relocation(tmp_path):
    linked = pointer(0x401034)
    result = observe(fixture(tmp_path, raw=object_bytes(linked, references=((0, 2, 0),)),
                             contents=linked, absolute=(0x10,)))
    assert result["contents"]["bytes"] == "pass"
    assert result["contents"]["missing_absolute_relocations"] == [0]
    assert result["result"] == "fail"


def test_target_contents_are_not_silently_verified_with_a_pointer_to_them(tmp_path):
    inputs = fixture(tmp_path, raw=object_bytes(pointer(0), references=((0, 2, 6),)),
                     contents=pointer(0x401034), absolute=(0x10,))
    raw = bytearray(inputs.image.data)
    raw[0x230] ^= 1
    inputs.image = PeImage(bytes(raw), sha256(raw).hexdigest())
    result = observe(inputs)
    assert result["result"] == "pass" and result["references"][0]["target_contents"] == "unknown"
    assert verify_data_contents(inputs, OWNER, 2)["result"] == "fail"


@pytest.mark.parametrize("references", [((15, 2, 6),), ((0, 2, 6), (2, 2, 6)), ((0, 2, 7),)])
def test_unsupported_overlapping_and_out_of_span_relocations_refuse(tmp_path, references):
    with pytest.raises(ValueError):
        observe(fixture(tmp_path, raw=object_bytes(references=references)))


@pytest.mark.parametrize("damage", ["alias", "unselected", "private_bounds", "self_bounds", "special"])
def test_ambiguous_and_unbounded_forms_cannot_borrow_an_envelope(tmp_path, damage):
    options = {}
    if damage == "alias": options["extra"] = (("data.obj/", object_bytes()),)
    elif damage == "unselected": options["finals"] = ()
    elif damage == "special": options["raw"] = object_bytes(name=b".tls")
    else:
        options.update(raw=object_bytes(pointer(16), references=((0, 2 if damage == "private_bounds" else 1, 6),)),
                       contents=pointer(0x401044), absolute=(0x10,))
    with pytest.raises(ValueError):
        observe(fixture(tmp_path, **options))


def test_final_physical_population_guard_runs_after_comparison(tmp_path, monkeypatch):
    value = fixture(tmp_path)
    original = value.image.read_initial
    def changed(span):
        raw = original(span)
        path = tmp_path / "runtime.lib"
        path.write_bytes(path.read_bytes() + b"changed during comparison")
        return raw
    monkeypatch.setattr(value.image, "read_initial", changed)
    with pytest.raises(ValueError, match="changed"):
        observe(value)


@pytest.mark.parametrize("warm", [False, True])
def test_physical_input_changes_refuse_even_with_warm_envelope_and_object_caches(tmp_path, warm):
    inputs = fixture(tmp_path)
    if warm:
        assert observe(inputs)["result"] == "pass"
    path = tmp_path / "runtime.lib"
    path.write_bytes(path.read_bytes() + b"changed before verification")
    with pytest.raises(ValueError):
        observe(inputs)


def test_map_alias_conflict_is_not_overridden_by_known_private_bytes(tmp_path):
    value = fixture(tmp_path, raw=object_bytes(pointer(0), references=((0, 2, 6),)),
                    contents=pointer(0x401034), absolute=(0x10,))
    target = next(row for row in value.mapping.statics if row.symbol == "_target")
    value.mapping = replace(value.mapping, statics=(*value.mapping.statics, replace(target, address=0x401038, offset=0x38)))
    with pytest.raises(ValueError):
        observe(value)
