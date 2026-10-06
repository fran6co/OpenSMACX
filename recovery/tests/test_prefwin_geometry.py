"""Actual-source controls for compiler-owned PrefWin geometry initialization.

The test extracts only the natural scalar definitions from src/prefwin.cpp and
executes those exact expressions in a host harness. It does not import the
proposal emitter or assert a VC6-generated symbol spelling/native ABI.
"""
from __future__ import annotations

import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.source import read_text
from match_recovery.source.model import Mode
from match_recovery.object_source import parse_objects
from match_recovery.source_inventory import image_pin
from match_recovery.source_config import SourceConfiguration


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "CMakeLists.txt").is_file())
CONFIG_ROOT = next(parent for parent in Path(__file__).resolve().parents
                   if (parent / "recovery/project.json").is_file())
SOURCE_ROOT = Path(os.environ.get("OPENSMACX_SOURCE_ROOT", ROOT)).resolve()
GEOMETRY = SOURCE_ROOT / "src/prefwin_geometry.cpp"
PREFWIN = SOURCE_ROOT / "src/prefwin.cpp"
BUTTON_GROUP = SOURCE_ROOT / "src/buttongroup.cpp"
SOURCE_LIST = SOURCE_ROOT / "CMakeLists.txt"
ADDRESSES = [
    0x48D550, 0x48D560, 0x48D570, 0x48D590, 0x48D5B0, 0x48D5D0,
    0x48D5F0, 0x48D610, 0x48D630, 0x48D650, 0x48D670, 0x48D690,
    0x48D6B0, 0x48D6D0, 0x48D6F0, 0x48D710, 0x48D730, 0x48D750,
    0x48D760, 0x48D770, 0x48D790, 0x48D7B0, 0x48D7D0, 0x48D7F0,
    0x48D810, 0x48D820,
]
OUTPUTS = [
    "8577EC", "8577E8", "8578AC", "8578B0", "857888", "857884",
    "8578C0", "85789C", "857898", "8578A8", "8578A4", "8578C8",
    "8578C4", "8578A0", "85788C", "857890", "857894", "8577D8",
    "8577DC", "8577E4", "8577E0", "8578B4", "8578BC", "8578CC",
    "8578D0", "8578B8",
]
SPAN_LENGTHS = [
    0x0E, 0x0B, 0x17, 0x17, 0x14, 0x14, 0x17, 0x1B, 0x14, 0x1E,
    0x17, 0x14, 0x14, 0x17, 0x14, 0x14, 0x17, 0x0B, 0x10, 0x17,
    0x1B, 0x14, 0x14, 0x1B, 0x0E, 0x1E,
]
LEVER_COUNTS = [0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1,
                1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0]
SEMANTIC_ADDRESSES = {0x48D610, 0x48D7F0}
NO_CLAIM_ADDRESS = 0x48D810
EXPECTED_BY_OUTPUT = {
    "8577D8": 256, "8577DC": 287, "8577E0": 92, "8577E4": 457,
    "8577E8": 4, "8577EC": 27, "857884": 7, "857888": 30,
    "85788C": 33, "857890": 10, "857894": 214, "857898": 7,
    "85789C": 253, "8578A0": 457, "8578A4": 376, "8578A8": 463,
    "8578AC": 692, "8578B0": 382, "8578B4": 259, "8578B8": 328,
    "8578BC": 290, "8578C0": 376, "8578C4": 10, "8578C8": 256,
    "8578CC": 382, "8578D0": 288,
}


def declarations(source: str) -> list[str]:
    found = []
    for line in source.splitlines():
        if line.startswith("int32_t PrefWinGeometryValue_") and " = " in line:
            found.append(line.strip())
    return found


