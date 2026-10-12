import pathlib
import shutil
import subprocess
import tempfile


ROOT = next(parent for parent in pathlib.Path(__file__).resolve().parents
            if (parent / "src/caviar.cpp").is_file())
SOURCE = ROOT / "src/caviar.cpp"
HEADER = ROOT / "src/caviar.h"


def extract(source, signature):
    begin = source.find(signature)
    assert begin >= 0, "missing actual source: " + signature
    opening = source.find("{", begin)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[begin:index + 1]
    raise AssertionError("unbalanced source body")


FIXTURE = r'''#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#ifndef __cdecl
#define __cdecl
#endif
struct VOX_Vect { float values[3]; };
struct VOX_Matrix { float values[3][3]; };
'''


HARNESS = r'''
static CaviarOutputMessageWindowCandidate CaviarOutputMessageWindow = {0};
static const char vox_msg_no_address_tables[] = "No room for address tables";
static CaviarSceneSetupDescriptorCandidate *setup_mutation;
static unsigned alloc_calls, free_calls;
static unsigned long requested[2];
static int allocation_mode;
static uint16_t *ramp_a_storage[8];
static uint16_t depth_sentinel;
static int32_t ramp_b_storage[8];
static void *alloc_stub(unsigned long size) {
    requested[alloc_calls] = size;
    ++alloc_calls;
    if (setup_mutation != 0) {
        if (alloc_calls == 1) {
            setup_mutation->height_ = 3;
        } else {
            setup_mutation->height_ = 2;
            setup_mutation->depth_row_step_ = 0x20;
            setup_mutation->colour_row_step_ = 7;
        }
    }
    if ((allocation_mode == 1 && alloc_calls == 2) ||
        (allocation_mode == 2 && alloc_calls == 1) ||
        allocation_mode == 3) return 0;
    return alloc_calls == 1 ? static_cast<void *>(ramp_a_storage)
                            : static_cast<void *>(ramp_b_storage);
}
static void free_stub(void *) { ++free_calls; }
#define vox_alloc_slot alloc_stub
#define vox_free_slot free_stub
'''


MAIN = r'''
int main() {
    CaviarSceneSetupDescriptorCandidate setup = {};
    VoxRenderRecord record = {};
    setup_mutation = &setup;
    setup.height_ = 2;
    setup.depth_buffer_ = reinterpret_cast<void *>(static_cast<uintptr_t>(0x1000));
    setup.depth_row_step_ = 0x10;
    setup.colour_row_step_ = 5;
    record.setup_block_ = &setup;
    memset(ramp_a_storage, 0, sizeof(ramp_a_storage));
    memset(ramp_b_storage, 0, sizeof(ramp_b_storage));
    assert(sub_63f9b0(&record) == 0);
    assert(alloc_calls == 2 && requested[0] == 8 && requested[1] == 12);
    assert(record.ramp_a_ptr_ == ramp_a_storage);
    assert(record.ramp_b_ptr_ == ramp_b_storage);
    assert(reinterpret_cast<uintptr_t>(ramp_a_storage[0]) == 0x1000);
    assert(reinterpret_cast<uintptr_t>(ramp_a_storage[1]) == 0x1040);
    assert(ramp_b_storage[0] == 0 && ramp_b_storage[1] == 7);

    // Zero base retains both new allocations and returns success without
    // initializing either ramp entry (inspect object bytes, not typed values).
    memset(ramp_a_storage, 0xA5, sizeof(ramp_a_storage));
    memset(ramp_b_storage, 0xA5, sizeof(ramp_b_storage));
    setup.depth_buffer_ = 0;
    alloc_calls = 0; setup_mutation = 0;
    assert(sub_63f9b0(&record) == 0);
    for (unsigned i = 0; i < sizeof(uint16_t *); ++i)
        assert(reinterpret_cast<unsigned char *>(ramp_a_storage)[i] == 0xA5);
    for (unsigned i = 0; i < sizeof(int32_t); ++i)
        assert(reinterpret_cast<unsigned char *>(ramp_b_storage)[i] == 0xA5);

    // The second-allocation failure preserves the first allocation and copies
    // exactly 27 bytes into the actual message member.
    allocation_mode = 1; alloc_calls = 0;
    memset(CaviarOutputMessageWindow.message_, 0xCC,
           sizeof(CaviarOutputMessageWindow.message_));
    assert(sub_63f9b0(&record) == 1);
    assert(alloc_calls == 2 && record.ramp_a_ptr_ == ramp_a_storage &&
           record.ramp_b_ptr_ == 0);
    assert(memcmp(CaviarOutputMessageWindow.message_,
                  "No room for address tables", 27) == 0);
    assert(static_cast<unsigned char>(CaviarOutputMessageWindow.message_[27]) == 0xCC);

    // Original SHL sizing is modulo 32 bits. Force both allocators to fail,
    // so even huge wrapped sizes are observed but never allocated.
    allocation_mode = 3; alloc_calls = 0; setup_mutation = 0;
    setup.height_ = -1;
    assert(sub_63f9b0(&record) == 1);
    assert(alloc_calls == 2 && requested[0] == 0xFFFFFFFCUL &&
           requested[1] == 0xFFFFFFFCUL);
    alloc_calls = 0;
    setup.height_ = 0x40000000;
    assert(sub_63f9b0(&record) == 1);
    assert(alloc_calls == 2 && requested[0] == 0 && requested[1] == 0);
    return 0;
}
'''


