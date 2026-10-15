"""Source and optional VC6 controls for MidiDelegate's shared Sound prefix."""
from __future__ import annotations

import os
import re
import shutil
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]


def test_pinned_ambience_provider_constructs_midi_prefix():
    """Bounded dependency witness, not original class-name/type identity proof."""
    from hashlib import sha256
    import struct
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    image_path = Path(os.environ.get('OPENSMACX_IMAGE', ROOT / '.opensmacx/game/terranx_original.exe'))
    dll = image_path.with_name('soundx.dll')
    if not dll.is_file():
        pytest.skip('pinned SoundX dependency unavailable')
    raw = dll.read_bytes()
    digest = sha256(raw).hexdigest()
    assert digest == 'ac272e244b3f1339d9ce63bc2e7acf76605c6ac2695bba3230e72ea59039a3f5'
    image = PeImage(raw, digest)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    instructions = list(decoder.disasm(image.read(Span(0x100039C0, 0x10003A2B)), 0x100039C0))
    rendered = [(i.mnemonic, i.op_str) for i in instructions]
    base_call = rendered.index(('call', '0x100022a7'))
    # Constructor saves ECX in ESI without adjusting the base receiver.
    assert ('mov', 'esi, ecx') in rendered[:base_call]
    assert not any(mnemonic in ('mov', 'lea', 'xor', 'add', 'sub', 'pop')
                   and (op == 'ecx' or op.startswith('ecx,'))
                   for mnemonic, op in rendered[:base_call])
    assert rendered.index(('mov', 'dword ptr [esi], 0x10064000')) > base_call
    thunk = list(decoder.disasm(image.read(Span(0x100022A7, 0x100022AC)), 0x100022A7))
    assert [(i.mnemonic, i.op_str) for i in thunk] == [('jmp', '0x1001b5e0')]
    midi = struct.unpack('<33I', image.read(Span(0x10064794, 0x10064818)))
    ambience = struct.unpack('<33I', image.read(Span(0x10064000, 0x10064084)))
    assert [i for i, (a, b) in enumerate(zip(midi, ambience)) if a != b] == [2, 4]
    assert midi[5] == ambience[5] == 0x10001046


def test_pinned_midi_provider_init_and_load_slot_contracts():
    """Mode-2 provider slots 51/52 retain the game's MIDI call contracts."""
    from hashlib import sha256
    import struct
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    image_path = Path(os.environ.get('OPENSMACX_IMAGE', ROOT / '.opensmacx/game/terranx_original.exe'))
    dll = image_path.with_name('soundx.dll')
    if not dll.is_file():
        pytest.skip('pinned SoundX dependency unavailable')
    raw = dll.read_bytes()
    digest = sha256(raw).hexdigest()
    assert digest == 'ac272e244b3f1339d9ce63bc2e7acf76605c6ac2695bba3230e72ea59039a3f5'
    image = PeImage(raw, digest)
    # Mode-2 table slots 51/52 are exactly +0xCC/+0xD0.
    slots = struct.unpack('<2I', image.read(Span(0x10064860, 0x10064868)))
    assert slots == (0x10001186, 0x1000134D)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    thunk51 = [(i.mnemonic, i.op_str) for i in decoder.disasm(
        image.read(Span(0x10001186, 0x1000118B)), 0x10001186)]
    thunk52 = [(i.mnemonic, i.op_str) for i in decoder.disasm(
        image.read(Span(0x1000134D, 0x10001352)), 0x1000134D)]
    assert thunk51 == [('jmp', '0x10029bc0')]
    assert thunk52 == [('jmp', '0x10029d20')]

    init = list(decoder.disasm(image.read(Span(0x10029BC0, 0x10029C15)), 0x10029BC0))
    assert any(i.mnemonic == 'mov' and i.op_str == 'edi, dword ptr [ebp + 8]' for i in init)
    assert any(i.mnemonic == 'mov' and i.op_str == 'dword ptr [esi + 0x4c], 0' for i in init)
    assert any(i.mnemonic == 'add' and i.op_str == 'esi, 0x54' for i in init)
    assert any(i.mnemonic == 'ret' and i.op_str == '8' for i in init)

    load = list(decoder.disasm(image.read(Span(0x10029D20, 0x10029D58)), 0x10029D20))
    assert any(i.mnemonic == 'mov' and i.op_str == 'esi, dword ptr [edi + 0x4c]' for i in load)
    assert any(i.mnemonic == 'mov' and i.op_str == 'ecx, dword ptr [edi + 0x3c]' for i in load)
    assert any(i.mnemonic == 'ret' and i.op_str == '' for i in load)


