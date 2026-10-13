from pathlib import Path
import importlib.util
import re
import resource
import shutil
import subprocess

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS
from recovery.tests.texture_direct_storage_host import PRELUDE_MARKER, direct_storage_prelude


ROOT = next(
    parent for parent in Path(__file__).resolve().parents
    if (parent / "src/texture.cpp").is_file()
)
SOURCE = ROOT / "src/texture.cpp"
BASE_FIXTURE = ROOT / "recovery/tests/fixtures/texture_draw_trans_host.cpp"
FIXTURE = ROOT / "recovery/tests/fixtures/texture_keyed_light_owner_oct10.cpp"
SIGNATURE = (
    "void Texture::draw_trans(Buffer *buffer, Vert *vertices, int *zvals,\n"
    "                         int count, Vert *uv) {"
)

_existing_test_spec = importlib.util.spec_from_file_location(
    "texture_draw_trans_actual_source",
    ROOT / "recovery/tests/test_texture_draw_trans_actual_source.py",
)
assert _existing_test_spec is not None and _existing_test_spec.loader is not None
_existing_test = importlib.util.module_from_spec(_existing_test_spec)
_existing_test_spec.loader.exec_module(_existing_test)


def _body(source: str, signature: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source
    )
    start = masked.index(signature)
    opening = masked.index("{", start)
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError(f"unterminated source body: {signature}")


def _configured_host_template() -> str:
    fixture = BASE_FIXTURE.read_text(encoding="utf-8")
    fixture = fixture.replace(
        "static int setup_calls;",
        "static int setup_calls;\n"
        "static int keyed_owner_rows;\n"
        "static int keyed_owner_delta_d;\n"
        "static unsigned int keyed_owner_lights[4];\n"
        "static unsigned short keyed_owner_steps[4];\n"
        "static int keyed_owner_span_count;",
        1,
    )
    fixture = fixture.replace("edge->counter = 1;",
                              "edge->counter = keyed_owner_rows;", 1)
    fixture = fixture.replace(
        "edge->deltaA = edge->deltaB = edge->deltaC = edge->deltaD = 0;",
        "edge->deltaA = edge->deltaB = edge->deltaC = edge->deltaD = 0;\n"
        "    edge->deltaD = keyed_owner_delta_d;",
        1,
    )
    fixture = fixture.replace(
        "edge->accumD = 0x00010000;",
        "edge->accumD = call == 0 ? 0x7fffffff : "
        "static_cast<int>(0x800000ffu);",
        1,
    )
    fixture = fixture.replace(
        "edge->accumD = 0x00010000;",
        "edge->accumD = static_cast<int>(0x800000ffu);",
        1,
    )
    fixture = fixture.replace("edge->accumC = 5;", "edge->accumC = 1;", 1)
    return fixture


def _replace_main(fixture: str, main: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), fixture
    )
    start = masked.index("int main()")
    opening = masked.index("{", start)
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return fixture[:start] + main.rstrip() + "\n"
    raise AssertionError("host fixture main is unterminated")


def _translation_unit(method: str | None = None,
                      helper_mutation: tuple[str, str] | None = None) -> str:
    source = SOURCE.read_text(encoding="utf-8")
    fixture = _configured_host_template()
    body = _existing_test._method_body(source) if method is None else method
    helpers = {
        name: _existing_test._definition(source, name)
        for name in _existing_test.HELPER_NAMES
    }
    assert fixture.count(PRELUDE_MARKER) == 1
    fixture = fixture.replace(PRELUDE_MARKER, direct_storage_prelude(source), 1)
    wrapper = helpers["texture_draw_keyed_lit_span"]
    anchor = "    texture_draw_lit_span_mode(destination, pixels, width, u, v, du, dv,"
    assert wrapper.count(anchor) == 1
    helpers["texture_draw_keyed_lit_span"] = wrapper.replace(
        anchor,
        "    keyed_owner_lights[keyed_owner_span_count] = "
        "static_cast<unsigned int>(light);\n"
        "    keyed_owner_steps[keyed_owner_span_count] = light_step;\n"
        "    ++keyed_owner_span_count;\n" + anchor,
        1,
    )
    if helper_mutation is not None:
        name, mutated = helper_mutation
        helpers[name] = mutated
    helper_source = "\n\n".join(
        helpers[name] for name in _existing_test.HELPER_NAMES
    )
    assert fixture.count(_existing_test.HELPER_INSERTION) == 1
    assert fixture.count(_existing_test.METHOD_INSERTION) == 1
    fixture = fixture.replace(_existing_test.HELPER_INSERTION,
                              helper_source, 1)
    fixture = fixture.replace(_existing_test.METHOD_INSERTION, body, 1)
    main = FIXTURE.read_text(encoding="utf-8")
    old_alias_reset = "    memset(slots, 0, sizeof(slots));\n"
    old_key_store = "    texture_raster_key = 99;"
    assert main.count(old_alias_reset) == 1
    assert main.count(old_key_store) == 1
    # The actual raster objects have static storage and begin zeroed in this
    # fresh host process; retain the fixture's nonzero-key setup on that object.
    main = main.replace(old_alias_reset, "", 1)
    main = main.replace(old_key_store, "    TextureRasterKey = 99;", 1)
    return _replace_main(fixture, main)


