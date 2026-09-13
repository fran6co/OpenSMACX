"""An unaddressed boundary label is metadata, never extra code or an address."""

from dataclasses import replace
import struct

import pytest

from match_recovery.pe_coff import Coff, Symbol, unaddressed_end_label


def end_object(*, value=8, kind=0, storage=6, auxiliary=b"", flags=0x60500020,
               body=b"\xcc" * 8, raw_backed=True, incoming=None, weak=None,
               section=1):
    symbols = [(b"_f", 0, 1, 32, 2, b""), (b"$end", value, section, kind, storage, auxiliary)]
    if weak is not None:
        symbols.append((b"_weak", 0, 0, 0, 105, weak))
    specs = [(b".text", body, (incoming[1],) if incoming and incoming[0] == 1 else (), flags),
             (b".debug$S", bytes(8), (incoming[1],) if incoming and incoming[0] == 2 else (), 0x42100040)]
    start = 20 + 40 * len(specs)
    payload, headers = bytearray(), []
    for number, (name, data, edges, section_flags) in enumerate(specs, 1):
        at = start + len(payload)
        payload.extend(data)
        reloc_at = start + len(payload) if edges else 0
        payload.extend(b"".join(struct.pack("<IIH", *row) for row in edges))
        headers.append(name.ljust(8, b"\0") + struct.pack("<IIIIIIHHI", 0, 0, len(data),
            0 if number == 1 and not raw_backed else at, reloc_at, 0, len(edges), 0, section_flags))
    records = b"".join(name.ljust(8, b"\0") + struct.pack("<IhHBB", offset, number, typ, cls, len(aux) // 18) + aux
                        for name, offset, number, typ, cls, aux in symbols)
    raw = (struct.pack("<HHIIIHH", 0x14C, len(specs), 0, start + len(payload), len(records) // 18, 0, 0)
           + b"".join(headers) + payload + records + struct.pack("<I", 4))
    return Coff(raw)


def qualifies(**options):
    obj = end_object(**options)
    return unaddressed_end_label(obj, obj.symbols[1])


def test_exact_unaddressed_code_end_label_does_not_extend_subject():
    obj = end_object()
    before = (obj.sections, dict(obj.symbols), dict(obj.auxiliary), obj.section_sizes)
    assert unaddressed_end_label(obj, obj.symbols[1]) is True
    assert len(obj.section_subject(1)[0]) == obj.symbols[1].value == 8
    assert (obj.sections, obj.symbols, obj.auxiliary, obj.section_sizes) == before


@pytest.mark.parametrize("value", [0, 7, 9, 0xFFFFFFFF])
def test_nearby_or_outside_offsets_are_not_end_labels(value):
    assert qualifies(value=value) is False


@pytest.mark.parametrize("kind,storage", [(32, 6), (0, 2), (32, 2), (0, 3), (32, 3), (0, 101), (0, 105)])
def test_function_public_static_and_other_symbol_classes_refuse(kind, storage):
    assert qualifies(kind=kind, storage=storage) is False


@pytest.mark.parametrize("auxiliary", [bytes(18), bytes(36), b"\xff" * 18])
def test_auxiliary_metadata_is_not_treated_as_inert(auxiliary):
    assert qualifies(auxiliary=auxiliary) is False


@pytest.mark.parametrize("flags", [0, 0x40500020, 0x60500040, 0xC0500080, 0x605000A0])
def test_label_requires_executable_raw_code_not_data_or_uninitialized_storage(flags):
    assert qualifies(flags=flags) is False


@pytest.mark.parametrize("options", [{"body": b"", "value": 0}, {"raw_backed": False},
                                      {"section": 0}, {"section": -1}, {"section": 2}])
def test_empty_unbacked_and_noncode_sections_refuse(options):
    assert qualifies(**options) is False


@pytest.mark.parametrize("source", [1, 2])
@pytest.mark.parametrize("kind", [0, 6, 7, 20])
def test_any_incoming_reference_including_debug_and_noop_forbids_end_label(source, kind):
    assert qualifies(incoming=(source, (0, 1, kind))) is False


def test_unrelated_relocation_does_not_make_the_label_addressed():
    assert qualifies(incoming=(1, (0, 0, 20))) is True


@pytest.mark.parametrize("mode", [1, 2, 3])
def test_weak_external_fallback_to_label_forbids_it(mode):
    assert qualifies(weak=struct.pack("<II10x", 1, mode)) is False


def test_unrelated_supported_weak_fallback_does_not_address_label():
    assert qualifies(weak=struct.pack("<II10x", 0, 3)) is True


@pytest.mark.parametrize("weak", [b"", bytes(36), struct.pack("<II10x", 1, 4),
                                  struct.pack("<II10x", 999, 3), struct.pack("<II9xB", 0, 3, 1)])
def test_malformed_weak_metadata_cannot_supply_absence_evidence(weak):
    assert qualifies(weak=weak) is False


def test_a_different_symbol_cannot_borrow_the_parsed_label_slot():
    obj = end_object()
    assert unaddressed_end_label(obj, replace(obj.symbols[1], name="$other")) is False
    assert unaddressed_end_label(obj, Symbol(999, "$end", 8, 1, 0, 6)) is False
    assert unaddressed_end_label(obj, None) is False
    assert unaddressed_end_label(None, obj.symbols[1]) is False
