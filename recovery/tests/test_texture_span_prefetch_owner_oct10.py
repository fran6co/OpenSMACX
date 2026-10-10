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
FIXTURE = ROOT / "recovery/tests/fixtures/texture_span_prefetch_owner_oct10.cpp"
HELPER_INSERTION = "// ACTUAL_HELPER_INSERTION"
KERNEL_INSERTION = "// ACTUAL_KERNEL_INSERTION"
MAIN_INSERTION = "// ACTUAL_MAIN_INSERTION"
HELPERS = (
    "texture_draw_source_step",
    "texture_draw_source_index",
    "texture_draw_next_source_index",
    "texture_draw_lit_span_mode",
    "texture_draw_lit_span",
    "texture_draw_keyed_lit_span",
)
METHODS = {
    "draw_trans": (
        "void Texture::draw_trans(Buffer *buffer, Vert *vertices, int use_eight_vertices) {",
        "raw",
    ),
    "draw_color_key": (
        "void Texture::draw_color_key(Buffer *buffer, Vert *vertices, int count, Vert *uv) {",
        "raw",
    ),
    "draw_below_cutoff": (
        "void Texture::draw_below_cutoff(Buffer *buffer, Vert *vertices, int *zvals, int count,\n"
        "                                Vert *uv, int cutoff) {",
        "cutoff",
    ),
    "draw_color_key_alt": (
        "void Texture::draw_color_key(Buffer *buffer, Vert *vertices, int count, Vert *uv, int color) {",
        "alternate",
    ),
}


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
    matches = []
    for match in re.finditer(r"\b" + re.escape(name) + r"\s*\(", masked):
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
    raise AssertionError(f"unterminated definition: {name}")


def _loop_kernel(method: str, kind: str) -> str:
    masked = _masked(method)
    loop_start = masked.index(
        "for (unsigned int written = 0; written < remaining; ++written)"
    )
    prefetch = masked.rfind(
        "const volatile unsigned char *source_reads = pixels;", 0, loop_start
    )
    assert prefetch >= 0
    opening = masked.index("{", loop_start)
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                break
    snippet = method[prefetch:end + 1]
    params = (
        "unsigned char *pixels, unsigned char *out, int64_t source, int width, "
        "int64_t step, unsigned int uf, unsigned int vf, int du, int dv, "
        "unsigned short light, unsigned short dz, unsigned char key, "
        "unsigned char alternate, int cutoff, unsigned int remaining, int parity"
    )
    return f"static void kernel_{kind}({params}) {{\n{snippet}\n}}"


