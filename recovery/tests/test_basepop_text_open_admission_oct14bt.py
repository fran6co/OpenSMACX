"""Bounded actual-source control for BasePop's pre-parser admission path.

The real function prefix through text_open is compiled; the large parser
continuation is explicitly replaced with a counter and sentinel return. This
does not validate parser behavior or establish native/runtime acceptance.
"""
from pathlib import Path
import shutil
import subprocess
import os

import pytest
from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/basepop.cpp"
SIGNATURE = "int BasePop::start(char *text_file, const char *text_label, int value,"


def actual_admission_prefix():
    source = SOURCE.read_text(encoding="utf-8")
    records = [record for record in read_file(SOURCE) if record.address == 0x601BF0]
    assert len(records) == 1, "BasePop::start identity must remain unique"
    assert records[0].state.value == "implemented"
    start = source.index(SIGNATURE)
    opening = source.index("{", start)
    label = source.index("      read_line:", opening)
    return source[start:label]


def fixture_for(prefix):
    # Deliberately provide only the real state and calls used before read_line.
    return r'''#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
static std::vector<std::string> events;
static void note(const char *s) { events.push_back(s); }
static void fail(const char *s) {
    std::fprintf(stderr, "SEMANTIC97: %s\n", s);
    std::exit(97);
}
static void require(bool ok, const char *s) { if (!ok) fail(s); }
static char PopupStartCaption[] = "default-popup.txt";
static int text_open_result;
static int text_open_calls;
static char opened_file[128];
static char opened_section[128];
static int parser_calls;
static int parser_result = 73;
static const char *expected_file;
static const char *expected_section;
static int init_result;
static int init_calls;
static int close_calls;
static int visible_calls;
static int override_calls;
static int override_order;
static int open_order;
static int last_init_mode;
static int last_init_flags;
static int text_open(char *file, const char *section) {
    ++text_open_calls;
    std::strncpy(opened_file, file, sizeof(opened_file) - 1);
    std::strncpy(opened_section, section, sizeof(opened_section) - 1);
    opened_file[sizeof(opened_file) - 1] = '\0';
    opened_section[sizeof(opened_section) - 1] = '\0';
    open_order = static_cast<int>(events.size());
    note("open");
    return text_open_result;
}
static int event_index(const char *name) {
    for (size_t i = 0; i != events.size(); ++i)
        if (events[i] == name) return static_cast<int>(i);
    return -1;
}
struct EditGroup {
    void set_default(char *text, int flags) {
        ++override_calls;
        override_order = static_cast<int>(events.size());
        note("override");
        (void)text; (void)flags;
    }
};
struct Dialog : EditGroup { int kind_; Dialog() : kind_(0) {} };
struct GraphicWin {
    void is_visible() { ++visible_calls; note("owner-visible"); }
};
struct BasePop {
    Dialog dialogs_;
    void *external_window_;
    GraphicWin *owner_window_;
    int value_;
    BasePop() : external_window_(reinterpret_cast<void *>(1)),
                owner_window_(0), value_(-99) {}
    void close() { ++close_calls; note("close"); }
    int init(int mode, int flags) {
        ++init_calls; last_init_mode = mode; last_init_flags = flags;
        note("init"); return init_result;
    }
    int start(char *text_file, const char *text_label, int value,
              char *override_text, int flags, GraphicWin *owner_window);
};
''' + prefix + r'''
    // The test replaces the parser, so expose otherwise parser-only locals
    // without reading or initializing them as product behavior.
    (void)add_as_escaped; (void)line; (void)text; (void)tok; (void)marks;
    (void)token; (void)width; (void)height;
    ++parser_calls;
    note("parser");
    return parser_result;
  }
  return result;
}

static void reset_case() {
    events.clear();
    text_open_result = text_open_calls = parser_calls = 0;
    opened_file[0] = opened_section[0] = '\0';
    init_result = init_calls = close_calls = visible_calls = 0;
    override_calls = override_order = open_order = 0;
    last_init_mode = last_init_flags = 0;
    expected_file = ""; expected_section = "";
}

static void check_success_default_and_side_effect_order() {
    reset_case();
    init_result = 0;
    expected_file = "default-popup.txt";
    expected_section = "MENU";
    BasePop popup;
    GraphicWin owner;
    char override_text[] = "custom default";
    const int result = popup.start(nullptr, "MENU", 17, override_text, 1, &owner);
    require(result == parser_result && parser_calls == 1,
            "successful text open admits parser");
    require(text_open_calls == 1 && std::strcmp(opened_file, expected_file) == 0,
            "null filename uses default popup filename");
    require(std::strcmp(opened_section, expected_section) == 0,
            "text section is passed unchanged");
    require(popup.dialogs_.kind_ == 1 && popup.value_ == 17,
            "kind and selected value publish before open");
    require(popup.owner_window_ == &owner && visible_calls == 1,
            "owner visibility callback precedes ownership publication");
    require(popup.external_window_ == nullptr,
            "default flags clear external owner slot");
    require(close_calls == 1 && init_calls == 1 && last_init_mode == 0 &&
            last_init_flags == 1,
            "normal admission closes and initializes with original arguments");
    require(override_calls == 1 && override_order < open_order,
            "override default is set before opening text");
    require(event_index("close") < event_index("init") &&
            event_index("init") < event_index("owner-visible") &&
            event_index("owner-visible") < event_index("override") &&
            event_index("override") < event_index("open"),
            "close/init/owner/override/open ordering is retained");
}

static void check_open_errors() {
    const int errors[2] = {1, 7};
    for (int i = 0; i != 2; ++i) {
        reset_case();
        text_open_result = errors[i];
        BasePop popup;
        GraphicWin owner;
        char filename[] = "explicit-script.txt";
        char override_text[] = "override";
        const int result = popup.start(filename, "OPTIONS", 9,
                                       override_text, 2, &owner);
        require(result == 1, "text-open error returns 1");
        require(parser_calls == 0 && text_open_calls == 1,
                "text-open error returns before parser continuation");
        require(std::strcmp(opened_file, filename) == 0 &&
                std::strcmp(opened_section, "OPTIONS") == 0,
                "explicit file and section reach text_open unchanged");
        require(popup.owner_window_ == &owner && visible_calls == 1 &&
                override_calls == 1 && override_order < open_order,
                "owner and override side effects precede open failure");
    }
}

static void check_init_failure() {
    reset_case();
    init_result = 7;
    BasePop popup;
    GraphicWin owner;
    const int result = popup.start(nullptr, "MENU", 3, nullptr, 0, &owner);
    require(result == 7, "init failure result is propagated");
    require(init_calls == 1 && text_open_calls == 0 && parser_calls == 0,
            "init failure prevents text open and parser");
    require(close_calls == 1 && popup.owner_window_ == nullptr && visible_calls == 0,
            "failed init preserves pre-admission owner behavior");
    require(popup.external_window_ == reinterpret_cast<void *>(1),
            "failed init does not clear external window");
}

static void check_flags_80_bypass() {
    reset_case();
    BasePop popup;
    GraphicWin owner;
    char filename[] = "bypass.txt";
    const int result = popup.start(filename, "MENU", 5, nullptr,
                                   0x80 | 4, &owner);
    require(result == parser_result && parser_calls == 1,
            "flags 0x80 success admits parser");
    require(close_calls == 0 && init_calls == 0,
            "flags 0x80 bypasses close and initialization");
    require(popup.dialogs_.kind_ == 4 && popup.owner_window_ == &owner &&
            visible_calls == 1 && popup.value_ == 5,
            "flags 0x80 still publishes kind, owner and value");
    require(text_open_calls == 1 && std::strcmp(opened_file, filename) == 0,
            "flags 0x80 still opens the requested script");
}

static void check_null_section() {
    reset_case();
    BasePop popup;
    GraphicWin owner;
    char override_text[] = "not-applied";
    const int result = popup.start(nullptr, nullptr, 11, override_text, 1, &owner);
    require(result == 3, "null section returns 3");
    require(text_open_calls == 0 && parser_calls == 0 && override_calls == 0,
            "null section returns before override and text open");
    require(popup.owner_window_ == &owner && visible_calls == 1 &&
            popup.value_ == 11,
            "owner and value side effects precede null-section return");
}

int main() {
    check_success_default_and_side_effect_order();
    check_open_errors();
    check_init_failure();
    check_flags_80_bypass();
    check_null_section();
    return 0;
}
'''


