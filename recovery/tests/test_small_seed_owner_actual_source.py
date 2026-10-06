"""Permanent product-source controls; copy this file into recovery/tests.

There is deliberately no fallback to a seed or proposal overlay. Missing
owner declarations/bodies fail before compiling, and host fixtures compile and
execute the exact method definitions from the product files.
"""

from pathlib import Path
import re
import shutil
import subprocess
import textwrap

import pytest

from match_recovery.object_source import parse_objects
from match_recovery.source.reader import read_text


def repository_root(start):
    for candidate in (start.resolve(), *start.resolve().parents):
        if ((candidate / "CMakeLists.txt").is_file()
                and (candidate / "src/IMAGE_PIN").is_file()
                and (candidate / "recovery/WORKFLOW.md").is_file()):
            return candidate
    raise RuntimeError(f"cannot locate OpenSMACX repository above {start}")


ROOT = repository_root(Path(__file__).parent)
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
SPACE = "pe:va"


def source(name):
    return (ROOT / name).read_text(encoding="utf-8")


def body_after(text, signature):
    start = text.index(signature)
    brace = text.index("{", start)
    depth = 0
    for index in range(brace, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[brace + 1:index]
    raise AssertionError(f"unterminated body for {signature}")


def definition_after(text, signature):
    start = text.index(signature)
    brace = text.index("{", start)
    body_after(text, signature)
    depth = 0
    for index in range(brace, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[start:index + 1]
    raise AssertionError(f"unterminated definition for {signature}")


def compile_and_run(tmp_path, name, program):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host g++ is required for actual-source owner controls")
    cpp = tmp_path / f"{name}.cpp"
    binary = tmp_path / name
    cpp.write_text(program, encoding="utf-8")
    compiled = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(binary)], capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True)


def mapped_records(path):
    if not (ROOT / path).exists():
        return []
    text = source(path)
    return read_text(text, ROOT / path)


def test_installed_slot24_source_closure_and_mapping_transfer():
    sound_h = source("src/sound.h")
    voice_h = source("src/voice.h")
    wave_h = source("src/wave.h")
    effect_h = source("src/hypothesis_layouts.h")
    assert "virtual int unk_slot24();" in sound_h
    assert "virtual int unk_slot24() = 0;" in voice_h
    assert "virtual int unk_slot24();" in wave_h
    assert "class Effect : public Wave" in effect_h
    assert "void unk_slot24" not in "\n".join(
        line for line in (sound_h + voice_h + wave_h).splitlines()
        if "unk_slot24" in line)
    sound_body = body_after(source("src/sound.cpp"), "int Sound::unk_slot24()")
    assert "flags_40_" in sound_body and "? -1 : 0" in sound_body
    assert [row.address for row in mapped_records("src/sound.cpp")].count(0x00448380) == 1
    assert all(row.address != 0x00448380
               for row in mapped_records("src/field_accessors.cpp"))
    for path in ("recovery/tests/sound_lifetime_harness.cpp",
                 "recovery/tests/sound_chain_storage_harness.cpp",
                 "recovery/tests/wave_lifetime_harness.cpp"):
        assert "void Sound::unk_slot24" not in source(path)


def test_installed_sound_method_executes_and_mutation_is_detected(tmp_path):
    implementation = definition_after(source("src/sound.cpp"),
                                      "int Sound::unk_slot24()")

    def program(method):
        return textwrap.dedent(f"""\
            #include <cassert>
            typedef unsigned int uint32_t;
            struct Sound {{ uint32_t flags_40_; int unk_slot24(); }};
            {method}
            int main() {{
                Sound sound;
                sound.flags_40_ = 0; assert(sound.unk_slot24() == 0);
                sound.flags_40_ = 1; assert(sound.unk_slot24() == -1);
                sound.flags_40_ = 2; assert(sound.unk_slot24() == 0);
                sound.flags_40_ = 0xFFFFFFFFU; assert(sound.unk_slot24() == -1);
                return 0;
            }}
        """)

    actual = compile_and_run(tmp_path, "sound_slot24_actual", program(implementation))
    assert actual.returncode == 0, actual.stdout + actual.stderr
    mutated = implementation.replace("? -1 : 0", "? 1 : 0", 1)
    assert mutated != implementation
    rejected = compile_and_run(tmp_path, "sound_slot24_mutation", program(mutated))
    assert rejected.returncode != 0


