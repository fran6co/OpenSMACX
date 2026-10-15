"""Actual StringBox insertion-result behavior; host tests do not prove ABI."""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_spritelist_base_lifetime import extract_definition


ROOT = Path(__file__).resolve().parents[2]


HARNESS = r'''
#include <cstdint>
#include <string>
#include <vector>

static std::vector<std::string> events;

struct StringList {
  char *pending_text_;
  uint32_t field_20_;
  uint32_t field_24_;
  int add_result_;
  int add_calls_;
  int last_id_;

  StringList() : pending_text_(0), field_20_(0), field_24_(0),
                 add_result_(0), add_calls_(0), last_id_(-1) {}
  int add(int id) {
    events.push_back("insert");
    ++add_calls_;
    last_id_ = id;
    return add_result_;
  }
  int add_special(char *text);
};

struct StringBox {
  StringList stringList_;
  int fixup_calls_;
  StringBox() : fixup_calls_(0) {}
  void add_fixup() { events.push_back("fixup"); ++fixup_calls_; }
  int add_special(char *text);
};
'''


MAIN = r'''
#define REQUIRE(test, code) do { if (!(test)) return code; } while (0)

int main() {
  StringBox box;
  char caret[] = "^entry";

  // The actual list parser forwards one ID-zero insertion and normalizes its
  // result; the enclosing box returns failure unchanged and skips fixup.
  box.stringList_.add_result_ = 6;
  events.clear();
  const int failed = box.add_special(caret);
  REQUIRE(failed == 1, 10);
  REQUIRE(box.stringList_.pending_text_ == caret + 1 &&
          box.stringList_.field_20_ == 1 && box.stringList_.field_24_ == 0, 11);
  REQUIRE(box.stringList_.last_id_ == 0 && box.stringList_.add_calls_ == 1, 12);
  REQUIRE(box.fixup_calls_ == 0 && events.size() == 1 &&
          events[0] == "insert", 13);

  // A successful insertion performs exactly one fixup after the insert.
  StringBox success;
  char escaped[] = "^^entry";
  success.stringList_.add_result_ = 0;
  events.clear();
  REQUIRE(success.add_special(escaped) == 0, 14);
  REQUIRE(success.stringList_.pending_text_ == escaped + 2 &&
          success.stringList_.field_20_ == 2 &&
          success.stringList_.last_id_ == 0 &&
          success.stringList_.add_calls_ == 1, 15);
  REQUIRE(success.fixup_calls_ == 1 && events.size() == 2 &&
          events[0] == "insert" && events[1] == "fixup", 16);

  // Direct callee checks cover triple-caret parsing and normalized result.
  StringList list;
  char triple[] = "^^^entry";
  list.add_result_ = 3;
  events.clear();
  REQUIRE(list.add_special(triple) == 1, 17);
  REQUIRE(list.pending_text_ == triple + 3 && list.field_20_ == 2 &&
          list.last_id_ == 0 && list.add_calls_ == 1 &&
          events.size() == 1 && events[0] == "insert", 18);
  return 0;
}
'''


def actual_bodies(damage):
    source = (ROOT / "src/stringbox.cpp").read_text()
    list_source = (ROOT / "src/stringlist.cpp").read_text()
    box = extract_definition(source, "int StringBox::add_special(char *text) {")
    string_list = extract_definition(list_source, "int StringList::add_special(char *text) {")
    if damage:
        old = "if (result != 0) {\n        return result;\n    }"
        assert old in box
        box = box.replace(old, "if (result != 0) {\n        return 0;\n    }", 1)
    return string_list, box


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (False, True))
def test_actual_stringbox_and_stringlist_bodies(compiler_name, damage, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip("host compiler unavailable")
    string_list, box = actual_bodies(damage)
    source = tmp_path / "stringbox_insertion.cpp"
    source.write_text(HARNESS + "\n" + string_list + "\n" + box + "\n" + MAIN)
    executable = tmp_path / "stringbox_insertion"
    subprocess.run([compiler, "-std=c++11", str(source), "-o", str(executable)], check=True)
    result = subprocess.run([str(executable)])
    assert result.returncode == (10 if damage else 0)
