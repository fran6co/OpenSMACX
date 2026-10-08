"""Compiled host controls for recovered Font and Buffer source bodies.

After central integration these tests extract definitions from actual root
``src`` and compare their source mapping metadata with the pinned task base.
The C++ fixture provides only the owning fields and a fake DirectDraw surface;
it compiles and executes the extracted methods, not a transcription of them.
"""
from __future__ import annotations

from pathlib import Path
import re
import subprocess
import tempfile

from match_recovery import source


ROOT = next(p for p in Path(__file__).resolve().parents if (p / 'src/IMAGE_PIN').is_file())
BASELINE = "2c29aa969"


def record_in(text: str, path: str, address: int):
    matches = [row for row in source.read_text(text, path)
               if row.address == address]
    assert len(matches) == 1, f"expected one 0x{address:08X} record in {path}"
    return matches[0]


def baseline_record(path: str, address: int):
    result = subprocess.run(
        ["git", "show", f"{BASELINE}:{path}"], cwd=ROOT, check=True,
        text=True, capture_output=True)
    return record_in(result.stdout, path, address)


def metadata(record):
    return (record.address, record.mode, record.name, record.image_spans,
            record.kind, record.exclusion, record.byte_exact,
            record.semantic, record.levers, record.ruled_out,
            record.unrecoverable, record.deferred)


def actual_record(path: str, address: int):
    return record_in((ROOT / path).read_text(), path, address)


def code_only(text: str) -> str:
    """Blank comments and literals while preserving offsets for C++ matching."""
    out = list(text)
    i = 0
    state = "code"
    quote = ""
    while i < len(text):
        if state == "code":
            if text.startswith("//", i):
                out[i] = out[i + 1] = " "
                i += 2
                state = "line"
                continue
            if text.startswith("/*", i):
                out[i] = out[i + 1] = " "
                i += 2
                state = "block"
                continue
            if text[i] in ("'", '"'):
                quote = text[i]
                out[i] = " "
                i += 1
                state = "literal"
                continue
            i += 1
        elif state == "line":
            if text[i] == "\n":
                state = "code"
            else:
                out[i] = " "
            i += 1
        elif state == "block":
            if text.startswith("*/", i):
                out[i] = out[i + 1] = " "
                i += 2
                state = "code"
            else:
                if text[i] != "\n":
                    out[i] = " "
                i += 1
        else:
            if text[i] == "\\":
                out[i] = " "
                if i + 1 < len(text):
                    if text[i + 1] != "\n":
                        out[i + 1] = " "
                    i += 2
                else:
                    i += 1
            elif text[i] == quote:
                out[i] = " "
                i += 1
                state = "code"
            else:
                if text[i] != "\n":
                    out[i] = " "
                i += 1
    return "".join(out)


def extract_definition(text: str, expression: str) -> str:
    clean = code_only(text)
    match = re.search(expression, clean)
    assert match, f"no C++ definition matching {expression!r}"
    brace = clean.find("{", match.start(), match.end() + 2)
    assert brace >= 0
    depth = 0
    for i in range(brace, len(clean)):
        if clean[i] == "{":
            depth += 1
        elif clean[i] == "}":
            depth -= 1
            if depth == 0:
                return text[match.start():i + 1]
    raise AssertionError("unterminated C++ function body")


FONT_PATTERN = r"\bint\s+Font::line_advance\s*\(\s*\)\s*const\s*\{"
BUFFER_PATTERN = (r"\bint\s+Buffer::fill_line\s*\(\s*int\s+x1\s*,\s*int\s+y1\s*,"
                  r"\s*int\s+x2\s*,\s*int\s+y2\s*,\s*char\s+fill_colour\s*\)\s*\{")


