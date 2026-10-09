"""Host controls for the actual SoundX ordinal accessors and loader lifetime."""
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
COMPILERS = [path for name in ("g++", "clang++")
             if (path := shutil.which(name)) is not None]


def definition(source: str, signature: str) -> str:
    start = source.find(signature)
    if start < 0 or source.find(signature, start + len(signature)) >= 0:
        raise AssertionError(f"expected one source definition: {signature}")
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        depth += (source[index] == "{") - (source[index] == "}")
        if depth == 0:
            return source[start:index + 1]
    raise AssertionError(f"unterminated definition: {signature}")


PRELUDE = r"""
#include <cassert>
#include <cstdint>
#include <cstring>
#define __cdecl
typedef void *HMODULE;
typedef void *SoundDllProcsTable[11];
static SoundDllProcsTable SoundDllProcs;
static HMODULE SoundDllModule;
static int WaveDeviceReleaseGuard;
static int missing_ordinal;
static int free_mode;
static int free_calls;
static int warning_calls;
static int ordinal_calls[12];
static int lookup_order[12];
static int lookup_count;
static void *provider_slot;
static const char *filename_seen;
static int mode_seen;
static unsigned long arg_one, arg_two;
static void *module_token;
static int ordinal1(void **slot, const char *name, int mode) {
    ++ordinal_calls[1]; provider_slot = slot; filename_seen = name; mode_seen = mode;
    *slot = &module_token; return 101;
}
static int ordinal2(void *value) { assert(value == &module_token); ++ordinal_calls[2]; return 102; }
static int ordinal3(unsigned long a, unsigned long b) {
    arg_one = a; arg_two = b; ++ordinal_calls[3]; return 103;
}
static int ordinal4() { ++ordinal_calls[4]; return 104; }
static int ordinal5(void **slot, unsigned long kind) {
    provider_slot = slot; arg_one = kind; ++ordinal_calls[5]; *slot = &module_token; return 105;
}
static int ordinal6() { ++ordinal_calls[6]; return 106; }
static int ordinal7(void **slot, unsigned long kind) {
    provider_slot = slot; arg_one = kind; ++ordinal_calls[7]; *slot = &module_token; return 107;
}
static int ordinal8() { ++ordinal_calls[8]; return 108; }
static int ordinal9(void **slot, unsigned long kind) {
    provider_slot = slot; arg_one = kind; ++ordinal_calls[9]; *slot = &module_token; return 109;
}
static int ordinal10() { ++ordinal_calls[10]; return 110; }
static unsigned int ordinal11() { ++ordinal_calls[11]; return 0x0101; }
static void *proc_for_ordinal(int ordinal) {
    switch (ordinal) {
    case 1: return reinterpret_cast<void *>(&ordinal1);
    case 2: return reinterpret_cast<void *>(&ordinal2);
    case 3: return reinterpret_cast<void *>(&ordinal3);
    case 4: return reinterpret_cast<void *>(&ordinal4);
    case 5: return reinterpret_cast<void *>(&ordinal5);
    case 6: return reinterpret_cast<void *>(&ordinal6);
    case 7: return reinterpret_cast<void *>(&ordinal7);
    case 8: return reinterpret_cast<void *>(&ordinal8);
    case 9: return reinterpret_cast<void *>(&ordinal9);
    case 10: return reinterpret_cast<void *>(&ordinal10);
    case 11: return reinterpret_cast<void *>(&ordinal11);
    default: return 0;
    }
}
static HMODULE LoadLibraryA(const char *) { return &module_token; }
#define MAKEINTRESOURCEA(value) reinterpret_cast<const char *>(static_cast<uintptr_t>(value))
static void *GetProcAddress(HMODULE, const char *name) {
    const int ordinal = static_cast<int>(reinterpret_cast<uintptr_t>(name));
    lookup_order[lookup_count++] = ordinal;
    return ordinal == missing_ordinal ? 0 : proc_for_ordinal(ordinal);
}
static int FreeLibrary(HMODULE module) {
    assert(module == &module_token);
    ++free_calls;
    if (free_mode == 1) {
        assert(WaveDeviceReleaseGuard == 0);
        assert(SoundDllModule == &module_token);
        for (int i = 0; i != 4; ++i) assert(SoundDllProcs[i] != 0);
        assert(SoundDllProcs[4] == 0);
        for (int i = 5; i != 11; ++i) assert(SoundDllProcs[i] == reinterpret_cast<void *>(1));
    } else if (free_mode == 2) {
        assert(WaveDeviceReleaseGuard == 0);
        assert(SoundDllModule == &module_token);
        for (int i = 0; i != 11; ++i) assert(SoundDllProcs[i] != 0);
    }
    return 1;
}
extern "C" int __cdecl sound_header_version() { return 1; }
extern "C" void __cdecl sound_version_warning(const char *, const char *) { ++warning_calls; }
"""


