"""Compile and execute the extracted InfoWin::setup_text implementation."""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest


SCRIPT_PATH = Path(__file__).resolve()
REPOSITORY_ROOT = (SCRIPT_PATH.parents[2] if len(SCRIPT_PATH.parents) > 2
                   else SCRIPT_PATH.parent)
DEFAULT_ROOT = (REPOSITORY_ROOT if (REPOSITORY_ROOT / "src/infowin.cpp").is_file()
                else Path.cwd())
ROOT = Path(os.environ.get("OPENSMACX_ROOT", DEFAULT_ROOT)).resolve()
HARNESS = SCRIPT_PATH.with_name("infowin_setup_text_harness.cpp")


def _function_end(source: str, start: int) -> int:
    brace = source.index("{", start)
    depth = 0
    state = "code"
    escaped = False
    i = brace
    while i < len(source):
        c = source[i]
        n = source[i + 1] if i + 1 < len(source) else ""
        if state == "line_comment":
            if c == "\n":
                state = "code"
        elif state == "block_comment":
            if c == "*" and n == "/":
                state = "code"
                i += 1
        elif state in ("string", "char"):
            if escaped:
                escaped = False
            elif c == "\\":
                escaped = True
            elif (state == "string" and c == '"') or (state == "char" and c == "'"):
                state = "code"
        elif c == "/" and n == "/":
            state = "line_comment"
            i += 1
        elif c == "/" and n == "*":
            state = "block_comment"
            i += 1
        elif c == '"':
            state = "string"
        elif c == "'":
            state = "char"
        elif c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                return i + 1
        i += 1
    raise AssertionError("unterminated InfoWin::setup_text body")


def _actual_source() -> tuple[str, str]:
    product = ROOT / "src/infowin.cpp"
    product_text = product.read_text()
    method = "void InfoWin::setup_text()"
    if method not in product_text:
        raise AssertionError(f"{method} missing from {product}")
    start = product_text.index(method)
    end = _function_end(product_text, start)
    body = product_text[start:end]
    if "BODY GOES HERE." in body:
        raise AssertionError(f"{product} still contains the setup_text placeholder")
    helper = product_text.find("static void infowin_append_label")
    if helper < 0 or helper > start:
        raise AssertionError("actual InfoWin helper definitions are missing")
    return product_text[helper:end], f"source:{product}"


def _run_host_control(source: str, macro: str) -> subprocess.CompletedProcess[str]:
    compiler = shutil.which(os.environ.get("CXX", "g++"))
    if not compiler:
        pytest.skip("no host C++ compiler available")
    harness = HARNESS.read_text()
    assert harness.count("/*ACTUAL_SOURCE*/") == 1
    translation_unit = harness.replace("/*ACTUAL_SOURCE*/", source)
    with tempfile.TemporaryDirectory(prefix="infowin-setup-control-") as temp:
        tmp = Path(temp)
        cpp = tmp / "control.cpp"
        exe = tmp / "control-test"
        cpp.write_text(translation_unit)
        build = subprocess.run(
            [compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror", "-pedantic",
             "-fsanitize=address,undefined", "-fno-omit-frame-pointer", "-g",
             "-D" + macro, str(cpp), "-o", str(exe)],
            text=True, capture_output=True, timeout=45,
        )
        assert build.returncode == 0, f"{build.stdout}\n{build.stderr}"
        env = dict(os.environ)
        env.setdefault("ASAN_OPTIONS", "detect_leaks=0:halt_on_error=1")
        env.setdefault("UBSAN_OPTIONS", "halt_on_error=1:print_stacktrace=1")
        return subprocess.run([str(exe)], text=True, capture_output=True,
                              timeout=15, env=env)


def test_source_extraction_uses_whole_method_and_owner_helpers() -> None:
    source, source_label = _actual_source()
    assert source_label.startswith("source:")
    assert "static void infowin_append_label" in source
    assert "static int infowin_best_stat_faction" in source
    assert "void InfoWin::setup_text()" in source
    assert "row_count_[pane] = displayed_rows;" in source
    assert "text_lengths_[pane] +=" in source
    assert "const int kind = pane_states_[pane] % 8;" in source
    assert "switch (kind)" in source
    assert "BODY GOES HERE." not in source


def test_setup_text_compiles_and_preserves_owner_behavior() -> None:
    compiler = shutil.which(os.environ.get("CXX", "g++"))
    if not compiler:
        pytest.skip("no host C++ compiler available")
    source, source_label = _actual_source()
    harness = HARNESS.read_text()
    assert harness.count("/*ACTUAL_SOURCE*/") == 1
    translation_unit = harness.replace("/*ACTUAL_SOURCE*/", source)
    with tempfile.TemporaryDirectory(prefix="infowin-setup-host-") as temp:
        tmp = Path(temp)
        cpp = tmp / "host.cpp"
        exe = tmp / "host-test"
        cpp.write_text(translation_unit)
        build = subprocess.run(
            [compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror", "-pedantic",
             "-fsanitize=address,undefined",
             "-fno-omit-frame-pointer", "-g", str(cpp), "-o", str(exe)],
            text=True, capture_output=True, timeout=45,
        )
        assert build.returncode == 0, f"{source_label}\n{build.stdout}\n{build.stderr}"
        env = dict(os.environ)
        env.setdefault("ASAN_OPTIONS", "detect_leaks=0:halt_on_error=1")
        env.setdefault("UBSAN_OPTIONS", "halt_on_error=1:print_stacktrace=1")
        run = subprocess.run([str(exe)], text=True, capture_output=True,
                             timeout=15, env=env)
        assert run.returncode == 0, f"{source_label}\n{run.stdout}\n{run.stderr}"


