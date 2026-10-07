"""Installed-source control for the typed 0x63FA80 VOX_Matrix input closure."""
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = next(p for p in Path(__file__).resolve().parents
            if (p / "src/caviar.cpp").is_file())
SOURCE = ROOT / "src/caviar.cpp"


def extract(source, signature):
    from match_recovery.includes import COMMENTS_OR_STRINGS
    masked = COMMENTS_OR_STRINGS.sub(
        lambda m: "".join("\n" if c == "\n" else " " for c in m[0]), source)
    start = masked.find(signature)
    while start >= 0:
        opening = masked.find("{", start)
        semicolon = masked.find(";", start)
        if opening >= 0 and (semicolon < 0 or opening < semicolon):
            break
        start = masked.find(signature, start + len(signature))
    assert start >= 0 and opening >= 0, "missing installed definition: " + signature
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError("unclosed installed function: " + signature)


PREAMBLE = r'''#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <math.h>
#include <sys/mman.h>
#define __cdecl
struct VOX_Matrix { float values[3][3]; };
struct CaviarFixedPointProjectionMatrix {
  int16_t word_00_, word_02_, word_04_, word_06_, word_08_, word_0A_;
  int32_t dword_0C_, dword_10_, dword_14_;
};
static CaviarFixedPointProjectionMatrix CaviarFixedPointProjection = {};
static int32_t vox_projection_render_stage_ = 7;
static int32_t vox_projection_prepared_ = 9;
static float *CaviarProjectionScaledMatrix = 0;
static const float CaviarProjectionDirectionScale = 256.0f;
static const float CaviarProjectionRoundThreshold = 0.0f;
static const float CaviarProjectionRoundBelow = -0.5f;
static const float CaviarProjectionRoundAbove = 0.5f;
'''

MAIN = r'''int main() {
  float *storage = (float *)mmap(0, 4096, PROT_READ|PROT_WRITE,
      MAP_PRIVATE|MAP_ANONYMOUS|MAP_32BIT, -1, 0);
  assert(storage != MAP_FAILED && (uintptr_t)storage < 0x80000000u);
  VOX_Matrix *matrix = (VOX_Matrix *)storage;
  matrix->values[0][0] = 0.10f; matrix->values[0][1] = -0.10f;
  matrix->values[0][2] = 0.0f; matrix->values[1][0] = 0.25f;
  matrix->values[1][1] = -0.25f; matrix->values[1][2] = 0.5f;
  matrix->values[2][0] = 1.0f; matrix->values[2][1] = -1.0f;
  matrix->values[2][2] = 2.0f;
  CaviarProjectionScaledMatrix = storage + 16;
  sub_63fa80(matrix);
  assert(CaviarProjectionScaledMatrix[0] == 25.6f);
  assert(CaviarProjectionScaledMatrix[1] == -25.6f);
  assert(CaviarProjectionScaledMatrix[8] == 512.0f);
  assert(CaviarFixedPointProjection.word_00_ == 26);
  assert(CaviarFixedPointProjection.word_02_ == -26);
  assert(CaviarFixedPointProjection.word_06_ == 64);
  assert(CaviarFixedPointProjection.word_08_ == -64);
  assert(CaviarFixedPointProjection.dword_0C_ == 256);
  assert(CaviarFixedPointProjection.dword_14_ == 512);
  assert(vox_projection_render_stage_ == 0 && vox_projection_prepared_ == 0);
}
'''


def test_matrix_input_and_all_three_installed_callers_are_typed():
    source = SOURCE.read_text()
    included = (ROOT / "src/caviar_raster_kernels.inc").read_text()
    declaration = next(line for line in included.splitlines()
                       if "sub_63fa80(" in line)
    assert declaration == 'extern "C" void __cdecl sub_63fa80(const VOX_Matrix *);'
    assert declaration in source
    body = extract(source, "void __cdecl sub_63fa80(")
    assert "sub_63fa80(const VOX_Matrix *sources)" in body
    assert "&sources->values[0][0]" in body
    caller_specs = (
        ("void __cdecl caviar_clip_shape(", "sub_63fa80(orientation);"),
        ("void __cdecl vox_draw_part(", "sub_63fa80(part_matrix);"),
        ("void __cdecl vox_draw_shadow(", "sub_63fa80(part_matrix);"),
    )
    for signature, call in caller_specs:
        assert call in extract(source, signature), signature

    definitions = "\n".join((
        declaration,
        extract(source, "int round_view_scaled("),
        body,
    ))
    compilers = (shutil.which("g++"), shutil.which("clang++"))
    assert all(compilers), "GCC and Clang are required for host controls"
    for compiler in compilers:
        with tempfile.TemporaryDirectory(prefix="caviar-matrix-host-") as temp:
            cpp = Path(temp) / "actual.cpp"
            exe = Path(temp) / "actual"
            cpp.write_text(PREAMBLE + definitions + MAIN)
            built = subprocess.run(
                [compiler, "-std=c++98", "-O0", str(cpp), "-o", str(exe)],
                capture_output=True,
                text=True,
            )
            assert built.returncode == 0, built.stdout + built.stderr
            ran = subprocess.run([str(exe)], capture_output=True, text=True)
            assert ran.returncode == 0, f"exit={ran.returncode}: {ran.stdout}{ran.stderr}"


def test_float_pointer_mutant_rejects_matrix_owner_call():
    source = SOURCE.read_text()
    body = extract(source, "void __cdecl sub_63fa80(")
    assert "const VOX_Matrix *sources" in body
    damaged = body.replace("const VOX_Matrix *sources", "float *sources", 1)
    damaged = damaged.replace("&sources->values[0][0]", "sources", 1)
    preamble = PREAMBLE + extract(source, "int round_view_scaled(") + damaged
    main = r'''int main() {
  VOX_Matrix matrix = {};
  sub_63fa80(&matrix);
}
'''
    compiler = shutil.which("g++")
    assert compiler, "GCC is required for the signature mutation control"
    with tempfile.TemporaryDirectory(prefix="caviar-matrix-mutant-") as temp:
        cpp = Path(temp) / "mutant.cpp"
        exe = Path(temp) / "mutant"
        cpp.write_text(preamble + main)
        built = subprocess.run(
            [compiler, "-std=c++98", "-O0", str(cpp), "-o", str(exe)],
            capture_output=True,
            text=True,
        )
        assert built.returncode != 0, "untyped float-pointer mutant accepted VOX_Matrix input"
