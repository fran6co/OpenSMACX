"""Entry-rooted source-boundary controls for pinned original entries.

These checks establish source mapping identity, original PE control flow, and
preservation of neighboring identities/data. They do not establish compiler,
linked selection, ABI, runtime, or game acceptance.
"""
from __future__ import annotations

from dataclasses import replace
import hashlib
import os
from pathlib import Path
import struct

import pytest

from match_recovery.associated_source import parse_associated
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import Mode, read
from match_recovery.source.reader import read_text
from match_recovery.source.writer import write_bytes
from match_recovery.source_inventory import image_pin
from match_recovery.source_config import SourceConfiguration


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
SPACE = "pe:va"

TARGETS = {
    "src/diplopop.cpp": {
        0x004406F0: {
            "name": "?release_modal@DiploPop@@QAEXXZ",
            "symbol": "?release_modal@DiploPop@@UAEXXZ",
            "span": (0x004406F0, 0x004406F5),
        },
    },
    "src/midi.cpp": {
        0x004C8050: {
            "name": "?set_play_type@Midi@@QAEHI@Z",
            "symbol": "?set_play_type@Midi@@UAEHI@Z",
            "span": (0x004C8050, 0x004C806A),
        },
    },
    "src/video.cpp": {
        0x006365B0: {
            "name": "Video_unk1",
            "symbol": "?close@Video@@QAEXXZ",
            "span": (0x006365B0, 0x00636732),
        },
    },
    "src/net_fifo.cpp": {
        0x00633F70: {
            "name": "?get@NetFifo@@QAEHPAXPAIPAHPAI@Z",
            "symbol": "?get@NetFifo@@QAEHPAXPAIPAH1@Z",
            "span": (0x00633F70, 0x0063401D),
            "mode": Mode.BODY,
        },
    },
}

# Source identities in the historical overhang and immediately following entry.
# The 0x637190 leaf is an unreferenced constant-return function bound in
# video.cpp by address neighbourhood, not a recovered Video member.
VIDEO_NEIGHBORS = {
    0x00636740: ("Video_unk2", (0x00636740, 0x0063684D)),
    0x00636850: ("Video_unk3", (0x00636850, 0x006368C8)),
    0x006368D0: ("Video_unk4", (0x006368D0, 0x00636954)),
    0x00636960: ("Video_unk5", (0x00636960, 0x00636F99)),
    0x00636FC0: ("?process@Video@@QAEXXZ", (0x00636FC0, 0x0063706F)),
    0x00637070: ("Video_unk6", (0x00637070, 0x006370D2)),
    0x006370E0: ("?play@Video@@QAEHXZ", (0x006370E0, 0x0063718F)),
    0x006371A0: ("Video_unk8", (0x006371A0, 0x006371C2)),
    0x006371D0: ("Video_unk10", (0x006371D0, 0x006372F7)),
    0x00637300: ("Video_unk11", (0x00637300, 0x00637308)),
    0x00637310: ("Video_unk12", (0x00637310, 0x00637395)),
}

VIDEO_TABLE = (0x00636960, Span(0x00636F9C, 0x00636FB4), 0x006369D0)


def _image_path() -> Path:
    configured = os.environ.get("OPENSMACX_IMAGE")
    if configured:
        return Path(configured).expanduser().resolve()
    for root in (ROOT, *ROOT.parents):
        candidate = root / ".opensmacx/game/terranx_original.exe"
        if candidate.is_file():
            return candidate
    return ROOT / ".opensmacx/game/terranx_original.exe"


def _original() -> PeImage:
    assert image_pin(ROOT) == PIN
    path = _image_path()
    if not path.is_file():
        pytest.skip("pinned original unavailable; set OPENSMACX_IMAGE")
    raw = path.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    return PeImage(raw, PIN)


def _source_records(relative: str, raw: bytes | None = None):
    path = ROOT / relative
    if raw is None:
        return read_text(path.read_text(encoding="utf-8"), relative)
    return read_text(raw.decode("utf-8", errors="surrogateescape"), relative)


def _by_address(relative: str, raw: bytes | None = None):
    return {row.address: row for row in _source_records(relative, raw)}


def _all_records():
    for path in SourceConfiguration.read(ROOT).files(ROOT):
        yield from read(path)


def _assert_target_record(relative: str, rows) -> None:
    for address, expected in TARGETS[relative].items():
        row = rows[address]
        assert row.name == expected["name"]
        assert row.symbol == expected["symbol"]
        assert row.image_spans == (expected["span"],)
        expected_mode = expected.get(
            "mode", Mode.BODY if relative == "src/midi.cpp" else Mode.FILE)
        assert row.mode is expected_mode


