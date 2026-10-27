"""Signed map scalar owner controls, anchored to pinned consumer evidence."""
from pathlib import Path
import re
import shutil
import subprocess
import pytest


ROOT = next(p for p in Path(__file__).resolve().parents if (p / "src/map.cpp").is_file())


def test_map_longitude_is_signed_owner_with_original_binding():
    header = (ROOT / "src/map.h").read_text()
    owner = (ROOT / "src/map.cpp").read_text()
    assert re.search(r"^extern int MapLongitude;", header, re.M)
    assert re.search(r"^int MapLongitude\s*=\s*1;", owner, re.M)
    assert "RECOVERY_OBJECT: v4 0x0068FAF0 ?MapLongitude@@3IA target ?MapLongitude@@3HA" in owner
    assert "MapLongitude = MapLongitudeBounds / 2;" in owner


def test_signed_area_root_owner_reaches_signed_consumers_without_reinterpretation():
    header = (ROOT / "src/map.h").read_text()
    faction = (ROOT / "src/faction.cpp").read_text()
    technology = (ROOT / "src/technology.cpp").read_text()
    base = (ROOT / "src/base.cpp").read_text()
    assert re.search(r"\bint area_sq_root_;\s*// \+0x018", header)
    assert "discovery_rate * MapAreaSqRoot) / 56" in technology
    assert "weight * MapAreaSqRoot) / 56 / 2" in faction
    assert "separation * MapAreaSqRoot / 56" in faction
    assert " / MapAreaSqRoot;" in base
    assert "base_yield_signed_bits(MapAreaSqRoot)" not in base


def test_signed_wrapped_distance_consumers_do_not_unsigned_promote_stride():
    faction = (ROOT / "src/faction.cpp").read_text()
    game = (ROOT / "src/game.cpp").read_text()
    veh = (ROOT / "src/veh.cpp").read_text()
    assert "MapLongitude < static_cast<uint32_t>(dx)" not in veh
    assert "dx > MapLongitude" in game
    assert "MapLongitude < static_cast<uint32_t>(x_distance)" not in veh
    # Packet framing is explicitly unsigned and remains so independently of
    # the signed scalar owner.
    netdaemon = (ROOT / "src/netdaemon.cpp").read_text()
    assert "* MapLongitude * 44u" in netdaemon


@pytest.mark.parametrize(("damage", "expected"), [(False, 0), (True, 1)])
def test_actual_owner_and_consumer_keep_signed_comparison(tmp_path, damage, expected):
    compiler = shutil.which("c++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    owner = (ROOT / "src/map.cpp").read_text()
    declaration = re.search(r"^int MapLongitude = 1;", owner, re.M).group()
    veh = (ROOT / "src/veh.cpp").read_text()
    start = veh.index("void __cdecl action_destroy(")
    body_start = veh.index("{", start)
    depth = 0
    for index in range(body_start, len(veh)):
        if veh[index] == "{":
            depth += 1
        elif veh[index] == "}":
            depth -= 1
            if depth == 0:
                action_destroy = veh[start:index + 1]
                break
    else:
        raise AssertionError("unterminated action_destroy")
    comparison = re.search(r"MapLongitude\s*<\s*dx", action_destroy).group()
    declaration = declaration.replace("= 1", "= -1")
    if damage:
        declaration = declaration.replace("int MapLongitude", "uint32_t MapLongitude")
    source = tmp_path / "longitude.cpp"
    source.write_text(
        "#include <stdint.h>\n"
        + declaration + "\n"
        + "int main() { const int dx = 1; return "
        + comparison + " ? 0 : 1; }\n"
    )
    binary = tmp_path / "longitude"
    flags = [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror"]
    if damage:
        flags.append("-Wno-sign-compare")
    built = subprocess.run(flags + [
                            str(source), "-o", str(binary)], capture_output=True, timeout=60)
    assert built.returncode == 0, built.stderr.decode(errors="replace")
    assert subprocess.run([str(binary)], timeout=15).returncode == expected


def _extract_x_dist(header):
    signature = "MEASURED inline int __cdecl x_dist("
    start = header.index(signature)
    body_start = header.index("{", start)
    depth = 0
    for index in range(body_start, len(header)):
        if header[index] == "{":
            depth += 1
        elif header[index] == "}":
            depth -= 1
            if depth == 0:
                return header[start:index + 1]
    raise AssertionError("unterminated x_dist definition")


def test_actual_x_dist_ring_flat_flags_and_original_signed_wrap(tmp_path):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    helper = _extract_x_dist((ROOT / "src/map.h").read_text())
    source = tmp_path / "x_dist.cpp"
    source.write_text(
        "#include <cstdlib>\n"
        "#include <cstdio>\n"
        "#define MEASURED\n"
        "#define __cdecl\n"
        "static int fixture_flat;\n"
        "static int fixture_longitude;\n"
        "static int fixture_bounds;\n"
        "#define MapIsFlat fixture_flat\n"
        "#define MapLongitude fixture_longitude\n"
        "#define MapLongitudeBounds fixture_bounds\n"
        + helper + "\n"
        "static void check(int a, int b, int flat, int longitude, int bounds, int expected) {\n"
        "  fixture_flat = flat; fixture_longitude = longitude; fixture_bounds = bounds;\n"
        "  const int actual = x_dist(a, b);\n"
        "  if (actual != expected) { std::fprintf(stderr, \"x_dist(%d,%d; flat=%d, half=%d, bounds=%d): %d != %d\\n\", a, b, flat, longitude, bounds, actual, expected); std::abort(); }\n"
        "}\n"
        "int main() {\n"
        "  // Hand-checked shortest distance on a 16-column wrapped ring.\n"
        "  check(0, 0, 0, 8, 16, 0); check(0, 3, 0, 8, 16, 3);\n"
        "  check(0, 7, 0, 8, 16, 7); check(0, 8, 0, 8, 16, 8);\n"
        "  check(0, 9, 0, 8, 16, 7); check(0, 12, 0, 8, 16, 4);\n"
        "  check(2, 14, 0, 8, 16, 4); check(15, 1, 0, 8, 16, 2);\n"
        "  // Only bit zero selects flat behavior; other bits do not.\n"
        "  check(0, 12, 1, 8, 16, 12); check(0, 12, 3, 8, 16, 12);\n"
        "  check(0, 12, 2, 8, 16, 4); check(0, 12, 0x100, 8, 16, 4);\n"
        "  // Preserve original off-range subtraction, including negative result.\n"
        "  check(0, 20, 0, 8, 16, -4);\n"
        "  // A negative signed half-width compares signed and triggers wrapping.\n"
        "  check(0, 6, 0, -8, 16, 10);\n"
        "  return 0;\n"
        "}\n"
    )
    binary = tmp_path / "x_dist"
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(binary)],
        capture_output=True,
        timeout=60,
    )
    assert built.returncode == 0, built.stderr.decode(errors="replace")
    ran = subprocess.run([str(binary)], capture_output=True, timeout=15)
    assert ran.returncode == 0, ran.stderr.decode(errors="replace")
