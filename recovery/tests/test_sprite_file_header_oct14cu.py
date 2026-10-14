"""Actual-source Sprite persistence controls for packed layout and ownership."""
from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
PRODUCT = ROOT / "src" / "sprite.cpp"
SAVE_SIGNATURE = "int Sprite::save(const char *filename)"
LOAD_SIGNATURE = "int Sprite::load(const char *filename)"


def extract_function(source, signature):
    start = source.find(signature)
    assert start >= 0, "mapped Sprite persistence body missing: " + signature
    opening = source.find("{", start)
    depth = 0
    state = "code"
    i = opening
    while i < len(source):
        ch = source[i]
        nxt = source[i + 1] if i + 1 < len(source) else ""
        if state == "code":
            if ch == "/" and nxt == "/": state = "line"; i += 2; continue
            if ch == "/" and nxt == "*": state = "block"; i += 2; continue
            if ch == '"': state = "string"
            elif ch == "'": state = "char"
            elif ch == "{": depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0: return source[start:i + 1]
        elif state == "line":
            if ch == "\n": state = "code"
        elif state == "block":
            if ch == "*" and nxt == "/": state = "code"; i += 2; continue
        else:
            if ch == "\\": i += 2; continue
            if (state == "string" and ch == '"') or (state == "char" and ch == "'"):
                state = "code"
        i += 1
    raise AssertionError("unterminated Sprite persistence body")


def native_record(address):
    rows = [row for row in read_file(PRODUCT) if row.address == address]
    assert len(rows) == 1 and rows[0].state.value == "implemented"
    return rows[0]


def header_block(source):
    start = source.index("#pragma pack(push, 1)")
    end_marker = 'static_assert(sizeof(SpriteFileHeader) == 0x1D, "packed Sprite file header");'
    end = source.index(end_marker, start) + len(end_marker)
    return source[start:end]


def actual_sources():
    source = PRODUCT.read_text(encoding="utf-8")
    native_record(0x005EAF8F)
    native_record(0x005EB0BF)
    return (header_block(source), extract_function(source, SAVE_SIGNATURE),
            extract_function(source, LOAD_SIGNATURE))


