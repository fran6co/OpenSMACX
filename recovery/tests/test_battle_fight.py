"""C++98 controls extracted from the installed battle_fight source owner.

These bounded host fixtures check control flow, live references and owner
interfaces. They do not establish native layout or whole-game acceptance.
All application definitions come from src/; missing definitions fail.
"""
from pathlib import Path
import shutil
import subprocess
import tempfile
import re

import pytest


ROOT = Path(__file__).resolve().parents[2]
OWNER = ROOT / "src" / "veh.cpp"
FIXTURES = ROOT / "recovery" / "tests" / "fixtures" / "battle_fight"


def _extract_definition(source: str, name: str, prefix: str = "static ") -> str:
    starts = []
    for match in re.finditer(r"\bstatic\b[^;{}]*\b" + re.escape(name) + r"\s*\(", source, re.S):
        open_paren = source.find("(", match.start(), match.end())
        depth = 0
        close_paren = -1
        for index in range(open_paren, len(source)):
            if source[index] == "(":
                depth += 1
            elif source[index] == ")":
                depth -= 1
                if depth == 0:
                    close_paren = index
                    break
        if close_paren < 0:
            continue
        after = close_paren + 1
        while after < len(source) and source[after].isspace():
            after += 1
        if source.startswith("{", after):
            starts.append((match.start(), after))
    assert starts, "expected genuine owner definition missing from src/veh.cpp: " + name
    start, brace = starts[0]
    assert source.startswith(prefix, start), "owner definition lost its typed source shape: " + name

    # Count braces while ignoring comments and literals, so nested lambdas or
    # explanatory comments cannot truncate the extracted C++ definition.
    depth = 0
    index = brace
    state = "code"
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == '/' and nxt == '/':
                state = "line_comment"
                index += 2
                continue
            if char == '/' and nxt == '*':
                state = "block_comment"
                index += 2
                continue
            if char == '"':
                state = "string"
            elif char == "'":
                state = "character"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    result = source[start:index + 1]
                    assert name + "(" in result
                    return result
        elif state == "line_comment":
            if char == "\n":
                state = "code"
        elif state == "block_comment":
            if char == '*' and nxt == '/':
                state = "code"
                index += 2
                continue
        elif state == "string":
            if char == "\\":
                index += 2
                continue
            if char == '"':
                state = "code"
        elif state == "character":
            if char == "\\":
                index += 2
                continue
            if char == "'":
                state = "code"
        index += 1
    raise AssertionError("unterminated owner definition: " + name)


def _compile_run(compiler: str, fixture_prefix: str, actual_definitions: str,
                 fixture_main: str = "", expected_run: int = 0):
    if "/* ACTUAL_STATUS_SETTER */" in fixture_prefix:
        fixture_prefix = fixture_prefix.replace("/* ACTUAL_STATUS_SETTER */", _actual_status_setter())
    with tempfile.TemporaryDirectory(prefix="osmx-battle-pipeline-") as temp:
        cpp = Path(temp) / "actual_source_fixture.cpp"
        exe = Path(temp) / "actual_source_fixture"
        cpp.write_text(fixture_prefix + actual_definitions + fixture_main)
        built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
             str(cpp), "-o", str(exe)],
            text=True, capture_output=True, check=False,
        )
        assert built.returncode == 0, (
            compiler + " compile rc=" + str(built.returncode) + "\n" +
            built.stdout + built.stderr
        )
        ran = subprocess.run([str(exe)], check=False)
        if expected_run is None:
            assert ran.returncode != 0, compiler + " mutant unexpectedly ran with rc=0"
        else:
            assert ran.returncode == expected_run, (
                compiler + " run rc=" + str(ran.returncode) +
                ", expected=" + str(expected_run)
            )
        return ran.returncode


def _compilers():
    found = [shutil.which(name) for name in ("g++", "clang++")]
    found = [compiler for compiler in found if compiler]
    if not found:
        pytest.skip("neither GCC nor Clang is installed")
    return found


