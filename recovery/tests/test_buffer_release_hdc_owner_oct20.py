"""Exercise Buffer::release_hdc from its real out-of-class source body."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
HEADER = ROOT / "src/buffer.h"
SOURCE = ROOT / "src/buffer.cpp"


def _body(source: str, signature: str) -> str:
    start = source.find(signature)
    if start < 0 or source.find(signature, start + len(signature)) >= 0:
        raise AssertionError(f"expected one source definition: {signature}")
    opening = source.find("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError(f"unterminated source definition: {signature}")


SIGNATURE = "MEASURED void Buffer::release_hdc(int count)"
HARNESS = r"""
#include <cassert>

#define MEASURED
#define nullptr 0

struct FakeSurface {
    int result;
    int releases;
    void *last_handle;

    FakeSurface() : result(0), releases(0), last_handle(0) {}
    int ReleaseDC(void *handle) {
        ++releases;
        last_handle = handle;
        return result;
    }
};

struct Buffer {
    FakeSurface *surface_;
    void *hdc2_;
    int hdc_lock_count_;
    int refreshes;
    int count_during_refresh;
    void *handle_during_refresh;

    Buffer() : surface_(0), hdc2_(0), hdc_lock_count_(0), refreshes(0),
               count_during_refresh(0), handle_during_refresh(0) {}
    void release_hdc(int count);
    void get_graphic_win() {
        ++refreshes;
        count_during_refresh = hdc_lock_count_;
        handle_during_refresh = hdc2_;
    }
};

BODY

int main() {
    int handle_token = 0;
    Buffer direct;
    direct.hdc2_ = &handle_token;
    direct.hdc_lock_count_ = 2;
    direct.release_hdc(1);
    assert(direct.hdc_lock_count_ == 1);
    assert(direct.hdc2_ == &handle_token);
    direct.release_hdc(1);
    assert(direct.hdc_lock_count_ == 0);
    assert(direct.hdc2_ == 0);
    assert(direct.refreshes == 0);

    Buffer surface_without_handle;
    FakeSurface unused_surface;
    surface_without_handle.surface_ = &unused_surface;
    surface_without_handle.hdc_lock_count_ = 0;
    surface_without_handle.release_hdc(1);
    assert(surface_without_handle.hdc_lock_count_ == -1);
    assert(surface_without_handle.hdc2_ == 0);
    assert(unused_surface.releases == 0);
    assert(surface_without_handle.refreshes == 0);

    Buffer held;
    FakeSurface surface;
    held.surface_ = &surface;
    held.hdc2_ = &handle_token;
    held.hdc_lock_count_ = 2;
    held.release_hdc(1);
    assert(held.hdc_lock_count_ == 1);
    assert(held.hdc2_ == &handle_token);
    assert(surface.releases == 0);
    held.release_hdc(1);
    assert(surface.releases == 1);
    assert(surface.last_handle == &handle_token);
    assert(held.hdc_lock_count_ == 0);
    assert(held.hdc2_ == 0);

    Buffer failed_release;
    FakeSurface failing_surface;
    failing_surface.result = 1;
    failed_release.surface_ = &failing_surface;
    failed_release.hdc2_ = &handle_token;
    failed_release.hdc_lock_count_ = 1;
    failed_release.release_hdc(1);
    assert(failing_surface.releases == 1);
    assert(failing_surface.last_handle == &handle_token);
    assert(failed_release.refreshes == 1);
    assert(failed_release.count_during_refresh == 0);
    assert(failed_release.handle_during_refresh == &handle_token);
    assert(failed_release.hdc_lock_count_ == 0);
    assert(failed_release.hdc2_ == 0);
}
"""


def _run_body(tmp_path: Path, body: str) -> subprocess.CompletedProcess:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    program = tmp_path / "release_hdc.cpp"
    executable = tmp_path / "release_hdc"
    program.write_text(HARNESS.replace("BODY", body))
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         str(program), "-o", str(executable)],
        text=True, capture_output=True, check=False,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], text=True, capture_output=True,
                          check=False)


def test_release_hdc_is_owned_out_of_line_by_buffer_cpp():
    header = HEADER.read_text()
    source = SOURCE.read_text()
    declaration = "MEASURED void release_hdc(int count);"
    assert declaration in header
    assert "MEASURED void release_hdc(int count) {" not in header
    assert "void release_hdc(int count) {" not in header
    assert source.count(SIGNATURE) == 1
    assert source.index(SIGNATURE) < source.index("int Buffer::set_clip(RECT *rect)")
    assert "get_hdc" in header and "release_hdc" in header


def test_release_hdc_preserves_reference_and_release_protocol(tmp_path):
    body = _body(SOURCE.read_text(), SIGNATURE)
    completed = _run_body(tmp_path, body)
    assert completed.returncode == 0, completed.stdout + completed.stderr


def test_release_hdc_control_rejects_increment_mutant(tmp_path):
    body = _body(SOURCE.read_text(), SIGNATURE)
    decrement = "    hdc_lock_count_ -= count;"
    last = body.rfind(decrement)
    assert last >= 0
    mutant = body[:last] + body[last:].replace(
        decrement, "    hdc_lock_count_ += count;", 1)
    completed = _run_body(tmp_path, mutant)
    assert completed.returncode != 0
