"""Actual-source controls for BasePop modal callback dispatch.

The compiled fixture inserts the complete live C++ function bodies. It checks
ordinary host C++ receiver selection, callback ordering, and branch effects; it
does not model the VC6 ABI or establish game behavior.
"""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/basepop.cpp"
HEADER = ROOT / "src/basepop.h"


def function_body(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 1
    for pos in range(brace + 1, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[brace + 1 : pos]
    raise AssertionError("unterminated function body: " + signature)


def live_bodies():
    source = SOURCE.read_text(encoding="utf-8")
    clicked = function_body(source, "void BasePop::on_button_clicked(int button_id)")
    double_click = function_body(
        source, "void __cdecl BasePop::double_click(int button_id)"
    )
    return clicked, double_click


FIXTURE = r'''#include <assert.h>
#include <stdint.h>
#include <string>
#include <vector>
#define __cdecl

struct BasePop;
static std::vector<std::string> events;

struct Win {
  const char *name;
  BasePop *mutation_target;
  uint32_t mutation_flags;
  Win(const char *n) : name(n), mutation_target(0), mutation_flags(0) {}
  virtual ~Win() {}
  virtual void release_modal();
  virtual void hide() { events.push_back(std::string(name) + ":base-hide"); }
};

struct GraphicWin : Win {
  explicit GraphicWin(const char *n) : Win(n) {}
};

struct Dialog {
  struct ItemList { int entry_count_; } item_list_;
  int selected_position_;
  int selected_id_;
  Dialog() : selected_position_(0), selected_id_(0) { item_list_.entry_count_ = 0; }
  int get_selected_id() { return selected_id_; }
};

struct Dialogs : Dialog {
  int kind_;
  Dialogs() : kind_(-1) {}
  int get_num_items() { return item_list_.entry_count_; }
};

struct BasePop : GraphicWin {
  GraphicWin *external_window_;
  Dialogs dialogs_;
  int selected_id_;
  int field_3100_;
  uint32_t field_30A8_;
  explicit BasePop(const char *n)
      : GraphicWin(n), external_window_(0), selected_id_(0), field_3100_(0),
        field_30A8_(0) {}
  virtual void on_button_clicked(int button_id);
  static void __cdecl double_click(int button_id);
};

static Win *WinModalFocus;

void Win::release_modal() {
  events.push_back(std::string(name) + ":release");
  if (mutation_target != 0) {
    mutation_target->field_30A8_ = mutation_flags;
  }
}

struct Popup : BasePop {
  explicit Popup(const char *n) : BasePop(n) {}
  virtual void hide() { events.push_back(std::string(name) + ":popup-hide"); }
};

struct ExternalWindow : GraphicWin {
  explicit ExternalWindow(const char *n) : GraphicWin(n) {}
  virtual void release_modal() {
    events.push_back(std::string(name) + ":external-release");
    if (mutation_target != 0) {
      mutation_target->field_30A8_ = mutation_flags;
    }
  }
};

void BasePop::on_button_clicked(int button_id) {
''' + "__CLICKED_BODY__" + r'''
}

void __cdecl BasePop::double_click(int button_id) {
''' + "__DOUBLE_CLICK_BODY__" + r'''
}

static void clear_events() { events.clear(); }
static void expect_events(const char *first, const char *second = 0) {
  assert(events.size() == (second == 0 ? 1U : 2U));
  assert(events[0] == first);
  if (second != 0) assert(events[1] == second);
}

int main() {
  Popup popup("popup");
  ExternalWindow external("external");
  WinModalFocus = &popup;
  popup.dialogs_.kind_ = 2;
  popup.dialogs_.item_list_.entry_count_ = 1;
  popup.dialogs_.selected_id_ = 41;
  popup.dialogs_.selected_position_ = 12;

  // -2 preserves the selected item unless lock bit 0x40 forces -1.
  // The external release mutates popup flags; on_button_clicked reads the
  // hide flag after the callback, so release must precede virtual Popup::hide.
  popup.external_window_ = &external;
  popup.field_30A8_ = 0;
  external.mutation_target = &popup;
  external.mutation_flags = 0x4000;
  clear_events();
  popup.on_button_clicked(-2);
  assert(popup.selected_id_ == 41 && popup.field_3100_ == 0);
  expect_events("external:external-release", "popup:popup-hide");

  popup.field_30A8_ = 0x40 | 0x100 | 0x4000;
  external.mutation_target = 0;
  clear_events();
  popup.on_button_clicked(-2);
  assert(popup.selected_id_ == -1 && popup.field_3100_ == 0);
  expect_events("popup:popup-hide");

  // -1 clears the poll state; a positive ID is retained. With no external
  // receiver, release_modal dispatches through the popup's Win base.
  popup.external_window_ = 0;
  popup.field_30A8_ = 0;
  popup.mutation_target = &popup;
  popup.mutation_flags = 0x4000;
  clear_events();
  popup.on_button_clicked(23);
  assert(popup.selected_id_ == 41 && popup.field_3100_ == 23);
  expect_events("popup:release", "popup:popup-hide");

  popup.external_window_ = &external;
  popup.field_30A8_ = 0x4000;
  external.mutation_target = &popup;
  external.mutation_flags = 0;
  clear_events();
  popup.on_button_clicked(-1);
  assert(popup.field_3100_ == 0);
  expect_events("external:external-release");

  // Double-click tab -1 skips only the selection inner block. It still clears
  // field_3100_, releases the external modal receiver, and hides using the
  // flags cached before release (even though release clears the live flag).
  popup.dialogs_.kind_ = -1;
  popup.selected_id_ = 77;
  popup.field_3100_ = 99;
  popup.field_30A8_ = 0x4000;
  external.mutation_target = &popup;
  external.mutation_flags = 0;
  clear_events();
  BasePop::double_click(0);
  assert(popup.selected_id_ == 77 && popup.field_3100_ == 0);
  assert(popup.field_30A8_ == 0);
  expect_events("external:external-release", "popup:popup-hide");

  // Tab 1 skips the entire update/release/hide tail.
  popup.dialogs_.kind_ = 1;
  popup.selected_id_ = 77;
  popup.field_3100_ = 99;
  popup.field_30A8_ = 0;
  clear_events();
  BasePop::double_click(0);
  assert(popup.selected_id_ == 77 && popup.field_3100_ == 99);
  assert(events.empty());

  // 0x100 suppresses release, independently of 0x4000 hide.
  popup.dialogs_.kind_ = 2;
  popup.dialogs_.item_list_.entry_count_ = 1;
  popup.selected_id_ = 77;
  popup.field_30A8_ = 0x100 | 0x4000;
  external.mutation_target = 0;
  clear_events();
  BasePop::double_click(0);
  assert(popup.selected_id_ == 41 && popup.field_3100_ == 0);
  expect_events("popup:popup-hide");

  // Empty list leaves selected_id_ untouched, while the own receiver is used.
  popup.dialogs_.kind_ = 2;
  popup.dialogs_.item_list_.entry_count_ = 0;
  popup.selected_id_ = 66;
  popup.external_window_ = 0;
  popup.field_30A8_ = 0;
  clear_events();
  BasePop::double_click(0);
  assert(popup.selected_id_ == 66 && popup.field_3100_ == 0);
  expect_events("popup:release");

  WinModalFocus = 0;
  clear_events();
  BasePop::double_click(0);
  assert(events.empty());
  return 0;
}
'''


def fixture(clicked, double_click):
    return FIXTURE.replace("__CLICKED_BODY__", clicked).replace(
        "__DOUBLE_CLICK_BODY__", double_click
    )


def compilers():
    found = [name for name in ("g++", "clang++") if shutil.which(name)]
    assert found, "neither GCC nor Clang is available"
    return found


def compile_and_run(tmp_path, compiler, source, stem):
    binary = tmp_path / (Path(compiler).name.replace("+", "p") + "-" + stem)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         "-Wno-unused-parameter", "-o", str(binary), "-x", "c++", "-"],
        input=source,
        text=True,
        capture_output=True,
        timeout=60,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)


