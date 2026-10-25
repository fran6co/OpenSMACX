"""Actual-source control for Credits::load's post-allocation global reload."""
from __future__ import annotations

from pathlib import Path
import re
import shutil
import subprocess

import pytest

from test_credits_timer_contract_oct19 import extract_definition


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/credits.cpp"
SIGNATURE = "void Credits::load()"

PREAMBLE = r'''#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <vector>
#include <string>

static std::vector<std::string> events;
static char *TextBufferGetPtr;
static char *replacement_after_allocation;
static char allocation_storage[64];
static char *allocation_slot = allocation_storage;
static size_t allocation_size;
static int allocation_fails;
static int text_open_result;
static int text_get_calls, text_close_calls, unload_calls, audio_calls;
static int width_calls;
static char width_input[32];

struct CreditsWave {
    void load(char *path, int mode) {
        assert(strcmp(path, "fx\\opening menu.wav") == 0 && mode == 2);
        ++audio_calls; events.push_back("audio");
    }
};
CreditsWave g_CREDITS_WAVE;

int text_open(const char *section, const char *name) {
    assert(strcmp(section, "CREDITS") == 0 && strcmp(name, "CREDITS") == 0);
    events.push_back("open");
    return text_open_result;
}
char *text_get() {
    ++text_get_calls; events.push_back("text");
    static char content[] = "x";
    static char end[] = "#";
    return text_get_calls == 1 ? content : end;
}
void text_close() { ++text_close_calls; events.push_back("close"); }
size_t host_strlen(const char *text) {
    events.push_back("strlen");
    return strlen(text);
}
char *host_strcpy(char *destination, const char *source) {
    events.push_back("strcpy");
    strncpy(width_input, source, sizeof(width_input) - 1);
    width_input[sizeof(width_input) - 1] = 0;
    return strcpy(destination, source);
}
void *mem_get(size_t bytes) {
    events.push_back("alloc");
    allocation_size = bytes;
    TextBufferGetPtr = replacement_after_allocation
        ? replacement_after_allocation : TextBufferGetPtr;
    return allocation_fails ? 0 : allocation_slot;
}

struct Font {
    int width(const char *text) {
        ++width_calls; events.push_back("width");
        assert(strcmp(width_input, text) == 0);
        return static_cast<int>(strlen(text) * 10);
    }
};
struct Credits {
    uint32_t line_count_;
    char *lines_[4];
    uint32_t field_1230_;
    Font font_;
    void unload() { ++unload_calls; events.push_back("unload"); }
    void load();
};

ACTUAL_BODY

static void reset_case() {
    events.clear();
    replacement_after_allocation = 0;
    allocation_size = 0;
    allocation_fails = text_open_result = 0;
    text_get_calls = text_close_calls = unload_calls = audio_calls = width_calls = 0;
    width_input[0] = 0;
    memset(allocation_slot, 0xCC, 64);
}

int main() {
    static char before[] = "Longer";
    static char after[] = "new";
    reset_case();
    TextBufferGetPtr = before;
    replacement_after_allocation = after;
    Credits changed;
    changed.line_count_ = 0; changed.field_1230_ = 99;
    for (int i = 0; i != 4; ++i) changed.lines_[i] = 0;
    changed.load();
    assert(allocation_size == sizeof(before));
    assert(changed.line_count_ == 1 && changed.lines_[0] == allocation_slot);
    assert(strcmp(allocation_slot, "new") == 0 && strcmp(width_input, "new") == 0);
    assert(changed.field_1230_ == 30 && width_calls == 1);
    assert(events.size() == 10);
    assert(events[0] == "unload" && events[1] == "audio" && events[2] == "open");
    assert(events[3] == "text" && events[4] == "strlen" && events[5] == "alloc");
    assert(events[6] == "strcpy" && events[7] == "width" &&
           events[8] == "text" && events[9] == "close");

    reset_case();
    static char stable[] = "Stay";
    TextBufferGetPtr = stable;
    replacement_after_allocation = 0;
    Credits unchanged;
    unchanged.line_count_ = 0; unchanged.field_1230_ = 0;
    for (int i = 0; i != 4; ++i) unchanged.lines_[i] = 0;
    unchanged.load();
    assert(allocation_size == sizeof(stable) && strcmp(allocation_slot, "Stay") == 0);
    assert(unchanged.line_count_ == 1 && unchanged.field_1230_ == 40);

    reset_case();
    TextBufferGetPtr = stable;
    allocation_fails = 1;
    Credits failed;
    failed.line_count_ = 0; failed.field_1230_ = 0;
    for (int i = 0; i != 4; ++i) failed.lines_[i] = 0;
    failed.load();
    assert(failed.lines_[0] == 0 && failed.line_count_ == 0 && width_calls == 0);
    assert(text_close_calls == 1);
}
'''


def _body() -> str:
    return extract_definition(SOURCE.read_text(encoding="utf-8"), SIGNATURE)


def _stale_snapshot_mutant(body: str) -> str:
    allocation = "char *stored = static_cast<char *>(mem_get(strlen(TextBufferGetPtr) + 1));"
    copy = "strcpy(stored, TextBufferGetPtr);"
    assert body.count(allocation) == 1 and body.count(copy) == 1
    stale_allocation = "char *line = TextBufferGetPtr;\n      char *stored = static_cast<char *>(mem_get(strlen(line) + 1));"
    body = body.replace(allocation, stale_allocation, 1)
    body = body.replace(copy, "strcpy(stored, line);", 1)
    assert body != _body()
    return body


def _compile_and_run(tmp_path: Path, compiler: str, body: str, name: str) -> subprocess.CompletedProcess[str]:
    body = re.sub(r"(?<![A-Za-z0-9_])strlen\(", "host_strlen(", body)
    body = re.sub(r"(?<![A-Za-z0-9_])strcpy\(", "host_strcpy(", body)
    source = PREAMBLE.replace("ACTUAL_BODY", body)
    cpp = tmp_path / f"{name}.cpp"
    exe = tmp_path / name
    cpp.write_text(source, encoding="utf-8")
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(exe)], check=True, capture_output=True,
                   text=True, timeout=60)
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=20)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_credits_load_observes_text_pointer_after_allocator_callback(tmp_path: Path, compiler_name: str) -> None:
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    result = _compile_and_run(tmp_path, compiler, _body(), f"credits-reload-{compiler_name}")
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_cached_preallocation_pointer_compiles_and_fails(tmp_path: Path, compiler_name: str) -> None:
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    result = _compile_and_run(tmp_path, compiler, _stale_snapshot_mutant(_body()),
                              f"credits-stale-pointer-{compiler_name}")
    assert result.returncode != 0, "stale preallocation pointer mutant survived"
