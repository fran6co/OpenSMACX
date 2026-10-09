"""Actual-source controls for pointer-backed Strings callers and Probe menus."""

from pathlib import Path
import re
import shutil
import subprocess
import tempfile


def repository_root():
    for candidate in Path(__file__).resolve().parents:
        if (candidate / "src/probe.cpp").is_file():
            return candidate
    raise AssertionError("could not locate OpenSMACX source root")


ROOT = repository_root()
PROBE_SOURCE = ROOT / "src/probe.cpp"
HELPER_NAME = "probe_native_selector2_prepare_local_text"
HOST_PREAMBLE = r"""
#include <cassert>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>
#include <utility>
#include <vector>
typedef char *LPSTR;
struct Base { int queue_production_id[1]; };
struct RulesFacility { LPSTR name; };
struct VehiclePrototype { LPSTR veh_name; };
static Base Bases[1];
static RulesFacility Facility[134];
static VehiclePrototype VehPrototypes[2];
struct Strings {
    std::map<LPSTR, LPSTR> resolved;
    LPSTR last_input;
    int calls;
    Strings() : last_input(nullptr), calls(0) {}
    LPSTR get(LPSTR text) {
        ++calls;
        last_input = text;
        return resolved[text];
    }
};
static Strings StringTable;
struct Dialogs {
    std::vector<std::pair<std::string, int> > items;
    void item(LPSTR text, int item_id) {
        items.push_back(std::make_pair(std::string(text), item_id));
    }
};
struct Popup {
    Dialogs dialog_list;
    int selected;
    Dialogs &dialogs() { return dialog_list; }
    int exec(int, void *) { return selected; }
};
static int label_offset;
static LPSTR label_text;
static LPSTR label_get(int offset) {
    label_offset = offset;
    return label_text;
}
static LPSTR parsed_text;
static int parse_say(int, LPSTR text, int, int) {
    parsed_text = StringTable.get(text);
    return 0;
}
"""

HOST_MAIN = r"""
int main() {
    (void)&label_get;
    (void)&parse_say;
    char local_text[80];
    static char facility_name[] = "Facility Pointer";
    static char localized_facility[] = "Localized Facility";
    Facility[1].name = facility_name;
    StringTable.resolved[facility_name] = localized_facility;
    Bases[0].queue_production_id[0] = -1;
    probe_native_selector2_prepare_local_text(0, local_text);
    assert(std::strcmp(local_text, "Localized Facility") == 0);
    assert(StringTable.calls == 1 && StringTable.last_input == facility_name);

    static char prototype_name[] = "Prototype Name";
    VehPrototypes[1].veh_name = prototype_name;
    Bases[0].queue_production_id[0] = 1;
    StringTable.calls = 0;
    probe_native_selector2_prepare_local_text(0, local_text);
    assert(std::strcmp(local_text, "Prototype Name") == 0);
    assert(StringTable.calls == 0);
    std::puts("actual-source pointer producer control passed");
}
"""

MENU_CONTROL_MAIN = r"""
int main() {
    (void)Bases;
    (void)VehPrototypes;
    static char facility_three[] = "Facility Three Pointer";
    static char localized_three[] = "Localized Facility Three";
    static char facility_seven[] = "Facility Seven Pointer";
    static char localized_seven[] = "Localized Facility Seven";
    static char localized_label[] = "Localized Label 404";
    Facility[3].name = facility_three;
    Facility[7].name = facility_seven;
    StringTable.resolved[facility_three] = localized_three;
    StringTable.resolved[facility_seven] = localized_seven;
    label_text = localized_label;

    Popup popup;
    popup.selected = 7;
    int choice = 3;
    FACILITY_ITEM_STATEMENT
    LABEL_ITEM_STATEMENT
    assert(popup.dialog_list.items.size() == 2);
    assert(popup.dialog_list.items[0].first == "Localized Facility Three");
    assert(popup.dialog_list.items[0].second == 3);
    assert(popup.dialog_list.items[1].first == "Localized Label 404");
    assert(popup.dialog_list.items[1].second == 0x62);
    assert(label_offset == 0x1010 / 4);

    int local_project_cursor = -1;
    POPUP_SELECTION_STATEMENT
    SELECTED_SLOT_PARSE_STATEMENT
    assert(local_project_cursor == 7);
    assert(StringTable.last_input == facility_seven);
    assert(parsed_text == localized_seven);
    assert(StringTable.calls == 2);
    std::puts("actual-source menu item and selected-slot controls passed");
}
"""


def extract_function(source, name):
    match = re.search(r"static void\s+" + re.escape(name) + r"\s*\(", source)
    assert match, f"missing {name} in tracked src/probe.cpp"
    opening = source.index("{", match.end())
    depth = 0
    quote = None
    escaped = False
    line_comment = False
    block_comment = False
    index = opening
    while index < len(source):
        char = source[index]
        next_char = source[index + 1] if index + 1 < len(source) else ""
        if line_comment:
            if char == "\n":
                line_comment = False
        elif block_comment:
            if char == "*" and next_char == "/":
                block_comment = False
                index += 1
        elif quote is not None:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == quote:
                quote = None
        elif char == "/" and next_char == "/":
            line_comment = True
            index += 1
        elif char == "/" and next_char == "*":
            block_comment = True
            index += 1
        elif char in ('"', "'"):
            quote = char
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return source[match.start():index + 1]
        index += 1
    raise AssertionError(f"unterminated body for {name}")


