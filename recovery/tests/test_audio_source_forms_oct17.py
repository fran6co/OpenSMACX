"""Actual-source host controls for the recovered VoiceRx init flow.

These check bounded factory/provider/attach traces. They do not establish the
target ABI, DLL implementation, native code match, or whole-owner acceptance.
"""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SIGNATURE = "int VoiceRx::init(unsigned long size, void *context, unsigned long flags)"


def extract_function(source: str, signature: str = SIGNATURE) -> str:
    assert source.count(signature) == 1, signature
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        depth += (source[index] == "{") - (source[index] == "}")
        if depth == 0:
            return source[start:index + 1]
    raise AssertionError("unterminated " + signature)


HARNESS = r"""
#include <cassert>

class VoiceRxDeviceInterface {
public:
    virtual int init(unsigned long, void *, unsigned long) = 0;
};

static int WaveDeviceReleaseGuard;
typedef int (*CreateSoundProc)(void **, void *, int);
typedef int (*DeleteSoundProc)(void *);
static CreateSoundProc create_slot;
static DeleteSoundProc delete_slot;
static int factory_calls;
static int factory_result;
static void *factory_device;
static int delete_calls;
static void *deleted_device;
static int provider_calls;
static int provider_result;
static unsigned long provider_size;
static void *provider_context;
static unsigned long provider_flags;
static int attach_calls;

static int fake_create(void **slot, void *name, int kind) {
    ++factory_calls;
    assert(name == 0);
    assert(kind == 6);
    if (factory_result != 0) return factory_result;
    *slot = factory_device;
    return 0;
}
static int fake_delete(void *device) {
    ++delete_calls;
    deleted_device = device;
    return 0;
}
static CreateSoundProc SoundXOrdinal1CreateSound() { return create_slot; }
static DeleteSoundProc SoundXOrdinal2DeleteSound() { return delete_slot; }

class FakeProvider : public VoiceRxDeviceInterface {
public:
    virtual int init(unsigned long size, void *context, unsigned long flags) {
        ++provider_calls;
        provider_size = size;
        provider_context = context;
        provider_flags = flags;
        return provider_result;
    }
};

class VoiceRx {
public:
    void *device_;
    void attach() { ++attach_calls; }
    int init(unsigned long, void *, unsigned long);
};

"""


