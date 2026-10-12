"""Actual renderer member/consumer controls, not original full-game proof."""
from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest

from test_caviar_descriptor_ramps_overlay_oct9 import extract
from test_caviar_rect_state_actual_source_oct9 import source_statement
from test_caviar_shape_chain_oct11 import assert_caviar_owner_continuity
from match_recovery.source import read_text

ROOT = Path(__file__).resolve().parents[2]
BASE = "d75dfb27268cf7fed8798eaa77399fce39a0dea2"


def test_original_identities_claims_symbols_and_history_are_preserved():
    path = ROOT / "src/caviar.cpp"
    old = subprocess.check_output(["git", "show", BASE + ":src/caviar.cpp"],
                                  cwd=ROOT, text=True)
    assert_caviar_owner_continuity(read_text(old, path))


def program():
    header = (ROOT / "src/caviar.h").read_text()
    source = (ROOT / "src/caviar.cpp").read_text()
    declarations = (extract(header, "struct VoxRenderClipBounds {") + ";\n" +
                    extract(header, "struct VoxRenderRecord {") + ";\n")
    initializer = extract(source, "int Caviar::init_class()")
    assert "zero_b_[" not in initializer
    stores = "\n".join(source_statement(initializer, "record->" + field + " =")
                       for field in ("draw_left_", "draw_top_", "draw_right_",
                                     "draw_bottom_", "draw_width_", "draw_height_"))
    assert initializer.index("record->draw_left_ =") < initializer.index("record->draw_top_ =")
    consumer = extract(source, "uint32_t *Caviar::clip_rect()")
    portions = []
    for name, signature, local in (
        ("scene_bounds", "int Caviar::render_scene(", "drawn"),
        ("primary_bounds", "int Caviar::render(", "covered"),
        ("secondary_bounds", "int Caviar::render(", "scene"),
    ):
        body = extract(source, signature)
        statements = "\n".join(source_statement(body, local + "." + field + " =")
                               for field in ("left", "top", "right", "bottom"))
        portions.append("static RECT " + name + "(VoxRenderRecord *record) { RECT " +
                        local + ";\n" + statements + "\nreturn " + local + "; }\n")
    fill = extract(source, "void Caviar::fill_clip_rect(")
    assert "if (out != 0)" in fill and "if (CaviarScene.created_ != 0)" in fill
    assert "VoxRenderRecord *record" not in fill  # alias-sensitive fresh loads
    portions.append(fill)
    return r'''#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#define __cdecl
struct VOX_Vect;
struct VOX_Matrix;
struct CaviarSceneSetupDescriptorCandidate;
''' + declarations + r'''
struct { VoxRenderRecord *created_; } CaviarScene;
static uint32_t vox_clip_rect[4];
struct Caviar {
    uint32_t *clip_rect();
    void fill_clip_rect(uint32_t *destination);
};
struct RECT { int32_t left, top, right, bottom; };
''' + consumer + r'''
static void initialize(VoxRenderRecord *record) {
''' + stores + r'''
}
''' + "\n".join(portions) + r'''
int main() {
    // Host pointer widths differ; these relational offsets test nesting only.
    assert(offsetof(VoxRenderRecord, secondary_clip_) ==
           offsetof(VoxRenderRecord, primary_clip_) + 24);
    assert(offsetof(VoxRenderRecord, draw_left_) ==
           offsetof(VoxRenderRecord, secondary_clip_) + 24);
    assert(sizeof(VoxRenderClipBounds) == 24);
    VoxRenderRecord record;
    memset(&record, 0xA5, sizeof(record));
    initialize(&record);
    assert(record.draw_left_ == 0 && record.draw_top_ == 0);
    assert(record.draw_right_ == 255 && record.draw_bottom_ == 255);
    assert(record.draw_width_ == 256 && record.draw_height_ == 256);
    assert(record.secondary_clip_.width_ == 0xA5A5A5A5U);
    assert(record.secondary_clip_.height_ == 0xA5A5A5A5U);
    Caviar owner;
    CaviarScene.created_ = 0;
    assert(owner.clip_rect() == 0);
    uint32_t unchanged[4] = {11,22,33,44};
    owner.fill_clip_rect(unchanged);
    assert(unchanged[0] == 11 && unchanged[1] == 22 &&
           unchanged[2] == 33 && unchanged[3] == 44);
    owner.fill_clip_rect(0);
    CaviarScene.created_ = &record;
    record.primary_clip_.left_ = 0xFFFFFFF4U;
    record.primary_clip_.top_ = 0xFFFFFFF9U;
    record.primary_clip_.right_ = 0xFFFFFFFFU;
    record.primary_clip_.bottom_ = 0x7FFFFFFFU;
    record.secondary_clip_.left_ = 77;
    assert(owner.clip_rect() == vox_clip_rect);
    assert(vox_clip_rect[0] == 0xFFFFFFF4U && vox_clip_rect[1] == 0xFFFFFFF9U);
    assert(vox_clip_rect[2] == 0);
    assert(static_cast<uint32_t>(vox_clip_rect[3]) == 0x80000000U);
    RECT scene = scene_bounds(&record);
    RECT primary = primary_bounds(&record);
    assert(scene.left == -12 && scene.top == -7 && scene.right == 0);
    assert(static_cast<uint32_t>(scene.bottom) == 0x80000000U);
    assert(primary.left == scene.left && primary.top == scene.top &&
           primary.right == scene.right && primary.bottom == scene.bottom);
    uint32_t out[4] = {0};
    owner.fill_clip_rect(out);
    owner.fill_clip_rect(0);
    assert(out[0] == 0xFFFFFFF4U && out[1] == 0xFFFFFFF9U && out[2] == 0);
    assert(static_cast<uint32_t>(out[3]) == 0x80000000U);
    record.secondary_clip_.top_ = 88;
    record.secondary_clip_.right_ = record.secondary_clip_.bottom_ = 0xFFFFFFFFU;
    RECT secondary = secondary_bounds(&record);
    assert(secondary.left == 77 && secondary.top == 88 &&
           secondary.right == 0 && secondary.bottom == 0);
}
'''


