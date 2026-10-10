from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = next(
    parent for parent in Path(__file__).resolve().parents
    if (parent / "src/texture.cpp").is_file()
)
SOURCE = ROOT / "src/texture.cpp"
HEADER = ROOT / "src/texture.h"
VERT_HEADER = ROOT / "src/vert.h"
FIXTURE = ROOT / "recovery/tests/fixtures/texture_opaque_lit_owner_oct10.cpp"
SIGNATURE = (
    "void Texture::draw(Buffer *a1, Vert *a2, int *a3, int a4, Vert *a5) {"
)
HELPER_NAMES = (
    "texture_draw_gouraud_edge_step",
    "texture_difference",
    "texture_span_step",
    "texture_span_pixels",
    "texture_draw_source_step",
    "texture_draw_source_index",
    "texture_draw_next_source_index",
    "texture_light_step",
    "texture_draw_opaque_lit_span",
)
DECLARATION_INSERTION = "// ACTUAL_VERTEX_AND_EDGE_DECLARATIONS"
HELPER_INSERTION = "// ACTUAL_HELPER_INSERTION"
METHOD_INSERTION = "// ACTUAL_METHOD_INSERTION"
MAIN_INSERTION = "// ACTUAL_MAIN_INSERTION"


def _masked(source: str) -> str:
    return COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source
    )


def _braced(source: str, signature: str) -> str:
    masked = _masked(source)
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
    raise AssertionError(f"unterminated definition: {signature}")


def _definition(source: str, name: str) -> str:
    masked = _masked(source)
    pattern = re.compile(r"\b" + re.escape(name) + r"\s*\(")
    matches = []
    for match in pattern.finditer(masked):
        depth = 1
        end = match.end()
        while end < len(masked) and depth:
            if masked[end] == "(":
                depth += 1
            elif masked[end] == ")":
                depth -= 1
            end += 1
        while end < len(masked) and masked[end].isspace():
            end += 1
        if depth == 0 and end < len(masked) and masked[end] == "{":
            matches.append((match.start(), end))
    assert len(matches) == 1, f"expected one definition for {name}: {len(matches)}"
    name_start, opening = matches[0]
    start = masked.rfind("\n", 0, name_start) + 1
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError(f"unterminated helper: {name}")


def _actual_translation_unit(method: str | None = None,
                             helpers: dict[str, str] | None = None,
                             main: str = "") -> str:
    source = SOURCE.read_text(encoding="utf-8")
    fixture = FIXTURE.read_text(encoding="utf-8")
    assert fixture.count(DECLARATION_INSERTION) == 1
    assert fixture.count(HELPER_INSERTION) == 1
    assert fixture.count(METHOD_INSERTION) == 1
    assert fixture.count(MAIN_INSERTION) == 1
    declarations = (
        _braced(VERT_HEADER.read_text(encoding="utf-8"), "struct Vert {") + ";\n" +
        _braced(HEADER.read_text(encoding="utf-8"), "struct EdgeScan {") + ";"
    )
    selected_helpers = dict(helpers) if helpers is not None else {
        name: _definition(source, name) for name in HELPER_NAMES
    }
    opaque_helper = selected_helpers["texture_draw_opaque_lit_span"]
    observation_anchor = "int pixel_count) {"
    assert opaque_helper.count(observation_anchor) == 1
    selected_helpers["texture_draw_opaque_lit_span"] = opaque_helper.replace(
        observation_anchor,
        observation_anchor + "\n    if (capture_span_light) "
        "span_light_observed[span_light_observed_count++] = light;",
        1,
    )
    helper_source = "\n\n".join(selected_helpers[name] for name in HELPER_NAMES)
    selected_method = _braced(source, SIGNATURE) if method is None else method
    return (fixture.replace(DECLARATION_INSERTION, declarations, 1)
            .replace(HELPER_INSERTION, helper_source, 1)
            .replace(METHOD_INSERTION, selected_method, 1)
            .replace(MAIN_INSERTION, main, 1))


