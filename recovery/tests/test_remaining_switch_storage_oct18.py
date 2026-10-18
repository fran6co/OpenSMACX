"""Pinned proof and exact source transport for the reviewed switch data rows.

These checks establish source annotation transport and original-image facts;
they do not establish compilation, linked selection, or runtime behavior.
"""

from __future__ import annotations

from dataclasses import replace
import hashlib
import os
from pathlib import Path
import struct
import subprocess

from capstone import CS_ARCH_X86, CS_GRP_JUMP, CS_MODE_32, Cs
import pytest

from match_recovery.associated_source import AssociatedDataBinding, parse_associated
from match_recovery.associated_writer import write_associated_bytes
from match_recovery.model import Origin, Span
from match_recovery.pe_coff import PeImage
from match_recovery.source.reader import read_text
from match_recovery.source.writer import write_bytes


ROOT = Path(__file__).resolve().parents[2]
BASELINE = "ad705eedea1a28b5060491124b73ee97fdcf806a"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
SPACE = "pe:va"

# unit, owner, kind, data start/end, original instruction which references it,
# and the source line anchor offset from that owner's ORIGINAL marker.
ROWS = (
    ("src/designwin.cpp", 0x439960, "switch_targets", 0x4399FC, 0x439A14, 0x43999B, 0),
    ("src/dialogs.cpp", 0x612970, "switch_targets", 0x612A48, 0x612A60, 0x61298F, 0),
    ("src/dialogs.cpp", 0x612970, "switch_selector", 0x612A60, 0x612A70, 0x612989, 0),
    ("src/dialogs.cpp", 0x612AE0, "switch_targets", 0x612B58, 0x612B70, 0x612AF1, 0),
    ("src/dialogs.cpp", 0x612AE0, "switch_selector", 0x612B70, 0x612B80, 0x612AEB, 0),
    # This mapping is written inside a purpose block; the valid insertion
    # point is the opening /* line immediately before its ORIGINAL marker.
    ("src/dialogs.cpp", 0x613740, "switch_targets", 0x613774, 0x613780, 0x613754, -2),
    ("src/dialogs.cpp", 0x613740, "switch_selector", 0x613780, 0x613790, 0x61374E, -2),
    ("src/win.cpp", 0x5F4C30, "switch_targets", 0x5F4C84, 0x5F4C94, 0x5F4C48, 0),
)

TARGET_TABLES = {
    ("src/designwin.cpp", 0x439960): (0x4399A2, 0x4399B0, 0x4399BE,
                                      0x4399CC, 0x4399DA, 0x4399EA),
    ("src/dialogs.cpp", 0x612970): (0x6129DA, 0x6129B8, 0x612A1E,
                                    0x6129FC, 0x612996, 0x612A40),
    ("src/dialogs.cpp", 0x612AE0): (0x612B28, 0x612B10, 0x612B55,
                                    0x612B40, 0x612AF8, 0x612B55),
    ("src/dialogs.cpp", 0x613740): (0x61375B, 0x613768, 0x61376F),
    ("src/win.cpp", 0x5F4C30): (0x5F4C4F, 0x5F4C5A, 0x5F4C65, 0x5F4C70),
}

SELECTORS = {
    0x612A60: bytes.fromhex("00 01 05 02 05 05 05 03 05 05 05 05 05 05 05 04"),
    0x612B70: bytes.fromhex("00 01 05 02 05 05 05 03 05 05 05 05 05 05 05 04"),
    0x613780: bytes.fromhex("00 00 02 00 02 02 02 01 02 02 02 02 02 02 02 00"),
}

OWNER_BODIES = {
    ("src/designwin.cpp", 0x439960): (0x439960, 0x4399FA),
    ("src/dialogs.cpp", 0x612970): (0x612970, 0x612A48),
    ("src/dialogs.cpp", 0x612AE0): (0x612AE0, 0x612B58),
    ("src/dialogs.cpp", 0x613740): (0x613740, 0x613772),
    ("src/win.cpp", 0x5F4C30): (0x5F4C30, 0x5F4C82),
}

