"""Source-derived checks for the VC6-compatible static_assert identity."""
from __future__ import annotations

import os
from pathlib import Path
import re

import pytest


ROOT = Path(__file__).resolve().parents[2]
HEADER = ROOT / "src/vc6_compat.h"
IDENTITY = "vc6_static_assert_check_"


def _macro_block(source: str) -> str:
    match = re.search(
        r"(?m)^#define OPENSMACX_ASSERT_JOIN\(a, b\).*?"
        r"^  typedef char [^\n]+\n",
        source,
        re.DOTALL,
    )
    assert match is not None
    return match.group(0)


def test_macro_uses_fixed_typedef_identity_inside_vc6_guard():
    source = HEADER.read_text(encoding="utf-8")
    guard_start = source.index("#if defined(_MSC_VER) && _MSC_VER <= 1200")
    guard_end = source.index("#endif  // _MSC_VER <= 1200", guard_start)
    guarded = source[guard_start:guard_end]
    block = _macro_block(guarded)
    assert "#define OPENSMACX_ASSERT_JOIN(a, b) OPENSMACX_ASSERT_JOIN2(a, b)" in block
    assert "#define OPENSMACX_ASSERT_JOIN2(a, b) a##b" in block
    assert re.search(
        r"#define static_assert\(condition, message\) \\\n"
        r"  typedef char vc6_static_assert_check_\[\(condition\) \? 1 : -1\]",
        block,
    )
    assert "__LINE__" not in block


def test_fixed_typedef_name_does_not_collide_with_product_sources():
    sources = [*ROOT.joinpath("src").rglob("*.h"), *ROOT.joinpath("src").rglob("*.cpp")]
    collisions = [path.relative_to(ROOT).as_posix() for path in sources
                  if path != HEADER and IDENTITY in path.read_text(encoding="utf-8")]
    assert collisions == []


def _positive_fixture() -> str:
    return r'''
static_assert(1, "file scope one");
static_assert(2, "file scope two");
namespace vc6_assertion_identity_probe {
static_assert(1, "namespace one");
static_assert(2, "namespace two");
}
struct vc6_assertion_identity_probe_type {
  static_assert(1, "class one");
  static_assert(2, "class two");
};
void vc6_assertion_identity_probe_function() {
  static_assert(1, "function one");
  static_assert(2, "function two");
}
'''


def _negative_fixture(scope: str) -> str:
    if scope == "file":
        return 'static_assert(0, "expected failure");\n'
    if scope == "namespace":
        return 'namespace vc6_assertion_negative { static_assert(0, "expected failure"); }\n'
    if scope == "class":
        return 'struct vc6_assertion_negative { static_assert(0, "expected failure"); };\n'
    if scope == "function":
        return 'void vc6_assertion_negative() { static_assert(0, "expected failure"); }\n'
    raise AssertionError(scope)


@pytest.mark.skipif(
    os.environ.get("OPENSMACX_LIVE_ASSERTION_CONTROLS") != "1",
    reason="opt-in VC6/Wine assertion identity controls",
)
def test_vc6_repeated_typedefs_and_comment_shift_are_stable():
    from match_recovery.compiler import Compiler
    from match_recovery.measurement import Measurement
    from match_recovery.pe_coff import Coff
    from match_recovery.coff_replica import object_signature

    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    source = ROOT / "src/base.cpp"
    flag_sets = settings.flags_for("src/base.cpp")
    base = source.read_bytes()
    fixture = _positive_fixture().encode("ascii")
    candidate = base + b"\n" + fixture
    shifted = base + b"\n// comment-only line shift\n// second comment-only line\n" + fixture
    for flags in flag_sets:
        # Do not add layout/debug flags here: their source-line metadata is
        # expected to change when comments move, even when code and refs do not.
        actual_flags = tuple(flags)
        first = Coff(compiler.overlay(source, {"src/base.cpp": candidate}, (), actual_flags))
        second = Coff(compiler.overlay(source, {"src/base.cpp": shifted}, (), actual_flags))
        assert object_signature(first) == object_signature(second)


@pytest.mark.parametrize("scope", ("file", "namespace", "class", "function"))
@pytest.mark.skipif(
    os.environ.get("OPENSMACX_LIVE_ASSERTION_CONTROLS") != "1",
    reason="opt-in VC6/Wine assertion identity controls",
)
def test_vc6_false_assertion_fails_in_each_scope(scope: str):
    from match_recovery.compiler import Compiler
    from match_recovery.measurement import Measurement

    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    source = ROOT / "src/base.cpp"
    flag_sets = settings.flags_for("src/base.cpp")
    candidate = source.read_bytes() + b"\n" + _negative_fixture(scope).encode("ascii")
    for flags in flag_sets:
        actual_flags = tuple(flags)
        with pytest.raises(ValueError, match="compiler command failed") as error:
            compiler.overlay(source, {"src/base.cpp": candidate}, (), actual_flags)
        assert "C2118" in str(error.value), str(error.value)