def test_default_tail_geometry_with_forced_row_count() -> None:
    source, _ = _actual_source()
    anchor = "int displayed_rows = 0;"
    assert source.count(anchor) == 1
    # Invalid negative states normally publish zero rows, so this focused
    # control seeds one row to execute the otherwise dormant common-tail test.
    forced = source.replace(anchor, "int displayed_rows = 1;", 1)
    run = _run_host_control(forced, "INFOWIN_TAIL_ALIGNMENT_CONTROL")
    assert run.returncode == 0, f"{run.stdout}\n{run.stderr}"


def test_active_pane_reload_native_control() -> None:
    source, _ = _actual_source()
    run = _run_host_control(source, "INFOWIN_ACTIVE_PANE_CONTROL")
    assert run.returncode == 0, f"{run.stdout}\n{run.stderr}"


@pytest.mark.parametrize("damage", ["old_lookup", "wrong_brace", "late_brace", "missing_close"])
def test_state_zero_heading_rejects_compiling_damage(damage: str) -> None:
    source, _ = _actual_source()
    start = source.index("case 0: {")
    end = source.index("case 1: {",start)
    branch = source[start:end]
    if damage == "old_lookup":
        good,bad = 'strcat(text, ": ");','strcat(text, label_get(0x63));'
    elif damage == "wrong_brace":
        good,bad = 'strcat(text, "{");','strcat(text, "}");'
    elif damage == "late_brace":
        good = 'strcat(text, "{");\n            say_year(text);'
        bad = 'say_year(text);\n            strcat(text, "{");'
    else:
        good,bad = 'strcat(text, "}");','(void)0;'
    assert good in branch,damage
    damaged = source[:start]+branch.replace(good,bad,1)+source[end:]
    result = _run_host_control(damaged,"INFOWIN_HEADING_CONTROL")
    assert result.returncode == 29 and "WITNESS: state-zero heading text and callback order" in result.stderr, result.stdout+result.stderr


def test_pinned_state_zero_heading_literal_and_call_order() -> None:
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    configured = os.environ.get("OPENSMACX_IMAGE")
    candidates = ([Path(configured)] if configured else []) + [
        parent / ".opensmacx/game/terranx_original.exe" for parent in (ROOT,*ROOT.parents)]
    path = next((p for p in candidates if p.is_file()),None)
    if path is None:
        pytest.skip("pinned proprietary image unavailable")
    image = PeImage(path.read_bytes(),"01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    start,end = 0x456300,0x45838D
    instructions = {}
    cursor = start
    for instruction in Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(start,end)),start):
        assert instruction.address == cursor
        cursor += instruction.size
        instructions[instruction.address] = (instruction.mnemonic,instruction.op_str)
    assert cursor == end
    anchors = {
        0x4564F6:("mov","edx, dword ptr [0x9b90f8]"),
        0x45650C:("mov","eax, dword ptr [edx]"), # zero-offset label table slot
        0x45650F:("call","0x6169a0"),
        0x456519:("call","0x645470"),
        0x456521:("push","0x682e94"),
        0x456527:("call","0x645470"),
        0x45652F:("push","0x68518c"),
        0x456535:("call","0x645470"),
        0x45653E:("call","0x5c89b0"),
        0x456546:("push","0x685190"),
        0x45654C:("call","0x645470"),
    }
    for address,expected in anchors.items():
        assert instructions[address] == expected,hex(address)
    for address,literal in ((0x682E94,b": \0"),(0x68518C,b"{\0"),(0x685190,b"}\0")):
        assert image.read(Span(address,address+len(literal))) == literal


