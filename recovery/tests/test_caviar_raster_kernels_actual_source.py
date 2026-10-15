"""Bounded host controls for the current Caviar raster callback bodies.

The test compiles definitions extracted from current configured C++ files. Its
golden outcomes are derived from pinned loop/compare/store/record instructions
listed in caviar-kernel-closure-oct6.md, not from another execution of the
candidate. Entry roots are the pinned 0x6647F0/0x664000/0x665640/0x664E50,
0x666440/0x665CA0/0x667B20/0x667380 part callbacks and 0x666FD0/0x666A80
shadow callbacks. Their record dispatch sites are 0x63E950 and 0x63F000.
Exceptional x87 state, invalid conversions, allocation bounds and native linked
layout are deliberately not qualified here.
Setup preparation helpers are host doubles: setup checks establish the selected
callback identities, not preparation or self-modifying-code equivalence.
"""

from pathlib import Path
import os
import shutil
import subprocess
import tempfile


REPOSITORY = next(p for p in Path(__file__).resolve().parents
                  if (p / "src/caviar.cpp").is_file())
# An explicit virtual source tree can be selected by coordinator validation.
# The default reads only current configured C++ owners, never ignored overlays.
SOURCE_ROOT = Path(os.environ.get("OPENSMACX_CAVIAR_SOURCE_ROOT", REPOSITORY)).resolve()
KERNELS = SOURCE_ROOT / "src/caviar_raster_kernels.inc"
PRODUCT = SOURCE_ROOT / "src/caviar.cpp"

PREAMBLE = r'''#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <fenv.h>
#ifndef __cdecl
#define __cdecl
#endif
struct CaviarProjectionCell { int32_t x_, y_, z_; uint32_t spare_; };
struct CaviarKernelStepRowCandidate { int32_t x_, y_, z_; uint32_t spare_; };
struct CaviarKernelAliasTailCandidate {
  float scratch_e0_, scratch_e4_, scratch_e8_, scratch_ec_;
  float scratch_f0_, scratch_f4_; uint32_t extent_; float scratch_fc_;
  float scratch_a00_, scratch_a04_; uint8_t spare_[0x38];
};
struct CaviarKernelViews { CaviarProjectionCell cells_[26]; CaviarKernelAliasTailCandidate tail_; };
union CaviarKernelProjectionWindowCandidate {
  CaviarKernelViews current_views_; CaviarKernelStepRowCandidate raster_step_rows_[32];
};
struct CaviarOutputMessageWindowCandidate { uint32_t projection_words_[0x6b]; uint8_t gap_[0x24]; char message_[0x100]; };
CaviarKernelProjectionWindowCandidate CaviarKernelProjectionWindow = {};
CaviarOutputMessageWindowCandidate CaviarOutputMessageWindow = {};
struct CaviarDrawSetupPointerStorageCandidate {
  void *depth_buffer_; void *colour_buffer_; void *shadow_buffer_;
  uint32_t colour_row_step_; uint32_t depth_row_step_; uint32_t shadow_row_step_;
};
CaviarDrawSetupPointerStorageCandidate vox_setup_copy_ = {};
int32_t vox_projection_octant_mask_ = 1;

/* CURRENT KERNEL VIEW DECLARATIONS */
static CaviarPreparedKernelBindingsCandidate caviar_prepared_kernel_bindings_candidate = {};

typedef void (__cdecl *PartContained)(int32_t,int32_t,int32_t,uint32_t,const uint8_t*,int32_t);
typedef void (__cdecl *PartClipped)(int32_t,int32_t,int32_t,const int32_t*,uint32_t,const uint8_t*,int32_t);
typedef void (__cdecl *ShadowContained)(int32_t,int32_t,uint32_t,const uint8_t*,int32_t);
typedef void (__cdecl *ShadowClipped)(int32_t,int32_t,const int32_t*,uint32_t,const uint8_t*,int32_t);
typedef void (__cdecl *SetupCallback)();
struct CaviarSceneSetupDescriptorCandidate { uint8_t pixel_size_; };
struct VoxRenderRecord {
  uint32_t setup_id_;
  CaviarSceneSetupDescriptorCandidate *setup_block_;
  PartContained part_contained_; PartClipped part_clipped_;
  ShadowContained shadow_contained_; ShadowClipped shadow_clipped_;
  SetupCallback setup_callback_;
};
void __cdecl caviar_original_666fd0(int32_t,int32_t,uint32_t,const uint8_t*,int32_t);
void __cdecl caviar_original_666a80(int32_t,int32_t,const int32_t*,uint32_t,const uint8_t*,int32_t);
void __cdecl caviar_original_6647f0(int32_t,int32_t,int32_t,uint32_t,const uint8_t*,int32_t);
void __cdecl caviar_original_664000(int32_t,int32_t,int32_t,const int32_t*,uint32_t,const uint8_t*,int32_t);
void __cdecl caviar_original_665640(int32_t,int32_t,int32_t,uint32_t,const uint8_t*,int32_t);
void __cdecl caviar_original_664e50(int32_t,int32_t,int32_t,const int32_t*,uint32_t,const uint8_t*,int32_t);
void __cdecl caviar_original_666440(int32_t,int32_t,int32_t,uint32_t,const uint8_t*,int32_t);
void __cdecl caviar_original_665ca0(int32_t,int32_t,int32_t,const int32_t*,uint32_t,const uint8_t*,int32_t);
void __cdecl caviar_original_667b20(int32_t,int32_t,int32_t,uint32_t,const uint8_t*,int32_t);
void __cdecl caviar_original_667380(int32_t,int32_t,int32_t,const int32_t*,uint32_t,const uint8_t*,int32_t);
void __cdecl caviar_original_63aee0();
void __cdecl caviar_original_63af00();
void __cdecl caviar_original_63af20();
void __cdecl caviar_original_63af40();
static void __cdecl caviar_setup_size1_aligned_candidate() {}
static void __cdecl caviar_setup_size1_odd_candidate() {}
static void __cdecl caviar_setup_size2_aligned_candidate() {}
static void __cdecl caviar_setup_size2_odd_candidate() {}
'''


