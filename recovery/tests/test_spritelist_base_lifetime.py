"""Actual-source controls for the independent SpriteListBase ring owner.

The fixture contains host ABI stubs and type declarations; tested list methods
are extracted from the installed header/source on each run.
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
    if configured:
        candidate = Path(configured).resolve()
        if ((candidate / "src/spritebox.h").exists()
                and (candidate / "src/stringstruct.cpp").exists()):
            return candidate
        raise AssertionError("OPENSMACX_TEST_SOURCE_ROOT lacks installed lifecycle source")
    for candidate in PACKET.parents:
        if (candidate / "src/spritebox.h").exists():
            return candidate
    raise AssertionError("cannot locate OpenSMACX src/spritebox.h")


def test_ring_member_annotations_are_native_layout_expectations():
    from match_recovery.layout_source import read_members
    from match_recovery.source_inventory import image_pin

    root = repo_root()
    members = [item for item in read_members(root, image_pin(root))
               if item.class_name == "SpriteListBase"]
    assert {item.member: item.offset for item in members} == {
        "head_": 0x08, "current_": 0x0C, "entry_count_": 0x10,
        "current_position_": 0x14, "allocator_": 0x18,
    }
    assert len(members) == 5


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
    from match_recovery.source import read, read_file

    header = (root / "src/spritebox.h").read_text()
    string_source = (root / "src/stringstruct.cpp").read_text()
    sprite_source = (root / "src/spritebox.cpp").read_text()
    assert re.search(
        r"class\s+SpriteListBase\s*:\s*public\s+virtual\s+StringAllocationBase\s*\{",
        header), "SpriteListBase must own its virtual allocation base directly"
    assert re.search(r"class\s+SpriteListBase\s*:\s*public\s+virtual\s+StringAllocationBase",
                     header) and not re.search(
                         r"class\s+SpriteListBase\s*:\s*public\s+StringStruct", header), (
        "SpriteListBase must not add a StringStruct base teardown stage")
    assert re.search(r"class\s+SpriteList\s*:\s*public\s+SpriteListBase\b",
                     header), "SpriteList must derive from the installed base"
    assert header.index("Spot spots_;") < header.index("SpriteList list_;"), (
        "SpriteBox member declaration order must preserve list-before-Spot destruction")

    base_ctor = extract_definition(header, "SpriteListBase()")
    base_ctor = base_ctor.replace("SpriteListBase()", "SpriteListBase::SpriteListBase()", 1)
    inline_base_dtor = extract_definition(header, "virtual ~SpriteListBase()")
    base_dtor = inline_base_dtor.replace(
        "virtual ~SpriteListBase()", "SpriteListBase::~SpriteListBase()", 1)
    remove_all = extract_definition(header, "void remove_all() {")
    remove_all = remove_all.replace("void remove_all()", "void SpriteListBase::remove_all()", 1)
    seek_pos = extract_definition(header, "int seek_pos(int a1)")
    seek_pos = seek_pos.replace("int seek_pos(int a1)", "int SpriteListBase::seek_pos(int a1)", 1)
    base_insert = extract_definition(header, "virtual int on_insert(void *)")
    base_insert = base_insert.replace("virtual int on_insert(void *)", "int SpriteListBase::on_insert(void *)", 1)
    base_remove = extract_definition(header, "virtual void on_remove(void *)")
    base_remove = base_remove.replace("virtual void on_remove(void *)", "void SpriteListBase::on_remove(void *)", 1)
    derived_dtor = extract_definition(sprite_source, "SpriteList::~SpriteList()")
    assert re.search(r"SpriteList::~SpriteList\(\)\s*\{\s*\}$", derived_dtor)
    assert "remove_all();" in base_dtor and "current_position_ = 0;" in base_dtor, (
        "base destructor must drain its ring and unconditionally reset position")
    assert not re.search(r"\{\s*\}", base_dtor), "empty placeholder destructor is not recovery"
    assert not re.search(
        r"^\s*SpriteListBase::~SpriteListBase\(\)\s*\{", sprite_source, re.M), (
        "base destructor must have only its inline header definition")

    # Markers/identities are read from the installed source as a separate
    # continuity check; this test does not move or manufacture any mapping.
    rows = [row for row in read(root / "src")
            if row.address == 0x611730]
    assert len(rows) == 1, "611730 identity must remain unique in installed source"
    row = rows[0]
    assert row.path == root / "src/spritebox.cpp"
    assert row.image_spans == ((0x611730, 0x6117C3),)
    base_rows = [row for row in read_file(root / "src/spritebox.cpp")
                 if row.address == 0x402BB0]
    assert len(base_rows) == 1, "402BB0 deleting-destructor identity must remain unique"
    base_row, = base_rows
    assert base_row.path == root / "src/spritebox.cpp"
    assert base_row.mode.value == "file"
    assert base_row.image_spans == ((0x402BB0, 0x402C16),)
    assert base_row.symbol == "??_GSpriteListBase@@UAEPAXI@Z"

    sprite_body = sprite_source
    required = (
        (sprite_body, "int SpriteList::on_insert(void *raw_payload)"),
        (sprite_body, "void SpriteList::on_remove(void *raw_payload)"),
        (sprite_body, "void SpriteList::close()"),
        (sprite_body, "SpriteList::~SpriteList()"),
        (sprite_body, "void SpriteBox::close()"),
        (sprite_body, "SpriteBox::~SpriteBox()"),
    )
    extracted = [base_ctor, base_insert, base_remove, remove_all, seek_pos, base_dtor]
    for src, sig in required:
        body = extract_definition(src, sig)
        extracted.append(body)
    bodies = "\n\n".join(extracted)
    return header, string_source, sprite_body, bodies, base_dtor


def fixture_source(root, bodies, string_source, base_dtor=None):
    if base_dtor is None:
        base_dtor = extract_definition(
            (root / "src/spritebox.h").read_text(), "virtual ~SpriteListBase()")
        base_dtor = base_dtor.replace(
            "virtual ~SpriteListBase()", "SpriteListBase::~SpriteListBase()", 1)
    fixture_path = root / "recovery/tests/sprite_list_lifecycle_control.cpp"
    fixture = fixture_path.read_text()
    header = (root / "src/spritebox.h").read_text()
    for member in ("right_click_callback_", "right_down_callback_",
                   "right_up_callback_", "right_double_click_callback_"):
        assert re.search(r"ClickCallback\s+" + member + r"\s*;", header)
        assert re.search(r"void\s*\(\*" + member + r"\)\(int\);", fixture)
    # The checked source header above is the authority for this compact host
    # declaration scaffold; this transformation adds no product method body.
    expected_base = "class SpriteListBase : public virtual StringAllocationBase {"
    assert expected_base in fixture
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

    # Instrument only the extracted ring walk: count live traversals and retain
    # the observed ordering before the later Spot member destruction.
    instrumented_dtor = base_dtor.replace(
        "remove_all();",
        "remove_all();\n    assert(head_ == 0); /* lifecycle ordering probe */", 1)
    bodies = bodies.replace(base_dtor, instrumented_dtor, 1)
    remove_all = extract_definition(
        header, "void remove_all() {").replace(
            "void remove_all()", "void SpriteListBase::remove_all()", 1)
    instrumented_remove_all = remove_all.replace(
        "if (!head_) {",
        "++remove_all_calls;\n    if (head_) ++live_cleanup_walks;\n"
        "    if (track_box) remove_all_order = ++lifetime_order;\n"
        "    if (!head_) {", 1)
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
    source = fixture_source(root, bodies, string_source, base_dtor)
    # Assert this is using an installed body, not a test-authored replacement.
    assert base_dtor in bodies
    for compiler in ("g++", "clang++"):
        compile_and_run(source, compiler, True)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_omitting_installed_intermediate_cleanup_fails(compiler):
    root = repo_root()
    _, string_source, _, bodies, base_dtor = installed_bodies(root)
    source = fixture_source(root, bodies, string_source, base_dtor)
    instrumented_dtor = base_dtor.replace(
        "remove_all();",
        "remove_all();\n    assert(head_ == 0); /* lifecycle ordering probe */", 1)
    mutated_dtor = instrumented_dtor.replace(
        "remove_all();", "/* omitted cleanup */", 1)
    assert instrumented_dtor != base_dtor and mutated_dtor != instrumented_dtor
    compile_and_run(source.replace(instrumented_dtor, mutated_dtor, 1), compiler, False)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_dispatching_intermediate_clear_to_sprite_hook_fails(compiler):
    root = repo_root()
    _, string_source, _, bodies, _ = installed_bodies(root)
    source = fixture_source(root, bodies, string_source)
    old = "on_remove(payload);"
    mutant = "static_cast<SpriteList *>(this)->SpriteList::on_remove(payload);"
    assert old in source
    compile_and_run(source.replace(old, mutant, 1), compiler, False)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_original_entry_count_walk_truncation_is_detected(compiler):
    root = repo_root()
    _, string_source, _, bodies, _ = installed_bodies(root)
    source = fixture_source(root, bodies, string_source)
    old = "} while (index < entry_count_);"
    assert old in bodies
    compile_and_run(source.replace(old, "} while (index < 0);", 1), compiler, False)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_sibling_cursor_keeps_signed_wrap_and_failure_state(compiler):
    root = repo_root()
    _, string_source, _, bodies, _ = installed_bodies(root)
    source = fixture_source(root, bodies, string_source)
    # The installed seek_pos body is exercised by the lifecycle fixture's
    # two-entry ring, including negative wrap and an out-of-range no-op.
    compile_and_run(source, compiler, True)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_sibling_cursor_negative_wrap_mutant_is_detected(compiler):
    root = repo_root()
    _, string_source, _, bodies, _ = installed_bodies(root)
    source = fixture_source(root, bodies, string_source)
    old = "current_position_ = a1 + entry_count_;"
    assert old in source
    compile_and_run(source.replace(old, "current_position_ = a1;", 1), compiler, False)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_close_skipping_real_sprite_removal_hook_is_detected(compiler):
    root = repo_root()
    _, string_source, sprite_source, bodies, _ = installed_bodies(root)
    close = extract_definition(sprite_source, "void SpriteList::close()")
    assert "on_remove(payload);" in close
    mutated = close.replace("on_remove(payload);", "/* omitted removal */", 1)
    source = fixture_source(root, bodies.replace(close, mutated, 1), string_source)
    compile_and_run(source, compiler, False)


def test_canonical_6116e0_claim_is_unique_and_keeps_its_full_span():
    from match_recovery.source import read
    rows = [row for row in read(repo_root() / "src")
            if row.address == 0x6116E0]
    assert len(rows) == 1
    row, = rows
    assert row.path == repo_root() / "src/spritebox.cpp"
    assert row.image_spans == ((0x6116E0, 0x611728),)
    assert row.byte_exact
    assert row.symbol == "??_GSpriteList@@UAEPAXI@Z"


def test_explicit_missing_source_root_cannot_fall_back_to_current_checkout(monkeypatch, tmp_path):
    monkeypatch.setenv("OPENSMACX_TEST_SOURCE_ROOT", str(tmp_path))
    with pytest.raises(AssertionError, match="OPENSMACX_TEST_SOURCE_ROOT"):
        repo_root()


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("member", ["right_click_callback_", "right_down_callback_",
                                    "right_up_callback_", "right_double_click_callback_"])
def test_close_omitting_typed_right_callback_reset_is_detected(compiler, member):
    root = repo_root()
    _, string_source, sprite_source, bodies, _ = installed_bodies(root)
    close = extract_definition(sprite_source, "void SpriteBox::close()")
    needle = member + " = 0;"
    assert close.count(needle) == 1
    mutated = close.replace(needle, "/* omitted callback reset */", 1)
    source = fixture_source(root, bodies.replace(close, mutated, 1), string_source)
    compile_and_run(source, compiler, False)

