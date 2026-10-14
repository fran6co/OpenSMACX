"""Host-only controls for TextIndex::make_index startup cleanup.

The actual function body is compiled against narrow Filemap/Heap doubles and
Filemap open is forced to fail. This tests the real startup order and automatic
scope exit, not the Win32 ABI, SEH table, or successful file parsing.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def _braced(text: str, signature: str) -> str:
    clean = re.sub(r"/\*.*?\*/|//[^\n]*", "", text, flags=re.S)
    starts = [m.start() for m in re.finditer(re.escape(signature), clean)]
    assert len(starts) == 1, f"expected one actual definition of {signature!r}"
    start = starts[0]
    opening = clean.index("{", start)
    depth = 0
    for pos in range(opening, len(clean)):
        if clean[pos] == "{":
            depth += 1
        elif clean[pos] == "}":
            depth -= 1
            if depth == 0:
                # `start`/`pos` index the comment-free text, so return that
                # same view rather than slicing the original at shifted offsets.
                return clean[start:pos + 1]
    raise AssertionError(f"unterminated actual definition: {signature}")


def _actual_body(source_text: str | None = None) -> str:
    source = source_text if source_text is not None else (ROOT / "src/textindex.cpp").read_text()
    body = _braced(source, "void TextIndex::make_index(")
    assert "heap_.reset();\n    section_count_ = 0;" in body
    assert "shutdown();" not in body
    # Keep the host bridge local to the extracted body. Redefining the
    # standard-library uint32_t name would conflict with <cstdint>.
    return body.replace("uint32_t", "HostU32")


HARNESS = r'''
#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

using std::uintptr_t;
using HostU32 = uintptr_t; // Host bridge for the source's 32-bit pointer arithmetic.
using LPVOID = void *;
using LPSTR = char *;
using LPCSTR = const char *;
using LPDWORD = uint32_t *;
using BOOL = int;

static std::vector<std::string> events;
static void record(const char *event) { events.emplace_back(event); }

static char *tracked_strcpy(char *to, const char *from) {
    record("strcpy");
    return std::strcpy(to, from);
}
static char *tracked_strcat(char *to, const char *from) {
    record("strcat");
    return std::strcat(to, from);
}
static char *tracked_strchr(const char *text, int value) {
    record("strchr");
    return const_cast<char *>(std::strchr(text, value));
}
#define strcpy tracked_strcpy
#define strcat tracked_strcat
#define strchr tracked_strchr

struct CountField {
    uint32_t value = 0;
    CountField &operator=(uint32_t next) {
        if (next == 0) record("count.clear");
        value = next;
        return *this;
    }
    CountField &operator++() { ++value; return *this; }
    CountField operator++(int) { CountField old = *this; ++value; return old; }
    operator uint32_t() const { return value; }
};

struct Heap {
    void reset() { record("heap.reset"); }
    void shutdown() { record("heap.shutdown"); }
    BOOL init(int) { record("heap.init"); return 0; }
    LPVOID get(int) { record("heap.get"); return nullptr; }
    void squeeze(int) { record("heap.squeeze"); }
};

struct Filemap {
    Filemap() { record("filemap.ctor"); }
    ~Filemap() { record("filemap.dtor"); }
    LPVOID open_read(LPCSTR path, int) {
        events.emplace_back(std::string("open:") + path);
        return nullptr; // The tested error path: no mapping is opened.
    }
    int get_handle() { return 7; }
    void close() { record("filemap.explicit_close"); }
};

class TextIndex {
public:
    char file_name_[256] = {};
    CountField section_count_;
    Heap heap_;
    void shutdown() {
        section_count_ = 0;
        file_name_[0] = 0;
        heap_.shutdown();
    }
    void make_index(LPCSTR source_txt);
};

static char TextBufferGetPtr[4096];
static uint32_t GetFileSize(int, void *) { return 0; }
static void kill_lf(char *) {}
static void purge_spaces(char *) {}

ACTUAL_BODY

int main() {
    TextIndex index;
    std::memcpy(index.file_name_, "prior/name", sizeof("prior/name"));
    index.section_count_.value = 9;

    index.make_index("scripts/alpha");
    const std::vector<std::string> expected = {
        "filemap.ctor", "heap.reset", "count.clear", "strcpy", "strchr",
        "strcat", "open:scripts/alpha.txt", "filemap.dtor"
    };
    assert(events == expected);
    assert(index.section_count_.value == 0);
    assert(std::strcmp(index.file_name_, "scripts/alpha.txt") == 0);

    events.clear();
    index.section_count_.value = 4;
    index.make_index("folder.v1/alpha");
    const std::vector<std::string> dotted = {
        "filemap.ctor", "heap.reset", "count.clear", "strcpy", "strchr",
        "open:folder.v1/alpha", "filemap.dtor"
    };
    assert(events == dotted); // Original strchr sees a dot anywhere in the path.
    assert(index.section_count_.value == 0);
    assert(std::strcmp(index.file_name_, "folder.v1/alpha") == 0);
}
'''


def _run(tmp_path: Path, body: str, compiler: str) -> subprocess.CompletedProcess:
    source = HARNESS.replace("ACTUAL_BODY", body)
    cpp = tmp_path / "textindex_cleanup.cpp"
    exe = tmp_path / "textindex_cleanup"
    cpp.write_text(source)
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(exe)], check=True, capture_output=True, text=True)
    return subprocess.run([str(exe)], capture_output=True, text=True)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_make_index_cleanup_precedes_copy_and_failed_open_exits_by_raii(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    result = _run(tmp_path, _actual_body(), compiler)
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_old_shutdown_body_is_a_live_damage_control(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    body = _actual_body()
    replacement = "heap_.reset();\n    section_count_ = 0;"
    assert body.count(replacement) == 1
    damaged = body.replace(replacement, "shutdown();", 1)
    result = _run(tmp_path, damaged, compiler)
    assert result.returncode != 0, "the old TextIndex::shutdown path unexpectedly passed"


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_wrong_heap_cleanup_and_reversed_order_are_live_damage_controls(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    body = _actual_body()
    replacement = "heap_.reset();\n    section_count_ = 0;"
    assert body.count(replacement) == 1
    wrong_callee = body.replace("heap_.reset();", "heap_.shutdown();", 1)
    wrong_order = body.replace(replacement, "section_count_ = 0;\n    heap_.reset();", 1)
    for damaged in (wrong_callee, wrong_order):
        result = _run(tmp_path, damaged, compiler)
        assert result.returncode != 0, "an incorrect cleanup callee/order passed"