def host_fixture(font_source: str, buffer_source: str) -> str:
    font_method = extract_definition(font_source, FONT_PATTERN)
    buffer_method = extract_definition(buffer_source, BUFFER_PATTERN)
    return r'''#define _GNU_SOURCE
#include <cassert>
#include <cstdint>
#include <cstring>
#include <sys/mman.h>
#include <unistd.h>

typedef void *LPVOID;
struct RECT { int left, top, right, bottom; };
struct DDSURFACEDESC { unsigned dwSize; int lPitch; void *lpSurface; };
static const int DDLOCK_WAIT = 1;

struct IDirectDrawSurface {
    virtual int Lock(void *, DDSURFACEDESC *, int, void *) = 0;
    virtual int Unlock(void *) = 0;
};

struct Font {
    int unk_1_;
    int line_height_;
    int height_;
    int line_advance() const;
};

struct Buffer {
    RECT rect1_;
    LPVOID locked_bits_;
    LPVOID dib_bits_;
    IDirectDrawSurface *surface_;
    int surface_lock_count_;
    unsigned stride_;
    int fill_line(int, int, int, int, char);
};

__FONT_METHOD__

__BUFFER_METHOD__

struct TestSurface : IDirectDrawSurface {
    char *pixels;
    int pitch;
    int lock_result;
    int unlock_result;
    int locks;
    int unlocks;
    bool descriptor_was_initialized;
    void *unlocked_bits;

    TestSurface(char *p, int row_pitch)
      : pixels(p), pitch(row_pitch), lock_result(0), unlock_result(0),
        locks(0), unlocks(0), descriptor_was_initialized(false),
        unlocked_bits(nullptr) {}

    int Lock(void *, DDSURFACEDESC *desc, int flags, void *) override {
        ++locks;
        descriptor_was_initialized = desc->dwSize == 0x6c && flags == DDLOCK_WAIT;
        if (lock_result != 0) return lock_result;
        desc->lPitch = pitch;
        desc->lpSurface = pixels;
        return 0;
    }
    int Unlock(void *bits) override {
        ++unlocks;
        unlocked_bits = bits;
        return unlock_result;
    }
};

static char *low_pixels() {
    void *memory = mmap(nullptr, 4096, PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
    assert(memory != MAP_FAILED);
    assert(reinterpret_cast<uintptr_t>(memory) <= 0x7fffffffU);
    std::memset(memory, 0, 4096);
    return static_cast<char *>(memory);
}

static Buffer make_buffer() {
    Buffer b = {};
    b.rect1_ = RECT{0, 0, 8, 8};
    b.stride_ = 8;
    return b;
}

int main() {
    Font f = {};
    f.line_height_ = 17;
    f.height_ = 0x7fffffff;
    f.unk_1_ = 1;
    assert(f.line_advance() == static_cast<int32_t>(0x80000000U));
    f.height_ = 0x80000000U;
    f.unk_1_ = -1;
    assert(f.line_advance() == 17); // signed jl path, not unsigned test
    f.unk_1_ = 2;
    assert(f.line_advance() == static_cast<int32_t>(0x80000002U));

    char *pixels = low_pixels();
    TestSurface surface(pixels, 8);

    // Successful lock, rasterization, and unlock. The shallow line visits
    // (2,1), (3,2), (4,3); the API's unlock result is the function result.
    Buffer drawn = make_buffer();
    drawn.surface_ = &surface;
    surface.unlock_result = 29;
    assert(drawn.fill_line(2, 1, 4, 3, char(0x5a)) == 29);
    assert(surface.locks == 1 && surface.unlocks == 1);
    assert(surface.descriptor_was_initialized && surface.unlocked_bits == pixels);
    assert(drawn.surface_lock_count_ == 0 && drawn.locked_bits_ == nullptr);
    assert(pixels[10] == 0x5a && pixels[19] == 0x5a && pixels[28] == 0x5a);

    // The original's unusual y2<=y1 disjunction admits this reversed row
    // even though the configured clip excludes x=1..2.
    std::memset(pixels, 0, 4096);
    Buffer reversed = make_buffer();
    reversed.surface_ = &surface;
    reversed.rect1_ = RECT{6, 6, 7, 7};
    surface.unlock_result = 0;
    assert(reversed.fill_line(1, 2, 2, 1, char(0x31)) == 0);
    assert(pixels[17] == 0x31 || pixels[18] == 0x31);

    // A failed Lock returns immediately: no pixel access, unlock, or count.
    std::memset(pixels, 0, 4096);
    Buffer lock_failure = make_buffer();
    lock_failure.surface_ = &surface;
    surface.lock_result = -7;
    const int unlocks_before_failure = surface.unlocks;
    assert(lock_failure.fill_line(2, 1, 4, 3, char(0x44)) == -7);
    assert(lock_failure.surface_lock_count_ == 0 && lock_failure.locked_bits_ == nullptr);
    assert(surface.unlocks == unlocks_before_failure && pixels[10] == 0);
    surface.lock_result = 0;

    // A normally oriented line outside rect1_ is not drawn, but the original
    // still releases the lock acquired before the clip predicate.
    std::memset(pixels, 0, 4096);
    Buffer clipped = make_buffer();
    clipped.surface_ = &surface;
    clipped.rect1_ = RECT{6, 6, 7, 7};
    int unlocks_before_clip = surface.unlocks;
    assert(clipped.fill_line(1, 1, 2, 2, char(0x22)) == 0);
    assert(surface.unlocks == unlocks_before_clip + 1 && pixels[9] == 0);

    // Nested acquisition reuses locked_bits_ and returns its 32-bit pointer
    // value without an additional DirectDraw Lock/Unlock pair.
    Buffer nested = make_buffer();
    nested.surface_ = &surface;
    nested.locked_bits_ = pixels;
    nested.surface_lock_count_ = 1;
    int locks_before_nested = surface.locks;
    int unlocks_before_nested = surface.unlocks;
    const int nested_result = nested.fill_line(0, 0, 0, 0, char(0));
    assert(static_cast<uint32_t>(nested_result) == static_cast<uint32_t>(reinterpret_cast<uintptr_t>(pixels)));
    assert(nested.surface_lock_count_ == 1 && nested.locked_bits_ == pixels);
    assert(surface.locks == locks_before_nested && surface.unlocks == unlocks_before_nested);

    // With no surface, DIB bits are borrowed as the lock and the counter is
    // returned to zero with the pointer cleared; no COM method is involved.
    std::memset(pixels, 0, 4096);
    Buffer dib = make_buffer();
    dib.dib_bits_ = pixels;
    locks_before_nested = surface.locks;
    assert(dib.fill_line(1, 1, 2, 2, char(0x66)) == 0);
    assert(pixels[9] == 0x66 && dib.surface_lock_count_ == 0 && dib.locked_bits_ == nullptr);
    assert(surface.locks == locks_before_nested);

    munmap(pixels, 4096);
    return 0;
}
'''.replace("__FONT_METHOD__", font_method).replace("__BUFFER_METHOD__", buffer_method)


