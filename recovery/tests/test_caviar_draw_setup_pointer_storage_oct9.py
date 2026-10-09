"""Exercise the actual bounded Caviar draw-setup copy and raster addresses.

The host fixture checks source behavior and pointer identity; its pointer width
is not evidence for the native x86 global layout or allocation extent.
"""

from pathlib import Path
import shutil
import subprocess
import tempfile


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/caviar.cpp").is_file())
PRODUCT = ROOT / "src/caviar.cpp"
KERNELS = ROOT / "src/caviar_raster_kernels.inc"


def extract_definition(source: str, signature: str) -> str:
    start = source.find(signature)
    assert start >= 0, "missing current source definition: " + signature
    opening = source.find("{", start)
    depth = 0
    state = "code"
    index = opening
    while index < len(source):
        char = source[index]
        following = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == '"': state = "string"
            elif char == "'": state = "char"
            elif char == "/" and following == "/": state = "line"; index += 1
            elif char == "/" and following == "*": state = "block"; index += 1
            elif char == "{": depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[start:index + 1]
        elif state in ("string", "char"):
            if char == "\\": index += 1
            elif (state == "string" and char == '"') or (state == "char" and char == "'"):
                state = "code"
        elif state == "line" and char == "\n":
            state = "code"
        elif state == "block" and char == "*" and following == "/":
            state = "code"; index += 1
        index += 1
    raise AssertionError("unbalanced source definition: " + signature)


def producer_block(product: str) -> tuple[str, str]:
    draw = extract_definition(product, "void __cdecl vox_draw_scene(")
    start = draw.index("vox_setup_copy_.colour_row_step_ =")
    finish = draw.index("vox_setup_copy_.shadow_buffer_ =", start)
    finish = draw.index(";", finish) + 1
    copies = draw[start:finish]
    callback_start = draw.index("if (VoxRenderRecord::SetupCallback callback = record->setup_callback_)", finish)
    callback_end = draw.index("}", callback_start) + 1
    callback = draw[callback_start:callback_end]
    fields = (
        "colour_row_step_", "depth_row_step_", "shadow_row_step_",
        "colour_buffer_", "depth_buffer_", "shadow_buffer_",
    )
    positions = [copies.index("vox_setup_copy_." + field) for field in fields]
    assert positions == sorted(positions), "scratch store order changed"
    assert finish <= callback_start, "setup callback moved ahead of scratch copies"
    return copies, callback


FIXTURE = r'''#include <assert.h>
#include <stdint.h>
#include <string.h>
#ifndef __cdecl
#define __cdecl
#endif
struct CaviarSceneSetupDescriptorCandidate {
  void *colour_buffer_; void *depth_buffer_; void *shadow_buffer_;
  int32_t colour_row_step_; int32_t depth_row_step_; int32_t shadow_row_step_;
};
struct VoxRenderRecord {
  typedef void (__cdecl *SetupCallback)();
  CaviarSceneSetupDescriptorCandidate *setup_block_;
  SetupCallback setup_callback_;
};
CaviarDrawSetupPointerStorageCandidate vox_setup_copy_ = {};
static VoxRenderRecord *active_record;
static unsigned callback_count;
static void *expected_colour;
static void *expected_depth;
static void *expected_shadow;
static uint32_t expected_colour_step = (uint32_t)-7;
static uint32_t expected_depth_step = 0x80000000u;
static uint32_t expected_shadow_step = 19u;
static void __cdecl observe_setup() {
  assert(active_record != 0);
  assert(vox_setup_copy_.colour_buffer_ == expected_colour);
  assert(vox_setup_copy_.depth_buffer_ == expected_depth);
  assert(vox_setup_copy_.shadow_buffer_ == expected_shadow);
  assert(vox_setup_copy_.colour_row_step_ == expected_colour_step);
  assert(vox_setup_copy_.depth_row_step_ == expected_depth_step);
  assert(vox_setup_copy_.shadow_row_step_ == expected_shadow_step);
  ++callback_count;
  active_record->setup_block_->colour_buffer_ = 0;
  active_record->setup_block_->depth_buffer_ = 0;
  active_record->setup_block_->shadow_buffer_ = 0;
  active_record->setup_block_->colour_row_step_ = 0;
  active_record->setup_block_->depth_row_step_ = 0;
  active_record->setup_block_->shadow_row_step_ = 0;
}
'''


