"""Pinned notification inputs, reload boundaries and fixed source migration."""
from pathlib import Path
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
FIXTURE = Path(__file__).with_name("netdaemon-project-notification-oct11.patch")


def apply_project_notification_migration(source, reverse=False):
    edits = []
    for hunk in FIXTURE.read_text().split("\n@@ ")[1:]:
        lines = hunk.splitlines(keepends=True)[1:]
        before = "".join(line[1:] for line in lines if line.startswith((" ", "-")))
        after = "".join(line[1:] for line in lines if line.startswith((" ", "+")))
        edits.append((before, after))
    assert edits
    for before, after in reversed(edits) if reverse else edits:
        if reverse:
            before, after = after, before
        assert before and source.count(before) == 1
        source = source.replace(before, after, 1)
    return source


def test_only_reviewed_project_notification_changes():
    from dataclasses import fields
    from match_recovery.source import read_text
    path = ROOT / "src/netdaemon.cpp"
    before = subprocess.check_output(["git", "show", "dde4a04c4:src/netdaemon.cpp"], cwd=ROOT, text=True)
    current = path.read_text()
    assert apply_project_notification_migration(before) == current
    assert apply_project_notification_migration(current, reverse=True) == before
    old = {r.address: r for r in read_text(before, path)}
    new = {r.address: r for r in read_text(current, path)}
    assert old.keys() == new.keys()
    for address, record in new.items():
        for field in fields(record):
            if field.name not in {"line", "region", "origin"}:
                assert getattr(record, field.name) == getattr(old[address], field.name)


def test_pinned_native_notification_reads_and_parser_tokens():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(), "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    instructions = {i.address: (i.mnemonic, i.op_str) for i in
                    Cs(CS_ARCH_X86, CS_MODE_32).disasm(image.read(Span(0x537775, 0x537971)), 0x537775)}
    expected = {
        0x53778F: ("mov", "ecx, dword ptr [ebx + 0x14]"),
        0x5377A8: ("call", "0x6169a0"),
        0x5377B3: ("call", "0x626250"),
        0x5377C8: ("mov", "eax, dword ptr [ebx + 0x14]"),
        0x5377CD: ("jl", "0x538266"),
        0x537852: ("push", "-1"),
        0x537856: ("mov", "edx, dword ptr [ecx]"),
        0x53785B: ("call", "0x625e50"),
        0x537905: ("mov", "ecx, dword ptr [eax]"),
        0x53790A: ("call", "0x625e50"),
        0x53790F: ("mov", "eax, dword ptr [ebx + 0x14]"),
        0x537917: ("je", "0x53795e"),
    }
    for address, instruction in expected.items():
        assert instructions[address] == instruction