def compile_source(helper, compiler, directory, label):
    source_path = directory / f"{label}.cpp"
    binary = directory / label
    source_path.write_text(HOST_PREAMBLE + "\n" + helper + "\n" + HOST_MAIN)
    return subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(source_path), "-o", str(binary)],
        capture_output=True, text=True,
    ), binary


def extract_statement(source, pattern, description):
    match = re.search(pattern, source, re.DOTALL)
    assert match, f"missing {description} in tracked src/probe.cpp"
    return match.group(0)


def test_probe_menu_uses_original_label_and_facility_pointer_sources():
    text = PROBE_SOURCE.read_text()
    assert "popup.dialogs().item(label_get(0x1010 / 4), 0x62);" in text
    assert "StringTable.get(Facility[choice].name), choice);" in text
    assert "strcat(StringTemp, label_get(0x3E4 / 4));" in text
    assert "StringTable.get(0x62)" not in text
    assert "StringTable.get(choice)" not in text
    assert "StringTable.get(0xF9)" not in text


def test_assigned_rules_and_label_callers_keep_pointer_values_typed():
    files = (
        "netdaemon.cpp", "popup.cpp", "diplopop.cpp", "infowin.cpp",
        "stringstruct.cpp",
    )
    for filename in files:
        text = (ROOT / "src" / filename).read_text()
        assert "StringTable.get(reinterpret_cast<int>" not in text
        assert "StringTable.get(static_cast<int>(reinterpret_cast" not in text

    netdaemon = (ROOT / "src/netdaemon.cpp").read_text()
    assert "LPSTR project_name = StringTable.get(project.name);" in netdaemon
    assert "LPSTR related_name =" in netdaemon
    assert "parse_say(3, project_name, -1, -1);" in netdaemon

    stringstruct = (ROOT / "src/stringstruct.cpp").read_text()
    assert "LPSTR *labels = Labels.strings_ptr;" in stringstruct
    assert "strcpy(PronounBuffer, StringTable.get(labels[code]));" in stringstruct


def test_actual_probe_pointer_producer_and_menu_flow_compile_and_run():
    source = PROBE_SOURCE.read_text()
    helper = extract_function(source, HELPER_NAME)
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers, "neither g++ nor clang++ is available for the host control"

    with tempfile.TemporaryDirectory() as temporary:
        directory = Path(temporary)
        for index, compiler in enumerate(compilers):
            result, binary = compile_source(
                helper, compiler, directory, f"probe_pointer_{index}")
            assert result.returncode == 0, result.stderr
            run = subprocess.run([str(binary)], capture_output=True, text=True)
            assert run.returncode == 0, run.stderr
            assert "actual-source pointer producer control passed" in run.stdout

            bypass_mutant = helper.replace(
                "strcat(local_text, StringTable.get(Facility[facility_id].name));",
                "strcat(local_text, Facility[facility_id].name);",
            )
            assert bypass_mutant != helper
            mutant_compile, mutant_binary = compile_source(
                bypass_mutant, compiler, directory,
                f"probe_get_bypass_mutant_{index}")
            assert mutant_compile.returncode == 0, mutant_compile.stderr
            mutant_run = subprocess.run(
                [str(mutant_binary)], capture_output=True, text=True)
            assert mutant_run.returncode != 0

            menu_main = MENU_CONTROL_MAIN.replace(
                "FACILITY_ITEM_STATEMENT",
                extract_statement(
                    source,
                    r"popup\.dialogs\(\)\.item\(\s*"
                    r"StringTable\.get\(Facility\[choice\]\.name\),\s*choice\);",
                    "facility item source statement",
                ),
            ).replace(
                "LABEL_ITEM_STATEMENT",
                extract_statement(
                    source,
                    r"popup\.dialogs\(\)\.item\(label_get\(0x1010\s*/\s*4\),\s*0x62\);",
                    "label item source statement",
                ),
            ).replace(
                "POPUP_SELECTION_STATEMENT",
                extract_statement(
                    source,
                    r"local_project_cursor\s*=\s*popup\.exec\(0,\s*nullptr\);",
                    "popup selection assignment",
                ),
            ).replace(
                "SELECTED_SLOT_PARSE_STATEMENT",
                extract_statement(
                    source,
                    r"parse_say\(1,\s*Facility\[local_project_cursor\]\.name,\s*-1,\s*-1\);",
                    "selected facility parser call",
                ),
            )
            menu_source = directory / f"probe_menu_control_{index}.cpp"
            menu_binary = directory / f"probe_menu_control_{index}"
            menu_source.write_text(HOST_PREAMBLE + "\n" + menu_main)
            menu_compile = subprocess.run(
                [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                 str(menu_source), "-o", str(menu_binary)],
                capture_output=True, text=True,
            )
            assert menu_compile.returncode == 0, menu_compile.stderr
            menu_run = subprocess.run(
                [str(menu_binary)], capture_output=True, text=True)
            assert menu_run.returncode == 0, menu_run.stderr
            assert "actual-source menu item and selected-slot controls passed" \
                in menu_run.stdout
