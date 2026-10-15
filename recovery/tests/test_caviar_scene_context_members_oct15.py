from pathlib import Path
import shutil
import subprocess
import tempfile

from match_recovery.layout_source import parse_members


ROOT = Path(__file__).resolve().parents[2]
HEADER = ROOT / "src/caviar.h"
SOURCE = ROOT / "src/caviar.cpp"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
MEMBERS = {
    "shadow_origin_adjustment_": 0x60,
    "projection_offset_x_": 0x64,
    "projection_offset_y_": 0x68,
    "projection_offset_z_": 0x6C,
    "screen_offset_x_": 0x70,
    "screen_offset_y_": 0x74,
    "projection_scale_": 0x78,
    "shadow_screen_offset_x_": 0x7C,
    "shadow_screen_offset_y_": 0x80,
    "shadow_scale_x_": 0x84,
    "shadow_scale_y_": 0x88,
    "shadow_enabled_": 0x8C,
}


def body(source, signature):
    begin = source.index(signature)
    opening = source.index("{", begin)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[begin:index + 1]
    raise AssertionError("unbalanced source function: " + signature)


def statement(source, start):
    first = source.index(start)
    return source[first:source.index(";", first) + 1] + "\n"


def between(source, start, end):
    first = source.index(start)
    return source[first:source.index(end, first)]


def test_member_expectations_use_context_relative_offsets_and_pinned_access_anchors():
    text = HEADER.read_text()
    rows = parse_members(text, "src/caviar.h", PIN, address_space="pe:va")
    actual = {row.member: row for row in rows
              if row.class_name == "CaviarSceneContext"}
    assert set(actual) == set(MEMBERS) | {"light_vector_", "shadow_vector_"}
    for name, offset in MEMBERS.items():
        row = actual[name]
        assert row.offset == offset
        assert row.evidence.address == 0x006396A0
    assert actual["shadow_enabled_"].offset == 0x8C
    context = body(text, "struct CaviarSceneContext {")
    assert "Caviar+0x18" in context
    assert "Caviar+0x" not in "\n".join(
        line for line in context.splitlines() if line.lstrip().startswith(("float ", "uint8_t ", "VOX_Vect ")))
    assert "offsetof(CaviarSceneContext, shadow_enabled_) == 0x8C" in text


FIXTURE = r'''#include <assert.h>
#include <stddef.h>
#include <stdint.h>
struct VOX_Vect { float values[3]; };
struct VOX_Matrix { float values[3][3]; };
struct VoxSceneState { float scale_; };
struct CaviarSceneContext {
  VoxSceneState state_;
  VOX_Vect light_vector_, shadow_vector_;
  float shadow_origin_adjustment_;
  float projection_offset_x_, projection_offset_y_, projection_offset_z_;
  float screen_offset_x_, screen_offset_y_, projection_scale_;
  float shadow_screen_offset_x_, shadow_screen_offset_y_;
  float shadow_scale_x_, shadow_scale_y_;
  uint8_t shadow_enabled_;
};
struct CaviarCamera { float scaling; };
struct CaviarChannelBounds { float lead_[5]; };
struct VoxCameraBlock { CaviarCamera camera_; CaviarChannelBounds bounds_; };
struct Scratch {
  float perspective_factor_, projected_scale_, viewport_aspect_;
  VOX_Vect camera_offset_, shadow_centre_;
  VOX_Matrix shadow_matrix_;
};
'''


