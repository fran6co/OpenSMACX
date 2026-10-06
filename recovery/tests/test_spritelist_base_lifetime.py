"""Actual-source controls for the SpriteListBase lifetime owner.

These controls fail closed: they require the actual
header declaration and actual non-empty source destructor. The fixture contains
only host ABI stubs and type declarations; every tested method body is extracted
verbatim from the installed src/ files on each run.
"""
from pathlib import Path
import os
import re
import subprocess
import tempfile

import pytest


PACKET = Path(__file__).resolve().parent


def repo_root():
    configured = os.environ.get("OPENSMACX_TEST_SOURCE_ROOT")
    candidates = [Path(configured).resolve()] if configured else []
    candidates.extend(PACKET.parents)
    for candidate in candidates:
        if (candidate / "src/spritebox.h").exists():
            return candidate
    raise AssertionError("cannot locate OpenSMACX src/spritebox.h")


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
    raise AssertionError(f"unterminated installed definition: {signature}")


def installed_bodies(root):
    from match_recovery.source import read

    header = (root / "src/spritebox.h").read_text()
    string_source = (root / "src/stringstruct.cpp").read_text()
    sprite_source = (root / "src/spritebox.cpp").read_text()
    assert re.search(
        r"class\s+SpriteListBase\s*:\s*public\s+StringStruct\s*\{"
        r"(?:(?!\};).)*virtual\s+~SpriteListBase\s*\(\s*\)\s*;",
        header, re.S), "installed genuine SpriteListBase declaration is missing"
    assert re.search(r"class\s+SpriteList\s*:\s*public\s+SpriteListBase\b",
                     header), "SpriteList must derive from the installed base"
    assert header.index("Spot spots_;") < header.index("SpriteList list_;"), (
        "SpriteBox member declaration order must preserve list-before-Spot destruction")

    base_dtor = extract_definition(sprite_source, "SpriteListBase::~SpriteListBase()")
    assert "remove_all()" in base_dtor, "intermediate destructor must drain the shared list"
    assert not re.search(r"\{\s*\}", base_dtor), "empty placeholder destructor is not recovery"
    assert "SpriteListBase::~SpriteListBase()" in sprite_source

    # Markers/identities are read from the installed source as a separate
    # continuity check; this test does not move or manufacture any mapping.
    rows = [row for row in read(root / "src") if row.address == 0x611730]
    assert len(rows) == 1, "611730 identity must remain unique in installed source"
    row = rows[0]
    assert row.path == root / "src/spritebox.cpp"
    assert row.image_spans == ((0x611730, 0x6117C3),)

    string_body = (root / "src/stringstruct.cpp").read_text()
    sprite_body = sprite_source
    required = (
        (string_body, "int StringStruct::on_insert(void *)"),
        (string_body, "void StringStruct::on_remove(void *)"),
        (string_body, "inline void clear_list_entries(StringStruct *self)"),
        (string_body, "inline void finish_list_teardown(StringStruct *self)"),
        (string_body, "void StringStruct::remove_all()"),
        (string_body, "StringStruct::~StringStruct()"),
        (sprite_body, "int SpriteList::on_insert(void *raw_payload)"),
        (sprite_body, "void SpriteList::on_remove(void *raw_payload)"),
        (sprite_body, "void SpriteList::close()"),
        (sprite_body, "SpriteList::~SpriteList()"),
        (sprite_body, "SpriteListBase::~SpriteListBase()"),
        (sprite_body, "void SpriteBox::close()"),
        (sprite_body, "SpriteBox::~SpriteBox()"),
    )
    bodies = "\n\n".join(extract_definition(src, sig) for src, sig in required)
    return header, string_body, sprite_body, bodies, base_dtor