def compile_and_run(font_source: str, buffer_source: str) -> None:
    fixture = host_fixture(font_source, buffer_source)
    with tempfile.TemporaryDirectory(prefix="osmx-graphics-host-") as directory:
        executable = str(Path(directory) / "graphics-host-controls")
        compiled = subprocess.run(
            ["g++", "-std=c++14", "-O0", "-fpermissive", "-x", "c++", "-o", executable, "-"],
            input=fixture, text=True, capture_output=True)
        assert compiled.returncode == 0, compiled.stdout + compiled.stderr
        completed = subprocess.run([executable], text=True, capture_output=True)
        assert completed.returncode == 0, completed.stdout + completed.stderr


def test_source_extractor_ignores_comment_and_string_decoys():
    decoys = ('/* int Font::line_advance() const { return 100; } */\n'
              'const char *s = "int Font::line_advance() const { return 200; }";\n')
    body = extract_definition(
        decoys + "int Font::line_advance() const { return unk_1_ < 0 ? 1 : 2; }",
        FONT_PATTERN)
    assert body == "int Font::line_advance() const { return unk_1_ < 0 ? 1 : 2; }"


def test_extracted_actual_owner_methods_compile_and_preserve_runtime_branches():
    compile_and_run((ROOT / "src/jackal/font.cpp").read_text(),
                    (ROOT / "src/buffer.cpp").read_text())


def test_moved_annotations_preserve_full_baseline_metadata_and_file_mode():
    font = actual_record("src/jackal/font.cpp", 0x005E3650)
    buffer = actual_record("src/buffer.cpp", 0x005E07A0)
    assert metadata(font) == metadata(baseline_record("src/leaf_recoveries.cpp", 0x005E3650))
    assert metadata(buffer) == metadata(baseline_record("src/leaf_recoveries.cpp", 0x005E07A0))
    assert buffer.mode is source.Mode.FILE
    assert font.byte_exact
    assert buffer.image_spans == ((0x005E07A0, 0x005E09CB),)


def test_unresolved_mappings_keep_identity_claims_and_lessons():
    for path, baseline_path, addresses in (
        ("src/texture.cpp", "src/leaf_recoveries.cpp", (0x006252C0,)),
        ("src/table_storage.cpp", "src/leaf_recoveries.cpp", (0x00616B40,)),
        ("src/vendor/zlib.recovery.h", "src/leaf_recoveries.cpp",
         (0x00640A80, 0x00644930)),
        ("src/field_accessors.cpp", "src/field_accessors.cpp", (0x005DA6A0, 0x005DA6B0,
                                      0x005E2460, 0x005E2470,
                                      0x00616D80)),
    ):
        candidate = (ROOT / path).read_text()
        for address in addresses:
            assert metadata(record_in(candidate, path, address)) == metadata(
                baseline_record(baseline_path, address))


def test_texture_terminal_fragment_keeps_identity_without_callable_stub():
    from dataclasses import replace
    from match_recovery.source import Mode, SourceRole

    before = baseline_record("src/field_accessors.cpp", 0x0061F785)
    after = actual_record("src/texture.cpp", 0x0061F785)
    assert metadata(after) == metadata(replace(before, mode=Mode.FILE))
    assert after.source_role is SourceRole.SCAFFOLD
    assert not after.symbol
    assert "fragment measurement remains unresolved" in after.source_role_reason
    assert not any(r.address == 0x0061F785
                   for r in source.read_file(ROOT / "src/field_accessors.cpp"))
    for path in ("src/field_accessors.cpp", "src/field_accessors.h"):
        assert "field_accessor_0061f785_redirect" not in code_only(
            (ROOT / path).read_text())
