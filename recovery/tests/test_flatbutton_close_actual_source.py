"""Actual-source host controls for FlatButton::close.

The harness compiles the current complete mapped method with small service
doubles. It checks reset/callback order and all 27 borrowed default links; it
does not establish VC6 code generation, ABI/layout, external callback behavior,
or whole-application acceptance. The existing close-dispatch suite covers the
broader class lifetime protocol.
"""
from pathlib import Path
import resource
import shutil
import subprocess

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/flatbutton.cpp").is_file())
SOURCE = ROOT / "src/flatbutton.cpp"
SIGNATURE = "void FlatButton::close()"
COMPILERS = [name for name in ("g++", "clang++") if shutil.which(name)]


def _masked(text):
    """Blank comments/literals while retaining offsets and brace positions."""
    out = list(text)
    i = 0
    while i < len(text):
        if text.startswith("//", i):
            end = text.find("\n", i)
            if end < 0:
                end = len(text)
            for j in range(i, end):
                out[j] = " "
            i = end
        elif text.startswith("/*", i):
            end = text.find("*/", i + 2)
            assert end >= 0, "unterminated block comment"
            end += 2
            for j in range(i, end):
                if text[j] != "\n":
                    out[j] = " "
            i = end
        elif text[i] in "\"'":
            quote = text[i]
            end = i + 1
            while end < len(text):
                if text[end] == "\\":
                    end += 2
                elif text[end] == quote:
                    end += 1
                    break
                else:
                    end += 1
            for j in range(i, min(end, len(text))):
                if text[j] != "\n":
                    out[j] = " "
            i = end
        else:
            i += 1
    return "".join(out)


def _actual_close():
    text = SOURCE.read_text(encoding="utf-8")
    masked = _masked(text)
    starts = []
    pos = 0
    while True:
        pos = masked.find(SIGNATURE, pos)
        if pos < 0:
            break
        starts.append(pos)
        pos += len(SIGNATURE)
    assert len(starts) == 1, "expected one current FlatButton::close definition"
    start = starts[0]
    opening = masked.find("{", start + len(SIGNATURE))
    assert opening >= 0
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return text[start:end + 1]
    raise AssertionError("unterminated FlatButton::close body")


HOST = r'''
#include <cassert>
#include <cstdint>
#include <vector>

struct Sprite { int id; };
static Sprite sprite_tokens[256];
Sprite *FlatButtonDefaultsTable[27];
class FlatButton;
static FlatButton *active_button;
static std::vector<int> events;
static int callback_generation;

class Time {
public:
    void close();
};

class BaseButton {
public:
    uint32_t field_A14_;
    uint32_t field_A18_;
    Time time1_;
    void close();
};

class FlatButton : public BaseButton {
public:
    uint32_t field_AB8_;
    uint32_t field_ABC_[3];
    Sprite *border_styles_[3];
    Sprite *field_AD4_[3];
    Sprite *config[27];
    void close();
};

void Time::close() {
    assert(active_button != 0);
    assert(active_button->field_A14_ == 0xA1400001U);
    assert(active_button->field_A18_ == 0xA1800002U);
    assert(active_button->field_AB8_ == 0xAB800003U);
    events.push_back(1);
    // The owner must not have copied defaults before Time::close returns.
    for (int i = 0; i < 27; ++i)
        FlatButtonDefaultsTable[i] = &sprite_tokens[20 + i];
}

void BaseButton::close() {
    assert(active_button != 0);
    assert(events.size() == 1 && events[0] == 1);
    // The derived reset phase is observable at the real base close call.
    assert(field_A14_ == 0 && field_A18_ == 0);
    assert(active_button->field_AB8_ == 0xFFFFFFFFU);
    // The per-column phase follows the qualified base close: its arrays still
    // carry their pre-call values here, even though scalar resets are visible.
    for (int i = 0; i < 3; ++i) {
        assert(active_button->field_ABC_[i] ==
               0x12340000U + static_cast<uint32_t>(i));
        assert(active_button->field_AD4_[i] == &sprite_tokens[40 + i]);
        assert(active_button->border_styles_[i] == &sprite_tokens[50 + i]);
    }
    for (int i = 0; i < 27; ++i)
        assert(active_button->config[i] == &sprite_tokens[1 + i]);
    events.push_back(2);
    // Values installed here must be the ones copied into all 27 links.
    const int first = 80 + callback_generation * 32;
    for (int i = 0; i < 27; ++i)
        FlatButtonDefaultsTable[i] = &sprite_tokens[first + i];
    ++callback_generation;
}

// ACTUAL_CLOSE

static void reset_case(FlatButton &button, int generation) {
    events.clear();
    callback_generation = generation;
    active_button = &button;
    button.field_A14_ = 0xA1400001U;
    button.field_A18_ = 0xA1800002U;
    button.field_AB8_ = 0xAB800003U;
    for (int i = 0; i < 27; ++i) {
        button.config[i] = &sprite_tokens[1 + i];
        FlatButtonDefaultsTable[i] = &sprite_tokens[2 + i];
    }
    for (int i = 0; i < 3; ++i) {
        button.field_ABC_[i] = 0x12340000U + static_cast<uint32_t>(i);
        button.field_AD4_[i] = &sprite_tokens[40 + i];
        button.border_styles_[i] = &sprite_tokens[50 + i];
    }
}

static void check_complete_result(const FlatButton &button, int generation) {
    assert((events == std::vector<int>{1, 2}));
    assert(button.field_A14_ == 0 && button.field_A18_ == 0);
    assert(button.field_AB8_ == 0xFFFFFFFFU);
    for (int i = 0; i < 3; ++i) {
        assert(button.field_ABC_[i] == 0xFFFFFFFFU);
        assert(button.field_AD4_[i] == 0);
        assert(button.border_styles_[i] == 0);
    }
    const int first = 80 + generation * 32;
    for (int group = 0; group < 9; ++group) {
        for (int column = 0; column < 3; ++column) {
            const int index = column + group * 3;
            assert(button.config[index] == &sprite_tokens[first + index]);
        }
    }
}

int main() {
    FlatButton button{};
    reset_case(button, 0);
    button.close();
    check_complete_result(button, 0);

    // A second call verifies every column/group is reset and repopulated from
    // the defaults installed by the callbacks on that invocation.
    reset_case(button, 1);
    button.close();
    check_complete_result(button, 1);
}
'''


