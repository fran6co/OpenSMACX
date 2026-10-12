"""Complete original mappings, not emitted-lifetime or runtime acceptance.

The original is read directly through the installed PE reader. These bounded
instruction/metadata witnesses do not consume an IDB or an ignored report, and
do not qualify current compiler output, dependency selection, or game behavior.
"""
from dataclasses import dataclass, fields
from pathlib import Path
import os
import struct
import subprocess

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import read, read_text


ROOT = Path(__file__).resolve().parents[2]
BASE = "acabdca02e18b80738bf6d5bf332a78a0de4e173"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
EXTENTS = {
    0x408410: ("src/basewin.cpp", ((0x408410, 0x40846B),)),
    0x6183D0: ("src/caviar.cpp", ((0x6183D0, 0x618426),)),
    0x618560: ("src/caviar.cpp", ((0x618560, 0x6185A0),)),
    0x43AC70: ("src/designwin.cpp", ((0x43AC70, 0x43BDE9),
                                     (0x654410, 0x6546C5))),
    0x472020: ("src/messagewin.cpp", ((0x472020, 0x472184),
                                      (0x655EA0, 0x655F23))),
    0x4BA120: ("src/statuswin.cpp", ((0x4BA120, 0x4BA19E),
                                     (0x659A20, 0x659A5F))),
    # Preserve the historical primary extent; its constructor/dispatcher
    # interpretation is separate from this newly restored cold obligation.
    0x612830: ("src/dialogs.cpp", ((0x612830, 0x612AE0),
                                   (0x663060, 0x6630D3))),
    0x614E50: ("src/editbox.cpp", ((0x614E50, 0x614F2C),
                                   (0x663130, 0x663142))),
}


@dataclass(frozen=True)
class EHCase:
    parent: int
    prolog_push: int
    handler: int
    metadata: int
    unwind_map: int
    actions: tuple[int, ...]


EH_CASES = (
    EHCase(0x43AC70, 0x43AC7B, 0x6546BB, 0x674980, 0x6749A0, (
        0x654410, 0x65441B, 0x654426, 0x654431, 0x65443C, 0x654447,
        0x654452, 0x65445D, 0x654468, 0x654473, 0x65447E, 0x654489,
        0x654491, 0x65449F, 0x6544AD, 0x6544B5, 0x6544BD, 0x6544C8,
        0x6544D3, 0x6544DE, 0x6544E9, 0x6544F4, 0x6544FF, 0x65450A,
        0x654515, 0x654520, 0x65452B, 0x654536, 0x65453E, 0x65454C,
        0x65455A, 0x654562, 0x65456A, 0x654575, 0x654580, 0x65458B,
        0x654596, 0x6545A1, 0x6545AC, 0x6545B7, 0x6545C2, 0x6545CD,
        0x6545D8, 0x6545E0, 0x6545E8, 0x6545F3, 0x6545FE, 0x654606,
        0x654614, 0x654622, 0x65462A, 0x654632, 0x65463D, 0x654648,
        0x654653, 0x65465E, 0x654669, 0x654674, 0x65467F, 0x65468A,
        0x654695, 0x6546A0, 0x6546AB, 0x6546B3)),
    EHCase(0x472020, 0x472025, 0x655F19, 0x675DA0, 0x675DC0, (
        0x655EA0, 0x655EA8, 0x655EB3, 0x655EBE, 0x655EC6, 0x655ED4,
        0x655EE2, 0x655EED, 0x655EF5, 0x655F03, 0x655F11)),
    EHCase(0x4BA120, 0x4BA125, 0x659A55, 0x678CC0, 0x678CE0, (
        0x659A20, 0x659A2B, 0x659A39, 0x659A47)),
    EHCase(0x612830, 0x612832, 0x6630C9, 0x67FD68, 0x67FD88, (
        0x663060, 0x66307D, 0x66309A, 0x6630A5, 0x6630B0, 0x6630BB)),
    EHCase(0x614E50, 0x614E52, 0x663138, 0x67FE18, 0x67FE38, (0x663130,)),
)


def cpp_tokens(text):
    """Ignore comments/spacing but retain every literal byte and code token."""
    result, cursor = [], 0
    for match in COMMENTS_OR_STRINGS.finditer(text):
        result.extend(text[cursor:match.start()].split())
        if not match[0].startswith(("//", "/*")):
            result.append(match[0])
        cursor = match.end()
    result.extend(text[cursor:].split())
    return result


def lesson_text(record):
    return [(item.kind, item.key, item.prose, tuple(text for _, text in item.lines))
            for item in record.origin or ()]


