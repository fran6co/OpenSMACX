"""Actual-source controls for the conditional Menu::UNK1 entry cleanup."""

from pathlib import Path
import subprocess
import tempfile

import pytest


def _function(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for position in range(opening, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[start : position + 1]
    raise AssertionError(f"unclosed {signature}")


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_menu_unk1_conditional_pointer_cleanup_actual_source(compiler):
    source = (Path(__file__).resolve().parents[2] / "src/menu.cpp").read_text()
    body = _function(source, "void Menu::UNK1()")
    prefix = r"""
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <new>

template<class T> struct Slot {
  T *value;
  int writes;
  Slot() : value(nullptr), writes(0) {}
  operator T *() const { return value; }
  Slot &operator=(T *next) { value = next; ++writes; return *this; }
};

struct PullDown;
struct Entry {
  int id;
  Slot<char> text;
  unsigned char flags;
  unsigned char padding[3];
  Slot<char> mnemonic;
  Slot<PullDown> pull_down;
};
struct Menu {
  typedef void (*MenuProc)(int);
  MenuProc proc_;
  int count_;
  unsigned int field_A1C_;
  unsigned int field_A20_;
  int field_A24_;
  unsigned int field_A28_;
  Entry entries_[15];
  void UNK1();
};

static Menu *observed_menu;
static int delete_calls;
static int freed_texts;
static bool initial_text[15];
static bool initial_pull[15];
static char initial_mnemonic[15];
struct PullDown {
  virtual ~PullDown() {
    assert(observed_menu != nullptr);
    int own_row = -1;
    for (int i = 0; i < 15; ++i)
      if (observed_menu->entries_[i].pull_down.value == this) own_row = i;
    assert(own_row >= 0);
    for (int i = 0; i < 15; ++i) {
      const Entry &entry = observed_menu->entries_[i];
      if (i <= own_row) {
        assert(entry.text.value == nullptr);
        assert(entry.id == -1 && entry.flags == 0 && entry.mnemonic.value == nullptr);
        if (i < own_row) assert(entry.pull_down.value == nullptr);
        else assert(entry.pull_down.value == this);
      } else {
        assert((entry.text.value != nullptr) == initial_text[i]);
        assert(entry.id == 100 + i);
        assert(entry.flags == static_cast<unsigned char>(i + 1));
        assert(entry.mnemonic.value == &initial_mnemonic[i]);
        assert((entry.pull_down.value != nullptr) == initial_pull[i]);
      }
      assert(entry.padding[0] == 0xA && entry.padding[1] == 0xB && entry.padding[2] == 0xC);
    }
    ++delete_calls;
  }
};
static void __attribute__((unused)) tracked_free(char *text) {
  assert(text != nullptr);
  ++freed_texts;
  std::free(text);
}
#define free tracked_free
"""
    checks = r"""
static void inert_callback(int) {}

int main() {
  Menu menu = {};
  observed_menu = &menu;
  menu.proc_ = &inert_callback;
  menu.count_ = 15;
  menu.field_A1C_ = 0xA1C;
  menu.field_A20_ = 0xA20;
  menu.field_A24_ = 14;
  menu.field_A28_ = 0xA28;
  for (int i = 0; i < 15; ++i) {
    initial_text[i] = (i % 2) == 0;
    initial_pull[i] = (i % 3) != 0;
    initial_mnemonic[i] = static_cast<char>(i + 1);
    menu.entries_[i].id = 100 + i;
    menu.entries_[i].flags = static_cast<unsigned char>(i + 1);
    menu.entries_[i].padding[0] = 0xA;
    menu.entries_[i].padding[1] = 0xB;
    menu.entries_[i].padding[2] = 0xC;
    if (initial_text[i]) {
      menu.entries_[i].text.value = static_cast<char *>(std::malloc(8));
      assert(menu.entries_[i].text.value != nullptr);
    }
    menu.entries_[i].mnemonic.value = &initial_mnemonic[i];
    if (initial_pull[i]) menu.entries_[i].pull_down.value = new PullDown;
  }

  menu.UNK1();

  assert(delete_calls == 10);
  assert(freed_texts == 8);
  for (int i = 0; i < 15; ++i) {
    const Entry &entry = menu.entries_[i];
    assert(entry.text.value == nullptr);
    assert(entry.pull_down.value == nullptr);
    assert(entry.text.writes == (initial_text[i] ? 1 : 0));
    assert(entry.pull_down.writes == (initial_pull[i] ? 1 : 0));
    assert(entry.id == -1);
    assert(entry.flags == 0);
    assert(entry.mnemonic.value == nullptr);
    assert(entry.mnemonic.writes == 1);
    assert(entry.padding[0] == 0xA && entry.padding[1] == 0xB && entry.padding[2] == 0xC);
  }
  assert(menu.proc_ == nullptr);
  assert(menu.count_ == 0);
  assert(menu.field_A20_ == 0);
  assert(menu.field_A24_ == -1);
  assert(menu.field_A1C_ == 0xA1C);
  assert(menu.field_A28_ == 0xA28);
}
"""
    program = prefix + "\n" + body + "\n" + checks
    with tempfile.TemporaryDirectory(prefix="menu-unk1-host-") as temp:
        binary = str(Path(temp) / "control")
        built = subprocess.run(
            [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", "-x", "c++", "-", "-o", binary],
            input=program,
            text=True,
            capture_output=True,
            timeout=30,
        )
        if built.returncode:
            raise AssertionError(built.stderr)
        result = subprocess.run([binary], text=True, capture_output=True, timeout=30)
        if result.returncode:
            raise AssertionError(result.stdout + result.stderr)

        mutations = {
            "unconditional text clear": (
                "            entries_[i].text = nullptr;\n        }",
                "        }\n        entries_[i].text = nullptr;",
            ),
            "unconditional pull-down clear": (
                "            entries_[i].pull_down = nullptr;\n        }",
                "        }\n        entries_[i].pull_down = nullptr;",
            ),
            "skip text release": ("free(entries_[i].text);", "((void)0);"),
            "skip pull-down destruction": (
                "delete entries_[i].pull_down;",
                "((void)0);",
            ),
            "skip fifteenth row": ("i < 15", "i < 14"),
            "reset fields after destruction": (
                "        entries_[i].id = -1;\n"
                "        entries_[i].flags = 0;\n"
                "        entries_[i].mnemonic = nullptr;\n"
                "        if (entries_[i].pull_down != nullptr) {\n"
                "            delete entries_[i].pull_down;\n"
                "            entries_[i].pull_down = nullptr;\n"
                "        }",
                "        if (entries_[i].pull_down != nullptr) {\n"
                "            delete entries_[i].pull_down;\n"
                "            entries_[i].pull_down = nullptr;\n"
                "        }\n"
                "        entries_[i].id = -1;\n"
                "        entries_[i].flags = 0;\n"
                "        entries_[i].mnemonic = nullptr;",
            ),
        }
        for name, (before, after) in mutations.items():
            mutant_body = body.replace(before, after, 1)
            assert mutant_body != body, f"{name} mutation did not apply"
            mutant_program = prefix + "\n" + mutant_body + "\n" + checks
            mutated_build = subprocess.run(
                [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", "-x", "c++", "-", "-o", binary],
                input=mutant_program,
                text=True,
                capture_output=True,
                timeout=30,
            )
            assert mutated_build.returncode == 0, f"{name} did not compile:\n{mutated_build.stderr}"
            mutated_result = subprocess.run([binary], text=True, capture_output=True, timeout=30)
            assert mutated_result.returncode != 0, f"{name} escaped runtime controls"