def _compile_and_run(tmp_path, compiler, body):
    source = tmp_path / "flatbutton_close.cpp"
    binary = tmp_path / "flatbutton_close"
    source.write_text(HOST.replace("// ACTUAL_CLOSE", body), encoding="utf-8")
    compile_result = subprocess.run(
        [shutil.which(compiler), "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(source), "-o", str(binary)],
        cwd=tmp_path, text=True, capture_output=True, timeout=30,
        preexec_fn=lambda: resource.setrlimit(resource.RLIMIT_CORE, (0, 0)),
    )
    assert compile_result.returncode == 0, compile_result.stderr
    return subprocess.run(
        [str(binary)], cwd=tmp_path, text=True, capture_output=True,
        timeout=10,
        preexec_fn=lambda: resource.setrlimit(resource.RLIMIT_CORE, (0, 0)),
    )


@pytest.mark.parametrize("compiler", COMPILERS)
def test_actual_close_resets_all_links_after_ordered_callbacks(tmp_path, compiler):
    result = _compile_and_run(tmp_path, compiler, _actual_close())
    assert result.returncode == 0, result.stderr


MUTATIONS = (
    ("wrong default source index",
     "config[column + 3] = FlatButtonDefaultsTable[column + 3];",
     "config[column + 3] = FlatButtonDefaultsTable[column + 6];"),
    ("skip final column",
     "column < 3; ++column", "column < 2; ++column"),
    ("call timer after resets",
     "    time1_.close();\n    field_A18_ = 0;\n    field_A14_ = 0;\n    field_AB8_ = 0xFFFFFFFFU;",
     "    field_A18_ = 0;\n    field_A14_ = 0;\n    field_AB8_ = 0xFFFFFFFFU;\n    time1_.close();"),
)


@pytest.mark.parametrize("compiler", COMPILERS)
@pytest.mark.parametrize("label,before,after", MUTATIONS)
def test_compiling_damage_mutations_fail_actual_control(tmp_path, compiler,
                                                        label, before, after):
    body = _actual_close()
    assert body.count(before) == 1, (label, before)
    damaged = body.replace(before, after, 1)
    result = _compile_and_run(tmp_path, compiler, damaged)
    assert result.returncode != 0, label
