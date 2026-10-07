"""Host controls for the two mapped Caviar draw dispatchers.

Run against the actual tracked source, with no candidate-overlay selection.
Projection services and type declarations are bounded host substitutes; these
controls do not prove native layout, x87 exceptions, or whole-renderer fidelity.
The callback bodies come only from src/caviar.cpp and their shared geometry
helpers only from src/caviar_raster_kernels.inc.
"""
from pathlib import Path
import shutil
import subprocess
import tempfile


REPOSITORY = next(p for p in Path(__file__).resolve().parents
                  if (p / "src/caviar.cpp").is_file())
SOURCE_ROOT = REPOSITORY
PRODUCT = SOURCE_ROOT / "src/caviar.cpp"
KERNELS = SOURCE_ROOT / "src/caviar_raster_kernels.inc"


def extract(text: str, signature: str) -> str:
    start = text.find(signature)
    while start >= 0:
        opening = text.find("{", start)
        semicolon = text.find(";", start)
        if opening >= 0 and (semicolon < 0 or opening < semicolon):
            break
        start = text.find(signature, start + len(signature))
    assert start >= 0, f"missing current owner definition: {signature}"
    depth, state, index = 0, "code", opening
    while index < len(text):
        c = text[index]
        n = text[index + 1] if index + 1 < len(text) else ""
        if state == "code":
            if c == '"': state = "string"
            elif c == "'": state = "char"
            elif c == "/" and n == "/": state = "line"; index += 1
            elif c == "/" and n == "*": state = "block"; index += 1
            elif c == "{": depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0: return text[start:index + 1]
        elif state in ("string", "char"):
            if c == "\\": index += 1
            elif (state == "string" and c == '"') or (state == "char" and c == "'"):
                state = "code"
        elif state == "line" and c == "\n": state = "code"
        elif state == "block" and c == "*" and n == "/": state = "code"; index += 1
        index += 1
    raise AssertionError(f"unclosed current definition: {signature}")


HELPER_SIGNATURES = (
    "static int caviar_fistp_qword_low_dword(",
    "static int caviar_round_projection_coordinate(",
    "static int caviar_truncate_projection_coordinate(",
    "static void caviar_project_raster_point(",
    "static int caviar_intersect_raster_box(",
    "static void caviar_clip_record_box(",
    "static int caviar_flat_raster_box(",
    "static int caviar_raster_item_box(",
    "static void caviar_apply_projected_offset(",
    "static int caviar_box_is_inside(",
)