def assert_source_family(source: str) -> None:
    rows = [row for row in read_text(source, Path("src/prefwin_geometry.cpp"))
            if row.address in ADDRESSES]
    assert [row.address for row in rows] == ADDRESSES
    assert all(row.mode is Mode.FILE for row in rows)
    for row, length, lever_count in zip(rows, SPAN_LENGTHS, LEVER_COUNTS):
        assert row.image_spans == ((row.address, row.address + length),)
        assert row.semantic is (row.address in SEMANTIC_ADDRESSES)
        assert row.byte_exact is (row.address not in SEMANTIC_ADDRESSES | {NO_CLAIM_ADDRESS})
        assert len(row.levers) == lever_count
    assert len(next(row for row in rows if row.address == NO_CLAIM_ADDRESS).ruled_out) == 2
    decls = declarations(source)
    assert len(decls) == 26
    actual_outputs = [line.split(" = ", 1)[0].removeprefix("int32_t PrefWinGeometryValue_")
                      for line in decls]
    assert actual_outputs == OUTPUTS

    config = SourceConfiguration.read(CONFIG_ROOT)
    objects = parse_objects(source, "src/prefwin_geometry.cpp", image_pin(CONFIG_ROOT, config),
                            address_space=config.address_space)
    object_symbols = {row.symbol for row in objects
                      if row.symbol.startswith("?PrefWinGeometryValue_")}
    assert object_symbols == {f"?PrefWinGeometryValue_{name}@@3HA" for name in OUTPUTS}


def inputs() -> str:
    return """#include <cstdint>
int32_t PrefWinGeometryConstant_686B6C = 4;
int32_t PrefWinGeometryConstant_686B70 = 4;
int32_t PrefWinGeometryConstant_686B74 = 3;
int32_t PrefWinGeometryConstant_686B78 = 3;
int32_t PrefWinDisplayWidth = 700;
int32_t PrefWinDisplayHeight = 390;
int32_t PrefWinButtonWidth = 220;
int32_t PrefWinToggleWidth = 120;
"""


def main_source(outputs: list[str]) -> str:
    externs = "\n".join(f"extern int32_t PrefWinGeometryValue_{name};" for name in outputs)
    checks = "\n".join(
        f"  assert(PrefWinGeometryValue_{name} == {EXPECTED_BY_OUTPUT[name]});"
        for name in outputs
    )
    return f"""#include <cassert>
#include <cstdint>
{externs}
int main() {{
{checks}
}}
"""


def execute(tmp_path: Path, init_decls: list[str]) -> subprocess.CompletedProcess:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    init_path = tmp_path / "prefwin_natural_init.cpp"
    input_path = tmp_path / "geometry_inputs.cpp"
    main_path = tmp_path / "check_outputs.cpp"
    binary = tmp_path / "prefwin-natural-init"
    init_path.write_text("#include <cstdint>\n" + inputs_declared_externally() +
                         "\n".join(init_decls) + "\n")
    input_path.write_text(inputs())
    main_path.write_text(main_source(OUTPUTS))
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(init_path), str(input_path), str(main_path), "-o", str(binary)],
                   check=True, capture_output=True, text=True)
    return subprocess.run([str(binary)], capture_output=True, text=True)


def inputs_declared_externally() -> str:
    names = (
        "PrefWinGeometryConstant_686B6C", "PrefWinGeometryConstant_686B70",
        "PrefWinGeometryConstant_686B74", "PrefWinGeometryConstant_686B78",
        "PrefWinDisplayWidth", "PrefWinDisplayHeight", "PrefWinButtonWidth",
        "PrefWinToggleWidth",
    )
    return "".join(f"extern int32_t {name};\n" for name in names)


def test_installed_natural_initializer_source_is_ordered_and_maps_all_originals():
    source = GEOMETRY.read_text()
    assert "??__EPrefWinGeometryValue_" not in source
    assert "hypothesized VC6 generated symbol" not in source
    assert_source_family(source)
    prefwin = PREFWIN.read_text()
    assert "ButtonGroup PrefWinButtonGroup;" in prefwin
    assert "PrefWinGeometryValue_8577EC =" not in prefwin
    assert "g_PREFWIN_BUTTONGROUP" not in BUTTON_GROUP.read_text()
    source_list = SOURCE_LIST.read_text()
    order = [source_list.index(f"    src/{unit}.cpp\n") for unit in
             ("infowin", "prefwin", "prefwin_geometry", "quaylewin", "reportif", "console")]
    assert order == sorted(order)


