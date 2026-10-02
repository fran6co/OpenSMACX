"""Compile and execute the complete candidate method extracted from overlay."""

from pathlib import Path
import shutil
import subprocess
import tempfile


ROOT = Path(__file__).parent
SOURCE = ROOT.parents[1] / "src" / "filewin.cpp"
FIXTURE = ROOT / "filewin_populate_directory_host_control.cpp"
MARKER = "/* EXTRACTED_POPULATE_DIRECTORY_METHOD */"
SIGNATURE = "int FileWin::populate_directory(char *directory)"


def extract_method(text: str) -> str:
    start = text.index(SIGNATURE)
    opening = text.index("{", start)
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[start : index + 1]
    raise AssertionError("unbalanced populate_directory body")


def test_extracted_full_method_with_gcc_and_clang():
    body = extract_method(SOURCE.read_text())
    assert "list_box_.has_dib_bits()" in body
    assert "FileWinInitSharedParameters.field_00_" in body
    assert "dialog.attach(" in body
    assert "dialog.attach_at_stored_position(" not in body
    assert "dialog.field_24_" in body and "dialog.field_28_" in body
    assert "FindClose(handle);" in body
    assert "list_box_.show(0);" in body
    fixture = FIXTURE.read_text()
    assert fixture.count(MARKER) == 1

    generated = fixture.replace(MARKER, body)
    failures = []
    compilers = [name for name in ("g++", "clang++") if shutil.which(name)]
    assert compilers, "no host compiler available for extracted-body control"
    for compiler in compilers:
        with tempfile.TemporaryDirectory(prefix="filewin-populate-body-") as temp:
            source_path = Path(temp) / "control.cpp"
            executable = Path(temp) / "control"
            source_path.write_text(generated)
            compile_result = subprocess.run(
                [
                    compiler,
                    "-std=c++98",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    str(source_path),
                    "-o",
                    str(executable),
                ],
                capture_output=True,
                text=True,
                timeout=30,
            )
            if compile_result.returncode != 0:
                failures.append(
                    compiler + " compile:\n" + compile_result.stdout
                    + compile_result.stderr
                )
                continue
            run_result = subprocess.run(
                [str(executable)], capture_output=True, text=True, timeout=30
            )
            if run_result.returncode != 0:
                failures.append(
                    compiler + " runtime:\n" + run_result.stdout
                    + run_result.stderr
                )
    assert not failures, "\n".join(failures)


from pathlib import Path


SOURCE_ROOT = Path(__file__).resolve().parents[2] / "src"


def body_after(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[start : index + 1]
    raise AssertionError("unbalanced callback body")


def test_draw_only_listbox_callback_family_is_void_and_typed():
    listbox_h = (SOURCE_ROOT / "listbox.h").read_text()
    messagewin_h = (SOURCE_ROOT / "messagewin.h").read_text()
    messagewin_cpp = (SOURCE_ROOT / "messagewin.cpp").read_text()
    designwin_cpp = (SOURCE_ROOT / "designwin.cpp").read_text()
    filewin_h = (SOURCE_ROOT / "filewin.h").read_text()
    filewin_cpp = (SOURCE_ROOT / "filewin.cpp").read_text()

    assert "typedef void (__cdecl *DrawCallback)" in listbox_h
    assert "typedef int (__cdecl *WriteCallback)" in listbox_h
    assert "typedef int (__cdecl *FillCallback)" in listbox_h
    assert "static void __cdecl listbox_draw" in messagewin_h
    draw_body = body_after(
        messagewin_cpp,
        "void __cdecl MessageWin::listbox_draw(GraphicWin *target",
    )
    assert "arrow.draw(" in draw_body
    assert "return arrow.draw" not in draw_body
    assert "set_callbacks(&hq_list_draw, 0, 0)" in designwin_cpp
    assert "workshop_list_draw_adapter" not in designwin_cpp
    assert "void __cdecl filebox_draw" in filewin_h
    assert "list_box_.set_callbacks(&filebox_draw, 0, 0)" in filewin_cpp
