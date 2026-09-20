"""Wave's derived virtual sequence and callers, not its unfinished base slots."""

import os
from pathlib import Path
import shutil

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86 import X86_OP_IMM, X86_OP_MEM

from match_recovery.codeview import class_layout
from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.compiler import Compiler
from match_recovery.generated_binary import check_generated
from match_recovery.generated_source import read_generated
from match_recovery.measurement import Measurement
from match_recovery.model import Origin, Span
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.source import read_file
from match_recovery.source_inventory import image_pin
from test_application_link_recovery import ROOT, references


@pytest.fixture(scope="module")
def family():
    from recovery.environment.wine import VC6_ROOT
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None or not path.is_file():
        pytest.skip("VC6, Wine32 and pinned original image are required")
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    objects = [{unit: Coff(compiler(ROOT / unit, (), flags + settings.layout_flags))
                for unit in ("src/wave.cpp", "src/fx.cpp")} for flags in settings.flag_sets]
    return compiler, settings, objects, PeImage(path.read_bytes(), image_pin(ROOT))


def symbol(address):
    row, = (r for r in read_file(ROOT / "src/wave.cpp") if r.address == address)
    return row.symbol or row.name


def table_observations(objects, image):
    bindings = [r for r in read_generated(ROOT, image_pin(ROOT))
                if r.symbol in ("??_7Wave@@6B@", "??_7Effect@@6B@")]
    assert {r.origin.address for r in bindings} == {0x66E44C, 0x66B7A4}
    targets = {r.symbol or r.name: r.address for r in read_file(ROOT / "src/wave.cpp")}
    _, rows = check_generated(bindings, objects, image, targets)
    assert {r["symbol"] for r in rows} == {"??_7Wave@@6B@", "??_7Effect@@6B@"}
    return rows


def observe_tail(objects, image):
    rows = table_observations(objects, image)
    for row in rows:
        # These are source-identity correlations, not verified callee bodies,
        # full original table bounds or linked selection. Unknown stays unknown.
        assert row["content"] == "unknown"
        assert row["diagnostic"]["emitted_size"] == 57 * 4
        entries = row["diagnostic"]["entries"]
        tail = entries[33:57]
        assert len(tail) == 24
        assert all(e["original_absolute_relocation"] for e in tail)
        assert all(e["observation"] == "target_word_agrees" for e in tail), tail
    return rows


def indirect_offsets(code):
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    return [op.operands[0].mem.disp for op in decoder.disasm(code, 0)
            if op.mnemonic == "call" and op.operands[0].type == X86_OP_MEM
            and op.operands[0].mem.base and not op.operands[0].mem.index]


SELF_DISPATCH_BODIES = (0x4C6EA0, 0x4C6920, 0x4C6CE0, 0x4C6DF0,
                        0x4C6BB0, 0x4C6C20, 0x4C69B0)


def assert_original_dispatch_order(obj, image, address):
    # Slot sequence is a bounded check, not receiver/dataflow or full-body proof.
    row, = (r for r in read_file(ROOT / "src/wave.cpp") if r.address == address)
    start, end = row.image_spans[0]
    original_code = image.read(Span(start, end))
    original = indirect_offsets(original_code)
    code, _, _ = obj.subject(obj.function(row.symbol or row.name))
    assert indirect_offsets(code) == original
    if address in (0x4C6CE0, 0x4C6C20):
        # The shared base loader is direct, not virtual recursion. Operand
        # identity is observed separately from relocation-masked matching.
        decoder = Cs(CS_ARCH_X86, CS_MODE_32)
        decoder.detail = True
        targets = [op.operands[0].imm for op in decoder.disasm(original_code, start)
                   if op.mnemonic == "call" and op.operands[0].type == X86_OP_IMM]
        assert targets == [0x4C6280]
        assert references(obj, row.symbol or row.name).count("?load@Sound@@UAEHPBD@Z") == 1


@pytest.mark.parametrize("address", SELF_DISPATCH_BODIES)
def test_typed_self_calls_preserve_original_dispatch_order(family, address):
    _, _, trials, image = family
    for objects in trials:
        assert_original_dispatch_order(objects["src/wave.cpp"], image, address)


@pytest.mark.parametrize("address,before,after", [
    (0x4C6EA0, b"        detach();", b"        Sound::detach();"),
    (0x4C6DF0, b"                attach();", b"                Sound::attach();"),
    (0x4C6BB0, b"const int attribs = get_attrib();", b"const int attribs = is_looping();"),
    (0x4C6C20, b"const int loaded = Sound::load(fname);", b"const int loaded = load(fname);"),
])
def test_self_dispatch_controls_reject_direct_base_wrong_slot_and_recursion(family, address, before, after):
    compiler, settings, _, image = family
    path = ROOT / "src/wave.cpp"
    source = path.read_bytes()
    assert before in source
    for flags in settings.flag_sets:
        obj = Coff(compiler.overlay(path, {"src/wave.cpp": source.replace(before, after)},
                                    (), flags + settings.layout_flags))
        with pytest.raises(AssertionError):
            assert_original_dispatch_order(obj, image, address)
    assert path.read_bytes() == source


