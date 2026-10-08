"""Host regression for the original pointer-valued Midi::load busy result.

The complete product method is extracted verbatim from src/midi.cpp. Host
stubs model external state only; mmap addresses exercise 32-bit pointer-bit
preservation without claiming the original target ABI.
"""
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest

from test_sprite_list_lifecycle_actual_source import extract_definition, repo_root


ROOT = repo_root()


def harness(method: str) -> str:
    return r'''#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <sys/mman.h>
#define __cdecl
typedef int(__cdecl func_wave_device_create)(void **, const char *, int);
void *SoundDllProcs[11] = {};
int WaveDeviceReleaseGuard = 0;
static int factory_result = 0;
static bool factory_sets_device = true;
static void *created_device = reinterpret_cast<void *>(0x1234);
static int factory_calls = 0;
static void **factory_slot = 0;
static const char *factory_name = 0;
static int factory_mode = 0;
static int base_calls = 0;
static const char *base_name = 0;
static int base_result = 0;
static bool check(const char *label, bool condition) {
    if (!condition) std::fprintf(stderr, "FAIL: %s\n", label);
    return condition;
}
static int factory(void **slot, const char *name, int mode) {
    ++factory_calls; factory_slot = slot; factory_name = name; factory_mode = mode;
    if (factory_sets_device) *slot = created_device;
    return factory_result;
}
class Sound {
protected:
    void *device_;
public:
    Sound() : device_(0) {}
    int load(const char *name) { ++base_calls; base_name = name; return base_result; }
};
class Midi : public Sound {
public:
    int load(const char *name);
    void set_device(void *value) { device_ = value; }
    void *device() const { return device_; }
};
''' + method + r'''
static void reset() {
    factory_result = 0; factory_sets_device = true;
    created_device = reinterpret_cast<void *>(0x1234);
    factory_calls = base_calls = 0; factory_slot = 0;
    factory_name = base_name = 0; factory_mode = 0;
    factory_result = 0; base_result = 29;
    WaveDeviceReleaseGuard = 0;
    for (int i = 0; i != 11; ++i) SoundDllProcs[i] = 0;
    SoundDllProcs[0] = reinterpret_cast<void *>(&factory);
}
int main() {
    const size_t page = 4096;
    char *low_filename = static_cast<char *>(mmap(0, page,
        PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0));
    if (low_filename == MAP_FAILED) return 90;
    std::strcpy(low_filename, "score.mid");
    assert(reinterpret_cast<uintptr_t>(low_filename) <= 0x7fffffffu);
    assert(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(low_filename)) != 0u);
    assert(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(low_filename)) != 1u);

    char *high_filename = 0;
#ifdef MAP_FIXED_NOREPLACE
    const uintptr_t candidates[] = { 0x81000000u, 0x90000000u, 0xa0000000u,
        0xb0000000u, 0xc0000000u, 0xd0000000u, 0xe0000000u };
    for (size_t i = 0; i != sizeof(candidates) / sizeof(candidates[0]); ++i) {
        void *mapped = mmap(reinterpret_cast<void *>(candidates[i]), page,
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
        if (mapped != MAP_FAILED) {
            high_filename = static_cast<char *>(mapped);
            break;
        }
    }
    if (high_filename) std::strcpy(high_filename, "high.mid");
#endif
    Midi midi;
    bool ok = true;

    reset();
    midi.set_device(reinterpret_cast<void *>(0x5678));
    const int low_pointer_status = static_cast<int>(reinterpret_cast<uintptr_t>(low_filename));
    ok &= check("busy pointer status (low address)", midi.load(low_filename) == low_pointer_status);
    ok &= check("busy path skips factory and base load", factory_calls == 0 && base_calls == 0);

    if (high_filename) {
        const uintptr_t high_bits = reinterpret_cast<uintptr_t>(high_filename);
        assert(high_bits <= 0xffffffffu && (high_bits & 0x80000000u) != 0u);
        assert(static_cast<uint32_t>(high_bits) != 0u && static_cast<uint32_t>(high_bits) != 1u);
        reset();
        midi.set_device(reinterpret_cast<void *>(0x5678));
        const int high_pointer_status = static_cast<int>(high_bits);
        ok &= check("busy pointer status (signed high-bit address)", midi.load(high_filename) == high_pointer_status);
        ok &= check("high-bit pointer status is signed negative", midi.load(high_filename) < 0);
        ok &= check("high busy path skips factory and base load", factory_calls == 0 && base_calls == 0);
    }

    reset();
    midi.set_device(reinterpret_cast<void *>(0x5678));
    ok &= check("null busy path delegates to base", midi.load(0) == base_result);
    ok &= check("null base call arguments", factory_calls == 0 && base_calls == 1 && base_name == 0);

    reset();
    midi.set_device(0);
    ok &= check("dead creation guard returns one", midi.load(low_filename) == 1);
    ok &= check("dead guard skips factory and base", factory_calls == 0 && base_calls == 0);

    reset();
    WaveDeviceReleaseGuard = 1;
    factory_result = 17;
    midi.set_device(0);
    ok &= check("creation error preserved", midi.load(low_filename) == 17);
    ok &= check("creation receives slot/name/mode", factory_calls == 1 && factory_slot != 0 && factory_name == low_filename && factory_mode == 2 && base_calls == 0);

    reset();
    WaveDeviceReleaseGuard = 1;
    factory_result = 0;
    midi.set_device(0);
    ok &= check("successful creation delegates to base", midi.load(low_filename) == base_result);
    ok &= check("creation stores wrapped device", factory_calls == 1 && factory_slot != 0 && factory_name == low_filename && factory_mode == 2 && midi.device() == created_device);
    ok &= check("base receives original filename", base_calls == 1 && base_name == low_filename);
    if (high_filename) munmap(high_filename, page);
    munmap(low_filename, page);
    return ok ? 0 : 1;
}
'''


