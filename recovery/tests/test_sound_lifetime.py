"""The real fade base and Sound initialization, not complete audio lifetime proof."""

from pathlib import Path

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86 import X86_OP_MEM, X86_OP_IMM, X86_REG_ESI, X86_REG_FS

from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.codeview import class_layout
from match_recovery.generated_binary import check_generated
from match_recovery.generated_source import read_generated
from match_recovery.model import Origin, Span
from match_recovery.shared_source import read_shared
from match_recovery.source import read_file
from match_recovery.source_inventory import image_pin
from test_application_link_recovery import ROOT, references, run_recovered_harness
from test_sound_virtual_surface import family


ROOT_TABLE = "??_7SoundFadeState@@6B@"
ROOT_DTOR = "??1SoundFadeState@@IAE@XZ"


def decode(code, start=0):
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    return list(decoder.disasm(code, start))


def test_original_constructor_and_cleanup_witness_the_nontrivial_prefix(family):
    _, _, _, image = family
    ops = decode(image.read(Span(0x4C6080, 0x4C611A)), 0x4C6080)
    stores = [op for op in ops if op.mnemonic == "mov" and op.operands[0].type == X86_OP_MEM
              and op.operands[0].mem.base == X86_REG_ESI]
    tables = [(op.address, op.operands[1].imm) for op in stores
              if op.operands[0].mem.disp == 0 and op.operands[1].type == X86_OP_IMM]
    assert tables == [(0x4C60AA, 0x66E444), (0x4C60CC, 0x66E3C0)]
    assert [op.operands[0].mem.disp for op in stores if op.address < 0x4C60CC] == [0, 4, 8, 0x30]
    assert not any(op.operands[0].mem.disp == 0x34 for op in stores)
    cleanup = decode(image.read(Span(0x4C8450, 0x4C8457)), 0x4C8450)
    assert [op.mnemonic for op in cleanup] == ["mov", "ret"]
    assert cleanup[0].operands[1].imm == 0x66E444
    assert image.read(Span(0x66E444, 0x66E44C)) == (0x4482F0).to_bytes(4, "little") * 2
    voice_clear = decode(image.read(Span(0x4C8D49, 0x4C8D62)), 0x4C8D49)
    assert voice_clear[0].mnemonic == "push" and voice_clear[0].operands[0].imm == 4
    assert voice_clear[1].mnemonic == "lea" and voice_clear[1].operands[1].mem.disp == 0x54
    assert voice_clear[-1].mnemonic == "call" and voice_clear[-1].operands[0].imm == 0x6465F0


def test_live_compiler_keeps_the_real_base_and_all_family_extents(family):
    _, _, trials, _ = family
    for objects in trials:
        obj = objects["src/sound.cpp"]
        base = class_layout(obj, "SoundFadeState")
        assert base["size"] == 0x3C and base["bases"] == []
        assert base["members"] == {"volume_": 4, "pan_8_": 8, "state_C_": 12,
                                   "loop_flag_30_": 48, "delay_": 52, "fade_38_": 56}
        assert base["member_types"]["state_C_"]["size"] == 36
        sound = class_layout(obj, "Sound")
        assert sound["size"] == 0x54
        assert [(b["class"], b["offset"], b["virtual"]) for b in sound["bases"]] == [("SoundFadeState", 0, False)]
        assert sound["members"] == {"device_": 60, "flags_40_": 64, "chain_prev_": 68,
                                    "chain_next_": 72, "fname_": 76, "type_": 80}
        assert class_layout(objects["src/wave.cpp"], "Wave")["size"] == 0x6C
        for name in ("??0Sound@@QAE@XZ", "??1Sound@@UAE@XZ"):
            code, _, _ = obj.subject(obj.function(name))
            assert any(operand.type == X86_OP_MEM and operand.mem.segment == X86_REG_FS
                       for op in decode(code) for operand in op.operands)
        assert ROOT_TABLE in references(obj, "??1Sound@@UAE@XZ")
        assert references(objects["src/wave.cpp"], "??0Wave@@QAE@XZ").count("??0Sound@@QAE@XZ") == 1


