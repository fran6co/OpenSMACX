"""Actual-body client controls; the opt-in VC6 probe checks the provider ABI."""
from __future__ import annotations

import os
import re
import shutil
import subprocess
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]


def body(source: str, signature: str) -> str:
    pattern = re.escape(signature).replace(r"\ ", r"\s+")
    match = re.search(pattern, source)
    assert match is not None, f"missing definition {signature}"
    opening = source.index("{", match.end())
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:index]
    raise AssertionError(f"unterminated function {signature}")


VIDEO_METHODS = (
    "VideoSoundState::~VideoSoundState()",
    "int VideoSoundState::release()",
    "int VideoSoundState::configure_streams(const void *audio_stream, const void *video_stream, const void *audio_format, int flags)",
    "int VideoSoundState::stop()",
    "int VideoSoundState::set_target_frame(int frame)",
    "void *VideoSoundState::acquire_audio_buffer(int bytes)",
    "int VideoSoundState::submit_audio_buffer(void *record)",
    "int VideoSoundState::current_frame()",
    "int VideoSoundState::frame_status()",
)


def test_source_uses_shared_prefix_and_keeps_video_extension_slots():
    header = (ROOT / "src/video.h").read_text()
    abi = (ROOT / "src/soundx_sound_abi.h").read_text()
    video = header.split("class VideoSoundDevice : public SoundDeviceInterface {", 1)[1]
    video = video.split("\n};", 1)[0]
    methods = re.findall(r"virtual\s+[^;]+;\s*//\s*slot\s+(\d+)", video)
    assert methods == ["33", "34", "35", "36", "37", "38"]
    assert "class SoundDeviceInterface {" in abi
    assert "destroy_device" not in header and "can_release" not in header
    assert "class VideoSoundDevice : public SoundDeviceInterface" in header
    source = (ROOT / "src/video.cpp").read_text()
    destructor = body(source, "VideoSoundState::~VideoSoundState()")
    release = body(source, "int VideoSoundState::release()")
    assert destructor.index("delete device;") < destructor.index("device_ = 0;")
    assert "device->destroy_device" not in destructor
    assert "->is_playing()" in release
    assert "->can_release()" not in release
    assert release.index("->is_playing()") < release.index("static_cast<VideoSoundDevice *>(device_)->release()")