SOURCE_BODY_ENDS = {
    0x439960: 0x4399FA,
    0x612970: 0x612A48,
    0x612AE0: 0x612B58,
    0x613740: 0x613772,
    0x5F4C30: 0x5F4C82,
}


def _git_baseline(path: str) -> bytes:
    return subprocess.run(
        ["git", "show", f"{BASELINE}:{path}"], cwd=ROOT,
        check=True, capture_output=True, timeout=10,
    ).stdout


def _identity(row: AssociatedDataBinding) -> tuple:
    return row.owner, row.span, row.kind, row.evidence, row.unit


def _new_bindings(path: str, raw: bytes) -> tuple[AssociatedDataBinding, ...]:
    text = raw.decode("utf-8", errors="surrogateescape")
    lines = text.splitlines()
    result = []
    for unit, owner, kind, start, end, evidence, anchor_delta in ROWS:
        if unit != path:
            continue
        marker = f"// ORIGINAL: 0x{owner:08X}"
        matches = [i for i, line in enumerate(lines) if line.startswith(marker)]
        assert len(matches) == 1, f"expected one source owner marker for {owner:08X}"
        line = matches[0] + 1 + anchor_delta
        result.append(AssociatedDataBinding(
            Origin(PIN, SPACE, owner), Span(start, end), kind,
            Origin(PIN, SPACE, evidence), unit, line,
        ))
    return tuple(result)


def _corrected_dialogs_baseline(raw: bytes) -> bytes:
    """Apply the one authorized extent correction through the native writer."""
    path = "src/dialogs.cpp"
    text = raw.decode("utf-8", errors="surrogateescape")
    records = read_text(text, path)
    owners = [row for row in records if row.address == 0x612830]
    assert len(owners) == 1
    old = owners[0]
    assert old.image_spans == ((0x612830, 0x612AE0), (0x663060, 0x6630D3))
    corrected = replace(old, image_spans=((0x612830, 0x61295F), (0x663060, 0x6630D3)))
    records[records.index(old)] = corrected
    result = write_bytes(raw, records)

    # These two prose edits are coordinator-reviewed evidence accompanying
    # that extent correction. Require the exact old text and change nothing
    # else; in particular the native writer, not this fixture, owns the marker.
    size_before = (
        b"// size      688 bytes\n"
    )
    size_after = (
        b"// size      303 bytes (primary constructor); cold EH region 115 bytes\n"
    )
    history_before = (
        b"The image's span 0x00612830..0x00612AE0 covers this ctor (through its\n"
        b"`ret 4` at 0x0061295C) PLUS the jump-dispatch function at 0x00612970 and its\n"
        b"case stubs, which our binary lays out elsewhere; this ctor's own body\n"
        b"converges through its full length.\n"
    )
    history_after = (
        b"Historical catalog span 0x00612830..0x00612AE0 (688 bytes) incorrectly\n"
        b"included independent close, attach and item entries and attach's switch data.\n"
        b"The constructor's contiguous primary body ends after `ret 4` at 0x0061295C,\n"
        b"exclusive 0x0061295F; its sole conditional edge rejoins at 0x006128BA.\n"
        b"The following NOP, close at 0x00612960 and attach at 0x00612970 are not\n"
        b"constructor instructions. The separately mapped cold EH region is unchanged;\n"
        b"its complete compiler correspondence remains unqualified.\n"
    )
    assert result.count(size_before) == 1
    assert result.count(history_before) == 1
    return result.replace(size_before, size_after, 1).replace(history_before, history_after, 1)


def _expected_source(path: str) -> tuple[bytes, bytes, tuple[AssociatedDataBinding, ...]]:
    original = _git_baseline(path)
    corrected = _corrected_dialogs_baseline(original) if path == "src/dialogs.cpp" else original
    additions = _new_bindings(path, corrected)
    existing = parse_associated(corrected.decode("utf-8", errors="surrogateescape"),
                                path, PIN, address_space=SPACE)
    populated = write_associated_bytes(
        corrected, expected=existing, additions=additions, path=path,
        pin=PIN, address_space=SPACE,
    )
    return corrected, populated, additions