def compile_source(tmp_path, body):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    path = tmp_path / "basepop-admission.cpp"
    binary = tmp_path / "basepop-admission"
    path.write_text(fixture_for(body), encoding="utf-8")
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                    str(path), "-o", str(binary)], check=True,
                   capture_output=True, text=True, timeout=30)
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10,
                          env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))


def test_actual_admission_prefix_cases(tmp_path):
    result = compile_source(tmp_path, actual_admission_prefix())
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("mutation,diagnostic", [
    ("inverted_guard", "successful text open admits parser"),
    ("wrong_error_result", "text-open error returns 1"),
])
def test_admission_mutants_compile_and_fail_semantically(tmp_path, mutation, diagnostic):
    prefix = actual_admission_prefix()
    good = "if (text_open(text_file != 0 ? text_file : (char *)PopupStartCaption,\n                      text_label) != 0) {\n            return 1;\n        }"
    assert good in prefix
    if mutation == "inverted_guard":
        bad = good.replace(") != 0) {", ") == 0) {")
    else:
        bad = good.replace("return 1;", "return result;")
    result = compile_source(tmp_path, prefix.replace(good, bad, 1))
    assert result.returncode == 97, result.stdout + result.stderr
    assert ("SEMANTIC97: " + diagnostic) in result.stderr
