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
static void (*after_copy_callback)();
static void (*width_callback)();
static const char *expected_width_inputs[2];

struct CreditsWave {
    void load(char *path, int mode) {
        assert(strcmp(path, "fx\\opening menu.wav") == 0 && mode == 2);
        ++audio_calls; events.push_back("audio");
    }
};
CreditsWave g_CREDITS_WAVE;

int text_open(const char *section, const char *name) {
    assert(strcmp(section, "CREDITS") == 0 && strcmp(name, "CREDITS") == 0);
    assert(section != name); // Pin passes two distinct original text objects.
    assert(section == CreditsFileLabel && name == CreditsSectionLabel);
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
    char *result = strcpy(destination, source);
    if (after_copy_callback) after_copy_callback();
    return result;
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
        const char *expected = expected_width_inputs[width_calls - 1];
        assert(strcmp(expected ? expected : width_input, text) == 0);
        const int result = static_cast<int>(strlen(text) * 10);
        if (width_callback) width_callback();
        return result;
    }
};
struct Credits {
    uint32_t line_count_;
    char *lines_[4];
    uint32_t field_1230_;
    Font font_;
    void unload() { assert(line_count_ == 0); ++unload_calls; events.push_back("unload"); }
    void load();
};

ACTUAL_BODY

static Credits *width_receiver;
static int maximum_before_width;
static char redirected_first[] = "first";
static char redirected_second[] = "second";
static void redirect_after_copy() {
    width_receiver->line_count_ = 2;
    width_receiver->lines_[1] = redirected_first;
    width_receiver->field_1230_ = maximum_before_width;
}
static void redirect_during_width() {
    if (width_calls == 1) {
        width_receiver->line_count_ = 3;
        width_receiver->lines_[2] = redirected_second;
        width_receiver->field_1230_ = 999;
    }
}

static void reset_case() {
    events.clear();
    replacement_after_allocation = 0;
    allocation_size = 0;
    allocation_fails = text_open_result = 0;
    text_get_calls = text_close_calls = unload_calls = audio_calls = width_calls = 0;
    width_input[0] = 0;
    after_copy_callback = width_callback = 0;
    expected_width_inputs[0] = expected_width_inputs[1] = 0;
    width_receiver = 0;
    memset(allocation_slot, 0xCC, 64);
}