MAIN = r'''
static void reset_fixture() {
    memset(slots, 0, sizeof(slots));
    memset(light_pack_words, 0, sizeof(light_pack_words));
    memset(events, 0, sizeof(events));
    event_count = setup_calls = fpu_resets = control_calls = 0;
    fail_right_initial = 0;
    multirow_step_case = 0;
    high_light_step_case = 0;
    capture_span_light = 0;
    span_light_observed_count = 0;
    memset(span_light_observed, 0, sizeof(span_light_observed));
    blit_max_y_index[0] = -1;
}

static void check_wrap_aware_edge_step() {
    EdgeScan edge = {};
    edge.accumA = 0x7fffffff;
    edge.deltaA = 1;
    edge.accumB = static_cast<int>(0x80000000u);
    edge.deltaB = -1;
    edge.accumC = 0x7fffffff;
    edge.deltaC = 1;
    edge.fracAccum = 0;
    edge.fracDelta = 1;
    edge.extraDeltaC = 2;
    edge.fracSub = 3;
    edge.accumD = 0x7fffffff;
    edge.deltaD = 1;
    texture_draw_gouraud_edge_step(edge);
    assert(edge.accumA == static_cast<int>(0x80000000u));
    assert(edge.accumB == 0x7fffffff);
    assert(edge.accumC == static_cast<int>(0x80000002u));
    assert(edge.fracAccum == -2 && edge.accumD == static_cast<int>(0x80000000u));
}

static void check_pixel_path() {
    static unsigned char source[8 * 8];
    static unsigned char storage[8 * 4];
    for (int i = 0; i < 64; ++i) source[i] = static_cast<unsigned char>(i + 1);
    source[9] = 12;
    source[10] = 20;
    source[11] = 40;
    memset(storage, 77, sizeof(storage));
    surface = storage;

    Vert vertices[3] = {{0, 0}, {5, 2}, {0, 0}};
    Vert uv[3] = {};
    int zvals[3] = {1000, -1000, 0};
    Buffer buffer = {};
    buffer.rect1_ = RECT{2, 0, 4, 2};
    buffer.stride_ = 8;
    Texture texture = {};
    texture.pixels_ = source;
    texture.iWidth_ = 8;

    reset_fixture();
    texture.draw(&buffer, vertices, zvals, 3, uv);

    assert(buffer.frees == 1 && fpu_resets == 2 && control_calls == 1);
    assert(event_count == 5 && events[0] == 1 && events[1] == 2 &&
           events[2] == 3 && events[3] == 4 && events[4] == 2);
    assert(setup_calls == 5);
    assert(setup_indices[0] == 0 && setup_indices[1] == 0);
    assert(setup_directions[0] == -1 && setup_directions[1] == 1);
    assert(setup_indices[2] == 1 && setup_indices[3] == 1 &&
           setup_indices[4] == 1);
    assert(setup_directions[2] == -1 && setup_directions[3] == 1 &&
           setup_directions[4] == -1);
    assert(zvals[0] == 256 && zvals[1] == -256 && zvals[2] == 0);
    assert(storage[2] == 13 && storage[3] == 21);
    assert(storage[8 + 2] == 21 && storage[8 + 3] == 21);
    assert(storage[0] == 77 && storage[1] == 77 && storage[4] == 77);
    assert(storage[8] == 77 && storage[8 + 1] == 77 && storage[8 + 4] == 77);
}

static void check_right_setup_failure() {
    static unsigned char source[64];
    static unsigned char storage[32];
    memset(source, 20, sizeof(source));
    memset(storage, 77, sizeof(storage));
    surface = storage;
    Vert vertices[3] = {{0, 0}, {5, 2}, {0, 0}};
    Vert uv[3] = {};
    int zvals[3] = {};
    Buffer buffer = {};
    buffer.rect1_ = RECT{0, 0, 8, 2};
    buffer.stride_ = 8;
    Texture texture = {};
    texture.pixels_ = source;
    texture.iWidth_ = 8;
    reset_fixture();
    fail_right_initial = 1;
    texture.draw(&buffer, vertices, zvals, 3, uv);
    assert(setup_calls == 2 && control_calls == 0);
    assert(buffer.frees == 1 && fpu_resets == 2);
    assert(event_count == 4 && events[0] == 1 && events[1] == 2 &&
           events[2] == 4 && events[3] == 2);
    for (unsigned int i = 0; i < sizeof(storage); ++i) assert(storage[i] == 77);
}

static void check_high_unsigned_light_clip() {
    static unsigned char source[8 * 8];
    static unsigned char storage[8 * 4];
    memset(source, 20, sizeof(source));
    memset(storage, 77, sizeof(storage));
    surface = storage;
    Vert vertices[3] = {{0, 0}, {5, 2}, {0, 0}};
    Vert uv[3] = {};
    int zvals[3] = {};
    Buffer buffer = {};
    buffer.rect1_ = RECT{2, 0, 4, 2};
    buffer.stride_ = 8;
    Texture texture = {};
    texture.pixels_ = source;
    texture.iWidth_ = 8;
    reset_fixture();
    high_light_step_case = 1;
    capture_span_light = 1;
    texture.draw(&buffer, vertices, zvals, 3, uv);
    assert(span_light_observed_count == 2);
    assert(span_light_observed[0] == 0x00810000);
    assert(span_light_observed[1] == 0x00010000);
}

static void check_multirow_edge_advance() {
    static unsigned char source[8 * 8];
    static unsigned char storage[8 * 4];
    for (int i = 0; i < 64; ++i) source[i] = static_cast<unsigned char>(i + 1);
    source[9] = 12;
    source[10] = 20;
    memset(storage, 77, sizeof(storage));
    surface = storage;
    Vert vertices[3] = {{0, 0}, {5, 2}, {0, 0}};
    Vert uv[3] = {};
    int zvals[3] = {};
    Buffer buffer = {};
    buffer.rect1_ = RECT{2, 0, 4, 2};
    buffer.stride_ = 8;
    Texture texture = {};
    texture.pixels_ = source;
    texture.iWidth_ = 8;
    reset_fixture();
    multirow_step_case = 1;
    texture.draw(&buffer, vertices, zvals, 3, uv);
    assert(setup_calls == 4);
    assert(storage[2] == 13 && storage[3] == 21);
    assert(storage[8 + 2] == 21 && storage[8 + 3] == 21);
}

static void check_lit_helper_golden_and_alias() {
    unsigned char source[16] = {};
    unsigned char destination[3] = {77, 77, 77};
    source[0] = 10;
    source[1] = 20;
    source[2] = 30;
    texture_draw_opaque_lit_span(destination, source, 4, 0, 0,
                                 1 << 16, 0, 1 << 16, 0, 2);
    assert(destination[0] == 11 && destination[1] == 21 && destination[2] == 77);

    unsigned char alias[8] = {10, 0, 0, 0, 0, 0, 0, 0};
    texture_draw_opaque_lit_span(alias, alias, 4, 0, 0, 0, 0,
                                 1 << 16, 0, 2);
    assert(alias[0] == 11 && alias[1] == 12); // next source load sees the first store
}

static void check_trailing_read_before_exit() {
    const long page_size = sysconf(_SC_PAGESIZE);
    assert(page_size > 0);
    unsigned char *mapping = static_cast<unsigned char *>(mmap(
        0, static_cast<size_t>(page_size) * 2, PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    assert(mapping != MAP_FAILED);
    assert(mprotect(mapping + page_size, static_cast<size_t>(page_size),
                    PROT_NONE) == 0);
    unsigned char *source = mapping + page_size - 1;
    source[0] = 23;
    unsigned char *destination = static_cast<unsigned char *>(mmap(
        0, static_cast<size_t>(page_size), PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    assert(destination != MAP_FAILED);
    destination[0] = 77;

    const pid_t child = fork();
    assert(child >= 0);
    if (child == 0) {
        const struct rlimit no_core = {0, 0};
        setrlimit(RLIMIT_CORE, &no_core);
        texture_draw_opaque_lit_span(destination, source, 1, 0, 0,
                                     1 << 16, 0, 0, 0, 1);
        _exit(0);
    }
    int status = 0;
    assert(waitpid(child, &status, 0) == child);
    assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV);
    assert(destination[0] == 23); // store precedes the faulting final preload
    munmap(destination, static_cast<size_t>(page_size));
    munmap(mapping, static_cast<size_t>(page_size) * 2);
}

int main() {
    const struct rlimit no_core = {0, 0};
    setrlimit(RLIMIT_CORE, &no_core);
    check_wrap_aware_edge_step();
    check_lit_helper_golden_and_alias();
    check_trailing_read_before_exit();
    check_pixel_path();
    check_high_unsigned_light_clip();
    check_multirow_edge_advance();
    check_right_setup_failure();
    return 0;
}
'''


