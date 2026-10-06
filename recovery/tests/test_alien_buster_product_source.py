"""GCC/Clang controls over the actual integrated veh.cpp bodies.

The permanent test deliberately extracts definitions from src/veh.cpp after
the coordinator's native source writer has placed their ORIGINAL rows. The
older standalone candidates are not inputs to this test.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


HERE = Path(__file__).resolve().parent
ROOT = next(parent for parent in HERE.parents if (parent / "src/veh.cpp").exists())
VEH_CPP = ROOT / "src/veh.cpp"
PRODUCT_POLICY = ROOT / "src/enemy_pb_policy.h"
FIXTURES = HERE / "fixtures/alien_buster"
ALIEN_HARNESS = FIXTURES / "alien_host.cpp"
BUSTER_HARNESS = FIXTURES / "buster_host.cpp"


def extract_definition(source: str, signature: str) -> str:
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    quote = None
    escaped = False
    line_comment = block_comment = False
    index = brace
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if line_comment:
            if char == "\n":
                line_comment = False
            index += 1
            continue
        if block_comment:
            if char == "*" and nxt == "/":
                block_comment = False
                index += 2
                continue
            index += 1
            continue
        if quote:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == quote:
                quote = None
            index += 1
            continue
        if char == "/" and nxt == "/":
            line_comment = True
            index += 2
            continue
        if char == "/" and nxt == "*":
            block_comment = True
            index += 2
            continue
        if char in ('"', "'"):
            quote = char
            index += 1
            continue
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
        index += 1
    raise AssertionError(f"unterminated definition: {signature}")


def require_identity(source: str, address: str, signature: str) -> None:
    start = source.index(signature)
    preceding = source[max(0, start - 1800) : start]
    marker = re.compile(r"// ORIGINAL: 0x" + address + r"\b")
    assert marker.search(preceding), (
        f"{signature} has no nearby writer-owned ORIGINAL identity for 0x{address}"
    )


def product_bodies():
    source = VEH_CPP.read_text(encoding="utf-8")
    for address, signature in (
        ("00566810", "int __cdecl alien_move(int veh_id)"),
        ("0056A8A0", "int __cdecl enemy_planet_buster(int vehicle_id)"),
    ):
        require_identity(source, address, signature)

    alien_signatures = (
        "static int alien_move_wrap_x(int x)",
        "static bool alien_move_in_map(int x, int y)",
        "static void alien_move_native_no_base_coordinates(int &x, int &y)",
        "int __cdecl alien_move(int veh_id)",
    )
    alien = "\n\n".join(extract_definition(source, sig) for sig in alien_signatures)
    climax_query = extract_definition(source, "struct EnemyPlanetBusterClimaxQuery") + ";"
    buster = "\n\n".join(
        (
            climax_query,
            extract_definition(source, "int __cdecl enemy_planet_buster(int vehicle_id)"),
        )
    )
    return source, alien, buster


def assert_buster_body_order(body: str) -> None:
    radius_signature = "for (int offset = 0; offset < blast_radius; ++offset)"
    radius_loop = body.index(radius_signature)
    radius_complete = radius_loop + len(extract_definition(body, radius_signature))
    odp = body.index("enemy_pb_policy::discount_for_orbital_defense(")
    best_update = body.index("if (best_score < score)")
    assert radius_complete < odp < best_update, (
        "ODP must follow the complete blast-radius scan and precede target selection"
    )

    scratch = re.search(r"\bint\s+previous_launch_base\s*;", body)
    assert scratch, "retain native uninitialized local scratch spelling"
    own_scan = body.index("for (int friendly_base_id = 0;")
    assignment = body.index("previous_launch_base = launch_base_id;")
    winning_read = body.index("? launch_base_id : previous_launch_base")
    assert own_scan < assignment < winning_read
    assert "int previous_launch_base =" not in body


def buster_source_order_is_native(source: str) -> None:
    assert_buster_body_order(
        extract_definition(source, "int __cdecl enemy_planet_buster(int vehicle_id)")
    )


def compose_alien(harness: str, alien: str) -> str:
    anchor = '// PRODUCT_BODY: alien_move'
    assert harness.count(anchor) == 1
    harness = harness.replace(anchor, alien)
    fixture_gap = "int test_garrison_required,test_defensive_required;"
    assert harness.count(fixture_gap) == 1
    compatibility_globals = """