int main() {
    static char before[] = "Longer";
    static char after[] = "new";
    reset_case();
    TextBufferGetPtr = before;
    replacement_after_allocation = after;
    Credits changed;
    changed.line_count_ = 3; changed.field_1230_ = 99;
    for (int i = 0; i != 4; ++i) changed.lines_[i] = 0;
    changed.load();
    assert(allocation_size == sizeof(before));
    assert(changed.line_count_ == 1 && changed.lines_[0] == allocation_slot);
    assert(strcmp(allocation_slot, "new") == 0 && strcmp(width_input, "new") == 0);
    assert(changed.field_1230_ == 30 && width_calls == 2);
    assert(events.size() == 11);
    assert(events[0] == "unload" && events[1] == "audio" && events[2] == "open");
    assert(events[3] == "text" && events[4] == "strlen" && events[5] == "alloc");
    assert(events[6] == "strcpy" && events[7] == "width" &&
           events[8] == "width" && events[9] == "text" && events[10] == "close");

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

    // Pin reloads the owner slot after strcpy and after the first width call.
    // It caches the pre-call maximum and stores that selection on either arm.
    for (int keep_old = 0; keep_old != 2; ++keep_old) {
        reset_case();
        TextBufferGetPtr = stable;
        Credits redirected;
        redirected.line_count_ = 3;
        redirected.field_1230_ = 123;
        for (int i = 0; i != 4; ++i) redirected.lines_[i] = 0;
        width_receiver = &redirected;
        maximum_before_width = keep_old ? 70 : 20;
        expected_width_inputs[0] = redirected_first;
        expected_width_inputs[1] = redirected_second;
        after_copy_callback = redirect_after_copy;
        width_callback = redirect_during_width;
        redirected.load();
        assert(strcmp(allocation_slot, "Stay") == 0);
        assert(redirected.line_count_ == 3);
        assert(redirected.lines_[1] == redirected_first &&
               redirected.lines_[2] == redirected_second);
        assert(width_calls == (keep_old ? 1 : 2));
        assert(redirected.field_1230_ == (keep_old ? 70u : 60u));
    }
}
'''


def _body() -> str:
    return extract_definition(SOURCE.read_text(encoding="utf-8"), SIGNATURE)


def _load_constants() -> str:
    source = SOURCE.read_text(encoding="utf-8")
    names = ("CreditsOpeningMenuWavePath", "CreditsSectionLabel", "CreditsFileLabel")
    declarations = []
    for name in names:
        matches = re.findall(r'(?m)^const char ' + name + r'\[\] = "[^"\n]*";', source)
        assert len(matches) == 1, f"missing actual text-object definition: {name}"
        declarations.append(matches[0])
    return "\n".join(declarations)


def _stale_snapshot_mutant(body: str) -> str:
    allocation = "lines_[line_count_] = static_cast<char *>(mem_get(strlen(TextBufferGetPtr) + 1));"
    copy = "strcpy(stored, TextBufferGetPtr);"
    assert body.count(allocation) == 1 and body.count(copy) == 1
    stale_allocation = "char *line = TextBufferGetPtr;\n      lines_[line_count_] = static_cast<char *>(mem_get(strlen(line) + 1));"
    body = body.replace(allocation, stale_allocation, 1)
    body = body.replace(copy, "strcpy(stored, line);", 1)
    assert body != _body()
    return body


def _compile_and_run(tmp_path: Path, compiler: str, body: str, name: str) -> subprocess.CompletedProcess[str]:
    body = re.sub(r"(?<![A-Za-z0-9_])strlen\(", "host_strlen(", body)
    body = re.sub(r"(?<![A-Za-z0-9_])strcpy\(", "host_strcpy(", body)
    source = _load_constants() + "\n" + PREAMBLE.replace("ACTUAL_BODY", body)
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


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_missing_early_count_reset_compiles_and_fails(tmp_path: Path, compiler_name: str) -> None:
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    body = _body()
    before = "  line_count_ = 0;\n  unload();"
    assert body.count(before) == 1
    damaged = body.replace(before, "  unload();", 1)
    assert damaged != body
    result = _compile_and_run(tmp_path, compiler, damaged, f"credits-early-count-{compiler_name}")
    assert result.returncode != 0, "missing pre-release count reset escaped the oracle"


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("mutation", ["cached_width_pointer", "live_maximum", "missing_maximum_store"])
def test_width_owner_lifetime_mutants_compile_and_fail(tmp_path: Path, compiler_name: str, mutation: str) -> None:
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    body = _body()
    if mutation == "cached_width_pointer":
        before = "font_.width(lines_[line_count_ - 1])"
        assert body.count(before) == 2
        damaged = body.replace(before, "font_.width(stored)")
    elif mutation == "live_maximum":
        before = "maximum > measured"
        assert body.count(before) == 1
        damaged = body.replace(before, "static_cast<int>(field_1230_) > measured", 1)
    else:
        before = "field_1230_ = maximum > measured\n            ? maximum : font_.width(lines_[line_count_ - 1]);"
        assert body.count(before) == 1
        damaged = body.replace(before, "(void)(maximum > measured ? maximum : font_.width(lines_[line_count_ - 1]));", 1)
    assert damaged != body
    result = _compile_and_run(tmp_path, compiler, damaged, f"credits-{mutation}-{compiler_name}")
    assert result.returncode != 0, f"{mutation} escaped the callback oracle"


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_pooled_filename_and_section_mutant_compiles_and_fails(tmp_path: Path, compiler_name: str) -> None:
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    body = _body()
    before = "text_open(CreditsFileLabel, CreditsSectionLabel)"
    assert body.count(before) == 1
    damaged = body.replace(before, "text_open(CreditsFileLabel, CreditsFileLabel)", 1)
    assert damaged != body
    result = _compile_and_run(tmp_path, compiler, damaged, f"credits-pooled-labels-{compiler_name}")
    assert result.returncode != 0, "merged original text identities escaped the oracle"
