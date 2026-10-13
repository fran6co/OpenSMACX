"""Host controls for ProdPicker's actual selected-frame selection phase.

The extracted source reaches the real ``box_sprite`` consumer. The host record
uses ten 32-bit words to retain the pinned 0x28 stride; it is not a target ABI
or native-runtime test.
"""
from __future__ import annotations

import os
import re
import resource
import shutil
import subprocess
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
IMAGE_FALLBACK = ROOT / ".opensmacx/game/terranx_original.exe"
DRAW_SIGNATURE = "void ProdPicker::draw_item(int item, int update)"


def extract_definition(source: str, signature: str = DRAW_SIGNATURE) -> str:
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
    raise AssertionError("unterminated ProdPicker::draw_item definition")


def actual_style_phase() -> str:
    body = extract_definition((ROOT / "src/prodpicker.cpp").read_text(encoding="utf-8"))
    start = body.index("    BoxSpriteParams *frame =")
    end_line = "    box_sprite(&cell, frame);"
    end = body.index(end_line, start) + len(end_line)
    fragment = body[start:end]
    assert fragment.count("BoxSpriteParams *frame =") == 1
    assert fragment.count("box_sprite(&cell, frame);") == 1
    assert "frame = &InterfaceBoxStyles[2];" in fragment, (
        "selected production rows must borrow complete style record 2"
    )
    assert "reinterpret_cast<BoxSpriteParams *>" not in fragment
    return fragment


HARNESS = r'''#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>

static const char *control_name="/* CONTROL_NAME */";
static void fail(const char *expression) {
  std::fprintf(stderr,"CONTROL_FAIL[%s]: %s\n",control_name,expression);
  std::exit(98);
}
#define CHECK(expression) do { if (!(expression)) fail(#expression); } while (0)

struct RECT { int left, top, right, bottom; };
struct BoxSpriteParams {
  int corner0_, corner1_, corner2_, corner3_;
  int edge0_, edge1_, edge2_, edge3_;
  int mask_, mask_color_;
};
static_assert(sizeof(BoxSpriteParams)==0x28, "host record stride witness");
BoxSpriteParams InterfaceBoxStyles[51];
static int clip_calls, sprite_calls;
static RECT last_rect;
static BoxSpriteParams *last_pointer;
static BoxSpriteParams *expected_pointer;
static int consumed[10];

class ProdPickerProbe {
public:
  uint32_t field_A774_;
  ProdPickerProbe() : field_A774_(0) {}
  void set_clip(RECT *rect) { ++clip_calls; last_rect=*rect; }
  int box_sprite(RECT *rect, BoxSpriteParams *params) {
    if (params != expected_pointer)
      fail("box_sprite received unexpected BoxSpriteParams pointer");
    ++sprite_calls; last_rect=*rect; last_pointer=params;
    consumed[0]=params->corner0_; consumed[1]=params->corner1_;
    consumed[2]=params->corner2_; consumed[3]=params->corner3_;
    consumed[4]=params->edge0_; consumed[5]=params->edge1_;
    consumed[6]=params->edge2_; consumed[7]=params->edge3_;
    consumed[8]=params->mask_; consumed[9]=params->mask_color_;
    return 0;
  }
  void run_style(int item) {
    RECT cell={11,22,33,44};
    /* ACTUAL_STYLE_PHASE */
  }
};

static void set_style(int style, int generation=0) {
  int words[10];
  for (int field=0; field<10; ++field)
    words[field]=generation*100000 + style*1000 + field + 1;
  std::memcpy(&InterfaceBoxStyles[style], words, sizeof words);
}
static void expect_style(ProdPickerProbe &picker, int item, uint32_t selected,
                         int expected_style, int generation=0) {
  clip_calls=sprite_calls=0; last_pointer=0;
  expected_pointer=&InterfaceBoxStyles[expected_style];
  picker.field_A774_=selected;
  picker.run_style(item);
  CHECK(clip_calls==0); // the tested phase starts after draw_item's set_clip
  CHECK(sprite_calls==1);
  CHECK(last_pointer==&InterfaceBoxStyles[expected_style]);
  int expected_words[10];
  for (int field=0; field<10; ++field) {
    const int expected=generation*100000 + expected_style*1000 + field + 1;
    expected_words[field]=expected;
  }
  CHECK(std::memcmp(last_pointer, expected_words, sizeof expected_words)==0);
  CHECK(std::memcmp(consumed, expected_words, sizeof expected_words)==0);
  CHECK(last_rect.left==11 && last_rect.top==22 &&
        last_rect.right==33 && last_rect.bottom==44);
}
int main() {
  for (int style=0; style<51; ++style) set_style(style);
  ProdPickerProbe picker;
  expect_style(picker, 7, 7, 2);
  expect_style(picker, 8, 7, 0);
  expect_style(picker, -1, UINT32_MAX, 2);
  expect_style(picker, -1, 0, 0);
  expect_style(picker, 2147483647, 2147483647, 2);
  expect_style(picker, (-2147483647-1), (-2147483647-1), 2);
  // The table is mutable shared storage. Each draw must borrow the current
  // complete record, not retain copied or cached field values.
  set_style(2, 9);
  expect_style(picker, 7, 7, 2, 9);
}
'''


