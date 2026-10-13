import re
import shutil
import subprocess
from pathlib import Path

import pytest

from match_recovery.source import read_text


ROOT = Path(__file__).resolve().parents[2]


def actual_sources():
    paths = ("src/listbox.cpp", "src/listbox.h", "src/win.cpp",
             "src/win.h", "src/dialogs.cpp", "src/dialogs.h")
    return {path: (ROOT / path).read_text() for path in paths}


def extract_function(text, signature):
    start = text.index(signature)
    opening = text.index("{", start + len(signature))
    depth = 0
    state = "code"
    escaped = False
    index = opening
    while index < len(text):
        ch = text[index]
        nxt = text[index + 1] if index + 1 < len(text) else ""
        if state == "line_comment":
            if ch == "\n":
                state = "code"
        elif state == "block_comment":
            if ch == "*" and nxt == "/":
                state = "code"
                index += 1
        elif state in ("string", "char"):
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif (state == "string" and ch == '"') or (state == "char" and ch == "'"):
                state = "code"
        else:
            if ch == "/" and nxt == "/":
                state = "line_comment"
                index += 1
            elif ch == "/" and nxt == "*":
                state = "block_comment"
                index += 1
            elif ch == '"':
                state = "string"
            elif ch == "'":
                state = "char"
            elif ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return text[start:index + 1]
        index += 1
    raise AssertionError(f"unterminated C++ function {signature}")


def actual_listbox_methods(sources):
    source = sources["src/listbox.cpp"]
    methods = [
        extract_function(source, "Scroll *ListBox::on_scroll_create()"),
        extract_function(source, "Scroll *ListBox::on_scroll_delete(Scroll *scroll)"),
    ]
    return "\n\n".join(methods)


def close_contract_error(source):
    close = extract_function(source, "void Win::close()")
    for member in ("scroll_vert_", "scroll_horz_"):
        guard = rf"if\s*\(\s*\({member}\s*!=\s*0\)\s*&&\s*\(\(iFlags_\s*&\s*0x40\)\s*!=\s*0\)\s*\)"
        if not re.search(guard, close):
            return f"missing original flag/nonnull guard for {member}"
        store = rf"{member}\s*=\s*this->on_scroll_delete\(\s*{member}\s*\)"
        if not re.search(store, close):
            return f"missing matching argument/result store for {member}"
    return None


def test_actual_source_retains_both_original_identities_and_spans():
    list_cpp = actual_sources()["src/listbox.cpp"]
    constructor_marker = list_cpp.index("// ORIGINAL: 0x00609DB0")
    assert constructor_marker > list_cpp.index("Scroll *ListBox::on_scroll_delete(Scroll *scroll)")
    constructor_prefix = list_cpp[constructor_marker:list_cpp.index("ListBox::ListBox() {")]
    assert "// symbol    ??0ListBox@@QAE@XZ" in constructor_prefix
    assert "// ORIGINAL: 0x0060CED0" not in constructor_prefix
    assert "// ORIGINAL: 0x0060CF10" not in constructor_prefix
    records = {record.address: record for record in read_text(list_cpp, "src/listbox.cpp")
               if record.address in (0x60CED0, 0x60CF10)}
    assert records[0x60CED0].image_spans == ((0x60CED0, 0x60CF01),)
    assert records[0x60CF10].image_spans == ((0x60CF10, 0x60CF4D),)
    assert records[0x60CED0].recipe.value == "verbatim"
    assert records[0x60CF10].recipe.value == "verbatim"
    assert "0x0060CED0 sub_60ced0" in list_cpp
    assert "0x0060CF10 sub_60cf10" in list_cpp


def test_slot_bindings_and_argument_order():
    sources = actual_sources()
    win_h = sources["src/win.h"]
    win_cpp = sources["src/win.cpp"]
    list_h = sources["src/listbox.h"]
    list_cpp = sources["src/listbox.cpp"]
    dialogs_h = sources["src/dialogs.h"]
    dialogs_cpp = sources["src/dialogs.cpp"]

    assert "virtual Scroll *on_scroll_create() { return 0; }" in win_h
    assert "virtual Scroll *on_scroll_delete(Scroll *scroll)" in win_h
    assert "scroll_vert_ = this->on_scroll_delete(scroll_vert_);" in win_cpp
    assert "scroll_horz_ = this->on_scroll_delete(scroll_horz_);" in win_cpp
    assert "scroll_vert_ = this->on_scroll_create();" in win_cpp
    assert "scroll_horz_ = this->on_scroll_create();" in win_cpp
    assert "this->buffer3_ = reinterpret_cast<Buffer *>(this->on_scroll_create())" not in win_cpp
    assert "this->buffer4_ = reinterpret_cast<Buffer *>(this->on_scroll_create())" not in win_cpp
    assert "this->vslot_07();" not in win_cpp
    assert "virtual Scroll *on_scroll_create();" in list_h
    assert "virtual Scroll *on_scroll_delete(Scroll *scroll);" in list_h
    assert "(iFlags_ & 0x40) != 0 && win_parent_ != 0" in list_cpp
    assert "win_parent_->on_listbox_scroll_create()" in list_cpp
    assert "win_parent_->on_listbox_scroll_delete(scroll)" in list_cpp
    assert "val_20_" not in list_cpp and "val_31_" not in list_cpp
    assert "virtual Scroll *on_scroll_delete(Scroll *scroll);" in dialogs_h
    assert "virtual Scroll *on_scroll_create();" in dialogs_h
    assert "Scroll *Dialogs::on_scroll_delete(Scroll *scroll)" in dialogs_cpp
    assert "kind_ == 2" in dialogs_cpp

    assert "0x005EB73E" in win_cpp and "scroll_vert_" in win_cpp
    assert "0x005EB774" in win_cpp and "scroll_horz_" in win_cpp


