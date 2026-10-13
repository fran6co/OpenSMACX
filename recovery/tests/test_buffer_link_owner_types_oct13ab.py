"""Producer-backed Buffer pointer types and strict task-base continuity.

These checks connect the recovered stores/readers to typed source members and
the pinned original. Host fixtures elsewhere in this task exercise the actual
link writer, click consumer, allocator callbacks, and reinitialize body; none
of them claims native ABI or game-runtime acceptance.
"""
from dataclasses import fields
import hashlib
import os
from pathlib import Path
import re
import subprocess

from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import read_text
from match_recovery.tasks import TaskStore


ROOT = Path(__file__).resolve().parents[2]
TASK = "buffer-link-owner-types-oct13ab"
BASE = "aa2c993d24d5c78af70ace427f396a4fe68291f3"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
OWNED_SPAN = (0x005DCCA8, 0x005DCD01)


def protected_metadata(row):
    return tuple((item.name, getattr(row, item.name)) for item in fields(row)
                 if item.name not in {"line", "region", "origin"})


def protected_history(row):
    return tuple((lesson.kind, lesson.key, lesson.prose,
                  tuple(line for _, line in lesson.lines)) for lesson in row.origin)


def cxx_tokens(source):
    """Drop comments/spacing while retaining literal tokens exactly."""
    pieces = re.findall(
        r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|'
        r'[A-Za-z_]\w*|(?:0[xX][0-9A-Fa-f]+|\d+)|[^\s]', source)
    return tuple(piece for piece in pieces
                 if not piece.startswith("//") and not piece.startswith("/*"))


def authorized_type_migration(relative, old_source):
    expected = re.sub(r"\bpoOwner_\b", "graphic_owner_", old_source)
    expected = re.sub(r"\bowned_\b", "link_labels_", expected)
    if relative == "src/buffer.h":
        assert expected.count("void *graphic_owner_") == 1
        expected = expected.replace("void *graphic_owner_", "GraphicWin *graphic_owner_", 1)
        assert expected.count("void *link_labels_[20]") == 1
        expected = expected.replace("void *link_labels_[20]", "char *link_labels_[20]", 1)
    if relative == "src/buffer.cpp":
        old_cast = "static_cast<char *>(link_labels_[link_count_ - 1])[label]"
        assert expected.count(old_cast) == 1
        expected = expected.replace(old_cast, "link_labels_[link_count_ - 1][label]", 1)
    if relative == "src/win.cpp":
        old_cast = "static_cast<char *>(static_cast<GraphicWin *>(this)->Buffer::link_labels_[idx])"
        assert expected.count(old_cast) == 1
        expected = expected.replace(old_cast,
            "static_cast<GraphicWin *>(this)->Buffer::link_labels_[idx]", 1)
    return expected


def function(source, signature):
    start = source.index(signature)
    opening = source.index("{", start + len(signature))
    depth = 0
    for index in range(opening, len(source)):
        depth += (source[index] == "{") - (source[index] == "}")
        if depth == 0:
            return source[start:index + 1]
    raise AssertionError(f"unclosed function: {signature}")


def test_full_immutable_task_base_annotation_identity_and_history():
    task = TaskStore(ROOT).read(TASK)
    assert task.base_revision == BASE
    for relative in ("src/buffer.h", "src/buffer.cpp", "src/graphicwin.cpp", "src/win.cpp"):
        path = ROOT / relative
        previous = subprocess.check_output(
            ["git", "show", f"{BASE}:{relative}"], cwd=ROOT, text=True, timeout=30)
        before = {row.address: row for row in read_text(previous, path)}
        current = {row.address: row for row in read_text(path.read_text(), path)}
        assert set(current) == set(before), relative
        for address, old in before.items():
            new = current[address]
            assert protected_metadata(new) == protected_metadata(old), (relative, hex(address))
            assert protected_history(new) == protected_history(old), (relative, hex(address))
        assert cxx_tokens(path.read_text()) == cxx_tokens(authorized_type_migration(relative, previous)), relative


