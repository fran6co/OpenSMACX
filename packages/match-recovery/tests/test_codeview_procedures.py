"""Complete CV5 procedure records; no skipped locals or type correspondence."""

from dataclasses import FrozenInstanceError, asdict
import struct

import pytest

from match_recovery.codeview_procedures import (
    decode_procedure, S_END, S_REGISTER_ST, S_BPREL32_ST, S_LPROC32_ST, S_GPROC32_ST,
)


def record(kind, body=b"", *, alignment=1):
    payload = struct.pack("<H", kind) + body
    payload += bytes(-(2 + len(payload)) % alignment)
    return struct.pack("<H", len(payload)) + payload


def name(value):
    raw = value.encode("latin1")
    return bytes([len(raw)]) + raw


def procedure(*, alignment=1, kind=S_GPROC32_ST, parent=0, end=0, next_=0,
              length=19, debug_start=3, debug_end=15, type_index=0x1007,
              offset=0, section=0, flags=1, label="Owner::add", locals_=None):
    body = struct.pack("<8IHB", parent, end, next_, length, debug_start, debug_end,
                       type_index, offset, section, flags) + name(label)
    if locals_ is None:
        locals_ = [(S_REGISTER_ST, struct.pack("<IH", 0x1001, 18) + name("this")),
                   (S_BPREL32_ST, struct.pack("<iI", 8, 0x74) + name("input"))]
    return (record(kind, body, alignment=alignment)
            + b"".join(record(k, raw, alignment=alignment) for k, raw in locals_)
            + record(S_END, alignment=alignment))


def test_complete_unpadded_coff_procedure_preserves_every_field_and_local():
    raw = procedure()
    result = decode_procedure(raw)
    assert len(raw) == 87
    assert asdict(result) == {
        "kind": S_GPROC32_ST, "parent": 0, "end": 0, "next": 0,
        "length": 19, "debug_start": 3, "debug_end": 15, "type_index": 0x1007,
        "offset": 0, "section": 0, "flags": 1, "name": "Owner::add",
        "locals": ({"kind": S_REGISTER_ST, "type_index": 0x1001, "name": "this", "register": 18, "frame_offset": None},
                   {"kind": S_BPREL32_ST, "type_index": 0x74, "name": "input", "register": None, "frame_offset": 8}),
        "records": ({"offset": 0, "size": 50, "kind": S_GPROC32_ST, "padding": b""},
                    {"offset": 50, "size": 15, "kind": S_REGISTER_ST, "padding": b""},
                    {"offset": 65, "size": 18, "kind": S_BPREL32_ST, "padding": b""},
                    {"offset": 83, "size": 4, "kind": S_END, "padding": b""})}
    assert result.end_record_offset == 83 and result.end_offset == 87


def test_linked_pdb_alignment_offsets_and_type_indices_are_not_normalized():
    locals_ = [(S_REGISTER_ST, struct.pack("<IH", 0x1008, 18) + name("this")),
               (S_BPREL32_ST, struct.pack("<iI", 8, 0x74) + name("input"))]
    raw = procedure(alignment=4, end=344, section=1, type_index=0x100C, locals_=locals_)
    result = decode_procedure(raw, alignment=4, base_offset=256)
    assert len(raw) == 92 and result.end_record_offset == result.end == 344
    assert result.end_offset == 348 and result.type_index == 0x100C
    assert result.locals[0].type_index == 0x1008
    assert [r.padding for r in result.records] == [bytes(2), bytes(1), bytes(2), b""]
    assert [r.offset for r in result.records] == [256, 308, 324, 344]
    assert result != decode_procedure(procedure())  # No inferred correspondence.


@pytest.mark.parametrize("kind", [S_GPROC32_ST, S_LPROC32_ST])
def test_signed_frame_offsets_empty_names_duplicate_locals_and_raw_flags_are_retained(kind):
    local = (S_BPREL32_ST, struct.pack("<iI", -8, 0x1009) + name(""))
    result = decode_procedure(procedure(kind=kind, flags=0xFF, label="\xffname", locals_=[local, local]))
    assert result.kind == kind and result.flags == 0xFF and result.name == "\xffname"
    assert len(result.locals) == 2 and result.locals[0] == result.locals[1]
    assert result.locals[0].frame_offset == -8 and result.locals[0].name == ""


def test_scope_links_are_literal_not_inferred_from_local_record_offsets():
    result = decode_procedure(procedure(parent=7, end=123, next_=999))
    assert (result.parent, result.end, result.next) == (7, 123, 999)
    assert result.end != result.end_record_offset  # Enclosing-stream verification remains required.


