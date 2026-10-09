"""Host controls for Caviar's source-owned object callback and record slots."""
from pathlib import Path
import shutil
import subprocess
import tempfile


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/caviar.cpp").is_file())
SOURCE = ROOT / "src/caviar.cpp"
HEADER = ROOT / "src/caviar.h"


def extract(text: str, signature: str) -> str:
    start = text.find(signature)
    while start >= 0:
        opening = text.find("{", start)
        semicolon = text.find(";", start)
        if opening >= 0 and (semicolon < 0 or opening < semicolon):
            break
        start = text.find(signature, start + len(signature))
    assert start >= 0, "missing current owner definition: " + signature
    depth, state, index = 0, "code", opening
    while index < len(text):
        current = text[index]
        following = text[index + 1] if index + 1 < len(text) else ""
        if state == "code":
            if current == '"':
                state = "string"
            elif current == "'":
                state = "char"
            elif current == "/" and following == "/":
                state = "line"
                index += 1
            elif current == "/" and following == "*":
                state = "block"
                index += 1
            elif current == "{":
                depth += 1
            elif current == "}":
                depth -= 1
                if depth == 0:
                    return text[start:index + 1]
        elif state in ("string", "char"):
            if current == "\\":
                index += 1
            elif (state == "string" and current == '"') or (
                    state == "char" and current == "'"):
                state = "code"
        elif state == "line" and current == "\n":
            state = "code"
        elif state == "block" and current == "*" and following == "/":
            state = "code"
            index += 1
        index += 1
    raise AssertionError("unclosed current owner definition: " + signature)


def test_actual_source_callback_identity_and_post_part_edge():
    source = SOURCE.read_text()
    header = HEADER.read_text()
    initializer = source[source.index("int Caviar::init_class()"):
                         source.index("int Caviar::init_class()") + 8000]
    draw = extract(source, "void __cdecl vox_draw_scene(")

    # Preserve the original identity while the coordinator later updates the
    # current emitted symbol through the native annotation writer.
    assert ("// ORIGINAL: 0x00618DA0 ?object_start@Caviar@@QAAX"
            "PAUVOX_ScnRenderHnd@@JPAUVOX_Vect@@PAUVOX_Matrix@@"
            "PAUVOX_Rect@@PAUVOX_Vect@@@Z 0x00618DA0-0x00618E06 SEMANTIC") in source
    assert "static void __cdecl object_start(VOX_ScnRenderHnd *handle" in header
    assert "record->object_start_callback_ = &Caviar::object_start;" in initializer
    assert "record->object_callback_[0] = 0x00618DA0" not in initializer
    assert "const void *render_argument_;" in source
    assert "const void *colour_table," in source and "const void *colour_table," in header
    assert "CaviarScene.render_argument_ = colour_table;" in source
    assert "memcpy(handle->colour_table_, CaviarScene.render_argument_, 0x1800);" in source
    assert "(void *)CaviarScene.render_argument_" not in source
    assert "// ORIGINAL: 0x00617670 ?UNK2@Caviar@@QAEHPAUBuffer@@HHHH@Z " in source
    assert "// CLAIMED BYTE_EXACT, homed from src/recovered/00617670.cpp" in source

    assert "typedef void (__cdecl *ObjectStartCallback)(VoxRenderRecord *, long," in header
    assert "VOX_Vect *, VOX_Matrix *, int32_t *, VOX_Vect *);" in header
    assert "int32_t *bounds" in header and "int32_t *bounds" in source
    assert "typedef void (__cdecl *PostPartCallback)(VoxRenderRecord *, int32_t);" in header
    assert "ObjectStartCallback object_start_callback_; // +0x58" in header
    assert "PostPartCallback post_part_callback_;       // +0x5C" in header
    assert "typedef VoxRenderRecord VOX_ScnRenderHnd;" in header
    assert "uint32_t object_callback_[2]" not in header

    first_call = draw.index("callback(record, index, &position, &part,")
    part_draw = draw.index("vox_draw_part(record, node, &position, &part,")
    post_guard = draw.index("record->post_part_callback_", part_draw)
    post_call = draw.index("callback(record, index);", post_guard)
    shadow_pass = draw.index("// The shadow pass", post_call)
    assert first_call < part_draw < post_guard < post_call < shadow_pass
    assert "if (VoxRenderRecord::ObjectStartCallback callback =" in draw
    assert ("callback(record, index, &position, &part,\n"
            "                     view->bounds_.values_, &context->light_vector_);") in draw
    assert "state_bytes" not in draw
    assert "context->field_A4_" in draw
    assert "&context->light_vector_" in draw
    assert "if (VoxRenderRecord::PostPartCallback callback =" in draw
    # The post-part slot is read at the call site after drawing, rather than
    # cached before the draw can mutate it.
    assert "post_part =" not in draw[:part_draw]


