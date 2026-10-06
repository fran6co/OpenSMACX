"""Bounded host controls compile the current complete enemy movement owner.

Only includes are removed for the typed host API doubles. The complete current
product TU supplies every recovered helper and the real enemy_move body. Two
additional bounded region adapters are extracted from that same current source;
no retained recovery packet provides an implementation. These controls do not
establish native ABI, byte equality, error-path outcomes, or full-game behavior.
"""
from hashlib import sha256
from pathlib import Path
import re
import shutil
import subprocess

from match_recovery.source import State, read_file

ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/enemy.cpp").is_file())
PRODUCT = ROOT / "src/enemy.cpp"
FIXTURE = Path(__file__).resolve().parent / "fixtures/enemy_move_full_body.cpp"
ORIGINAL_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def actual_region(source, begin, end):
    start_anchor = "\nlabel_" + begin + ":\n"
    end_anchor = "\nlabel_" + end + ":\n"
    assert source.count(start_anchor) == source.count(end_anchor) == 1
    start = source.index(start_anchor) + 1
    stop = source.index(end_anchor) + 1
    assert start < stop
    return source[start:stop]


def actual_unit():
    source = PRODUCT.read_text()
    source = re.sub(r"(?m)^[ \t]*#include[^\n]*(?:\n|$)", "", source)
    assert "#include" not in source
    fixture = FIXTURE.read_text()
    replacements = {
        "/* ACTUAL_ENEMY_SOURCE */": source,
        "/* ACTUAL_LATE_SCANS */": actual_region(source, "56ed87", "570002"),
        "/* ACTUAL_FINAL_UPSTREAM */": actual_region(source, "56e507", "56ed87"),
    }
    for marker, implementation in replacements.items():
        assert fixture.count(marker) == 1
        fixture = fixture.replace(marker, implementation)
    return fixture


def test_complete_enemy_identity_and_path_dependent_frame():
    source = PRODUCT.read_text()
    owner, = [row for row in read_file(PRODUCT) if row.address == 0x56B5B0]
    assert owner.name == "?enemy_move@@YAHH@Z"
    assert owner.image_spans == ((0x56B5B0, 0x579208),)
    assert owner.kind == "game"
    assert owner.state is State.IMPLEMENTED and not owner.extract_error
    signature = "int __cdecl enemy_move(int incoming_vehicle_id) {"
    assert source.count(signature) == 1
    body = source.split(signature, 1)[1]
    assert body.count("EnemyMoveFullFrame f;") == 1
    assert "EnemyMoveFullFrame f =" not in body and "memset" not in body
    assert "BODY GOES HERE" not in source and "UNRESOLVED" not in source
    assert "__asm" not in source and "PENDING_BODY" not in source
    assert "base_near(" not in source
    original = ROOT / ".opensmacx/game/terranx_original.exe"
    if original.is_file():
        assert sha256(original.read_bytes()).hexdigest() == ORIGINAL_SHA256


def test_complete_enemy_direct_native_join_closure():
    source = PRODUCT.read_text()
    definitions = re.findall(r"(?m)^\s*(label_[0-9a-f]+):", source)
    targets = set(re.findall(r"goto\s+(label_[0-9a-f]+)", source))
    assert len(definitions) == len(set(definitions)), "duplicate live native label"
    assert targets <= set(definitions), "unclosed native continuation"
    fixture = FIXTURE.read_text()
    assert "build/recovery" not in fixture and '#include "' not in fixture


def test_coast_or_border_declaration_matches_existing_signed_owner():
    header = (ROOT / "src/map.h").read_text()
    implementation = (ROOT / "src/map.cpp").read_text()
    declaration = re.search(r"BOOL\s+__cdecl\s+coast_or_border\(([^)]*)\)\s*;", header)
    definition = re.search(r"BOOL\s+__cdecl\s+coast_or_border\(([^)]*)\)\s*\{", implementation)
    assert declaration is not None and definition is not None
    arguments = [part.strip() for part in declaration[1].split(",")]
    assert len(arguments) == 5 and all(part.startswith("int ") for part in arguments)
    assert arguments == [part.strip() for part in definition[1].split(",")]
    owner, = [row for row in read_file(ROOT / "src/map.cpp") if row.address == 0x56B480]
    assert owner.name == "?coast_or_border@@YAHHHHHH@Z"


def test_actual_complete_enemy_body_gcc_and_clang(tmp_path):
    compilers = [path for name in ("g++", "clang++") if (path := shutil.which(name))]
    assert compilers, "a host C++ compiler is required"
    cpp = tmp_path / "actual_enemy.cpp"
    cpp.write_text(actual_unit())
    for compiler_index, compiler in enumerate(compilers):
        for optimization in ("-O0", "-O2"):
            executable = tmp_path / ("enemy_" + str(compiler_index) + optimization)
            # Suppress only extraction/helper warnings and the native unchecked
            # route-array access. Do not guard or normalize the product's bugs.
            subprocess.run([
                compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
                "-Wno-unused-function", "-Wno-unused-label", "-Wno-array-bounds",
                optimization, str(cpp), "-o", str(executable),
            ], check=True, capture_output=True, text=True, timeout=60)
            subprocess.run([str(executable)], check=True, capture_output=True,
                           text=True, timeout=30)