def host_harness(source: str, mutation: str | None = None) -> str:
    bodies = {signature: body(source, signature) for signature in VIDEO_METHODS}
    if mutation == "release_keeps_stale_pointer":
        old = '''  if (device_ != 0) {
    if (static_cast<VideoSoundDevice *>(device_)->is_playing() == 0) {'''
        new = '''  if (device_ != 0) {
    VideoSoundDevice *const queried_device = static_cast<VideoSoundDevice *>(device_);
    if (queried_device->is_playing() == 0) {'''
        assert old in bodies["int VideoSoundState::release()"]
        bodies["int VideoSoundState::release()"] = bodies["int VideoSoundState::release()"].replace(old, new, 1)
        old = "static_cast<VideoSoundDevice *>(device_)->release();"
        assert old in bodies["int VideoSoundState::release()"]
        bodies["int VideoSoundState::release()"] = bodies["int VideoSoundState::release()"].replace(
            old, "queried_device->release();", 1
        )
    elif mutation == "clear_before_delete":
        old = "    delete device;\n    device_ = 0;"
        assert old in bodies["VideoSoundState::~VideoSoundState()"]
        bodies["VideoSoundState::~VideoSoundState()"] = bodies["VideoSoundState::~VideoSoundState()"].replace(
            old, "    device_ = 0;\n    delete device;", 1
        )

    return r'''#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>
static std::vector<std::string> events;
static int detach_calls;
struct Sound;
class SoundDeviceInterface {
// This host-only subset executes client bodies; it does not model ABI slots.
public:
  virtual ~SoundDeviceInterface();
  virtual int stop() = 0;
  virtual int release() = 0;
  virtual int is_playing() = 0;
  virtual int configure_streams(const void *, const void *, const void *, int) = 0;
  virtual int set_target_frame(int) = 0;
  virtual void *acquire_audio_buffer(int) = 0;
  virtual int submit_audio_buffer(void *) = 0;
  virtual int current_frame() = 0;
  virtual int frame_status() = 0;
};
struct VideoSoundDevice : SoundDeviceInterface {};
struct Sound {
  void *device_;
  Sound() : device_(0) {}
  ~Sound();
  int detach() { ++detach_calls; events.push_back("detach"); return 0; }
  int attach() { events.push_back("attach"); return 0; }
};
struct VideoSoundState : Sound {
  ~VideoSoundState(); int release(); int stop();
  int configure_streams(const void *, const void *, const void *, int);
  int set_target_frame(int); void *acquire_audio_buffer(int);
  int submit_audio_buffer(void *); int current_frame(); int frame_status();
};
static VideoSoundState *owner;
static VideoSoundDevice *replacement;
static int query_result, stop_calls, target_calls, target_value, submit_result;
static int config_calls, config_result, config_flags;
static const void *config_audio, *config_video, *config_format;
static int create_result;
static void sound_delete(void *p) { delete static_cast<VideoSoundDevice *>(p); }
static void (*SoundDeleteTarget)(void *);
static void (*SoundXOrdinal2DeleteSound())(void *) { return SoundDeleteTarget; }
static int create_sound(void **device, const char *, int) {
  *device = 0;
  return create_result;
}
static int (*SoundXOrdinal1CreateSound())(void **, const char *, int) { return create_sound; }
static int WaveDeviceReleaseGuard;
''' + ''.join(
        "\n" + signature + " {" + bodies[signature] + "}\n" for signature in VIDEO_METHODS
    ) + r'''
SoundDeviceInterface::~SoundDeviceInterface() { events.push_back("abi-dtor"); }
Sound::~Sound() {
  assert(device_ == 0);
  events.push_back("sound-dtor");
}
struct Probe : VideoSoundDevice {
  int local_release_calls = 0, release_result = 0, frame = 17, status = 23;
  void *buffer = reinterpret_cast<void *>(0x1234);
  int release() override { ++local_release_calls; events.push_back("provider-release"); return release_result; }
  int is_playing() override {
    events.push_back("query");
    if (replacement) owner->device_ = replacement;
    return query_result;
  }
  int stop() override { ++stop_calls; return 91; }
  int configure_streams(const void *audio, const void *video, const void *format,
                        int flags) override {
    ++config_calls; config_audio=audio; config_video=video; config_format=format;
    config_flags=flags; return config_result;
  }
  int set_target_frame(int value) override { ++target_calls; target_value=value; return 42; }
  void *acquire_audio_buffer(int) override { return buffer; }
  int submit_audio_buffer(void *) override { return submit_result; }
  int current_frame() override { return frame; }
  int frame_status() override { return status; }
  ~Probe() override {
    assert(owner == 0 || owner->device_ == this);
    events.push_back("provider-dtor");
  }
};
static void reset(VideoSoundState &state) {
  owner=&state; replacement=0; query_result=1; detach_calls=stop_calls=0;
  target_calls=target_value=submit_result=config_calls=config_flags=0;
  config_audio=config_video=config_format=0; config_result=create_result=0;
  WaveDeviceReleaseGuard=0; events.clear(); SoundDeleteTarget=sound_delete;
}
int main() {
  {
    VideoSoundState state; reset(state);
    Probe *first=new Probe, *second=new Probe; replacement=second;
    state.device_=first;
    assert(state.release()==0);
    assert(first->local_release_calls==0 && second->local_release_calls==1);
    assert(state.device_==0 && events[0]=="query" && events[1]=="provider-release");
    assert(events.back()=="detach" && detach_calls==1);
    // release clears the reentrant replacement before normal owner teardown.
    owner=0; delete first; delete second;
  }
  {
    VideoSoundState state; reset(state);
    Probe *device=new Probe; state.device_=device; query_result=0;
    WaveDeviceReleaseGuard=1;
    assert(state.release()==0 && state.device_==0);
    assert(events.size()==4 && events[0]=="query" && events[1]=="provider-dtor");
    assert(events[2]=="abi-dtor" && events[3]=="detach" && detach_calls==1);
  }
  {
    VideoSoundState state; reset(state);
    Probe *device=new Probe; state.device_=device; query_result=0;
    assert(state.release()==0 && state.device_==0);
    assert(events.size()==2 && events[0]=="query" && events[1]=="detach");
    owner=0; delete device;
  }
  {
    VideoSoundState state; reset(state);
    assert(state.release()==0 && detach_calls==1);
    assert(events.size()==1 && events[0]=="detach");
  }
  {
    VideoSoundState *state=new VideoSoundState; reset(*state);
    Probe *device=new Probe; state->device_=device;
    delete state;
    assert(events.size() == 3);
    assert(events[0]=="provider-dtor" && events[1]=="abi-dtor");
    assert(events[2]=="sound-dtor");
  }
  {
    VideoSoundState state; reset(state);
    assert(state.stop()==0 && stop_calls==0);
    Probe device; state.device_=&device;
    assert(state.stop()==0 && stop_calls==1);
    assert(state.set_target_frame(19)==0 && target_calls==1 && target_value==19);
    assert(state.acquire_audio_buffer(32)==device.buffer);
    assert(state.current_frame()==17 && state.frame_status()==23);
    submit_result=0x57; assert(state.submit_audio_buffer(0)==0x57);
    int audio=1, video=2, format=3;
    config_result=0x46;
    assert(state.configure_streams(&audio, &video, &format, 19)==0x46);
    assert(config_calls==1 && config_audio==&audio && config_video==&video);
    assert(config_format==&format && config_flags==19 && events.back()=="attach");
    assert(state.device_==0);
    assert(state.current_frame()==-1 && state.frame_status()==0);
    assert(state.acquire_audio_buffer(32)==0);
    assert(state.submit_audio_buffer(0)==0x13);
    assert(state.set_target_frame(7)==0 && target_calls==1);
    state.device_=0;
    owner=0;
  }
}
'''


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_video_provider_bodies_preserve_reentry_lifetime_and_wrappers(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    source = (ROOT / "src/video.cpp").read_text()
    harness = tmp_path / "video_provider.cpp"
    binary = tmp_path / "video_provider"
    harness.write_text(host_harness(source))
    subprocess.run([compiler, "-std=c++11", str(harness), "-o", str(binary)], check=True)
    subprocess.run([str(binary)], check=True)


@pytest.mark.parametrize("mutation", ("release_keeps_stale_pointer", "clear_before_delete"))
def test_actual_video_provider_body_controls_reject_lifetime_damage(tmp_path, mutation):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("g++ and clang++ are unavailable")
    source = (ROOT / "src/video.cpp").read_text()
    harness = tmp_path / f"video_provider_{mutation}.cpp"
    binary = tmp_path / f"video_provider_{mutation}"
    harness.write_text(host_harness(source, mutation))
    subprocess.run([compiler, "-std=c++11", str(harness), "-o", str(binary)], check=True)
    result = subprocess.run([str(binary)], capture_output=True)
    assert result.returncode != 0, f"damaged body unexpectedly passed: {mutation}"


VC6_PROBES = (
    (2, "delete p"),
    (8, "p->stop()"),
    (14, "p->release()"),
    (23, "p->is_playing()"),
    (33, "p->configure_streams(0, 0, 0, 0)"),
    (34, "p->set_target_frame(1)"),
    (35, "p->acquire_audio_buffer(1)"),
    (36, "p->submit_audio_buffer(0)"),
    (37, "p->current_frame()"),
    (38, "p->frame_status()"),
)


def test_opt_in_vc6_probe_maps_inherited_and_video_extension_slots():
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
    path = ROOT / "src/video.cpp"
    original = path.read_bytes()
    probes = [
        f'extern "C" int video_provider_slot_{slot:02d}(VideoSoundDevice *p) '
        f'{{ {expression}; return 0; }}'
        for slot, expression in VC6_PROBES
    ]
    overlay = original.decode("utf-8") + "\n" + "\n".join(probes)
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    for flags in settings.flag_sets:
        raw = compiler.overlay(path, {"src/video.cpp": overlay.encode("utf-8")}, (), flags + settings.layout_flags)
        obj = Coff(raw)
        for slot, _ in VC6_PROBES:
            function = obj.function(f"_video_provider_slot_{slot:02d}")
            code, _, _ = obj.subject(function)
            calls = [
                operand.operands[0].mem.disp
                for operand in decoder.disasm(code, 0)
                if operand.mnemonic == "call"
                and operand.operands[0].type == X86_OP_MEM
                and operand.operands[0].mem.base
                and not operand.operands[0].mem.index
            ]
            assert calls == [slot * 4], (slot, calls)