def fixture_source(root, bodies, string_source):
    fixture_path = root / "recovery/tests/sprite_list_lifecycle_control.cpp"
    fixture = fixture_path.read_text()
    # The checked source header above is the authority for this compact host
    # declaration scaffold; this transformation adds no product method body.
    fixture = fixture.replace(
        "class SpriteList : public StringStruct {",
        "class SpriteListBase : public StringStruct {\n"
        "public:\n  virtual ~SpriteListBase();\n};\n"
        "class SpriteList : public SpriteListBase {", 1)
    fixture = fixture.replace(
        "    virtual ~StringStruct();\n};",
        "    virtual ~StringStruct();\n    void remove_all();\n};", 1)
    fixture = fixture.replace(
        "    SpriteList list_;\n    Spot spots_;",
        "    Spot spots_;\n    SpriteList list_;", 1)
    fixture = fixture.replace(
        "    void close();\n};\n\nstatic uint32_t SpriteBoxDefaultField8;",
        "    void close();\n    ~SpriteBox();\n};\n\nstatic uint32_t SpriteBoxDefaultField8;", 1)
    fixture = fixture.replace(
        "class Spot { public: void shutdown() {} };",
        "class Spot { public: void shutdown() {} ~Spot(); };", 1)
    fixture = fixture.replace(
        "static int entry_destroys;",
        "static int entry_destroys;\nstatic int remove_all_calls;\n"
        "static int track_box;\nstatic int remove_all_order;\n"
        "static int spot_destroy_order;\nstatic int lifetime_order;", 1)

    # Behavior-only probes around actual extracted bodies: require the
    # intermediate stage to empty the ring before StringStruct destruction,
    # and observe member destruction order without changing product behavior.
    string_dtor = extract_definition(string_source, "StringStruct::~StringStruct()")
    instrumented_dtor = string_dtor.replace(
        "finish_list_teardown(this);",
        "assert(head_ == 0); // lifecycle ordering probe\n"
        "    finish_list_teardown(this);", 1)
    bodies = bodies.replace(string_dtor, instrumented_dtor, 1)
    remove_all = extract_definition(string_source, "void StringStruct::remove_all()")
    instrumented_remove_all = remove_all.replace(
        "clear_list_entries(this);",
        "++remove_all_calls;\n    if (track_box) remove_all_order = ++lifetime_order;\n"
        "    clear_list_entries(this);", 1)
    bodies = bodies.replace(remove_all, instrumented_remove_all, 1)
    bodies += "\nSpot::~Spot() { if (track_box) spot_destroy_order = ++lifetime_order; }\n"

    anchor = "/* PRODUCT LIFECYCLE BODIES */"
    assert fixture.count(anchor) == 1
    fixture = fixture.replace("    SpriteBox box;",
                              "    track_box = 1;\n    {\n    SpriteBox box;", 1)
    closing = "    tracked_text = 0;\n    return 0;"
    assert closing in fixture
    fixture = fixture[:fixture.rfind(closing)] + fixture[fixture.rfind(closing):].replace(
        closing,
        "    tracked_text = 0;\n    }\n    track_box = 0;\n"
        "    assert(remove_all_order < spot_destroy_order);\n"
        "    return 0;", 1)
    return fixture.replace(anchor, bodies)


def compile_and_run(source, compiler, expected_success):
    with tempfile.TemporaryDirectory(prefix="sprite-list-installed-source-") as tmp:
        cpp = Path(tmp) / "fixture.cpp"
        exe = Path(tmp) / "fixture"
        cpp.write_text(source)
        built = subprocess.run(
            [compiler, "-std=c++98", "-O2", "-Wall", "-Wextra",
             "-Werror=return-type", str(cpp), "-o", str(exe)],
            capture_output=True, text=True, timeout=30)
        assert built.returncode == 0, built.stderr
        ran = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
        assert (ran.returncode == 0) is expected_success, ran.stderr


def test_intermediate_owner_and_actual_cleanup_source():
    root = repo_root()
    _, string_source, _, bodies, base_dtor = installed_bodies(root)
    source = fixture_source(root, bodies, string_source)
    # Assert this is using an installed body, not a test-authored replacement.
    assert base_dtor in bodies
    for compiler in ("g++", "clang++"):
        compile_and_run(source, compiler, True)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_omitting_installed_intermediate_cleanup_fails(compiler):
    root = repo_root()
    _, string_source, _, bodies, base_dtor = installed_bodies(root)
    source = fixture_source(root, bodies, string_source)
    mutated_dtor = base_dtor.replace("remove_all();", "/* omitted cleanup */", 1)
    assert mutated_dtor != base_dtor
    compile_and_run(source.replace(base_dtor, mutated_dtor, 1), compiler, False)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_dispatching_intermediate_clear_to_sprite_hook_fails(compiler):
    root = repo_root()
    _, string_source, _, bodies, _ = installed_bodies(root)
    source = fixture_source(root, bodies, string_source)
    old = "self->on_remove(payload);"
    mutant = "static_cast<SpriteList *>(self)->SpriteList::on_remove(payload);"
    assert old in source
    compile_and_run(source.replace(old, mutant, 1), compiler, False)