def test_installed_wave_method_dispatches_and_mutation_is_detected(tmp_path):
    text = source("src/wave.cpp")
    assert '#include "voice.h"' in text
    method = definition_after(text, "int Wave::unk_slot24()")
    assert [row.address for row in mapped_records("src/wave.cpp")].count(0x00448340) == 1
    assert all(row.address != 0x00448340
               for row in mapped_records("src/recovered/00448340.cpp"))

    def program(implementation):
        return textwrap.dedent(f"""\
            #include <cassert>
            class SoundDeviceInterface {{
             public: virtual int unk_slot24() = 0;
            }};
            struct Wave {{ void *device_; int unk_slot24(); }};
            {implementation}
            class Provider : public SoundDeviceInterface {{
             public:
                int calls, result;
                Provider(int value) : calls(0), result(value) {{}}
                int unk_slot24() {{ ++calls; return result; }}
            }};
            int main() {{
                Wave wave; wave.device_ = 0;
                assert(wave.unk_slot24() == 0);
                Provider provider(-37); wave.device_ = &provider;
                assert(wave.unk_slot24() == -37 && provider.calls == 1);
                provider.result = 0x1234567;
                assert(wave.unk_slot24() == 0x1234567 && provider.calls == 2);
                return 0;
            }}
        """)

    actual = compile_and_run(tmp_path, "wave_slot24_actual", program(method))
    assert actual.returncode == 0, actual.stdout + actual.stderr
    mutated = method.replace(
        "return static_cast<SoundDeviceInterface *>(device_)->unk_slot24();",
        "return 0;", 1)
    assert mutated != method
    rejected = compile_and_run(tmp_path, "wave_slot24_mutation", program(mutated))
    assert rejected.returncode != 0


def test_installed_win_method_routes_and_mutation_is_detected(tmp_path):
    header = source("src/win.h")
    text = source("src/win_reparent.cpp")
    assert "friend int __stdcall win_reparent_window(HWND, int);" in header
    assert "int __stdcall win_reparent_window(HWND hwnd, int unused);" in header
    method = definition_after(
        text, "int __stdcall win_reparent_window(HWND hwnd, int unused)")
    operations = ["GetWindowLongA(hwnd, GWL_USERDATA)", "GetParent(hwnd)",
                  "WinReparentMatchParent", "SetParent(hwnd, WinReparentTargetParent)",
                  "return 1;"]
    positions = [method.index(operation) for operation in operations]
    assert positions == sorted(positions)
    objects = parse_objects(text, "src/win_reparent.cpp", PIN,
                            address_space=SPACE)
    assert {(row.origin.address, row.span) for row in objects} == {
        (0x009B7B40, None), (0x009B7B44, None)}
    assert not re.search(r"RECOVERY_OBJECT: v2", text)
    assert [row.address for row in mapped_records("src/win_reparent.cpp")].count(0x005F4D50) == 1
    assert "src/win_reparent.cpp" in source("CMakeLists.txt")

    def program(implementation):
        return textwrap.dedent(f"""\
            #include <cassert>
            typedef unsigned char uint8_t;
            typedef unsigned int uint32_t;
            typedef int HWND;
            #define __stdcall
            const int GWL_USERDATA = -21;
            const uint32_t WinFlagClipToParent = 0x20;
            struct Win {{ uint32_t iFlags_; }};
            Win *g_owner = 0;
            HWND g_parent = 0;
            HWND g_set_parent_target = 0;
            int g_calls[3] = {{0, 0, 0}};
            int g_call_count = 0;
            HWND WinReparentMatchParent = 7;
            HWND WinReparentTargetParent = 9;
            long GetWindowLongA(HWND hwnd, int index) {{
                assert(hwnd == 13 && index == GWL_USERDATA);
                g_calls[g_call_count++] = 1;
                return reinterpret_cast<long>(g_owner);
            }}
            HWND GetParent(HWND hwnd) {{
                assert(hwnd == 13); g_calls[g_call_count++] = 2; return g_parent;
            }}
            HWND SetParent(HWND hwnd, HWND target) {{
                assert(hwnd == 13); g_calls[g_call_count++] = 3;
                g_set_parent_target = target; return 0;
            }}
            {implementation}
            void reset() {{
                g_call_count = 0; g_calls[0] = g_calls[1] = g_calls[2] = 0;
                g_set_parent_target = 0;
            }}
            int main() {{
                Win owner; owner.iFlags_ = 0;
                assert(win_reparent_window(13, 0x1234) == 1);
                assert(g_call_count == 1 && g_calls[0] == 1);
                reset(); g_owner = &owner; owner.iFlags_ = WinFlagClipToParent;
                assert(win_reparent_window(13, 99) == 1);
                assert(g_call_count == 1 && g_calls[0] == 1);
                reset(); owner.iFlags_ = 0; g_parent = 8;
                assert(win_reparent_window(13, -1) == 1);
                assert(g_call_count == 2 && g_calls[0] == 1 && g_calls[1] == 2);
                reset(); g_parent = WinReparentMatchParent;
                assert(win_reparent_window(13, 17) == 1);
                assert(g_call_count == 3 && g_calls[0] == 1 &&
                       g_calls[1] == 2 && g_calls[2] == 3);
                assert(g_set_parent_target == WinReparentTargetParent);
                return 0;
            }}
        """)

    actual = compile_and_run(tmp_path, "win_reparent_actual", program(method))
    assert actual.returncode == 0, actual.stdout + actual.stderr
    mutated = method.replace("WinFlagClipToParent) == 0",
                            "WinFlagClipToParent) != 0", 1)
    assert mutated != method
    rejected = compile_and_run(tmp_path, "win_reparent_mutation", program(mutated))
    assert rejected.returncode != 0