SCROLL_HOST = r'''#include <cassert>
#include <cstdint>
struct Scroll {};
struct Sound {};
struct AutoSound { Sound *val_20_=nullptr; Sound *val_31_=nullptr; };
struct Win : AutoSound {
  uint32_t iFlags_=0;
  Win *win_parent_=nullptr;
  virtual Scroll *on_scroll_create() { return nullptr; }
  virtual Scroll *on_scroll_delete(Scroll *s) { return s; }
  virtual Scroll *on_listbox_scroll_create() { return nullptr; }
  virtual Scroll *on_listbox_scroll_delete(Scroll *s) { return s; }
};
struct GraphicWin : Win {
  Win *parent_window() const { return win_parent_; }
};
struct Dialog {};
struct Recipient : Win {
  int create_calls=0, delete_calls=0;
  Scroll *last_scroll=nullptr, *create_result=nullptr, *delete_result=nullptr;
  Scroll *on_listbox_scroll_create() override { ++create_calls; return create_result; }
  Scroll *on_listbox_scroll_delete(Scroll *s) override {
    ++delete_calls; last_scroll=s; return delete_result;
  }
};
struct ListBox : virtual GraphicWin, virtual Dialog {
  Scroll *on_scroll_create() override;
  Scroll *on_scroll_delete(Scroll *scroll) override;
};
/* ACTUAL_LISTBOX_METHODS */
int main() {
  ListBox list; Recipient recipient; Scroll vertical, horizontal, created, replacement;
  Sound decoy; list.val_20_=&decoy; list.val_31_=&decoy;
  list.win_parent_=&recipient;
  assert(list.on_scroll_create()==nullptr);
  assert(list.on_scroll_delete(&vertical)==&vertical);
  assert(recipient.create_calls==0 && recipient.delete_calls==0);

  list.iFlags_=0x40; list.win_parent_=nullptr;
  assert(list.on_scroll_create()==nullptr);
  assert(list.on_scroll_delete(&horizontal)==&horizontal);
  assert(recipient.create_calls==0 && recipient.delete_calls==0);

  list.win_parent_=&recipient;
  recipient.create_result=&created; recipient.delete_result=&replacement;
  assert(list.on_scroll_create()==&created);
  assert(list.on_scroll_delete(&vertical)==&replacement);
  assert(recipient.create_calls==1 && recipient.delete_calls==1);
  assert(recipient.last_scroll==&vertical);
  recipient.delete_result=nullptr;
  assert(list.on_scroll_delete(&horizontal)==nullptr);
  assert(recipient.last_scroll==&horizontal);
}
'''


def compile_scroll_host(tmp_path, compiler, methods, stem):
    source = tmp_path / f"{stem}.cpp"
    source.write_text(SCROLL_HOST.replace("/* ACTUAL_LISTBOX_METHODS */", methods))
    binary = tmp_path / stem
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(binary)], check=True,
                   capture_output=True, text=True)
    return subprocess.run([str(binary)], capture_output=True, text=True)


def test_host_behavior_control_extracts_actual_listbox_bodies(tmp_path):
    compiler = shutil.which("c++") or shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("no host C++ compiler available")
    sources = actual_sources()
    methods = actual_listbox_methods(sources)
    result = compile_scroll_host(tmp_path, compiler, methods, "win_slot67_control")
    assert result.returncode == 0, result.stderr


def test_listbox_mutations_are_exercised_against_extracted_bodies(tmp_path):
    compiler = shutil.which("c++") or shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("no host C++ compiler available")
    original = actual_listbox_methods(actual_sources())
    mutants = (
        original.replace("0x40", "0x20"),
        original.replace("on_listbox_scroll_delete(scroll)",
                         "on_listbox_scroll_delete(nullptr)", 1),
        original.replace("return scroll;", "return nullptr;", 1),
    )
    assert all(mutant != original for mutant in mutants)
    for index, mutant in enumerate(mutants):
        source = tmp_path / f"mutant_{index}.cpp"
        source.write_text(SCROLL_HOST.replace("/* ACTUAL_LISTBOX_METHODS */", mutant))
        binary = tmp_path / f"mutant_{index}"
        built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                                str(source), "-o", str(binary)],
                               stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        assert built.returncode == 0, built.stderr
        ran = subprocess.run([str(binary)], stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                             text=True)
        assert ran.returncode != 0, f"mutant {index} unexpectedly passed"


def test_win_close_argument_and_result_stores_come_from_actual_source():
    source = actual_sources()["src/win.cpp"]
    assert close_contract_error(source) is None
    close = extract_function(source, "void Win::close()")
    mutants = (
        close.replace("(iFlags_ & 0x40)", "(iFlags_ & 0x20)", 1),
        close.replace("on_scroll_delete(scroll_vert_)",
                      "on_scroll_delete(scroll_horz_)", 1),
        close.replace("scroll_vert_ = this->on_scroll_delete(scroll_vert_);",
                      "this->on_scroll_delete(scroll_vert_);", 1),
    )
    assert all(mutant != close for mutant in mutants)
    for mutant in mutants:
        assert close_contract_error("void Win::close() {" + mutant.split("{", 1)[1]) is not None
