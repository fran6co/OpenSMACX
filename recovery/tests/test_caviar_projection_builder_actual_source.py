import pathlib
import shutil
import subprocess
import tempfile


ROOT = next(parent for parent in pathlib.Path(__file__).resolve().parents
            if (parent / "src/caviar.cpp").is_file())
PRODUCT = ROOT / "src/caviar.cpp"
HEADER = ROOT / "src/caviar.h"


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
#include <stddef.h>
#include <string.h>
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
#pragma pack(push, 1)
struct CaviarKernelTail { float scratch_e0_, scratch_e4_, scratch_e8_, scratch_ec_;
    float scratch_f0_, scratch_f4_; uint32_t vox_scene_extent_; float scratch_fc_;
    float scratch_a00_, scratch_a04_; uint8_t unknown_[0x38]; };
struct CaviarKernelKnownViews { CaviarProjectionCell cells_[0x1A]; CaviarKernelTail tail_; };
struct CaviarKernelStepRow { int32_t x_, y_, z_; uint32_t unobserved_word_; };
union CaviarKernelProjectionWindowCandidate { CaviarKernelKnownViews current_views_;
    CaviarKernelStepRow raster_step_rows_[32]; uint32_t accessed_words_[0x80]; };
#pragma pack(pop)
typedef char kernel_window_is_0x200[sizeof(CaviarKernelProjectionWindowCandidate) == 0x200 ? 1 : -1];
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
        CaviarOutputMessageWindow.projection_words_[i] = 0xfeedfaceu;
    }
    for (int i = 0; i < 0x1A; ++i) {
        CaviarKernelProjectionWindow.current_views_.cells_[i].unobserved_word_ = 0x13579BDFu;
    }
    uint32_t seek_sentinel = 0x11223344u;
    uint32_t tell_sentinel = 0xaabbccddu;
    memcpy(reinterpret_cast<unsigned char *>(&CaviarOutputMessageWindow) +
               offsetof(VoxIoCallbacks, seek), &seek_sentinel, sizeof(seek_sentinel));
    memcpy(reinterpret_cast<unsigned char *>(&CaviarOutputMessageWindow) +
               offsetof(VoxIoCallbacks, tell), &tell_sentinel, sizeof(tell_sentinel));
    caviar_build_projection_tables();
    if (CaviarKernelProjectionWindow.current_views_.cells_[0].x_ != -10 ||
        CaviarKernelProjectionWindow.current_views_.cells_[0].y_ != -31 ||
        CaviarKernelProjectionWindow.current_views_.cells_[0].z_ != -15104) return 2;
    if (CaviarOutputMessageWindow.projection_words_[5] != (uint32_t)-30 ||
        CaviarOutputMessageWindow.projection_words_[0] != (uint32_t)-167771935) {
        return 3;
    }
    if (CaviarOutputMessageWindow.projection_words_[100] == 0xfeedfaceu ||
        CaviarOutputMessageWindow.projection_words_[105] == 0xfeedfaceu ||
        CaviarOutputMessageWindow.projection_words_[106] == 0xfeedfaceu) return 4;
    uint32_t seek_after = 0;
    uint32_t tell_after = 0;
    memcpy(&seek_after, reinterpret_cast<unsigned char *>(&CaviarOutputMessageWindow) +
                            offsetof(VoxIoCallbacks, seek), sizeof(seek_after));
    memcpy(&tell_after, reinterpret_cast<unsigned char *>(&CaviarOutputMessageWindow) +
                            offsetof(VoxIoCallbacks, tell), sizeof(tell_after));
    if (CaviarKernelProjectionWindow.current_views_.cells_[0].unobserved_word_ != 0x13579BDFu ||
        CaviarKernelProjectionWindow.current_views_.cells_[25].unobserved_word_ != 0x13579BDFu ||
        CaviarOutputMessageWindow.projection_words_[1] != 0xfeedfaceu ||
        CaviarOutputMessageWindow.projection_words_[2] != tell_sentinel ||
        CaviarOutputMessageWindow.projection_words_[3] != 0xfeedfaceu) return 5;
    if (seek_after != CaviarOutputMessageWindow.projection_words_[0] ||
        tell_after != tell_sentinel || tell_after == seek_sentinel) return 7;
    if (CaviarKernelProjectionWindow.current_views_.tail_.scratch_e0_ != 13.0f ||
        CaviarKernelProjectionWindow.current_views_.tail_.scratch_e4_ != 11.0f ||
        CaviarKernelProjectionWindow.current_views_.tail_.scratch_e8_ != 7.0f ||
        CaviarKernelProjectionWindow.current_views_.tail_.scratch_ec_ != 5.0f ||
        CaviarKernelProjectionWindow.current_views_.tail_.scratch_f0_ != 3.0f ||
        CaviarKernelProjectionWindow.current_views_.tail_.scratch_f4_ != 2.0f ||
        CaviarKernelProjectionWindow.current_views_.tail_.scratch_fc_ != 5888.0f ||
        CaviarKernelProjectionWindow.current_views_.tail_.scratch_a00_ != 4864.0f ||
        CaviarKernelProjectionWindow.current_views_.tail_.scratch_a04_ != 4352.0f) return 6;
    return 0;
}
'''


def compile_and_run(compiler, body, expected_success):
    with tempfile.TemporaryDirectory() as directory:
        source = pathlib.Path(directory) / "actual_projection.cpp"
        executable = pathlib.Path(directory) / "actual_projection"
        output_type = extract_body(HEADER.read_text(),
            "struct CaviarOutputMessageWindowCandidate")
        descriptor = extract_body(HEADER.read_text(), "struct VoxIoCallbacks {")
        fixture = (FIXTURE + "\n" + descriptor + ";\n" + output_type + ";\n" +
            "typedef char projection_follows_seek[offsetof(CaviarOutputMessageWindowCandidate, projection_words_) == offsetof(VoxIoCallbacks, seek) ? 1 : -1];\n" +
            "typedef char host_storage_extent[sizeof(CaviarOutputMessageWindowCandidate) == offsetof(VoxIoCallbacks, seek) + 0x2D0 ? 1 : -1];\n" +
            "CaviarKernelProjectionWindowCandidate CaviarKernelProjectionWindow = {};\n" +
            "CaviarOutputMessageWindowCandidate CaviarOutputMessageWindow = {};\n" +
            "CaviarFixedPointProjectionMatrix CaviarFixedPointProjection = {};\n" +
            "uint32_t vox_setup_copy_[6] = {};\n")
        source.write_text(fixture + "\n" + body + MAIN)
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
    mutant = body.replace("CaviarOutputMessageWindow.projection_words_[outputIndex] =",
                          "CaviarOutputMessageWindow.projection_words_[outputIndex + 1] =", 1)
    assert mutant != body
    compile_and_run(shutil.which("g++"), mutant, False)
