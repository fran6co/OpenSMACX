"""Recovered inherited slots and a nonvirtual helper, not full Sound recovery."""

import os
from pathlib import Path
import shutil

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86 import X86_OP_MEM

from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.codeview import class_layout
from match_recovery.compiler import Compiler
from match_recovery.generated_binary import check_generated
from match_recovery.generated_source import read_generated
from match_recovery.measurement import Measurement
from match_recovery.model import Origin, Span
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.source import read_file
from match_recovery.source_inventory import image_pin
from test_application_link_recovery import ROOT, references


TABLES = {"??_7Sound@@6B@": 0x66E3C0, "??_7Wave@@6B@": 0x66E44C,
          "??_7Effect@@6B@": 0x66B7A4}
SLOTS = (1, 3, 6, 7, 11, 12, 25, 26, 30)
LOCAL_MEMBERS = {0x4482F0: "set_fade_interval", 0x448310: "get_next",
                 0x448320: "get_prev", 0x448330: "get_type"}


@pytest.fixture(scope="module")
def family():
    from recovery.environment.wine import VC6_ROOT
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None or not path.is_file():
        pytest.skip("VC6, Wine32 and pinned original image are required")
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    trials = [{unit: Coff(compiler(ROOT / unit, (), flags + settings.layout_flags))
               for unit in ("src/sound.cpp", "src/wave.cpp", "src/fx.cpp")}
              for flags in settings.flag_sets]
    return compiler, settings, trials, PeImage(path.read_bytes(), image_pin(ROOT))


def observations(objects, image):
    bindings = [b for b in read_generated(ROOT, image_pin(ROOT)) if b.symbol in TABLES]
    assert {b.symbol: b.origin.address for b in bindings} == TABLES
    targets = {r.symbol or r.name: r.address for unit in ("sound", "wave")
               for r in read_file(ROOT / f"src/{unit}.cpp")}
    _, rows = check_generated(bindings, objects, image, targets)
    assert {r["symbol"] for r in rows} == set(TABLES)
    return rows


def test_attribute_setter_is_the_original_wave_and_effect_override(family):
    _, _, trials, image = family
    assert int.from_bytes(image.read(Span(0x66E3C0 + 108, 0x66E3C0 + 112)), "little") == 0x645844
    for table in (0x66E44C, 0x66B7A4):
        assert int.from_bytes(image.read(Span(table + 108, table + 112)), "little") == 0x4C6F20
    row, = (r for r in read_file(ROOT / "src/wave.cpp") if r.address == 0x4C6F20)
    binding = FunctionBinding(Origin(image_pin(ROOT), "pe:va", row.address),
                              tuple(Span(a, b) for a, b in row.image_spans),
                              row.symbol or row.name, "src/wave.cpp")
    passing = []
    for objects in trials:
        for table in observations(objects, image):
            if table["symbol"] == "??_7Sound@@6B@":
                continue  # The base is pure in both source and image.
            entry = table["diagnostic"]["entries"][27]
            assert entry["original_absolute_relocation"]
            assert entry["observation"] == "target_word_agrees", entry
            assert [r["symbol"] for r in entry["relocations"]] == ["?set_attrib@Wave@@UAEHK@Z"]
        result, = check_objects([binding], objects, image)
        passing.append(result["boundary"] == result["bytes"] == result["references"] == "pass")
    assert any(passing), "no fixed compiler trial reproduces the complete setter"


def test_restoring_old_setter_placeholder_breaks_table_contract(family):
    compiler, settings, trials, image = family
    header = (ROOT / "src/sound.h").read_bytes()
    before = b"virtual int set_attrib(unsigned long attrib) = 0;"
    assert header.count(before) == 1
    for flags, objects in zip(settings.flag_sets, trials):
        changed = Coff(compiler.overlay(ROOT / "src/wave.cpp", {
            "src/sound.h": header.replace(before, b"virtual void unk_slot27();")
        }, (), flags + settings.layout_flags))
        table, = (r for r in observations({**objects, "src/wave.cpp": changed}, image)
                  if r["symbol"] == "??_7Wave@@6B@")
        assert table["diagnostic"]["entries"][27]["observation"] != "target_word_agrees"


def test_typed_sound_setter_call_dispatches_through_original_slot(family):
    compiler, settings, _, _ = family
    path = ROOT / "src/wave.cpp"
    probe = b'\nextern "C" int attribute_setter_probe(Sound *s, unsigned long value) { return s->set_attrib(value); }\n'
    source = path.read_bytes()
    for flags in settings.flag_sets:
        obj = Coff(compiler.overlay(path, {"src/wave.cpp": source + probe}, (), flags + settings.layout_flags))
        code, _, _ = obj.subject(obj.function("_attribute_setter_probe"))
        decoder = Cs(CS_ARCH_X86, CS_MODE_32)
        decoder.detail = True
        indirect = [op.operands[0].mem.disp for op in decoder.disasm(code, 0)
                    if op.mnemonic in ("call", "jmp") and op.operands[0].type == X86_OP_MEM]
        assert indirect == [27 * 4]
    assert path.read_bytes() == source


