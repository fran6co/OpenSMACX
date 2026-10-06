"""The product Interlude key callback returns its handled result through Win."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def extract(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated function: " + signature)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_interlude_key_callback_releases_and_reports_handled(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    source = (ROOT / "src/interlude.cpp").read_text()
    body = extract(source, "int Interlude::on_key_down(int)")
    harness = r'''
#include <cassert>
struct Win {
    int modal_releases = 0;
    virtual int on_key_down(int) { return 0; }
    void release_modal() { ++modal_releases; }
};
struct Interlude : Win { int on_key_down(int) override; };
BODY
int main() {
    Interlude interlude;
    Win *window = &interlude;
    assert(window->on_key_down(0x1B) == 1);
    assert(interlude.modal_releases == 1);
}
'''.replace("BODY", body)
    cpp = tmp_path / "interlude-key.cpp"
    exe = tmp_path / "interlude-key"
    cpp.write_text(harness)
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(exe)], check=True, capture_output=True,
                   text=True, timeout=30)
    subprocess.run([str(exe)], check=True, capture_output=True, timeout=30)