@pytest.fixture(scope="module")
def source_pairs():
    result = {}
    for relative in sorted({path for path, _ in EXTENTS.values()}):
        path = ROOT / relative
        before = subprocess.check_output(
            ["git", "show", f"{BASE}:{relative}"], cwd=ROOT, text=True)
        current = path.read_text()
        result[relative] = (before, current,
                            {r.address: r for r in read_text(before, path)},
                            {r.address: r for r in read_text(current, path)})
    return result


def test_only_eight_extents_change_and_cpp_claims_identities_survive(source_pairs):
    from test_caviar_record_api_typing_oct12 import assert_api_symbol_transition, restore_api_migration
    for relative, (before, current, old, new) in source_pairs.items():
        compared = restore_api_migration(current) if relative == "src/caviar.cpp" else current
        assert cpp_tokens(compared) == cpp_tokens(before), relative
        assert new.keys() == old.keys(), relative
        for address, row in new.items():
            previous = old[address]
            for field in fields(row):
                if field.name not in {"line", "region", "origin", "image_spans", "symbol"}:
                    assert getattr(row, field.name) == getattr(previous, field.name), (
                        relative, hex(address), field.name)
            assert_api_symbol_transition(address, previous.symbol, row.symbol)
            assert lesson_text(row) == lesson_text(previous)
            expected = EXTENTS[address][1] if address in EXTENTS else previous.image_spans
            assert row.image_spans == expected, (relative, hex(address))


@pytest.mark.parametrize("address", tuple(EXTENTS))
def test_complete_declared_spans(address, source_pairs):
    relative, expected = EXTENTS[address]
    assert source_pairs[relative][3][address].image_spans == expected


def test_expanded_bytes_do_not_claim_other_functions_or_components(source_pairs):
    from match_recovery.component_source import read_components
    current = list(read(ROOT / "src"))
    dependency_spans = [span for c in read_components(ROOT / "src")
                        for binding in c.bindings for span in binding.spans]
    for address, (relative, expected) in EXTENTS.items():
        old = source_pairs[relative][2][address].image_spans
        # This task only extends a prior end or appends a new cold span.
        added = [(previous[1], present[1]) for previous, present in zip(old, expected)
                 if previous[1] != present[1]] + list(expected[len(old):])
        for low, high in added:
            assert low < high
            assert not any(r.address != address and a < high and low < b
                           for r in current for a, b in r.image_spans)
            assert not any(s.start < high and low < s.end for s in dependency_spans)