def _compile_and_run(source: str, tmp_path: Path, stem: str) -> subprocess.CompletedProcess:
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host g++ required")
    cpp = tmp_path / f"{stem}.cpp"
    binary = tmp_path / stem
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-O0", "-ftrapv", "-Wall", "-Wextra",
         "-Wno-unused-parameter", "-no-pie", str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)


def test_actual_opaque_draw_and_lit_helper_execute_pinned_path(tmp_path: Path) -> None:
    result = _compile_and_run(_actual_translation_unit(main=MAIN), tmp_path,
                              "texture_opaque_lit_actual")
    assert result.returncode == 0, result.stdout + result.stderr


def test_vertex_scan_uses_existing_extraction_shape() -> None:
    source = SOURCE.read_text(encoding="utf-8")
    method = _braced(source, SIGNATURE)
    required = (
        "int minY = 0x7fff;", "int maxY = -0x7ffd;",
        "int maxX = static_cast<int>(0xffff8003u);", "int topVert = 0;",
        "int minX = 0x7fff;", "int count = *blit_vertex_count;",
        "Vert *v = a2;", "int y = v->y;", "if (y < minY)",
        "if (maxY < y)", "int x = v->x;", "if (minY < maxY)",
    )
    assert all(token in method for token in required)


@pytest.mark.parametrize(
    "mutation",
    ("noop", "keyed", "wrong_left_direction", "wrong_top_index", "wrong_count",
     "right_clip", "refresh_order", "missing_final_prefetch", "signed_clip_light",
     "signed_edge_add"),
)
def test_compiling_actual_source_mutants_are_rejected(
    mutation: str, tmp_path: Path,
) -> None:
    source = SOURCE.read_text(encoding="utf-8")
    method = _braced(source, SIGNATURE)
    helpers = {name: _definition(source, name) for name in HELPER_NAMES}
    if mutation == "signed_clip_light":
        old, new = ("static_cast<unsigned short>(light_step)",
                    "static_cast<short>(light_step)")
        assert old in method
        method = method.replace(old, new, 1)
    elif mutation == "signed_edge_add":
        helpers["texture_draw_gouraud_edge_step"] = helpers[
            "texture_draw_gouraud_edge_step"
        ].replace(
            "edge.accumA = static_cast<int>(\n        static_cast<unsigned int>(edge.accumA) +\n        static_cast<unsigned int>(edge.deltaA));",
            "edge.accumA += edge.deltaA;",
            1,
        )
    elif mutation == "noop":
        helpers["texture_draw_opaque_lit_span"] = helpers[
            "texture_draw_opaque_lit_span"
        ].replace(
            "destination[0] = static_cast<unsigned char>(\n            static_cast<unsigned int>(source_texel) + shade);",
            "destination[0] = destination[0];",
            1,
        )
    elif mutation == "keyed":
        helpers["texture_draw_opaque_lit_span"] = helpers[
            "texture_draw_opaque_lit_span"
        ].replace(
            "destination[0] = static_cast<unsigned char>(",
            "if (source_texel != 12) destination[0] = static_cast<unsigned char>(",
            1,
        )
    elif mutation == "missing_final_prefetch":
        helpers["texture_draw_opaque_lit_span"] = helpers[
            "texture_draw_opaque_lit_span"
        ].replace("\n        source_texel = source[source_offset];",
                  "\n        source_texel = 0;", 1)
    else:
        substitutions = {
            "wrong_left_direction": ("left_edge.f0 = -1;", "left_edge.f0 = 1;"),
            "wrong_top_index": ("setup_edge_gouraud(&right_edge, topVert)",
                                "setup_edge_gouraud(&right_edge, topVert + 1)"),
            "wrong_count": ("const int pixel_count = texture_span_pixels(\n"
                            "                                    texture_difference(clipped_right, clipped_left));",
                            "const int pixel_count = 1;"),
            "right_clip": ("if (*blit_clip_right <= rightX)", "if (false)"),
            "refresh_order": ("if (!left_failed) {", "if (true) {"),
        }
        old, new = substitutions[mutation]
        assert old in method
        method = method.replace(old, new, 1)

    if mutation in ("noop", "keyed", "missing_final_prefetch"):
        assert helpers["texture_draw_opaque_lit_span"] != _definition(source,
                                                                       "texture_draw_opaque_lit_span")
    elif mutation == "signed_edge_add":
        assert helpers["texture_draw_gouraud_edge_step"] != _definition(
            source, "texture_draw_gouraud_edge_step")
    else:
        assert method != _braced(source, SIGNATURE)
    result = _compile_and_run(_actual_translation_unit(method, helpers, MAIN),
                              tmp_path, f"texture_opaque_{mutation}")
    assert result.returncode != 0, result.stdout + result.stderr