def _assert_fifo_boundary_records(rows) -> None:
    _assert_target_record("src/net_fifo.cpp", rows)
    sibling = rows[0x00634020]
    assert (sibling.name, sibling.symbol, sibling.image_spans) == (
        "sub_634020", "?peek_key_caller@NetFifo@@QAEHPAXIHPAI@Z",
        ((0x00634020, 0x006340B5),))
    fifo = rows[0x00633F70]
    assert not (fifo.image_spans[0][0] < sibling.image_spans[0][1]
                and sibling.image_spans[0][0] < fifo.image_spans[0][1])


def _identity_projection(rows, addresses):
    return {
        address: (rows[address].name, rows[address].symbol,
                  rows[address].image_spans, rows[address].mode,
                  rows[address].byte_exact, rows[address].semantic)
        for address in addresses
    }


def _entry_flow(image: PeImage, start: int, end: int,
                tail_target: int | None = None):
    from capstone import CS_ARCH_X86, CS_GRP_JUMP, CS_GRP_RET, CS_MODE_32, Cs
    from capstone.x86 import X86_OP_IMM

    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    pending = [start]
    instructions = {}
    covered = {}
    exits = []
    escaped = []
    while pending:
        address = pending.pop()
        if address in instructions:
            continue
        assert start <= address < end, hex(address)
        decoded = list(decoder.disasm(image.read(Span(address, min(address + 15, end))),
                                      address, count=1))
        assert len(decoded) == 1, hex(address)
        instruction = decoded[0]
        stop = address + instruction.size
        assert stop <= end, (hex(address), instruction.mnemonic)
        for byte in range(address, stop):
            assert byte not in covered, (hex(byte), hex(covered.get(byte, 0)))
            covered[byte] = address
        instructions[address] = instruction

        if instruction.group(CS_GRP_RET):
            exits.append((address, "return", stop))
            continue

        if instruction.group(CS_GRP_JUMP):
            assert len(instruction.operands) == 1
            operand = instruction.operands[0]
            assert operand.type == X86_OP_IMM, (hex(address), instruction.op_str)
            destination = operand.imm
            if start <= destination < end:
                pending.append(destination)
            else:
                escaped.append((address, instruction.mnemonic, destination))
                assert instruction.mnemonic == "jmp"
                assert destination == tail_target
                exits.append((address, "tail_call", destination))
            if instruction.mnemonic == "jmp":
                continue

        assert stop < end, (hex(address), instruction.mnemonic, instruction.op_str)
        pending.append(stop)

    assert set(covered) == set(range(start, end))
    assert len(covered) == end - start
    return instructions, tuple(exits), tuple(escaped)


def test_current_mappings_keep_the_corrected_identities_and_flags():
    for relative in TARGETS:
        rows = _by_address(relative)
        _assert_target_record(relative, rows)


def test_pinned_entry_rooted_flows_end_inside_their_own_extents():
    image = _original()
    flows = (
        (0x004406F0, 0x004406F5, 0x005EE280),
        (0x004C8050, 0x004C806A, None),
        (0x006365B0, 0x00636732, None),
    )
    for start, end, tail_target in flows:
        instructions, exits, escaped = _entry_flow(image, start, end, tail_target)
        if tail_target is not None:
            assert escaped == ((start, "jmp", tail_target),)
            assert exits == ((start, "tail_call", tail_target),)
        else:
            assert escaped == ()
            assert len(exits) == 1 and exits[0][1] == "return"
            assert exits[0][2] == end
        assert min(instructions) == start

    release, _, _ = _entry_flow(image, 0x004406F0, 0x004406F5, 0x005EE280)
    assert (release[0x004406F0].mnemonic,
            release[0x004406F0].op_str) == ("jmp", "0x5ee280")
    midi, _, _ = _entry_flow(image, 0x004C8050, 0x004C806A)
    assert (midi[0x004C8067].mnemonic, midi[0x004C8067].op_str) == ("ret", "4")
    video, _, _ = _entry_flow(image, 0x006365B0, 0x00636732)
    assert (video[0x00636731].mnemonic, video[0x00636731].op_str) == ("ret", "")

    fifo, fifo_exits, fifo_escaped = _entry_flow(
        image, 0x00633F70, 0x0063401D)
    assert len(fifo) == 72 and fifo_escaped == ()
    assert {(address, kind, destination)
            for address, kind, destination in fifo_exits} == {
        (0x00633F91, "return", 0x00633F94),
        (0x0063401A, "return", 0x0063401D),
    }
    assert (fifo[0x00633F70].mnemonic, fifo[0x00633F70].op_str) == (
        "push", "ebp")

    # This separately mapped sibling begins after the alignment bytes and has
    # its own complete entry-rooted flow, rather than being a tail of get.
    peek, peek_exits, peek_escaped = _entry_flow(
        image, 0x00634020, 0x006340B5)
    assert len(peek) == 62 and peek_escaped == ()
    assert len(peek_exits) == 4
    assert (peek[0x00634020].mnemonic, peek[0x00634020].op_str) == (
        "push", "esi")
    assert image.read(Span(0x0063401D, 0x00634020)) == b"\x90\x90\x90"