def test_actual_object_start_body_copies_all_three_palette_sources():
    source = SOURCE.read_text()
    body = extract(source, "void __cdecl Caviar::object_start(")
    preamble = r'''#include <assert.h>
#include <stdint.h>
#include <string.h>
#ifndef __cdecl
#define __cdecl
#endif
struct VOX_Vect { float values[3]; };
struct VOX_Matrix { float values[3][3]; };
struct VoxRenderRecord { void *colour_table_; };
typedef VoxRenderRecord VOX_ScnRenderHnd;
struct Caviar {
  uint32_t field_4_;
  uint8_t *instance_palettes_[8];
  static void __cdecl object_start(VOX_ScnRenderHnd *, long, VOX_Vect *,
      VOX_Matrix *, int32_t *, VOX_Vect *);
};
struct CaviarSceneFixture {
  Caviar *renderer_;
  const void *render_argument_;
};
static CaviarSceneFixture CaviarScene = {};
static uint8_t CaviarColourSource[0x1800];

/* ACTUAL_PRODUCT_BODY */

int main() {
  Caviar renderer = {};
  CaviarScene.renderer_ = &renderer;
  uint8_t output[0x1800] = {};
  uint8_t argument_table[0x1800];
  uint8_t instance_table[0x1800];
  VoxRenderRecord record = {output};
  VOX_Vect vector = {};
  VOX_Matrix matrix = {};

  for (int i = 0; i != 0x1800; ++i) {
    CaviarColourSource[i] = (uint8_t)(i * 3 + 1);
    argument_table[i] = (uint8_t)(i * 5 + 2);
    instance_table[i] = (uint8_t)(i * 7 + 3);
  }
  typedef void (__cdecl *Callback)(VOX_ScnRenderHnd *, long, VOX_Vect *,
      VOX_Matrix *, int32_t *, VOX_Vect *);
  Callback callback = &Caviar::object_start;

  renderer.field_4_ = 8;
  callback(&record, 3, &vector, &matrix, 0, &vector);
  assert(memcmp(output, CaviarColourSource, sizeof(output)) == 0);

  memset(output, 0, sizeof(output));
  CaviarScene.render_argument_ = argument_table;
  renderer.field_4_ = 0x10;
  callback(&record, 3, &vector, &matrix, 0, &vector);
  assert(memcmp(output, argument_table, sizeof(output)) == 0);

  memset(output, 0, sizeof(output));
  CaviarScene.render_argument_ = argument_table;
  renderer.instance_palettes_[3] = instance_table;
  renderer.field_4_ = 0;
  callback(&record, 3, &vector, &matrix, 0, &vector);
  assert(memcmp(output, instance_table, sizeof(output)) == 0);

  memset(output, 0, sizeof(output));
  renderer.field_4_ = 0x18;
  callback(&record, 3, &vector, &matrix, 0, &vector);
  assert(memcmp(output, CaviarColourSource, sizeof(output)) == 0);
  return 0;
}
'''
    translation_unit = preamble.replace("/* ACTUAL_PRODUCT_BODY */", body)
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    available = [compiler for compiler in compilers if compiler]
    assert available, "GCC or Clang is required for extracted product-source controls"
    with tempfile.TemporaryDirectory(prefix="caviar-object-callback-") as directory:
        directory = Path(directory)
        source_file = directory / "actual_callback.cpp"
        source_file.write_text(translation_unit)
        for compiler in available:
            executable = directory / Path(compiler).name.replace("+", "p")
            subprocess.run(
                [compiler, "-std=c++98", "-Wall", "-Wextra", str(source_file),
                 "-o", str(executable)],
                check=True, timeout=60,
            )
            subprocess.run([str(executable)], check=True, timeout=30)


