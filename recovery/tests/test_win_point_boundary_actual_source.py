"""C++98 actual-source controls for Win point helpers in the reunited owner."""

from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path

import pytest
from match_recovery.source import read_file
from match_recovery.source.reader import function_line, _region_end


ADDRESSES = {
    "client": 0x005ED240,
    "screen": 0x005ED2D0,
    "nonclient": 0x005ED3F0,
}
SIGNATURES = {
    "client": "Win::client_to_screen",
    "screen": "Win::screen_to_client",
    "nonclient": "Win::nonclient_to_screen",
}
NAMES = {
    "client": "?client_to_screen@Win@@QAEXPAH0@Z",
    "screen": "?screen_to_client@Win@@QAEXPAHPAH@Z",
    "nonclient": "?nonclient_to_screen@Win@@QAEXPAH0@Z",
}
SPANS = {
    "client": ((0x005ED240, 0x005ED2C5),),
    "screen": ((0x005ED2D0, 0x005ED355),),
    "nonclient": ((0x005ED3F0, 0x005ED48D),),
}


def _root() -> Path:
    for parent in Path(__file__).resolve().parents:
        if (parent / "src/win.cpp").is_file():
            return parent
    raise AssertionError("could not find repository root")


def _actual_bodies() -> dict[str, str]:
    root = _root()
    owner = root / "src/win.cpp"
    source = owner.read_text(encoding="utf-8")
    rows = {row.address: row for row in read_file(owner)}
    retired_split = root / "src/win_point_coordinates.cpp"
    assert not retired_split.exists(), "point-only split TU should be retired"

    bodies: dict[str, str] = {}
    for key, address in ADDRESSES.items():
        row = rows[address]
        assert row.name == NAMES[key]
        assert row.image_spans == SPANS[key]
        assert row.byte_exact and not row.semantic
        assert Path(row.path).resolve() == owner.resolve()
        # Resolve the overload only inside its selected mapped row. The full
        # Win TU also contains same-named RECT overloads.
        next_line = min((candidate.line for candidate in rows.values()
                         if candidate.line > row.line), default=None)
        start = function_line(source, SIGNATURES[key], after_line=row.line,
                              before_line=next_line)
        lines = source.splitlines()
        end = _region_end(lines, start - 1, SIGNATURES[key])
        assert end is not None, "native reader could not bound " + SIGNATURES[key]
        body = "".join(source.splitlines(keepends=True)[start - 1:end + 1])
        assert body in row.region, "actual definition must remain in its mapped row"
        assert body in source, "actual definition must remain in the compiled Win owner"
        bodies[key] = body

    assert all(rows[address].kind == "game" for address in ADDRESSES.values())
    return bodies


def _source_boundary_checks() -> None:
    root = _root()
    header = (root / "src/win.h").read_text(encoding="utf-8")
    cmake = (root / "CMakeLists.txt").read_text(encoding="utf-8")
    assert re.search(r"\bvoid\s+client_to_screen\s*\(\s*int\s*\*\s*x\s*,\s*int\s*\*\s*y\s*\)\s*;", header)
    assert re.search(r"\bvoid\s+screen_to_client\s*\(\s*int\s*\*\s*x\s*,\s*int\s*\*\s*y\s*\)\s*;", header)
    assert re.search(r"\bvoid\s+nonclient_to_screen\s*\(\s*int\s*\*\s*x\s*,\s*int\s*\*\s*y\s*\)\s*;", header)
    for declaration in ("void client_to_screen(int *x, int *y);",
                        "void screen_to_client(int *x, int *y);",
                        "void nonclient_to_screen(int * x, int * y);"):
        assert declaration in header
    assert re.search(r"(?m)^\s*src/win\.cpp\s*$", cmake), "combined Win owner must be built"
    for body in _actual_bodies().values():
        assert "__forceinline" not in body and not re.search(r"__declspec\s*\(\s*noinline", body)


HARNESS = r'''#include <assert.h>

struct RECT { int left, top, right, bottom; };
struct Win {
  RECT client_rect_;
  RECT outer_rect_;
  unsigned int iFlags_;
  Win *win_parent_;
  Win() : iFlags_(0), win_parent_(0) {
    client_rect_.left = client_rect_.top = 0;
    client_rect_.right = client_rect_.bottom = 0;
    outer_rect_.left = outer_rect_.top = 0;
    outer_rect_.right = outer_rect_.bottom = 0;
  }
  void client_to_screen(int *, int *);
  void screen_to_client(int *, int *);
  void nonclient_to_screen(int *, int *);
};

static void set_win(Win *w, int cl, int ct, int ol, int ot,
                    unsigned int flags, Win *parent) {
  w->client_rect_.left = cl; w->client_rect_.top = ct;
  w->outer_rect_.left = ol; w->outer_rect_.top = ot;
  w->iFlags_ = flags; w->win_parent_ = parent;
}

'''