def test_midi_delegate_inherits_sound_prefix_and_retains_all_extension_slots():
    header = (ROOT / "src/midi.h").read_text()
    block = header.split("class MidiDelegate : public SoundDeviceInterface {", 1)[1]
    block = block.split("\n};", 1)[0]
    methods = re.findall(r"virtual\s+[^;]+;", block)
    assert len(methods) == 42
    assert len(VC6_CALLS) == 75
    annotated = re.findall(r"virtual\s+[^;]+;\s*//\s*slot\s+(0x[0-9a-fA-F]+)", block)
    assert all(0x84 <= int(slot, 16) <= 0x128 for slot in annotated)
    assert "slot005" not in header
    assert "->unload()" in (ROOT / "src/midi.cpp").read_text()
    assert "->unload()" in (ROOT / "src/ambience.cpp").read_text()
    assert "->slot005()" not in (ROOT / "src/midi.cpp").read_text()
    assert "->slot005()" not in (ROOT / "src/ambience.cpp").read_text()


def test_host_declarations_allow_inherited_and_midi_get_time_calls(tmp_path):
    compiler_name = shutil.which("g++") or shutil.which("clang++")
    if compiler_name is None:
        pytest.skip("g++ and clang++ are unavailable")
    sound = (ROOT / "src/soundx_sound_abi.h").read_text()
    midi = (ROOT / "src/midi.h").read_text()
    sound_decl = sound[sound.index("class SoundDeviceInterface {"):]
    sound_decl = sound_decl[:sound_decl.index("};") + 2]
    midi_decl = midi[midi.index("class MidiDelegate : public SoundDeviceInterface {"):]
    midi_decl = midi_decl[:midi_decl.index("\n};") + 3]
    source = tmp_path / "midi_prefix.cpp"
    source.write_text(f'''#include <cstdint>
struct Sound;
struct TrackSet; struct ChannelCtrl; struct ProgramCtrl; struct TrackCtrl;
{sound_decl}
{midi_decl}
int call_inherited_time(MidiDelegate *p) {{
  return static_cast<SoundDeviceInterface *>(p)->get_time();
}}
int call_midi_time(MidiDelegate *p) {{ return p->get_time(1UL); }}
int call_inherited_unload(MidiDelegate *p) {{ return p->unload(); }}
int call_filename_load(MidiDelegate *p) {{ return p->load("x"); }}
int call_noarg_load(MidiDelegate *p) {{ return p->load(); }}
int call_midi_init(MidiDelegate *p) {{ return p->init("x", 0UL); }}
''')
    import subprocess
    subprocess.run([compiler_name, "-std=c++11", "-c", str(source), "-o", str(tmp_path / "midi_prefix.o")], check=True)


