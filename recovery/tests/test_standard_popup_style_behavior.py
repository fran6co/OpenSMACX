import pathlib
import shutil
import subprocess
import pytest


ROOT = pathlib.Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "game.cpp"


def extract_initializer(text):
    start = text.index("BoxSpriteParams InterfaceStandardPopupBoxStyle =")
    end = text.index(";", text.index("}", start)) + 1
    return text[start:end]


def test_popup_style_candidate_targets_existing_assets(tmp_path):
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    if not compilers:
        pytest.skip("neither g++ nor clang++ is installed")
    initializer = extract_initializer(SOURCE.read_text())
    harness = r'''
#include <assert.h>
struct Sprite { int id; };
struct Buffer { int id; };
struct BoxSpriteParams {
  Sprite *corner0_, *corner1_, *corner2_, *corner3_;
  Sprite *edge0_, *edge1_, *edge2_, *edge3_;
  Buffer *mask_; int color_;
};
static Sprite g_IFACE_STD_POPUPS_TOP_LEFT_SPRITE = {0};
static Sprite g_IFACE_STD_POPUPS_TOP_RIGHT_SPRITE = {1};
static Sprite g_IFACE_STD_POPUPS_BOT_LEFT_SPRITE = {2};
static Sprite g_IFACE_STD_POPUPS_BOT_RIGHT_SPRITE = {3};
static Sprite g_IFACE_STD_POPUPS_TOP_MID_SPRITE = {4};
static Sprite g_IFACE_STD_POPUPS_BOT_MID_SPRITE = {5};
static Sprite g_IFACE_STD_POPUPS_MID_LEFT_SPRITE = {6};
static Sprite g_IFACE_STD_POPUPS_MID_RIGHT_SPRITE = {7};
static Buffer g_IFACE_STD_POPUPS_MIDDLE_BUFFER = {8};
''' + initializer + r'''
int main() {
  assert(InterfaceStandardPopupBoxStyle.corner0_ == &g_IFACE_STD_POPUPS_TOP_LEFT_SPRITE);
  assert(InterfaceStandardPopupBoxStyle.corner1_ == &g_IFACE_STD_POPUPS_TOP_RIGHT_SPRITE);
  assert(InterfaceStandardPopupBoxStyle.corner2_ == &g_IFACE_STD_POPUPS_BOT_LEFT_SPRITE);
  assert(InterfaceStandardPopupBoxStyle.corner3_ == &g_IFACE_STD_POPUPS_BOT_RIGHT_SPRITE);
  assert(InterfaceStandardPopupBoxStyle.edge0_ == &g_IFACE_STD_POPUPS_TOP_MID_SPRITE);
  assert(InterfaceStandardPopupBoxStyle.edge1_ == &g_IFACE_STD_POPUPS_BOT_MID_SPRITE);
  assert(InterfaceStandardPopupBoxStyle.edge2_ == &g_IFACE_STD_POPUPS_MID_LEFT_SPRITE);
  assert(InterfaceStandardPopupBoxStyle.edge3_ == &g_IFACE_STD_POPUPS_MID_RIGHT_SPRITE);
  assert(InterfaceStandardPopupBoxStyle.mask_ == &g_IFACE_STD_POPUPS_MIDDLE_BUFFER);
  assert(InterfaceStandardPopupBoxStyle.color_ == 7);
}
'''
    source = tmp_path / "popup_style_control.cpp"
    source.write_text(harness)
    for compiler in compilers:
        binary = tmp_path / ("popup-style-" + pathlib.Path(compiler).name)
        subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                        str(source), "-o", str(binary)], check=True, timeout=30)
        subprocess.run([str(binary)], check=True, timeout=30)