def test_associated_source_population_is_exact_add_only_transport() -> None:
    """Round-trip eight additions against the corrected immutable baseline."""
    for path in ("src/designwin.cpp", "src/dialogs.cpp", "src/win.cpp"):
        corrected, populated, additions = _expected_source(path)
        corrected_rows = parse_associated(
            corrected.decode("utf-8", errors="surrogateescape"), path, PIN,
            address_space=SPACE,
        )
        populated_rows = parse_associated(
            populated.decode("utf-8", errors="surrogateescape"), path, PIN,
            address_space=SPACE,
        )
        assert {_identity(row) for row in populated_rows} == (
            {_identity(row) for row in corrected_rows}
            | {_identity(row) for row in additions}
        )

        # Inverse exactly the added canonical lines; the remainder is exactly
        # the native-writer corrected baseline plus two named prose edits.
        stripped = populated
        for row in additions:
            marker = (
                f"// RECOVERY_ASSOCIATED: v1 {row.kind} "
                f"0x{row.owner.address:08X} 0x{row.span.start:08X} "
                f"0x{row.span.end:08X} 0x{row.evidence.address:08X}\n"
            ).encode("ascii")
            assert stripped.count(marker) == 1
            stripped = stripped.replace(marker, b"", 1)
        assert stripped == corrected

    assert sum(len(_expected_source(path)[2]) for path in
               ("src/designwin.cpp", "src/dialogs.cpp", "src/win.cpp")) == 8


def test_live_bindings_preserve_all_associated_rows_and_corrected_spans() -> None:
    for path in ("src/designwin.cpp", "src/dialogs.cpp", "src/win.cpp"):
        text = (ROOT / path).read_bytes().decode(
            "utf-8", errors="surrogateescape"
        )
        live_rows = parse_associated(text, path, PIN, address_space=SPACE)
        corrected, _populated, additions = _expected_source(path)
        prior_rows = parse_associated(
            corrected.decode("utf-8", errors="surrogateescape"), path, PIN,
            address_space=SPACE,
        )
        live_ids = {_identity(row) for row in live_rows}
        assert {_identity(row) for row in prior_rows} <= live_ids
        selected_owners = {row.owner for row in additions}
        assert {_identity(row) for row in live_rows if row.owner in selected_owners} == {
            _identity(row) for row in additions
        }

        mapped = {row.address: row for row in read_text(text, path)}
        for address, expected_end in SOURCE_BODY_ENDS.items():
            if any(unit == path and owner == address
                   for unit, owner, *_ in ROWS):
                assert mapped[address].image_spans[0] == (address, expected_end)
        if path == "src/dialogs.cpp":
            assert mapped[0x612830].image_spans == (
                (0x612830, 0x61295F), (0x663060, 0x6630D3)
            )


def _instructions(image: PeImage, start: int, end: int):
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    engine.detail = True
    raw = image.read(Span(start, end))
    instructions = list(engine.disasm(raw, start))
    cursor = start
    by_address = {}
    for instruction in instructions:
        assert instruction.address == cursor, f"undecoded byte at {cursor:08X}"
        cursor += instruction.size
        by_address[instruction.address] = instruction
    assert cursor == end and instructions
    return by_address


def _expect_instruction(instructions, address: int, mnemonic: str, operands: str) -> None:
    instruction = instructions[address]
    assert (instruction.mnemonic, instruction.op_str) == (mnemonic, operands), (
        f"{address:08X}: got {instruction.mnemonic} {instruction.op_str}"
    )