DIRECT_CALLS = r"""
static void exercise_all_roles() {
    for (int i = 0; i != 11; ++i) SoundDllProcs[i] = proc_for_ordinal(i + 1);
    void *slot = 0;
    const char name[] = "score.wav";
    assert(SoundXOrdinal1CreateSound()(&slot, name, 3) == 101);
    assert(provider_slot == &slot && slot == &module_token && filename_seen == name && mode_seen == 3);
    assert(SoundXOrdinal2DeleteSound()(slot) == 102);
    assert(SoundXOrdinal3InitTimer()(0x1234UL, 0x5678UL) == 103);
    assert(arg_one == 0x1234UL && arg_two == 0x5678UL);
    assert(SoundXOrdinal4ReleaseSound()() == 104);
    assert(SoundXOrdinal5WaveDeviceCreate()(&slot, 0x15UL) == 105 && provider_slot == &slot && arg_one == 0x15UL);
    assert(SoundXOrdinal6WaveDeviceDelete()() == 106);
    assert(SoundXOrdinal7MidiDeviceCreate()(&slot, 0x27UL) == 107 && provider_slot == &slot && arg_one == 0x27UL);
    assert(SoundXOrdinal8MidiDeviceDelete()() == 108);
    assert(SoundXOrdinal9WaveInDeviceCreate()(&slot, 0x39UL) == 109 && provider_slot == &slot && arg_one == 0x39UL);
    assert(SoundXOrdinal10WaveInDeviceDelete()() == 110);
    assert(SoundXOrdinal11Version()() == 0x0101);
    for (int i = 1; i != 12; ++i) assert(ordinal_calls[i] == 1);

    // The accessors reload the live void* cell rather than retaining a
    // function-pointer reference or a previously returned callback.
    SoundDllProcs[0] = reinterpret_cast<void *>(&ordinal1_rebound);
    assert(SoundXOrdinal1CreateSound()(&slot, name, 9) == 201);
    assert(ordinal1_rebound_calls == 1 && mode_seen == 9);
}
"""

# Appended separately so the rebind callback can share the ordinal-1 state.
REBIND = r"""
static int ordinal1_rebound_calls;
static int ordinal1_rebound(void **slot, const char *name, int mode) {
    ++ordinal1_rebound_calls; provider_slot = slot; filename_seen = name;
    mode_seen = mode; *slot = &module_token; return 201;
}
"""


LOADER_TESTS = r"""
static void loader_failure_order() {
    SoundDllModule = 0; WaveDeviceReleaseGuard = 1; missing_ordinal = 5;
    lookup_count = free_calls = 0; free_mode = 1;
    for (int i = 0; i != 11; ++i) SoundDllProcs[i] = reinterpret_cast<void *>(1);
    assert(load_sound_dll() == 1);
    assert(lookup_count == 5);
    for (int i = 0; i != 5; ++i) assert(lookup_order[i] == i + 1);
    assert(free_calls == 1 && SoundDllModule == 0 && WaveDeviceReleaseGuard == 0);
    for (int i = 0; i != 11; ++i) assert(SoundDllProcs[i] == 0);
}
static void loader_success_and_unload_order() {
    SoundDllModule = 0; WaveDeviceReleaseGuard = 1; missing_ordinal = 0;
    lookup_count = free_calls = warning_calls = 0; free_mode = 2;
    for (int i = 0; i != 11; ++i) SoundDllProcs[i] = 0;
    assert(load_sound_dll() == 0);
    assert(lookup_count == 11 && ordinal_calls[11] == 2); // direct test plus loader version check
    for (int i = 0; i != 11; ++i) assert(lookup_order[i] == i + 1);
    assert(warning_calls == 0);
    unload_sound_dll();
    assert(free_calls == 1 && SoundDllModule == 0 && WaveDeviceReleaseGuard == 0);
    for (int i = 0; i != 11; ++i) assert(SoundDllProcs[i] == 0);
}
int main() {
    exercise_all_roles();
    loader_failure_order();
    loader_success_and_unload_order();
    return 0;
}
"""