PREAMBLE = r'''#include <assert.h>
#include <stdint.h>
#include <string.h>
#ifndef __cdecl
#define __cdecl
#endif
struct VOX_Vect { float values[3]; };
struct VOX_Matrix { float values[3][3]; };
struct CaviarRenderClipWindowCandidate {
  int32_t left_, top_, right_, bottom_, width_, height_;
};
struct CaviarSceneProjectionScratchCandidate {
  CaviarRenderClipWindowCandidate part_clip_, shadow_clip_;
};
CaviarSceneProjectionScratchCandidate CaviarSceneProjectionScratch = {};
struct CvrShape {
  uint32_t field_0_;
  int16_t axis_x_, axis_y_, axis_z_;
  int16_t offset_x_, offset_y_, offset_z_;
  int16_t raster_origin_[3];
  uint32_t item_count_;
  const uint8_t *item_stream_;
};
#pragma pack(push, 1)
struct CvrRasterItemHeaderCandidate {
  int16_t endpoint_[6]; int16_t extent_[3]; int16_t base_[3];
  uint32_t item_count_; uint8_t flags_; uint8_t unknown_1d_[3];
  uint8_t inline_payload_[1];
};
#pragma pack(pop)
typedef void (__cdecl *PartContained)(int32_t,int32_t,int32_t,uint32_t,const uint8_t*,int32_t);
typedef void (__cdecl *PartClipped)(int32_t,int32_t,int32_t,const int32_t*,uint32_t,const uint8_t*,int32_t);
typedef void (__cdecl *ShadowContained)(int32_t,int32_t,uint32_t,const uint8_t*,int32_t);
typedef void (__cdecl *ShadowClipped)(int32_t,int32_t,const int32_t*,uint32_t,const uint8_t*,int32_t);
struct VoxRenderRecord {
  int32_t draw_left_, draw_top_, draw_right_, draw_bottom_;
  PartContained part_contained_; PartClipped part_clipped_;
  ShadowContained shadow_contained_; ShadowClipped shadow_clipped_;
};
static const float CaviarProjectionDirectionScale = 256.0f;
static const float CaviarProjectionRoundThreshold = 0.0f;
static const float CaviarProjectionRoundBelow = -0.5f;
static const float CaviarProjectionRoundAbove = 0.5f;
static int32_t vox_projection_prepared_ = 0;
static int32_t box_words[6] = {1,1,3,3,3,3};
static int direction_calls = 0, inverse_direction_calls = 0;
static int event_count = 0;
struct Event { int slot, x, y, z, flags; uint32_t count; const uint8_t *stream; int clip[4]; };
static Event events[16];
static void remember(int slot, int x, int y, int z, const int32_t *clip,
                     uint32_t count, const uint8_t *stream, int flags) {
  assert(event_count < 16);
  Event &event = events[event_count++];
  event.slot=slot; event.x=x; event.y=y; event.z=z; event.flags=flags;
  event.count=count; event.stream=stream;
  for (int i=0;i<4;++i) event.clip[i]=clip ? clip[i] : -999;
}
static void __cdecl part_contained(int32_t x,int32_t y,int32_t z,uint32_t n,const uint8_t*s,int32_t f) {
  remember(1,x,y,z,0,n,s,f);
}
static void __cdecl part_clipped(int32_t x,int32_t y,int32_t z,const int32_t*r,uint32_t n,const uint8_t*s,int32_t f) {
  remember(2,x,y,z,r,n,s,f);
}
static void __cdecl shadow_contained(int32_t x,int32_t y,uint32_t n,const uint8_t*s,int32_t f) {
  remember(3,x,y,0,0,n,s,f);
}
static void __cdecl shadow_clipped(int32_t x,int32_t y,const int32_t*r,uint32_t n,const uint8_t*s,int32_t f) {
  remember(4,x,y,0,r,n,s,f);
}
static void __cdecl sub_63fa80(float *matrix) { (void)matrix; }
static void __cdecl caviar_direction_to_projection_index(const VOX_Matrix*m,const VOX_Vect*d,float e,const VOX_Matrix*c) {
  (void)m;(void)d;(void)e;(void)c;
}
static void __cdecl caviar_build_projection_tables() {}
static void __cdecl vox_direction_to_octant(const VoxRenderRecord*r) {
  (void)r; ++direction_calls; vox_projection_prepared_=1;
}
static void __cdecl vox_inverse_direction_to_octant(const VOX_Matrix*m) {
  (void)m; ++inverse_direction_calls; vox_projection_prepared_=1;
}
static void __cdecl sub_6401b0(const int16_t*c,const int16_t*e,int*out) {
  (void)c;(void)e; for(int i=0;i<6;++i) out[i]=box_words[i];
}
static void vox_matrix_vector_mul(const float m[3][3],const float v[3],float out[3]) {
  for(int r=0;r<3;++r) out[r]=m[r][0]*v[0]+m[r][1]*v[1]+m[r][2]*v[2];
}
static void vector_add(const float*a,const float*b,float*out) {
  for(int i=0;i<3;++i) out[i]=a[i]+b[i];
}
static void reset(VoxRenderRecord &record) {
  event_count=direction_calls=inverse_direction_calls=0;
  record.draw_left_=0; record.draw_top_=0; record.draw_right_=10; record.draw_bottom_=10;
  record.part_contained_=&part_contained; record.part_clipped_=&part_clipped;
  record.shadow_contained_=&shadow_contained; record.shadow_clipped_=&shadow_clipped;
  CaviarSceneProjectionScratch.part_clip_ = {};
  CaviarSceneProjectionScratch.shadow_clip_ = {};
}
static void set_box(int l,int t,int r,int b) {
  box_words[0]=l;box_words[1]=t;box_words[2]=r;box_words[3]=b;
  box_words[4]=r-l+1;box_words[5]=b-t+1;
}
static VOX_Matrix identity_matrix() {
  VOX_Matrix m={};m.values[0][0]=m.values[1][1]=m.values[2][2]=1.0f;return m;
}
'''


