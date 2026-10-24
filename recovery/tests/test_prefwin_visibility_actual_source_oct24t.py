"""Bounded host controls for PrefWin's preference-page visibility calls.

The test selects the complete mapped update_pref_buttons definition through
match_recovery.source.read_file and compiles that definition against recording
widget/label doubles. It checks visible-state arguments, selected widget and
call order; it does not claim native ABI or UI-runtime behavior.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest

from match_recovery.source import read_file


ROOT = Path(os.environ.get("OPENSMACX_SOURCE_ROOT", Path(__file__).resolve().parents[2]))
UNIT = ROOT / "src/prefwin.cpp"
ADDRESS = 0x00490140
SIGNATURE = "void PrefWin::update_pref_buttons()"


def _actual_source():
    records = [record for record in read_file(UNIT) if record.address == ADDRESS]
    assert len(records) == 1
    return records[0].region


def _definition(source):
    start = source.index(SIGNATURE)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError("unterminated PrefWin::update_pref_buttons")


PREFWIN_FIXTURE = r'''
#include <cassert>
#include <cstring>
#include <string>
#include <vector>
using std::string;
using std::vector;
vector<string> events;
char StringTemp[256];
const char *label_1100 = "more-options";
const char *label_a60 = "continue-warning";
bool label_1100_null = false;
bool label_a60_null = false;
struct NullConcatCall {};

static string number(int value) { return std::to_string(value); }
int label_index(int offset) {
    events.push_back("index:" + number(offset));
    return offset;
}
const char *label_get(int index) {
    events.push_back("label:" + number(index));
    if (index == 0x1100) return label_1100_null ? 0 : label_1100;
    if (index == 0xA60) {
        events.push_back(StringTemp[0] == '\0' ? "temp-cleared" : "temp-not-cleared");
        return label_a60_null ? 0 : label_a60;
    }
    return 0;
}
char *observed_strcat(char *destination, const char *source) {
    events.push_back(source == 0 ? "append:<null>" : string("append:") + source);
    if (source == 0) throw NullConcatCall();
    return std::strcat(destination, source);
}

struct Widget {
    string tag;
    string name;
    bool null_name;
    explicit Widget(const char *value) : tag(value), name(), null_name(false) {}
    void show(int visible) {
        events.push_back("show:" + tag + ":" + number(visible));
    }
    void hide() { events.push_back("hide:" + tag); }
    void set_name(const char *value) {
        null_name = value == 0;
        name = value == 0 ? "<null>" : value;
        events.push_back("name:" + tag + ":" + name);
    }
};

// Ordinary reference spelling uses the same mock service boundary.
typedef Widget FlatButton;

struct PrefWin {
    int field_A14_;
    Widget bscPrefCheckBox_, advPrefCheckBox_, warnPrefCheckBox_, moreButton_;
    Widget autoPrefCheckBox_, audioVisPrefCheckBox_;
    Widget volumeMainScroll_, volumeVoiceScroll_, volumeSFXScroll_, volumeMusicScroll_;
    Widget volumeMainToggle_, volumeVoiceToggle_, volumeSFXToggle_, volumeMusicToggle_;
    Widget mapPrefCheckBox_, warnContPrefCheckBox_;
    PrefWin() : field_A14_(0), bscPrefCheckBox_("basic"), advPrefCheckBox_("advanced"),
        warnPrefCheckBox_("warnings"), moreButton_("more"), autoPrefCheckBox_("auto"),
        audioVisPrefCheckBox_("audio-visual"), volumeMainScroll_("main-scroll"),
        volumeVoiceScroll_("voice-scroll"), volumeSFXScroll_("sfx-scroll"),
        volumeMusicScroll_("music-scroll"), volumeMainToggle_("main-toggle"),
        volumeVoiceToggle_("voice-toggle"), volumeSFXToggle_("sfx-toggle"),
        volumeMusicToggle_("music-toggle"), mapPrefCheckBox_("map"),
        warnContPrefCheckBox_("warning-continue") {}
    void update_pref_buttons();
};
'''


def _compile_run(compiler, tmp_path, body, main, label, should_pass):
    source = tmp_path / (label + ".cpp")
    binary = tmp_path / label
    source.write_text(PREFWIN_FIXTURE + body + "\n" + main, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(source), "-o", str(binary)], capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    if should_pass:
        assert ran.returncode == 0, ran.stdout + ran.stderr
    else:
        assert ran.returncode != 0, "show(3) mutant escaped the visible-state oracle"


MAIN = r'''
static void check(int page, const vector<string> &expected) {
    events.clear();
    StringTemp[0] = '\0';
    label_1100_null = false;
    label_a60_null = false;
    PrefWin window;
    window.field_A14_ = page;
    window.update_pref_buttons();
    assert(events == expected);
}
int main() {
    check(0, vector<string>{"show:basic:0"});
    check(1, vector<string>{"show:advanced:0"});
    check(2, vector<string>{"show:warnings:0", "index:4352", "label:4352",
                            "name:more:more-options", "show:more:0"});
    check(3, vector<string>{"show:auto:0"});
    check(4, vector<string>{"show:audio-visual:0", "show:main-scroll:0",
                            "show:voice-scroll:0", "show:sfx-scroll:0",
                            "show:music-scroll:0", "show:main-toggle:0",
                            "show:voice-toggle:0", "show:sfx-toggle:0",
                            "show:music-toggle:0"});
    check(5, vector<string>{"show:map:0"});
    check(6, vector<string>{"show:warning-continue:0", "index:2656", "label:2656",
                            "temp-cleared", "append:continue-warning",
                            "name:more:continue-warning", "show:more:0"});
    check(-1, vector<string>());
    check(7, vector<string>());

    events.clear();
    label_1100_null = true;
    PrefWin direct_null;
    direct_null.field_A14_ = 2;
    direct_null.update_pref_buttons();
    assert(events == vector<string>({"show:warnings:0", "index:4352", "label:4352",
                                    "name:more:<null>", "show:more:0"}));
    assert(direct_null.moreButton_.null_name);

    events.clear();
    label_a60_null = true;
    PrefWin null_concat;
    null_concat.field_A14_ = 6;
    bool caught_null_concat = false;
    try {
        null_concat.update_pref_buttons();
    } catch (const NullConcatCall &) {
        caught_null_concat = true;
    }
    assert(caught_null_concat);
    assert(events == vector<string>({"show:warning-continue:0", "index:2656", "label:2656",
                                    "temp-cleared", "append:<null>"}));
    assert(StringTemp[0] == '\0');
}
'''


def test_prefwin_page_visibility_actual_source_and_show_mutant(tmp_path):
    body = _definition(_actual_source())
    # The reviewed candidate has 17 show calls, and the pinned push-0 control
    # is checked through their observable arguments on every page.
    assert body.count(".show(0)") == 17
    append_call = re.compile(
        r"strcat\s*\(\s*StringTemp\s*,\s*label_get\s*\(\s*label_index\s*\(\s*0xA60\s*\)\s*\)\s*\)\s*;"
    )
    assert len(append_call.findall(body)) == 1
    guarded_raw, replaced = append_call.subn(
        "if (const char *label = label_get(label_index(0xA60))) "
        "strcat(StringTemp, label);",
        body,
        count=1,
    )
    assert replaced == 1
    # Replace only the strcat service boundary, so a null input records the
    # original call and exits before C-library UB. Non-null inputs use real strcat.
    instrumented, replaced = re.subn(r"\bstrcat\s*\(", "observed_strcat(", body, count=1)
    assert replaced == 1
    guarded, replaced = re.subn(r"\bstrcat\s*\(", "observed_strcat(", guarded_raw, count=1)
    assert replaced == 1
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    if not compilers:
        pytest.skip("GCC/Clang host compilers unavailable")
    show_mutant, replacements = instrumented.replace(".show(0)", ".show(3)"), instrumented.count(".show(0)")
    assert replacements == 17
    for compiler in compilers:
        label = Path(compiler).name
        _compile_run(compiler, tmp_path, instrumented, MAIN, "prefwin_visibility_" + label, True)
        _compile_run(compiler, tmp_path, show_mutant, MAIN, "prefwin_show_three_mutant_" + label, False)
        _compile_run(compiler, tmp_path, guarded, MAIN, "prefwin_guarded_append_mutant_" + label, False)
