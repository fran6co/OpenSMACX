import pathlib
import shutil
import subprocess
import tempfile

import pytest


PROJECT_ROOT = pathlib.Path(__file__).parents[2]


def extract_definition(source: str, signature: str) -> str:
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for index in range(brace, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
    raise AssertionError(f"unterminated definition: {signature}")


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_filewin_method_and_alphasave_forwarder(compiler):
    filewin = (PROJECT_ROOT / "src/jackal/filewin.cpp").read_text()
    alphasave = (PROJECT_ROOT / "src/alphasave.cpp").read_text()
    body = extract_definition(filewin, "int FileWin::add_def_ext(")
    wrapper = extract_definition(alphasave, "int AlphaSave::add_default_extension(")
    fixture = r"""
#include <assert.h>
#include <string.h>
class FileWin {
public:
    int add_def_ext(char *extension);
    int extension_count_;
    char extension_filters_[4][5];
};
class AlphaSave {
public:
    int add_default_extension(char *extension);
    FileWin file_win_;
};
""" + body + "\n" + wrapper + r"""
int main() {
    AlphaSave dialog = {};
    strcpy(dialog.file_win_.extension_filters_[0], "OLD");
    assert(dialog.add_default_extension(0) == 0x10);
    assert(dialog.file_win_.extension_count_ == 0);
    assert(dialog.add_default_extension((char *)"TOOLONG") == 3);
    assert(dialog.file_win_.extension_count_ == 0);
    assert(dialog.file_win_.extension_filters_[0][0] == 0);
    assert(dialog.add_default_extension((char *)"SAV") == 0);
    assert(dialog.file_win_.extension_count_ == 1);
    assert(strcmp(dialog.file_win_.extension_filters_[0], "SAV") == 0);
    assert(dialog.add_default_extension((char *)"SC") == 0);
    assert(dialog.add_default_extension((char *)"MAP") == 0);
    assert(dialog.add_default_extension((char *)"MP") == 0);
    assert(dialog.file_win_.extension_count_ == 4);
    assert(dialog.add_default_extension(0) == 1);
    assert(dialog.add_default_extension((char *)"SAV") == 1);
    return 0;
}
"""
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} unavailable")
    with tempfile.TemporaryDirectory(prefix="filewin-add-ext-control-") as temp:
        source = pathlib.Path(temp) / "actual_bodies.cpp"
        binary = pathlib.Path(temp) / "control"
        source.write_text(fixture)
        compiled = subprocess.run(
            [executable, "-std=c++11", "-Wall", "-Wextra", str(source), "-o", str(binary)],
            capture_output=True,
            text=True,
            timeout=30,
            check=False,
        )
        assert compiled.returncode == 0, compiled.stderr
        subprocess.run([str(binary)], timeout=30, check=True)
