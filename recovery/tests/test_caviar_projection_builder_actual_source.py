import pathlib
import shutil
import subprocess
import tempfile


ROOT = pathlib.Path(__file__).resolve().parents[2]
PRODUCT = ROOT / "src/caviar.cpp"


def extract_body(source, signature):
    begin = source.find(signature)
    assert begin >= 0, "missing actual source body: " + signature
    opening = source.find("{", begin)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[begin:index + 1]
    raise AssertionError("unbalanced actual source body")


FIXTURE = r'''#include <stdint.h>
#ifndef __cdecl
#define __cdecl
#endif
struct CaviarProjectionCell {
    int32_t x_; int32_t y_; int32_t z_; uint32_t unobserved_word_;
};
struct CaviarFixedPointProjectionMatrix {
    int16_t word_00_; int16_t word_02_; int16_t word_04_;
    int16_t word_06_; int16_t word_08_; int16_t word_0A_;
    int32_t dword_0C_; int32_t dword_10_; int32_t dword_14_;
};
CaviarProjectionCell CaviarProjectionCells[0x1A] = {};
uint32_t CaviarProjectionOutputWords[0x6B] = {};
CaviarFixedPointProjectionMatrix CaviarFixedPointProjection = {};
uint32_t vox_setup_copy_[6] = {};
float CaviarProjectionScratchE0 = 0.0f;
float CaviarProjectionScratchE4 = 0.0f;
float CaviarProjectionScratchE8 = 0.0f;
float CaviarProjectionScratchEC = 0.0f;
float CaviarProjectionScratchF0 = 0.0f;
float CaviarProjectionScratchF4 = 0.0f;
float CaviarProjectionScratchFC = 0.0f;
float CaviarProjectionScratchA00 = 0.0f;
float CaviarProjectionScratchA04 = 0.0f;
'''


MAIN = r'''
int main() {
    CaviarFixedPointProjection.word_00_ = 2;
    CaviarFixedPointProjection.word_02_ = 3;
    CaviarFixedPointProjection.word_04_ = 5;
    CaviarFixedPointProjection.word_06_ = 7;
    CaviarFixedPointProjection.word_08_ = 11;
    CaviarFixedPointProjection.word_0A_ = 13;
    CaviarFixedPointProjection.dword_0C_ = 17;
    CaviarFixedPointProjection.dword_10_ = 19;
    CaviarFixedPointProjection.dword_14_ = 23;
    vox_setup_copy_[4] = 29;
    for (int i = 0; i < 0x6B; ++i) {
        CaviarProjectionOutputWords[i] = 0xfeedfaceu;
    }
    for (int i = 0; i < 0x1A; ++i) {
        CaviarProjectionCells[i].unobserved_word_ = 0x13579BDFu;
    }
    caviar_build_projection_tables();
    if (CaviarProjectionCells[0].x_ != -10 ||
        CaviarProjectionCells[0].y_ != -31 ||
        CaviarProjectionCells[0].z_ != -15104) return 2;
    if (CaviarProjectionOutputWords[5] != (uint32_t)-30 ||
        CaviarProjectionOutputWords[0] != (uint32_t)-167771935) {
        return 3;
    }
    if (CaviarProjectionOutputWords[100] == 0xfeedfaceu ||
        CaviarProjectionOutputWords[105] == 0xfeedfaceu ||
        CaviarProjectionOutputWords[106] == 0xfeedfaceu) return 4;
    if (CaviarProjectionCells[0].unobserved_word_ != 0x13579BDFu ||
        CaviarProjectionCells[25].unobserved_word_ != 0x13579BDFu ||
        CaviarProjectionOutputWords[1] != 0xfeedfaceu ||
        CaviarProjectionOutputWords[2] != 0xfeedfaceu ||
        CaviarProjectionOutputWords[3] != 0xfeedfaceu) return 5;
    if (CaviarProjectionScratchE0 != 13.0f ||
        CaviarProjectionScratchE4 != 11.0f ||
        CaviarProjectionScratchE8 != 7.0f ||
        CaviarProjectionScratchEC != 5.0f ||
        CaviarProjectionScratchF0 != 3.0f ||
        CaviarProjectionScratchF4 != 2.0f ||
        CaviarProjectionScratchFC != 5888.0f ||
        CaviarProjectionScratchA00 != 4864.0f ||
        CaviarProjectionScratchA04 != 4352.0f) return 6;
    return 0;
}
'''


def compile_and_run(compiler, body, expected_success):
    with tempfile.TemporaryDirectory() as directory:
        source = pathlib.Path(directory) / "actual_projection.cpp"
        executable = pathlib.Path(directory) / "actual_projection"
        source.write_text(FIXTURE + "\n" + body + MAIN)
        subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-pedantic",
            str(source), "-o", str(executable)],
            check=True,
        )
        result = subprocess.run([str(executable)], check=False)
        assert (result.returncode == 0) == expected_success


def test_extracts_actual_product_body_and_runs_with_gcc_clang():
    body = extract_body(PRODUCT.read_text(),
        "void __cdecl caviar_build_projection_tables(")
    compilers = [shutil.which("g++"), shutil.which("clang++")]
    assert all(compilers), "both GCC and Clang are required"
    for compiler in compilers:
        compile_and_run(compiler, body, True)


def test_loop_count_mutant_fails_actual_source_control():
    body = extract_body(PRODUCT.read_text(),
        "void __cdecl caviar_build_projection_tables(")
    mutant = body.replace("int localCount = 0x1a;",
                          "int localCount = 0x19;", 1)
    assert mutant != body
    compile_and_run(shutil.which("g++"), mutant, False)


def test_depth_stream_row_offset_mutant_fails_actual_source_control():
    body = extract_body(PRODUCT.read_text(),
                        "void __cdecl caviar_build_projection_tables(")
    mutant = body.replace("CaviarProjectionOutputWords[outputIndex] =",
                          "CaviarProjectionOutputWords[outputIndex + 4] =", 1)
    assert mutant != body
    compile_and_run(shutil.which("g++"), mutant, False)