def _draw2_row_block(method: str) -> str:
    masked = _masked(method)
    start = masked.index("int64_t source =", masked.index("void Texture::draw2"))
    branch = masked.index("if ((y & 1) == 0) {", start)
    opening = masked.index("{", branch)
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                break
    else_branch = masked.index("else {", end)
    else_opening = masked.index("{", else_branch)
    depth = 0
    for end in range(else_opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                break
    block = method[start:end + 1]
    return (
        "static void kernel_draw2_row(unsigned char *pixels, unsigned char *out, "
        "int y, int width, int u, int v, int du, int dv, int z, unsigned short dz, "
        "int cutoff, int fillColor, unsigned int remaining) {\n" + block + "\n}"
    )


def _translation_unit(source: str, main: str, mutations: dict[str, str] | None = None) -> str:
    fixture = FIXTURE.read_text(encoding="utf-8")
    assert fixture.count(HELPER_INSERTION) == 1
    assert fixture.count(KERNEL_INSERTION) == 1
    assert fixture.count(MAIN_INSERTION) == 1
    helpers = {name: _definition(source, name) for name in HELPERS}
    kernels = []
    for name, (signature, kind) in METHODS.items():
        method = _braced(source, signature)
        kernels.append(_loop_kernel(method, f"{kind}_{name}"))
    draw2 = _braced(
        source,
        "void Texture::draw2(Buffer *buffer, Vert *vertices, int *zvals, int count,\n"
        "                    Vert *uv, int cutoff, int fillColor) {",
    )
    kernels.append(_draw2_row_block(draw2))
    if mutations:
        for helper_name, old, new in mutations.get("helpers", []):
            assert old in helpers[helper_name]
            helpers[helper_name] = helpers[helper_name].replace(old, new, 1)
        for kernel_index, old, new in mutations.get("kernels", []):
            assert old in kernels[kernel_index]
            kernels[kernel_index] = kernels[kernel_index].replace(old, new, 1)
    return (fixture.replace(HELPER_INSERTION,
                            "\n\n".join(helpers[name] for name in HELPERS), 1)
            .replace(KERNEL_INSERTION, "\n\n".join(kernels), 1)
            .replace(MAIN_INSERTION, main, 1))


MAIN = r'''
static void check_helper_sequences() {
    unsigned char pixels[16] = {10, 20, 30, 40};
    unsigned char output[8] = {77, 77, 77, 77};
    texture_draw_lit_span(output, pixels, 4, 0, 0, 1 << 16, 0, 0, 0, 4);
    assert(output[0] == 10 && output[1] == 10 &&
           output[2] == 20 && output[3] == 30);

    unsigned char alias[8] = {10, 0, 0, 0, 0, 0, 0, 0};
    texture_draw_lit_span(alias, alias, 4, 0, 0, 1 << 16, 0, 0, 0, 4);
    assert(alias[0] == 10);
    assert(alias[1] == 10);
    assert(alias[2] == 10);
    assert(alias[3] == 10);

    unsigned char keyed[8] = {77, 77, 77, 77};
    texture_draw_keyed_lit_span(keyed, pixels, 4, 0, 0, 1 << 16, 0,
                                0, 0, 3, 20);
    assert(keyed[0] == 10 && keyed[1] == 77 && keyed[2] == 30);
}

static void check_guarded_after_advance(bool keyed_skip) {
    const long page = sysconf(_SC_PAGESIZE);
    assert(page > 0);
    unsigned char *mapping = static_cast<unsigned char *>(mmap(
        0, static_cast<size_t>(page) * 2, PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    assert(mapping != MAP_FAILED);
    assert(mprotect(mapping + page, static_cast<size_t>(page), PROT_NONE) == 0);
    unsigned char *pixels = mapping + page - 1;
    pixels[0] = keyed_skip ? 23 : 23;
    unsigned char *output = static_cast<unsigned char *>(mmap(
        0, static_cast<size_t>(page), PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    assert(output != MAP_FAILED);
    output[0] = 77;
    const pid_t child = fork();
    assert(child >= 0);
    if (child == 0) {
        texture_draw_keyed_lit_span(output, pixels, 1, 0, 0, 1 << 16, 0,
                                    0, 0, 1, keyed_skip ? 23 : 0);
        _exit(0);
    }
    int status = 0;
    assert(waitpid(child, &status, 0) == child);
    assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV);
    assert(output[0] == (keyed_skip ? 77 : 23));
    munmap(output, static_cast<size_t>(page));
    munmap(mapping, static_cast<size_t>(page) * 2);
}

static void check_guarded_delayed_sample() {
    const long page = sysconf(_SC_PAGESIZE);
    assert(page > 0);
    unsigned char *mapping = static_cast<unsigned char *>(mmap(
        0, static_cast<size_t>(page) * 2, PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    assert(mapping != MAP_FAILED);
    assert(mprotect(mapping + page, static_cast<size_t>(page), PROT_NONE) == 0);
    unsigned char *pixels = mapping + page - 1;
    pixels[0] = 31;
    unsigned char *output = static_cast<unsigned char *>(mmap(
        0, static_cast<size_t>(page), PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    assert(output != MAP_FAILED);
    output[0] = output[1] = 77;
    const pid_t child = fork();
    assert(child >= 0);
    if (child == 0) {
        texture_draw_lit_span(output, pixels, 1, 0, 0, 1 << 16, 0, 0, 0, 2);
        _exit(0);
    }
    int status = 0;
    assert(waitpid(child, &status, 0) == child);
    assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV);
    assert(output[0] == 31 && output[1] == 31);
    munmap(output, static_cast<size_t>(page));
    munmap(mapping, static_cast<size_t>(page) * 2);
}

static void check_inline_loops() {
    unsigned char pixels[16] = {10, 20, 30, 40};
    unsigned char output[8] = {77, 77, 77, 77};
    kernel_raw_draw_trans(pixels, output, 0, 4, 1, 0, 0, 1 << 16, 0, 0,
                          0, 20, 0, 0, 3, 0);
    assert(output[0] == 10 && output[1] == 77 && output[2] == 30);
    memset(output, 77, sizeof(output));
    kernel_raw_draw_color_key(pixels, output, 0, 4, 1, 0, 0, 1 << 16, 0, 0,
                              0, 20, 0, 0, 3, 0);
    assert(output[0] == 10 && output[1] == 77 && output[2] == 30);
    memset(output, 77, sizeof(output));
    kernel_cutoff_draw_below_cutoff(pixels, output, 0, 4, 1, 0, 0,
                                    1 << 16, 0, 0x0100, 0, 0, 0, 2, 3, 0);
    assert(output[0] == 11 && output[1] == 21 && output[2] == 31);
    memset(output, 77, sizeof(output));
    kernel_alternate_draw_color_key_alt(pixels, output, 0, 4, 1, 0, 0,
                                        1 << 16, 0, 0, 0, 20, 90, 0, 3, 0);
    assert(output[0] == 90 && output[1] == 77 && output[2] == 90);
}

static void check_draw2_initial_fill_read() {
    unsigned char pixels[8] = {1, 2, 3, 4};
    unsigned char output[8] = {77, 77, 77, 77};
    kernel_draw2_row(pixels, output, 0, 4, 0, 0, 1 << 16, 0,
                     0x010000, 0, 2, 55, 2);
    assert(output[0] == 55 && output[1] == 55);
    memset(output, 77, sizeof(output));
    kernel_draw2_row(pixels, output, 1, 4, 0, 0, 1 << 16, 0,
                     0x010000, 0, 2, 55, 3);
    assert(output[0] == 2 && output[1] == 3 && output[2] == 4);
}

static void check_guarded_draw2_fill_read() {
    const long page = sysconf(_SC_PAGESIZE);
    assert(page > 0);
    unsigned char *mapping = static_cast<unsigned char *>(mmap(
        0, static_cast<size_t>(page) * 2, PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    assert(mapping != MAP_FAILED);
    assert(mprotect(mapping + page, static_cast<size_t>(page), PROT_NONE) == 0);
    unsigned char *pixels = mapping + page;
    unsigned char *output = static_cast<unsigned char *>(mmap(
        0, static_cast<size_t>(page), PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    assert(output != MAP_FAILED);
    output[0] = 77;
    const pid_t child = fork();
    assert(child >= 0);
    if (child == 0) {
        kernel_draw2_row(pixels, output, 0, 4, 0, 0, 1 << 16, 0,
                         0, 0, 0, 55, 1);
        _exit(0);
    }
    int status = 0;
    assert(waitpid(child, &status, 0) == child);
    assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV);
    assert(output[0] == 77); // initial source load precedes fill-row store
    munmap(output, static_cast<size_t>(page));
    munmap(mapping, static_cast<size_t>(page) * 2);
}

static void check_guarded_inline_terminal_read(int flavor) {
    const long page = sysconf(_SC_PAGESIZE);
    assert(page > 0);
    unsigned char *mapping = static_cast<unsigned char *>(mmap(
        0, static_cast<size_t>(page) * 2, PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    assert(mapping != MAP_FAILED);
    assert(mprotect(mapping + page, static_cast<size_t>(page), PROT_NONE) == 0);
    unsigned char *pixels = mapping + page - 1;
    pixels[0] = 31;
    unsigned char *output = static_cast<unsigned char *>(mmap(
        0, static_cast<size_t>(page), PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    assert(output != MAP_FAILED);
    output[0] = 77;
    const pid_t child = fork();
    assert(child >= 0);
    if (child == 0) {
        if (flavor == 0) {
            kernel_raw_draw_trans(pixels, output, 0, 1, 1, 0, 0,
                                 1 << 16, 0, 0, 0, 0, 0, 0, 1, 0);
        } else if (flavor == 1) {
            kernel_raw_draw_color_key(pixels, output, 0, 1, 1, 0, 0,
                                      1 << 16, 0, 0, 0, 31, 0, 0, 1, 0);
        } else if (flavor == 2) {
            kernel_cutoff_draw_below_cutoff(
                pixels, output, 0, 1, 1, 0, 0, 1 << 16, 0,
                0, 0, 0, 0, 1, 1, 0);
        } else if (flavor == 3) {
            kernel_alternate_draw_color_key_alt(
                pixels, output, 0, 1, 1, 0, 0, 1 << 16, 0,
                0, 0, 0, 99, 0, 1, 0);
        } else if (flavor == 4) {
            kernel_draw2_row(pixels, output, 1, 1, 0, 0, 1 << 16, 0,
                             0, 0, 1, 55, 1);
        } else {
            kernel_cutoff_draw_below_cutoff(
                pixels, output, 0, 1, 1, 0, 0, 1 << 16, 0,
                0x0100, 0, 0, 0, 1, 1, 0);
        }
        _exit(0);
    }
    int status = 0;
    assert(waitpid(child, &status, 0) == child);
    assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV);
    const unsigned char expected = flavor == 1 || flavor == 5 ? 77
        : flavor == 3 ? 99 : 31;
    assert(output[0] == expected); // destination write precedes terminal read fault
    munmap(output, static_cast<size_t>(page));
    munmap(mapping, static_cast<size_t>(page) * 2);
}

int run_texture_span_prefetch_controls() {
    const struct rlimit no_core = {0, 0};
    setrlimit(RLIMIT_CORE, &no_core);
    check_helper_sequences();
    check_guarded_after_advance(false);
    check_guarded_after_advance(true);
    check_guarded_delayed_sample();
    check_inline_loops();
    check_draw2_initial_fill_read();
    check_guarded_draw2_fill_read();
    for (int flavor = 0; flavor < 6; ++flavor)
        check_guarded_inline_terminal_read(flavor);
    return 0;
}
'''


def _compile_and_run(source: str, compiler: str, tmp_path: Path,
                     stem: str) -> subprocess.CompletedProcess:
    tool = shutil.which(compiler)
    if tool is None:
        pytest.skip(f"{compiler} unavailable")
    cpp = tmp_path / f"{stem}.cpp"
    binary = tmp_path / stem
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [tool, "-std=c++11", "-O0", "-Wall", "-Wextra", "-no-pie",
         str(cpp), "-o", str(binary)],
        cwd=ROOT, check=False, capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], cwd=ROOT, check=False,
                          capture_output=True, text=True, timeout=30)