MAIN = r"""
static FakeProvider provider;
static int dummy_context;

static void reset_trace() {
    factory_calls = 0;
    factory_result = 0;
    factory_device = &provider;
    delete_calls = 0;
    deleted_device = 0;
    provider_calls = 0;
    provider_result = 0;
    provider_size = 0;
    provider_context = 0;
    provider_flags = 0;
    attach_calls = 0;
    create_slot = &fake_create;
    delete_slot = &fake_delete;
}

static int run(VoiceRx &rx, unsigned long size, void *context, unsigned long flags) {
    return rx.init(size, context, flags);
}

int main() {
    // Existing provider: no factory call, success preserves the device and attaches.
    reset_trace();
    WaveDeviceReleaseGuard = 0;
    {
        VoiceRx value;
        value.device_ = &provider;
        provider_result = 0;
        assert(run(value, 0x1234UL, &dummy_context, 0x55UL) == 0);
        assert(factory_calls == 0 && provider_calls == 1 && attach_calls == 1);
        assert(provider_size == 0x1234UL && provider_context == &dummy_context);
        assert(provider_flags == 0x55UL && value.device_ == &provider);
        assert(delete_calls == 0);
    }

    // No provider and disabled release guard returns 1 without any side effect.
    reset_trace();
    WaveDeviceReleaseGuard = 0;
    {
        VoiceRx value;
        value.device_ = 0;
        assert(run(value, 1, &dummy_context, 2) == 1);
        assert(factory_calls == 0 && provider_calls == 0 && attach_calls == 0);
        assert(delete_calls == 0 && value.device_ == 0);
    }

    // Factory error is returned verbatim and skips provider init and attachment.
    reset_trace();
    WaveDeviceReleaseGuard = 1;
    factory_result = 23;
    {
        VoiceRx value;
        value.device_ = 0;
        assert(run(value, 1, &dummy_context, 2) == 23);
        assert(factory_calls == 1 && provider_calls == 0 && attach_calls == 0);
        assert(delete_calls == 0 && value.device_ == 0);
    }

    // Successful factory call which leaves the slot null returns zero, no attach.
    reset_trace();
    WaveDeviceReleaseGuard = 1;
    factory_device = 0;
    {
        VoiceRx value;
        value.device_ = 0;
        assert(run(value, 1, &dummy_context, 2) == 0);
        assert(factory_calls == 1 && provider_calls == 0 && attach_calls == 0);
        assert(value.device_ == 0 && delete_calls == 0);
    }

    // Factory success then provider success keeps the new pointer and attaches.
    reset_trace();
    WaveDeviceReleaseGuard = 1;
    {
        VoiceRx value;
        value.device_ = 0;
        assert(run(value, 7, &dummy_context, 9) == 0);
        assert(factory_calls == 1 && provider_calls == 1 && attach_calls == 1);
        assert(value.device_ == &provider && delete_calls == 0);
        assert(provider_size == 7 && provider_context == &dummy_context);
        assert(provider_flags == 9);
    }

    // Provider failure deletes only when guarded, clears device, and still attaches.
    reset_trace();
    WaveDeviceReleaseGuard = 1;
    provider_result = 37;
    {
        VoiceRx value;
        value.device_ = 0;
        assert(run(value, 3, &dummy_context, 4) == 37);
        assert(factory_calls == 1 && provider_calls == 1 && attach_calls == 1);
        assert(delete_calls == 1 && deleted_device == &provider && value.device_ == 0);
    }

    // A preexisting provider's failure follows the same cleanup/attach order.
    reset_trace();
    WaveDeviceReleaseGuard = 1;
    provider_result = 41;
    {
        VoiceRx value;
        value.device_ = &provider;
        assert(run(value, 5, &dummy_context, 6) == 41);
        assert(factory_calls == 0 && provider_calls == 1 && attach_calls == 1);
        assert(delete_calls == 1 && deleted_device == &provider && value.device_ == 0);
    }

    // The historical unguarded failure case still clears and attaches, but does not delete.
    reset_trace();
    WaveDeviceReleaseGuard = 0;
    provider_result = 43;
    {
        VoiceRx value;
        value.device_ = &provider;
        assert(run(value, 8, &dummy_context, 10) == 43);
        assert(factory_calls == 0 && provider_calls == 1 && attach_calls == 1);
        assert(delete_calls == 0 && value.device_ == 0);
    }
    return 0;
}
"""


def source_body() -> str:
    return extract_function((ROOT / "src/voice.cpp").read_text(encoding="utf-8"))


def compile_and_run(compiler: str, body: str, tmp_path: Path, label: str) -> subprocess.CompletedProcess[str]:
    harness = tmp_path / f"voice_rx_{label}.cpp"
    binary = tmp_path / f"voice_rx_{label}"
    harness.write_text(HARNESS + body + MAIN, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(harness), "-o", str(binary)],
        capture_output=True,
        text=True,
        timeout=60,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_voice_rx_init_actual_source_trace(compiler_name: str, tmp_path: Path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    result = compile_and_run(compiler, source_body(), tmp_path, "actual")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize(
    "damage,expected_failure",
    [
        ("attach_after_failure", "failed provider attempt must still attach"),
        ("wrong_factory_guard", "disabled guard must return before factory"),
        ("omit_device_reset", "failed provider attempt must clear device"),
        ("unguarded_delete", "disabled guard must suppress provider deletion"),
        ("wrong_result", "factory failure result must be preserved"),
    ],
)
def test_voice_rx_init_mutation_controls(
    compiler_name: str, damage: str, expected_failure: str, tmp_path: Path
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    body = source_body()
    if damage == "attach_after_failure":
        old, new = "    attach();\n    return result;", "    if (result == 0) attach();\n    return result;"
    elif damage == "wrong_factory_guard":
        old, new = "if (WaveDeviceReleaseGuard == 0)", "if (WaveDeviceReleaseGuard != 0)"
    elif damage == "omit_device_reset":
        old, new = "        device_ = 0;", "        (void)0;"
    elif damage == "unguarded_delete":
        old = "        if (WaveDeviceReleaseGuard != 0) {\n            SoundXOrdinal2DeleteSound()(device_);\n        }"
        new = "        SoundXOrdinal2DeleteSound()(device_);"
    else:
        old, new = "    attach();\n    return result;", "    attach();\n    return 0;"
    assert body.count(old) == 1
    damaged = body.replace(old, new, 1)
    result = compile_and_run(compiler, damaged, tmp_path, damage)
    assert result.returncode != 0, (
        f"mutation {damage} escaped the actual-source behavioral controls; expected "
        f"{expected_failure}"
    )
