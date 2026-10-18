"""Precision-boundary controls for the recovered free float-array operations.

Host execution checks the finite reciprocal-rounding witness against an
explicit float32 oracle; it is not x87 or original-executable proof. Managed
VC6 controls are coordinator-only and inspect fresh COFF plus native output.
"""

from __future__ import annotations

import os
from pathlib import Path
import shutil
import subprocess
import sys

import pytest

from test_game_logic_seed_batch_behavior import _definition


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
NORMALIZER = "void __cdecl vox_normalise_vector(const float *source, float *out)"
LENGTH = "float __cdecl vec3_length(const float *vector)"
VECTOR_NORMALIZE = "Vector Vector::normalize()"
LENGTH_INTRINSIC_SCOPE = (
    "#if defined(_MSC_VER) && _MSC_VER <= 1200\n"
    "// The pinned free operation uses inline fsqrt; Vector::normalize keeps the\n"
    "// translation unit's CRT sqrt policy before and after this one definition.\n"
    "#pragma intrinsic(sqrt)\n"
    "#endif\n"
)
LENGTH_FUNCTION_RESTORE = (
    "#if defined(_MSC_VER) && _MSC_VER <= 1200\n"
    "// Restore the prior TU policy immediately for following Vector methods.\n"
    "#pragma function(sqrt)\n"
    "#endif\n"
)


def actual_parts() -> dict[str, str]:
    caviar = (ROOT / "src/caviar.cpp").read_text()
    vector = (ROOT / "src/vector.cpp").read_text()
    normalizer = _definition(caviar, NORMALIZER)
    length = _definition(vector, LENGTH)
    normalize = _definition(vector, VECTOR_NORMALIZE)
    intrinsic_at = vector.index(LENGTH_INTRINSIC_SCOPE)
    restore_at = vector.index(LENGTH_FUNCTION_RESTORE)
    assert intrinsic_at + len(LENGTH_INTRINSIC_SCOPE) == vector.index(LENGTH)
    length_at = vector.index(LENGTH)
    assert vector[length_at:length_at + len(length)].endswith("}")
    assert vector[length_at + len(length):].startswith("\n" + LENGTH_FUNCTION_RESTORE)
    assert restore_at > length_at + len(length)
    assert "volatile float scale = 1.0f / length;" in normalizer
    assert normalizer.count("volatile float scale") == 1
    assert "double length = sqrt(static_cast<double>(y) * y" in normalizer
    assert "static_cast<double>(z) * z" in normalizer
    assert "static_cast<double>(x) * x" in normalizer
    assert normalizer.count("out[0] = source[0] * scale;") == 1
    assert normalizer.count("out[1] = source[1] * scale;") == 1
    assert normalizer.count("out[2] = source[2] * scale;") == 1
    return {"normalizer": normalizer, "length": length,
            "normalize": normalize}


def host_source(parts: dict[str, str]) -> str:
    return (r"""
#include <cmath>
#include <cstdlib>
#include <cstring>
#define __cdecl
NORMALIZER
float original_reciprocal_oracle(const float *source) {
    const float x = source[0], y = source[1], z = source[2];
    const double length = std::sqrt(static_cast<double>(y) * y
                                  + static_cast<double>(z) * z
                                  + static_cast<double>(x) * x);
    volatile float scale = 1.0f / length;
    return source[0] * scale;
}
float unrounded_reciprocal_mutant(const float *source) {
    const float x = source[0], y = source[1], z = source[2];
    const double length = std::sqrt(static_cast<double>(y) * y
                                  + static_cast<double>(z) * z
                                  + static_cast<double>(x) * x);
    const double scale = 1.0 / length;
    return static_cast<float>(source[0] * scale);
}
static unsigned bits(float value) {
    unsigned result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}
int main() {
    const float input[3] = {1657220.875f,
                            0.3149696290493011474609375f,
                            351.1258544921875f};
    float output[3] = {0.0f, 0.0f, 0.0f};
    vox_normalise_vector(input, output);
    if (bits(original_reciprocal_oracle(input)) != 0x3f7fffffu) return 1;
    if (bits(unrounded_reciprocal_mutant(input)) != 0x3f800000u) return 2;
    if (bits(output[0]) != bits(original_reciprocal_oracle(input))) return 3;
    float overlap[4] = {1.0f, 0.0f, 0.0f, 0.0f};
    vox_normalise_vector(overlap, overlap + 1);
    if (overlap[1] != 1.0f || overlap[2] != 1.0f || overlap[3] != 1.0f)
        return 4;
    return 0;
}
""".replace("NORMALIZER", parts["normalizer"]))