def _extract_seven_argument_owner(source: str) -> str:
    for match in re.finditer(
        r"\bint\s+__cdecl\s+battle_fight\s*\(([^)]*)\)\s*\{", source, re.S
    ):
        parameters = match.group(1)
        if parameters.count(",") != 6:
            continue
        brace = source.find("{", match.start(), match.end())
        # Reuse the same C++ comment/string-aware brace walk as the helper
        # extractor, starting at this overload's opening brace.
        depth = 0
        index = brace
        state = "code"
        while index < len(source):
            char = source[index]
            nxt = source[index + 1] if index + 1 < len(source) else ""
            if state == "code":
                if char == "/" and nxt == "/":
                    state = "line_comment"
                    index += 2
                    continue
                if char == "/" and nxt == "*":
                    state = "block_comment"
                    index += 2
                    continue
                if char == '"': state = "string"
                elif char == "'": state = "character"
                elif char == "{": depth += 1
                elif char == "}":
                    depth -= 1
                    if depth == 0:
                        return source[match.start():index + 1]
            elif state == "line_comment":
                if char == "\n": state = "code"
            elif state == "block_comment":
                if char == "*" and nxt == "/":
                    state = "code"
                    index += 2
                    continue
            elif state == "string":
                if char == "\\":
                    index += 2
                    continue
                if char == '"': state = "code"
            elif state == "character":
                if char == "\\":
                    index += 2
                    continue
                if char == "'": state = "code"
            index += 1
    raise AssertionError("complete seven-argument battle_fight definition missing from src/veh.cpp")


PRE_ROUND_FIXTURE = (FIXTURES / "pre_round_fixture.cpp").read_text()

PRE_ROUND_MAIN = (FIXTURES / "pre_round_main.cpp").read_text()

ROUTE_FIXTURE = (FIXTURES / "route_fixture.cpp").read_text()

ROUTE_MAIN = (FIXTURES / "route_main.cpp").read_text()

ARTILLERY_FIXTURE = (FIXTURES / "artillery_fixture.cpp").read_text()

ARTILLERY_MAIN = (FIXTURES / "artillery_main.cpp").read_text()

REMOVAL_FIXTURE = (FIXTURES / "removal_fixture.cpp").read_text()

REMOVAL_MAIN = (FIXTURES / "removal_main.cpp").read_text()


def test_actual_entry_early_return_and_artillery_vs_normal_dispatch():
    source = OWNER.read_text()
    # The pre-round helper owns the planet-buster early exit; post-prompt owns
    # the exact mutable-mode equality that selects artillery versus normal.
    pre_round = "struct BattleFightPreRoundStateV7 {\n    int round_scan_gate_minus_24;\n    int detachment_roll_budget_minus_80;\n    int damage_band_scale_minus_6c;\n    int post_hit_handoff_gate_minus_48;\n    int status_animation_flag_minus_88;\n    int transition_type_minus_54;\n    int early_return;\n    int return_value;\n};\n"
    pre_round += _extract_definition(source, "battle_fight_remaining_hit_points_v7")
    pre_round += _extract_definition(source, "battle_fight_prepare_pre_round_v7")
    route = _extract_definition(source, "battle_fight_normal_post_prompt_v1")
    route_enum = "enum BattleFightPostPromptRouteV1 {\n    BATTLE_FIGHT_POST_PROMPT_SOURCE_MODE_V1 = 0,\n    BATTLE_FIGHT_POST_PROMPT_NORMAL_V1 = 1\n};\n"
    for compiler in _compilers():
        _compile_run(compiler, PRE_ROUND_FIXTURE, pre_round, PRE_ROUND_MAIN)
        _compile_run(compiler, ROUTE_FIXTURE, route_enum + route, ROUTE_MAIN)