@pytest.mark.parametrize("cut", range(87))
def test_every_truncation_of_complete_coff_form_refuses(cut):
    with pytest.raises(ValueError):
        decode_procedure(procedure()[:cut])


@pytest.mark.parametrize("cut", range(92))
def test_every_truncation_of_complete_pdb_form_refuses(cut):
    with pytest.raises(ValueError):
        decode_procedure(procedure(alignment=4)[:cut], alignment=4, base_offset=256)


@pytest.mark.parametrize("kind", [0xFFFF, S_GPROC32_ST, S_LPROC32_ST, 0x0207, 0x1012])
def test_unknown_or_nested_records_are_not_skipped(kind):
    raw = procedure(locals_=[(kind, b"opaque")])
    with pytest.raises(ValueError, match="unsupported"):
        decode_procedure(raw)


@pytest.mark.parametrize("kind", [S_END, S_REGISTER_ST, S_BPREL32_ST, 0x9])
def test_block_cannot_start_with_a_nonprocedure(kind):
    with pytest.raises(ValueError, match="begin"):
        decode_procedure(record(kind, b"opaque"))


@pytest.mark.parametrize("tail", [b"\0", bytes(4), record(S_END), procedure()])
def test_trailing_bytes_or_another_procedure_are_not_ignored(tail):
    with pytest.raises(ValueError, match="trailing"):
        decode_procedure(procedure() + tail)


@pytest.mark.parametrize("offset", [0, 50, 65, 83])
@pytest.mark.parametrize("size", [0, 1, 0xFFFF])
def test_record_size_damage_is_bounded(offset, size):
    raw = bytearray(procedure())
    struct.pack_into("<H", raw, offset, size)
    with pytest.raises(ValueError):
        decode_procedure(bytes(raw))


@pytest.mark.parametrize("offset", [50, 51, 67, 86, 87])
def test_nonzero_linked_padding_cannot_hide_extension_bytes(offset):
    raw = bytearray(procedure(alignment=4))
    raw[offset] = 1
    with pytest.raises(ValueError, match="padding"):
        decode_procedure(bytes(raw), alignment=4)


def test_padding_is_exact_not_arbitrary_trailing_zeros():
    raw = bytearray(procedure(alignment=4))
    size, = struct.unpack_from("<H", raw)
    raw[2 + size:2 + size] = bytes(4)
    struct.pack_into("<H", raw, 0, size + 4)
    with pytest.raises(ValueError, match="padding"):
        decode_procedure(bytes(raw), alignment=4)
    with pytest.raises(ValueError, match="padding"):
        decode_procedure(procedure(alignment=4))
    with pytest.raises(ValueError, match="padding"):
        decode_procedure(procedure(), alignment=4)


@pytest.mark.parametrize("options", [{"length": 0}, {"debug_start": 16}, {"debug_end": 20},
                                    {"offset": 0xFFFFFFFE, "length": 19}])
def test_invalid_code_or_debug_bounds_refuse(options):
    with pytest.raises(ValueError, match="bounds"):
        decode_procedure(procedure(**options))


@pytest.mark.parametrize("options", [{"alignment": True}, {"alignment": 2}, {"base_offset": True},
                                    {"base_offset": -1}, {"base_offset": 0xFFFFFFFE},
                                    {"alignment": 4, "base_offset": 1}])
def test_invalid_alignment_and_stream_offsets_refuse(options):
    with pytest.raises(ValueError):
        decode_procedure(procedure(), **options)


def test_decoded_values_are_immutable_and_no_mutable_input_is_retained():
    with pytest.raises(ValueError):
        decode_procedure(bytearray(procedure()))
    result = decode_procedure(procedure())
    with pytest.raises(FrozenInstanceError):
        result.type_index = 0
    with pytest.raises(FrozenInstanceError):
        result.locals[0].type_index = 0
    with pytest.raises(FrozenInstanceError):
        result.records[0].offset = 0


def test_declared_parser_resource_bounds_refuse_instead_of_truncating(monkeypatch):
    import match_recovery.codeview_procedures as implementation
    monkeypatch.setattr(implementation, "MAX_PROCEDURE_RECORDS", 3)
    with pytest.raises(ValueError, match="record bound"):
        decode_procedure(procedure())
    monkeypatch.setattr(implementation, "MAX_PROCEDURE_BYTES", 86)
    with pytest.raises(ValueError, match="bounded"):
        decode_procedure(procedure())
