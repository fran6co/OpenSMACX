"""Separate an unclaimed writer body from an independently catalogued leaf."""
from dataclasses import replace
from pathlib import Path
import subprocess

import pytest

from match_recovery.binding_source import parse
from match_recovery.source import read_text
from match_recovery.source.writer import write_bytes

ROOT = Path(__file__).resolve().parents[2]
BASE = "4655012ec8488bef8253f15308dafbcc602548eb"
PATH = "src/buffer.cpp"


def test_native_writer_only_corrects_unclaimed_extent():
    before = subprocess.check_output(["git", "show", BASE + ":" + PATH], cwd=ROOT, text=True)
    after = (ROOT / PATH).read_text()
    prior = read_text(before, ROOT / PATH)
    current = read_text(after, ROOT / PATH)
    old = next(r for r in prior if r.address == 0x5DF910)
    new = next(r for r in current if r.address == 0x5DF910)
    assert old.image_spans == ((0x5DF910, 0x5DFB48),)
    assert new.image_spans == ((0x5DF910, 0x5DFB3F),)
    assert not old.byte_exact and not old.semantic
    assert new.byte_exact == old.byte_exact and new.semantic == old.semantic
    assert (new.name, new.symbol, new.function_fragment) == (old.name, old.symbol, old.function_fragment)
    binding = after.splitlines()[new.line]
    assert binding.startswith("// RECOVERY_BINDING: v1 ")
    evidence = parse(after, current)[new.line]
    assert "still-unowned" in evidence["reason"]
    assert "5DF910..5DFB48" in evidence["evidence"][1]
    assert "No byte claim" in evidence["evidence"][1]
    emitted = write_bytes(before.encode(), [replace(old, image_spans=new.image_spans)]).decode()
    marker = emitted.splitlines()[old.line - 1]
    emitted = emitted.replace(marker + "\n", marker + "\n" + binding + "\n", 1)
    emitted = emitted.replace("// size      568 bytes\n", "// size      559 bytes\n", 1)
    assert emitted == after
    normalize = lambda r: replace(r, line=1, region="", image_spans=old.image_spans if r.address == old.address else r.image_spans)
    assert [normalize(r) for r in prior] == [normalize(r) for r in current]
    assert not any(r.address == 0x5DFB40 for r in current)


def test_pinned_terminal_padding_and_separate_constant_return_body():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("pinned executable required")
    image = PeImage(path.read_bytes(), "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    actual = [(i.address, i.mnemonic, i.op_str, i.size) for i in decoder.disasm(image.read(Span(0x5DFB3C, 0x5DFB48)), 0x5DFB3C)]
    assert actual == [
        (0x5DFB3C, "ret", "4", 3),
        (0x5DFB3F, "nop", "", 1),
        (0x5DFB40, "mov", "eax, 8", 5),
        (0x5DFB45, "ret", "0x10", 3),
    ]


def test_independent_catalogue_separates_entries():
    import csv
    path = ROOT / ".opensmacx/game/ida94-functions.csv"
    if not path.is_file():
        pytest.skip("local independent catalogue required")
    with path.open() as stream:
        rows = {int(r["address"], 16): r for r in csv.DictReader(stream)}
    assert int(rows[0x5DFB40]["end"], 16) == 0x5DFB48
    assert int(rows[0x5DF910]["end"], 16) <= 0x5DFB40
