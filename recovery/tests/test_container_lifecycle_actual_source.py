"""Actual-product controls for shared allocation and list/widget lifetimes.

These tests extract the live product method bodies; fixtures only provide
host-observable stand-ins for platform bases, heap ownership and event logs.
They do not claim VC6 layout, linked selection, or byte recovery.
"""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest
from match_recovery.source import read_file

ROOT = next(p for p in Path(__file__).resolve().parents
            if (p / "src/jackal/radiobutton.cpp").is_file())
COMPILERS = [p for p in (shutil.which("g++"), shutil.which("clang++")) if p]


def _body(path: str, signature: str) -> str:
    source = (ROOT / path).read_text(encoding="utf-8")
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for pos in range(opening, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[opening : pos + 1]
    raise AssertionError("unclosed product method: " + signature)


def _record(path: str, address: int):
    found = [row for row in read_file(ROOT / path) if row.address == address]
    assert len(found) == 1, f"expected one transported identity {address:#x} in {path}"
    return found[0]


def _fixture(cleanup: str, string_dtor: str, radio_close: str,
             radio_dtor: str) -> str:
    return r"""#include <cassert>
#include <cstddef>
#include <cstdlib>
typedef unsigned int uint32_t;
static int frees;
static void *last_freed;
static void tracked_free(void *p) { ++frees; last_freed = p; std::free(p); }
struct Heap {};
static Heap *StringAllocationHeap;

static int list_removals, cursor_seen_by_base;
struct StringStruct {
  void *head_; void *current_; int entry_count_; int current_position_;
  void *allocator_;
  virtual ~StringStruct() { cursor_seen_by_base = current_position_; }
};
struct StringList : StringStruct {
  StringList() {}
  void remove_all() { ++list_removals; head_ = current_ = 0; entry_count_ = 0; current_position_ = 99; }
  ~StringList();
};

static int close_order[2], close_count;
static int default_one, default_two;
static int *RadioButtonDefault1 = &default_one;
static int *RadioButtonDefault2 = &default_two;
struct Dialog { virtual ~Dialog() {} void close() { close_order[close_count++] = 1; } };
struct GraphicWin { virtual ~GraphicWin() {} void close() { close_order[close_count++] = 2; } };
struct RadioButton : public virtual GraphicWin, public virtual Dialog {
  uint32_t field_C_; int *double_click_callback_;
  int *checked_sprite_; int *unchecked_sprite_;
  RadioButton() : field_C_(7), double_click_callback_(reinterpret_cast<int *>(1)),
    checked_sprite_(0), unchecked_sprite_(0) {}
  void close();
  virtual ~RadioButton() """ + radio_dtor + r"""
};
#define free tracked_free
void cleanup_failed_net_construction(void *memory) {
""" + cleanup[1:-1] + r"""
}
#undef free
StringList::~StringList() """ + string_dtor + r"""
void RadioButton::close() """ + radio_close + r"""
int main() {
  Heap owner;
  StringAllocationHeap = &owner;
  cleanup_failed_net_construction(0);
  if (StringAllocationHeap != &owner || frees != 0) return 11;
  StringAllocationHeap = 0;
  void *plain = std::malloc(8);
  cleanup_failed_net_construction(plain);
  if (frees != 1 || last_freed != plain || StringAllocationHeap != 0) return 12;
  StringAllocationHeap = &owner;
  void *owned = std::malloc(8);
  cleanup_failed_net_construction(owned);
  if (frees != 1 || StringAllocationHeap != 0) return 13;
  std::free(owned);

  {
    StringList list;
    list.head_ = reinterpret_cast<void *>(1); list.current_ = list.head_;
    list.entry_count_ = 2; list.current_position_ = 1;
  }
  if (list_removals != 1 || cursor_seen_by_base != 0) return 14;

  close_count = 0;
  {
    RadioButton radio;
    radio.close();
    if (radio.field_C_ != 0 || radio.double_click_callback_ != 0 ||
        radio.checked_sprite_ != RadioButtonDefault2 ||
        radio.unchecked_sprite_ != RadioButtonDefault1) return 15;
    if (close_count != 2 || close_order[0] != 1 || close_order[1] != 2) return 16;
    close_count = 0;
  }
  if (close_count != 2 || close_order[0] != 1 || close_order[1] != 2) return 17;
  return 0;
}
"""


@pytest.fixture(params=COMPILERS or [None], ids=[Path(p).name for p in COMPILERS] or ["no-host-compiler"])
def compiler(request):
    if request.param is None:
        pytest.skip("no GCC or Clang host compiler available")
    return request.param


def _run(tmp_path: Path, compiler: str, source: str) -> subprocess.CompletedProcess[str]:
    tmp_path.mkdir(parents=True, exist_ok=True)
    cpp, exe = tmp_path / "lifecycle.cpp", tmp_path / "lifecycle"
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                            str(cpp), "-o", str(exe)], capture_output=True,
                           text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)


