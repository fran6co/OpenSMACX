"""Execute ReportWin's actual satellite animation body with a host fixture."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def _extract_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
    raise AssertionError(f"unterminated body for {signature}")


def _actual_sat_anim_source() -> str:
    header = (ROOT / "src/reportwin.h").read_text()
    source = (ROOT / "src/reportwin.cpp").read_text()
    assert "RECOVERY_MEMBER: v1 ReportWin satellite_index_ 0xE29C 0x0049FE40" in header
    assert "int satellite_index_;  // 0xE29C" in header
    assert "uint32_t field_E29C_" not in header
    return _extract_body(source, "void ReportWin::sat_anim()")


HOST_FIXTURE = r"""
#include <cmath>
#include <cstdio>

struct RECT { int left, top, right, bottom; };
struct ReportWin;

static int failures = 0;
static void check(bool condition, const char *message) {
    if (!condition) {
        std::fprintf(stderr, "failed: %s\n", message);
        ++failures;
    }
}

struct Buffer {
    ReportWin *owner;
    void draw(Buffer *source, int style, int x, int y);
};

struct Flic {
    ReportWin *owner;
    void decode_stored_frame();
};

struct ReportWin : Buffer {
    void sat_anim();
    void fill(RECT *rect, int style);
    void fill(int x, int y, int width, int height, int color);
    void soft_update(RECT *rect);

    int satellite_index_;
    float satellite_x_[16];
    float satellite_y_[16];
    float satellite_dx_[16];
    float satellite_dy_[16];
    float satellite_acceleration_[16];
    int satellite_live_[16];
    float field_EE30_;
    int field_E2A0_;
    int field_E2A4_;
    int field_E2A8_;
    int field_E2AC_;
    RECT satellite_rect_;
    Flic flic_;
    Buffer buffer2_;

    int decode_index;
    int draw_index;
    int soft_update_index;
    int rect_fill_count;
    int marker_count;
    int marker_indices[16];
    int marker_x[16];
    int marker_y[16];

    ReportWin() : Buffer(), satellite_index_(0), field_EE30_(0.25f),
                  field_E2A0_(17), field_E2A4_(23), field_E2A8_(100),
                  field_E2AC_(200), satellite_rect_{1, 2, 3, 4},
                  decode_index(-1), draw_index(-1), soft_update_index(-1),
                  rect_fill_count(0), marker_count(0) {
        owner = this;
        flic_.owner = this;
        buffer2_.owner = this;
    }
};

void Flic::decode_stored_frame() {
    owner->decode_index = owner->satellite_index_;
    // Model a callback-visible write between the repeated member accesses.
    owner->satellite_index_ = 7;
    owner->satellite_live_[15] = 1;
    owner->satellite_x_[15] = 25.75f;
    owner->satellite_y_[15] = 34.75f;
}

void Buffer::draw(Buffer *source, int style, int x, int y) {
    check(source == static_cast<Buffer *>(owner), "buffer draw source is this ReportWin");
    check(style == 9 && x == 17 && y == 23, "buffer draw retains original arguments");
    owner->draw_index = owner->satellite_index_;
    // A second callback mutation is visible to the following member reset.
    owner->satellite_index_ = 11;
    owner->satellite_live_[2] = 1;
    owner->satellite_x_[2] = 12.75f;
    owner->satellite_y_[2] = 22.75f;
}

void ReportWin::fill(RECT *rect, int style) {
    check(rect == &satellite_rect_ && style == 0x41,
          "orbit rectangle is cleared before integration");
    ++rect_fill_count;
}

void ReportWin::fill(int x, int y, int width, int height, int color) {
    check(width == 2 && height == 2 && color == 0xFF,
          "live satellite marker retains 2x2 white fill");
    if (marker_count < 16) {
        marker_indices[marker_count] = satellite_index_;
        marker_x[marker_count] = x;
        marker_y[marker_count] = y;
    }
    ++marker_count;
}

