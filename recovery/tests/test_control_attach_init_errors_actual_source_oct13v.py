"""Host GCC/Clang controls for the three actual attach definitions.

The fixture preserves the complete product method bodies and supplies only
the API calls those methods use. It checks source-level control flow and
argument/lifecycle effects; it says nothing about the target ABI.
"""
from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]


def extract_definition(source: str, signature: str) -> str:
    from match_recovery.includes import COMMENTS_OR_STRINGS

    code = COMMENTS_OR_STRINGS.sub(lambda m: re.sub(r"[^\n]", " ", m[0]), source)
    if code.count(signature) != 1:
        raise AssertionError(f"expected one product definition: {signature}")
    start = code.index(signature)
    opening = code.index("{", start)
    depth = 0
    for end in range(opening, len(code)):
        if code[end] == "{":
            depth += 1
        elif code[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start : end + 1]
    raise AssertionError(f"unterminated product definition: {signature}")


SIGNATURES = {
    "radio": ("src/jackal/radiobutton.cpp", "int RadioButton::attach("),
    "checkbox": ("src/jackal/checkbox.cpp", "int CheckBox::attach("),
    "spritebox": ("src/spritebox.cpp", "int SpriteBox::attach("),
}


HARNESS = r'''#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
using std::uint32_t;

static int dimension_result, init_result, dimension_calls, init_calls;
static int setter_calls, spot_init_calls, spot_add_calls;
static int *HandleMain;
static void (*init_callback)();
static int init_args[9];
static void *init_parent;
static int colors[12], fonts[4];
static int spot_args[6];

struct Buffer {
  void set_text_color(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    ++setter_calls; colors[0]=a; colors[1]=b; colors[2]=c; colors[3]=d;
  }
  void set_text_color2(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    ++setter_calls; colors[4]=a; colors[5]=b; colors[6]=c; colors[7]=d;
  }
  void set_text_color3(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    ++setter_calls; colors[8]=a; colors[9]=b; colors[10]=c; colors[11]=d;
  }
  void set_font(void *a, void *b, void *c, int d) {
    ++setter_calls; fonts[0]=a ? *static_cast<int *>(a) : -1;
    fonts[1]=b ? *static_cast<int *>(b) : -1;
    fonts[2]=c ? *static_cast<int *>(c) : -1; fonts[3]=d;
  }
};
struct GraphicWin : Buffer {
  int iFlags_;
  GraphicWin() : iFlags_(0) {}
  int init(int x, int y, int width, int height, int title, int flags,
           GraphicWin *parent, int menu, int border) {
    ++init_calls;
    init_args[0]=x; init_args[1]=y; init_args[2]=width; init_args[3]=height;
    init_args[4]=title; init_args[5]=flags; init_args[6]=menu; init_args[7]=border;
    init_parent=parent;
    if (init_callback) init_callback();
    return init_result;
  }
};
struct Dialog {
  int field_24_, field_28_;
  uint32_t field_40_, field_44_, field_4C_, field_54_, field_58_, field_5C_, field_60_, field_68_;
  void *font1_, *font2_, *font3_;
  uint32_t text_color_a_, text_color_b_, text_color_c_, text_color_d_;
  uint32_t text_color_2a_, text_color_2b_, text_color_2c_, text_color_2d_;
  uint32_t text_color_3a_, text_color_3b_, text_color_3c_, text_color_3d_;
  Dialog() : field_24_(0), field_28_(0), field_40_(3), field_44_(1),
    field_4C_(1), field_54_(1), field_58_(1), field_5C_(31), field_60_(47),
    field_68_(4), font1_(0), font2_(0), font3_(0), text_color_a_(1),
    text_color_b_(2), text_color_c_(3), text_color_d_(4), text_color_2a_(5),
    text_color_2b_(6), text_color_2c_(7), text_color_2d_(8), text_color_3a_(9),
    text_color_3b_(10), text_color_3c_(11), text_color_3d_(12) {}
};
struct Parent : GraphicWin {};
static int font_values[3] = {101, 202, 303};

struct RadioButton : virtual GraphicWin, virtual Dialog {
  int calculate_dimensions() { ++dimension_calls; return dimension_result; }
  int attach(GraphicWin *, int, int, int);
};
struct CheckBox : virtual GraphicWin, virtual Dialog {
  int calculate_dimensions() { ++dimension_calls; return dimension_result; }
  int attach(GraphicWin *, int, int, int);
};
struct Item { int id; };
struct ItemList {
  int entry_count_;
  Item item_, *current_;
  ItemList() : entry_count_(1), item_{73}, current_(&item_) {}
  bool seek_pos(int position) { current_=&item_; return position==0; }
};
struct Spot {
  void init(int n) { ++spot_init_calls; assert(n==1); }
  void add(int id, int type, int x, int y, int width, int height) {
    ++spot_add_calls; spot_args[0]=id; spot_args[1]=type; spot_args[2]=x;
    spot_args[3]=y; spot_args[4]=width; spot_args[5]=height;
  }
};
struct SpriteBox : virtual GraphicWin, virtual Dialog {
  ItemList list_;
  Spot spots_;
  uint32_t field_4C_, field_54_, field_58_;
  SpriteBox() : field_4C_(1), field_54_(40), field_58_(6) {}
  int calculate_dimensions() { ++dimension_calls; return dimension_result; }
  int attach(GraphicWin *, int, int, int);
};

/* RADIO */
/* CHECKBOX */
/* SPRITEBOX */

static void reset() {
  dimension_result=0; init_result=0; dimension_calls=init_calls=0;
  setter_calls=spot_init_calls=spot_add_calls=0; init_callback=0;
  init_parent=0; for (int &v : init_args) v=-999;
  for (int &v : colors) v=-999;
  for (int &v : fonts) v=-999;
  for (int &v : spot_args) v=-999;
}
static void mutate_radio() {
  // Callback-time source fields are intentionally read after GraphicWin::init.
  extern RadioButton *radio_under_test;
  radio_under_test->field_24_=701; radio_under_test->field_28_=702;
  radio_under_test->text_color_a_=21; radio_under_test->text_color_b_=22;
  radio_under_test->text_color_c_=23; radio_under_test->text_color_d_=24;
  radio_under_test->text_color_2a_=25; radio_under_test->text_color_2b_=26;
  radio_under_test->text_color_2c_=27; radio_under_test->text_color_2d_=28;
  radio_under_test->text_color_3a_=29; radio_under_test->text_color_3b_=30;
  radio_under_test->text_color_3c_=31; radio_under_test->text_color_3d_=32;
  radio_under_test->font1_=&font_values[0]; radio_under_test->font2_=&font_values[1];
  radio_under_test->font3_=&font_values[2];
}
static void mutate_checkbox() {
  extern CheckBox *checkbox_under_test;
  checkbox_under_test->field_24_=701; checkbox_under_test->field_28_=702;
  checkbox_under_test->text_color_a_=21; checkbox_under_test->text_color_b_=22;
  checkbox_under_test->text_color_c_=23; checkbox_under_test->text_color_d_=24;
  checkbox_under_test->text_color_2a_=25; checkbox_under_test->text_color_2b_=26;
  checkbox_under_test->text_color_2c_=27; checkbox_under_test->text_color_2d_=28;
  checkbox_under_test->text_color_3a_=29; checkbox_under_test->text_color_3b_=30;
  checkbox_under_test->text_color_3c_=31; checkbox_under_test->text_color_3d_=32;
  checkbox_under_test->font1_=&font_values[0]; checkbox_under_test->font2_=&font_values[1];
  checkbox_under_test->font3_=&font_values[2];
}
static void mutate_spritebox() {
  extern SpriteBox *spritebox_under_test;
  spritebox_under_test->field_24_=701; spritebox_under_test->field_28_=702;
  spritebox_under_test->text_color_a_=21; spritebox_under_test->text_color_b_=22;
  spritebox_under_test->text_color_c_=23; spritebox_under_test->text_color_d_=24;
  spritebox_under_test->text_color_2a_=25; spritebox_under_test->text_color_2b_=26;
  spritebox_under_test->text_color_2c_=27; spritebox_under_test->text_color_2d_=28;
  spritebox_under_test->text_color_3a_=29; spritebox_under_test->text_color_3b_=30;
  spritebox_under_test->text_color_3c_=31; spritebox_under_test->text_color_3d_=32;
  spritebox_under_test->font1_=&font_values[0]; spritebox_under_test->font2_=&font_values[1];
  spritebox_under_test->font3_=&font_values[2];
}
RadioButton *radio_under_test;
CheckBox *checkbox_under_test;
SpriteBox *spritebox_under_test;

static void check_init_arguments(int x, int y, int flags, GraphicWin *parent) {
  assert(init_calls==1 && init_args[0]==x && init_args[1]==y);
  assert(init_args[2]==31 && init_args[3]==47 && init_args[4]==0);
  assert(init_args[5]==flags && init_args[6]==0 && init_args[7]==0);
  assert(init_parent==parent);
}
static void check_callback_setters() {
  assert(setter_calls==4);
  const int expected[12]={21,22,23,24,25,26,27,28,29,30,31,32};
  for (int i=0;i<12;++i) assert(colors[i]==expected[i]);
  assert(fonts[0]==101 && fonts[1]==202 && fonts[2]==303 && fonts[3]==0);
}
static void check_radio() {
  int handle=1; HandleMain=&handle; Parent parent; RadioButton b; radio_under_test=&b;
  reset(); parent.iFlags_=0; init_result=-17; init_callback=mutate_radio;
  int result=b.attach(&parent,13,29,0x20);
  assert(result==-17 && b.field_24_==701 && b.field_28_==702);
  check_init_arguments(13,29,0x100020,&parent);
  assert(setter_calls==0 && spot_init_calls==0 && spot_add_calls==0);
  reset(); init_result=23; init_callback=mutate_radio;
  assert(b.attach(&parent,13,29,0x20)==23);
  check_init_arguments(13,29,0x100020,&parent);
  assert(setter_calls==0 && spot_init_calls==0 && spot_add_calls==0);
  reset(); dimension_result=19;
  assert(b.attach(&parent,13,29,0x20)==19);
  assert(dimension_calls==1 && init_calls==0 && setter_calls==0);
  reset(); dimension_result=-23;
  assert(b.attach(&parent,13,29,0x20)==-23);
  assert(dimension_calls==1 && init_calls==0 && setter_calls==0);
  reset(); dimension_result=1;
  assert(b.attach(&parent,13,29,0x20)==1);
  assert(dimension_calls==1 && init_calls==0 && setter_calls==0);
  reset(); dimension_result=-1;
  assert(b.attach(&parent,13,29,0x20)==-1);
  assert(dimension_calls==1 && init_calls==0 && setter_calls==0);
  reset(); parent.iFlags_=0x1000; init_callback=mutate_radio;
  assert(b.attach(&parent,-7,41,0x20)==0);
  check_init_arguments(-7,41,0x20,&parent); check_callback_setters();
  assert(b.field_24_==701 && b.field_28_==702);
  reset(); assert(b.attach(0,1,2,3)==3 && init_calls==0 && dimension_calls==0);
  int *old=HandleMain; HandleMain=0;
  assert(b.attach(&parent,1,2,3)==7 && init_calls==0 && dimension_calls==0);
  HandleMain=old;
}
static void check_checkbox() {
  int handle=1; HandleMain=&handle; Parent parent; CheckBox b; checkbox_under_test=&b;
  reset(); parent.iFlags_=0; dimension_result=-31; init_result=23; init_callback=mutate_checkbox;
  assert(b.attach(&parent,13,29,0x20)==23);
  assert(dimension_calls==1 && b.field_24_==701 && b.field_28_==702);
  check_init_arguments(13,29,0x100020,&parent);
  assert(setter_calls==0 && spot_init_calls==0 && spot_add_calls==0);
  reset(); dimension_result=37; init_result=-29; init_callback=mutate_checkbox;
  assert(b.attach(&parent,13,29,0x20)==-29);
  assert(dimension_calls==1 && setter_calls==0 && spot_init_calls==0);
  reset(); parent.iFlags_=0x1000; dimension_result=-19; init_callback=mutate_checkbox;
  assert(b.attach(&parent,-7,41,0x20)==0);
  assert(dimension_calls==1); check_init_arguments(-7,41,0x20,&parent);
  check_callback_setters(); assert(b.field_24_==701 && b.field_28_==702);
  reset(); assert(b.attach(0,1,2,3)==3 && init_calls==0 && dimension_calls==0);
  int *old=HandleMain; HandleMain=0;
  assert(b.attach(&parent,1,2,3)==7 && init_calls==0 && dimension_calls==0);
  HandleMain=old;
}
static void check_spritebox() {
  int handle=1; HandleMain=&handle; Parent parent; SpriteBox b; spritebox_under_test=&b;
  reset(); parent.iFlags_=0; dimension_result=-31; init_result=23; init_callback=mutate_spritebox;
  assert(b.attach(&parent,13,29,0x20)==23);
  assert(dimension_calls==1 && b.field_24_==701 && b.field_28_==702);
  check_init_arguments(13,29,0x100020,&parent);
  assert(setter_calls==0 && spot_init_calls==0 && spot_add_calls==0);
  reset(); dimension_result=37; init_result=-29; init_callback=mutate_spritebox;
  assert(b.attach(&parent,13,29,0x20)==-29);
  assert(dimension_calls==1 && setter_calls==0 && spot_init_calls==0);
  reset(); parent.iFlags_=0x1000; dimension_result=-19; init_callback=mutate_spritebox;
  assert(b.attach(&parent,-7,41,0x20)==0);
  assert(dimension_calls==1); check_init_arguments(-7,41,0x20,&parent);
  assert(spot_init_calls==1 && spot_add_calls==1);
  assert(spot_args[0]==73 && spot_args[1]==0 && spot_args[2]==0 &&
         spot_args[3]==2 && spot_args[4]==40 && spot_args[5]==4);
  check_callback_setters(); assert(b.field_24_==701 && b.field_28_==702);
  reset(); assert(b.attach(0,1,2,3)==3 && init_calls==0 && dimension_calls==0);
  int *old=HandleMain; HandleMain=0;
  assert(b.attach(&parent,1,2,3)==7 && init_calls==0 && dimension_calls==0);
  HandleMain=old;
}
int main() { check_radio(); check_checkbox(); check_spritebox(); }
'''


