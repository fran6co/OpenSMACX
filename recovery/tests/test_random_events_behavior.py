"""Actual-source host controls, not native ABI or game acceptance."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
TESTS = ROOT / "recovery/tests/random_events"
from match_recovery.source import Mode, read_file

COMPILERS = [c for c in ("g++", "clang++") if shutil.which(c)]
FIXTURES = (
    "cases_0_5.cpp", "case6.cpp", "cases_7_8.cpp", "cases_9_11.cpp",
    "cases_12_13.cpp", "cases_15_16.cpp", "cases_17_18.cpp",
    "cases_19_21.cpp", "case14.cpp", "native_services.cpp",
)


def masked(source):
    chars = list(source); i = 0; state = "code"; quote = ""
    while i < len(source):
        c = source[i]; n = source[i + 1] if i + 1 < len(source) else ""
        if state == "code":
            if c == "/" and n == "/":
                chars[i] = chars[i + 1] = " "; i += 2; state = "line"; continue
            if c == "/" and n == "*":
                chars[i] = chars[i + 1] = " "; i += 2; state = "block"; continue
            if c in ("'", '"'):
                quote = c; chars[i] = " "; state = "quote"
        elif state == "line":
            if c == "\n": state = "code"
            else: chars[i] = " "
        elif state == "block":
            if c == "*" and n == "/":
                chars[i] = chars[i + 1] = " "; i += 2; state = "code"; continue
            if c != "\n": chars[i] = " "
        else:
            if c == "\\":
                chars[i] = " "
                if i + 1 < len(source) and source[i + 1] != "\n":
                    chars[i + 1] = " "; i += 2; continue
            elif c == quote: chars[i] = " "; state = "code"
            elif c != "\n": chars[i] = " "
        i += 1
    return "".join(chars)


def block(source, start, op="{", cl="}"):
    code = masked(source); opening = code.find(op, start); depth = 0
    assert opening >= 0
    for pos in range(opening, len(code)):
        if code[pos] == op: depth += 1
        elif code[pos] == cl:
            depth -= 1
            if not depth:
                end = pos + 1
                if cl == "}" and source[end:end + 1] == ";": end += 1
                return source[start:end]
    raise AssertionError("unclosed source block")


def function(source, name):
    code = masked(source)
    for match in re.finditer(r"\b" + re.escape(name) + r"\s*\(", code):
        p = code.find("(", match.start()); depth = 0
        for q in range(p, len(code)):
            if code[q] == "(": depth += 1
            elif code[q] == ")":
                depth -= 1
                if not depth: break
        brace = q + 1
        while brace < len(code) and code[brace].isspace(): brace += 1
        if brace == len(code) or code[brace] != "{": continue
        start = source.rfind("\n", 0, match.start()) + 1
        prior = source[source.rfind("\n", 0, start - 1) + 1:start].strip()
        if prior.startswith("template <"):
            start = source.rfind("\n", 0, start - 1) + 1
        return block(source, start)
    raise AssertionError("source definition missing: " + name)


def struct(source, name):
    code = masked(source)
    match = re.search(r"\bstruct\s+" + re.escape(name) + r"\s*\{", code)
    assert match, name
    return block(source, match.start())


def guarded(source, macro):
    lines = source.splitlines(True)
    start = next(i for i, line in enumerate(lines)
                 if line.strip() == "#ifndef " + macro)
    depth = 0
    for i in range(start, len(lines)):
        item = lines[i].lstrip()
        if item.startswith(("#if ", "#ifdef ", "#ifndef ")): depth += 1
        elif item.startswith("#endif"):
            depth -= 1
            if depth == 0: return "".join(lines[start:i + 1])
    raise AssertionError("unterminated guard: " + macro)


def _definition_parts(definition):
    code = masked(definition)
    opening = code.find("{")
    assert opening >= 0
    closing = len(code.rstrip()) - 1
    assert code[closing] == "}"
    return definition[:opening + 1], definition[opening + 1:closing]


def _mapped_case_block(body, condition):
    code = masked(body)
    match = re.search(r"\bif\s*\(\s*" + re.escape(condition) + r"\s*\)", code)
    assert match, "mapped random_events branch missing: " + condition
    return block(body, match.start())


def _test_case_wrapper(name, signature, branch, context):
    return ("static void " + name + "(" + signature + ") {\n" + context +
            "\n" + branch + "\n}\n")


def source_fragments():
    path = ROOT / "src/game.cpp"
    text = path.read_text(encoding="utf-8")
    rows = [r for r in read_file(path) if r.address == 0x51F2E0]
    assert len(rows) == 1 and rows[0].mode is Mode.BODY
    assert "BODY GOES HERE." not in rows[0].region
    definition = function(text, "random_events")
    header, body = _definition_parts(definition)
    code = masked(body)
    first_case = re.search(r"\bif\s*\(\s*event_kind\s*<=\s*5\s*\)", code)
    assert first_case
    entry_prefix = body[:first_case.start()]
    cases = {
        "actual_cases_0_5.inc": ("random_events_cases_0_5",
            "int event_kind, int selected_faction, int selected_base_arg, int selected_roll",
            "event_kind <= 5"),
        "actual_case6.hpp": ("random_events_case6_from_mapped_body",
            "int event_kind, int selected_faction, int selected_base_arg, int selected_roll, int snapshot_x, int snapshot_y",
            "event_kind == 6"),
        "actual_cases_7_8.hpp": ("random_events_cases_7_8_from_mapped_body",
            "int event_kind, int selected_faction, int selected_base_arg, int selected_roll, int snapshot_x, int snapshot_y",
            "event_kind == 7 || event_kind == 8"),
        "actual_cases_9_11.hpp": ("random_events_native_cases_9_11",
            "int event_kind",
            "event_kind >= 9 && event_kind <= 11"),
        "actual_cases_12_13.inc": ("random_events_cases_12_13",
            "int event_kind, int selected_faction, int selected_base_arg, int selected_roll, int snapshot_x, int snapshot_y",
            "event_kind == 12 || event_kind == 13"),
        "actual_case14.inc": ("random_events_case14_from_mapped_body",
            "int event_kind, int selected_faction, int selected_base_arg, int selected_roll, int snapshot_x, int snapshot_y",
            "event_kind == 14"),
        "actual_cases_15_16.inc": ("random_events_cases_15_16",
            "int event_kind, int selected_faction, int selected_base_arg, int selected_roll, int snapshot_x, int snapshot_y",
            "event_kind == 15 || event_kind == 16"),
        "actual_cases_17_18.inc": ("random_events_cases_17_18",
            "int event_kind, int selected_faction, int selected_base_arg, int selected_roll, int snapshot_x, int snapshot_y",
            "event_kind == 17 || event_kind == 18"),
        "actual_cases_19_21.inc": ("random_events_cases_19_21",
            "int event_kind, int selected_faction, int selected_base_arg, int selected_roll, int snapshot_x, int snapshot_y",
            "event_kind >= 19 && event_kind <= 21"),
    }
    result = {}
    for filename, (name, signature, condition) in cases.items():
        branch = _mapped_case_block(body, condition)
        if condition == "event_kind >= 9 && event_kind <= 11":
            context_for_branch = """const int selected_base_id = 0;
    const int base_roll = 0;
    const int faction_id = 0;
    const int base_x_snapshot = 0;
    const int base_y_snapshot = 0;"""
        elif condition == "event_kind >= 19 && event_kind <= 21":
            context_for_branch = """const int selected_base_id = selected_base_arg;
    (void)selected_roll;
    const int faction_id = selected_faction;
    const int base_x_snapshot = snapshot_x;
    const int base_y_snapshot = snapshot_y;"""
        elif condition == "event_kind <= 5":
            context_for_branch = """const int selected_base_id = selected_base_arg;
    const int base_roll = selected_roll;
    const int faction_id = selected_faction;"""
        else:
            context_for_branch = """const int selected_base_id = selected_base_arg;
    const int base_roll = selected_roll;
    const int faction_id = selected_faction;
    const int base_x_snapshot = snapshot_x;
    const int base_y_snapshot = snapshot_y;"""
        result[filename] = _test_case_wrapper(name, signature, branch, context_for_branch)
    result["actual_entry_prefix.inc"] = entry_prefix
    result["actual_entry_probe.inc"] = (
        "static void random_events_entry_probe(int mode) {\n" + entry_prefix +
        "\nentry_probe_reached = 1;\n" +
        "entry_probe_event = event_kind; entry_probe_faction = faction_id;\n" +
        "entry_probe_base = selected_base_id; entry_probe_roll = base_roll;\n" +
        "entry_probe_x = base_x_snapshot; entry_probe_y = base_y_snapshot;\n}\n")
    result["actual_random_events_definition.inc"] = definition
    return text, rows[0].region, result


def compile_run(tmp_path, compiler, source_text, name):
    source = tmp_path / (name + ".cpp")
    binary = tmp_path / (name + ".exe")
    source.write_text(source_text)
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    "-pedantic", "-I", str(tmp_path), str(source), "-o", str(binary)],
                   check=True, capture_output=True, text=True)
    subprocess.run([str(binary)], check=True, timeout=20, capture_output=True, text=True)


@pytest.mark.parametrize("compiler", COMPILERS)
@pytest.mark.parametrize("fixture", FIXTURES)
def test_reviewed_case_bodies_are_extracted_from_current_owner(tmp_path, compiler, fixture):
    _, _, fragments = source_fragments()
    for name, body in fragments.items():
        (tmp_path / name).write_text(body + "\n")
    compile_run(tmp_path, compiler, (TESTS / fixture).read_text(), fixture)


def test_mapped_entry_source_has_snapshot_and_prefix_before_case_selection():
    text, region, fragments = source_fragments()
    assert "void __cdecl random_events(int mode)" in region
    assert fragments["actual_entry_prefix.inc"]
    assert "const int base_x_snapshot = selected_base.x;" in fragments["actual_entry_prefix.inc"]
    assert "const int base_y_snapshot = selected_base.y;" in fragments["actual_entry_prefix.inc"]
    assert fragments["actual_entry_prefix.inc"].index("base_x_snapshot") < fragments["actual_entry_prefix.inc"].index("parse_num")
    assert fragments["actual_entry_prefix.inc"].index("base_y_snapshot") < fragments["actual_entry_prefix.inc"].index("parse_says")
    assert "RandomEventsDispatchInput" not in text


def test_test_fragments_select_each_mapped_case_branch():
    _, _, fragments = source_fragments()
    expected = {
        "actual_cases_0_5.inc": "event_kind <= 5",
        "actual_case6.hpp": "event_kind == 6",
        "actual_cases_7_8.hpp": "event_kind == 7 || event_kind == 8",
        "actual_cases_9_11.hpp": "event_kind >= 9 && event_kind <= 11",
        "actual_cases_12_13.inc": "event_kind == 12 || event_kind == 13",
        "actual_case14.inc": "event_kind == 14",
        "actual_cases_15_16.inc": "event_kind == 15 || event_kind == 16",
        "actual_cases_17_18.inc": "event_kind == 17 || event_kind == 18",
        "actual_cases_19_21.inc": "event_kind >= 19 && event_kind <= 21",
    }
    assert set(expected).issubset(fragments)
    for filename, condition in expected.items():
        assert condition in fragments[filename]
    assert "random_events_dispatch_cases" not in fragments["actual_random_events_definition.inc"]
    assert "random_events_case6(" not in fragments["actual_random_events_definition.inc"]


def test_case15_16_cache_access_is_owned_by_the_mapped_branch():
    _, _, fragments = source_fragments()
    branch = fragments["actual_cases_15_16.inc"]
    assert "tile.bit2" in branch
    assert "cache_word = &tile.bit_visible[0]" in branch
    assert "clear_pass < 7" in branch
    assert "random_events_blight_cache_word" not in branch


@pytest.mark.parametrize("damage", [
    "case0_event", "case12_reserves", "case21_popup", "entry_snapshot", "entry_reseed",
])
def test_controls_reject_damaged_actual_source(tmp_path, damage):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ is unavailable")
    _, _, fragments = source_fragments()
    if damage in ("entry_snapshot", "entry_reseed"):
        fixture = "native_services.cpp"
        fragment = "actual_entry_probe.inc"
        old, new = (
            ("const int base_x_snapshot = selected_base.x;",
             "const int base_x_snapshot = selected_base.x + 1;")
            if damage == "entry_snapshot" else
            ("13u * static_cast<unsigned int>(current_turn)",
             "14u * static_cast<unsigned int>(current_turn)")
        )
    else:
        fixture, fragment, old, new = {
            "case0_event": ("cases_0_5.cpp", "actual_cases_0_5.inc",
                            "base.event |= BEVENT_BUMPER;", "base.event &= ~BEVENT_BUMPER;"),
            "case12_reserves": ("cases_12_13.cpp", "actual_cases_12_13.inc",
                                "PlayersData[faction_id].energy_reserves = new_reserves;",
                                "PlayersData[faction_id].energy_reserves = 0;"),
            "case21_popup": ("cases_19_21.cpp", "actual_cases_19_21.inc",
                             "const int popup_base_id = faction_id == ConsoleGlobal.faction_id_\n                ? selected_base_id : -1;",
                             "const int popup_base_id = -1;"),
        }[damage]
    assert fragments[fragment].count(old) >= 1
    changed = dict(fragments)
    changed[fragment] = fragments[fragment].replace(old, new, 1)
    for name, body in changed.items(): (tmp_path / name).write_text(body + "\n")
    with pytest.raises(subprocess.CalledProcessError) as failure:
        compile_run(tmp_path, compiler, (TESTS / fixture).read_text(), "damaged-control")
    assert failure.value.returncode < 0, "damage must be detected at execution, not by compile failure"
