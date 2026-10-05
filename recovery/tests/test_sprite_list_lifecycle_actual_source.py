"""Compile actual StringStruct/SpriteBox lifecycle bodies from product source.

The fixture contributes only host ABI types and dependency stubs; tested
method bodies are extracted verbatim from
src/stringstruct.cpp and src/spritebox.cpp on every run.
"""

from pathlib import Path
import os
import subprocess
import tempfile

import pytest


HERE = Path(__file__).resolve().parent
FIXTURE = HERE / "sprite_list_lifecycle_control.cpp"


def repo_root():
    configured = os.environ.get("OPENSMACX_TEST_SOURCE_ROOT")
    if configured:
        candidate = Path(configured).resolve()
        if (candidate / "src/stringstruct.cpp").exists():
            return candidate
        raise AssertionError("OPENSMACX_TEST_SOURCE_ROOT lacks src/stringstruct.cpp")
    for candidate in (HERE, *HERE.parents):
        if (candidate / "src/stringstruct.cpp").exists():
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


def product_bodies(string_source, sprite_source):
    signatures = (
        (string_source, "int StringStruct::on_insert(void *)"),
        (string_source, "void StringStruct::on_remove(void *)"),
        (string_source, "inline void clear_list_entries(StringStruct *self)"),
        (string_source, "inline void finish_list_teardown(StringStruct *self)"),
        (string_source, "StringStruct::~StringStruct()"),
        (sprite_source, "int SpriteList::on_insert(void *raw_payload)"),
        (sprite_source, "void SpriteList::on_remove(void *raw_payload)"),
        (sprite_source, "void SpriteList::close()"),
        (sprite_source, "SpriteList::~SpriteList()"),
        (sprite_source, "void SpriteBox::close()"),
    )
    return "\n\n".join(extract_definition(source, signature)
                          for source, signature in signatures) + "\n"


def compile_and_run(bodies, expected_success=True):
    fixture = FIXTURE.read_text()
    anchor = "/* PRODUCT LIFECYCLE BODIES */"
    assert fixture.count(anchor) == 1
    source = fixture.replace(anchor, bodies)
    with tempfile.TemporaryDirectory(prefix="spritebox-lifecycle-source-") as directory:
        cpp = Path(directory) / "fixture.cpp"
        exe = Path(directory) / "fixture"
        cpp.write_text(source)
        built = subprocess.run(
            ["c++", "-std=c++98", "-O2", "-Wall", "-Wextra",
             "-Werror=return-type", str(cpp), "-o", str(exe)],
            capture_output=True, text=True)
        assert built.returncode == 0, built.stderr
        result = subprocess.run([str(exe)], capture_output=True, text=True)
        assert (result.returncode == 0) is expected_success, result.stderr


def read_sources():
    root = repo_root()
    return ((root / "src/stringstruct.cpp").read_text(),
            (root / "src/spritebox.cpp").read_text())


def test_real_base_dtor_uses_base_noop_and_dynamic_payload_delete():
    string_source, sprite_source = read_sources()
    bodies = product_bodies(string_source, sprite_source)
    dtor = extract_definition(string_source, "StringStruct::~StringStruct()")
    assert "finish_list_teardown(this)" in dtor
    sprite_dtor = extract_definition(sprite_source, "SpriteList::~SpriteList()")
    assert sprite_dtor.strip().endswith("SpriteList::~SpriteList() {}")
    assert "kill_entry" not in sprite_dtor and "on_remove" not in sprite_dtor
    # Base teardown does invoke its hook before polymorphic delete. The host
    # C++ destructor dispatches this call to StringStruct's no-op hook exactly
    # as a real base destructor does; SpriteBox/ordinary close use the derived
    # removal hook while SpriteList is still alive.
    clear = extract_definition(string_source,
                               "inline void clear_list_entries(StringStruct *self)")
    assert "self->on_remove(payload);" in clear
    assert "ListPayloadBase *const payload" in clear
    assert "delete payload;" in clear
    compile_and_run(bodies)


def test_base_dtor_mutant_dispatching_sprite_hook_is_detected():
    string_source, sprite_source = read_sources()
    bodies = product_bodies(string_source, sprite_source)
    old = "self->on_remove(payload);"
    mutant = "static_cast<SpriteList *>(self)->SpriteList::on_remove(payload);"
    assert old in bodies
    compile_and_run(bodies.replace(old, mutant, 1), expected_success=False)


def test_base_teardown_truncation_mutant_is_detected():
    string_source, sprite_source = read_sources()
    bodies = product_bodies(string_source, sprite_source)
    old = "} while (index < self->entry_count_);"
    mutant = "} while (index < 0);"
    assert old in bodies
    compile_and_run(bodies.replace(old, mutant, 1), expected_success=False)


def test_close_skipping_sprite_removal_hook_is_detected():
    string_source, sprite_source = read_sources()
    bodies = product_bodies(string_source, sprite_source)
    old = "on_remove(payload);"
    assert old in bodies
    # Replace only the SpriteList close call, not the StringStruct base walk.
    close = extract_definition(sprite_source, "void SpriteList::close()")
    assert old in close
    mutant_close = close.replace(old, "/* omitted removal */", 1)
    compile_and_run(bodies.replace(close, mutant_close, 1), expected_success=False)


def test_611730_identity_is_preserved_at_virtual_base_destructor_owner():
    string_source, sprite_source = read_sources()
    dtor = extract_definition(sprite_source, "SpriteList::~SpriteList()")
    assert dtor.strip().endswith("SpriteList::~SpriteList() {}")
    identity = "// ORIGINAL: 0x00611730 sub_611730 0x00611730-0x006117C3"
    assert sprite_source.count(identity) == 1
    identity_line = sprite_source[:sprite_source.index(identity)].count("\n")
    dtor_line = sprite_source[:sprite_source.index("SpriteList::~SpriteList()")].count("\n")
    # Retain the original 147-byte span as a mapping/restoration obligation;
    # its current 32-byte emitted destructor body is not asserted exact here.
    assert abs(dtor_line - identity_line) <= 80


def test_canonical_6116e0_claim_is_unique_and_keeps_its_full_span():
    from match_recovery.source import read
    rows = [r for r in read(repo_root() / "src") if r.address == 0x6116E0]
    assert len(rows) == 1
    row, = rows
    assert row.path.name == "spritebox.cpp"
    assert row.image_spans == ((0x6116E0, 0x611728),)
    assert row.byte_exact
    assert row.symbol == "??_GSpriteList@@UAEPAXI@Z"