def test_live_source_has_typed_dispatch_and_graphic_window_storage():
    clicked, double_click = live_bodies()
    header = HEADER.read_text(encoding="utf-8")
    source = SOURCE.read_text(encoding="utf-8")
    diplomatic_popup = (ROOT / "src/diplopop.cpp").read_text(encoding="utf-8")
    assert "GraphicWin *external_window_;  // 0x0a14" in header
    assert "void set_external_window(GraphicWin *external_window" in header
    assert "static void set_def_external_window(GraphicWin *external_window" in header
    assert "external_window_ = external_window;" in source
    assert "BasePopDefaultLayout[0] = reinterpret_cast<uint32_t>(external_window);" in source
    assert "BasePop::set_def_external_window(this," in diplomatic_popup
    assert "external_window_ = reinterpret_cast<GraphicWin *>(BasePopDefaultLayout[0]);" in source
    assert "external_window_->release_modal();" in clicked
    assert "            release_modal();" in clicked
    assert "        hide();" in clicked
    assert "pop->external_window_->release_modal();" in double_click
    assert "pop->release_modal();" in double_click
    assert "pop->hide();" in double_click
    assert "vtable_slot<func_basepop_slot" not in clicked + double_click
    assert "func_basepop_slot058" not in source
    assert "func_basepop_slot090" not in source
    assert "func_basepop_slot002" not in source


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_bodies_preserve_modal_receivers_flags_and_order(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is unavailable")
    clicked, double_click = live_bodies()
    result = compile_and_run(tmp_path, compiler, fixture(clicked, double_click), "live")
    assert result.returncode == 0, result.stdout + result.stderr


def test_runtime_controls_reject_receiver_virtuality_order_and_guard_mutants(tmp_path):
    clicked, double_click = live_bodies()
    mutants = {
        "wrong-external-receiver": (
            clicked,
            double_click.replace(
                "pop->external_window_->release_modal();",
                "pop->release_modal();",
                1,
            ),
        ),
        "bypassed-virtual-hide": (
            clicked.replace("        hide();", "        Win::hide();", 1),
            double_click.replace("pop->hide();", "pop->Win::hide();", 1),
        ),
        "release-after-extra-hide": (
            clicked.replace(
                "            external_window_->release_modal();",
                "            hide();\n            external_window_->release_modal();",
                1,
            ),
            double_click,
        ),
        "inverted-0x100-guard": (
            clicked.replace("(field_30A8_ & 0x100) == 0", "(field_30A8_ & 0x100) != 0", 1),
            double_click,
        ),
    }
    for compiler in compilers():
        for name, bodies in mutants.items():
            result = compile_and_run(tmp_path, compiler, fixture(*bodies), name)
            assert result.returncode != 0, (compiler, name, result.stdout, result.stderr)