VC6_CALLS = (
    "static_cast<SoundDeviceInterface *>(p)->set_fade(1UL)",
    "static_cast<SoundDeviceInterface *>(p)->set_fade_interval(1UL)",
    "delete static_cast<SoundDeviceInterface *>(p)",
    "static_cast<SoundDeviceInterface *>(p)->UNK1(1)",
    'p->load("x")',
    "static_cast<SoundDeviceInterface *>(p)->unload()",
    "static_cast<SoundDeviceInterface *>(p)->play(1U)",
    "static_cast<SoundDeviceInterface *>(p)->play()",
    "static_cast<SoundDeviceInterface *>(p)->stop()",
    "static_cast<SoundDeviceInterface *>(p)->fade(1UL)",
    "static_cast<SoundDeviceInterface *>(p)->fade()",
    "static_cast<SoundDeviceInterface *>(p)->fade_in(1U)",
    "static_cast<SoundDeviceInterface *>(p)->fade_in()",
    "static_cast<SoundDeviceInterface *>(p)->ramp(1, 2, 3U)",
    "static_cast<SoundDeviceInterface *>(p)->release()",
    "static_cast<SoundDeviceInterface *>(p)->get_game_hwnd()",
    "static_cast<SoundDeviceInterface *>(p)->set_volume(1)",
    "static_cast<SoundDeviceInterface *>(p)->set_pan(1)",
    "static_cast<SoundDeviceInterface *>(p)->set_loop_state(1L)",
    "static_cast<SoundDeviceInterface *>(p)->set_delay(1U)",
    "static_cast<SoundDeviceInterface *>(p)->set_type(1U)",
    "static_cast<SoundDeviceInterface *>(p)->set_fade_in(1U)",
    "static_cast<SoundDeviceInterface *>(p)->is_looping()",
    "static_cast<SoundDeviceInterface *>(p)->is_playing()",
    "static_cast<SoundDeviceInterface *>(p)->unk_slot24()",
    "static_cast<SoundDeviceInterface *>(p)->get_next()",
    "static_cast<SoundDeviceInterface *>(p)->get_prev()",
    "static_cast<SoundDeviceInterface *>(p)->set_attrib(1UL)",
    "static_cast<SoundDeviceInterface *>(p)->get_attrib()",
    "static_cast<SoundDeviceInterface *>(p)->get_time()",
    "static_cast<SoundDeviceInterface *>(p)->get_type()",
    "static_cast<SoundDeviceInterface *>(p)->attach()",
    "static_cast<SoundDeviceInterface *>(p)->detach()",
    "p->set_track(static_cast<uint8_t *>(0), 0UL)",
    "p->set_tempo(1UL)",
    "p->mute_track(1UL)",
    "p->set_active_tracks(1U)",
    "p->slot037(1UL, 2UL)",
    "p->add_active_trackset(static_cast<TrackSet *>(0))",
    "p->get_trackset(1U)",
    "p->set_active_range_lo(1U, 2U)",
    "p->set_active_range_hi(1U, 2U)",
    "p->remove_active_trackset(1U)",
    "p->unmute_track(1UL)",
    "p->slot044(1, 2)",
    "p->slot045(1U)",
    "p->slot046(1, 2)",
    "p->slot047(1, 2)",
    "p->set_track_ctrl(static_cast<TrackCtrl *>(0))",
    "p->get_ntracks()",
    "p->reset()",
    "p->init(static_cast<char *>(0), 0UL)",
    "p->load()",
    "p->get_time(1UL)",
    "p->map_patch(static_cast<char *>(0), 1UL)",
    "p->slot055(static_cast<char *>(0), 1UL, 2UL)",
    "p->clear_patch(1UL)",
    "p->set_channel_ctrl(static_cast<ChannelCtrl *>(0))",
    "p->set_program_ctrl(static_cast<ProgramCtrl *>(0))",
    "p->set_base_path(\"x\")",
    "p->slot060(1)",
    "p->get_control_track()",
    "p->get_total_track_ticks(1UL)",
    "p->switch_state(true)",
    "p->set_switch_type(1UL)",
    "p->set_nswitch_threads(1UL)",
    "p->add_switch_range(1UL, 2UL)",
    "p->play_trackset(1U)",
    "p->stop_trackset(1U)",
    "p->xpose_trackset(1U, 2)",
    "p->is_trackset_playing(1U)",
    "p->set_play_type(1U)",
    "p->load_patch(1U)",
    "p->unload_patch(1U)",
    "p->set_patch(1U, 2U)",
)


def test_opt_in_vc6_probe_maps_sound_and_midi_delegate_calls_to_slots():
    if os.environ.get("OPENSMACX_RUN_VC6_TESTS") != "1":
        pytest.skip("set OPENSMACX_RUN_VC6_TESTS=1 in the coordinator VC6 environment")
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from capstone.x86 import X86_OP_MEM
    from match_recovery.compiler import Compiler
    from match_recovery.measurement import Measurement
    from match_recovery.pe_coff import Coff
    from recovery.environment.wine import VC6_ROOT

    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or not shutil.which("wine32"):
        pytest.skip("VC6 and Wine32 are required for the native provider-slot control")
    path = ROOT / "src/midi.cpp"
    original = path.read_bytes()
    expressions = []
    for slot, expression in enumerate(VC6_CALLS):
        expressions.append(
            f'extern "C" int midi_provider_slot_{slot:02d}(MidiDelegate *p) '
            f'{{ {expression}; return 0; }}'
        )
    overlay = original.decode("utf-8") + "\n" + "\n".join(expressions)
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    for flags in settings.flag_sets:
        raw = compiler.overlay(path, {"src/midi.cpp": overlay.encode("utf-8")}, (), flags + settings.layout_flags)
        obj = Coff(raw)
        for slot in range(len(VC6_CALLS)):
            function = obj.function(f"_midi_provider_slot_{slot:02d}")
            code, _, _ = obj.subject(function)
            indirect_calls = [
                operand.operands[0].mem.disp
                for operand in decoder.disasm(code, 0)
                if operand.mnemonic == "call"
                and operand.operands[0].type == X86_OP_MEM
                and operand.operands[0].mem.base
                and not operand.operands[0].mem.index
            ]
            assert indirect_calls == [slot * 4], (slot, indirect_calls)
