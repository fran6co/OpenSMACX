"""Actual-source controls for UV2Player DirectDraw close/play recovery."""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest
from owner_transfer_test_support import assert_current_owner, assert_leaf_collection_retired

HERE = Path(__file__).resolve()
ROOT = Path(os.environ.get("OPENSMACX_ROOT", str(HERE.parents[2]))).resolve()


def mask_comments(text):
    chars = list(text)
    mode = "code"
    i = 0
    while i < len(text):
        c = text[i]
        n = text[i + 1] if i + 1 < len(text) else ""
        if mode == "code":
            if c == "/" and n == "/":
                chars[i] = chars[i + 1] = " "
                i += 2
                mode = "line"
                continue
            if c == "/" and n == "*":
                chars[i] = chars[i + 1] = " "
                i += 2
                mode = "block"
                continue
            if c == '"':
                chars[i] = " "
                mode = "string"
            elif c == "'":
                chars[i] = " "
                mode = "char"
        elif mode == "line":
            if c == "\n":
                mode = "code"
            else:
                chars[i] = " "
        elif mode == "block":
            chars[i] = " "
            if c == "*" and n == "/":
                chars[i + 1] = " "
                i += 2
                mode = "code"
                continue
        else:
            chars[i] = " "
            if c == "\\" and i + 1 < len(text):
                chars[i + 1] = " "
                i += 2
                continue
            if (mode == "string" and c == '"') or (mode == "char" and c == "'"):
                mode = "code"
        i += 1
    return "".join(chars)


def definition(text, signature):
    code = mask_comments(text)
    match = re.search(signature + r"\s*\{", code)
    assert match, "missing production source definition: " + signature
    opening = code.index("{", match.start())
    depth = 0
    for end in range(opening, len(code)):
        depth += (code[end] == "{") - (code[end] == "}")
        if depth == 0:
            body = text[match.start():end + 1]
            assert "BODY GOES HERE" not in body
            return body
    raise AssertionError("unterminated method: " + signature)


def product_source():
    source = (ROOT / "src/uv2player.cpp").read_text()
    header = (ROOT / "src/uv2player.h").read_text()
    assert_leaf_collection_retired(ROOT)
    assert_current_owner(ROOT, 0x004BF400, "src/uv2player.cpp")
    assert_current_owner(ROOT, 0x004BF4B0, "src/uv2player.cpp")

    code = mask_comments(header)
    for field_decl in (
        r"IDirectDraw\s*\*\s*direct_draw_\s*;",
        r"IDirectDraw2\s*\*\s*direct_draw2_\s*;",
        r"IDirectDrawSurface2\s*\*\s*primary_surface_\s*;",
        r"IDirectDrawSurface2\s*\*\s*back_buffer_surface_\s*;",
        r"IDirectDrawClipper\s*\*\s*clipper_\s*;",
    ):
        assert re.search(field_decl, code), "missing typed member: " + field_decl
    assert re.search(r"int\s+play\(int\s+width,\s*int\s+height\)\s*;", code)
    assert "ComVCall" not in mask_comments(source)

    active = re.search(
        r"^UV2Player\s*\*\s*UV2PlayerActive\s*;\s*$",
        mask_comments(source), re.MULTILINE)
    assert active, "expected the existing source-owned active-player object"
    product_globals = source[active.start():active.end()]
    close = definition(source, r"void\s+UV2Player::close\(\)")
    play = definition(source, r"int\s+UV2Player::play\(int\s+width,\s*int\s+height\)")
    return product_globals, "\n\n".join((close, play))


def compile_and_run(tmp_path, product, scenario, damage=None):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    product_globals, product_methods = product
    if damage:
        old, new = damage
        assert old in product_methods
        product_methods = product_methods.replace(old, new, 1)
    template = HERE.with_name("uv2player_directdraw_harness.cpp").read_text()
    cpp = tmp_path / "uv2player_owner.cpp"
    exe = tmp_path / "uv2player_owner"
    cpp.write_text(template.replace("/* PRODUCT GLOBALS */", product_globals)
                         .replace("/* PRODUCT METHODS */", product_methods))
    command = [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
               "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
               str(cpp), "-o", str(exe)]
    subprocess.run(command, check=True, capture_output=True, text=True)
    env = dict(os.environ, ASAN_OPTIONS="detect_leaks=0",
               UBSAN_OPTIONS="halt_on_error=1")
    return subprocess.run([str(exe), scenario], capture_output=True, text=True,
                          timeout=15, env=env)


@pytest.mark.parametrize("scenario", [
    "guard", "create_failure", "cooperative_failure", "all_modes_fail",
    "attached_failure", "clipper_failure", "success", "close_disabled",
    "close_windowed_large", "close_without_device", "receiver_rebind",
])
def test_production_uv2player_methods(tmp_path, scenario):
    result = compile_and_run(tmp_path, product_source(), scenario)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("scenario,old,new", [
    ("success", "SetDisplayMode(width, height, 0x18, 0, 0)",
     "SetDisplayMode(width, height, 0x20, 0, 0)"),
    ("close_windowed_large", "primary_surface_->Release();", "direct_draw2_->Release();"),
    ("all_modes_fail", "close();\n        return 0x12;", "return 0x12;"),
    ("clipper_failure", "return 0x12;\n    }\n\n    DDBLTFX effects;",
     "close();\n        return 0x12;\n    }\n\n    DDBLTFX effects;"),
    ("receiver_rebind", "direct_draw2_->SetDisplayMode(width, height, 0x18, 0, 0)",
     "draw.draw2->SetDisplayMode(width, height, 0x18, 0, 0)"),
])
def test_damage_controls_fail(tmp_path, scenario, old, new):
    result = compile_and_run(tmp_path, product_source(), scenario, (old, new))
    assert result.returncode != 0, "deliberate native-behavior damage escaped"