def test_real_window_and_timer_owners_retain_original_identity_and_order():
    console = (SOURCE_ROOT / "src/console.cpp").read_text()
    info = (SOURCE_ROOT / "src/infowin.cpp").read_text()
    pref = (SOURCE_ROOT / "src/prefwin.cpp").read_text()
    time_source = (SOURCE_ROOT / "src/time.cpp").read_text()
    config = SourceConfiguration.read(CONFIG_ROOT)
    pin = image_pin(CONFIG_ROOT, config)

    assert console.count("Console ConsoleGlobal;") == 1
    assert console.count("Time ConsoleTimer;") == 1
    assert "Time g_CONSOLE_TIMER;" not in console
    timer_start = console.index("Time ConsoleTimer;")
    global_start = console.index("Console ConsoleGlobal;")
    grouped_start = console.index("Time BlinkTimer,")
    assert global_start < timer_start < grouped_start
    grouped = re.search(r"Time\s+BlinkTimer,(.*?)TurnTimer\s*;", console, re.DOTALL)
    assert grouped
    declarators = re.sub(r"//[^\n]*", "", grouped.group(0))
    assert re.findall(r"\b(BlinkTimer|Blink2Timer|LineTimer|GoTimer|TurnTimer)\b", declarators) == [
        "BlinkTimer", "Blink2Timer", "LineTimer", "GoTimer", "TurnTimer"]
    assert "?ConsoleInfoWin@@3VInfoWin@@A" in info and "InfoWin ConsoleInfoWin;" in info
    assert "?ConsoleInfoWin@@3VInfoWin@@A" not in console
    assert "?ConsolePrefWin@@3VPrefWin@@A" in pref and "PrefWin ConsolePrefWin;" in pref
    assert "?ConsolePrefWin@@3VPrefWin@@A" not in console

    expected_units = {
        "?ConsoleInfoWin@@3VInfoWin@@A": "src/infowin.cpp",
        "?ConsolePrefWin@@3VPrefWin@@A": "src/prefwin.cpp",
        "?ConsoleTimer@@3VTime@@A": "src/console.cpp",
        "?LineTimer@@3VTime@@A": "src/console.cpp",
        "?BlinkTimer@@3VTime@@A": "src/console.cpp",
        "?Blink2Timer@@3VTime@@A": "src/console.cpp",
    }
    object_rows = []
    for relative in expected_units.values():
        source = (SOURCE_ROOT / relative).read_text()
        object_rows.extend(parse_objects(source, relative, pin, address_space=config.address_space))
    by_symbol = {row.symbol: row for row in object_rows}
    for symbol, unit in expected_units.items():
        assert by_symbol[symbol].unit == unit
    timer = by_symbol["?ConsoleTimer@@3VTime@@A"]
    assert timer.origin.address == 0x00939E88 and timer.span is None and timer.evidence is None
    assert {symbol: by_symbol[symbol].origin.address for symbol in expected_units
            if symbol != "?ConsoleTimer@@3VTime@@A"} == {
        "?ConsoleInfoWin@@3VInfoWin@@A": 0x007AD2A0,
        "?ConsolePrefWin@@3VPrefWin@@A": 0x008578D8,
        "?LineTimer@@3VTime@@A": 0x00915658,
        "?BlinkTimer@@3VTime@@A": 0x00915688,
        "?Blink2Timer@@3VTime@@A": 0x00939EB0,
    }
    assert "Time ConsoleTimer;" not in time_source
    assert "Time BlinkTimer," not in time_source
    for address, name, end, symbol in (
        (0x0050E9B0, "sub_50e9b0", 0x0050E9EE, "_$E21"),
        (0x0050E9F0, "sub_50e9f0", 0x0050EA22, "_$E19"),
    ):
        rows = [row for row in read_text(console, Path("src/console.cpp")) if row.address == address]
        assert len(rows) == 1 and rows[0].name == name and rows[0].symbol == symbol
        assert rows[0].image_spans == ((address, end),) and rows[0].byte_exact
        assert not [row for row in read_text(time_source, Path("src/time.cpp")) if row.address == address]