FIXTURE = r"""
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define static_assert(condition, message) typedef char sprite_header_check[(condition) ? 1 : -1]

static void fail(const char *message) {
    fprintf(stderr, "SEMANTIC97: %s\n", message);
    std::exit(97);
}

// HEADER_BLOCK_INSERTION_POINT

static unsigned char mapped_file[512];
static const char *expected_filename;
static int fail_create, fail_open;
static int create_calls, open_calls, close_calls, destructor_calls;
static int last_requested_size, lifecycle_sequence, last_close_sequence, last_destructor_sequence;
static int free_calls, SpriteMemoryUsed;

class Filemap {
public:
    Filemap() {}
    ~Filemap() {
        ++destructor_calls;
        last_destructor_sequence = ++lifecycle_sequence;
    }
    void *create(const char *filename, uint32_t size, int mode) {
        ++create_calls;
        if (filename != expected_filename) fail("save filename pointer changed");
        if (mode != 1) fail("save file mode changed");
        last_requested_size = (int)size;
        if (fail_create) return 0;
        return mapped_file;
    }
    void *open(const char *filename, int mode) {
        ++open_calls;
        if (filename != expected_filename) fail("load filename pointer changed");
        if (mode != 1) fail("load file mode changed");
        if (fail_open) return 0;
        return mapped_file;
    }
    void close() {
        ++close_calls;
        last_close_sequence = ++lifecycle_sequence;
    }
};
static void tracked_free(void *p) {
    if (p != 0) ++free_calls;
    std::free(p);
}
#define free tracked_free

class Sprite {
public:
    int iSpriteWidth_, iSpriteHeight_, iSpriteWidth2_;
    int iWidth_, iHeight_, iLeftOffset_, iTopOffset_;
    unsigned char cTransparentIndex_;
    unsigned char *pcBits_;
    char *ppszFileName_;
    int fObj1Exists_;
    Sprite() : iSpriteWidth_(0), iSpriteHeight_(0), iSpriteWidth2_(0),
        iWidth_(0), iHeight_(0), iLeftOffset_(0), iTopOffset_(0),
        cTransparentIndex_(0), pcBits_(0), ppszFileName_(0), fObj1Exists_(0) {}
    int save(const char *filename);
    int load(const char *filename);
};

// SAVE_BODY_INSERTION_POINT
// LOAD_BODY_INSERTION_POINT

static void reset_filemap() {
    memset(mapped_file, 0xCC, sizeof(mapped_file));
    fail_create = fail_open = 0;
    create_calls = open_calls = close_calls = destructor_calls = 0;
    last_requested_size = lifecycle_sequence = 0;
    last_close_sequence = last_destructor_sequence = 0;
    free_calls = 0;
}
static void require(bool ok, const char *message) {
    if (!ok) fail(message);
}
static void initialize_sprite(Sprite &s, unsigned char *pixels) {
    s.iSpriteWidth_ = 3; s.iSpriteHeight_ = 2; s.iSpriteWidth2_ = 5;
    s.iWidth_ = 7; s.iHeight_ = 8; s.iLeftOffset_ = -2; s.iTopOffset_ = 4;
    s.cTransparentIndex_ = 9; s.pcBits_ = pixels;
}
static int save_success() {
    reset_filemap();
    char filename[] = "sprite.dat";
    unsigned char pixels[32];
    int i;
    for (i = 0; i < 32; ++i) pixels[i] = (unsigned char)(i + 1);
    Sprite s; initialize_sprite(s, pixels); expected_filename = filename;
    require(s.save(filename) == 0, "save success result changed");
    require(create_calls == 1 && last_requested_size == 50,
            "save allocation size is not width*height+0x2c");
    require(close_calls == 1 && destructor_calls == 1 &&
            last_close_sequence < last_destructor_sequence,
            "save explicit close/destructor order changed");
    require(sizeof(SpriteFileHeader) == 0x1D &&
            offsetof(SpriteFileHeader, stride) == 0x19,
            "packed Sprite header layout changed");
    {
        const SpriteFileHeader *h = (const SpriteFileHeader *)mapped_file;
        if (h->sprite_width != 3 || h->sprite_height != 2 || h->width != 7 ||
            h->height != 8 || h->left_offset != -2 || h->top_offset != 4 ||
            h->transparent_index != 9 || h->stride != 5)
            fail("saved Sprite metadata order changed");
    }
    {
        const int expected[6] = { 3, 2, 7, 8, -2, 4 };
        int raw;
        int offset;
        for (offset = 0; offset != 6; ++offset) {
            memcpy(&raw, mapped_file + offset * 4, 4);
            if (raw != expected[offset])
                fail("raw Sprite header offsets changed");
        }
        if (mapped_file[0x18] != 9) fail("raw transparency offset changed");
        memcpy(&raw, mapped_file + 0x19, 4);
        if (raw != 5) fail("raw unaligned stride offset changed");
    }
    if (memcmp(mapped_file + 0x1D, pixels, 10) != 0)
        fail("save pixel payload extent changed");
    if (mapped_file[0x1D + 10] != 0xCC)
        fail("save copied width*height instead of height*stride");
    return 0;
}
static int save_early_returns() {
    char filename[] = "empty.dat";
    reset_filemap(); expected_filename = filename;
    Sprite no_pixels;
    require(no_pixels.save(filename) == 7, "null-pixel save result changed");
    require(create_calls == 0 && destructor_calls == 1 && close_calls == 0,
            "null-pixel save Filemap lifetime changed");
    reset_filemap(); expected_filename = filename; fail_create = 1;
    unsigned char pixels[8] = {0}; Sprite no_mapping; initialize_sprite(no_mapping, pixels);
    require(no_mapping.save(filename) == 4, "failed-create save result changed");
    require(create_calls == 1 && destructor_calls == 1 && close_calls == 0,
            "failed-create save Filemap lifetime changed");
    return 0;
}
static void prepare_raw_load_fixture(const unsigned char *pixels) {
    const int fields[6] = { 3, 2, 7, 8, -2, 4 };
    int i;
    for (i = 0; i != 6; ++i) memcpy(mapped_file + i * 4, &fields[i], 4);
    mapped_file[0x18] = 9;
    {
        const int stride = 5;
        memcpy(mapped_file + 0x19, &stride, 4);
    }
    memcpy(mapped_file + 0x1D, pixels, 10);
}
static int load_borrowed_success() {
    char filename[] = "sprite.dat";
    unsigned char pixels[32];
    int i;
    for (i = 0; i < 32; ++i) pixels[i] = (unsigned char)(i + 1);
    reset_filemap(); expected_filename = filename;
    prepare_raw_load_fixture(pixels);
    unsigned char borrowed[32]; memset(borrowed, 0x55, sizeof(borrowed));
    Sprite target;
    target.pcBits_ = borrowed; target.fObj1Exists_ = 1;
    target.ppszFileName_ = (char *)std::malloc(16);
    target.iSpriteWidth_ = 3; target.iSpriteHeight_ = 2;
    target.iWidth_ = 1; target.iHeight_ = 1;
    SpriteMemoryUsed = 100;
    const int frees_before = free_calls;
    require(target.load(filename) == 0, "borrowed load result changed");
    require(target.pcBits_ == borrowed && target.fObj1Exists_ == 0,
            "load replaced or allocated borrowed pixel destination");
    require(target.iSpriteWidth_ == 3 && target.iSpriteHeight_ == 2 &&
            target.iWidth_ == 7 && target.iHeight_ == 8 &&
            target.iLeftOffset_ == -2 && target.iTopOffset_ == 4 &&
            target.iSpriteWidth2_ == 5 && target.cTransparentIndex_ == 9,
            "load header field order/transparency/stride changed");
    require(memcmp(borrowed, pixels, 10) == 0,
            "load did not copy height*stride bytes into borrowed destination");
    require(free_calls == frees_before + 1,
            "borrowed load freed pixels or failed to free old filename");
    require(open_calls == 1 && close_calls == 1 && destructor_calls == 1 &&
            last_close_sequence < last_destructor_sequence,
            "load explicit close/destructor order changed");
    return 0;
}
static int load_failed_open_cleanup() {
    reset_filemap();
    char filename[] = "missing.dat"; expected_filename = filename; fail_open = 1;
    Sprite target;
    target.pcBits_ = (unsigned char *)std::malloc(16);
    target.ppszFileName_ = (char *)std::malloc(16);
    target.iSpriteWidth_ = 3; target.iSpriteHeight_ = 2; target.iSpriteWidth2_ = 5;
    target.iWidth_ = 7; target.iHeight_ = 8;
    target.iLeftOffset_ = -2; target.iTopOffset_ = 4;
    target.cTransparentIndex_ = 77; target.fObj1Exists_ = 0;
    SpriteMemoryUsed = 100;
    require(target.load(filename) == 6, "failed-open result changed");
    require(target.pcBits_ == 0 && target.ppszFileName_ == 0 && free_calls == 2,
            "failed-open destructive owned-pixel/name cleanup changed");
    require(SpriteMemoryUsed == 94, "failed-open memory accounting changed");
    require(target.iSpriteWidth_ == 0 && target.iSpriteHeight_ == 0 &&
            target.iSpriteWidth2_ == 0 && target.iWidth_ == 0 && target.iHeight_ == 0 &&
            target.iLeftOffset_ == 0 && target.iTopOffset_ == 0 &&
            target.fObj1Exists_ == 0 && target.cTransparentIndex_ == 77,
            "failed-open fields/transparency cleanup changed");
    require(open_calls == 1 && close_calls == 0 && destructor_calls == 1,
            "failed-open Filemap cleanup sequence changed");
    return 0;
}
int main() {
    require(sizeof(SpriteFileHeader) == 0x1D &&
            offsetof(SpriteFileHeader, stride) == 0x19,
            "packed Sprite header layout changed");
    save_success();
    save_early_returns();
    load_borrowed_success();
    load_failed_open_cleanup();
    puts("Sprite packed file header and ownership controls: PASS");
    return 0;
}
"""