def extract_definition(source: str, signature: str) -> str:
    start = source.find(signature)
    assert start >= 0, f"missing current source definition: {signature}"
    while start >= 0:
        opening = source.find("{", start)
        semicolon = source.find(";", start)
        if opening >= 0 and (semicolon < 0 or opening < semicolon):
            break
        start = source.find(signature, start + len(signature))
    assert start >= 0 and opening >= 0, f"missing function body: {signature}"
    depth = 0
    state = "code"
    i = opening
    while i < len(source):
        c = source[i]
        n = source[i + 1] if i + 1 < len(source) else ""
        if state == "code":
            if c == '"': state = "string"
            elif c == "'": state = "char"
            elif c == "/" and n == "/": state = "line"; i += 1
            elif c == "/" and n == "*": state = "block"; i += 1
            elif c == "{": depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0: return source[start:i + 1]
        elif state in ("string", "char"):
            if c == "\\": i += 1
            elif (state == "string" and c == '"') or (state == "char" and c == "'"): state = "code"
        elif state == "line" and c == "\n": state = "code"
        elif state == "block" and c == "*" and n == "/": state = "code"; i += 1
        i += 1
    raise AssertionError(f"unbalanced current source definition: {signature}")


KERNEL_FUNCTIONS = (
    "static uint32_t caviar_kernel_u32_add(",
    "static int32_t caviar_kernel_i32_from_bits(",
    "static int caviar_kernel_rounding_mode_candidate(",
    "static int32_t caviar_kernel_fistp_dword_candidate(",
    "static int32_t caviar_kernel_sar8(",
    "static void caviar_kernel_project_coordinate(",
    "static int caviar_kernel_clip_coordinate(",
    "static void caviar_kernel_store_lane(",
    "static void caviar_kernel_draw_pixel(",
    "static void caviar_kernel_walk_stream(",
    "static CaviarRasterKernelViewCandidate caviar_make_kernel_view_candidate(",
    "static void caviar_kernel_contained_scalar_candidate(",
    "static void caviar_kernel_clipped_scalar_candidate(",
    "static int caviar_shadow_clip_repair_candidate(",
    "static int caviar_shadow_clip_inside_candidate(",
    "static int caviar_shadow_clip_half_open_candidate(",
    "static void caviar_shadow_kernel_project_candidate(",
    "static void caviar_shadow_kernel_walk_candidate(",
    "static void caviar_shadow_contained_scalar_candidate(",
    "static void caviar_shadow_clipped_scalar_candidate(",
    "static void caviar_part_kernel_candidate(",
    "static void __cdecl caviar_part_s1a_contained_candidate(",
    "static void __cdecl caviar_part_s1a_clipped_candidate(",
    "static void __cdecl caviar_part_s1o_contained_candidate(",
    "static void __cdecl caviar_part_s1o_clipped_candidate(",
    "static void __cdecl caviar_part_s2a_contained_candidate(",
    "static void __cdecl caviar_part_s2a_clipped_candidate(",
    "static void __cdecl caviar_part_s2o_contained_candidate(",
    "static void __cdecl caviar_part_s2o_clipped_candidate(",
    "static void caviar_shadow_kernel_candidate(",
    "static void __cdecl caviar_shadow_clipped_callback_candidate(",
)
DISPATCHERS = (
    "void __cdecl sub_63ad60(VoxRenderRecord *record) {",
    "void __cdecl sub_63ae20(VoxRenderRecord *record) {",
)
ORIGINAL_FUNCTIONS = (
    "void __cdecl caviar_original_666fd0(",
    "void __cdecl caviar_original_6647f0(", "void __cdecl caviar_original_664000(",
    "void __cdecl caviar_original_665640(", "void __cdecl caviar_original_664e50(",
    "void __cdecl caviar_original_666440(", "void __cdecl caviar_original_665ca0(",
    "void __cdecl caviar_original_667b20(", "void __cdecl caviar_original_667380(",
    "void __cdecl caviar_original_666a80(",
)
SETUP_ORIGINALS = (
    ("void __cdecl caviar_original_63aee0(", 2, 0, "caviar_setup_size2_aligned_candidate"),
    ("void __cdecl caviar_original_63af00(", 1, 0, "caviar_setup_size1_aligned_candidate"),
    ("void __cdecl caviar_original_63af20(", 2, 1, "caviar_setup_size2_odd_candidate"),
    ("void __cdecl caviar_original_63af40(", 1, 1, "caviar_setup_size1_odd_candidate"),
)