def test_console_timer_and_grouped_time_declarations_keep_native_source_order(tmp_path):
    console = (SOURCE_ROOT / "src/console.cpp").read_text()
    source_decl = re.search(r"Time ConsoleTimer;.*?Time BlinkTimer,.*?TurnTimer\s*;",
                            console, re.DOTALL)
    assert source_decl
    declarations = re.sub(r"//[^\n]*", "", source_decl.group(0))
    declarations = re.sub(r"\bTime\b", "Probe", declarations)
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    source = f"""#include <cstddef>
struct Probe;
static Probe* built[8]; static int built_n;
static Probe* destroyed[8]; static int destroyed_n;
struct Probe {{ Probe() {{ built[built_n++] = this; }} ~Probe() {{ destroyed[destroyed_n++] = this; }} }};
int main() {{
  Probe* expected[6];
  {{ {declarations}
    expected[0] = &ConsoleTimer; expected[1] = &BlinkTimer;
    expected[2] = &Blink2Timer; expected[3] = &LineTimer;
    expected[4] = &GoTimer; expected[5] = &TurnTimer;
  }}
  if (built_n != 6 || destroyed_n != 6) return 1;
  for (int i = 0; i < 6; ++i) if (built[i] != expected[i]) return 2;
  for (int i = 0; i < 6; ++i) if (destroyed[i] != expected[5 - i]) return 3;
  return 0;
}}
"""
    cpp = tmp_path / "console-timer-order.cpp"
    binary = tmp_path / "console-timer-order"
    cpp.write_text(source)
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(binary)], check=True, capture_output=True, text=True)
    assert subprocess.run([str(binary)], check=False).returncode == 0

    reordered = declarations.replace("Probe ConsoleTimer;", "", 1) + " Probe ConsoleTimer;"
    mutant_cpp = tmp_path / "console-timer-order-mutant.cpp"
    mutant_binary = tmp_path / "console-timer-order-mutant"
    mutant_cpp.write_text(source.replace(declarations, reordered, 1))
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    str(mutant_cpp), "-o", str(mutant_binary)],
                   check=True, capture_output=True, text=True)
    assert subprocess.run([str(mutant_binary)], check=False).returncode != 0


def test_actual_natural_initializers_produce_original_geometry_values(tmp_path):
    assert execute(tmp_path, declarations(GEOMETRY.read_text())).returncode == 0


def test_omission_order_and_arithmetic_mutants_are_detected(tmp_path):
    source = GEOMETRY.read_text()
    decls = declarations(source)
    assert len(decls) == 26

    omitted = decls[0]
    with pytest.raises(AssertionError):
        assert_source_family(source.replace(omitted + "\n", "", 1))

    # Executable controls prove the expressions are live startup stores rather
    # than dead declarations or helper-only values.
    mutated = list(decls)
    left, right = mutated[0], mutated[1]
    mutated[0], mutated[1] = right, left
    with pytest.raises(AssertionError):
        assert_source_family(source.replace(left, "__FIRST__", 1)
                             .replace(right, left, 1).replace("__FIRST__", right, 1))

    arithmetic = list(decls)
    needle = " + 23u"
    index = next(i for i, line in enumerate(arithmetic) if needle in line)
    arithmetic[index] = arithmetic[index].replace(needle, " + 24u", 1)
    result = execute(tmp_path, arithmetic)
    assert result.returncode != 0
