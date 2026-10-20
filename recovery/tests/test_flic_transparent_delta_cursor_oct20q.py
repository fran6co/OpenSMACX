"""Exercise the real Flic transparent delta decoder's paired pixel cursors.

The pinned 0x62B190 opaque repeated-word loop writes at 0x62B52C and advances
both output ESI and background EBX by two at 0x62B533/0x62B536. This host test
checks that source-level behavior with later transparent packets; it is not a
VC6 ABI or native-runtime proof.
"""
from pathlib import Path
import shutil
import subprocess


WORKER = Path(__file__).resolve().parents[2]
SOURCE = WORKER / "src/flic.cpp"
SIGNATURE = "void Flic::decode_delta_flc_trans()"


def _definition(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated function: {signature}")


HOST = r"""
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string.h>
#include <vector>

struct RECT { int left, top, right, bottom; };
struct Buffer {
    uint32_t stride_;
    uint8_t *pixels_;
    Buffer() : stride_(0), pixels_(0) {}
    uint8_t *get_data() { return pixels_; }
    uint8_t *get_data(uint32_t x, uint32_t y) {
        return pixels_ + y * stride_ + x;
    }
};

void *mem_get(unsigned int count) { return malloc(count); }

struct Flic {
    uint8_t transparent_index_;
    Buffer background_buffer_;
    RECT update_rect_;
    uint32_t decoded_width_, decoded_height_, palette_offset_;
    Buffer *destination_buffer_;
    uint32_t origin_x_, origin_y_;
    FILE *file_;
    void decode_delta_flc_trans();
    Flic() : transparent_index_(0), decoded_width_(0), decoded_height_(0),
             palette_offset_(0), destination_buffer_(0), origin_x_(0),
             origin_y_(0), file_(0) {
        update_rect_.left = update_rect_.top =
            update_rect_.right = update_rect_.bottom = 0;
    }
};

@@BODY@@

static void put16(std::vector<uint8_t> &bytes, unsigned value) {
    bytes.push_back(static_cast<uint8_t>(value));
    bytes.push_back(static_cast<uint8_t>(value >> 8));
}

static void append_row(std::vector<uint8_t> &bytes, uint8_t low, uint8_t high,
                       bool transparent_repeat) {
    put16(bytes, 2);                 // two packets on this line
    bytes.push_back(1);              // leave one destination/background pixel
    bytes.push_back(0xff);           // repeat one word
    bytes.push_back(low);
    bytes.push_back(high);
    bytes.push_back(0);              // second packet has no extra skip
    if (transparent_repeat) {
        bytes.push_back(0xff);       // repeat the following transparent word
        bytes.push_back(96);
        bytes.push_back(96);
    } else {
        bytes.push_back(1);          // copy one literal word
        bytes.push_back(96);         // + palette offset 3 == transparent 99
        bytes.push_back(40);         // + palette offset 3 == opaque 43
    }
}

static FILE *make_file(const std::vector<uint8_t> &bytes) {
    FILE *file = tmpfile();
    assert(file != 0);
    assert(fwrite(&bytes[0], 1, bytes.size(), file) == bytes.size());
    rewind(file);
    return file;
}

static void initialize(Flic &flic, Buffer &destination,
                       uint8_t *destination_pixels, uint8_t *background_pixels,
                       unsigned width, unsigned height, unsigned stride) {
    memset(destination_pixels, 0xee, stride * height);
    for (unsigned i = 0; i < stride * height * 4; ++i)
        background_pixels[i] = static_cast<uint8_t>(0x30 + i);
    destination.stride_ = stride;
    destination.pixels_ = destination_pixels;
    flic.background_buffer_.stride_ = stride;
    flic.background_buffer_.pixels_ = background_pixels;
    flic.destination_buffer_ = &destination;
    flic.decoded_width_ = width;
    flic.decoded_height_ = height;
    flic.origin_x_ = 0;
    flic.origin_y_ = 0;
    flic.transparent_index_ = 99;
    flic.palette_offset_ = 3;
}

static void decode_and_close(Flic &flic, const std::vector<uint8_t> &bytes) {
    flic.file_ = make_file(bytes);
    flic.decode_delta_flc_trans();
    fclose(flic.file_);
    flic.file_ = 0;
}

static void test_four_word_cases_and_skip() {
    const uint8_t words[4][2] = {
        {10, 20},   // neither byte transparent after palette adjustment
        {10, 96},   // high byte transparent
        {96, 20},   // low byte transparent
        {96, 96}    // both bytes transparent
    };
    for (int mode = 0; mode < 4; ++mode) {
        uint8_t dest_pixels[8];
        uint8_t background[32];
        Buffer destination;
        Flic flic;
        initialize(flic, destination, dest_pixels, background, 6, 1, 8);
        std::vector<uint8_t> bytes;
        put16(bytes, 1);             // one scanline
        append_row(bytes, words[mode][0], words[mode][1], false);
        decode_and_close(flic, bytes);

        const uint8_t adjusted_low =
            static_cast<uint8_t>(words[mode][0] + 3);
        const uint8_t adjusted_high =
            static_cast<uint8_t>(words[mode][1] + 3);
        assert(dest_pixels[0] == 0xee); // initial packet skip remains untouched
        if (adjusted_low == 99 && adjusted_high == 99) {
            assert(dest_pixels[1] == background[1]);
            assert(dest_pixels[2] == background[2]);
        } else if (adjusted_low == 99) {
            assert(dest_pixels[1] == adjusted_high);
            assert(dest_pixels[2] == background[2]);
        } else if (adjusted_high == 99) {
            assert(dest_pixels[1] == background[1]);
            assert(dest_pixels[2] == adjusted_low);
        } else {
            assert(dest_pixels[1] == adjusted_low);
            assert(dest_pixels[2] == adjusted_high);
        }
        assert(dest_pixels[3] == background[3]); // later transparent literal
        assert(dest_pixels[4] == 43);
        assert(flic.update_rect_.left == 1 && flic.update_rect_.right == 6);
        assert(flic.update_rect_.top == 0 && flic.update_rect_.bottom == 1);
    }
}

static void test_later_transparent_repeat_uses_advanced_cursor() {
    uint8_t dest_pixels[8];
    uint8_t background[32];
    Buffer destination;
    Flic flic;
    initialize(flic, destination, dest_pixels, background, 6, 1, 8);
    std::vector<uint8_t> bytes;
    put16(bytes, 1);
    append_row(bytes, 10, 20, true);
    decode_and_close(flic, bytes);
    assert(dest_pixels[3] == background[3]);
    assert(dest_pixels[4] == background[4]);
}

static void test_two_rows_keep_original_cursor_stride_progression() {
    uint8_t dest_pixels[16];
    uint8_t background[64];
    Buffer destination;
    Flic flic;
    initialize(flic, destination, dest_pixels, background, 6, 2, 8);
    std::vector<uint8_t> bytes;
    put16(bytes, 2);
    append_row(bytes, 10, 20, false);
    append_row(bytes, 10, 20, false);
    decode_and_close(flic, bytes);
    // The source cursor advances through row 0's skip/packets, then by the
    // background stride before row 1, as the original code does.
    assert(dest_pixels[11] == background[16]);
    assert(flic.update_rect_.top == 0 && flic.update_rect_.bottom == 2);
}

int main() {
    test_four_word_cases_and_skip();
    test_later_transparent_repeat_uses_advanced_cursor();
    test_two_rows_keep_original_cursor_stride_progression();
    return 0;
}
"""


def _compile_run(tmp_path, body):
    compiler = shutil.which("g++")
    assert compiler, "g++ is required for this host control"
    harness = HOST.replace("@@BODY@@", body)
    (tmp_path / "flic_transparent_cursor.cpp").write_text(harness)
    executable = tmp_path / "flic_transparent_cursor"
    compiled = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         "flic_transparent_cursor.cpp", "-o", str(executable)],
        cwd=tmp_path, capture_output=True, text=True,
    )
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True)


def test_actual_transparent_delta_cursor_and_missing_increment_mutant(tmp_path):
    source = SOURCE.read_text(encoding="utf-8")
    marker = "// ORIGINAL: 0x0062B190 ?decode_delta_flc_trans@Flic@@QAEXXZ"
    assert marker in source
    body = _definition(source, SIGNATURE)
    assert "out += 2;\n                                src += 2;" in body
    actual_dir = tmp_path / "actual"
    actual_dir.mkdir()
    actual = _compile_run(actual_dir, body)
    assert actual.returncode == 0, actual.stderr

    old = ("                                std::memcpy(out, &word, sizeof(word));\n"
           "                                out += 2;\n"
           "                                src += 2;")
    assert body.count(old) == 1
    mutant = body.replace(
        old,
        "                                std::memcpy(out, &word, sizeof(word));\n"
        "                                out += 2;",
        1,
    )
    mutant_dir = tmp_path / "missing_src_advance"
    mutant_dir.mkdir()
    damaged = _compile_run(mutant_dir, mutant)
    assert damaged.returncode != 0