MAIN = r'''
static uint8_t angle_rows[0x800];
static CaviarKernelStepRowCandidate steps[32];
static int32_t row_offsets[16];
static uint8_t colours[4][128];
static uint16_t depths[4][128];
static uint16_t ramp_depth_storage[16][16];
static uint16_t *ramp_depth_rows[16];
static uint8_t shadow[4][128];
static uint8_t palette8[0x10000];
static uint16_t palette16[0x10000];
static uint8_t active_size = 0x5a;
static uint8_t stream[64];

static void prepare(int size, int odd) {
  caviar_prepared_kernel_bindings_candidate.step_rows_ = steps;
  caviar_prepared_kernel_bindings_candidate.row_offsets_ = row_offsets;
  caviar_prepared_kernel_bindings_candidate.angle_rows_ = angle_rows;
  caviar_prepared_kernel_bindings_candidate.colour_table_ =
      size == 1 ? palette8 : reinterpret_cast<const uint8_t *>(palette16);
  caviar_prepared_kernel_bindings_candidate.active_size_ = &active_size;
  for (int row=0; row<16; ++row)
    ramp_depth_rows[row] = ramp_depth_storage[row];
  caviar_prepared_kernel_bindings_candidate.depth_rows_ = ramp_depth_rows;
  for (int i=0; i<4; ++i) {
    caviar_prepared_kernel_bindings_candidate.colour_lane_base_[i] = colours[i];
    caviar_prepared_kernel_bindings_candidate.depth_lane_base_[i] = depths[i];
    caviar_prepared_kernel_bindings_candidate.shadow_lane_base_[i] = shadow[i];
  }
  (void)odd;
}
static void clear_stream() { memset(stream, 0, sizeof(stream)); stream[0] = 1; }
static void assert_color_pixel(int size, int idx, uint16_t value, uint16_t depth) {
  for (int i=0; i<4; ++i) {
    if (size == 1) assert(colours[i][idx] == (uint8_t)value);
    else assert(reinterpret_cast<uint16_t *>(colours[i])[idx] == value);
    if (size == 1) assert(depths[i][idx] == depth);
    else assert(ramp_depth_rows[i / 2][idx + i % 2] == depth);
  }
}
int main() {
  const int32_t clip[4] = {0,0,2,2};
  for (int i=0; i<16; ++i) row_offsets[i] = i * 8;
  angle_rows[0] = 3;
  palette8[0x305] = 0x6c;
  palette16[0x305] = 0x7b2d;
  // Native record selector mask at +0, selector at +8, first pixel at +9.
  // The selected count-path pixel executes one 3-byte iteration; row offset
  // and x are measured independently, with four native lane writes when bit0=0.
  for (int size=1; size<=2; ++size) {
    for (int odd=0; odd<=1; ++odd) {
      prepare(size, odd); clear_stream(); stream[8]=0; stream[11]=5;
      for (int i=0; i<4; ++i) { memset(colours[i],0,sizeof(colours[i]));
        for (int j=0;j<128;++j) depths[i][j] = odd ? 50 : 150; }
      for (int row=0; row<16; ++row)
        for (int col=0; col<16; ++col)
          ramp_depth_storage[row][col] = odd ? 50 : 150;
      VoxRenderRecord record = {};
      // The pinned 0x63E950 record dispatch selects one of the eight part
      // entry roots listed in the module evidence note.
      CaviarSceneSetupDescriptorCandidate setup = {(uint8_t)size};
      record.setup_id_ = (uint32_t)odd;
      record.setup_block_ = &setup;
      if ((size + odd) & 1) sub_63ad60(&record);
      else sub_63ae20(&record);
      assert(record.part_contained_ != 0 && record.part_clipped_ != 0);
      /* CHECK_SETUP_CALLBACK */
      // Original slot -> extracted source wrapper -> actual profile body.
      // z enters as 8.8 here and becomes a 16.16 depth word after the kernel's
      // native <<8. z=0x6400 therefore presents depth 100 to unsigned compare.
      record.part_contained_(256,0,0x6400,1,stream,0);
      // x=256 maps to pixel index row_offsets[0]+1 == 1.
      const int idx = 1;
      assert_color_pixel(size, idx, size == 1 ? 0x6c : 0x7b2d, 100);
    }
  }

  // Packed walk: a rejected header skips 12 + 3*byte(+4), then selects the
  // next header. Pinned 6647F0 confirms the 12-byte prefix and +3 pixel stride.
  prepare(1,0); clear_stream();
  stream[0]=0; stream[4]=1; stream[8]=0;
  stream[15]=1; stream[23]=0; stream[26]=5;
  memset(colours,0,sizeof(colours)); for(int i=0;i<4;++i) for(int j=0;j<128;++j) depths[i][j]=200;
  caviar_original_6647f0(0,0,0,1,stream,0);
  for(int i=0;i<4;++i) assert(colours[i][0] == 0x6c);

  // Clip repair is distinct from conventional clamping: the inclusive right
  // and bottom edges are repaired inward by one; farther points are rejected.
  prepare(1,0); clear_stream(); stream[8]=0; stream[11]=5;
  memset(colours,0,sizeof(colours)); for(int i=0;i<4;++i) for(int j=0;j<128;++j) depths[i][j]=200;
  caviar_original_664000(512,512,0,clip,1,stream,0);
  for(int i=0;i<4;++i) assert(colours[i][row_offsets[1]+1] == 0x6c);
  memset(colours,0,sizeof(colours));
  caviar_original_664000(768,0,0,clip,1,stream,0);
  for(int i=0;i<4;++i) assert(colours[i][0] == 0);

  // Depth ties do not store in either polarity: aligned roots 0x6647F0 and
  // siblings use unsigned JAE skip; odd roots 0x665640/0x664E50 and siblings
  // use unsigned JBE skip, as recorded in the pinned closure evidence.
  for (int odd=0; odd<=1; ++odd) {
    prepare(1,odd); clear_stream(); stream[8]=0; stream[11]=5;
    for(int i=0;i<4;++i) { depths[i][1]=100; colours[i][1]=0x11; }
    if (odd) caviar_original_665640(256,0,0x6400,1,stream,0);
    else caviar_original_6647f0(256,0,0x6400,1,stream,0);
    for(int i=0;i<4;++i) { assert(depths[i][1]==100); assert(colours[i][1]==0x11); }
  }

  // At pinned 0x666FD0 (contained dispatch from 0x63F000), the callback stores
  // the active-size byte, no depth/palette operation, and its
  // count path stores four adjacent shadow lanes from one shared row index.
  prepare(1,0); clear_stream(); stream[8]=0; stream[11]=0;
  memset(shadow,0,sizeof(shadow));
  caviar_original_666fd0(256,0,1,stream,0);
  for(int i=0;i<4;++i) assert(shadow[i][1] == active_size);
  // Clipped shadow flag 1 uses a half-open rectangle and one lane.
  memset(shadow,0,sizeof(shadow));
  caviar_original_666a80(512,0,clip,1,stream,1);
  assert(shadow[0][2] == 0); // x==right edge is excluded
  // Inclusive path (flag 0x10) accepts the right/bottom endpoints; phase D0
  // is the sentinel and is processed before terminating.
  memset(shadow,0,sizeof(shadow)); stream[11]=0xd0;
  caviar_original_666a80(512,0,clip,1,stream,0x10);
  assert(shadow[0][2] == active_size);
  return 0;
}
'''