def compile_and_run(tmp_path: Path, compiler: str, stem: str,
                    fragment: str, case_name: str = "actual") -> subprocess.CompletedProcess:
    source = HARNESS.replace("/* ACTUAL_STYLE_PHASE */", fragment)
    source = source.replace("/* CONTROL_NAME */", case_name)
    assert source.count("/* ACTUAL_STYLE_PHASE */") == 0
    cpp = tmp_path / f"{stem}.cpp"
    binary = tmp_path / stem
    cpp.write_text(source, encoding="utf-8")
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    "-Wno-sign-compare",
                    str(cpp), "-o", str(binary)], check=True,
                   capture_output=True, text=True, timeout=30)
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=30,
                          preexec_fn=lambda: resource.setrlimit(
                              resource.RLIMIT_CORE, (0, 0)))


def named_mutants(fragment: str) -> list[tuple[str, str]]:
    selected = "frame = &InterfaceBoxStyles[2];"
    assert fragment.count(selected) == 1
    mutants = [
        ("wrong_old_edge1_view",
         fragment.replace(selected,
             "frame = reinterpret_cast<BoxSpriteParams *>(&InterfaceBoxStyles[0].edge1_);")),
        ("reversed_selection",
         fragment.replace("field_A774_ == static_cast<uint32_t>(item)",
                           "field_A774_ != static_cast<uint32_t>(item)")),
    ]
    for index in [0, 1, *range(3, 48)]:
        mutants.append((f"wrong_selected_index_{index}",
                        fragment.replace(selected,
                                         f"frame = &InterfaceBoxStyles[{index}];")))
    assert len(mutants)==49
    assert any(name=="wrong_selected_index_1" for name, _ in mutants)
    assert all(damaged != fragment for _, damaged in mutants)
    return mutants


def test_pinned_style_index_sequence_and_original_span():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin

    image_path = Path(os.environ.get("OPENSMACX_IMAGE", IMAGE_FALLBACK))
    if not image_path.is_file():
        pytest.skip("pinned executable unavailable; set OPENSMACX_IMAGE")
    assert image_pin(ROOT) == PIN
    image = PeImage(image_path.read_bytes(), PIN)
    ins = {i.address: (i.mnemonic, i.op_str)
           for i in Cs(CS_ARCH_X86, CS_MODE_32).disasm(
               image.read(Span(0x4927C0, 0x492F9F)), 0x4927C0)}
    expected = {
        0x492867: ("mov", "ecx, dword ptr [esi + 0xa774]"),
        0x49286F: ("cmp", "ecx, ebx"),
        0x492871: ("setne", "al"),
        0x492874: ("dec", "eax"),
        0x492878: ("and", "eax, 2"),
        0x49287B: ("lea", "eax, [eax + eax*4]"),
        0x49287E: ("lea", "ecx, [eax*8 + 0x78d528]"),
        0x492885: ("push", "ecx"),
        0x492889: ("call", "0x5e2dd3"),
    }
    for address, instruction in expected.items():
        assert ins[address] == instruction
    assert 0x492886 in ins and 0x492887 in ins
    # Equality produces offset 0x50 in the native multiply/add sequence.
    assert 2 * 0x28 == 0x50


def test_actual_style_phase_and_full_mutable_record_consumption():
    assert "InterfaceBoxStyles[0].edge1_" not in actual_style_phase()
    assert "InterfaceBoxStyles[2]" in actual_style_phase()


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_literal_actual_style_phase_under_gcc_and_clang(
        compiler_name: str, tmp_path: Path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    result = compile_and_run(tmp_path, compiler, f"actual-{compiler_name}",
                             actual_style_phase())
    assert result.returncode == 0, result.stderr.decode(errors="replace")


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_98_compile_success_named_style_mutants_fail_at_runtime(
        compiler_name: str, tmp_path: Path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    mutants = named_mutants(actual_style_phase())
    assert len(mutants)==49
    for name, damaged in mutants:
        result = compile_and_run(tmp_path, compiler, f"{compiler_name}-{name}",
                                 damaged, f"{compiler_name}-{name}")
        diagnostic = f"CONTROL_FAIL[{compiler_name}-{name}]:"
        assert result.returncode == 98, (
            f"{name} must exit 98, got {result.returncode}: {result.stderr!r}"
        )
        assert diagnostic in result.stderr, (
            f"{name} failed without its named runtime diagnostic: {result.stderr!r}"
        )
