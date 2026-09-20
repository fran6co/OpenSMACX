"""Two BasePop reset copies, not the remainder of close or FX initialization."""

import os
from pathlib import Path
import shutil

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_OP_REG

from match_recovery.codeview import class_layout
from match_recovery.compiler import Compiler
from match_recovery.measurement import Measurement
from match_recovery.model import Span
from match_recovery.object_data import check_initial_data
from match_recovery.object_source import read_objects
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.source_inventory import image_pin
from test_application_link_recovery import ROOT, run_recovered_harness


UNIT = "src/basepop.cpp"
FIRST = "?BasePopDefaultBlock2E64@@3PAIA"
SECOND = "?BasePopDefaultBlock2F7C@@3PAIA"
SYMBOL = "?close@BasePop@@QAEXXZ"


@pytest.fixture(scope="module")
def family():
    from recovery.environment.wine import VC6_ROOT
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None or not path.is_file():
        pytest.skip("VC6, Wine32 and pinned image are required")
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    objects = [Coff(compiler(ROOT / UNIT, (), flags + settings.layout_flags))
               for flags in settings.flag_sets]
    return compiler, settings, objects, PeImage(path.read_bytes(), image_pin(ROOT))


def copy_prefix(code, start, relocations=None):
    """Trace the short straight-line copy prefix, refusing unhandled writers."""
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    values = {"ecx": ("this", 0)}
    transfers = []
    for op in decoder.disasm(code, start):
        if op.mnemonic == "rep movsd":
            assert values.get("ecx") == 70
            transfers.append((values.get("esi"), values.get("edi"), 280))
            for register in ("esi", "edi", "ecx"):
                values.pop(register, None)
            if len(transfers) == 2:
                return transfers
            continue
        assert op.mnemonic not in ("call", "jmp", "ret") and not op.mnemonic.startswith("j")
        if op.mnemonic == "push":
            continue
        handled = None
        if op.mnemonic in ("mov", "lea") and op.operands[0].type == X86_OP_REG:
            handled = op.reg_name(op.operands[0].reg)
            source = op.operands[1]
            value = None
            if source.type == X86_OP_IMM:
                value = source.imm
                if relocations:
                    matches = [target for offset, target in relocations.items()
                               if op.address <= offset < op.address + op.size]
                    if matches:
                        assert source.imm == 0, "nonzero defaults addend"
                        value, = matches
            elif source.type == X86_OP_REG:
                value = values.get(op.reg_name(source.reg))
            elif source.type == X86_OP_MEM and op.mnemonic == "lea":
                base = values.get(op.reg_name(source.mem.base))
                if isinstance(base, tuple) and not source.mem.index:
                    value = (base[0], base[1] + source.mem.disp)
            values[handled] = value
        for register in op.regs_access()[1]:
            name = op.reg_name(register)
            if name != handled:
                values.pop(name, None)
    raise AssertionError("did not observe both complete block copies")


def test_original_and_full_compiled_function_copy_complete_distinct_blocks(family):
    _, _, objects, image = family
    assert copy_prefix(image.read(Span(0x600F00, 0x600F41)), 0x600F00) == [
        (0x9B8AB8, ("this", 0x2E64), 280), (0x9B8BD0, ("this", 0x2F7C), 280)]
    for obj in objects:
        layout = class_layout(obj, "BasePop")
        assert layout["members"]["text_block_a_"] == 0x2E64
        assert layout["members"]["text_block_b_"] == 0x2F7C
        assert class_layout(obj, "BasePopTextBlock")["size"] == 280
        code, relocations, _ = obj.subject(obj.function(SYMBOL))
        targets = {row.offset: obj.symbols[int(row.target)].name for row in relocations}
        assert copy_prefix(code, 0, targets) == [
            (FIRST, ("this", 0x2E64), 280), (SECOND, ("this", 0x2F7C), 280)]