def test_actual_source_mode_artillery_stack_routing():
    source = OWNER.read_text()
    roll = "struct BattleFightArtilleryRoll { int damage; int minimum_remaining; };\n"
    roll += _extract_definition(source, "battle_fight_artillery_roll_v2")
    cleanup = _extract_definition(source, "battle_fight_artillery_cleanup")
    show = _extract_definition(source, "battle_fight_should_show_artillery")
    mode = _extract_definition(source, "battle_fight_source_mode")
    for compiler in _compilers():
        _compile_run(compiler, ARTILLERY_FIXTURE, roll + cleanup + show + mode,
                     ARTILLERY_MAIN)


def test_actual_seven_argument_owner_wires_the_full_pipeline():
    source = OWNER.read_text()
    body = _extract_seven_argument_owner(source)
    for required_stage in (
        "battle_fight_normal_post_prompt_v1(",
        "battle_fight_source_mode(",
        "battle_fight_mid_owner_v7(",
        "battle_fight_after_a44d_compose_v3(",
    ):
        assert required_stage in body, "seven-argument owner lost stage: " + required_stage
    assert "PENDING_BODY" not in body


def test_actual_linked_stack_removal_walk_and_mutant_control():
    source = OWNER.read_text()
    helper = _extract_definition(source, "battle_fight_remove_damaged_stack_v22")
    for compiler in _compilers():
        _compile_run(compiler, REMOVAL_FIXTURE, helper, REMOVAL_MAIN)

        good_walk = """int vehicle_id = frame_minus_18_vehicle_id;
        int predecessor = Vehs[frame_minus_8_vehicle_id].prev_veh_id_stack;
        while (predecessor >= 0) {
            vehicle_id = predecessor;
            predecessor = Vehs[vehicle_id].prev_veh_id_stack;
        }"""
        bad_walk = """int vehicle_id = Vehs[frame_minus_8_vehicle_id].prev_veh_id_stack;
        while (vehicle_id >= 0) {
            vehicle_id = Vehs[vehicle_id].prev_veh_id_stack;
        }"""
        assert helper.count(good_walk) == 1, "removal helper no longer exposes the tested predecessor walk"
        mutant = helper.replace(good_walk, bad_walk)
        # The fixture's assertion is the damage control: the sentinel-overwrite
        # mutant must compile, then fail at runtime with a nonzero exit status.
        _compile_run(compiler, REMOVAL_FIXTURE, mutant, REMOVAL_MAIN,
                     expected_run=None)


def test_artillery_status_store_damage_control():
    source = OWNER.read_text()
    mode = _extract_definition(source, "battle_fight_source_mode")
    assert "status_animation_flag" not in mode
    marker = "ConsoleStatusWin.set_field_15D4(1);"
    assert mode.count(marker) == 1
    roll = "struct BattleFightArtilleryRoll { int damage; int minimum_remaining; };\n"
    roll += _extract_definition(source, "battle_fight_artillery_roll_v2")
    cleanup = _extract_definition(source, "battle_fight_artillery_cleanup")
    show = _extract_definition(source, "battle_fight_should_show_artillery")
    for compiler in _compilers():
        # The old composition's missing StatusWin write must fail at flush_input.
        mutant = mode.replace(marker, "/* omitted native status store */")
        _compile_run(compiler, ARTILLERY_FIXTURE, roll + cleanup + show + mutant,
                     ARTILLERY_MAIN, expected_run=None)



def _extract_struct(source: str, name: str) -> str:
    match = re.search(r"\bstruct\s+" + re.escape(name) + r"\s*\{", source)
    assert match, "actual owner structure missing: " + name
    opening = source.index("{", match.start(), match.end())
    depth = 0
    tokens = r"//[^\n]*|/\*.*?\*/|\"(?:\\.|[^\"\\])*\"|'(?:\\.|[^'\\])*'|[{}]"
    for token in re.finditer(tokens, source[opening:], re.S):
        if token.group() == "{":
            depth += 1
        elif token.group() == "}":
            depth -= 1
            if depth == 0:
                closing = opening + token.end()
                assert source[closing:].lstrip().startswith(";")
                return source[match.start():closing] + ";\n"
    raise AssertionError("actual owner structure is unterminated: " + name)


