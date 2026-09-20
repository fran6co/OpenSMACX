"""Actual category arithmetic controls, not full technology or VC6 ABI proof."""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def braced(text, start):
    depth = 0
    for end in range(text.index("{", start), len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated source body")


def category_body():
    source = (ROOT / "src/technology.cpp").read_text()
    return braced(source, source.index("int __cdecl tech_category(int tech_id) {"))


def run_host(tmp_path, body):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host g++ required")
    header = (ROOT / "src/technology.h").read_text()
    declaration = braced(header, header.index("struct RulesTechnology {")) + ";"
    source = r'''
        #include <algorithm>
        #include <climits>
        #include <cstdint>
        #include <cstring>
        #include <initializer_list>
        #define __cdecl
        typedef char *LPSTR;
        DECLARATION
        RulesTechnology Technology[89];
        BODY
        int main() {
            // Host pointers need not have VC6 width: this exercises the actual
            // typed field accesses and arithmetic, not original record ABI.
            static_assert(sizeof(int) == 4, "32-bit signed values required");
            const int values[] = {INT_MIN, -7, -1, 0, 1, 7, INT_MAX};
            for (int index : {0, 37, 88})
              for (int growth : values) for (int tech : values)
                for (int wealth : values) for (int power : values) {
                    for (auto &record : Technology) {
                        record = RulesTechnology();
                        record.power_value = INT_MAX;
                    }
                    Technology[index].growth_value = growth;
                    Technology[index].tech_value = tech;
                    Technology[index].wealth_value = wealth;
                    Technology[index].power_value = power;
                    unsigned char before[sizeof(Technology)];
                    std::memcpy(before, Technology, sizeof(Technology));
                    const int priorities[] = {growth, tech, wealth, power};
                    int expected = std::max_element(priorities, priorities + 4) - priorities;
                    if (tech_category(index) != expected) return 42;
                    if (std::memcmp(before, Technology, sizeof(Technology))) return 43;
                }
        }
    '''.replace("DECLARATION", declaration).replace("BODY", body)
    cpp = tmp_path / "category.cpp"
    executable = tmp_path / "category"
    cpp.write_text(source)
    result = subprocess.run([compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
                             "-fno-sanitize-recover=undefined", str(cpp), "-o", str(executable)],
                            capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)


def test_ordinary_external_definition_precedes_its_same_tu_caller():
    cpp = (ROOT / "src/technology.cpp").read_text()
    header = (ROOT / "src/technology.h").read_text()
    definition = "int __cdecl tech_category(int tech_id) {"
    assert cpp.count(definition) == 1
    assert cpp.index(definition) < cpp.index("void __cdecl say_tech(LPSTR output")
    assert "int __cdecl tech_category(int tech_id);" in header
    assert not re.search(r"tech_category\([^)]*\)\s*\{", header)
    prefix = cpp[:cpp.index(definition)].rsplit("\n", 1)[-1]
    assert not prefix.strip()  # No forcing export/inline decoration.


def test_actual_body_preserves_signed_maximum_and_first_tie(tmp_path):
    result = run_host(tmp_path, category_body())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("field", ["tech_value", "wealth_value", "power_value"])
def test_tie_priority_damage_is_rejected(tmp_path, field):
    body = category_body()
    before = f"Technology[tech_id].{field} > compare"
    assert body.count(before) == 1
    result = run_host(tmp_path, body.replace(before, before.replace(" > ", " >= ")))
    assert result.returncode == 42, result.stderr


@pytest.mark.parametrize("before,after", [
    ("int compare =", "unsigned compare ="),
    ("Technology[tech_id].power_value > compare", "Technology[tech_id].wealth_value > compare"),
    ("Technology[tech_id]", "Technology[0]"),
])
def test_signedness_field_and_index_damage_is_rejected(tmp_path, before, after):
    body = category_body()
    assert before in body
    result = run_host(tmp_path, body.replace(before, after))
    assert result.returncode == 42, result.stderr