def test_play_type_null_path_does_not_reach_padding_or_write_eax():
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs

    image = _original()
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    ops = list(decoder.disasm(image.read(Span(0x004C8050, 0x004C806A)),
                              0x004C8050))
    by_address = {op.address: op for op in ops}
    null_path = (0x004C8050, 0x004C8051, 0x004C8053, 0x004C8056,
                 0x004C8058, 0x004C8066, 0x004C8067)
    assert [(by_address[address].mnemonic, by_address[address].op_str)
            for address in null_path] == [
        ("push", "ebp"), ("mov", "ebp, esp"),
        ("mov", "ecx, dword ptr [ecx + 0x3c]"),
        ("test", "ecx, ecx"), ("je", "0x4c8066"),
        ("pop", "ebp"), ("ret", "4"),
    ]
    for address in null_path:
        _, written = by_address[address].regs_access()
        assert "eax" not in [decoder.reg_name(reg) for reg in written]
    assert by_address[0x004C8067].address + by_address[0x004C8067].size == 0x004C806A


def test_historical_overhang_is_padding_other_entries_or_associated_data():
    image = _original()
    raw_nops = (
        (0x004406F5, 0x00440700),
        (0x004C806A, 0x004C8070),
        (0x00636732, 0x00636740),
        (0x0063684D, 0x00636850),
        (0x006368C8, 0x006368D0),
        (0x00636954, 0x00636960),
        (0x00636FB4, 0x00636FC0),
        (0x0063706F, 0x00637070),
        (0x006370D2, 0x006370E0),
        (0x0063718F, 0x00637190),
        (0x00637195, 0x006371A0),
        (0x006371C2, 0x006371D0),
        (0x006372F7, 0x00637300),
        (0x00637308, 0x00637310),
        (0x0063401D, 0x00634020),
    )
    for start, end in raw_nops:
        assert image.read(Span(start, end)) == b"\x90" * (end - start)

    from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    alignment = list(decoder.disasm(image.read(Span(0x00636F99, 0x00636F9C)),
                                   0x00636F99))
    assert [(op.mnemonic, op.op_str, op.size) for op in alignment] == [
        ("lea", "ecx, [ecx]", 3)]

    expected_video = {
        address: (name, (span,)) for address, (name, span) in VIDEO_NEIGHBORS.items()
    }
    rows = _by_address("src/video.cpp")
    assert set(expected_video) <= rows.keys()
    for address, (name, spans) in expected_video.items():
        assert (rows[address].name, rows[address].image_spans) == (name, spans)

    # Retain the original identity and claim even if a later reviewed recovery
    # binds this leaf to a real source owner. Do not freeze the pending count.
    leaf, = [row for row in _all_records() if row.address == 0x00637190]
    assert leaf.name == "Video_unk7"
    assert leaf.image_spans == ((0x00637190, 0x00637195),)
    assert leaf.byte_exact
    leaf_ops = list(decoder.disasm(image.read(Span(0x00637190, 0x00637195)),
                                   0x00637190))
    assert [(op.mnemonic, op.op_str) for op in leaf_ops] == [
        ("xor", "eax, eax"), ("ret", "8")]

    unit, table, evidence = VIDEO_TABLE
    bindings = parse_associated(
        (ROOT / "src/video.cpp").read_text(encoding="utf-8"),
        "src/video.cpp", PIN, address_space=SPACE)
    selected = [row for row in bindings if row.owner.address == unit]
    assert len(selected) == 1
    binding, = selected
    assert (binding.span, binding.kind, binding.evidence.address, binding.unit) == (
        table, "switch_targets", evidence, "src/video.cpp")
    assert table.end - table.start == 6 * 4

    # The indexed jump is bounded to six entries by the original unsigned
    # range guard. All table relocations resolve to labels inside Video::load.
    load = list(decoder.disasm(image.read(Span(0x00636960, 0x00636F99)),
                               0x00636960))
    load_by_address = {op.address: op for op in load}
    assert (load_by_address[0x006369CB].mnemonic,
            load_by_address[0x006369CB].op_str) == ("cmp", "eax, 5")
    assert (load_by_address[0x006369CE].mnemonic,
            load_by_address[0x006369CE].op_str) == ("ja", "0x6369d7")
    assert (load_by_address[evidence].mnemonic,
            load_by_address[evidence].op_str) == (
        "jmp", "dword ptr [eax*4 + 0x636f9c]")
    targets = struct.unpack("<6I", image.read(table))
    assert targets == (0x006369FB, 0x006369D7, 0x006369D7,
                       0x006369D7, 0x006369D7, 0x006369E9)
    assert set(targets) <= load_by_address.keys()
    assert image.absolute_sites(table) == frozenset(range(0, 24, 4))