def run_host(tmp_path, compiler, sources, header=None, mutation=None):
    actual_header, save_body, load_body = sources
    if header is not None:
        actual_header = header
    if mutation is not None:
        which, before, after = mutation
        body = save_body if which == "save" else load_body
        assert before in body
        body = body.replace(before, after, 1)
        if which == "save": save_body = body
        else: load_body = body
    projected = (FIXTURE.replace("// HEADER_BLOCK_INSERTION_POINT", actual_header)
                 .replace("// SAVE_BODY_INSERTION_POINT", save_body)
                 .replace("// LOAD_BODY_INSERTION_POINT", load_body))
    source = tmp_path / "sprite-file-header.cpp"
    binary = tmp_path / "sprite-file-header"
    source.write_text(projected, encoding="utf-8")
    built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                            "-Wno-unused-parameter",
                            str(source), "-o", str(binary)], capture_output=True,
                           text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_sprite_file_header_save_load_and_ownership(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    result = run_host(tmp_path, compiler, actual_sources())
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_padded_header_mutant_fails_layout_witness(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    header, save_body, load_body = actual_sources()
    padded = header.replace("#pragma pack(push, 1)", "#pragma pack(push, 4)")
    padded = padded.replace("sizeof(SpriteFileHeader) == 0x1D",
                            "sizeof(SpriteFileHeader) == 0x20")
    result = run_host(tmp_path, compiler, (header, save_body, load_body), header=padded)
    assert result.returncode == 97, (result.returncode, result.stdout, result.stderr)
    assert "SEMANTIC97: packed Sprite header layout changed" in result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_reordered_metadata_mutant_fails_raw_offset_witness(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    header, save_body, load_body = actual_sources()
    before = "    int width;\n    int height;"
    after = "    int height;\n    int width;"
    assert before in header
    reordered = header.replace(before, after, 1)
    result = run_host(tmp_path, compiler, (header, save_body, load_body), header=reordered)
    assert result.returncode == 97, (result.returncode, result.stdout, result.stderr)
    assert "SEMANTIC97: raw Sprite header offsets changed" in result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("which,before,after,diagnostic", [
    ("save", "fm.create(\n        filename,", 'fm.create(\n        "wrong.dat",',
     "save filename pointer changed"),
    ("load", "fm.open(filename, 1)", 'fm.open("wrong.dat", 1)',
     "load filename pointer changed"),
    ("save", "static_cast<uint32_t>(iSpriteHeight_) * iSpriteWidth2_",
     "static_cast<uint32_t>(iSpriteHeight_) * iSpriteWidth_",
     "save pixel payload extent changed"),
    ("load", "* static_cast<uint32_t>(iSpriteWidth_));",
     "* static_cast<uint32_t>(iSpriteWidth2_));",
     "failed-open memory accounting changed"),
])
def test_filename_copy_and_cleanup_mutants_fail_semantically(
        tmp_path, compiler_name, which, before, after, diagnostic):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    result = run_host(tmp_path, compiler, actual_sources(),
                      mutation=(which, before, after))
    assert result.returncode == 97, (result.returncode, result.stdout, result.stderr)
    assert ("SEMANTIC97: " + diagnostic) in result.stderr