def _actual_alias_context():
    source = OWNER.read_text()
    body = _extract_seven_argument_owner(source)
    declarations = []
    for variable, argument in (("combat_type", "source_mode"),
                               ("report_selector", "allow_interceptor")):
        matches = re.findall(r"\bint\s*&?\s*" + variable + r"\s*=\s*" + argument + ";", body)
        assert len(matches) == 1, "actual owner slot binding missing or ambiguous: " + variable
        declarations.append(matches[0])
    construction = re.search(r"BattleFightRoundFrameV4\s+round_frame\s*\([^;]+\);", body)
    assert construction, "actual owner lost its round frame constructor"
    context = _extract_struct(source, "BattleFightRoundFrameV4")
    main = ("int main() {\nint source_mode=3,allow_interceptor=1,dir=4,current_stack_head=2;\n" +
            "\n".join(declarations) + "\n" + construction.group() + r'''
    assert(&round_frame.source_mode==&source_mode);
    assert(&round_frame.execute_display==&allow_interceptor);
    assert(&round_frame.mutable_second_argument==&dir);
    assert(&round_frame.selected_defender_id==&current_stack_head);
    source_mode=13; allow_interceptor=7; dir=6; current_stack_head=9;
    assert(round_frame.source_mode==13 && round_frame.execute_display==7);
    assert(round_frame.mutable_second_argument==6 && round_frame.selected_defender_id==9);
    round_frame.source_mode=21; round_frame.execute_display=5;
    round_frame.mutable_second_argument=1; round_frame.selected_defender_id=11;
    assert(source_mode==21 && allow_interceptor==5 && dir==1 && current_stack_head==11);
    return 0;
}
''')
    return context, main


def test_round_frames_alias_actual_argument_and_stack_storage():
    context, main = _actual_alias_context()
    for compiler in _compilers():
        _compile_run(compiler, "#include <assert.h>\n", context, main)


@pytest.mark.parametrize("member", ["source_mode", "execute_display", "mutable_second_argument", "selected_defender_id"])
def test_round_reference_copy_mutants_are_detected(member):
    context, main = _actual_alias_context()
    pattern = r"\bint\s*&\s*" + member + r"\b"
    assert len(re.findall(pattern, context)) == 1
    mutant = re.sub(pattern, "int " + member, context)
    for compiler in _compilers():
        _compile_run(compiler, "#include <assert.h>\n", mutant, main, expected_run=None)


@pytest.mark.parametrize("variable", ["combat_type", "report_selector"])
def test_outer_argument_alias_copy_mutants_are_detected(variable):
    context, main = _actual_alias_context()
    pattern = r"\bint\s*&\s*" + variable + r"\b"
    assert len(re.findall(pattern, main)) == 1
    mutant = re.sub(pattern, "int " + variable, main)
    for compiler in _compilers():
        _compile_run(compiler, "#include <assert.h>\n", context, mutant, expected_run=None)


def _signed_api_declarations():
    header = (ROOT / "src" / "veh.h").read_text()
    source = OWNER.read_text()
    prototype = re.search(r"(?m)^\w+\s+__cdecl\s+best_defender\s*\([^;{}]*\)\s*;", header)
    definition = re.search(r"(?m)^\w+\s+__cdecl\s+best_defender\s*\([^;{}]*\)\s*\{", source)
    assert prototype and definition, "actual best_defender declaration/definition missing"
    return prototype.group(), definition.group().rstrip().removesuffix("{").rstrip() + ";"