@pytest.fixture(scope="module")
def original():
    path = Path(os.environ.get("OPENSMACX_IMAGE",
                               ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.exists() and "OPENSMACX_IMAGE" not in os.environ:
        pytest.skip("pinned image unavailable; set OPENSMACX_IMAGE for original evidence")
    return PeImage(path.read_bytes(), PIN)


@pytest.fixture(scope="module")
def decoder():
    capstone = pytest.importorskip("capstone")
    engine = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    engine.detail = True
    return engine


def decode(image, decoder, span):
    low, high = span
    instructions = list(decoder.disasm(image.read(Span(low, high)), low))
    assert instructions and instructions[0].address == low
    cursor = low
    for instruction in instructions:
        assert instruction.address == cursor
        cursor += instruction.size
    assert cursor == high, "span cuts an instruction or contains undecoded bytes"
    return instructions


def immediate(instruction):
    from capstone.x86_const import X86_OP_IMM
    assert instruction.operands[-1].type == X86_OP_IMM
    return instruction.operands[-1].imm


def assert_epilogue(image, decoder, address, span):
    rows = decode(image, decoder, span)
    by_address = {i.address: i for i in rows}
    if address == 0x408410:
        assert rows[-2].address == 0x408469
        assert (rows[-2].mnemonic, rows[-2].op_str) == ("pop", "esi")
        assert rows[-1].address == 0x40846A
        assert rows[-1].mnemonic == "ret" and not rows[-1].operands
        branch = by_address[0x408424]
        assert branch.mnemonic == "jne" and immediate(branch) == rows[-1].address
    else:
        expected_return = {0x6183D0: 0x618423, 0x618560: 0x61859D}[address]
        assert rows[-1].address == expected_return
        assert rows[-1].mnemonic == "ret" and immediate(rows[-1]) == 4
    assert rows[-1].address + rows[-1].size == span[1]


@pytest.mark.parametrize("address", (0x408410, 0x6183D0, 0x618560))
def test_pinned_complete_returns_and_load_inverts_branch(address, source_pairs, original, decoder):
    relative, _ = EXTENTS[address]
    assert_epilogue(original, decoder, address,
                    source_pairs[relative][3][address].image_spans[0])


@pytest.mark.parametrize("address", (0x408410, 0x6183D0, 0x618560))
def test_epilogue_controls_reject_all_three_old_truncations(address, source_pairs, original, decoder):
    relative, _ = EXTENTS[address]
    with pytest.raises(AssertionError):
        assert_epilogue(original, decoder, address,
                        source_pairs[relative][2][address].image_spans[0])


def assert_eh_chain(image, decoder, case, primary, cold):
    assert primary[0] == case.parent and primary[0] <= case.prolog_push < primary[1]
    push = decode(image, decoder, (case.prolog_push, case.prolog_push + 5))
    assert len(push) == 1 and push[0].mnemonic == "push"
    assert immediate(push[0]) == case.handler, "wrong parent-to-handler edge"
    assert case.prolog_push + 1 in image.relocations
    rows = decode(image, decoder, cold)
    starts = {i.address for i in rows}
    assert cold[0] == case.actions[0] and cold[1] == case.handler + 10
    handler = [i for i in rows if i.address >= case.handler]
    assert len(handler) == 2
    assert (handler[0].address, handler[0].mnemonic) == (case.handler, "mov")
    assert handler[0].op_str.startswith("eax,")
    assert immediate(handler[0]) == case.metadata
    assert case.handler + 1 in image.relocations
    assert handler[1].mnemonic == "jmp" and immediate(handler[1]) == 0x644FD6
    # Explicit VC6 metadata witness for these five parents, not a generic EH
    # parser or a provider/linked-selection proof.
    header = struct.unpack("<8I", image.read(Span(case.metadata, case.metadata + 32)))
    assert header == (0x19930520, len(case.actions), case.unwind_map, 0, 0, 0, 0, 0)
    assert case.metadata + 8 in image.relocations
    actions = []
    for state in range(len(case.actions)):
        cell = case.unwind_map + state * 8
        previous, action = struct.unpack("<iI", image.read(Span(cell, cell + 8)))
        assert -1 <= previous < state
        assert cell + 4 in image.relocations
        assert cold[0] <= action < case.handler and action in starts
        actions.append(action)
    assert tuple(actions) == case.actions, "wrong cleanup entry or action order"
    for low, high in zip(actions, (*actions[1:], case.handler)):
        block = decode(image, decoder, (low, high))
        assert block[0].mnemonic in {"mov", "lea"}
        assert block[-1].mnemonic in {"jmp", "ret"}


@pytest.mark.parametrize("case", EH_CASES, ids=lambda case: hex(case.parent))
def test_pinned_parent_handler_metadata_actions_and_full_cold_decode(case, source_pairs, original, decoder):
    relative, _ = EXTENTS[case.parent]
    record = source_pairs[relative][3][case.parent]
    assert_eh_chain(original, decoder, case, *record.image_spans)


class CorruptView:
    """Damage one read or relocation observation without altering the pinned file."""
    def __init__(self, image, address=0, replacement=b"", missing_relocation=None):
        self.image, self.address, self.replacement = image, address, replacement
        self.relocations = image.relocations - {missing_relocation}

    def read(self, span):
        data = bytearray(self.image.read(span))
        low = max(span.start, self.address)
        high = min(span.end, self.address + len(self.replacement))
        if low < high:
            data[low - span.start:high - span.start] = self.replacement[
                low - self.address:high - self.address]
        return bytes(data)


@pytest.mark.parametrize("case", EH_CASES, ids=lambda case: hex(case.parent))
@pytest.mark.parametrize("damage", ("wrong_parent_handler", "wrong_metadata", "wrong_action",
                                   "missing_action_relocation", "truncated_cold_return"))
def test_eh_controls_reject_wrong_edges_pointers_and_incomplete_extents(
        case, damage, original, decoder):
    primary, cold = EXTENTS[case.parent][1]
    if damage == "wrong_parent_handler":
        other = next(c for c in EH_CASES if c.parent != case.parent)
        damaged = CorruptView(original, case.prolog_push + 1, struct.pack("<I", other.handler))
    elif damage == "wrong_metadata":
        other = next(c for c in EH_CASES if c.parent != case.parent)
        damaged = CorruptView(original, case.handler + 1, struct.pack("<I", other.metadata))
    elif damage == "wrong_action":
        # Still inside the cold span, but in the middle of its first instruction.
        damaged = CorruptView(original, case.unwind_map + 4,
                              struct.pack("<I", cold[0] + 1))
    elif damage == "missing_action_relocation":
        damaged = CorruptView(original, missing_relocation=case.unwind_map + 4)
    else:
        damaged = original
        cold = (cold[0], cold[1] - 1)
    with pytest.raises(AssertionError):
        assert_eh_chain(damaged, decoder, case, primary, cold)


def test_epilogue_control_rejects_wrong_load_inverts_branch(original, decoder):
    damaged = CorruptView(original, 0x408425, struct.pack("<b", 0))
    with pytest.raises(AssertionError):
        assert_epilogue(damaged, decoder, 0x408410, EXTENTS[0x408410][1][0])
