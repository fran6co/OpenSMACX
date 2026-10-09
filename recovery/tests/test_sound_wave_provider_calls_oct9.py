"""Source controls for typed Sound/Wave DLL call views.

These checks cover client call selection and control flow. They do not claim
that a host compiler reproduces the VC6 external provider vtable ABI.
"""
from pathlib import Path
import os
import hashlib
import re
import shutil
import subprocess

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86 import X86_OP_MEM

from match_recovery.compiler import Compiler
from match_recovery.measurement import Measurement
from match_recovery.pe_coff import Coff


ROOT = Path(__file__).resolve().parents[2]


VC6_PROVIDER_PROBES = (
    (0, "p->set_fade(1UL)"),
    (1, "p->set_fade_interval(1UL)"),
    (2, "delete static_cast<SoundDeviceInterface *>(p)"),
    (3, "p->UNK1(1)"),
    (4, 'p->load("x")'),
    (5, "p->unload()"),
    (6, "p->play(static_cast<unsigned int>(1))"),
    (7, "p->play()"),
    (8, "p->stop()"),
    (9, "p->fade(1UL)"),
    (10, "p->fade()"),
    (11, "p->fade_in(static_cast<unsigned int>(1))"),
    (12, "p->fade_in()"),
    (13, "p->ramp(1, 2, 3U)"),
    (14, "p->release()"),
    (15, "p->get_game_hwnd()"),
    (16, "p->set_volume(1)"),
    (17, "p->set_pan(1)"),
    (18, "p->set_loop_state(1L)"),
    (19, "p->set_delay(1U)"),
    (20, "p->set_type(1U)"),
    (21, "p->set_fade_in(1U)"),
    (22, "p->is_looping()"),
    (23, "p->is_playing()"),
    (24, "p->unk_slot24()"),
    (25, "p->get_next()"),
    (26, "p->get_prev()"),
    (27, "p->set_attrib(1UL)"),
    (28, "p->get_attrib()"),
    (29, "p->get_time()"),
    (30, "p->get_type()"),
    (31, "p->attach()"),
    (32, "p->detach()"),
    (33, "p->reload()"),
    (34, "p->init(static_cast<char *>(0), 1UL)"),
    (35, "p->load()"),
    (36, "p->load(static_cast<char *>(0), 1UL)"),
    (37, "p->play(1)"),
    (38, "p->set_pitch(1)"),
    (39, "p->set_bufflimit(1U)"),
    (40, "p->set_asdr()"),
    (41, "p->set_attack(1U, 2U, 3U)"),
    (42, "p->set_sustain(1U, 2U, 3U)"),
    (43, "p->set_decay(1U, 2U, 3U)"),
    (44, "p->set_release(1U, 2U, 3U)"),
    (45, "p->get_time(1U)"),
    (46, "p->get_current_marker()"),
    (47, "p->get_ndevices()"),
    (48, "p->get_device_description(static_cast<char *>(0), 1, 2)"),
    (49, "p->get_ms_length()"),
    (50, "p->is_hwbuffer()"),
    (51, "p->set_position3d(1.0f, 2.0f, 3.0f)"),
    (52, "p->set_xpos(1.0f)"),
    (53, "p->set_ypos(1.0f)"),
    (54, "p->set_zpos(1.0f)"),
    (55, "p->is_3d()"),
    (56, "p->set_reverb_mix(1.0f)"),
)


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:index]
    raise AssertionError(f"unterminated function {signature}")