def test_actual_source_has_the_explicit_float32_reciprocal_boundary(tmp_path):
    parts = actual_parts()
    compiler = shutil.which(os.environ.get("OSMX_HOST_CXX", "g++"))
    if not compiler:
        pytest.skip("host g++ required")
    source = tmp_path / "vector_array_precision.cpp"
    executable = tmp_path / "vector_array_precision"
    source.write_text(host_source(parts))
    compiled = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-ffp-contract=off", str(source),
         "-o", str(executable)], capture_output=True, text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True,
                            timeout=10)
    assert result.returncode == 0, result.stdout + result.stderr


def native_source(parts: dict[str, str], *, unrounded: bool = False,
                  length_runtime_sqrt: bool = False,
                  neighbor_intrinsic: bool = False) -> str:
    normalizer = parts["normalizer"]
    if unrounded:
        old = "volatile float scale = 1.0f / length;"
        assert normalizer.count(old) == 1
        normalizer = normalizer.replace(old, "double scale = 1.0f / length;", 1)
    intrinsic_scope = LENGTH_INTRINSIC_SCOPE
    if length_runtime_sqrt:
        intrinsic_scope = intrinsic_scope.replace(
            "#pragma intrinsic(sqrt)", "#pragma function(sqrt)", 1)
    restore = LENGTH_FUNCTION_RESTORE
    if neighbor_intrinsic:
        restore = restore.replace("#pragma function(sqrt)",
                                  "#pragma intrinsic(sqrt)", 1)
    return (r"""
#include <cmath>
#include <float.h>
#include <cstring>
class Vector { public: float x_, y_, z_; Vector normalize(); };
#pragma intrinsic(sqrt)
NORMALIZER
#pragma function(sqrt)
INTRINSIC_SCOPE
LENGTH
RESTORE
VECTOR_NORMALIZE
static unsigned bits(float value) {
    unsigned result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}
int main() {
    _controlfp(_PC_64, _MCW_PC);
    if ((_controlfp(0, 0) & _MCW_PC) != _PC_64) return 10;
    const float input[3] = {1657220.875f,
                            0.3149696290493011474609375f,
                            351.1258544921875f};
    float output[3] = {0.0f, 0.0f, 0.0f};
    vox_normalise_vector(input, output);
    if (bits(output[0]) != 0x3f7fffffu) return 11;
    float overlap[4] = {1.0f, 0.0f, 0.0f, 0.0f};
    vox_normalise_vector(overlap, overlap + 1);
    if (overlap[1] != 1.0f || overlap[2] != 1.0f || overlap[3] != 1.0f)
        return 14;
    const float length_input[3] = {3.0f, 4.0f, 12.0f};
    if (vec3_length(length_input) != 13.0f) return 12;
    Vector value;
    value.x_ = 3.0f; value.y_ = 4.0f; value.z_ = 0.0f;
    Vector normalized = value.normalize();
    if (normalized.x_ < 0.599999f || normalized.x_ > 0.600001f ||
        normalized.y_ < 0.799999f || normalized.y_ > 0.800001f ||
        normalized.z_ != 0.0f) return 13;
    return 0;
}
""".replace("NORMALIZER", normalizer)
        .replace("INTRINSIC_SCOPE", intrinsic_scope)
        .replace("LENGTH", parts["length"])
        .replace("RESTORE", restore)
        .replace("VECTOR_NORMALIZE", parts["normalize"]))


