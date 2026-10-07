"""Actual-source host controls for EditGroup::close.

The exact current method body, class inheritance clause and owned member
declarations are compiled into a projected host fixture. The fixture models
only exercised members/lifetime behavior, not the complete VC6 ABI/layout.
"""
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = next(p for p in Path(__file__).resolve().parents
            if (p / "src/IMAGE_PIN").is_file())


def extract(text, signature):
    assert text.count(signature) == 1, signature
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for i in range(opening, len(text)):
        if text[i] == "{": depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return text[start:i + 1]
    raise AssertionError("unbalanced source body: " + signature)


def source_parts():
    header = (ROOT / "src/editgroup.h").read_text(encoding="utf-8")
    source = (ROOT / "src/editgroup.cpp").read_text(encoding="utf-8")
    editbox = (ROOT / "src/editbox.h").read_text(encoding="utf-8")
    dialog = (ROOT / "src/dialog.h").read_text(encoding="utf-8")
    inheritance = "class EditGroup : public virtual GraphicWin, public virtual Dialog {"
    assert header.count(inheritance) == 1
    for declaration in ("EditBox *boxes_[10];", "char *text_buffers_[10];",
                        "uint32_t limits_[10];", "uint32_t field_84_;"):
        assert header.count(declaration) == 1
    assert "class EditBox : public GraphicWin" in editbox
    assert "void close();" in dialog
    fields = "\n".join("  " + declaration for declaration in
                      ("EditBox *boxes_[10];", "char *text_buffers_[10];",
                       "uint32_t limits_[10];", "uint32_t field_84_;"))
    body = extract(source, "void EditGroup::close()")
    return inheritance, fields, body


PREAMBLE = r'''#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
typedef unsigned int uint32_t;
static void event(int value);
struct GraphicWin {
  virtual ~GraphicWin() {}
  virtual void close() { event(400); }
};
struct Dialog {
  virtual ~Dialog() {}
  void close();
};
struct EditBox : GraphicWin { virtual ~EditBox() {} };
class EditGroup;
static void before_free(EditGroup *, int, char *);
static int derived_dtor_count;
struct DerivedBox : EditBox {
  EditGroup *owner; int id; char *replacement;
  DerivedBox(EditGroup *g, int i, char *r) : owner(g), id(i), replacement(r) {}
  virtual ~DerivedBox();
};
'''