def test_actual_span_kernels_preserve_distinct_read_order(
    tmp_path: Path,
) -> None:
    source = SOURCE.read_text(encoding="utf-8")
    result = _compile_and_run(_translation_unit(source, MAIN), "g++", tmp_path,
                              "texture_span_prefetch_actual")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize(
    "mutations",
    (
        {"helpers": [("texture_draw_lit_span_mode",
                      "++destination;\n        source_texel = source[source_offset];",
                      "++destination;\n        source_texel = 0;")]},
        {"helpers": [("texture_draw_lit_span",
                      "source_texel = source[source_offset];\n        source_offset =",
                      "source_texel = 0;\n        source_offset =")]},
        {"kernels": [(0, "++out;\n                    texel = source_reads[source];",
                      "++out;\n                    texel = 0;")]},
        {"kernels": [
            (4, "unsigned char texel = source_reads[source];",
             "unsigned char texel = 0;"),
            (4, "} else {\n                        unsigned int uf =",
             "} else {\n                        texel = source_reads[source];\n"
             "                        unsigned int uf ="),
        ]},
    ),
)
def test_compiling_prefetch_mutants_are_rejected(
    mutations: dict[str, list[tuple]], tmp_path: Path,
) -> None:
    source = SOURCE.read_text(encoding="utf-8")
    result = _compile_and_run(_translation_unit(source, MAIN, mutations),
                              "g++", tmp_path, "texture_span_prefetch_mutant")
    assert result.returncode != 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_draw2_initial_texture_read_precedes_fill_branch(
    compiler: str, tmp_path: Path,
) -> None:
    source = SOURCE.read_text(encoding="utf-8")
    result = _compile_and_run(_translation_unit(source, MAIN), compiler, tmp_path,
                              f"texture_span_draw2_{compiler}")
    assert result.returncode == 0, result.stdout + result.stderr


