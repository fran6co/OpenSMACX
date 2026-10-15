"""Actual-source controls for recovered human-byte member accesses."""

from pathlib import Path
import shutil
import subprocess
import re

import pytest


ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / "src/base.cpp"
TECH = ROOT / "src/technology.cpp"
ACCESS = "GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_"


def _body(path, signature):
    source = path.read_text()
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated function: " + signature)


OWNERS = (
    (BASE, "void __cdecl base_doctors()", 2),
    (BASE, "void __cdecl bases_reset(", 2),
    (BASE, "void __cdecl base_change(", 1),
    (BASE, "void __cdecl base_reset(", 2),
    (BASE, "void __cdecl base_ecology()", 1),
    (BASE, "void __cdecl upgrade_any_prototypes(", 1),
    (BASE, "void __cdecl do_upgrade(", 1),
    (BASE, "int __cdecl upgrade_prototype(", 4),
    (TECH, "void __cdecl tech_achieved(", 5),
    (TECH, "int __cdecl tech_advance(", 2),
)


def _actual_accesses():
    accesses = []
    for path, signature, expected_count in OWNERS:
        body = _body(path, signature)
        count = body.count(ACCESS)
        assert count == expected_count, (signature, count, expected_count)
        assert "lock_masks_.enable_mask_" not in body, signature
        accesses.extend(body[index:index + len(ACCESS)]
                        for index in range(len(body))
                        if body.startswith(ACCESS, index))
    return accesses


def _mask_declarations():
    header = (ROOT / "src/game_data_state_window.h").read_text()
    match = re.search(
        r"struct GameDataLockMaskBytes\s*\{.*?\};\s*union GameDataLockMaskView\s*\{.*?\};",
        header, re.DOTALL,
    )
    assert match, "production lock-mask declarations missing"
    return match.group(0)


def _run(tmp_path, compiler_name, accesses, mutant):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    source = r"""
        #include <cassert>
        #include <cstdint>
        /* EXTRACTED_GAME_DATA_LOCK_MASK_TYPES */
        struct GameDataFields { GameDataLockMaskView lock_masks_; };
        struct GameDataStorage { GameDataFields fields_; };
        GameDataStorage GameDataStateStorage{};
    """
    assert source.count("/* EXTRACTED_GAME_DATA_LOCK_MASK_TYPES */") == 1
    source = source.replace("/* EXTRACTED_GAME_DATA_LOCK_MASK_TYPES */", _mask_declarations())
    for index, access in enumerate(accesses):
        expression = access.replace("bytes_.human_mask_", "enable_mask_") if mutant else access
        source += (
            "uint32_t access_%d() { return %s; }\n" % (index, expression)
        )
    source += "int main() {\n"
    source += "GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0xA5C00000u;\n"
    source += "if ("
    source += " || ".join("access_%d() != 0" % i for i in range(len(accesses)))
    source += ") return 1;\n"
    source += "GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0x5A1234B2u;\n"
    source += "if ("
    source += " || ".join("access_%d() != 0xB2u" % i for i in range(len(accesses)))
    source += ") return 2;\nreturn 0;\n}\n"
    cpp = tmp_path / (compiler_name.replace("+", "p") + ("_mutant" if mutant else "") + ".cpp")
    exe = cpp.with_suffix("")
    cpp.write_text(source)
    built = subprocess.run(
        (compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(exe)),
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stderr
    return subprocess.run((str(exe),), capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_all_ten_actual_owners_select_only_the_human_byte(tmp_path, compiler_name):
    accesses = _actual_accesses()
    result = _run(tmp_path, compiler_name, accesses, mutant=False)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_compiling_dword_member_mutant_fails_high_byte_controls(tmp_path, compiler_name):
    accesses = _actual_accesses()
    result = _run(tmp_path, compiler_name, accesses, mutant=True)
    assert result.returncode != 0, "dword-member mutant unexpectedly passed"
