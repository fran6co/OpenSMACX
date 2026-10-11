"""Actual initializer definitions under host compilers; no game/runtime proof."""
from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest
from recovery.tests.test_pact_ascent import ROOT, definition

mutations = {
    "wrong_title_left": ("field_103EC_ + 9", "category_rect_.bottom + 9"),
    "wrong_special_top": ("topic_button_bounds_[17].top = side_width + 0x14e;", "topic_button_bounds_[17].top = 0x288;"),
    "wrong_matrix_row": ("camera_rotation.values[2][axis] = -camera_rotation.values[2][axis];", "camera_rotation.values[1][axis] = -camera_rotation.values[1][axis];"),
    "wrong_forward_source": ("caviar_.shadow_vector().values[0] = forward[0];", "caviar_.shadow_vector().values[0] = transformed_forward[0];"),
    "wrong_dialog_position": ("rect.left, rect.top,\n                  0x1000020", "rect.right, rect.bottom,\n                  0x1000020"),
    "wrong_blurb_font": ("set_font(&auxiliary_font8_, 0, 0, 0)", "set_font(&link_font_, 0, 0, 0)"),
    "wrong_concept_parent": ("static_cast<Win *>(this), 0, -1);\n    static_cast<Buffer &>(concept_box_)", "static_cast<Win *>(&MainInterfaceGlobal), 0, -1);\n    static_cast<Buffer &>(concept_box_)"),
    "wrong_detail_inset": ("lower_description_rect.left + 6", "lower_description_rect.left + 3"),
    "wrong_button_xy": ("row.left, row.top, row.right - row.left", "row.top, row.left, row.right - row.left"),
    "wrong_button_group_order": ("    buttonGroup_.init(0, 0);", ""),
    "wrong_list_state": ("listBox_.field_8_ = 1;", "listBox_.field_8_ = 0;"),
    "wrong_compact_threshold": ("WinScreenWidth == 800", "WinScreenWidth <= 800"),
}
@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_initializer_geometry_and_damage(tmp_path, compiler_name):
    body = definition((ROOT / "src/datalink.cpp").read_text(), r"void Datalink::init\(\)")
    fixture = Path(__file__).with_name("datalink_initializer_control.cpp").read_text()
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    with tempfile.TemporaryDirectory(prefix="osmx-init-control-") as directory:
        directory = Path(directory)
        for name in ("actual", *mutations):
            candidate = body
            if name != "actual":
                before, after = mutations[name]
                assert body.count(before) == 1, name
                candidate = body.replace(before, after, 1)
            translation = fixture.replace('// @@BODIES@@', candidate)
            source, binary = directory / (name + ".cpp"), directory / name
            source.write_text(translation)
            build = subprocess.run([compiler, "-std=c++98", "-fsanitize=undefined",
                "-fno-sanitize-recover=all", str(source), "-o", str(binary)],
                capture_output=True, text=True, timeout=45)
            assert build.returncode == 0, name + build.stderr
            run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
            assert (run.returncode == 0) == (name == "actual"), name + run.stderr
        print(compiler_name + ": actual passes all three layouts; 12 damaged variants refused", flush=True)