def actual_bodies() -> dict[str, str]:
    return {
        name: extract_definition((ROOT / path).read_text(encoding="utf-8"), signature)
        for name, (path, signature) in SIGNATURES.items()
    }


def source_text(bodies: dict[str, str]) -> str:
    result = HARNESS
    for key in SIGNATURES:
        marker = f"/* {key.upper()} */"
        assert result.count(marker) == 1
        result = result.replace(marker, bodies[key])
    return result


def compile_and_run(tmp_path: Path, compiler: str, stem: str,
                    bodies: dict[str, str]) -> subprocess.CompletedProcess:
    cpp = tmp_path / f"{stem}.cpp"
    binary = tmp_path / stem
    cpp.write_text(source_text(bodies), encoding="utf-8")
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(binary)], check=True,
                   capture_output=True, text=True, timeout=30)
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)


def named_mutants(bodies: dict[str, str]) -> list[tuple[str, dict[str, str]]]:
    mutants: list[tuple[str, dict[str, str]]] = []

    def change(name: str, owner: str, old: str, new: str) -> None:
        copied = dict(bodies)
        assert copied[owner].count(old) == 1, (name, owner, old)
        copied[owner] = copied[owner].replace(old, new, 1)
        mutants.append((name, copied))

    init_old = "if (init_result != 0) return init_result;"
    variants = (
        ("ignore", "if (false) return init_result;"),
        ("zero", "if (init_result != 0) return 0;"),
        ("plus_one", "if (init_result != 0) return init_result + 1;"),
        ("minus_one", "if (init_result != 0) return init_result - 1;"),
        ("positive_only", "if (init_result > 0) return init_result;"),
        ("negative_only", "if (init_result < 0) return init_result;"),
        ("positive_magnitude", "if (init_result > 1) return init_result;"),
        ("negative_magnitude", "if (init_result < -1) return init_result;"),
        ("negated", "if (init_result != 0) return -init_result;"),
        ("always_return", "if (init_result == 0) return init_result;"),
    )
    for owner in ("radio", "checkbox", "spritebox"):
        for label, replacement in variants:
            change(f"{owner}_init_{label}", owner, init_old, replacement)

    # RadioButton alone must propagate calculate_dimensions() failures.
    radio_dimension = "if (dimensions != 0) return dimensions;"
    for label, replacement in (
        ("ignore", "if (false) return dimensions;"),
        ("zero", "if (dimensions != 0) return 0;"),
        ("positive_only", "if (dimensions > 0) return dimensions;"),
        ("negative_only", "if (dimensions < 0) return dimensions;"),
        ("magnitude", "if (dimensions > 1 || dimensions < -1) return dimensions;"),
        ("negated", "if (dimensions != 0) return -dimensions;"),
        ("plus_one", "if (dimensions != 0) return dimensions + 1;"),
    ):
        change(f"radio_dimensions_{label}", "radio", radio_dimension, replacement)

    # CheckBox and SpriteBox deliberately ignore their dimensions result.
    for owner in ("checkbox", "spritebox"):
        old = "calculate_dimensions();"
        for label, replacement in (
            ("propagate", "if (calculate_dimensions() != 0) return dimension_result;"),
            ("normalize", "if (calculate_dimensions() != 0) return 0;"),
            ("positive_only", "if (calculate_dimensions() > 0) return dimension_result;"),
            ("negative_only", "if (calculate_dimensions() < 0) return dimension_result;"),
            ("wrong_error", "if (calculate_dimensions() != 0) return -1;"),
            ("duplicate_call", "calculate_dimensions(); calculate_dimensions();"),
        ):
            change(f"{owner}_dimensions_{label}", owner, old, replacement)
    assert len(mutants) == 49
    return mutants


