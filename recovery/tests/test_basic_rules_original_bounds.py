"""Host controls for the complete product rules loader and its real clamp.

Service doubles supply parser values; product declarations and implementations
are extracted unchanged. These tests do not prove native layout or byte matching.
"""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def block(text, start, end):
    begin = text.index(start)
    finish = text.index(end, begin) + len(end)
    return text[begin:finish]


def product_source():
    header = (ROOT / "src/alpha.h").read_text()
    declarations = "\n".join(
        block(header, "struct " + name + " {", "\n};")
        for name in ("RulesBasic", "RulesWorldbuilder")
    )
    clamp = block(
        (ROOT / "src/general.h").read_text(),
        "MEASURED inline int __cdecl range(", "\n}",
    )
    loader = block(
        (ROOT / "src/alpha.cpp").read_text(),
        "BOOL __cdecl read_basic_rules() {", "\n}",
    )
    return declarations + "\n" + clamp + "\n" + loader


PRELUDE = r"""
#include <stdint.h>
#include <stdlib.h>
#define __cdecl
#define MEASURED
typedef int BOOL;
enum { TRIAD_LAND=0, TRIAD_SEA=1, TRIAD_AIR=2, AlphaxFileID=0 };
static int parsed_value, opens, fail_open;
static int text_open(int, const char*) { return ++opens == fail_open; }
static void text_get() {}
static int text_item_number() { return parsed_value; }
static int tech_item() { return 0; }
"""


HARNESS = r"""
int main(int argc, char **argv) {
    if (argc != 2) return 99;
    parsed_value=atoi(argv[1]);
    if (read_basic_rules() != 0 || opens != 2) return 10;
    const int strictness=parsed_value<0 ? 0 : parsed_value>2 ? 2 : parsed_value;
    const int toggle=parsed_value<-1 ? -1 : parsed_value>1 ? 1 : parsed_value;
    if (Rules.retool_strictness != static_cast<uint32_t>(strictness)) return 11;
    if (Rules.tgl_probe_can_steal_tech != toggle) return 12;
    if (Rules.tgl_human_always_contact_net != toggle) return 13;
    if (Rules.tgl_humans_always_contact_pbem != toggle) return 14;
    if (Rules.tgl_oblit_base_atrocity != toggle) return 15;
    // Both parser-open failure paths return without continuing that section.
    opens=0; fail_open=1; Rules.retool_strictness=77;
    if (read_basic_rules() != 1 || opens != 1 || Rules.retool_strictness != 77)
        return 16;
    opens=0; fail_open=2; WorldBuilder.land_base=88;
    if (read_basic_rules() != 1 || opens != 2 || WorldBuilder.land_base != 88)
        return 17;
    return 0;
}
"""


def compile_control(tmp_path, product):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable for bounded host control")
    # Definitions follow the extracted declarations, before the actual loader.
    marker = "BOOL __cdecl read_basic_rules() {"
    product = product.replace(marker, "RulesBasic Rules;\nRulesWorldbuilder WorldBuilder;\n" + marker, 1)
    source = tmp_path / "rules.cpp"
    binary = tmp_path / "rules"
    source.write_text(PRELUDE + product + HARNESS)
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(binary)], check=True, capture_output=True)
    return binary


def test_complete_loader_preserves_original_signed_bounds(tmp_path):
    binary = compile_control(tmp_path, product_source())
    for value in (-2147483648, -2, -1, 0, 1, 2, 3, 2147483647):
        subprocess.run([str(binary), str(value)], check=True)


@pytest.mark.parametrize("field,old,new,value", [
    ("retool_strictness", "0, 2", "0, 3", 3),
    ("tgl_probe_can_steal_tech", "-1, 1", "0, 1", -1),
    ("tgl_human_always_contact_net", "-1, 1", "0, 1", -1),
    ("tgl_humans_always_contact_pbem", "-1, 1", "0, 1", -1),
    ("tgl_oblit_base_atrocity", "-1, 1", "0, 1", -1),
])
def test_controls_reject_each_superseded_fix(tmp_path, field, old, new, value):
    product = product_source()
    anchor = f"Rules.{field} = range(text_item_number(), {old});"
    assert product.count(anchor) == 1
    mutated = product.replace(anchor, f"Rules.{field} = range(text_item_number(), {new});", 1)
    binary = compile_control(tmp_path, mutated)
    assert subprocess.run([str(binary), str(value)], check=False).returncode != 0