def assembled(kernels: str, product: str) -> str:
    defs = [extract_definition(kernels, sig) for sig in KERNEL_FUNCTIONS]
    defs += [extract_definition(product, sig) for sig in ORIGINAL_FUNCTIONS]
    defs += [extract_definition(product, sig) for sig in DISPATCHERS]
    setup_asserts = []
    for signature, size, odd, _helper in SETUP_ORIGINALS:
        defs.append(extract_definition(product, signature))
        callback = signature.split("caviar_original_", 1)[1].split("(", 1)[0]
        callback = "caviar_original_" + callback
        setup_asserts.append(
            "      if (size == %d && odd == %d) assert(record.setup_callback_ == &%s);" %
            (size, odd, callback))
    main = MAIN.replace("/* CHECK_SETUP_CALLBACK */", "\n".join(setup_asserts))
    view_declarations = "\n".join(
        extract_definition(kernels, "struct " + name) + ";"
        for name in ("CaviarRasterKernelViewCandidate",
                     "CaviarShadowKernelViewCandidate",
                     "CaviarPreparedKernelBindingsCandidate"))
    preamble = PREAMBLE.replace("/* CURRENT KERNEL VIEW DECLARATIONS */",
                                view_declarations)
    preamble = preamble.replace(
        "struct CaviarDrawSetupPointerStorageCandidate {\n  void *depth_buffer_; void *colour_buffer_; void *shadow_buffer_;\n  uint32_t colour_row_step_; uint32_t depth_row_step_; uint32_t shadow_row_step_;\n};\nCaviarDrawSetupPointerStorageCandidate vox_setup_copy_ = {};",
        extract_definition(product, "struct CaviarDrawSetupPointerStorageCandidate") + ";" +
        "\nCaviarDrawSetupPointerStorageCandidate vox_setup_copy_ = {};",
    )
    return preamble + "\n".join(defs) + main


