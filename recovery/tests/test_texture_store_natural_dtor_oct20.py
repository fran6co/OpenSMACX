"""Actual-source C++98 checks for TextureStore's natural member stores."""

import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest

from recovery.tests.test_pact_ascent import ROOT, definition


def _harness(body):
    return r'''struct TextureStore {
public:
    unsigned int flags_;
    int next_record_;
    unsigned int member_canary_;
    void apply_destructor_body();
};

BODY

struct Frame {
    unsigned int before_canary;
    TextureStore store;
    unsigned int after_canary;
};

int main() {
    Frame frame;
    frame.before_canary = 0x13579BDF;
    frame.after_canary = 0x2468ACE0;
    frame.store.flags_ = 0x11223344;
    frame.store.next_record_ = 0x556677;
    frame.store.member_canary_ = 0x76543210;
    frame.store.apply_destructor_body();

    if (frame.store.flags_ != 3 || frame.store.next_record_ != 0 ||
        frame.store.member_canary_ != 0x76543210)
        return 1;
    if (frame.before_canary != 0x13579BDF || frame.after_canary != 0x2468ACE0)
        return 2;
    return 0;
}
'''.replace("BODY", body, 1)


def _run(tmp_path, compiler, source, *, succeeds):
    cpp = tmp_path / "texture_store_dtor.cpp"
    executable = tmp_path / "texture_store_dtor"
    cpp.write_text(source)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(cpp), "-o", str(executable)],
        capture_output=True, text=True, timeout=45)
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=15)
    if succeeds:
        assert result.returncode == 0, result.stdout + result.stderr
    else:
        assert result.returncode != 0, "actual-source destructor mutation escaped the harness"


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_texture_store_natural_destructor_and_independent_damage_controls(compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + " is unavailable")
    source = (ROOT / "src/texture.cpp").read_text(encoding="utf-8")
    actual = definition(source, r"TextureStore::~TextureStore\(\)\s*")
    # Rename only the extracted signature: inspecting these effects while the
    # fixture is live avoids post-destructor lifetime UB. This is body-effect
    # coverage, not evidence for destructor ABI, lifetime, or native codegen.
    actual = actual.replace("TextureStore::~TextureStore()", "void TextureStore::apply_destructor_body()", 1)
    with tempfile.TemporaryDirectory(prefix="texture-store-natural-dtor-") as directory:
        tmp_path = Path(directory)
        _run(tmp_path, compiler, _harness(actual), succeeds=True)

        damaged_flags = actual.replace("flags_ = 3;", "flags_ = 7;", 1)
        assert damaged_flags != actual and damaged_flags.count("flags_ = 7;") == 1
        _run(tmp_path, compiler, _harness(damaged_flags), succeeds=False)

        damaged_cursor = actual.replace("next_record_ = 0;", "next_record_ = 1;", 1)
        assert damaged_cursor != actual and damaged_cursor.count("next_record_ = 1;") == 1
        _run(tmp_path, compiler, _harness(damaged_cursor), succeeds=False)