def current_source() -> tuple[str, str]:
    sound_h = (ROOT / "src/sound.h").read_text(encoding="utf-8")
    loader = (ROOT / "src/sound_loader.cpp").read_text(encoding="utf-8")
    typedef_start = sound_h.index("typedef int (__cdecl *SoundXCreateSoundProc)")
    accessors_start = sound_h.index("inline SoundXCreateSoundProc SoundXOrdinal1CreateSound()")
    types = sound_h[typedef_start:accessors_start]
    accessors = "\n".join(definition(sound_h, f"inline {kind} {name}()") for kind, name in (
        ("SoundXCreateSoundProc", "SoundXOrdinal1CreateSound"),
        ("SoundXDeleteSoundProc", "SoundXOrdinal2DeleteSound"),
        ("SoundXInitTimerProc", "SoundXOrdinal3InitTimer"),
        ("SoundXReleaseSoundProc", "SoundXOrdinal4ReleaseSound"),
        ("SoundXWaveDeviceCreateProc", "SoundXOrdinal5WaveDeviceCreate"),
        ("SoundXWaveDeviceDeleteProc", "SoundXOrdinal6WaveDeviceDelete"),
        ("SoundXMidiDeviceCreateProc", "SoundXOrdinal7MidiDeviceCreate"),
        ("SoundXMidiDeviceDeleteProc", "SoundXOrdinal8MidiDeviceDelete"),
        ("SoundXWaveInDeviceCreateProc", "SoundXOrdinal9WaveInDeviceCreate"),
        ("SoundXWaveInDeviceDeleteProc", "SoundXOrdinal10WaveInDeviceDelete"),
        ("SoundXVersionProc", "SoundXOrdinal11Version"),
    ))
    table = sound_h[sound_h.index("typedef void *SoundDllProcsTable[11];"):
                    sound_h.index("extern HMODULE SoundDllModule;")]
    load = definition(loader, "int __cdecl load_sound_dll()")
    unload = definition(loader, "void __cdecl unload_sound_dll()")
    # The source compares a 32-bit pointer cast as part of its x86 loop. Keep
    # the extracted algorithm but express that same ordered-range test portably.
    load = load.replace("(int)slot < (int)procs_end", "slot < procs_end")
    assert "(int)slot < (int)procs_end" not in load
    return types + table + accessors, load + "\n" + unload


@pytest.mark.parametrize("compiler", COMPILERS or [None],
                         ids=lambda value: Path(value).name if value else "no-host-cxx")
def test_actual_ordinal_accessors_and_loader_order(tmp_path: Path, compiler: str | None) -> None:
    if compiler is None:
        pytest.skip("host GCC or Clang required")
    accessors, loader = current_source()
    source = (PRELUDE + accessors + loader + REBIND + DIRECT_CALLS + LOADER_TESTS)
    path = tmp_path / "actual.cpp"
    binary = tmp_path / "actual"
    path.write_text(source)
    built = subprocess.run([compiler, "-std=c++11", "-Wno-pointer-to-int-cast",
                            str(path), "-o", str(binary)], capture_output=True,
                           text=True, timeout=30)
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stderr

    wrong = accessors.replace("SoundDllProcs[4]", "SoundDllProcs[6]", 1)
    assert wrong != accessors
    mutant = PRELUDE + wrong + loader + REBIND + DIRECT_CALLS + LOADER_TESTS
    path.write_text(mutant)
    built = subprocess.run([compiler, "-std=c++11", "-Wno-pointer-to-int-cast",
                            str(path), "-o", str(binary)], capture_output=True,
                           text=True, timeout=30)
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    assert result.returncode != 0