void ReportWin::soft_update(RECT *rect) {
    check(rect == &satellite_rect_, "published rectangle is the orbit rectangle");
    soft_update_index = satellite_index_;
}
"""


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_sat_anim_uses_signed_shared_cursor_and_publishes_all_live_markers(
    tmp_path, compiler_name
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is not installed")

    body = _actual_sat_anim_source()
    main = r"""
int main() {
    ReportWin window;
    float expected_x[16], expected_y[16], expected_dx[16], expected_dy[16];
    for (int i = 0; i < 16; ++i) {
        window.satellite_x_[i] = expected_x[i] = static_cast<float>(i + 10);
        window.satellite_y_[i] = expected_y[i] = static_cast<float>(i + 20);
        window.satellite_dx_[i] = expected_dx[i] = static_cast<float>(i + 1);
        window.satellite_dy_[i] = expected_dy[i] = static_cast<float>(i + 2);
        window.satellite_acceleration_[i] = 0.5f;
        window.satellite_live_[i] = (i % 4 == 0);

        expected_dx[i] -= 0.5f * expected_x[i];
        expected_dy[i] -= 0.5f * expected_y[i];
        expected_x[i] += expected_dx[i] * window.field_EE30_;
        expected_y[i] += expected_dy[i] * window.field_EE30_;
    }

    window.sat_anim();
    expected_x[2] = 12.75f;
    expected_y[2] = 22.75f;
    expected_x[15] = 25.75f;
    expected_y[15] = 34.75f;

    check(window.rect_fill_count == 1, "one initial rectangle clear");
    check(window.decode_index == 16, "decoder observes completed first loop cursor");
    check(window.draw_index == 7, "buffer callback observes decoder's shared-cursor write");
    check(window.soft_update_index == 16, "publication observes completed marker loop cursor");
    check(window.satellite_index_ == 16, "member retains post-loop cursor value");
    for (int i = 0; i < 16; ++i) {
        check(std::fabs(window.satellite_dx_[i] - expected_dx[i]) < 0.0001f,
              "all sixteen x velocities are integrated");
        check(std::fabs(window.satellite_dy_[i] - expected_dy[i]) < 0.0001f,
              "all sixteen y velocities are integrated");
        check(std::fabs(window.satellite_x_[i] - expected_x[i]) < 0.0001f,
              "all sixteen x positions are integrated");
        check(std::fabs(window.satellite_y_[i] - expected_y[i]) < 0.0001f,
              "all sixteen y positions are integrated");
    }

    int expected_markers = 0;
    for (int i = 0; i < 16; ++i) {
        if (window.satellite_live_[i]) {
            check(expected_markers < window.marker_count,
                  "each callback-updated live satellite is painted");
            if (expected_markers < window.marker_count) {
                check(window.marker_indices[expected_markers] == i,
                      "marker reads the current shared member index");
                check(window.marker_x[expected_markers]
                          == static_cast<int>(window.satellite_x_[i]) + 100,
                      "marker uses the live x value after callbacks");
                check(window.marker_y[expected_markers]
                          == static_cast<int>(window.satellite_y_[i]) + 200,
                      "marker uses the live y value after callbacks");
            }
            ++expected_markers;
        }
    }
    check(window.marker_count == expected_markers,
          "only live satellites receive markers");
    return failures == 0 ? 0 : 1;
}
"""
    cpp = tmp_path / f"satellite-cursor-{compiler_name}.cpp"
    executable = tmp_path / f"satellite-cursor-{compiler_name}"
    cpp.write_text(HOST_FIXTURE + "\n" + body + "\n" + main)
    compiled = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(executable)],
        text=True,
        capture_output=True,
    )
    assert compiled.returncode == 0, compiled.stderr
    ran = subprocess.run([str(executable)], text=True, capture_output=True)
    assert ran.returncode == 0, ran.stderr