def test_owner_and_link_slots_are_producer_backed_typed_members():
    bh = (ROOT / "src/buffer.h").read_text()
    assert re.search(r"\bGraphicWin\s*\*\s*graphic_owner_\s*;", bh)
    assert re.search(r"\bchar\s*\*\s*link_labels_\s*\[\s*20\s*\]", bh)
    assert not re.search(r"\b(?:void\s*\*\s*poOwner_|void\s*\*\s*owned_\s*\[)", bh)
    assert "class GraphicWin;" in bh or "class GraphicWin" in bh

    graphic = (ROOT / "src/graphicwin.cpp").read_text()
    init = function(graphic, "int GraphicWin::init(int x, int y, int width, int height,")
    reinit = function(graphic, "int GraphicWin::reinitialize(")
    for method in (init, reinit):
        assert "graphic_owner_ = this;" in method
        assert method.index("graphic_owner_ = this;") < method.index("compute_min_size();")
    assert "poOwner_ =" not in init + reinit

    buffer = (ROOT / "src/buffer.cpp").read_text()
    constructor = function(buffer, "Buffer::Buffer()")
    writer = function(buffer, "int Buffer::write_multi_font_raw_l(")
    assert "graphic_owner_ = 0;" in constructor
    assert "link_labels_[slot] = nullptr;" in constructor
    assert writer.count("link_labels_[link_count_ - 1]") == 4
    assert "char *const kept = static_cast<char *>(mem_get(label + 1));" in writer
    assert "static_cast<char *>(link_labels_" not in writer

    win = function((ROOT / "src/win.cpp").read_text(),
                   "void Win::on_l_button_down(long double_click, int x, int y,")
    assert "Buffer::link_labels_[idx]" in win
    assert "Buffer::owned_[idx]" not in win


def test_pinned_original_link_slot_extent_and_consumer_reference():
    explicit = os.environ.get("OPENSMACX_IMAGE")
    image_path = Path(explicit) if explicit else ROOT.parents[2] / ".opensmacx/game/terranx_original.exe"
    raw = image_path.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    start, end = OWNED_SPAN
    instructions = list(decoder.disasm(image.read(Span(start, end)), start))
    assert instructions and instructions[0].address == start
    assert sum(item.size for item in instructions) == end - start
    assert instructions[-1].address + instructions[-1].size == end
    by_address = {item.address: (item.mnemonic, item.op_str) for item in instructions}
    # One-based slot publication, a reload shared by the null check and
    # memcpy destination, then another reload for the terminator. The host
    # getter probes exercise source evaluations, not extra native callbacks.
    assert by_address[0x005DCCCA] == ("mov", "dword ptr [esi + ecx*4 + 0x4b8], eax")
    assert by_address[0x005DCCD7] == ("mov", "eax, dword ptr [esi + edx*4 + 0x4b8]")
    assert by_address[0x005DCCF6] == ("mov", "eax, dword ptr [esi + edx*4 + 0x4b8]")
    assert by_address[0x005DCCFD] == ("mov", "byte ptr [eax + ebx], 0")
    click = list(decoder.disasm(image.read(Span(0x005F6523, 0x005F6540)), 0x005F6523))
    assert click and click[0].address == 0x005F6523
    assert any("[esi + eax*4 + 0x900]" in item.op_str for item in click)
    for lo, hi, address in ((0x005D5010, 0x005D5018, 0x005D5012),
                            (0x005D51BD, 0x005D51C5, 0x005D51BF)):
        producer = list(decoder.disasm(image.read(Span(lo, hi)), lo))
        assert sum(item.size for item in producer) == hi - lo
        stores = [item for item in producer if item.address == address]
        assert len(stores) == 1
        assert (stores[0].mnemonic, stores[0].op_str) == (
            "mov", "dword ptr [esi + 0x448], esi")