def assert_inherited_slots(objects, image):
    rows = observations(objects, image)
    for row in rows:
        assert row["content"] == "unknown"  # Whole-table bounds/contracts remain open.
        for slot in SLOTS:
            entry = row["diagnostic"]["entries"][slot]
            assert entry["original_absolute_relocation"]
            assert entry["observation"] == "target_word_agrees", (row["symbol"], entry)
    return rows


def test_original_tables_and_all_emitted_copies_preserve_inherited_method_identity(family):
    _, _, trials, image = family
    for table, address in TABLES.items():
        for slot, target in ((1, 0x4482F0), (3, 0x4C6430), (6, 0x4C64A0),
                             (7, 0x4C6480 if table == "??_7Sound@@6B@" else 0x4C6920),
                             (11, 0x4C6660), (12, 0x4C6620),
                             (25, 0x448310), (26, 0x448320), (30, 0x448330)):
            assert int.from_bytes(image.read(Span(address + slot * 4, address + slot * 4 + 4)), "little") == target
    for objects in trials:
        assert_inherited_slots(objects, image)
        assert class_layout(objects["src/sound.cpp"], "Sound")["size"] == 0x54


@pytest.mark.parametrize("method,first,second,slots", [
    ("play", "virtual int play();", "virtual int play(unsigned int effect);", (6, 7)),
    ("fade_in", "virtual int fade_in();", "virtual void fade_in(unsigned int a1);", (11, 12)),
])
def test_reinstating_old_overload_order_is_rejected(family, method, first, second, slots):
    compiler, settings, trials, image = family
    path = ROOT / "src/sound.h"
    source = path.read_bytes()
    a, b = first.encode(), second.encode()
    assert source.count(a) == source.count(b) == 1
    damaged_header = source.replace(a, b"DECLARATION_SWAP").replace(b, a).replace(b"DECLARATION_SWAP", b)
    for flags, pristine in zip(settings.flag_sets, trials):
        assert_inherited_slots(pristine, image)
        obj = Coff(compiler.overlay(ROOT / "src/sound.cpp", {"src/sound.h": damaged_header},
                                    (), flags + settings.layout_flags))
        damaged = {**pristine, "src/sound.cpp": obj}
        row, = (r for r in observations(damaged, image)
                if r["symbol"] == "??_7Sound@@6B@" and r["unit"] == "src/sound.cpp")
        assert row["diagnostic"]["emitted_size"] == 33 * 4
        assert all(row["diagnostic"]["entries"][slot]["observation"] == "target_word_differs"
                   for slot in slots)
        with pytest.raises(AssertionError):
            assert_inherited_slots(damaged, image)
    assert path.read_bytes() == source


def test_restoring_the_old_helper_name_reintroduces_the_wrong_override(family):
    compiler, settings, trials, image = family
    sources = {unit: (ROOT / unit).read_bytes() for unit in ("src/wave.h", "src/wave.cpp")}
    overrides = {unit: source.replace(b"probe_device_chain", b"UNK1") for unit, source in sources.items()}
    assert all(overrides[unit] != source for unit, source in sources.items())
    for flags, pristine in zip(settings.flag_sets, trials):
        assert_inherited_slots(pristine, image)
        obj = Coff(compiler.overlay(ROOT / "src/wave.cpp", overrides, (), flags + settings.layout_flags))
        damaged = {**pristine, "src/wave.cpp": obj}
        row, = (r for r in observations(damaged, image)
                if r["symbol"] == "??_7Wave@@6B@" and r["unit"] == "src/wave.cpp")
        entry = row["diagnostic"]["entries"][3]
        assert [r["symbol"] for r in entry["relocations"]] == ["?UNK1@Wave@@UAEHH@Z"]
        assert entry["observation"] != "target_word_agrees"
        with pytest.raises(AssertionError):
            assert_inherited_slots(damaged, image)
    assert all((ROOT / unit).read_bytes() == source for unit, source in sources.items())


def test_renamed_helper_keeps_its_original_body(family):
    _, _, trials, image = family
    record, = (r for r in read_file(ROOT / "src/wave.cpp") if r.address == 0x4C7080)
    binding = FunctionBinding(Origin(image_pin(ROOT), "pe:va", record.address),
                              tuple(Span(a, b) for a, b in record.image_spans),
                              record.symbol or record.name, "src/wave.cpp")
    rows = [check_objects([binding], {"src/wave.cpp": trial["src/wave.cpp"]}, image)[0]
            for trial in trials]
    assert any(r["boundary"] == r["bytes"] == r["references"] == "pass" for r in rows), rows