def run(text, succeeds):
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    assert any(compilers), "an actual-source host compiler is required"
    with tempfile.TemporaryDirectory(prefix="caviar-clip-members-") as directory:
        cpp = Path(directory) / "control.cpp"
        cpp.write_text(text)
        for compiler in filter(None, compilers):
            exe = Path(directory) / Path(compiler).name.replace("+", "p")
            subprocess.run([compiler, "-std=c++98", "-O2", "-fsanitize=undefined",
                            "-fno-sanitize-recover=undefined", str(cpp), "-o", str(exe)],
                           check=True, capture_output=True, timeout=60)
            result = subprocess.run([str(exe)], capture_output=True, timeout=30)
            assert (result.returncode == 0) == succeeds, result.stderr.decode()


def test_actual_members_initialization_and_unsigned_inclusive_edges():
    run(program(), True)


@pytest.mark.parametrize("old,new", (
    ("record->draw_top_ = 0;", "record->draw_top_ = 1;"),
    ("record->primary_clip_.right_ + 1", "record->primary_clip_.right_"),
    ("record->primary_clip_.left_", "record->secondary_clip_.left_"),
    ("uint32_t left_, top_, right_, bottom_, width_, height_;",
     "int32_t left_, top_, right_, bottom_, width_, height_;"),
))
def test_initialization_wrong_window_edge_and_signed_overflow_damage(old, new):
    text = program()
    assert old in text
    run(text.replace(old, new, 1), False)


@pytest.mark.parametrize("old,new", (
    ("drawn.right = CaviarScene.created_->primary_clip_.right_ + 1;",
     "drawn.right = CaviarScene.created_->primary_clip_.right_;"),
    ("covered.top = record->primary_clip_.top_;",
     "covered.top = record->secondary_clip_.top_;"),
    ("scene.right = record->secondary_clip_.right_ + 1;",
     "scene.right = record->secondary_clip_.right_;"),
    ("out[2] = CaviarScene.created_->primary_clip_.right_ + 1;",
     "out[2] = CaviarScene.created_->secondary_clip_.right_ + 1;"),
))
def test_each_remaining_consumer_expression_is_protected(old, new):
    text = program()
    assert old in text
    run(text.replace(old, new, 1), False)