def _compile_run(source: str, tmp_path: Path, stem: str,
                 *, trap_signed_overflow: bool = False) -> subprocess.CompletedProcess:
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    cpp = tmp_path / f"{stem}.cpp"
    binary = tmp_path / stem
    cpp.write_text(source, encoding="utf-8")
    command = [compiler, "-std=c++11", "-Wall", "-Wextra",
               "-Wno-unused-parameter", "-no-pie"]
    if trap_signed_overflow:
        command.append("-ftrapv")
    command.extend((str(cpp), "-o", str(binary)))
    built = subprocess.run(command, cwd=ROOT, check=False,
                           capture_output=True, text=True, timeout=45)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run(
        [str(binary)], check=False, capture_output=True, text=True, timeout=30,
        preexec_fn=lambda: resource.setrlimit(resource.RLIMIT_CORE, (0, 0)),
    )


def test_actual_keyed_draw_trans_uses_wrapped_gouraud_steps_and_unsigned_clip(
    tmp_path: Path,
) -> None:
    source = SOURCE.read_text(encoding="utf-8")
    body = _existing_test._method_body(source)
    helper = _existing_test._definition(source, "texture_draw_gouraud_edge_step")
    assert "texture_draw_gouraud_edge_step(left_edge);" in body
    assert "texture_draw_gouraud_edge_step(right_edge);" in body
    assert "static_cast<unsigned short>(light_step)" in body
    assert "static_cast<short>(light_step)" not in body
    assert "static_cast<unsigned int>(edge.accumD)" in helper
    assert "static_cast<unsigned int>(edge.deltaD)" in helper
    result = _compile_run(_translation_unit(), tmp_path,
                          "texture_keyed_light_owner")
    assert result.returncode == 0, result.stdout + result.stderr


def test_actual_keyed_draw_trans_rejects_signed_high_word_clip_mutant(
    tmp_path: Path,
) -> None:
    source = SOURCE.read_text(encoding="utf-8")
    body = _existing_test._method_body(source)
    old = "static_cast<unsigned short>(light_step)"
    mutant = body.replace(old, "static_cast<short>(light_step)", 1)
    assert mutant != body
    result = _compile_run(_translation_unit(mutant), tmp_path,
                          "texture_keyed_signed_clip_mutant")
    assert result.returncode != 0


def test_actual_keyed_edge_step_rejects_signed_overflow_mutant(
    tmp_path: Path,
) -> None:
    source = SOURCE.read_text(encoding="utf-8")
    helper = _existing_test._definition(source, "texture_draw_gouraud_edge_step")
    old = (
        "edge.accumD = static_cast<int>(\n"
        "        static_cast<unsigned int>(edge.accumD) +\n"
        "        static_cast<unsigned int>(edge.deltaD));"
    )
    mutant = helper.replace(old, "edge.accumD += edge.deltaD;", 1)
    assert mutant != helper
    result = _compile_run(
        _translation_unit(helper_mutation=("texture_draw_gouraud_edge_step",
                                           mutant)),
        tmp_path, "texture_keyed_signed_dda_mutant",
        trap_signed_overflow=True,
    )
    assert result.returncode != 0
