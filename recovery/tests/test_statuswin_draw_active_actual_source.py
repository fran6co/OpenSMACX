from __future__ import annotations

from pathlib import Path
import re
import shutil
import subprocess
import tempfile

import pytest


ROOT = next(
    parent for parent in Path(__file__).resolve().parents
    if (parent / "src" / "statuswin.cpp").is_file()
)
SOURCE = ROOT / "src" / "statuswin.cpp"
SIGNATURE = "void StatusWin::draw_active(int vehicle_id, int base_id, int add_spot)"


def extract_function(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    state = "code"
    index = opening
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and nxt == "/":
                state = "line"
                index += 2
                continue
            if char == "/" and nxt == "*":
                state = "block"
                index += 2
                continue
            if char in ('"', "'"):
                state = "string" if char == '"' else "character"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[start : index + 1]
        elif state == "line":
            if char == "\n":
                state = "code"
        elif state == "block":
            if char == "*" and nxt == "/":
                state = "code"
                index += 2
                continue
        else:
            if char == "\\":
                index += 2
                continue
            if (state == "string" and char == '"') or (
                state == "character" and char == "'"
            ):
                state = "code"
        index += 1
    raise AssertionError(f"unterminated function: {signature}")


def contract(body: str) -> tuple[str, str, str]:
    def predicate(pattern: str) -> str:
        match = re.search(pattern, body)
        assert match, f"missing native lower-only predicate: {pattern}"
        return " ".join(match.group(1).split())

    vehicle = predicate(
        r"if\s*\((base_id\s*<\s*0\s*&&\s*vehicle_id\s*>=\s*0)\)\s*\{"
    )
    home = predicate(r"if\s*\((vehicle\.home_base_id\s*>=\s*0)\)\s*\{")
    base = predicate(r"if\s*\((base_id\s*>=\s*0)\)\s*\{")

    summary_start = body.find('strcat(StringTemp, "(");')
    assert summary_start >= 0
    stats_call = body.find("say_stats_3(StringTemp, vehicle.proto_id);", summary_start)
    assert stats_call >= 0
    defender = body.find("if (vehicle.state & VSTATE_DESIGNATE_DEFENDER)", stats_call)
    assert defender >= 0
    closing = body.find('strcat(StringTemp, ")");', defender)
    assert closing >= 0
    summary_wrap = body.find("line_y = screen->wrap_cent(StringTemp", closing)
    assert summary_wrap >= 0
    assert summary_start < stats_call < defender < closing < summary_wrap
    assert body.count("say_orders(StringTemp, vehicle_id);") == 1
    later_orders = body.find("say_orders(StringTemp, vehicle_id);")
    known_orders = body.find("if (known_orders)", summary_wrap)
    assert later_orders >= 0 and known_orders >= 0
    assert later_orders > known_orders
    return vehicle, home, base


def host_control(expressions: tuple[str, str, str]) -> str:
    vehicle, home, base = expressions
    return f'''#include <assert.h>
struct Vehicle {{ int home_base_id; Vehicle(int value) : home_base_id(value) {{}} }};
static bool vehicle_path(int base_id, int vehicle_id, int live_vehicle_count) {{
  (void)live_vehicle_count;
  return {vehicle};
}}
static bool home_path(Vehicle vehicle, int live_base_count) {{
  (void)live_base_count;
  return {home};
}}
static bool base_path(int base_id, int live_base_count) {{
  (void)live_base_count;
  return {base};
}}
int main() {{
  assert(vehicle_path(-1, 0, 1));
  assert(vehicle_path(-1, 99, 1));
  assert(!vehicle_path(0, 0, 1));
  assert(!vehicle_path(-1, -1, 1));
  assert(home_path(Vehicle(0), 1));
  assert(home_path(Vehicle(99), 1));
  assert(!home_path(Vehicle(-1), 1));
  assert(base_path(0, 1));
  assert(base_path(99, 1));
  assert(!base_path(-1, 1));
}}
'''


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_statuswin_draw_active_real_owner_predicates_and_summary(compiler_name: str) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    source = SOURCE.read_text(encoding="utf-8")
    body = extract_function(source, SIGNATURE)
    expressions = contract(body)
    with tempfile.TemporaryDirectory(prefix="statuswin-draw-active-") as directory:
        cpp = Path(directory) / "actual_predicates.cpp"
        exe = Path(directory) / "actual_predicates"
        cpp.write_text(host_control(expressions), encoding="utf-8")
        built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
            capture_output=True,
            text=True,
            timeout=30,
            check=False,
        )
        assert built.returncode == 0, built.stdout + built.stderr
        ran = subprocess.run([str(exe)], capture_output=True, text=True, timeout=30)
        assert ran.returncode == 0, ran.stdout + ran.stderr


def test_statuswin_draw_active_mutants_are_rejected() -> None:
    source = SOURCE.read_text(encoding="utf-8")
    body = extract_function(source, SIGNATURE)
    vehicle_bad = body.replace(
        "if (base_id < 0 && vehicle_id >= 0)",
        "if (base_id < 0 && vehicle_id >= 0 && vehicle_id < live_vehicle_count)",
        1,
    )
    home_bad = body.replace(
        "if (vehicle.home_base_id >= 0)",
        "if (vehicle.home_base_id >= 0 && vehicle.home_base_id < live_base_count)",
        1,
    )
    base_bad = body.replace(
        "if (base_id >= 0)", "if (base_id >= 0 && base_id < live_base_count)", 1
    )
    stats_bad = body.replace(
        "say_stats_3(StringTemp, vehicle.proto_id);",
        "say_orders(StringTemp, vehicle_id);",
        1,
    )
    for mutant in (vehicle_bad, home_bad, base_bad, stats_bad):
        with pytest.raises(AssertionError):
            contract(mutant)