def test_wave_external_interface_declares_the_observed_tail():
    header = (ROOT / "src/voice.h").read_text()
    block = header.split("class WavePlaybackDeviceInterface", 1)[1].split("};", 1)[0]
    methods = re.findall(r"virtual\s+[^;]+;\s*//\s*(\d+)", block)
    observed = [int(slot) for slot in methods]
    assert sorted(observed) == list(range(33, 57))
    assert observed[2:4] == [36, 35]  # VC6 overload slot emission order.
    assert "virtual int load(char *, unsigned long) = 0;" in block
    assert "virtual int load() = 0;" in block


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_provider_interface_overloads_are_callable_without_adding_slots(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    header = (ROOT / "src/voice.h").read_text()
    declarations = header[header.index("class SoundDeviceInterface {"):]
    declarations = declarations[:declarations.index("class VoiceRxDeviceInterface :")]
    harness = f'''#include <cstdint>
struct Sound;
{declarations}
void call_overloads(WavePlaybackDeviceInterface &device, unsigned int effect,
                    int wave_effect, char *mutable_name, const char *name,
                    uint32_t wave_time) {{
  int a=device.play();
  int b=device.play(effect);
  int c=device.play(wave_effect);
  int d=device.load();
  int e=device.load(mutable_name, 0UL);
  int f=device.load(name);
  int g=device.get_time();
  int h=device.get_time(wave_time);
  (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h;
}}
'''
    source = tmp_path / "provider_overloads.cpp"
    source.write_text(harness)
    subprocess.run([compiler, "-std=c++11", "-c", str(source), "-o", str(tmp_path / "provider_overloads.o")], check=True)


def test_sound_calls_use_named_prefix_methods_and_keep_null_paths():
    source = (ROOT / "src/sound.cpp").read_text()
    assert "typedef int(__fastcall *sound_device_query)" not in source
    assert "query_sound_device" not in source
    code = re.sub(r"/\*.*?\*/|//[^\n]*", "", source, flags=re.S)
    assert "vtable_slot<" not in code and "original_slot<" not in code
    checks = {
        "int Sound::play()": ("int result = 0x14;", "if (device)", "->play()", "return result;"),
        "int Sound::fade()": ("if (fade_38_ == 0)", "->fade()"),
        "int Sound::unload()": ("->unload()", "detach();", "device_ = nullptr;"),
        "int Sound::set_fade_in(unsigned int fade_in)": ("fade_38_ = fade_in;", "->set_fade_in(fade_in)"),
        "int Sound::is_playing()": ("if (device_)", "->is_playing()", "return 0;"),
        "int Sound::is_looping()": ("if (device_)", "->is_looping()", "return 0;"),
        "int Sound::get_time()": ("if (device_)", "->get_time()", "return 0;"),
    }
    for signature, fragments in checks.items():
        body = function_body(code, signature)
        positions = [body.index(fragment) for fragment in fragments]
        assert positions == sorted(positions), signature


def test_wave_calls_use_named_provider_methods_with_null_guards():
    source = (ROOT / "src/wave.cpp").read_text()
    code = re.sub(r"/\*.*?\*/|//[^\n]*", "", source, flags=re.S)
    assert "vtable_slot<" not in code and "vtable_method<" not in code
    assert "original_slot<" not in code
    assert code.count("device_vtable + 0x6C") == 2
    checks = {
        "int Wave::set_pitch(int a1)": ("if (device_)", "->set_pitch(pitch)"),
        "int Wave::get_device_description(char *a1, int a2, int a3)": (
            "if (device_)", "->get_device_description(a1, a2, a3)", "return 1;"),
        "int Wave::load(const char *fname)": ("if (!device_)", "->set_attrib(", "Sound::load(fname)", "->get_ms_length()"),
        "int Wave::reload()": ("if (!device_)", "->set_attrib(", "->reload()"),
    }
    for signature, fragments in checks.items():
        body = function_body(source, signature)
        positions = [body.index(fragment) for fragment in fragments]
        assert positions == sorted(positions), signature
    for signature in ("int Wave::dyna_load(char *fname)", "void Wave::init(char *group_id, unsigned long flags)"):
        body = function_body(code, signature)
        assert body.index("device_vtable =") < body.index("get_attrib()")
        assert body.index("get_attrib()") < body.index("device_vtable + 0x6C")


def test_opt_in_vc6_probe_maps_all_provider_methods_to_slots():
    if os.environ.get("OPENSMACX_RUN_VC6_TESTS") != "1":
        pytest.skip("set OPENSMACX_RUN_VC6_TESTS=1 in the coordinator VC6 environment")
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or not shutil.which("wine32"):
        pytest.skip("VC6 and Wine32 are required for the native provider-slot control")

    path = ROOT / "src/wave.cpp"
    original = path.read_bytes()
    original_digest = hashlib.sha256(original).hexdigest()
    probes = []
    for slot, expression in VC6_PROVIDER_PROBES:
        probes.append(
            f'extern "C" int provider_slot_probe_{slot:02d}(WavePlaybackDeviceInterface *p) {{ '
            f"{expression}; return 0; }}"
        )
    probe_source = original.decode("utf-8") + "\n" + "\n".join(probes) + "\n"
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True

    for flags in settings.flag_sets:
        raw = compiler.overlay(path, {"src/wave.cpp": probe_source.encode("utf-8")},
                               (), flags + settings.layout_flags)
        obj = Coff(raw)
        assert hashlib.sha256(path.read_bytes()).hexdigest() == original_digest
        for slot, _ in VC6_PROVIDER_PROBES:
            function = obj.function(f"_provider_slot_probe_{slot:02d}")
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


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_host_mock_controls_exercise_production_call_bodies(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    sound = (ROOT / "src/sound.cpp").read_text()
    wave = (ROOT / "src/wave.cpp").read_text()
    sound_play = function_body(sound, "int Sound::play()")
    sound_loop = function_body(sound, "void Sound::set_loop_state(long loop_state)")
    wave_description = function_body(
        wave, "int Wave::get_device_description(char *a1, int a2, int a3)"
    )
    harness = f'''#include <cassert>
struct SoundDeviceInterface {{
  virtual int play() = 0;
  virtual void set_loop_state(long) = 0;
}};
struct WavePlaybackDeviceInterface {{
  virtual int get_device_description(char *, int, int) = 0;
}};
static SoundDeviceInterface *sound_device(void *p) {{ return static_cast<SoundDeviceInterface *>(p); }}
static WavePlaybackDeviceInterface *wave_playback_device(void *p) {{ return static_cast<WavePlaybackDeviceInterface *>(p); }}
struct Sound {{ void *device_; long loop_flag_30_; int play(); void set_loop_state(long); }};
struct Wave {{ void *device_; int get_device_description(char *, int, int); }};
int Sound::play() {{{sound_play}}}
void Sound::set_loop_state(long loop_state) {{{sound_loop}}}
int Wave::get_device_description(char *a1, int a2, int a3) {{{wave_description}}}
static int play_calls, play_result;
static long loop_argument;
static int description_calls, description_result, description_a2, description_a3;
struct SoundMock : SoundDeviceInterface {{
  int play() override {{ ++play_calls; return play_result; }}
  void set_loop_state(long value) override {{ ++description_calls; loop_argument=value; }}
}};
struct WaveMock : WavePlaybackDeviceInterface {{
  int get_device_description(char *buffer, int a2, int a3) override {{
    ++description_calls; description_a2=a2; description_a3=a3; buffer[0]='X'; return description_result;
  }}
}};
int main() {{
  Sound sound={{nullptr, 0}};
  assert(sound.play()==0x14 && play_calls==0);
  SoundMock sm; sound.device_=&sm; play_result=0x5a;
  assert(sound.play()==0x5a && play_calls==1);
  sound.device_=nullptr; sound.set_loop_state(0x12345678L);
  assert(sound.loop_flag_30_==0x12345678L && description_calls==0);
  sound.device_=&sm; sound.set_loop_state(-17);
  assert(sound.loop_flag_30_==-17 && loop_argument==-17 && description_calls==1);
  Wave wave={{nullptr}}; char text[2]={{'?', '\\0'}};
  assert(wave.get_device_description(text, 4, 1)==1 && text[0]=='\\0');
  WaveMock wm; wave.device_=&wm; description_result=0x37;
  assert(wave.get_device_description(text, 9, 1)==0x37 && text[0]=='X');
  assert(description_a2==9 && description_a3==1 && description_calls==2);
}}
'''
    source = tmp_path / "provider_calls.cpp"
    binary = tmp_path / "provider_calls"
    source.write_text(harness)
    subprocess.run([compiler, "-std=c++11", str(source), "-o", str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
