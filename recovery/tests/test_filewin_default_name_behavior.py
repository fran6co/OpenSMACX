import pathlib
import shutil
import subprocess
import pytest


ROOT = pathlib.Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "filewin.cpp"


def extract_body(text, signature):
    start = text.index(signature)
    brace = text.index("{", start)
    depth = 0
    for cursor in range(brace, len(text)):
        if text[cursor] == "{":
            depth += 1
        elif text[cursor] == "}":
            depth -= 1
            if depth == 0:
                return text[start:cursor + 1]
    raise AssertionError("unbalanced method body")


def test_projected_filewin_setter_uses_actual_body(tmp_path):
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    if not compilers:
        pytest.skip("neither g++ nor clang++ is installed")
    body = extract_body(SOURCE.read_text(), "int FileWin::set_def_name(char *name)")
    harness = r'''
#include <assert.h>
#include <string.h>
class FileWin {
 public:
  char file_name_[260];
  char directory_[0x107];
  int set_def_name(char *name);
};
''' + body + r'''
int main() {
  FileWin f = {};
  strcpy(f.directory_, "preserve");
  assert(f.set_def_name(0) == 0);
  assert(f.file_name_[0] == 0 && strcmp(f.directory_, "preserve") == 0);
  char p[] = "C:\\maps\\slot.MP";
  assert(f.set_def_name(p) == 0);
  assert(strcmp(f.file_name_, "slot.MP") == 0);
  assert(strcmp(f.directory_, "C:\\maps\\") == 0);
  char rooted[] = "\\root.MP";
  assert(f.set_def_name(rooted) == 0);
  assert(strcmp(f.file_name_, "\\root.MP") == 0 && f.directory_[0] == 0);
  char long_name[260]; memset(long_name, 'n', sizeof(long_name) - 1);
  long_name[sizeof(long_name) - 1] = 0;
  assert(f.set_def_name(long_name) == 0);
  assert(strlen(f.file_name_) == 259);
  strcpy(f.file_name_, "old"); strcpy(f.directory_, "old-dir");
  char oversized[262]; memset(oversized, 'z', sizeof(oversized) - 1);
  oversized[sizeof(oversized) - 1] = 0;
  assert(f.set_def_name(oversized) == 3);
  assert(strcmp(f.file_name_, "old") == 0 && strcmp(f.directory_, "old-dir") == 0);
}
'''
    source = tmp_path / "control.cpp"
    source.write_text(harness)
    for compiler in compilers:
        binary = tmp_path / ("control-" + pathlib.Path(compiler).name)
        subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                        str(source), "-o", str(binary)], check=True, timeout=30)
        subprocess.run([str(binary)], check=True, timeout=30)