def compile_and_run(compiler: str, body: str, tmp_path: Path, label: str,
                    sanitizer: bool = False):
    cpp = tmp_path / f"{label}.cpp"
    exe = tmp_path / label
    cpp.write_text(harness(body))
    command = [compiler, "-D_GNU_SOURCE", "-std=c++11", "-Wall", "-Wextra", "-Werror"]
    if sanitizer:
        command += ["-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
    command += [str(cpp), "-o", str(exe)]
    built = subprocess.run(command, capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10,
                         env={**__import__("os").environ, "ASAN_OPTIONS": "detect_leaks=0"})
    return ran


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("sanitizer", (False, True))
def test_pointer_status_and_adjacent_load_paths(
        compiler_name: str, sanitizer: bool, tmp_path: Path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is not installed")
    source = (ROOT / "src/midi.cpp").read_text()
    method = extract_definition(source, "int Midi::load(const char *a1)")
    assert "// ORIGINAL: 0x004C7D80" in source
    ran = compile_and_run(compiler, method, tmp_path,
                          f"midi_load_{compiler_name}_{int(sanitizer)}", sanitizer)
    assert ran.returncode == 0, ran.stdout + ran.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_result_one_mutation_is_caught_at_pointer_oracle(compiler_name: str, tmp_path: Path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is not installed")
    source = (ROOT / "src/midi.cpp").read_text()
    method = extract_definition(source, "int Midi::load(const char *a1)")
    faithful_branch = "result = static_cast<int>(reinterpret_cast<uintptr_t>(a1));"
    assert faithful_branch in method, "apply the proposed faithful source change before running mutation control"
    mutant = method.replace(faithful_branch, "result = 1;", 1)
    assert mutant != method
    ran = compile_and_run(compiler, mutant, tmp_path, f"midi_load_mutant_{compiler_name}")
    assert ran.returncode == 1, ran.stdout + ran.stderr
    assert "FAIL: busy pointer status (low address)" in ran.stderr