def test_live_declared_tail_matches_both_original_tables(family):
    _, _, trials, image = family
    for objects in trials:
        observe_tail(objects, image)
        # Inherited slots remain outside this test's acceptance scope. Their
        # pending discovery must not become a requirement to preserve defects.
        for unit, obj in objects.items():
            assert class_layout(obj, "Wave")["size"] == 0x6C


def test_original_calls_require_virtual_initialization_load_and_pitch(family):
    _, _, _, image = family
    assert indirect_offsets(image.read(Span(0x44602B, 0x44603A))) == [0x98]
    assert indirect_offsets(image.read(Span(0x445CB5, 0x445CBF))) == [0x8C]
    assert indirect_offsets(image.read(Span(0x4C6DB0, 0x4C6DEF))) == [0x88, 0x8C]


def test_compiled_calls_use_real_virtual_methods_and_preserve_direct_base_call(family):
    _, _, trials, _ = family
    for objects in trials:
        obj = objects["src/wave.cpp"]
        code, _, _ = obj.subject(obj.function(symbol(0x4C6DB0)))
        assert indirect_offsets(code) == [0x88, 0x8C]
        effect, _, _ = obj.subject(obj.function(symbol(0x445BB0)))
        assert indirect_offsets(effect).count(0x8C) == 1
        assert symbol(0x4C69B0) in references(obj, symbol(0x445BB0))
        assert references(obj, symbol(0x4C6920)).count(symbol(0x4C6CE0)) == 1


@pytest.mark.parametrize("damage", ["nonvirtual_pitch", "overload_order"])
def test_compiled_tables_reject_missing_virtual_and_wrong_overload_order(family, damage):
    compiler, settings, trials, image = family
    path = ROOT / "src/wave.h"
    source = path.read_bytes()
    if damage == "nonvirtual_pitch":
        before, after = b"virtual int set_pitch(int pitch);", b"int set_pitch(int pitch);"
    else:
        before = (b"  virtual int load(char *a1, unsigned long a2);      // 36\n"
                  b"  virtual int load();                              // 35")
        after = (b"  virtual int load();                              // 35\n"
                 b"  virtual int load(char *a1, unsigned long a2);      // 36")
    assert source.count(before) == 1
    for flags, pristine in zip(settings.flag_sets, trials):
        observe_tail(pristine, image)
        obj = Coff(compiler.overlay(ROOT / "src/wave.cpp", {"src/wave.h": source.replace(before, after)},
                                    (), flags + settings.layout_flags))
        damaged = {**pristine, "src/wave.cpp": obj}
        rows = table_observations(damaged, image)
        table, = (r for r in rows if r["unit"] == "src/wave.cpp" and r["symbol"] == "??_7Wave@@6B@")
        if damage == "nonvirtual_pitch":
            assert table["diagnostic"]["emitted_size"] == 56 * 4
        else:
            assert table["diagnostic"]["emitted_size"] == 57 * 4
            assert all(table["diagnostic"]["entries"][slot]["observation"] == "target_word_differs"
                       for slot in (35, 36))
        with pytest.raises(AssertionError):
            observe_tail(damaged, image)
    assert path.read_bytes() == source


def test_recovered_load_has_exact_bytes_and_virtual_reference_operands(family):
    _, _, trials, image = family
    row, = (r for r in read_file(ROOT / "src/wave.cpp") if r.address == 0x4C6DB0)
    binding = FunctionBinding(Origin(image_pin(ROOT), "pe:va", row.address),
                              tuple(Span(a, b) for a, b in row.image_spans),
                              row.symbol or row.name, "src/wave.cpp")
    results = [check_objects([binding], {"src/wave.cpp": objects["src/wave.cpp"]}, image)[0]
               for objects in trials]
    assert any(result["boundary"] == result["bytes"] == result["references"] == "pass"
               for result in results), results


def test_pitch_contract_compiles_to_the_original_virtual_slot(family):
    compiler, settings, _, _ = family
    path = ROOT / "src/wave.cpp"
    source = path.read_bytes()
    # A compiler contract probe only: FX::init itself remains unrecovered.
    probe = b"\nextern \"C\" int recovery_wave_pitch_probe(Effect *effect) { return effect->set_pitch(1100); }\n"
    for flags in settings.flag_sets:
        obj = Coff(compiler.overlay(path, {"src/wave.cpp": source + probe}, (), flags + settings.layout_flags))
        code, _, _ = obj.subject(obj.function("_recovery_wave_pitch_probe"))
        # VC6 may tail-jump rather than call; retain the same virtual operand.
        decoder = Cs(CS_ARCH_X86, CS_MODE_32)
        decoder.detail = True
        targets = [op.operands[0].mem.disp for op in decoder.disasm(code, 0)
                   if op.mnemonic in ("call", "jmp") and op.operands[0].type == X86_OP_MEM
                   and op.operands[0].mem.base]
        assert targets == [0x98]
    assert path.read_bytes() == source