def assert_native_arithmetic_paths(obj, vector_obj=None) -> None:
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32

    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    if vector_obj is None:
        vector_obj = obj

    normalizer = obj.function("?vox_normalise_vector@@YAXPBMPAM@Z")
    code, _, _ = obj.subject(normalizer)
    instructions = tuple(decoder.disasm(code, 0))
    divisions = [index for index, item in enumerate(instructions)
                 if item.mnemonic.startswith("fdiv")]
    assert divisions, "normalizer must retain its x87 reciprocal"
    division = divisions[-1]
    roots = [index for index, item in enumerate(instructions)
             if item.mnemonic == "fsqrt"]
    assert roots and roots[-1] < division, "normalizer must divide by inline fsqrt"
    assert not any(index > roots[-1] and index < division and
                   (item.mnemonic == "fstp" or
                    (item.mnemonic == "fld" and "[" in item.op_str))
                   for index, item in enumerate(instructions)), (
        "the root must not be popped or reloaded through a rounded temporary")
    stores = [index for index, item in enumerate(instructions)
              if index > division and item.mnemonic == "fstp"
              and "dword ptr" in item.op_str]
    assert stores, "reciprocal must cross an explicit float32 store boundary"
    store = stores[0]
    import re
    operands = re.findall(r"\[[^\]]+\]", instructions[store].op_str)
    assert len(operands) == 1, "reciprocal store must have one local memory destination"
    scale_slot = operands[0]
    scale_reads = [item for item in instructions[store + 1:]
                   if scale_slot in item.op_str and
                   item.mnemonic in ("fld", "fmul", "fmuls")]
    assert len(scale_reads) >= 3, (
        "the rounded reciprocal slot must be read for all three component products")

    length = vector_obj.function("?vec3_length@@YAMPBM@Z")
    code, _, _ = vector_obj.subject(length)
    instructions = tuple(decoder.disasm(code, 0))
    assert any(item.mnemonic == "fsqrt" for item in instructions)
    assert not any(item.mnemonic.startswith("call") for item in instructions)

    normalize = vector_obj.function("?normalize@Vector@@QAE?AV1@XZ")
    _, relocations, _ = vector_obj.subject(normalize)
    targets = [vector_obj.symbols[int(item.target)].name.lower()
               for item in relocations]
    assert any("sqrt" in target for target in targets), (
        "Vector::normalize must retain its neighboring CRT sqrt reference")


@pytest.mark.parametrize("profile_index", (0, 1))
def test_managed_vc6_profiles_keep_the_original_numeric_paths(tmp_path,
                                                               profile_index):
    if not os.environ.get("MATCH_RECOVERY_RESOURCE_LEASE"):
        pytest.skip("native controls require a coordinator-owned compiler lease")
    from match_recovery.measurement import Measurement
    from match_recovery.pe_coff import Coff
    from match_recovery.processes import run_bounded
    from match_recovery.source_inventory import image_pin
    from recovery.environment.compile import compile_object
    from recovery.environment.wine import VC6_ROOT, environment

    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None:
        pytest.skip("VC6 and Wine32 are required")
    settings = Measurement.read(ROOT)
    assert image_pin(ROOT) == PIN
    assert len(settings.flag_sets) == 2, "both configured native profiles are required"
    parts = actual_parts()
    wrapper = ROOT / "recovery/environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")

    def compile_variant(name: str, *, unrounded: bool = False,
                        length_runtime_sqrt: bool = False,
                        neighbor_intrinsic: bool = False):
        source = tmp_path / (name + ".cpp")
        source.write_text(native_source(
            parts, unrounded=unrounded,
            length_runtime_sqrt=length_runtime_sqrt,
            neighbor_intrinsic=neighbor_intrinsic))
        raw = compile_object(source, command,
                             (*settings.flag_sets[profile_index],
                              *settings.layout_flags,
                              "/FI" + str(ROOT / "src/vc6_compat.h")))
        return source, Coff(raw)

    source, actual = compile_variant("actual")
    assert_native_arithmetic_paths(actual)
    obj_path = tmp_path / "actual.obj"
    obj_path.write_bytes(actual.raw_data)
    executable = tmp_path / "actual.exe"
    linked = run_bounded(
        [sys.executable, str(wrapper), "--tool=link", "/nologo",
         "/subsystem:console", "/incremental:no", "/out:" + str(executable),
         str(obj_path), "kernel32.lib"], cwd=tmp_path, timeout=60)
    assert linked.returncode == 0, (linked.stdout + linked.stderr).decode(errors="replace")
    result = subprocess.run(["wine32", str(executable)], env=environment(),
                            capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stdout + result.stderr

    _, unrounded = compile_variant("unrounded", unrounded=True)
    with pytest.raises(AssertionError):
        assert_native_arithmetic_paths(unrounded)
    _, runtime_sqrt = compile_variant("runtime_sqrt", length_runtime_sqrt=True)
    with pytest.raises(AssertionError):
        assert_native_arithmetic_paths(runtime_sqrt)
    _, wrong_neighbor = compile_variant("wrong_neighbor", neighbor_intrinsic=True)
    with pytest.raises(AssertionError):
        assert_native_arithmetic_paths(wrong_neighbor)
