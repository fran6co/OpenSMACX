from pathlib import Path
import resource
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


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


def actual_fragments():
    win_source = (ROOT / "src/win.cpp").read_text()
    win_init = extract_function(
        win_source,
        "int Win::init(int x, int y, int width, int height, char * caption,")
    prefix_start = win_init.index("    if (border != 0) {")
    prefix_end = win_init.index("    if ((flags & 0xF0000000) != 0)", prefix_start)
    prefix = win_init[prefix_start:prefix_end]
    assert prefix.count("new Buffer()") == 4
    assert "operator new(0x588U)" not in prefix
    assert "new (raw) Buffer()" not in prefix
    assert "border->caption_height" in prefix
    assert "border->border_thickness" in prefix
    assert "border->bottom_border_thickness" in prefix
    assert "reinterpret_cast<int *>(border)" not in prefix

    basepop_source = (ROOT / "src/basepop.cpp").read_text()
    basepop_exec = extract_function(
        basepop_source, "int BasePop::exec(int flags, int (__cdecl *callback)())")
    local = "        BorderSizing sizing = {caption_height_, border_thickness_,\n" \
            "                               bottom_border_thickness_};"
    assert local in basepop_exec
    call_start = basepop_exec.index("if (GraphicWin::init(")
    call_end = basepop_exec.index(") != 0", call_start)
    init_call = basepop_exec[call_start:call_end]
    assert "&sizing" in init_call
    assert "reinterpret_cast<BorderSizing *>(sizing)" not in init_call

    win_header = (ROOT / "src/win.h").read_text()
    border_decl = "struct BorderSizing {\n    int caption_height;\n" \
                  "    int border_thickness;\n" \
                  "    int bottom_border_thickness;\n};"
    assert border_decl in win_header
    return prefix, local


def projected(mutant=None):
    prefix, border_local = actual_fragments()
    if mutant == "placement-allocation":
        prefix = prefix.replace(
            "this->buffer1_ = new Buffer();",
            "void *raw = ::operator new(sizeof(Buffer));\n"
            "        this->buffer1_ = (raw != 0) ? new (raw) Buffer() : 0;", 1)
        assert "new (raw) Buffer()" in prefix
    elif mutant == "border-field-order":
        border_local = border_local.replace(
            "{caption_height_, border_thickness_,\n"
            "                               bottom_border_thickness_};",
            "{border_thickness_, caption_height_,\n"
            "                               bottom_border_thickness_};", 1)
        assert border_local != actual_fragments()[1]
    elif mutant == "win-field-read":
        prefix = prefix.replace("border->bottom_border_thickness",
                                "border->border_thickness", 1)
        assert "border->bottom_border_thickness" not in prefix

    fixture = (ROOT / "recovery/tests/fixtures/"
               "win_buffer_lifetime_owner_oct10.cpp").read_text()
    fixture = fixture.replace("// ACTUAL_WIN_INIT_PREFIX", prefix)
    fixture = fixture.replace("// ACTUAL_BORDER_LOCAL", border_local)
    return fixture


def compile_and_run(tmp_path, compiler, source):
    source_path = tmp_path / "control.cpp"
    source_path.write_text(source)
    binary = tmp_path / "control"
    command = [compiler, "-std=c++17", "-fcheck-new", "-Wall", "-Wextra",
               "-Werror"]
    if Path(compiler).name == "clang++":
        command.append("-Wno-new-returns-null")
    command.extend([str(source_path), "-o", str(binary)])
    subprocess.run(command, check=True, capture_output=True)

    def disable_core_dumps():
        resource.setrlimit(resource.RLIMIT_CORE, (0, 0))

    return subprocess.run([str(binary)], capture_output=True,
                          preexec_fn=disable_core_dumps)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_win_buffer_lifetime_and_border_handoff(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    result = compile_and_run(tmp_path, compiler, projected())
    assert result.returncode == 0, result.stderr.decode(errors="replace")


def test_constructor_unwind_control_rejects_placement_allocation(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    result = compile_and_run(tmp_path, compiler,
                            projected(mutant="placement-allocation"))
    assert result.returncode != 0, "constructor-unwind mutant must leak and fail witness"


def test_named_border_control_rejects_wrong_member(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    result = compile_and_run(tmp_path, compiler,
                            projected(mutant="border-field-order"))
    assert result.returncode != 0, "wrong BorderSizing field must fail witness"


def test_win_init_border_control_rejects_wrong_field_read(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    result = compile_and_run(tmp_path, compiler,
                            projected(mutant="win-field-read"))
    assert result.returncode != 0, "wrong Win::init field read must fail witness"