MAIN = r'''int main() {
  Win grand, parent, child;
  set_win(&grand, 1, 2, 10, 20, 0, 0);
  set_win(&parent, 3, 4, 30, 40, 0x20U | 0x8000U, &grand);
  set_win(&child, 5, 6, 50, 60, 0x20U | 0x8000U, &parent);

  // Separate x/y outputs walk child -> parent -> grandparent. Parent and
  // child each apply their own 0x8000 subtraction after the recursive call.
  int x = 100, y = 200;
  child.client_to_screen(&x, &y);
  assert(x == 159 && y == 272);
  child.screen_to_client(&x, &y);
  assert(x == 100 && y == 200);

  // nonclient_to_screen invokes the actual client helper then subtracts this
  // receiver's outer origin last.
  x = 1; y = 2;
  child.nonclient_to_screen(&x, &y);
  assert(x == 10 && y == 14);

  // 0x8000 alone does not authorize parent traversal; null parent has the
  // same effect even when both bits are set.
  Win detached, distant;
  set_win(&distant, 100, 200, 300, 400, 0, 0);
  set_win(&detached, 5, 6, 50, 60, 0x8000U, &distant);
  x = 100; y = 200;
  detached.client_to_screen(&x, &y);
  assert(x == 155 && y == 266);
  detached.screen_to_client(&x, &y);
  assert(x == 100 && y == 200);
  detached.iFlags_ = 0x20U | 0x8000U;
  detached.win_parent_ = 0;
  x = 100; y = 200;
  detached.client_to_screen(&x, &y);
  assert(x == 155 && y == 266);

  // Aliased output pointers preserve the helpers' sequential x-then-y stores.
  Win alias;
  set_win(&alias, 3, 7, 5, 11, 0, 0);
  int same = 100;
  alias.client_to_screen(&same, &same);
  assert(same == 126);
  same = 100;
  alias.screen_to_client(&same, &same);
  assert(same == 74);
  same = 100;
  alias.nonclient_to_screen(&same, &same);
  assert(same == 110);
  return 0;
}
'''


def _program(bodies: dict[str, str]) -> str:
    return HARNESS + "\n".join(bodies[key] for key in ("client", "screen", "nonclient")) + "\n" + MAIN


def _compile_run(tmp_path: Path, compiler: str, source: str, label: str,
                 should_pass: bool) -> None:
    cpp = tmp_path / (label + ".cpp")
    exe = tmp_path / label
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                            str(cpp), "-o", str(exe)],
                           capture_output=True, text=True, timeout=45)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
    if should_pass:
        assert ran.returncode == 0, ran.stdout + ran.stderr
    else:
        assert ran.returncode != 0, "compile-success behavior mutant unexpectedly passed"


def _compilers() -> list[str]:
    values = [shutil.which("g++"), shutil.which("clang++")]
    values = [value for value in values if value]
    if not values:
        pytest.skip("host GCC or Clang unavailable")
    return values


def test_native_source_identity_claims_signatures_and_default_boundary() -> None:
    _source_boundary_checks()
    bodies = _actual_bodies()
    assert "win_parent_->client_to_screen(x, y);" in bodies["client"]
    assert "win_parent_->screen_to_client(x, y);" in bodies["screen"]
    assert "client_to_screen(x, y);" in bodies["nonclient"]


@pytest.mark.parametrize("compiler", _compilers())
def test_actual_point_coordinate_bodies_under_nested_parents_and_aliasing(
    tmp_path: Path, compiler: str,
) -> None:
    _compile_run(tmp_path, compiler, _program(_actual_bodies()), "point-coordinate-actual", True)


@pytest.mark.parametrize("compiler", _compilers())
@pytest.mark.parametrize("mutant", ("wrong-offset", "wrong-recursion-guard"))
def test_compile_success_damage_mutants_fail_positive_witness(
    tmp_path: Path, compiler: str, mutant: str,
) -> None:
    bodies = _actual_bodies()
    if mutant == "wrong-offset":
        old = "*x = *x + client_rect_.left + outer_rect_.left;"
        new = "*x = *x + client_rect_.left + outer_rect_.top;"
    else:
        old = "if ((iFlags_ & 0x20U) == 0 || !win_parent_) {"
        new = "if ((iFlags_ & 0x20U) == 0 || win_parent_) {"
    assert bodies["client"].count(old) == 1
    bodies["client"] = bodies["client"].replace(old, new, 1)
    _compile_run(tmp_path, compiler, _program(bodies), "point-coordinate-" + mutant, False)