def test_actual_draw_edge_passes_the_six_int_window_and_rereads_post_callback():
    source = SOURCE.read_text()
    draw = extract(source, "void __cdecl vox_draw_scene(")
    start = draw.index("        if (VoxRenderRecord::ObjectStartCallback callback =")
    end = draw.index("        // The shadow pass", start)
    actual_edge = draw[start:end]
    preamble = r'''#include <assert.h>
#include <stdint.h>
#ifndef __cdecl
#define __cdecl
#endif
struct VOX_Vect { float values[3]; };
struct VOX_Matrix { float values[3][3]; };
struct CaviarChannelBounds { float lead_[5]; int32_t values_[6]; };
struct CaviarCamera { VOX_Matrix rotation; };
struct VoxCameraBlock { CaviarCamera camera_; CaviarChannelBounds bounds_; };
struct CaviarSceneContext { VOX_Vect light_vector_; };
struct CvrShape {};
struct VoxRenderRecord {
  typedef void (__cdecl *ObjectStartCallback)(VoxRenderRecord *, long,
      VOX_Vect *, VOX_Matrix *, int32_t *, VOX_Vect *);
  typedef void (__cdecl *PostPartCallback)(VoxRenderRecord *, int32_t);
  ObjectStartCallback object_start_callback_;
  PostPartCallback post_part_callback_;
};
static int events[4];
static int32_t *seen_bounds;
static VOX_Vect *seen_direction;
static int32_t seen_index;
static int old_post_calls;
static bool replace_after_draw;
static void __cdecl post_after_draw(VoxRenderRecord *, int32_t);
static void __cdecl object_start(VoxRenderRecord *record, long index,
    VOX_Vect *, VOX_Matrix *, int32_t *bounds, VOX_Vect *direction) {
  assert(record != 0);
  events[0] = 1;
  seen_bounds = bounds;
  seen_direction = direction;
  seen_index = (int32_t)index;
}
static void __cdecl post_before_draw(VoxRenderRecord *, int32_t) {
  ++old_post_calls;
  events[2] = 4;
}
static void __cdecl post_after_draw(VoxRenderRecord *, int32_t index) {
  assert(index == seen_index);
  events[2] = 3;
}
static void vox_draw_part(VoxRenderRecord *record, const CvrShape *,
    VOX_Vect *, VOX_Matrix *, const VOX_Matrix *, const VOX_Vect *, float, int) {
  events[1] = 2;
  if (replace_after_draw)
    record->post_part_callback_ = &post_after_draw;
}
static void dispatch_edge(VoxRenderRecord *record, CaviarSceneContext *context,
    VoxCameraBlock *view, const CvrShape *node, int index, VOX_Vect &position,
    VOX_Matrix &part, float extent, int visible) {
/* ACTUAL_DRAW_EDGE */
}
int main() {
  VoxRenderRecord record = {};
  CaviarSceneContext context = {};
  VoxCameraBlock view = {};
  CvrShape shape;
  VOX_Vect position = {};
  VOX_Matrix part = {};
  view.bounds_.values_[0] = 41;
  context.light_vector_.values[0] = 73.0f;
  record.object_start_callback_ = &object_start;
  record.post_part_callback_ = &post_before_draw;
  replace_after_draw = true;
  dispatch_edge(&record, &context, &view, &shape, 6, position, part, 1.0f, 1);
  assert(events[0] == 1 && events[1] == 2 && events[2] == 3);
  assert(seen_bounds == view.bounds_.values_);
  assert(seen_bounds[0] == 41);
  assert(seen_direction == &context.light_vector_);
  assert(seen_direction->values[0] == 73.0f);
  assert(seen_index == 6 && old_post_calls == 0);

  events[0] = events[1] = events[2] = 0;
  record.object_start_callback_ = 0;
  record.post_part_callback_ = 0;
  replace_after_draw = false;
  dispatch_edge(&record, &context, &view, &shape, 2, position, part, 1.0f, 1);
  assert(events[0] == 0 && events[1] == 2 && events[2] == 0);
  return 0;
}
'''
    translation_unit = preamble.replace("/* ACTUAL_DRAW_EDGE */", actual_edge)
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    available = [compiler for compiler in compilers if compiler]
    assert available, "GCC or Clang is required for extracted product-source controls"
    with tempfile.TemporaryDirectory(prefix="caviar-draw-edge-") as directory:
        directory = Path(directory)
        source_file = directory / "actual_draw_edge.cpp"
        source_file.write_text(translation_unit)
        for compiler in available:
            executable = directory / Path(compiler).name.replace("+", "p")
            subprocess.run(
                [compiler, "-std=c++98", "-Wall", "-Wextra", str(source_file),
                 "-o", str(executable)],
                check=True, timeout=60,
            )
            subprocess.run([str(executable)], check=True, timeout=30)
