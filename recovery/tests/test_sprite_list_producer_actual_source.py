"""Compile the actual source-owned SpriteBox producer bodies as C++98."""
from pathlib import Path
import os
import subprocess
import tempfile

import pytest


HERE = Path(__file__).resolve().parent
FIXTURE = HERE / "sprite_list_producer_control.cpp"


def repo_root():
    configured = os.environ.get("OPENSMACX_TEST_SOURCE_ROOT")
    if configured:
        candidate = Path(configured).resolve()
        if (candidate / "src/spritebox.cpp").exists():
            return candidate
        raise AssertionError("OPENSMACX_TEST_SOURCE_ROOT lacks src/spritebox.cpp")
    for candidate in (HERE, *HERE.parents):
        if (candidate / "src/spritebox.cpp").exists():
            return candidate
    raise AssertionError("cannot locate OpenSMACX source root")


def extract_definition(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    quote = None
    escaped = False
    line_comment = block_comment = False
    index = brace
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if line_comment:
            if char == "\n":
                line_comment = False
            index += 1
            continue
        if block_comment:
            if char == "*" and nxt == "/":
                block_comment = False
                index += 2
                continue
            index += 1
            continue
        if quote:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == quote:
                quote = None
            index += 1
            continue
        if char == "/" and nxt == "/":
            line_comment = True
            index += 2
            continue
        if char == "/" and nxt == "*":
            block_comment = True
            index += 2
            continue
        if char in ("'", '"'):
            quote = char
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
        index += 1
    raise AssertionError(f"unterminated product definition: {signature}")


def owner_bodies():
    root = repo_root()
    sprite_source = (root / "src/spritebox.cpp").read_text()
    string_source = (root / "src/stringstruct.cpp").read_text()
    return "\n\n".join((
        extract_definition(sprite_source, "template<class Node>"),
        extract_definition(string_source, "int StringStruct::on_insert(void *)"),
        extract_definition(string_source, "void StringStruct::on_remove(void *)"),
        extract_definition(sprite_source, "int SpriteList::on_insert(void *raw_payload)"),
        extract_definition(sprite_source, "void SpriteList::on_remove(void *raw_payload)"),
        extract_definition(sprite_source, "int SpriteList::add(int id)"),
        extract_definition(sprite_source, "int SpriteBox::sprite(Sprite *row_sprite, char *text, int id)"),
    )) + "\n"


def compile_and_run(bodies, expected_success=True):
    fixture = FIXTURE.read_text()
    anchor = "/* PRODUCT OWNER BODIES */"
    assert fixture.count(anchor) == 1
    with tempfile.TemporaryDirectory(prefix="spritebox-owner-source-") as directory:
        cpp = Path(directory) / "fixture.cpp"
        exe = Path(directory) / "fixture"
        cpp.write_text(fixture.replace(anchor, bodies))
        built = subprocess.run(
            ["c++", "-std=c++98", "-O2", "-Wall", "-Wextra",
             "-Werror=return-type", str(cpp), "-o", str(exe)],
            capture_output=True, text=True)
        assert built.returncode == 0, built.stderr
        run = subprocess.run([str(exe)], capture_output=True, text=True)
        assert (run.returncode == 0) is expected_success, run.stderr


def test_actual_owner_and_producer_bodies_preserve_partial_failure():
    source = owner_bodies()
    assert "if (row_sprite == 0) return 3;" in source
    assert "if (Dialog::item_list_.add(id) != 0) return 4;" in source
    assert "if (list_.add(id) != 0) return 4;" in source
    compile_and_run(source)


def test_actual_fill_entry_mutant_is_detected():
    source = owner_bodies()
    original = "payload->sprite_ = pending_sprite_;"
    mutant = "payload->sprite_ = 0;"
    assert original in source
    compile_and_run(source.replace(original, mutant, 1), expected_success=False)


def test_actual_producer_rollback_mutant_is_detected():
    source = owner_bodies()
    original = "if (Dialog::item_list_.add(id) != 0) return 4;"
    mutant = "if (Dialog::item_list_.add(id) != 0) { list_.entry_count_ = 0; return 4; }"
    assert original in source
    compile_and_run(source.replace(original, mutant, 1), expected_success=False)