int test_add_goal_calls, test_add_goal_args[6];
int test_zoc_move_result, test_zoc_veh_result;
int test_zoc_sea_result, test_zoc_sea_calls, test_zoc_sea_args[3];
int test_base_at_result, test_base_at_args[2], test_base_at_calls;
"""
    return harness.replace(fixture_gap, fixture_gap + compatibility_globals)


def compose_buster(harness: str, buster: str) -> str:
    anchor = '// PRODUCT_BODY: enemy_planet_buster'
    assert harness.count(anchor) == 1
    # Avoid host execution of the native uninitialized foreign-target read.
    # These cases exercise reachable own-base state, empty/protected center
    # early exits, and the launch-base fallback only.
    safe_main = """int main() {
    test_last_reachable_launch_base_and_callback_mutation();
    test_no_winning_target_falls_back_to_launch_slot_zero();
    test_protected_city_exits_radius_scan();
    test_empty_center_exits_radius_scan();
    return 0;
}
"""
    before_main = harness[: harness.index("int main() {")]
    return before_main.replace(anchor, '#include "enemy_pb_policy.h"\n' + buster) + safe_main


def compile_run(source: str, compiler: str, include_dirs, tmp_path: Path, label: str,
                expect_success: bool) -> None:
    source_path = tmp_path / f"{label}.cpp"
    binary_path = tmp_path / label
    source_path.write_text(source, encoding="utf-8")
    command = [compiler, "-std=c++98", "-Wall", "-Wextra"]
    command.extend(f"-I{directory}" for directory in include_dirs)
    command.extend([str(source_path), "-o", str(binary_path)])
    built = subprocess.run(command, text=True, capture_output=True)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary_path)], text=True, capture_output=True)
    if expect_success:
        assert ran.returncode == 0, ran.stdout + ran.stderr
    else:
        assert ran.returncode != 0, f"damage control was not detected: {label}"


def test_actual_product_source_with_gcc_and_clang(tmp_path):
    source, alien, buster = product_bodies()
    buster_source_order_is_native(source)
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers, "at least GCC or Clang is required for host controls"

    alien_harness = compose_alien(ALIEN_HARNESS.read_text(encoding="utf-8"), alien)
    buster_harness = compose_buster(BUSTER_HARNESS.read_text(encoding="utf-8"), buster)
    include_dirs = (
        FIXTURES,
    )
    for index, compiler in enumerate(compilers):
        compiler_dir = tmp_path / f"compiler_{index}"
        compiler_dir.mkdir()
        # Stage the actual product policy helper beside the generated harness;
        # avoid adding src/ as an include directory because its strings.h shadows
        # the C library <string.h> under GCC/Clang.
        (compiler_dir / "enemy_pb_policy.h").write_text(
            PRODUCT_POLICY.read_text(encoding="utf-8"), encoding="utf-8"
        )
        # Stage the current host storage adapters too, so the compiled
        # fixture uses the same source snapshot as the extracted product body.
        for header in ("alien_host_fixture.h", "enemy_pb_host_fixture.h"):
            (compiler_dir / header).write_text(
                (FIXTURES / header).read_text(encoding="utf-8"), encoding="utf-8"
            )
        include_dirs = (compiler_dir,)
        compile_run(alien_harness, compiler, include_dirs, compiler_dir,
                    "alien_product", True)
        compile_run(buster_harness, compiler, include_dirs, compiler_dir,
                    "buster_product", True)

        # Native no-base coordinates are nonzero words in the live
        # PlayersData[7].sites[14].base_id storage; zeroing or selecting another
        # slot must break the observable movement fixture.
        zero_coords = alien.replace("x = x_word;\n    y = y_word;",
                                    "x = 0;\n    y = 0;")
        wrong_slot = alien.replace("PlayersData[7].sites[14].base_id",
                                   "PlayersData[0].sites[14].base_id")
        compile_run(compose_alien(ALIEN_HARNESS.read_text(encoding="utf-8"), zero_coords),
                    compiler, include_dirs, compiler_dir, "alien_zero_coords", False)
        compile_run(compose_alien(ALIEN_HARNESS.read_text(encoding="utf-8"), wrong_slot),
                    compiler, include_dirs, compiler_dir, "alien_wrong_slot", False)

        # Two compiled damage controls protect the native center/protected-city
        # `break` behavior. They are not tests of uninitialized scratch.
        no_center_break = buster.replace(
            "if (offset == 0) {\n                        score = -9999;\n                        break;\n                    }",
            "if (offset == 0) {\n                        score = -9999;\n                    }",
        )
        no_protected_break = buster.replace(
            "score = -9999;\n                        break;\n                    }\n\n                    int owned_projects",
            "score = -9999;\n                    }\n\n                    int owned_projects",
        )
        assert no_center_break != buster
        assert no_protected_break != buster
        compile_run(compose_buster(BUSTER_HARNESS.read_text(encoding="utf-8"),
                                   no_center_break),
                    compiler, include_dirs, compiler_dir, "buster_no_center_break", False)
        compile_run(compose_buster(BUSTER_HARNESS.read_text(encoding="utf-8"),
                                   no_protected_break),
                    compiler, include_dirs, compiler_dir,
                    "buster_no_protected_break", False)


def test_odp_order_and_uninitialized_scratch_are_source_only_controls():
    source = VEH_CPP.read_text(encoding="utf-8")
    buster_source_order_is_native(source)
    body = extract_definition(source, "int __cdecl enemy_planet_buster(int vehicle_id)")
    block_start = body.index("            const int defense_available =")
    block_end = body.index(");", body.index("enemy_pb_policy::discount_for_orbital_defense(", block_start)) + 2
    odp_block = body[block_start:block_end]
    mutated = body[:block_start] + body[block_end:]
    radius = "for (int offset = 0; offset < blast_radius; ++offset)"
    insertion = mutated.index(radius)
    mutated = mutated[:insertion] + odp_block + "\n\n" + mutated[insertion:]
    with pytest.raises(AssertionError, match="ODP must follow"):
        assert_buster_body_order(mutated)
    # Host runs never claim a result for the native first foreign winning read
    # of uninitialized scratch; retain only the source-level native fact here.
    assert "int previous_launch_base;" in body
    assert "best_launch_base = candidate_faction == faction_id\n                    ? launch_base_id : previous_launch_base;" in body
