"""Actual-source controls for pinned formatter data-read boundaries.

Provider doubles deliberately mutate inputs between operations. This tests the
observed read ordering, not a claim that the real CRT invokes game callbacks.
"""
from pathlib import Path
import runpy
import shutil
import subprocess

import pytest

FIXTURE = runpy.run_path(str(Path(__file__).with_name("test_vehicle_stats_owner_oct21.py")))


def test_source(variant):
    bodies = FIXTURE["production_bodies"]()
    if variant == "cached_reactor":
        body = bodies[0x57D7D0]
        anchor = "    if (VehPrototypes[proto_id].reactor_id > RECT_FISSION) {"
        assert body.count(anchor) == 1
        changed = body.replace(anchor,
            "    uint8_t reactor = VehPrototypes[proto_id].reactor_id;\n"
            "    if (reactor > RECT_FISSION) {")
        anchor = "_itoa(VehPrototypes[proto_id].reactor_id, num_buf, 10);"
        assert changed.count(anchor) == 1
        changed = changed.replace(anchor, "_itoa(reactor, num_buf, 10);")
        assert changed != body
        bodies[0x57D7D0] = changed
    elif variant == "exclusive_triad":
        body = bodies[0x57D8E0]
        anchor = "    }\n    if (get_proto_triad(proto_id) == TRIAD_AIR) {"
        assert body.count(anchor) == 1
        changed = body.replace(anchor,
            "    } else if (get_proto_triad(proto_id) == TRIAD_AIR) {")
        assert changed != body
        bodies[0x57D8E0] = changed
    elif variant == "cached_verbose_offense":
        body = bodies[0x57DAA0]
        anchor = "get_proto_offense_rating(proto_id)"
        assert body.count(anchor) >= 3
        changed = body.replace(anchor, "cached_offense")
        signature = "void __cdecl say_stats(LPSTR stat, int proto_id, LPSTR custom_spacer) {"
        assert changed.count(signature) == 1
        changed = changed.replace(signature, signature +
            "\n    int8_t cached_offense = get_proto_offense_rating(proto_id);")
        assert changed != body
        bodies[0x57DAA0] = changed
    prefix = FIXTURE["PREFIX"]
    anchor = "struct FakeStringTable {"
    assert prefix.count(anchor) == 1
    prefix = prefix.replace(anchor, "static void on_table_get(char *);\n" + anchor)
    anchor = "    table_log[table_calls++] = address;"
    assert prefix.count(anchor) == 1
    prefix = prefix.replace(anchor, anchor + "\n    on_table_get(address);")
    return prefix + HOOKS + "\n" + "\n".join(bodies.values()) + MAIN


# Helper, not a collected test.
test_source.__test__ = False

HOOKS = r'''
static int mutation;
static void on_table_get(char *text) {
  if (mutation == 1 && text == label_explore) {
    offense_rating[0] = -1; defense_rating[0] = -1; Weapon[0].mode = 0;
  }
  if (mutation == 2 && text == label_sea) Chassis[0].triad = TRIAD_AIR;
  if (mutation == 4 && text == label_explore) {
    VehPrototypes[0].plan = PLAN_NAVAL_TRANSPORT;
    VehPrototypes[0].carry_capacity = 5;
  }
}
static char *observed_strcat(char *out, const char *text) {
  char *result = ::strcat(out, text);
  if (mutation == 3 && strcmp(text, "*") == 0) VehPrototypes[0].reactor_id = 4;
  return result;
}
#define strcat observed_strcat
'''

MAIN = r'''
int main() {
  char output[512] = {};
  initialize_case(); mutation = 1;
  VehPrototypes[0].plan = PLAN_RECONNAISANCE;
  say_stats(output, 0, 0);
  assert(strcmp(output, "Explore/Defense, Psi/Psi/2") == 0);

  initialize_case(); mutation = 2; output[0] = 0;
  Chassis[0].triad = TRIAD_SEA;
  say_stats_2(output, 0);
  assert(strcmp(output, "9-7-2 Sea Air") == 0);
  initialize_case(); mutation = 2; output[0] = 0;
  Chassis[0].triad = TRIAD_SEA;
  say_stats(output, 0, 0);
  assert(strcmp(output, "Explore/Defense,  Sea Air") == 0);

  initialize_case(); mutation = 3; output[0] = 0;
  VehPrototypes[0].reactor_id = 2;
  say_stats_3(output, 0);
  assert(strcmp(output, "9-7-2*4") == 0);
  initialize_case(); mutation = 3; output[0] = 0;
  VehPrototypes[0].reactor_id = 2;
  say_stats_2(output, 0);
  assert(strcmp(output, "9-7-2*4") == 0);
  initialize_case(); mutation = 3; output[0] = 0;
  VehPrototypes[0].reactor_id = 2;
  say_stats(output, 0, 0);
  assert(strcmp(output, "Explore/Defense (*4)") == 0);

  initialize_case(); mutation = 4; output[0] = 0;
  say_stats(output, 0, 0);
  assert(strcmp(output, "Explore/Defense(5)") == 0);
}
'''


@pytest.mark.parametrize("variant", ["actual", "cached_reactor", "exclusive_triad", "cached_verbose_offense"])
def test_actual_formatter_fresh_read_boundaries(tmp_path, variant):
    compiler = shutil.which("clang++") or shutil.which("g++")
    assert compiler, "host C++ compiler required"
    cpp = tmp_path / "stats.cpp"
    binary = tmp_path / "stats"
    cpp.write_text(test_source(variant))
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                            str(cpp), "-o", str(binary)], capture_output=True, text=True)
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True)
    assert (result.returncode == 0) == (variant == "actual"), result.stdout + result.stderr