def test_pinned_switch_domains_cells_selectors_and_highlow_sites() -> None:
    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", str(ROOT.parents[2] / ".opensmacx/game/terranx_original.exe")
    ))
    if not image_path.is_file():
        pytest.skip("pinned proprietary image unavailable; source transport remains covered")
    raw_image = image_path.read_bytes()
    assert hashlib.sha256(raw_image).hexdigest() == PIN
    image = PeImage(raw_image, PIN)

    # Independently establish the corrected constructor's complete contiguous
    # primary body. CALLs are not branches in this local CFG proof.
    ctor_start, ctor_end = 0x612830, 0x61295F
    ctor = _instructions(image, ctor_start, ctor_end)
    ctor_starts = set(ctor)
    branches = [instruction for instruction in ctor.values()
                if instruction.group(CS_GRP_JUMP)]
    assert [(row.address, row.mnemonic, row.op_str) for row in branches] == [
        (0x61285D, "je", "0x6128ba")
    ]
    assert 0x6128BA in ctor_starts
    assert (ctor[0x61295C].mnemonic, ctor[0x61295C].op_str,
            bytes(ctor[0x61295C].bytes)) == ("ret", "4", b"\xC2\x04\x00")
    assert image.read(Span(0x61295F, 0x612960)) == b"\x90"
    close = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(
        image.read(Span(0x612960, 0x612961)), 0x612960
    ))
    assert [(item.address, item.mnemonic) for item in close] == [(0x612960, "ret")]
    attach_body = _instructions(image, *OWNER_BODIES[("src/dialogs.cpp", 0x612970)])
    item_body = _instructions(image, 0x612A70, 0x612ABF)
    assert 0x612970 in attach_body and 0x612A70 in item_body
    assert item_body[max(item_body)].mnemonic.startswith("ret")
    assert ctor_end < 0x612960 < 0x612970 < 0x612A70

    expected_owners = {
        ("src/designwin.cpp", 0x439960): {
            "dispatch": (0x43999B, "jmp", "dword ptr [eax*4 + 0x4399fc]"),
            "guard": ((0x439996, "cmp", "eax, 5"),
                      (0x439999, "ja", "0x4399f3")),
            "targets": TARGET_TABLES[("src/designwin.cpp", 0x439960)],
            "target_span": (0x4399FC, 0x439A14),
            "selector_span": None,
            "domain": ("index", 0, 5),
        },
        ("src/dialogs.cpp", 0x612970): {
            "dispatch": (0x61298F, "jmp", "dword ptr [edx*4 + 0x612a48]"),
            "guard": ((0x612974, "and", "eax, 0x1f"),
                      (0x61297D, "dec", "eax"),
                      (0x61297E, "cmp", "eax, 0xf"),
                      (0x612981, "ja", "0x612a40"),
                      (0x612989, "mov", "dl, byte ptr [eax + 0x612a60]")),
            "targets": TARGET_TABLES[("src/dialogs.cpp", 0x612970)],
            "target_span": (0x612A48, 0x612A60),
            "selector_span": (0x612A60, 0x612A70),
            "domain": ("masked-kind", 0x1F, 1, 16),
        },
        ("src/dialogs.cpp", 0x612AE0): {
            "dispatch": (0x612AF1, "jmp", "dword ptr [edx*4 + 0x612b58]"),
            "guard": ((0x612AE0, "mov", "eax, dword ptr [ecx - 8]"),
                      (0x612AE3, "dec", "eax"),
                      (0x612AE4, "cmp", "eax, 0xf"),
                      (0x612AE7, "ja", "0x612b55"),
                      (0x612AEB, "mov", "dl, byte ptr [eax + 0x612b70]")),
            "targets": TARGET_TABLES[("src/dialogs.cpp", 0x612AE0)],
            "target_span": (0x612B58, 0x612B70),
            "selector_span": (0x612B70, 0x612B80),
            "domain": ("kind", 1, 16),
        },
        ("src/dialogs.cpp", 0x613740): {
            "dispatch": (0x613754, "jmp", "dword ptr [edx*4 + 0x613774]"),
            "guard": ((0x613740, "mov", "eax, dword ptr [ecx + 0x180]"),
                      (0x613746, "dec", "eax"),
                      (0x613747, "cmp", "eax, 0xf"),
                      (0x61374A, "ja", "0x61376f"),
                      (0x61374E, "mov", "dl, byte ptr [eax + 0x613780]")),
            "targets": TARGET_TABLES[("src/dialogs.cpp", 0x613740)],
            "target_span": (0x613774, 0x613780),
            "selector_span": (0x613780, 0x613790),
            "domain": ("kind", 1, 16),
        },
        ("src/win.cpp", 0x5F4C30): {
            "dispatch": (0x5F4C48, "jmp", "dword ptr [edi*4 + 0x5f4c84]"),
            "guard": ((0x5F4C43, "cmp", "edi, 3"),
                      (0x5F4C46, "ja", "0x5f4c7b")),
            "targets": TARGET_TABLES[("src/win.cpp", 0x5F4C30)],
            "target_span": (0x5F4C84, 0x5F4C94),
            "selector_span": None,
            "domain": ("index", 0, 3),
        },
    }

    for owner_key, proof in expected_owners.items():
        start, end = OWNER_BODIES[owner_key]
        ins = _instructions(image, start, end)
        terminal = ins[max(ins)]
        assert terminal.mnemonic.startswith("ret") or terminal.mnemonic == "jmp"
        for address, mnemonic, operands in proof["guard"]:
            _expect_instruction(ins, address, mnemonic, operands)
        dispatch_address, mnemonic, operands = proof["dispatch"]
        _expect_instruction(ins, dispatch_address, mnemonic, operands)

        table_start, table_end = proof["target_span"]
        assert table_start >= end
        gap = image.read(Span(end, table_start)) if table_start > end else b""
        assert gap in (b"", b"\x8B\xFF")
        assert image.read(Span(table_start, table_end)) == struct.pack(
            f"<{len(proof['targets'])}I", *proof["targets"]
        )
        cells = Span(table_start, table_end)
        assert image.absolute_sites(cells) == frozenset(range(0, table_end - table_start, 4))
        assert all(table_start + offset in image.relocations
                   for offset in range(0, table_end - table_start, 4))
        assert all(start <= target < end and target in ins
                   for target in proof["targets"]), (
            "switch target must be a decoded instruction start in its owner body"
        )

        selector_span = proof["selector_span"]
        if selector_span is not None:
            selector = Span(*selector_span)
            assert selector.start == table_end
            selector_values = image.read(selector)
            assert selector_values == SELECTORS[selector.start]
            assert image.absolute_sites(selector) == frozenset()
            assert max(selector_values) < len(proof["targets"]), (
                "guarded selector emits an index outside its target table"
            )

        # Derive the index set from the decoded unsigned guard semantics and
        # cross-check table capacity. A masked kind is reduced to 0..31 first,
        # then decremented; the other kind paths decrement their stored value.
        domain = proof["domain"]
        if domain[0] == "masked-kind":
            _, mask, first, last = domain
            and_address = proof["guard"][0][0]
            and_instruction = ins[and_address]
            assert and_instruction.operands[1].imm == mask
            values = tuple(range(first, last + 1))
            assert len(values) == len(SELECTORS[proof["selector_span"][0]])
        elif domain[0] == "kind":
            _, first, last = domain
            values = tuple(range(first, last + 1))
            assert len(values) == len(SELECTORS[proof["selector_span"][0]])
        else:
            _, first, last = domain
            values = tuple(range(first, last + 1))
            assert len(values) == len(proof["targets"])
        assert values

    # The complete gated domains are tied to their body-local unsigned checks:
    # 0..5 palette slots, masked kind residues 1..16 for attach, and kinds
    # 1..16 for the two Dialogs queries; redraw indices are 0..3.
    assert len(TARGET_TABLES[("src/designwin.cpp", 0x439960)]) == 6
    assert len(SELECTORS[0x612A60]) == 16
    assert len(TARGET_TABLES[("src/win.cpp", 0x5F4C30)]) == 4
