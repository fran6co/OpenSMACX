"""Source and optional VC6 controls for the shared Sound/Midi hierarchy."""
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


def test_midi_inherits_sound_and_retains_all_extension_slots():
    header = (ROOT / "src/midi.h").read_text()
    block = header.split("class Midi : public Sound {", 1)[1]
    block = block.split("\n};", 1)[0]
    extension = block.split("// Genuine added primary slots33..74", 1)[1]
    methods = re.findall(r"virtual\s+[^;]+;", extension)
    assert len(methods) == 42
    assert len(VC6_CALLS) == 75
    annotated = re.findall(r"virtual\s+[^;]+;\s*//\s*(\d+)", extension)
    assert sorted(int(slot) for slot in annotated) == list(range(33, 75))
    assert "class MidiDelegate" not in header
    assert "->unload()" in (ROOT / "src/midi.cpp").read_text()
    assert "->unload()" in (ROOT / "src/ambience.cpp").read_text()
    assert "Midi *const delegate = static_cast<Midi *>(device_);" in (ROOT / "src/midi.cpp").read_text()
    assert "static_cast<Midi *>(device_)" in (ROOT / "src/ambience.cpp").read_text()


def test_host_declarations_allow_inherited_and_midi_get_time_calls(tmp_path):
    compiler_name = shutil.which("g++") or shutil.which("clang++")
    if compiler_name is None:
        pytest.skip("g++ and clang++ are unavailable")
    midi = (ROOT / "src/midi.h").read_text()
    midi_decl = midi[midi.index("class Midi : public Sound {"):]
    midi_decl = midi_decl[:midi_decl.index("\n};") + 3]
    source = tmp_path / "midi_prefix.cpp"
    source.write_text(f'''#include <cstdint>
typedef unsigned char uint8_t;
struct TrackSet; struct ChannelCtrl; struct ProgramCtrl; struct TrackCtrl;
class Sound {{ public:
  virtual ~Sound() {{}}
  virtual int set_fade(unsigned long); virtual int set_fade_interval(unsigned long);
  virtual int UNK1(int); virtual int load(const char *); virtual int unload();
  virtual int play(); virtual int play(unsigned int); virtual int stop();
  virtual int fade(); virtual void fade(unsigned long);
  virtual int fade_in(); virtual void fade_in(unsigned int);
  virtual void ramp(int, int, unsigned int); virtual int release();
  virtual int get_game_hwnd(); virtual void set_volume(int); virtual void set_pan(int);
  virtual void set_loop_state(long); virtual void set_delay(unsigned int);
  virtual void set_type(unsigned int); virtual int set_fade_in(unsigned int);
  virtual int is_looping(); virtual int is_playing(); virtual int unk_slot24();
  virtual Sound *get_next(); virtual Sound *get_prev();
  virtual int set_attrib(unsigned long); virtual int get_attrib();
  virtual int get_time(); virtual uint32_t get_type(); virtual int attach(); virtual int detach();
}};
{midi_decl}
int call_inherited_time(Midi *p) {{
  return static_cast<Sound *>(p)->get_time();
}}
int call_midi_time(Midi *p) {{ return p->get_time(1UL); }}
int call_inherited_unload(Midi *p) {{ return p->unload(); }}
int call_filename_load(Midi *p) {{ return p->load("x"); }}
int call_noarg_load(Midi *p) {{ return p->load(); }}
int call_midi_init(Midi *p) {{ return p->init("x", 0UL); }}
''')
    import subprocess
    subprocess.run([compiler_name, "-std=c++11", "-c", str(source), "-o", str(tmp_path / "midi_prefix.o")], check=True)


VC6_CALLS = (
    "static_cast<Sound *>(p)->set_fade(1UL)",
    "static_cast<Sound *>(p)->set_fade_interval(1UL)",
    "delete static_cast<Sound *>(p)",
    "static_cast<Sound *>(p)->UNK1(1)",
    'p->load("x")',
    "static_cast<Sound *>(p)->unload()",
    "static_cast<Sound *>(p)->play(1U)",
    "static_cast<Sound *>(p)->play()",
    "static_cast<Sound *>(p)->stop()",
    "static_cast<Sound *>(p)->fade(1UL)",
    "static_cast<Sound *>(p)->fade()",
    "static_cast<Sound *>(p)->fade_in(1U)",
    "static_cast<Sound *>(p)->fade_in()",
    "static_cast<Sound *>(p)->ramp(1, 2, 3U)",
    "static_cast<Sound *>(p)->release()",
    "static_cast<Sound *>(p)->get_game_hwnd()",
    "static_cast<Sound *>(p)->set_volume(1)",
    "static_cast<Sound *>(p)->set_pan(1)",
    "static_cast<Sound *>(p)->set_loop_state(1L)",
    "static_cast<Sound *>(p)->set_delay(1U)",
    "static_cast<Sound *>(p)->set_type(1U)",
    "static_cast<Sound *>(p)->set_fade_in(1U)",
    "static_cast<Sound *>(p)->is_looping()",
    "static_cast<Sound *>(p)->is_playing()",
    "static_cast<Sound *>(p)->unk_slot24()",
    "static_cast<Sound *>(p)->get_next()",
    "static_cast<Sound *>(p)->get_prev()",
    "static_cast<Sound *>(p)->set_attrib(1UL)",
    "static_cast<Sound *>(p)->get_attrib()",
    "static_cast<Sound *>(p)->get_time()",
    "static_cast<Sound *>(p)->get_type()",
    "static_cast<Sound *>(p)->attach()",
    "static_cast<Sound *>(p)->detach()",
    "p->set_track(static_cast<uint8_t *>(0), 0UL)",
    "p->set_tempo(1UL)",
    "p->mute_track(1UL)",
    "p->set_active_tracks(1U)",
    "p->set_active_tracks(1UL, 2UL)",
    "p->add_active_trackset(static_cast<TrackSet *>(0))",
    "p->get_trackset(1U)",
    "p->set_active_range_lo(1U, 2U)",
    "p->set_active_range_hi(1U, 2U)",
    "p->remove_active_trackset(1U)",
    "p->unmute_track(1UL)",
    "p->sub_4c7f70(1, 2)",
    "p->sub_4c7fa0(1U)",
    "p->sub_4c7ff0(1, 2)",
    "p->sub_4c7fc0(1, 2)",
    "p->set_track_ctrl(static_cast<TrackCtrl *>(0))",
    "p->get_ntracks()",
    "p->reset()",
    "p->init(static_cast<char *>(0), 0UL)",
    "p->load()",
    "p->get_time(1UL)",
    "p->map_patch(static_cast<char *>(0), 1UL)",
    "p->map_patch(static_cast<char *>(0), 1UL, 2UL)",
    "p->clear_patch(1UL)",
    "p->set_channel_ctrl(static_cast<ChannelCtrl *>(0))",
    "p->set_program_ctrl(static_cast<ProgramCtrl *>(0))",
    "p->set_base_path(\"x\")",
    "p->sub_4c8020(1)",
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
            f'extern "C" int midi_provider_slot_{slot:02d}(Midi *p) '
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