def test_root_slots_and_compiler_cleanup_keep_their_original_targets(family):
    _, _, trials, image = family
    pin = image_pin(ROOT)
    shared, = (b for b in read_shared(ROOT, pin) if b.symbol == "?set_fade@SoundFadeState@@UAEHK@Z")
    body, = (r for r in read_file(ROOT / "src/sound.cpp") if r.address == 0x4482F0)
    binding, = (b for b in read_generated(ROOT, pin) if b.symbol == ROOT_TABLE)
    assert binding.span is None  # Complete original table bounds remain unclaimed.
    targets = {body.symbol: body.address, shared.symbol: shared.origin.address}
    for objects in trials:
        _, rows = check_generated([binding], objects, image, targets)
        assert rows
        for row in rows:
            assert row["content"] == "unknown"
            assert row["diagnostic"]["emitted_size"] == 8
            assert all(entry["observation"] == "target_word_agrees" for entry in row["diagnostic"]["entries"])
        # Bounded cleanup comparison with the declared root-table identity;
        # it does not certify the complete table or final-link selection.
        cleanup = FunctionBinding(Origin(pin, "pe:va", 0x4C8450), (Span(0x4C8450, 0x4C8457),), ROOT_DTOR, "src/sound.cpp")
        row, = check_objects([cleanup], objects, image, external_targets={ROOT_TABLE: 0x66E444})
        assert row["boundary"] == row["bytes"] == row["references"] == "pass", row
    rows = [check_objects([FunctionBinding(shared.origin, (shared.span,), shared.symbol, shared.unit)], objects, image)[0]
            for objects in trials]
    assert any(row["result"] == "pass" for row in rows), rows


def run_constructor(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "sound_lifetime_harness.cpp", [
        ("// LIVE_FADE_INTERVAL", "src/sound.cpp", 0x4482F0),
        ("// LIVE_SOUND_FADE", "src/sound.cpp", 0x4C6580),
        ("// LIVE_SOUND_CONSTRUCTOR", "src/sound.cpp", 0x4C6080),
        ("// LIVE_NEXT", "src/sound.cpp", 0x448310),
        ("// LIVE_PREV", "src/sound.cpp", 0x448320),
        ("// LIVE_TYPE", "src/sound.cpp", 0x448330),
        ("// LIVE_SET_TYPE", "src/sound.cpp", 0x4C61E0),
        ("// LIVE_VOICE_CONSTRUCTOR", "src/net_class.cpp", 0x4C8CC0)], declarations=[
        ("// LIVE_FADE_DECLARATION", "src/sound.h", "class SoundFadeState {", "// RECOVERY_MEMBER: v1 SoundFadeState volume_"),
        ("// LIVE_SOUND_DECLARATION", "src/sound.h", "class Sound : public SoundFadeState {", "// Two independent sources agree on 0x54"),
        ("// LIVE_VOICE_DECLARATION", "src/net_class.h", "class VoiceTx : public Sound {", "static_assert(sizeof(VoiceTx)"),
        ("// LIVE_ROOT_FADE", "src/sound.cpp", "int SoundFadeState::set_fade(", "// Construct the Sound-owned fields")])


def test_actual_constructor_sets_owned_state_and_leaves_uninitialized_delay_untouched(tmp_path, monkeypatch):
    run_constructor(tmp_path, monkeypatch)


@pytest.mark.parametrize("unit,before,after", [
    ("src/sound.h", "volume_ = 0x7F;", "volume_ = 0x7E;"),
    ("src/sound.h", "sizeof(state_C_)", "sizeof(state_C_) - 4"),
    ("src/sound.h", "loop_flag_30_ = 0;", "loop_flag_30_ = 1;"),
    ("src/sound.cpp", "Sound::set_fade(1000);", "Sound::set_fade(999);"),
    ("src/sound.cpp", "Sound::set_fade(1000);", "Sound::set_fade(1000); delay_ = 0;"),
    ("src/sound.cpp", "device_ = 0;", "device_ = reinterpret_cast<void *>(1);"),
    ("src/net_class.cpp", "memset(&flags_54_, 0, sizeof(flags_54_));", "/* missing own-state clear */"),
])
def test_actual_constructor_contract_rejects_damaged_initialization(tmp_path, monkeypatch, unit, before, after):
    original_read = Path.read_text
    target = ROOT / unit
    assert original_read(target).count(before) == 1
    def damaged_read(path, *args, **kwargs):
        text = original_read(path, *args, **kwargs)
        return text.replace(before, after) if path == target else text
    monkeypatch.setattr(Path, "read_text", damaged_read)
    with pytest.raises(AssertionError):
        run_constructor(tmp_path, monkeypatch)
