"""Bounded actual-source controls for Console's shared version-string slots.

The C++ harness compiles the real show_version definition and the real
menu/key switch arms extracted from console.cpp. The key control exercises the
arm in a small switch wrapper, not the complete on_key_click function or the
game runtime. Runtime pointer replacement proves these consumers load the
source-owned pointer objects rather than embedding the historical strings.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "console.cpp"


def extract_definition(source: str, signature: str) -> str:
    if source.count(signature) != 1:
        raise AssertionError(f"expected one {signature!r} in console.cpp")
    start = source.index(signature)
    opening = source.find("{", start)
    if opening < 0:
        raise AssertionError(f"missing body for {signature}")
    depth = 0
    state = "code"
    i = opening
    while i < len(source):
        c = source[i]
        n = source[i + 1] if i + 1 < len(source) else ""
        if state == "code":
            if c == "/" and n == "/":
                state = "line"
                i += 2
                continue
            if c == "/" and n == "*":
                state = "block"
                i += 2
                continue
            if c == '"':
                state = "string"
            elif c == "'":
                state = "char"
            elif c == "{":
                depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0:
                    return source[start : i + 1]
        elif state == "line":
            if c == "\n":
                state = "code"
        elif state == "block":
            if c == "*" and n == "/":
                state = "code"
                i += 2
                continue
        else:
            if c == "\\":
                i += 2
                continue
            if (state == "string" and c == '"') or (state == "char" and c == "'"):
                state = "code"
        i += 1
    raise AssertionError(f"unclosed body for {signature}")


def case_region_for_call(body: str, call: str) -> str:
    if body.count(call) != 1:
        raise AssertionError(f"expected one branch call {call!r} in selected function")
    call_at = body.index(call)
    start = body.rfind("case ", 0, call_at)
    if start < 0:
        raise AssertionError(f"missing case label for {call}")
    end = body.find("break;", start)
    if end < 0:
        raise AssertionError(f"missing break after {call}")
    return body[start : end + len("break;")]


def _global_declaration(source: str, declaration: str) -> str:
    matches = re.findall(rf"(?m)^{re.escape(declaration)}$", source)
    if len(matches) != 1:
        raise AssertionError(f"expected one source declaration: {declaration}")
    return matches[0]


def actual_fragments(source: str):
    declarations = "\n".join(
        _global_declaration(source, line)
        for line in (
            'char ConsoleVersionReleaseText[] = "RELEASE 2.0";',
            'char ConsoleVersionBuildDateText[] = "20-Dec-1999";',
            "char *ConsoleVersionRelease = ConsoleVersionReleaseText;",
            "char *ConsoleVersionBuildDate = ConsoleVersionBuildDateText;",
            'char ConsoleStrAcLogoSm[] = "ac_logo_sm.pcx";  // 0x0068AA7C',
            'char ConsoleStrVersionControlX[] = "VERSIONCONTROLX";  // 0x0068AA8C',
        )
    )
    show = extract_definition(source, "void Console::show_version()")
    menu = extract_definition(source, "void Console::menu_proc(int command)")
    key = extract_definition(source, "int Console::on_key_click(int a1, int a2)")
    menu_case = case_region_for_call(menu, "parse_says(0, ConsoleVersionRelease, -1, -1);")
    key_case = case_region_for_call(key, "parse_says(0, ConsoleVersionRelease, -1, -1);")
    assert "switch (a2)" in key and "native_command_done:" in key
    return declarations, show, menu_case, key_case


HARNESS_TEMPLATE = r'''
#include <cassert>
#include <cstddef>
#include <cstring>

DECLARATIONS
typedef int (*NullCallback)(void);
char ScriptPopupFile[] = "SCRIPT";

struct ParseRecord { int selector; char *text; int third; int fourth; };
struct PopupRecord { char *script; char *label; int flags; char *picture; NullCallback callback; };
static ParseRecord parsed[8];
static PopupRecord popup;
static int parse_count;
static int trace[8];
static int trace_count;
static int menu_tail;
static int key_tail;

void parse_says(int selector, char *text, int third, int fourth) {
    parsed[parse_count++] = ParseRecord{selector, text, third, fourth};
    trace[trace_count++] = selector + 1;
}
int popp(char *script, char *label, int flags, char *picture, NullCallback callback) {
    popup = PopupRecord{script, label, flags, picture, callback};
    trace[trace_count++] = 3;
    return 0;
}

class Console {
public:
    void show_version();
};

SHOW_DEFINITION

static void run_menu(int command) {
    switch (command) {
MENU_CASE
    default: break;
    }
    ++menu_tail;
}

static void run_key(int a1, int a2) {
    switch (a2) {
KEY_CASE
    default: break;
    }
    (void)a1;
    ++key_tail;
}

static void clear_log() { parse_count = 0; trace_count = 0; }
static void check_version_popup() {
    assert(parse_count == 2);
    assert(parsed[0].selector == 0 && parsed[0].text == ConsoleVersionRelease);
    assert(parsed[1].selector == 1 && parsed[1].text == ConsoleVersionBuildDate);
    assert(parsed[0].third == -1 && parsed[0].fourth == -1);
    assert(parsed[1].third == -1 && parsed[1].fourth == -1);
    assert(trace_count == 3 && trace[0] == 1 && trace[1] == 2 && trace[2] == 3);
    assert(popup.script == ScriptPopupFile);
    assert(popup.label == ConsoleStrVersionControlX);
    assert(popup.flags == 0 && popup.picture == ConsoleStrAcLogoSm);
    assert(popup.callback == 0);
}

int main() {
    char replacement_release[] = "runtime release";
    char replacement_date[] = "runtime build date";
    ConsoleVersionRelease = replacement_release;
    ConsoleVersionBuildDate = replacement_date;

    Console console;
    clear_log();
    console.show_version();
    check_version_popup();

    clear_log();
    run_menu(0x751);
    check_version_popup();
    assert(menu_tail == 1);
    clear_log();
    run_menu(0x750);
    assert(parse_count == 0 && trace_count == 0 && menu_tail == 2);

    clear_log();
    run_key(0, 0x20073);
    check_version_popup();
    assert(key_tail == 1);
    clear_log();
    run_key(0xA1, 0x20073);
    check_version_popup();
    assert(key_tail == 2);
    clear_log();
    run_key(0, 0x20074);
    assert(parse_count == 0 && trace_count == 0 && key_tail == 3);
    return 0;
}
'''


def harness_for(source: str) -> str:
    declarations, show, menu_case, key_case = actual_fragments(source)
    return (
        HARNESS_TEMPLATE.replace("DECLARATIONS", declarations)
        .replace("SHOW_DEFINITION", show)
        .replace("MENU_CASE", menu_case)
        .replace("KEY_CASE", key_case)
    )


def compile_and_run(tmp_path: Path, compiler: str, source: str, *, expect_success: bool):
    cpp = tmp_path / "console_version_control.cpp"
    exe = tmp_path / "console_version_control"
    cpp.write_text(harness_for(source), encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", "-pedantic", str(cpp), "-o", str(exe)],
        text=True,
        capture_output=True,
        check=False,
        timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(exe)], text=True, capture_output=True, check=False, timeout=15)
    if expect_success:
        assert ran.returncode == 0, ran.stdout + ran.stderr
    else:
        assert ran.returncode != 0, "the source mutation escaped the runtime oracle"


def compiler_names():
    names = [name for name in ("g++", "clang++") if shutil.which(name)]
    assert len(names) == 2, "both GCC and Clang are required for this host control"
    return names


@pytest.mark.parametrize("compiler", compiler_names())
def test_actual_version_consumers_use_replaceable_slots_and_preserve_call_order(tmp_path, compiler):
    source = SOURCE.read_text(encoding="utf-8")
    assert "case 0x751:" in actual_fragments(source)[2]
    assert "case 0x20073:" in actual_fragments(source)[3]
    assert "if (a1" not in actual_fragments(source)[3]
    compile_and_run(tmp_path, compiler, source, expect_success=True)


@pytest.mark.parametrize("compiler", compiler_names())
@pytest.mark.parametrize(
    "mutation",
    ("wrong_slot", "wrong_menu_key", "wrong_raw_key", "extra_a1_guard", "parse_order"),
)
def test_actual_source_mutations_are_caught(tmp_path, compiler, mutation):
    source = SOURCE.read_text(encoding="utf-8")
    if mutation == "wrong_slot":
        target = "parse_says(0, ConsoleVersionRelease, -1, -1);"
        assert source.count(target) == 3
        source = source.replace(target, "parse_says(0, ConsoleVersionBuildDate, -1, -1);", 1)
    elif mutation == "wrong_menu_key":
        menu = extract_definition(source, "void Console::menu_proc(int command)")
        changed = menu.replace("case 0x751:", "case 0x752:", 1)
        assert changed != menu
        source = source.replace(menu, changed, 1)
    elif mutation == "wrong_raw_key":
        key = extract_definition(source, "int Console::on_key_click(int a1, int a2)")
        changed = key.replace("case 0x20073:", "case 0x20074:", 1)
        assert changed != key
        source = source.replace(key, changed, 1)
    elif mutation == "extra_a1_guard":
        key = extract_definition(source, "int Console::on_key_click(int a1, int a2)")
        changed = key.replace("case 0x20073:\n", "case 0x20073:\n    if (a1 != 0xA1) break;\n", 1)
        assert changed != key
        source = source.replace(key, changed, 1)
    else:
        target = (
            "parse_says(0, ConsoleVersionRelease, -1, -1);\n"
            "    parse_says(1, ConsoleVersionBuildDate, -1, -1);"
        )
        replacement = (
            "parse_says(1, ConsoleVersionBuildDate, -1, -1);\n"
            "    parse_says(0, ConsoleVersionRelease, -1, -1);"
        )
        assert target in source
        source = source.replace(target, replacement, 1)
    compile_and_run(tmp_path, compiler, source, expect_success=False)