def test_product_definitions_are_complete_and_exercise_the_frozen_contract():
    bodies = actual_bodies()
    assert all("int " + cls + "::attach(" in bodies[key]
               for key, cls in (("radio", "RadioButton"), ("checkbox", "CheckBox"),
                                ("spritebox", "SpriteBox")))
    assert "if (dimensions != 0) return dimensions;" in bodies["radio"]
    assert "calculate_dimensions();" in bodies["checkbox"]
    assert "calculate_dimensions();" in bodies["spritebox"]
    assert all("if (init_result != 0) return init_result;" in bodies[k]
               for k in SIGNATURES)
    assert "spots_.init" in bodies["spritebox"] and "spots_.add" in bodies["spritebox"]


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_attach_bodies_with_gcc_and_clang(compiler_name: str, tmp_path: Path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    result = compile_and_run(tmp_path, compiler, f"actual-{compiler_name}", actual_bodies())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_98_compile_success_named_mutants_fail_at_runtime(
        compiler_name: str, tmp_path: Path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    bodies = actual_bodies()
    mutants = named_mutants(bodies)
    assert len(mutants) == 49
    for name, damaged in mutants:
        result = compile_and_run(tmp_path, compiler, f"{compiler_name}-{name}", damaged)
        assert result.returncode != 0, f"{name} escaped runtime controls"