@pytest.mark.parametrize(
    "needle,replacement,case",
    [
        ("PlayersData[faction].tech_commerce_bonus + 1",
         "PlayersData[static_cast<int>(DiploFrictionFactionID)].tech_commerce_bonus + 1",
         "commerce owner"),
        ("PlayersData[faction].diplo_treaties[other]",
         "PlayersData[other].diplo_treaties[faction]", "treaty direction"),
        ("0x41c / 4", "0x41c", "case-six label units"),
        ("unk_78[4 + sample]", "unk_78[sample]", "region sample window"),
        ("FactionRankingsUnk[faction_index]) *",
         "FactionRankingsUnk[faction_index - 1]) *", "rank-seven bar score"),
        ("static_cast<int>(GameDataStateStorage.fields_.faction_rankings_[7]));",
         "static_cast<int>(GameDataStateStorage.fields_.faction_rankings_[6]));", "shared ranking owner slot"),
        ("case 0: value = static_cast<int>(PlayersData[candidate].tech_ranking); break;",
         "case 0: value = PlayersData[candidate].unk_78[5]; break;", "typed technology field"),
    ],
)
def test_source_damage_controls_are_detected(needle: str, replacement: str,
                                             case: str) -> None:
    compiler = shutil.which(os.environ.get("CXX", "g++"))
    if not compiler:
        pytest.skip("no host C++ compiler available")
    source, _ = _actual_source()
    assert needle in source, f"proposal/source lost mutation anchor: {case}"
    damaged = source.replace(needle, replacement, 1)
    harness = HARNESS.read_text().replace("/*ACTUAL_SOURCE*/", damaged)
    with tempfile.TemporaryDirectory(prefix="infowin-setup-damage-") as temp:
        tmp = Path(temp)
        cpp = tmp / "damage.cpp"
        exe = tmp / "damage-test"
        cpp.write_text(harness)
        build = subprocess.run(
            [compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror", "-pedantic",
             "-fsanitize=address,undefined",
             "-fno-omit-frame-pointer", str(cpp), "-o", str(exe)],
            text=True, capture_output=True, timeout=45,
        )
        assert build.returncode == 0, f"damage harness compile failed: {case}\n{build.stderr}"
        run = subprocess.run([str(exe)], text=True, capture_output=True,
                             timeout=15,
                             env={**os.environ, "ASAN_OPTIONS": "detect_leaks=0"})
        assert run.returncode != 0, f"damage control escaped: {case}"


@pytest.mark.parametrize(
    "needle,replacement,case",
    [
        ("Players[faction].title_leader", "Players[static_cast<int>(DiploFrictionFactionID)].title_leader",
         "case-three title owner"),
        ("Players[faction].name_leader", "Players[static_cast<int>(DiploFrictionFactionID)].name_leader",
         "case-three leader owner"),
    ],
)
def test_case_three_owner_damage_controls(needle: str, replacement: str,
                                          case: str) -> None:
    compiler = shutil.which(os.environ.get("CXX", "g++"))
    if not compiler:
        pytest.skip("no host C++ compiler available")
    source, _ = _actual_source()
    start = source.index("case 3: {")
    end = source.index("case 4: {", start)
    case_three = source[start:end]
    assert needle in case_three, f"case-three mutation anchor missing: {case}"
    damaged_case = case_three.replace(needle, replacement, 1)
    damaged = source[:start] + damaged_case + source[end:]
    harness = HARNESS.read_text().replace("/*ACTUAL_SOURCE*/", damaged)
    with tempfile.TemporaryDirectory(prefix="infowin-setup-case3-damage-") as temp:
        tmp = Path(temp)
        cpp = tmp / "damage.cpp"
        exe = tmp / "damage-test"
        cpp.write_text(harness)
        build = subprocess.run(
            [compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror", "-pedantic",
             "-fsanitize=address,undefined", "-fno-omit-frame-pointer", str(cpp),
             "-o", str(exe)],
            text=True, capture_output=True, timeout=45,
        )
        assert build.returncode == 0, f"damage harness compile failed: {case}\n{build.stderr}"
        run = subprocess.run([str(exe)], text=True, capture_output=True, timeout=15,
                             env={**os.environ, "ASAN_OPTIONS": "detect_leaks=0"})
        assert run.returncode != 0, f"case-three damage escaped: {case}"


def test_case_three_gender_damage_control() -> None:
    compiler = shutil.which(os.environ.get("CXX", "g++"))
    if not compiler:
        pytest.skip("no host C++ compiler available")
    source, _ = _actual_source()
    start = source.index("case 3: {")
    end = source.index("case 4: {", start)
    case_three = source[start:end]
    needle = "GenderDefault = Players[faction].is_leader_female;"
    assert case_three.count(needle) == 2, "expected both case-three gender stores"
    damaged_case = case_three.replace(
        needle, "GenderDefault = Players[static_cast<int>(DiploFrictionFactionID)].noun_gender;")
    damaged = source[:start] + damaged_case + source[end:]
    harness = HARNESS.read_text().replace("/*ACTUAL_SOURCE*/", damaged)
    with tempfile.TemporaryDirectory(prefix="infowin-setup-case3-gender-") as temp:
        tmp = Path(temp)
        cpp = tmp / "damage.cpp"
        exe = tmp / "damage-test"
        cpp.write_text(harness)
        build = subprocess.run(
            [compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror", "-pedantic",
             "-fsanitize=address,undefined", "-fno-omit-frame-pointer", str(cpp),
             "-o", str(exe)],
            text=True, capture_output=True, timeout=45,
        )
        assert build.returncode == 0, f"gender damage harness compile failed:\n{build.stderr}"
        run = subprocess.run([str(exe)], text=True, capture_output=True, timeout=15,
                             env={**os.environ, "ASAN_OPTIONS": "detect_leaks=0"})
        assert run.returncode != 0, "case-three gender damage escaped"