def run(compiler: str, source: str, success: bool):
    with tempfile.TemporaryDirectory(prefix="caviar-raster-controls-") as d:
        cpp = Path(d) / "actual.cpp"
        exe = Path(d) / "actual"
        cpp.write_text(source)
        built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra",
                                "-pedantic", str(cpp), "-o", str(exe)],
                               stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        assert built.returncode == 0, built.stdout + built.stderr
        result = subprocess.run([str(exe)], check=False)
        assert (result.returncode == 0) == success, (result.returncode, success)


def test_actual_kernels_and_callbacks_gcc_clang():
    kernels, product = KERNELS.read_text(), PRODUCT.read_text()
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers, "GCC or Clang is required for these host controls"
    for compiler in compilers:
        run(compiler, assembled(kernels, product), True)


def test_single_change_mutants_are_rejected():
    kernels, product = KERNELS.read_text(), PRODUCT.read_text()
    variants = [
        (kernels.replace("if (depth <= *depth_at) return;", "if (depth < *depth_at) return;", 1), product),
        (kernels.replace("record += 12 + 3 * record[4];", "record += 12 + 2 * record[4];", 1), product),
        (kernels, product.replace("caviar_part_s1a_contained_candidate(x, y, z, count, stream, flags);",
                                  "caviar_part_s2a_contained_candidate(x, y, z, count, stream, flags);", 1)),
        (kernels.replace("view.value_ = *active_size;", "view.value_ = 0;", 1), product),
    ]
    compiler = shutil.which("g++") or shutil.which("clang++")
    assert compiler
    for k, p in variants:
        assert k != kernels or p != product
        run(compiler, assembled(k, p), False)