def test_transported_lifecycle_identities_and_distinct_radio_deleting_entry():
    delete_cleanup = _record("src/net_class.cpp", 0x00402320)
    list_delete = _record("src/dialog.cpp", 0x00406670)
    radio_body = _record("src/dialogs.cpp", 0x00406A90)
    radio_scalar = _record("src/dialogs.cpp", 0x00406F60)
    assert delete_cleanup.byte_exact and delete_cleanup.symbol == "?cleanup_failed_net_construction@@YAXPAX@Z"
    assert list_delete.byte_exact and list_delete.symbol == "??_GStringList@@UAEPAXI@Z"
    assert radio_body.byte_exact and radio_body.symbol == "??1RadioButton@@UAE@XZ"
    assert radio_scalar.byte_exact and radio_scalar.symbol == "??_GRadioButton@@UAEPAXI@Z"
    for address, name, span, symbol in (
        (0x00406A90, "sub_406a90", (0x00406A90, 0x00406AE3), "??1RadioButton@@UAE@XZ"),
        (0x00406F60, "??1RadioButton@@QAE@H@Z", (0x00406F60, 0x00406FE2), "??_GRadioButton@@UAEPAXI@Z"),
        (0x00406F50, "??1thunk1_RadioButton@@QAE@H@Z", (0x00406F50, 0x00406F58), "??_ERadioButton@@$4PPPPPPPM@A@AEPAXI@Z"),
        (0x00407060, "??1thunk2_RadioButton@@QAE@H@Z", (0x00407060, 0x00407071), "??_ERadioButton@@$4PPPPPLLI@EEE@AEPAXI@Z"),
        (0x00407080, "??1thunk3_RadioButton@@QAE@H@Z", (0x00407080, 0x0040708E), "??_ERadioButton@@$4PPPPPPPM@KBI@AEPAXI@Z"),
    ):
        row = _record("src/dialogs.cpp", address)
        assert row.name == name
        assert row.image_spans == (span,)
        assert row.mode.value == "file"
        assert row.byte_exact and not row.semantic
        assert row.symbol == symbol


def test_actual_cleanup_and_stringlist_destructor_bodies_reject_mutants(tmp_path: Path, compiler: str):
    cleanup = _body("src/net_class.cpp", "void cleanup_failed_net_construction(void *memory)")
    string_dtor = _body("src/stringlist.cpp", "StringList::~StringList()")
    close = _body("src/jackal/radiobutton.cpp", "void RadioButton::close()")
    radio_dtor = _body("src/jackal/radiobutton.h", "virtual ~RadioButton()")
    good = _run(tmp_path / "good", compiler, _fixture(cleanup, string_dtor, close, radio_dtor))
    assert good.returncode == 0, good.stdout + good.stderr

    null_mutant = cleanup.replace("if (memory != 0)", "if (memory == 0)", 1)
    assert null_mutant != cleanup
    assert _run(tmp_path / "null-mutant", compiler,
                _fixture(null_mutant, string_dtor, close, radio_dtor)).returncode != 0

    list_mutant = string_dtor.replace("current_position_ = 0;", "/* omitted cursor reset */", 1)
    assert list_mutant != string_dtor
    assert _run(tmp_path / "list-mutant", compiler,
                _fixture(cleanup, list_mutant, close, radio_dtor)).returncode != 0

    radio_mutant = radio_dtor.replace("close();", "/* omitted close */", 1)
    assert radio_mutant != radio_dtor
    assert _run(tmp_path / "radio-mutant", compiler,
                _fixture(cleanup, string_dtor, close, radio_mutant)).returncode != 0


def test_radio_destructor_is_inline_and_not_a_second_scalar_deleting_body():
    header = (ROOT / "src/jackal/radiobutton.h").read_text(encoding="utf-8")
    cpp = (ROOT / "src/jackal/radiobutton.cpp").read_text(encoding="utf-8")
    assert _body("src/jackal/radiobutton.h", "virtual ~RadioButton()") == "{ close(); }"
    assert "RadioButton::~RadioButton()" not in cpp
    assert "??_GRadioButton@@UAEPAXI@Z" in _record("src/dialogs.cpp", 0x00406F60).symbol
    assert _record("src/dialogs.cpp", 0x00406A90).symbol != _record("src/dialogs.cpp", 0x00406F60).symbol
    # This is declaration evidence only, not a host layout or native ABI claim.
    assert "class RadioButton : public virtual GraphicWin, public virtual Dialog" in header