def fixture(inheritance, fields, body, mutant=None):
    # The class declaration text and member fields come directly from the
    # current private declarations; only test accessors are added afterward.
    access = r'''
public:
  EditGroup() : field_84_(0) {
    for (int i=0;i<10;++i) { boxes_[i]=0; text_buffers_[i]=0; limits_[i]=0; }
  }
  void close();
  void test_box(int i, EditBox *p) { boxes_[i] = p; }
  EditBox *test_box(int i) const { return boxes_[i]; }
  void test_text(int i, char *p) { text_buffers_[i] = p; }
  char *test_text(int i) const { return text_buffers_[i]; }
  void test_limit(int i, uint32_t n) { limits_[i] = n; }
  uint32_t test_limit(int i) const { return limits_[i]; }
  void test_field(uint32_t n) { field_84_ = n; }
  uint32_t test_field() const { return field_84_; }
};
static int events[128], event_count;
static EditGroup *active_group;
static int active_index;
static char *expected_free;
static char *post_free_replacement;
static char *independent_text;
static uint32_t EditGroupDefault = 0x87654321u;
// The host language mode is intentionally C++98; map the production null
// token to its C++98 spelling for this isolated control translation unit.
#define nullptr 0
static void event(int v) { events[event_count++] = v; }
static void before_free(EditGroup *g, int i, char *replacement) {
  active_group = g; active_index = i; expected_free = replacement;
  g->test_text(i, replacement);
}
DerivedBox::~DerivedBox() {
  ++derived_dtor_count; event(100 + id);
  before_free(owner, id, replacement);
  if (id == 9) EditGroupDefault = 0xABCDEF01u;
}
static void observed_free(void *p) {
  int index = -1;
  for (int i=0;i<10;++i) {
    if (active_group->test_text(i) == p) { assert(index == -1); index = i; }
  }
  assert(index >= 0);
  if (index == 3) assert(p == independent_text);
  else { assert(index == active_index); assert(p == expected_free); }
  active_index = index;
  event(200 + active_index);
  active_group->test_text(active_index, post_free_replacement);
}
void Dialog::close() {
  assert(active_group->test_field() == EditGroupDefault);
  event(300);
}
#define free observed_free
'''
    if mutant == "wrong_order":
        body = body.replace("            delete boxes_[index];",
                            "            free(text_buffers_[index]);\n            delete boxes_[index];", 1)
    elif mutant == "omit_last":
        body = body.replace("index < 10", "index < 9", 1)
    elif mutant == "wrong_limit":
        body = body.replace("limits_[index] = 10;", "limits_[index] = 0;", 1)
    elif mutant == "wrong_close":
        body = body.replace("Dialog::close();", "GraphicWin::close();", 1)
    elif mutant == "nonvirtual_delete":
        body = body.replace("delete boxes_[index];",
                            "boxes_[index]->EditBox::~EditBox();", 1)
    main = r'''int main() {
  EditGroup g; active_group = &g;
  char initial[10][4], changed[10][4], after_free[10][4];
  for (int i=0;i<10;++i) {
    initial[i][0]='a'; changed[i][0]='b'; after_free[i][0]='c';
    if (i != 3) {
      g.test_box(i, new DerivedBox(&g, i, changed[i]));
    }
    g.test_text(i, initial[i]);
    g.test_limit(i, i == 9 ? 99 : 77);
  }
  g.test_limit(3, 33); g.test_field(7);
  independent_text = initial[3];
  post_free_replacement = after_free[0];
  g.close();
  assert(event_count == 20); // nine destructors, ten frees and Dialog close
  int at=0;
  for (int i=0;i<10;++i) {
    if (i != 3) {
      assert(events[at++] == 100+i);
      assert(g.test_box(i) == 0 && g.test_limit(i) == 10);
      assert(g.test_text(i) == 0); // free callback mutation is cleared afterward
    } else {
      assert(g.test_limit(i) == 33 && g.test_text(i) == 0);
    }
    assert(events[at++] == 200+i);
  }
  assert(events[at++] == 300 && at == event_count);
  assert(derived_dtor_count == 9 && g.test_field() == EditGroupDefault);
  return 0;
}'''
    return (PREAMBLE + inheritance + "\nprivate:\n" + fields + access +
            body + "\n" + main)


def compile_run(compiler, code, expected):
    with tempfile.TemporaryDirectory(prefix="editgroup-close-host-") as td:
        cpp, exe = Path(td) / "control.cpp", Path(td) / "control"
        cpp.write_text(code, encoding="utf-8")
        built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra",
                                "-Werror", str(cpp), "-o", str(exe)],
                               text=True, capture_output=True, timeout=10)
        assert built.returncode == 0, built.stdout + built.stderr
        run = subprocess.run([str(exe)], text=True, capture_output=True,
                             timeout=10)
        assert (run.returncode == 0) == expected, run.stdout + run.stderr


def test_actual_body_member_declarations_and_host_behavior():
    inheritance, fields, body = source_parts()
    assert "reinterpret_cast" not in body and "CloseVCall" not in body
    assert "delete boxes_[index]" in body and "free(text_buffers_[index])" in body
    assert body.index("delete boxes_[index]") < body.index("free(text_buffers_[index])")
    compilers = (shutil.which("g++"), shutil.which("clang++"))
    assert all(compilers), "GCC and Clang are required"
    for compiler in compilers:
        compile_run(compiler, fixture(inheritance, fields, body), True)


def test_mutants_reject_order_last_slot_limit_close_and_virtual_delete():
    inheritance, fields, body = source_parts()
    for compiler_name in ("g++", "clang++"):
        compiler = shutil.which(compiler_name)
        assert compiler, compiler_name + " required"
        for mutation in ("wrong_order", "omit_last", "wrong_limit",
                         "wrong_close", "nonvirtual_delete"):
            compile_run(compiler, fixture(inheritance, fields, body, mutation), False)
