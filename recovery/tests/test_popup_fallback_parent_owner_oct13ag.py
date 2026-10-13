"""Scoped source/token and live-expression controls for BasePop's fallback parent.

The host projection checks the exact parent-selection declaration from the
production exec body. It does not establish native layout, callback ABI, or
whole-game behavior.
"""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

from test_popup_dimensions_background import ROOT


BASE_REVISION = "c90e2c838259d5b0cbbb4991a8ddd573a154133a"
SOURCE_FILES = ("src/basepop.h", "src/basepop.cpp", "src/popup.cpp")


def _definition(source, signature):
    start = source.find(signature)
    assert start >= 0, "missing production definition: " + signature
    opening = source.find("{", start)
    depth = 0
    for position in range(opening, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[start:position + 1]
    raise AssertionError("unterminated production definition: " + signature)


def test_three_source_files_match_only_authorized_identifier_and_cast_delta():
    for relative in SOURCE_FILES:
        original = subprocess.run(
            ["git", "show", f"{BASE_REVISION}:{relative}"],
            cwd=ROOT, check=True, capture_output=True,
        ).stdout
        authorized = original.replace(b"field_322C_", b"fallback_parent_window_")
        if relative == "src/basepop.cpp":
            old_cast = b"reinterpret_cast<Win *>(fallback_parent_window_)"
            assert authorized.count(old_cast) == 1
            authorized = authorized.replace(old_cast, b"fallback_parent_window_", 1)
        assert (ROOT / relative).read_bytes() == authorized, (
            f"unexpected content/history change in {relative}"
        )


def test_actual_exec_parent_selection_covers_attached_fallback_and_null(tmp_path):
    source = (ROOT / "src/basepop.cpp").read_text(encoding="utf-8")
    body = _definition(source, "int BasePop::exec(int flags, int (__cdecl *callback)())")
    matches = re.findall(
        r"(?m)^\s*Win \*const parent = external_window_ != 0\s*"
        r"\? external_window_\s*:\s*fallback_parent_window_;\s*$",
        body,
    )
    assert len(matches) == 1, "exec must select its external or fallback Win pointer once"
    selection = matches[0].strip()
    code = r'''#include <cassert>
struct Win { int marker; };
struct ParentSelection {
    Win *external_window_;
    Win *fallback_parent_window_;
    Win *selected() const {
        ACTUAL_SELECTION
        return parent;
    }
};
int main() {
    Win fallback = {11};
    Win external = {22};
    ParentSelection state = {0, &fallback};
    assert(state.selected() == &fallback);
    state.external_window_ = &external;
    assert(state.selected() == &external);
    state.external_window_ = 0;
    state.fallback_parent_window_ = 0;
    assert(state.selected() == 0);
}
'''.replace("ACTUAL_SELECTION", selection)
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    assert all(compilers), "both GCC and Clang are required for host controls"
    for compiler in compilers:
        with tempfile.TemporaryDirectory(prefix="popup-parent-selection-") as directory:
            cpp = Path(directory) / "control.cpp"
            exe = Path(directory) / "control"
            cpp.write_text(code, encoding="utf-8")
            built = subprocess.run(
                [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
                 str(cpp), "-o", str(exe)],
                capture_output=True, text=True, check=False,
            )
            assert built.returncode == 0, built.stdout + built.stderr
            result = subprocess.run([str(exe)], capture_output=True, text=True)
            assert result.returncode == 0, result.stdout + result.stderr


def test_parent_field_identity_and_popup_virtual_base_conversion_are_retained():
    header = (ROOT / "src/basepop.h").read_text(encoding="utf-8")
    basepop = (ROOT / "src/basepop.cpp").read_text(encoding="utf-8")
    popup = (ROOT / "src/popup.cpp").read_text(encoding="utf-8")
    assert len(re.findall(
        r"(?m)^\s*Win \*fallback_parent_window_;\s*$", header
    )) == 1
    assert "Win *BasePopDefaultField322C;" in basepop
    assert "fallback_parent_window_ = BasePopDefaultField322C;" in basepop
    assert "fallback_parent_window_ = static_cast<Win *>(static_cast<GraphicWin *>(&ConsoleGlobal));" in popup
    assert "reinterpret_cast<Win *>(fallback_parent_window_)" not in basepop
