"""A switch-table owner may also carry its own VC6 `.text$x` EH region.

The switch data stays bound to the primary root body; the EH tail and its
`.xdata$x` FuncInfo are associative children the caller compares separately.
VC6 writes the tail's labels as S_LABEL32 records before the procedure.
"""
import struct

import pytest

from match_recovery.associated_binary import compare_associated_code
from match_recovery.codeview_procedures import S_LABEL32_ST
from match_recovery.linked_codeview import source_procedure
from match_recovery.model import Result
from match_recovery.pe_coff import Coff
from test_associated_binary import fixture, BODY, GAP
from test_codeview_procedures import procedure, record, name

TAIL = b"\xb8\0\0\0\0\xe9\0\0\0\0"
TAIL_FLAGS, FUNCINFO_FLAGS, DEBUG_FLAGS = 0x60501020, 0x40401040, 0x42101048


def _short(value):
    raw = value.encode("ascii")
    assert len(raw) <= 8
    return raw.ljust(8, b"\0")


def extend(raw, children):
    """Append associative children of section 1, keeping symbol indexes.

    Each child is (name, flags, data, relocations, labels): relocations are
    (offset, symbol index, kind) and labels (name, value) local label symbols
    of that child. Returns the new raw COFF.
    """
    count, = struct.unpack_from("<H", raw, 2)
    symbol_at, symbol_count = struct.unpack_from("<II", raw, 8)
    old_end = 20 + 40 * count
    body, symbols = raw[old_end:symbol_at], bytearray(raw[symbol_at:symbol_at + 18 * symbol_count])
    strings = raw[symbol_at + 18 * symbol_count:]
    total = count + len(children)
    shift = 40 * len(children)
    headers = bytearray(raw[20:old_end])
    for at in range(0, len(headers), 40):
        for field in (20, 24):
            value, = struct.unpack_from("<I", headers, at + field)
            if value:
                struct.pack_into("<I", headers, at + field, value + shift)
    payload, at = bytearray(), 20 + 40 * total + len(body)
    for number, (child, flags, data, relocations, labels) in enumerate(children, count + 1):
        blob = b"".join(struct.pack("<IIH", *row) for row in relocations)
        headers += _short(child) + struct.pack("<IIIIIIHHI", 0, 0, len(data), at,
                                               at + len(data) if blob else 0, 0,
                                               len(relocations), 0, flags)
        payload += data + blob
        at += len(data) + len(blob)
        symbols += _short(child) + struct.pack("<IhHBB", 0, number, 0, 3, 1)
        symbols += struct.pack("<IHHIHB3x", len(data), len(relocations), 0, 0, 1, 5)
        for label, value in labels:
            symbols += _short(label) + struct.pack("<IhHBB", value, number, 0, 6, 0)
    header = bytearray(raw[:20])
    struct.pack_into("<H", header, 2, total)
    struct.pack_into("<II", header, 8, at, len(symbols) // 18)
    return bytes(header + headers + body + payload + symbols + strings)


def tail_label_record(label):
    return record(S_LABEL32_ST, struct.pack("<IHB", 0, 0, 0) + name(label))


# Symbol indexes after extend(): 6 .text$x, 7 aux, 8 $L9, 9 .xdata$x, 10 aux.
def eh_case(*, debug=False, tail_flags=TAIL_FLAGS, funcinfo=True, label="$L9",
            debug_target=8, prefix=None):
    obj, subject, span, image, bindings, targets = fixture()
    children = [(".text$x", tail_flags, TAIL, (), [("$L9", 0)])]
    if funcinfo:
        children.append((".xdata$x", FUNCINFO_FLAGS, bytes(8), (), []))
    if debug:
        prefix = tail_label_record(label) if prefix is None else prefix
        data = prefix + procedure(length=len(BODY) + len(GAP) + 5, debug_start=0,
                                  debug_end=len(BODY), label="_f", locals_=[])
        start = len(prefix)
        relocations = [(4, debug_target, 11), (8, debug_target, 10),
                       (start + 32, 0, 11), (start + 36, 0, 10)]
        children.append((".debug$S", DEBUG_FLAGS, data, relocations, []))
    candidate = Coff(extend(obj.raw_data, children))
    return candidate, candidate.function(subject.name), span, image, bindings, targets


@pytest.mark.parametrize("debug", [False, True])
def test_switch_owner_with_its_eh_tail_compares_the_primary_body(debug):
    report = compare_associated_code(*eh_case(debug=debug), eh_tail=2)
    assert report["result"] == Result.PASS
    assert len(report["source_debug_metadata"]) == int(debug)


def test_eh_children_are_unclassified_without_the_callers_tail():
    with pytest.raises(ValueError, match="unclassified child"):
        compare_associated_code(*eh_case())


@pytest.mark.parametrize("damage", [{"tail_flags": 0x60301020}, {"funcinfo": False}])
def test_eh_layout_damage_refuses(damage):
    with pytest.raises(ValueError, match="exact .text\\$x EH tail"):
        compare_associated_code(*eh_case(**damage), eh_tail=2)


def test_eh_tail_must_name_the_code_child():
    with pytest.raises(ValueError, match="exact .text\\$x EH tail"):
        compare_associated_code(*eh_case(), eh_tail=3)


@pytest.mark.parametrize("damage", [{"label": "$L8"}, {"debug_target": 5},
                                    {"prefix": b""}])
def test_eh_tail_debug_label_damage_refuses(damage):
    with pytest.raises(ValueError):
        compare_associated_code(*eh_case(debug=True, **damage), eh_tail=2)


def test_leading_tail_labels_need_the_explicit_tail():
    obj, root, *_ = eh_case(debug=True)
    with pytest.raises(ValueError, match="must begin with one supported procedure"):
        source_procedure(obj, 4, root, local_addresses=True)
    assert source_procedure(obj, 4, root, local_addresses=True, eh_tail=2).name == "_f"
    with pytest.raises(ValueError, match="EH-tail labels require"):
        source_procedure(obj, 4, root, local_addresses=True, eh_tail=3)