def test_ordinary_cpp_calls_use_the_original_inherited_slots(family):
    compiler, settings, _, _ = family
    path = ROOT / "src/wave.cpp"
    source = path.read_bytes()
    probe = b'''
extern "C" int sound_play_arg_probe(Sound *s, unsigned int n) { return s->play(n); }
extern "C" int sound_play_probe(Sound *s) { return s->play(); }
extern "C" void sound_fade_arg_probe(Sound *s, unsigned int n) { s->fade_in(n); }
extern "C" int sound_fade_probe(Sound *s) { return s->fade_in(); }
extern "C" int wave_base_slot_probe(Wave *w, int n) { return w->UNK1(n); }
extern "C" int wave_chain_probe(Wave *w, int n) { return w->probe_device_chain(n); }
extern "C" int sound_interval_probe(Sound *s, unsigned long n) { return s->set_fade_interval(n); }
extern "C" Sound *sound_next_probe(Sound *s) { return s->get_next(); }
extern "C" Sound *sound_prev_probe(Sound *s) { return s->get_prev(); }
extern "C" unsigned int sound_type_probe(Sound *s) { return s->get_type(); }
'''
    for flags in settings.flag_sets:
        obj = Coff(compiler.overlay(path, {"src/wave.cpp": source + probe}, (), flags + settings.layout_flags))
        for name, offset in (("sound_play_arg", 0x18), ("sound_play", 0x1C),
                             ("sound_fade_arg", 0x2C), ("sound_fade", 0x30), ("wave_base_slot", 0xC),
                             ("sound_interval", 4), ("sound_next", 0x64),
                             ("sound_prev", 0x68), ("sound_type", 0x78)):
            code, _, _ = obj.subject(obj.function("_" + name + "_probe"))
            decoder = Cs(CS_ARCH_X86, CS_MODE_32)
            decoder.detail = True
            targets = [op.operands[0].mem.disp for op in decoder.disasm(code, 0)
                       if op.mnemonic in ("call", "jmp") and op.operands[0].type == X86_OP_MEM
                       and op.operands[0].mem.base]
            assert targets == [offset]
        helper, = (r for r in read_file(path) if r.address == 0x4C7080)
        assert helper.symbol in references(obj, "_wave_chain_probe")
    assert path.read_bytes() == source


def local_binding(image, address):
    record, = (r for r in read_file(ROOT / "src/sound.cpp") if r.address == address)
    assert record.byte_exact  # The move retains, rather than re-grants, the old obligation.
    owner = "SoundFadeState" if address == 0x4482F0 else "Sound"
    assert record.symbol.startswith("?" + LOCAL_MEMBERS[address] + "@" + owner + "@@UAE")
    return FunctionBinding(Origin(image_pin(ROOT), "pe:va", address),
                           tuple(Span(a, b) for a, b in record.image_spans),
                           record.symbol, "src/sound.cpp")


@pytest.mark.parametrize("address", LOCAL_MEMBERS)
def test_typed_members_restore_the_original_body_without_raw_offset_redirects(family, address):
    _, _, trials, image = family
    binding = local_binding(image, address)
    rows = [check_objects([binding], objects, image)[0] for objects in trials]
    assert any(row["boundary"] == row["bytes"] == row["references"] == "pass" for row in rows), rows
    assert not (ROOT / "src/leaf_recoveries.cpp").exists()
    assert not (ROOT / "src/leaf_recoveries.h").exists()
    for unit in ("field_accessors",):
        assert all(r.address != address for r in read_file(ROOT / f"src/{unit}.cpp"))
        assert f"{address:08x}_redirect" not in (ROOT / f"src/{unit}.h").read_text()


def test_other_original_family_tables_share_these_local_member_contracts(family):
    _, _, _, image = family
    for table in (0x66E538, 0x66E664, 0x66E8C4):
        for slot, address in ((1, 0x4482F0), (25, 0x448310), (26, 0x448320), (30, 0x448330)):
            assert int.from_bytes(image.read(Span(table + 4 * slot, table + 4 * slot + 4)), "little") == address


@pytest.mark.parametrize("address,before,after", [
    (0x4482F0, b"if (interval == 0)", b"if (interval == 1)"),
    (0x448310, b"return chain_next_;", b"return chain_prev_;"),
    (0x448320, b"return chain_prev_;", b"return chain_next_;"),
    (0x448330, b"return type_;", b"return flags_40_;"),
])
def test_compiler_rejects_wrong_zero_guard_or_member_identity(family, address, before, after):
    compiler, settings, _, image = family
    path = ROOT / "src/sound.cpp"
    source = path.read_bytes()
    assert source.count(before) == (2 if address == 0x4482F0 else 1)
    binding = local_binding(image, address)
    for flags in settings.flag_sets:
        obj = Coff(compiler.overlay(path, {"src/sound.cpp": source.replace(before, after)},
                                    (), flags + settings.layout_flags))
        row, = check_objects([binding], {"src/sound.cpp": obj}, image)
        assert row["result"] == "fail", row
    assert path.read_bytes() == source