MAIN = r'''
int main() {
  VoxRenderRecord record={}; VOX_Matrix matrix=identity_matrix();
  VOX_Vect position={{0,0,0}}, direction={{0,0,1}};
  CvrShape shape={}; uint8_t flat_stream[8]={1,2,3}; shape.item_stream_=flat_stream;

  // Flat part: contained route, count zero is still dispatched, bit 8 adds 0x20.
  reset(record); set_box(1,1,3,3); shape.field_0_=8; shape.item_count_=0;
  vox_draw_part(&record,&shape,&position,&matrix,&matrix,&direction,1.0f,1);
  assert(event_count==1 && events[0].slot==1 && events[0].count==0);
  assert(events[0].stream==flat_stream && events[0].flags==0x21);
  // Flat part: clipped route passes the exact effective clip rectangle.
  reset(record); set_box(8,8,12,12); shape.field_0_=0; shape.item_count_=4;
  vox_draw_part(&record,&shape,&position,&matrix,&matrix,&direction,1.0f,0);
  assert(event_count==1 && events[0].slot==2 && events[0].count==4);
  assert(events[0].clip[0]==0 && events[0].clip[1]==0 && events[0].clip[2]==10 && events[0].clip[3]==10);
  // Bit 4 substitutes rounded position and performs direction setup.
  reset(record); set_box(1,1,3,3); shape.field_0_=4; shape.item_count_=1;
  vox_draw_part(&record,&shape,&position,&matrix,&matrix,&direction,1.0f,0);
  assert(event_count==1 && events[0].slot==1 && events[0].flags==0x10 && direction_calls==1);

  // Item-list zero count returns without dispatch, unlike flat count zero above.
  reset(record); shape.field_0_=2; shape.item_count_=0;
  vox_draw_part(&record,&shape,&position,&matrix,&matrix,&direction,1.0f,1);
  assert(event_count==0);

  // Item flags bit 0 clears/sets 0x10; shape bit 8 clears/sets 0x20.
  alignas(4) uint8_t item_bytes[160]={};
  CvrRasterItemHeaderCandidate *item=reinterpret_cast<CvrRasterItemHeaderCandidate*>(item_bytes);
  item->item_count_=1; item->flags_=0;
  reset(record); set_box(1,1,3,3); shape.field_0_=2; shape.item_count_=1; shape.item_stream_=item_bytes;
  vox_draw_part(&record,&shape,&position,&matrix,&matrix,&direction,1.0f,1);
  assert(event_count==1 && events[0].slot==1 && events[0].flags==1 && direction_calls==0);
  item->flags_=1; shape.field_0_=2|8;
  reset(record); set_box(8,8,12,12);
  vox_draw_part(&record,&shape,&position,&matrix,&matrix,&direction,1.0f,1);
  assert(event_count==1 && events[0].slot==2 && events[0].flags==0x31 && direction_calls==1);
  assert(events[0].stream==item->inline_payload_);

  // Native traversal advances by item_count*3 when bit 0 is clear. The two
  // records begin at payload+3; the third is a safe sentinel for a <= mutant.
  memset(item_bytes,0,sizeof(item_bytes));
  item=reinterpret_cast<CvrRasterItemHeaderCandidate*>(item_bytes);
  CvrRasterItemHeaderCandidate *second=reinterpret_cast<CvrRasterItemHeaderCandidate*>(item->inline_payload_+3);
  CvrRasterItemHeaderCandidate *third=reinterpret_cast<CvrRasterItemHeaderCandidate*>(second->inline_payload_+3);
  item->item_count_=second->item_count_=third->item_count_=1;
  reset(record); set_box(1,1,3,3); shape.field_0_=2; shape.item_count_=2; shape.item_stream_=item_bytes;
  vox_draw_part(&record,&shape,&position,&matrix,&matrix,&direction,1.0f,1);
  assert(event_count==2);

  // Shadow uses its separate two-coordinate slots and non-null final arg.
  reset(record); set_box(1,1,3,3); shape.field_0_=8; shape.item_count_=0;
  vox_draw_shadow(&record,&shape,&position,&matrix,(const void*)1);
  assert(event_count==1 && events[0].slot==3 && events[0].count==0 && events[0].flags==0x21);
  reset(record); set_box(8,8,12,12); shape.field_0_=0; shape.item_count_=3;
  vox_draw_shadow(&record,&shape,&position,&matrix,0);
  assert(event_count==1 && events[0].slot==4 && events[0].flags==0);
  assert(events[0].clip[2]==10 && events[0].clip[3]==10);
  reset(record); set_box(1,1,3,3); shape.field_0_=2; shape.item_count_=0;
  vox_draw_shadow(&record,&shape,&position,&matrix,(const void*)1);
  assert(event_count==0);
  memset(item_bytes,0,sizeof(item_bytes)); item=reinterpret_cast<CvrRasterItemHeaderCandidate*>(item_bytes);
  item->item_count_=1; item->flags_=1; shape.item_stream_=item_bytes; shape.item_count_=1; shape.field_0_=2|8;
  reset(record); set_box(1,1,3,3);
  vox_draw_shadow(&record,&shape,&position,&matrix,(const void*)1);
  assert(event_count==1 && events[0].slot==3 && events[0].flags==0x31 && inverse_direction_calls==1);
  // Two-record shadow loop; third record is a safe loop-condition mutant witness.
  memset(item_bytes,0,sizeof(item_bytes)); item=reinterpret_cast<CvrRasterItemHeaderCandidate*>(item_bytes);
  second=reinterpret_cast<CvrRasterItemHeaderCandidate*>(item->inline_payload_+3);
  third=reinterpret_cast<CvrRasterItemHeaderCandidate*>(second->inline_payload_+3);
  item->item_count_=second->item_count_=third->item_count_=1; item->flags_=second->flags_=third->flags_=0;
  reset(record); set_box(1,1,3,3); shape.field_0_=2; shape.item_count_=2; shape.item_stream_=item_bytes;
  vox_draw_shadow(&record,&shape,&position,&matrix,(const void*)1);
  assert(event_count==2);
  return 0;
}
'''


