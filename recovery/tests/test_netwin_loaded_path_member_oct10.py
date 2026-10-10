"""Typed loaded-path member closure; not whole-game or linked-preservation proof."""
from dataclasses import fields
from pathlib import Path
import subprocess

import pytest
from match_recovery.source import read_text, write

ROOT = Path(__file__).resolve().parents[2]
BASE = "c8680c9d0bc53276abb6893fc02623a85ab9323e"
DECLARATION = """  // load_map/load_game append the extensionless map/save/scenario path here
  // (0x5AB8AE/0x5AAD9D); pick_map takes its basename (0x47D7D7).
  // The next word bounds this occupied span, not a checked input length.
  // RECOVERY_MEMBER: v1 NetWin loaded_file_path_ 0x7624 0x0047D300
  char loaded_file_path_[0x104];  // 0x7624..0x7727"""


def baseline(relative):
    return subprocess.check_output(
        ["git", "show", BASE + ":" + relative], cwd=ROOT
    ).decode()


def restore_loaded_path_member_migration(source):
    """Undo just the verified game caller spelling for earlier token controls."""
    return source.replace(
        "strcat(NetWinDialog.loaded_file_path_,",
        "strcat(reinterpret_cast<char *>(NetWinDialog.field_7624_),",
    ).replace("NetWinDialog.loaded_file_path_[0]", "NetWinDialog.field_7624_[0]")


@pytest.mark.parametrize("relative", ("src/game.cpp", "src/netwin.cpp"))
def test_only_member_name_and_redundant_casts_change(relative):
    old = baseline(relative)
    expected = old.replace("field_7624_", "loaded_file_path_")
    for expression in ("NetWinDialog.loaded_file_path_", "loaded_file_path_"):
        expected = expected.replace("reinterpret_cast<char *>(" + expression + ")", expression)
    path = ROOT / relative
    now = path.read_text()
    assert now == expected
    if relative == "src/game.cpp":
        assert restore_loaded_path_member_migration(now) == old
    current = read_text(now, path)
    previous = {row.address: row for row in read_text(old, path)}
    assert {row.address for row in current} == set(previous)
    assert write(now, current) == now
    for row in current:
        original = previous[row.address]
        for field in fields(row):
            if field.name not in {"line", "region", "origin"}:
                assert getattr(row, field.name) == getattr(original, field.name)
        def lessons(record):
            return [(x.kind, x.key, x.prose, tuple(text for _, text in x.lines))
                    for x in record.origin]
        assert lessons(row) == lessons(original)


def test_header_only_changes_string_member_and_offset_evidence():
    old = baseline("src/netwin.h")
    before = "  uint8_t field_7624_[0x104];  // 0x7624"
    assert old.count(before) == 1
    assert (ROOT / "src/netwin.h").read_text() == old.replace(before, DECLARATION, 1)
    assert 0x7728 - 0x7624 == 0x104
    assert 0x80A6F8 + 0x7624 == 0x811D1C


def test_pinned_string_receiver_writes_and_reads():
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    from match_recovery.source_inventory import image_pin

    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.exists():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    encodings = (
        (0x5AAD9D, "881d1c1d8100"),
        (0x5AADA4, "681c1d8100"),
        (0x5AADA9, "e8c2a60900"),
        (0x5AB8AE, "c6051c1d810000"),
        (0x5AB8B6, "681c1d8100"),
        (0x5AB8BB, "e8b09b0900"),
        (0x47D7D7, "8db324760000"),
        (0x47D7E0, "6a5c"),
        (0x47D7FB, "e8a0801c00"),
    )
    for address, encoding in encodings:
        expected = bytes.fromhex(encoding)
        assert image.read(Span(address, address + len(expected))) == expected