def compile_and_run(snippet):
    compiler = shutil.which("g++") or shutil.which("clang++")
    assert compiler, "GCC or Clang is required for actual-source controls"
    harness = FIXTURE + r'''
int main() {
  CaviarSceneContext context_storage = {};
  VoxCameraBlock camera_storage = {};
  CaviarSceneContext *context = &context_storage;
  VoxCameraBlock *camera = &camera_storage;
  Scratch scratch = {};
  float half_width = 10.0f, half_height = 20.0f;
  VOX_Matrix shadow_axes = {};
  for (int axis = 0; axis != 3; ++axis) {
    shadow_axes.values[axis][0] = float(axis + 1);
    shadow_axes.values[axis][1] = float(axis + 4);
  }
  context->state_.scale_ = 2.0f;
  camera->camera_.scaling = 2.5f;
  camera->bounds_.lead_[1] = 0.0f;
  camera->bounds_.lead_[2] = 2.0f;
  camera->bounds_.lead_[3] = 1.0f;
  camera->bounds_.lead_[4] = 3.0f;
  scratch.perspective_factor_ = 0.5f;
  context->projection_scale_ = 3.0f;
  context->projection_offset_x_ = 2.0f;
  context->projection_offset_y_ = 4.0f;
  context->screen_offset_x_ = 5.0f;
  context->screen_offset_y_ = 6.0f;
  scratch.camera_offset_.values[0] = 1.25f;
  scratch.camera_offset_.values[1] = -2.0f;
  context->shadow_scale_x_ = 3.0f;
  context->shadow_scale_y_ = 2.0f;
  context->shadow_screen_offset_x_ = 11.0f;
  context->shadow_screen_offset_y_ = 13.0f;
  scratch.shadow_centre_.values[0] = 0.5f;
  scratch.shadow_centre_.values[1] = -1.0f;
/* ACTUAL_PROJECTION */
/* ACTUAL_SHADOW */
  assert(scratch.projected_scale_ == 3.75f);
  assert(scratch.camera_offset_.values[0] == 25.6875f);
  assert(scratch.camera_offset_.values[1] == 30.5f);
  assert(scratch.shadow_centre_.values[0] == 30.75f);
  assert(scratch.shadow_centre_.values[1] == 36.0f);
  return 0;
}
'''
    with tempfile.TemporaryDirectory() as directory:
        cpp = Path(directory) / "context.cpp"
        exe = Path(directory) / "context"
        cpp.write_text(harness.replace("/* ACTUAL_PROJECTION */", snippet[0])
                       .replace("/* ACTUAL_SHADOW */", snippet[1]))
        compile_result = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", str(cpp), "-o", str(exe)],
            capture_output=True, text=True, timeout=60)
        assert compile_result.returncode == 0, compile_result.stdout + compile_result.stderr
        runtime = subprocess.run([str(exe)], capture_output=True, text=True, timeout=30)
        return runtime.returncode, runtime.stdout + runtime.stderr


def actual_equations():
    text = SOURCE.read_text()
    stage = body(text, "float __cdecl vox_stage_scene(")
    projection = "".join(statement(stage, token) for token in (
        "scratch.projected_scale_ = camera->camera_.scaling *",
        "scratch.camera_offset_.values[0] =",
        "scratch.camera_offset_.values[1] =",
    ))
    shadow = "".join(statement(stage, token) for token in (
        "const float shadow_projected_scale = context->state_.scale_ *",
        "const float shadow_x_scale = shadow_projected_scale * context->shadow_scale_x_",
        "const float shadow_y_scale = shadow_projected_scale * context->shadow_scale_y_",
        "scratch.shadow_centre_.values[0] =",
        "scratch.shadow_centre_.values[1] =",
    ))
    return projection, shadow


def test_actual_projection_and_shadow_equations_consume_named_context_fields():
    projection, shadow = actual_equations()
    assert "context->projection_scale_" in projection
    assert "context->projection_offset_x_" in projection
    assert "context->screen_offset_y_" in projection
    assert "context->shadow_scale_x_" in shadow
    assert "context->shadow_screen_offset_y_" in shadow
    status, output = compile_and_run((projection, shadow))
    assert status == 0, output


def test_wrong_projection_member_mutation_is_compilable_but_fails_runtime_control():
    projection, shadow = actual_equations()
    damaged_projection = projection.replace(
        "context->projection_offset_x_", "context->projection_offset_y_", 1)
    assert damaged_projection != projection
    status, _ = compile_and_run((damaged_projection, shadow))
    assert status != 0


def test_actual_render_scene_producer_keeps_projection_zeroing_and_flag_mapping():
    render = body(SOURCE.read_text(), "int Caviar::render_scene(Buffer *buffer, int flags)")
    producer = between(
        render,
        "scene_context_.projection_offset_x_ = 0;",
        "CaviarScene.renderer_ = this;",
    )
    harness = FIXTURE + r'''
void render_scene_producer(CaviarSceneContext &scene_context_, int flags) {
/* ACTUAL_PRODUCER */
}
int main() {
  CaviarSceneContext context = {};
  context.projection_offset_x_ = 17.0f;
  context.projection_offset_y_ = 19.0f;
  context.projection_offset_z_ = 23.0f;
  render_scene_producer(context, 2);
  assert(context.projection_offset_x_ == 0.0f);
  assert(context.projection_offset_y_ == 0.0f);
  assert(context.projection_offset_z_ == 0.0f);
  assert(context.shadow_enabled_ == 0);
  render_scene_producer(context, 0);
  assert(context.shadow_enabled_ == 1);
  return 0;
}
'''.replace("/* ACTUAL_PRODUCER */", producer)
    compiler = shutil.which("g++") or shutil.which("clang++")
    assert compiler, "GCC or Clang is required for actual-source controls"
    with tempfile.TemporaryDirectory() as directory:
        cpp = Path(directory) / "producer.cpp"
        exe = Path(directory) / "producer"
        cpp.write_text(harness)
        result = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", str(cpp), "-o", str(exe)],
            capture_output=True, text=True, timeout=60)
        assert result.returncode == 0, result.stdout + result.stderr
        assert subprocess.run([str(exe)], timeout=30).returncode == 0

