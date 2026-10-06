"""Current PickTech owner statements with explicit host API/component doubles.

The full class declaration comes from src/picktech.h and all implemented
members/helpers come from src/picktech.cpp on every run. The fixture owns only
external doubles and test inputs/assertions, never a recovered PickTech body.
Host projection deliberately omits the native sizeof assertion: pointer width,
subobject layout, vtable entries, actual callees, native restoration and game
runtime remain separate evidence obligations.
"""

from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/picktech.cpp"
HEADER = ROOT / "src/picktech.h"
FIXTURE = Path(__file__).with_name("picktech_host_fixture.inc")

# Consume complete comments and quoted literals before observing brace tokens.
# In particular, draw_tech contains literal "{" and "}" caption delimiters.
LEXEMES = re.compile(
    r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|'
    r"'(?:\\.|[^'\\])*'|[{}]"
)
METHODS = (
    "PickTech::PickTech()",
    "void PickTech::close()",
    "PickTech::~PickTech()",
    "void PickTech::on_mouse_move(",
    "int PickTech::draw_item(",
    "void PickTech::UNK1(",
    "void PickTech::on_left_click(",
    "void PickTech::on_right_click(",
    "int PickTech::on_key_click(",
    "static const char *picktech_name(",
    "static void picktech_add_label(",
    "int PickTech::draw_tech(",
    "static void picktech_append_rule_annotation(",
    "int PickTech::pick(",
)
DAMAGES = (
    "unsigned_speed", "weapon_label", "reactor_label", "caption_setter",
    "eager_annotation", "lookup_order", "close_fields",
)


def extract_braced(source: str, marker: str, *, declaration=False) -> str:
    match = re.search(r"(?m)^" + re.escape(marker), source)
    assert match is not None, f"missing production declaration/body: {marker}"
    start = match.start()
    opening = source.index("{", start)
    depth = 0
    for token in LEXEMES.finditer(source, opening):
        if token.group() == "{":
            depth += 1
        elif token.group() == "}":
            depth -= 1
            if depth == 0:
                end = token.end()
                if declaration:
                    assert source[end] == ";"
                    end += 1
                return source[start:end]
    raise AssertionError(f"unterminated production declaration/body: {marker}")


def replace_once(source: str, old: str, new: str) -> str:
    assert source.count(old) == 1, f"damage anchor changed: {old!r}"
    return source.replace(old, new, 1)


def damage_statements(methods: str, damage: str) -> str:
    if damage == "unsigned_speed":
        return replace_once(methods, "static_cast<int>(rule.speed)",
                            "static_cast<int>(static_cast<int8_t>(rule.speed))")
    if damage == "weapon_label":
        return replace_once(
            methods,
            "                secondary_buffer[1] = '\\0';",
            "                /* damaged: no one-character weapon truncation */",
        )
    if damage == "reactor_label":
        return replace_once(
            methods,
            "            strcat(secondary_buffer, label_get(0x13c / 4));\n"
            "            secondary_buffer[1] = '\\0';",
            "            strcat(secondary_buffer, label_get(0x13c / 4));\n"
            "            /* damaged: no one-character reactor truncation */",
        )
    if damage == "caption_setter":
        return replace_once(methods, "    caption_height_ = 29;",
                            "    set_caption_height(29);")
    if damage == "eager_annotation":
        return replace_once(
            methods,
            '    if (has_annotation) {\n        strcat(StringTemp, "+");',
            "    if (has_annotation) {\n"
            "        if (label_index >= 0) label_get(label_index);\n"
            "        if (name_is_string_reference)\n"
            "            picktech_name(const_cast<char *>(name));\n"
            '        strcat(StringTemp, "+");',
        )
    if damage == "lookup_order":
        methods = replace_once(
            methods,
            '    strcat(StringTemp, "(");\n    if (label_index >= 0) {',
            "    const char *early_name = name_is_string_reference\n"
            "        ? picktech_name(const_cast<char *>(name)) : name;\n"
            '    strcat(StringTemp, "(");\n    if (label_index >= 0) {',
        )
        return replace_once(
            methods,
            "strcat(StringTemp, name_is_string_reference\n"
            "            ? picktech_name(const_cast<char *>(name)) : name);",
            "strcat(StringTemp, early_name);",
        )
    if damage == "close_fields":
        return replace_once(
            methods,
            "void PickTech::close() {\n"
            "    column_count_ = 0;\n    row_count_ = 0;\n"
            "    selected_index_ = 0;\n    selection_active_ = 0;",
            "void PickTech::close() {\n"
            "    /* damaged: picker fields not cleared */",
        )
    raise AssertionError(f"unknown damage: {damage}")


def projected_unit(damage=None) -> str:
    product = SOURCE.read_text()
    header = HEADER.read_text()
    # This assertion belongs to the native header, not to this host ABI.
    assert 'static_assert(sizeof(PickTech) == 0xC5C' in header
    declaration = extract_braced(header, "class PickTech : public GraphicWin",
                                declaration=True)
    methods = "\n\n".join(extract_braced(product, marker) for marker in METHODS)
    if damage is not None:
        methods = damage_statements(methods, damage)
    fixture = FIXTURE.read_text()
    assert "class PickTech :" not in fixture
    assert "PickTech::" not in fixture
    for marker in ("/* PICKTECH_PRODUCT_DECLARATION */",
                   "/* PICKTECH_PRODUCT_IMPLEMENTATION */"):
        assert fixture.count(marker) == 1
    return fixture.replace("/* PICKTECH_PRODUCT_DECLARATION */", declaration).replace(
        "/* PICKTECH_PRODUCT_IMPLEMENTATION */", methods
    )


def compile_and_run(tmp_path, compiler_name, damage=None):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    unit = tmp_path / "picktech_behavior.cpp"
    unit.write_text(projected_unit(damage))
    executable = tmp_path / "picktech_behavior"
    compiled = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
         str(unit), "-o", str(executable)],
        capture_output=True, text=True, timeout=60,
    )
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    # Leak enumeration is unavailable under the managed ptrace environment.
    # Address and undefined-behavior instrumentation remain enabled.
    environment = dict(os.environ, ASAN_OPTIONS="detect_leaks=0",
                       UBSAN_OPTIONS="halt_on_error=1")
    return subprocess.run([str(executable)], capture_output=True, text=True,
                          env=environment, timeout=20)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_picktech_full_current_owner_behavior(tmp_path, compiler_name):
    result = compile_and_run(tmp_path, compiler_name)
    assert result.returncode == 0, result.stdout + result.stderr
    assert "controls passed" in result.stdout


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("damage", DAMAGES)
def test_picktech_reviewed_body_damage_is_detected(tmp_path, compiler_name, damage):
    result = compile_and_run(tmp_path, compiler_name, damage)
    # A compiler/sanitizer crash is not the intended behavior-control witness.
    assert result.returncode != 0, f"damage escaped controls: {damage}"
    assert "Assertion" in result.stderr, result.stdout + result.stderr