def test_source_owned_default_extents_reject_undersized_and_oversized_storage(family):
    compiler, settings, objects, image = family
    bindings = [row for row in read_objects(ROOT, image_pin(ROOT)) if row.symbol in (FIRST, SECOND)]
    assert {row.symbol: row.span for row in bindings} == {
        FIRST: Span(0x9B8AB8, 0x9B8BD0), SECOND: Span(0x9B8BD0, 0x9B8CE8)}
    assert all(row.unit == UNIT and row.evidence.address == 0x600F00 for row in bindings)
    path = ROOT / UNIT
    source = path.read_bytes()
    for flags, obj in zip(settings.flag_sets, objects):
        rows = check_initial_data(bindings, {UNIT: obj}, image, {})
        assert all(row["content"] == row["proof"]["boundary"] == "pass" for row in rows), rows
        assert all(row["compiler_storage"]["size"] == 280 for row in rows)
        for block, symbol in (("2E64", FIRST), ("2F7C", SECOND)):
            declaration = f"uint32_t BasePopDefaultBlock{block}[70];".encode()
            assert source.count(declaration) == 1
            for count in (69, 71):
                damaged = Coff(compiler.overlay(path, {UNIT: source.replace(
                    declaration, declaration.replace(b"[70]", f"[{count}]".encode()))},
                    (), flags + settings.layout_flags))
                row, = (row for row in check_initial_data(bindings, {UNIT: damaged}, image, {})
                         if row["symbol"] == symbol)
                assert row["content"] == row["proof"]["boundary"] == "fail"
    assert path.read_bytes() == source


@pytest.mark.parametrize("block,source_name", [("a", "2E64"), ("b", "2F7C")])
def test_compiled_prefix_rejects_a_shifted_default_symbol(family, block, source_name):
    compiler, settings, _, _ = family
    path = ROOT / UNIT
    source = path.read_bytes()
    good = f"memcpy(&text_block_{block}_, BasePopDefaultBlock{source_name}, sizeof(text_block_{block}_));".encode()
    bad = (f"memcpy(&text_block_{block}_, BasePopDefaultBlock{source_name} + 1, "
           f"sizeof(text_block_{block}_) - sizeof(uint32_t));").encode()
    assert source.count(good) == 1
    for flags in settings.flag_sets:
        obj = Coff(compiler.overlay(path, {UNIT: source.replace(good, bad)},
                                    (), flags + settings.layout_flags))
        code, relocations, _ = obj.subject(obj.function(SYMBOL))
        targets = {row.offset: obj.symbols[int(row.target)].name for row in relocations}
        with pytest.raises(AssertionError, match="nonzero defaults addend"):
            copy_prefix(code, 0, targets)
    assert path.read_bytes() == source


def test_live_prefix_copies_every_word_and_preserves_neighboring_storage(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "basepop_defaults_harness.cpp", [], declarations=[
        ("// LIVE_TEXT_BLOCK", "src/basepop.h", "struct BasePopTextBlock {",
         "static_assert(sizeof(BasePopTextBlock)"),
        ("// LIVE_DEFAULT_BLOCKS", UNIT, "uint32_t BasePopDefaultBlock2E64[70];",
         "// Copied to +0x3094, the head"),
        ("// LIVE_COPY_PREFIX", UNIT, "void BasePop::close() {",
         "    if (*(void **)(self + 0x3224) != 0) {")])


@pytest.mark.parametrize("block,source", [("a", "2E64"), ("b", "2F7C")])
@pytest.mark.parametrize("damage", ["old_index", "short_copy", "wrong_source"])
def test_live_prefix_rejects_index_extent_and_identity_damage(tmp_path, monkeypatch, block, source, damage):
    original = Path.read_text
    good = f"memcpy(&text_block_{block}_, BasePopDefaultBlock{source}, sizeof(text_block_{block}_));"
    if damage == "old_index":
        bad = (f"for (i = 0; i < 70; ++i) reinterpret_cast<uint32_t *>(&text_block_{block}_)[i]"
               f" = BasePopDefaultBlock{source}[i / 4];")
    elif damage == "short_copy":
        bad = good.replace(f"sizeof(text_block_{block}_)", "sizeof(uint32_t)")
    else:
        bad = good.replace(source, "2F7C" if source == "2E64" else "2E64")

    def damaged(path, *args, **kwargs):
        text = original(path, *args, **kwargs)
        if path == ROOT / UNIT:
            assert text.count(good) == 1
            return text.replace(good, bad)
        return text

    monkeypatch.setattr(Path, "read_text", damaged)
    with pytest.raises(AssertionError, match="Assertion failed"):
        test_live_prefix_copies_every_word_and_preserves_neighboring_storage(tmp_path, monkeypatch)