def test_native_boundary_rewrites_reject_old_ends_and_preserve_neighbors():
    for relative, targets in TARGETS.items():
        raw = (ROOT / relative).read_bytes()
        rows = _by_address(relative, raw)
        _assert_target_record(relative, rows)
        target_address, expected = next(iter(targets.items()))
        current = rows[target_address]
        start, end = expected["span"]
        historical_end = {
            "src/diplopop.cpp": 0x00440707,
            "src/midi.cpp": 0x004C8072,
            "src/video.cpp": 0x00637310,
            "src/net_fifo.cpp": 0x00634067,
        }[relative]

        other_addresses = set(rows) - {target_address}
        baseline_projection = _identity_projection(rows, other_addresses)
        bad_spans = (((start, historical_end),), ((start, end + 1),))
        for spans in bad_spans:
            bad_record = replace(current, image_spans=spans)
            damaged = write_bytes(raw, [bad_record])
            reread = _by_address(relative, damaged)
            with pytest.raises(AssertionError):
                _assert_target_record(relative, reread)
            assert _identity_projection(reread, other_addresses) == baseline_projection

        # Rewriting just this native marker cannot discard adjacent owner
        # annotations; for Video it must also leave the independent table row.
        corrected = write_bytes(raw, [replace(current, image_spans=((start, end),))])
        reread = _by_address(relative, corrected)
        _assert_target_record(relative, reread)
        assert _identity_projection(reread, other_addresses) == baseline_projection
        assert (reread[target_address].byte_exact, reread[target_address].semantic) == (
            current.byte_exact, current.semantic)
        if relative == "src/video.cpp":
            associated = parse_associated(
                corrected.decode("utf-8", errors="surrogateescape"),
                relative, PIN, address_space=SPACE)
            assert [(row.owner.address, row.span, row.kind, row.evidence.address)
                    for row in associated] == [
                (0x00636960, Span(0x00636F9C, 0x00636FB4),
                 "switch_targets", 0x006369D0)]


def test_corrected_code_spans_do_not_overlap_other_native_function_mappings():
    mappings = list(_all_records())
    corrected = {
        0x004406F0: Span(0x004406F0, 0x004406F5),
        0x004C8050: Span(0x004C8050, 0x004C806A),
        0x006365B0: Span(0x006365B0, 0x00636732),
        0x00633F70: Span(0x00633F70, 0x0063401D),
    }
    for owner, span in corrected.items():
        for row in mappings:
            if row.address == owner:
                continue
            for low, high in row.image_spans:
                assert not (low < span.end and span.start < high), (
                    hex(owner), hex(row.address), hex(low), hex(high))


@pytest.mark.parametrize("bad_end", [0x0063401E, 0x00634067])
def test_fifo_pop_extent_rejects_padding_and_neighbor_overhang(bad_end):
    image = _original()
    with pytest.raises(AssertionError):
        _entry_flow(image, 0x00633F70, bad_end)

    rows = _by_address("src/net_fifo.cpp")
    fifo = rows[0x00633F70]
    sibling = rows[0x00634020]
    _assert_fifo_boundary_records(rows)


def test_fifo_boundary_writer_damage_preserves_sibling_identity():
    relative = "src/net_fifo.cpp"
    raw = (ROOT / relative).read_bytes()
    rows = _by_address(relative, raw)
    fifo = rows[0x00633F70]
    sibling_before = rows[0x00634020]

    bad = write_bytes(raw, [replace(
        fifo, image_spans=((0x00633F70, 0x00634067),))])
    bad_rows = _by_address(relative, bad)
    with pytest.raises(AssertionError):
        _assert_fifo_boundary_records(bad_rows)
    assert bad_rows[0x00634020] == sibling_before

    corrected = write_bytes(raw, [replace(
        fifo, image_spans=((0x00633F70, 0x0063401D),))])
    corrected_rows = _by_address(relative, corrected)
    _assert_fifo_boundary_records(corrected_rows)
    assert corrected_rows[0x00634020] == sibling_before

    bad_sibling = write_bytes(raw, [replace(
        sibling_before, image_spans=((0x00634020, 0x006340B4),))])
    with pytest.raises(AssertionError):
        _assert_fifo_boundary_records(_by_address(relative, bad_sibling))