def test_source_records_each_pinned_order_and_keeps_opaque_helper_separate() -> None:
    source = SOURCE.read_text(encoding="utf-8")
    delayed = _definition(source, "texture_draw_lit_span")
    store = delayed.index("destination[0] =")
    reload = delayed.index("source_texel = source[source_offset];", store)
    advance = delayed.index("source_offset = texture_draw_next_source_index", reload)
    assert store < reload < advance
    ordinary = _definition(source, "texture_draw_lit_span_mode")
    store = ordinary.index("if (!keyed || source_texel != key)")
    advance = ordinary.index("source_offset = texture_draw_next_source_index", store)
    reload = ordinary.index("source_texel = source[source_offset];", advance)
    assert store < advance < reload
    assert ordinary.index("light_accumulator = static_cast<unsigned short>(", store) < advance
    opaque = _definition(source, "texture_draw_opaque_lit_span")
    assert "source_texel = source[source_offset];" in opaque
    assert "texture_draw_lit_span_mode" not in opaque

    for name, (signature, _) in METHODS.items():
        body = _braced(source, signature)
        assert "const volatile unsigned char *source_reads = pixels;" in body
        assert "texel = source_reads[source];" in body
    draw2 = _braced(
        source,
        "void Texture::draw2(Buffer *buffer, Vert *vertices, int *zvals, int count,\n"
        "                    Vert *uv, int cutoff, int fillColor) {",
    )
    assert draw2.index("texel = source_reads[source];") < draw2.index(
        "if ((y & 1) == 0) {"
    )
    assert "texture_draw_keyed_lit_span(" in _braced(
        source,
        "void Texture::draw_trans(Buffer *buffer, Vert *vertices, int *zvals,\n"
        "                         int count, Vert *uv) {",
    )
    mixed_lit = _braced(
        source,
        "void Texture::draw(Buffer *buffer, Vert *vertices, int *zvals, int count,\n"
        "                   Vert *edge_vertices, int fill_color) {",
    )
    assert "texture_draw_lit_span(" in mixed_lit
