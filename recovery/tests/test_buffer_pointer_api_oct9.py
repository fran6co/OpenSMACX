"""Exercise the actual zero-argument Buffer pixel-storage accessor."""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = Path(__file__).resolve().parents[2]
HEADER = ROOT / "src/buffer.h"

HARNESS = r'''#include <cassert>
#include <cstdint>
#include <cstring>

static const unsigned int DDLOCK_WAIT = 1;
struct DDSURFACEDESC {
  unsigned int dwSize;
  int lPitch;
  void *lpSurface;
  unsigned char rest[48];
};
struct IDirectDrawSurface {
  int result;
  int pitch;
  void *pixels;
  int calls;
  int Lock(void *window, DDSURFACEDESC *description, unsigned int flags,
           void *event) {
    assert(window == nullptr && event == nullptr && flags == DDLOCK_WAIT);
    assert(description->dwSize == sizeof(*description));
    ++calls;
    if (result != 0) return result;
    description->lPitch = pitch;
    description->lpSurface = pixels;
    return 0;
  }
};

class Buffer {
 public:
  IDirectDrawSurface *surface_;
  void *locked_bits_;
  void *dib_bits_;
  int surface_lock_count_;
  unsigned int stride_;
  ACTUAL_GET_DATA
};

static unsigned char dib_storage[32];
static unsigned char surface_storage[64];

int main() {
  Buffer dib = {};
  dib.dib_bits_ = dib_storage + 3;
  assert(dib.get_data() == dib_storage + 3);
  assert(dib.locked_bits_ == dib_storage + 3 && dib.surface_lock_count_ == 1);
  assert(dib.get_data() == dib_storage + 3 && dib.surface_lock_count_ == 2);

  Buffer empty = {};
  assert(empty.get_data() == nullptr);
  assert(empty.locked_bits_ == nullptr && empty.surface_lock_count_ == 0);

  IDirectDrawSurface surface = {0, 19, surface_storage + 7, 0};
  Buffer already_locked = {};
  already_locked.surface_ = &surface;
  already_locked.locked_bits_ = surface_storage + 2;
  already_locked.surface_lock_count_ = 4;
  assert(already_locked.get_data() == surface_storage + 2);
  assert(already_locked.surface_lock_count_ == 5 && surface.calls == 0);

  Buffer lock = {};
  lock.surface_ = &surface;
  lock.stride_ = 99;
  assert(lock.get_data() == surface_storage + 7);
  assert(lock.locked_bits_ == surface_storage + 7);
  assert(lock.surface_lock_count_ == 1 && lock.stride_ == 19);
  assert(surface.calls == 1);

  IDirectDrawSurface failing = {7, 23, surface_storage + 11, 0};
  Buffer failure = {};
  failure.surface_ = &failing;
  failure.dib_bits_ = dib_storage + 9;
  failure.stride_ = 101;
  assert(failure.get_data() == nullptr);
  assert(failing.calls == 1 && failure.surface_lock_count_ == 0);
  assert(failure.stride_ == 101 && failure.locked_bits_ == nullptr);
  return 0;
}
'''


def _actual_method() -> str:
    source = HEADER.read_text(encoding="utf-8")
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " "
                               for char in match[0]), source
    )
    signature = "void *get_data()"
    start = masked.index(signature)
    opening = masked.index("{", start + len(signature))
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError("unterminated Buffer::get_data body")


def _run(method: str, compiler: str, directory: Path, name: str) -> int:
    translation_unit = HARNESS.replace("ACTUAL_GET_DATA", method)
    cpp = directory / f"{name}.cpp"
    executable = directory / name
    cpp.write_text(translation_unit, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", str(cpp), "-o", str(executable)],
        capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    completed = subprocess.run([str(executable)], capture_output=True,
                               text=True, timeout=20)
    return completed.returncode


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_get_data_returns_the_backing_pointer_and_keeps_lock_order(
    compiler: str, tmp_path: Path,
) -> None:
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} unavailable")
    assert _run(_actual_method(), executable, tmp_path, "buffer_get_data") == 0


def test_actual_get_data_controls_reject_pointer_and_failure_mutants(
    tmp_path: Path,
) -> None:
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    body = _actual_method()
    mutations = (
        ("return description.lpSurface;", "return dib_bits_;"),
        ("++surface_lock_count_;\n          stride_ = description.lPitch;",
         "stride_ = description.lPitch;"),
        ("if (surface_->Lock(nullptr, &description, DDLOCK_WAIT, nullptr) != 0) {\n              return 0;",
         "if (surface_->Lock(nullptr, &description, DDLOCK_WAIT, nullptr) != 0) {\n              return dib_bits_;"),
    )
    with tempfile.TemporaryDirectory(prefix="buffer-get-data-host-") as temp:
        directory = Path(temp)
        for index, (old, new) in enumerate(mutations):
            mutant = body.replace(old, new, 1)
            assert mutant != body
            assert _run(mutant, compiler, directory,
                        f"buffer_get_data_mutant_{index}") != 0