def build_and_run(body, expected_success=True):
    compiler = shutil.which("g++")
    assert compiler, "GCC is required for the actual-source host control"
    header = HEADER.read_text()
    source = SOURCE.read_text()
    declarations = (
        extract(header, "struct VoxIoCallbacks {") + ";\n" +
        extract(header, "struct CaviarSceneSetupDescriptorCandidate {") + ";\n" +
        extract(header, "struct VoxRenderClipBounds {") + ";\n" +
        extract(header, "struct VoxRenderRecord {") + ";\n" +
        extract(header, "struct CaviarOutputMessageWindowCandidate {") + ";\n")
    with tempfile.TemporaryDirectory(prefix="caviar-ramps-") as directory:
        cpp = pathlib.Path(directory) / "actual_ramps.cpp"
        exe = pathlib.Path(directory) / "actual_ramps"
        cpp.write_text(FIXTURE + declarations + HARNESS + body + MAIN)
        result = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-pedantic",
             str(cpp), "-o", str(exe)],
            text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        assert result.returncode == 0, result.stdout
        run = subprocess.run([str(exe)], check=False)
        assert (run.returncode == 0) == expected_success


def test_actual_ramp_body_preserves_borrowed_descriptor_and_distinct_domains():
    header = HEADER.read_text()
    source = SOURCE.read_text()
    assert "CaviarSceneSetupDescriptorCandidate *setup_block_" in header
    assert "uint16_t **ramp_a_ptr_" in header
    assert "int32_t *ramp_b_ptr_" in header
    assert "const int32_t *vox_active_ramp_b_" in source
    output_start = header.index("struct CaviarOutputMessageWindowCandidate {")
    output_end = header.index("};", output_start)
    output_view = header[output_start:output_end]
    assert "union {" not in output_view
    assert "uint8_t callback_prefix_[offsetof(VoxIoCallbacks, seek)]" in output_view
    assert "uint32_t projection_words_[0x6B]" in output_view
    assert "memcpy(&callback" in source
    body = extract(source, "extern \"C\" char __cdecl sub_63f9b0(VoxRenderRecord *record) {")
    build_and_run(body)


def test_ramp_controls_reject_mutated_base_and_second_allocation_flow():
    source = SOURCE.read_text()
    body = extract(source, "extern \"C\" char __cdecl sub_63f9b0(VoxRenderRecord *record) {")
    mutated = body.replace("if (base_bits == 0)", "if (base_bits != 0)", 1)
    assert mutated != body
    try:
        build_and_run(mutated)
    except AssertionError:
        return
    raise AssertionError("base-guard mutation survived actual-source execution")


def test_unsigned_wrapped_sizes_and_signed_shift_mutant():
    source = SOURCE.read_text()
    body = extract(source, "extern \"C\" char __cdecl sub_63f9b0(VoxRenderRecord *record) {")
    build_and_run(body)
    signed = body.replace(
        "static_cast<uint32_t>(\n            typed->setup_block_->height_) << 2",
        "typed->setup_block_->height_ << 2", 1)
    assert signed != body
    build_and_run(signed, expected_success=False)