def _compile_signed_api(compiler, header, implementation, expected_success=True):
    fixture = ("#include <stdint.h>\n#define __cdecl\ntypedef int BOOL;\n" +
               header + "\n" + implementation + "\n" +
               "int main() { int (*result)(int,int,BOOL)=&best_defender; (void)result; return 0; }\n")
    with tempfile.TemporaryDirectory(prefix="osmx-battle-api-") as temp:
        cpp = Path(temp) / "actual_api.cpp"
        cpp.write_text(fixture)
        built = subprocess.run([compiler,"-std=c++98","-Wall","-Wextra","-Werror","-fsyntax-only",str(cpp)],
                               text=True,capture_output=True,check=False)
    if expected_success:
        assert built.returncode == 0, built.stdout + built.stderr
    else:
        assert built.returncode != 0 and "error:" in built.stderr, "unsigned return mutation was not rejected"


def test_best_defender_signed_api_matches_actual_source_mapping():
    from match_recovery.source import read_text
    header, implementation = _signed_api_declarations()
    records = read_text(OWNER.read_text(), OWNER)
    record = [row for row in records if row.address == 0x5044D0]
    assert len(record) == 1
    assert record[0].name == "?best_defender@@YAHHHH@Z"
    assert record[0].symbol == "?best_defender@@YAHHHH@Z"
    for compiler in _compilers():
        _compile_signed_api(compiler, header, implementation)


@pytest.mark.parametrize("changed", ["header", "definition", "both"])
def test_unsigned_best_defender_api_mutants_are_detected(changed):
    header, implementation = _signed_api_declarations()
    if changed in ("header", "both"):
        assert header.startswith("int __cdecl best_defender")
        header = header.replace("int __cdecl best_defender", "uint32_t __cdecl best_defender", 1)
    if changed in ("definition", "both"):
        assert implementation.startswith("int __cdecl best_defender")
        implementation = implementation.replace("int __cdecl best_defender", "uint32_t __cdecl best_defender", 1)
    for compiler in _compilers():
        _compile_signed_api(compiler, header, implementation, expected_success=False)


def test_artillery_wrong_owner_store_is_detected():
    source = OWNER.read_text()
    mode = _extract_definition(source, "battle_fight_source_mode")
    marker = "ConsoleStatusWin.set_field_15D4(1);"
    assert mode.count(marker) == 1
    roll = "struct BattleFightArtilleryRoll { int damage; int minimum_remaining; };\n"
    roll += _extract_definition(source, "battle_fight_artillery_roll_v2")
    cleanup = _extract_definition(source, "battle_fight_artillery_cleanup")
    show = _extract_definition(source, "battle_fight_should_show_artillery")
    mutant = mode.replace(marker, "ConsoleStatusWin.draws = 1; /* wrong storage */")
    for compiler in _compilers():
        _compile_run(compiler, ARTILLERY_FIXTURE, roll+cleanup+show+mutant, ARTILLERY_MAIN, expected_run=None)



def _actual_status_setter():
    header = (ROOT / "src" / "statuswin.h").read_text()
    match = re.search(r"\bvoid\s+set_field_15D4\s*\([^{}]*\)\s*\{[^{}]*\}", header)
    assert match, "actual StatusWin inline accessor missing"
    return match.group()


def test_status_accessor_wrong_member_mutant_is_detected():
    source = OWNER.read_text()
    mode = _extract_definition(source, "battle_fight_source_mode")
    roll = "struct BattleFightArtilleryRoll { int damage; int minimum_remaining; };\n"
    roll += _extract_definition(source, "battle_fight_artillery_roll_v2")
    cleanup = _extract_definition(source, "battle_fight_artillery_cleanup")
    show = _extract_definition(source, "battle_fight_should_show_artillery")
    setter = _actual_status_setter()
    assert setter.count("field_15D4_") == 1
    mutated_setter = setter.replace("field_15D4_", "draws")
    fixture = ARTILLERY_FIXTURE.replace("/* ACTUAL_STATUS_SETTER */", mutated_setter)
    for compiler in _compilers():
        _compile_run(compiler, fixture, roll+cleanup+show+mode, ARTILLERY_MAIN, expected_run=None)