def source_bodies(product: str, kernels: str) -> tuple[list[str], str, str]:
    assert "caviar_draw_part_candidate" not in kernels
    assert "caviar_draw_shadow_candidate" not in kernels
    helpers = [extract(kernels, signature) for signature in HELPER_SIGNATURES]
    part = extract(product, "void __cdecl vox_draw_part(")
    shadow = extract(product, "void __cdecl vox_draw_shadow(")
    assert "caviar_draw_part_candidate" not in part
    assert "caviar_draw_shadow_candidate" not in shadow
    return helpers, part, shadow


def build_and_run(compiler: str, helpers: list[str], part: str, shadow: str,
                  should_pass: bool) -> None:
    translation = PREAMBLE + "\n".join(helpers + [part, shadow]) + MAIN
    with tempfile.TemporaryDirectory(prefix="caviar-owner-dispatch-") as folder:
        cpp, binary = Path(folder) / "control.cpp", Path(folder) / "control"
        cpp.write_text(translation, encoding="utf-8")
        built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                                str(cpp), "-o", str(binary)], capture_output=True, text=True)
        assert built.returncode == 0, built.stdout + built.stderr
        run = subprocess.run([str(binary)], capture_output=True, text=True)
        assert (run.returncode == 0) == should_pass, run.stdout + run.stderr


def test_original_owner_bodies_and_bounded_dispatch_controls() -> None:
    product, kernels = PRODUCT.read_text(encoding="utf-8"), KERNELS.read_text(encoding="utf-8")
    helpers, part, shadow = source_bodies(product, kernels)
    assert "// ORIGINAL: 0x0063E950 sub_63e950" in product
    assert "// ORIGINAL: 0x0063F000 sub_63f000" in product
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    assert all(compilers), "GCC and Clang required"
    for compiler in compilers:
        build_and_run(compiler, helpers, part, shadow, True)


def test_loop_and_dispatch_route_mutants_are_rejected() -> None:
    product, kernels = PRODUCT.read_text(encoding="utf-8"), KERNELS.read_text(encoding="utf-8")
    helpers, part, shadow = source_bodies(product, kernels)
    part_mutants = (
        (part.replace("} while (visited_items < shape->item_count_);",
                      "} while (visited_items <= shape->item_count_);", 1), shadow),
        (part.replace("if (contained) {", "if (!contained) {", 1), shadow),
    )
    shadow_mutants = (
        (part, shadow.replace("} while (visited_items < shape->item_count_);",
                              "} while (visited_items <= shape->item_count_);", 1)),
        (part, shadow.replace("if (contained) {", "if (!contained) {", 1)),
    )
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    assert all(compilers), "GCC and Clang required"
    for compiler in compilers:
        for mutant_part, mutant_shadow in part_mutants + shadow_mutants:
            assert (mutant_part, mutant_shadow) != (part, shadow)
            build_and_run(compiler, helpers, mutant_part, mutant_shadow, False)