MAIN = r'''
int main() {
  uint8_t colour[256] = {}, depth_bytes[256] = {}, shadow[256] = {};
  CaviarSceneSetupDescriptorCandidate descriptor = {};
  descriptor.colour_buffer_ = colour;
  descriptor.depth_buffer_ = depth_bytes;
  descriptor.shadow_buffer_ = shadow;
  descriptor.colour_row_step_ = -7;
  descriptor.depth_row_step_ = (int32_t)0x80000000u;
  descriptor.shadow_row_step_ = 19;
  VoxRenderRecord record_value = { &descriptor, &observe_setup };
  VoxRenderRecord *record = &record_value;
  active_record = record;
  expected_colour = colour; expected_depth = depth_bytes; expected_shadow = shadow;
  /* ACTUAL_DESCRIPTOR_COPY */
  /* ACTUAL_SETUP_CALLBACK */
  assert(callback_count == 1);
  assert(vox_setup_copy_.colour_buffer_ == colour);
  assert(vox_setup_copy_.depth_buffer_ == depth_bytes);
  assert(vox_setup_copy_.shadow_buffer_ == shadow);
  assert(vox_setup_copy_.colour_row_step_ == (uint32_t)-7);
  assert(vox_setup_copy_.depth_row_step_ == 0x80000000u);
  assert(vox_setup_copy_.shadow_row_step_ == 19u);

  uint8_t colour2[256] = {}, depth_bytes2[256] = {}, shadow2[256] = {};
  CaviarSceneSetupDescriptorCandidate descriptor2 = {};
  descriptor2.colour_buffer_ = colour2;
  descriptor2.depth_buffer_ = depth_bytes2;
  descriptor2.shadow_buffer_ = shadow2;
  descriptor2.colour_row_step_ = 23;
  descriptor2.depth_row_step_ = -3;
  descriptor2.shadow_row_step_ = 31;
  record->setup_block_ = &descriptor2;
  expected_colour = colour2; expected_depth = depth_bytes2; expected_shadow = shadow2;
  expected_colour_step = 23u; expected_depth_step = (uint32_t)-3;
  expected_shadow_step = 31u;
  /* ACTUAL_DESCRIPTOR_COPY_AGAIN */
  /* ACTUAL_SETUP_CALLBACK */
  assert(callback_count == 2);
  assert(vox_setup_copy_.colour_buffer_ == colour2);
  assert(vox_setup_copy_.depth_buffer_ == depth_bytes2);
  assert(vox_setup_copy_.shadow_buffer_ == shadow2);
  assert(vox_setup_copy_.colour_row_step_ == 23u);
  assert(vox_setup_copy_.depth_row_step_ == (uint32_t)-3);
  assert(vox_setup_copy_.shadow_row_step_ == 31u);

  uint8_t *colour_out[4]; uint16_t *depth_out[4]; uint8_t *shadow_out[4];
  vox_setup_copy_.colour_buffer_ = colour;
  vox_setup_copy_.depth_buffer_ = depth_bytes;
  vox_setup_copy_.shadow_buffer_ = shadow;
  vox_setup_copy_.colour_row_step_ = 16;
  vox_setup_copy_.depth_row_step_ = 32;
  vox_setup_copy_.shadow_row_step_ = 24;
  caviar_part_output_bases_candidate(2, colour_out, depth_out);
  caviar_shadow_output_bases_candidate(shadow_out);
  assert(colour_out[0] == colour && colour_out[1] == colour + 2);
  assert(colour_out[2] == colour + 32 && colour_out[3] == colour + 34);
  assert(depth_out[0] == (uint16_t *)depth_bytes && depth_out[1] == (uint16_t *)depth_bytes + 1);
  assert(depth_out[2] == (uint16_t *)depth_bytes + 32 && depth_out[3] == (uint16_t *)depth_bytes + 33);
  assert(shadow_out[0] == shadow && shadow_out[1] == shadow + 1);
  assert(shadow_out[2] == shadow + 24 && shadow_out[3] == shadow + 25);
  caviar_part_output_bases_candidate(1, colour_out, depth_out);
  assert(colour_out[2] == colour + 16 && colour_out[3] == colour + 17);
  assert(depth_out[2] == (uint16_t *)depth_bytes + 32);
  return 0;
}
'''


def assembled(product: str, kernels: str) -> str:
    copies, callback = producer_block(product)
    candidate = extract_definition(product,
        "struct CaviarDrawSetupPointerStorageCandidate")
    part = extract_definition(kernels, "static void caviar_part_output_bases_candidate(")
    shadow = extract_definition(kernels, "static void caviar_shadow_output_bases_candidate(")
    main = MAIN.replace("/* ACTUAL_DESCRIPTOR_COPY */", copies).replace(
        "/* ACTUAL_DESCRIPTOR_COPY_AGAIN */", copies).replace(
        "/* ACTUAL_SETUP_CALLBACK */", callback)
    return (FIXTURE.replace("CaviarDrawSetupPointerStorageCandidate vox_setup_copy_ = {};",
                            candidate + ";\nCaviarDrawSetupPointerStorageCandidate vox_setup_copy_ = {};") +
            part + "\n" + shadow + "\n" + main)


def run(compiler: str, source: str, success: bool):
    with tempfile.TemporaryDirectory(prefix="caviar-draw-setup-") as directory:
        cpp = Path(directory) / "actual.cpp"
        exe = Path(directory) / "actual"
        cpp.write_text(source)
        built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra",
                                "-pedantic", str(cpp), "-o", str(exe)],
                               stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                               text=True)
        assert built.returncode == 0, built.stdout + built.stderr
        result = subprocess.run([str(exe)], check=False)
        assert (result.returncode == 0) == success, (result.returncode, success)


def test_actual_descriptor_copy_callback_and_raster_destinations():
    product, kernels = PRODUCT.read_text(), KERNELS.read_text()
    source = assembled(product, kernels)
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers, "GCC or Clang is required for this host control"
    for compiler in compilers:
        run(compiler, source, True)


def test_descriptor_and_destination_mutants_are_rejected():
    product, kernels = PRODUCT.read_text(), KERNELS.read_text()
    variants = [
        (product.replace("record->setup_block_->colour_row_step_);",
                         "record->setup_block_->depth_row_step_);", 1), kernels),
        (product.replace("vox_setup_copy_.colour_buffer_ = record->setup_block_->colour_buffer_;",
                         "vox_setup_copy_.colour_buffer_ = record->setup_block_->depth_buffer_;", 1), kernels),
        (product, kernels.replace("colour + colour_pitch + pixel_size;",
                                 "colour + colour_pitch;", 1)),
    ]
    compiler = shutil.which("g++") or shutil.which("clang++")
    assert compiler
    for changed_product, changed_kernels in variants:
        assert changed_product != product or changed_kernels != kernels
        run(compiler, assembled(changed_product, changed_kernels), False)
