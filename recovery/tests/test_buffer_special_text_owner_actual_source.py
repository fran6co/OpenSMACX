"""Permanent pytest control for the installed BufferSpecialTexts owner.

The fixture extracts current product declarations and method bodies on every
run. It deliberately has no source-hash pin, proposal fallback, or synthesized
owner body: missing/incomplete product source is a test failure.
"""
from __future__ import annotations

import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest


def repository_root() -> Path:
    for parent in Path(__file__).resolve().parents:
        if (parent / "CMakeLists.txt").is_file() and (parent / "src/buffer.cpp").is_file():
            return parent
    raise RuntimeError("cannot locate OpenSMACX repository root")


def extract_definition(source: str, signature: str) -> str:
    start = source.find(signature)
    if start < 0 or source.find(signature, start + len(signature)) >= 0:
        raise AssertionError(f"expected exactly one installed definition: {signature}")
    opening = source.find("{", start)
    if opening < 0:
        raise AssertionError(f"missing method body: {signature}")
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated method body: {signature}")


def inject_after_opening(definition: str, statements: str) -> str:
    opening = definition.find("{")
    return definition[:opening + 1] + "\n" + statements + definition[opening + 1:]


PRELUDE = r"""
#include <cassert>
#include <cstddef>
#include <cstring>
#include <new>
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
#define OSMX_JOIN_INNER(a, b) a##b
#define OSMX_JOIN(a, b) OSMX_JOIN_INNER(a, b)
#define static_assert(condition, message) \
    typedef char OSMX_JOIN(recovery_static_assert_, __LINE__)[(condition) ? 1 : -1]
struct RECT { int left; int top; int right; int bottom; };
#include "buffer_special_text.h"
"""

SUPPORT = r"""
static bool controls_enabled = false;
static char *owner_bytes = 0;
static BufferSpecialTextArrayOwner *active_owner = 0;
static int constructor_attempts = 0;
static int constructor_failure = -1;
static int destructor_count = 0;
static int destructor_order[20];
static bool cause_reentry = false;
static int expected_guard_bit = 0;
static int element_index(const BufferSpecialText *item) {
    return static_cast<int>(reinterpret_cast<const char *>(item) - owner_bytes) / 0x30;
}
"""

MAIN = r"""
union OwnerStorage {
    long alignment[sizeof(BufferSpecialTextArrayOwner) / sizeof(long)];
    char bytes[sizeof(BufferSpecialTextArrayOwner)];
};
static void check_partial_ctor_unwind() {
    OwnerStorage storage;
    std::memset(storage.bytes, 0, sizeof(storage.bytes));
    owner_bytes = storage.bytes;
    active_owner = 0;
    controls_enabled = true;
    constructor_attempts = 0;
    constructor_failure = 5;
    destructor_count = 0;
    BufferSpecialTextsCleanupGuard = 0;
    expected_guard_bit = 0;
    bool caught = false;
    try { new (storage.bytes) BufferSpecialTextArrayOwner(); }
    catch (int value) { caught = value == 5; }
    assert(caught && constructor_attempts == 6 && destructor_count == 5);
    for (int index = 0; index < 5; ++index) assert(destructor_order[index] == 4 - index);
    assert(BufferSpecialTextsCleanupGuard == 0);
}
static void check_success_reentrant_teardown() {
    OwnerStorage storage;
    std::memset(storage.bytes, 0, sizeof(storage.bytes));
    owner_bytes = storage.bytes;
    active_owner = 0;
    controls_enabled = true;
    constructor_attempts = 0;
    constructor_failure = -1;
    destructor_count = 0;
    BufferSpecialTextsCleanupGuard = 0;
    expected_guard_bit = 0;
    for (int index = 0; index < 10; ++index) {
        uint32_t sentinel = 0xA5A5A5A5;
        std::memcpy(storage.bytes + index * 0x30 + 0x24, &sentinel, 4);
        std::memcpy(storage.bytes + index * 0x30 + 0x28, &sentinel, 4);
        std::memcpy(storage.bytes + index * 0x30 + 0x2C, &sentinel, 4);
    }
    BufferSpecialTextArrayOwner *owner = new (storage.bytes) BufferSpecialTextArrayOwner();
    active_owner = owner;
    assert(constructor_attempts == 10 && BufferSpecialTextsCleanupGuard == 0);
    for (int index = 0; index < 10; ++index) {
        assert(owner->at(index).bounds[0].right == 0);
        assert(owner->at(index).field_20_ == 0);
        assert(owner->at(index).field_24_ == 0xA5A5A5A5);
        assert(owner->at(index).field_28_ == 0xA5A5A5A5);
        assert(owner->at(index).color_ == 0xA5A5A5A5);
    }
    BufferSpecialTextsCleanupGuard = 0xA4;
    expected_guard_bit = 1;
    cause_reentry = true;
    owner->~BufferSpecialTextArrayOwner();
    assert(BufferSpecialTextsCleanupGuard == 0xA5 && destructor_count == 10);
    for (int index = 0; index < 10; ++index) {
        assert(destructor_order[index] == 9 - index);
        for (int offset = 0x24; offset <= 0x2C; offset += 4) {
            uint32_t suffix = 0;
            std::memcpy(&suffix, storage.bytes + index * 0x30 + offset, 4);
            assert(suffix == 0xA5A5A5A5);
        }
    }
}
int main() {
    check_partial_ctor_unwind();
    check_success_reentrant_teardown();
    BufferSpecialTextsCleanupGuard = 0;
    controls_enabled = false;
    return 0;
}
"""


def installed_sources() -> tuple[str, str, str, str]:
    root = repository_root()
    header_path = root / "src/buffer_special_text.h"
    cpp_path = root / "src/buffer.cpp"
    if not header_path.is_file() or not cpp_path.is_file():
        raise AssertionError("installed BufferSpecialText source is missing")
    header = header_path.read_text(encoding="utf-8")
    cpp = cpp_path.read_text(encoding="utf-8")
    required_header = (
        "struct BufferSpecialTextArrayOwner",
        "BufferSpecialTextStorage elements_[10];",
        "BufferSpecialTextsCleanupGuard |= 1;",
        "BufferSpecialText &at(int index)",
    )
    missing = [needle for needle in required_header if needle not in header]
    if missing:
        raise AssertionError(f"installed owner declaration is incomplete: {missing}")
    guard = "uint8_t BufferSpecialTextsCleanupGuard;"
    owner = "BufferSpecialTextArrayOwner BufferSpecialTextsOwner;"
    if cpp.count(guard) != 1 or cpp.count(owner) != 1:
        raise AssertionError("installed guard/owner globals are missing or ambiguous")
    if cpp.index(guard) > cpp.index(owner):
        raise AssertionError("installed global construction order changed")
    signatures = ("BufferSpecialText::BufferSpecialText()", "BufferSpecialText::~BufferSpecialText()")
    methods = [extract_definition(cpp, signature) for signature in signatures]
    if "BufferSpecialTextsOwner.at(special)" not in cpp:
        raise AssertionError("installed consumers do not use the genuine owner")
    return header, guard + "\n" + owner + "\n", methods[0], methods[1]


def instrumented_methods(ctor: str, dtor: str) -> str:
    ctor = inject_after_opening(ctor, """    if (controls_enabled) {
        const int index = element_index(this);
        ++constructor_attempts;
        if (index == constructor_failure) throw index;
    }""")
    dtor = inject_after_opening(dtor, """    if (controls_enabled) {
        const int index = element_index(this);
        assert((BufferSpecialTextsCleanupGuard & 1) == expected_guard_bit);
        if (cause_reentry && index == 9) {
            cause_reentry = false;
            active_owner->~BufferSpecialTextArrayOwner();
        }
        destructor_order[destructor_count++] = index;
    }""")
    return ctor + "\n\n" + dtor + "\n"


def run_fixture(compiler: str, header: str, globals_source: str, methods: str) -> subprocess.CompletedProcess[str]:
    with tempfile.TemporaryDirectory(prefix="buffer-owner-actual-source-") as directory:
        temp = Path(directory)
        (temp / "buffer_special_text.h").write_text(header, encoding="utf-8")
        executable = temp / "owner-control"
        source = PRELUDE + globals_source + SUPPORT + methods + MAIN
        compiled = subprocess.run(
            [compiler, "-std=c++98", "-O0", "-Wall", "-Wextra", "-Werror", "-I", str(temp), "-x", "c++", "-", "-o", str(executable)],
            input=source, text=True, capture_output=True, timeout=30, check=False)
        assert compiled.returncode == 0, f"{compiler} compile failed:\n{compiled.stderr}"
        return subprocess.run([str(executable)], text=True, capture_output=True, timeout=10, check=False)


COMPILERS = [name for name in ("g++", "clang++") if shutil.which(name)]
pytestmark = pytest.mark.skipif(not COMPILERS, reason="no supported host C++ compiler found")


@pytest.mark.parametrize("compiler", COMPILERS)
def test_installed_owner_behaviors(compiler: str) -> None:
    header, globals_source, ctor, dtor = installed_sources()
    result = run_fixture(compiler, header, globals_source, instrumented_methods(ctor, dtor))
    assert result.returncode == 0, f"{compiler} runtime control failed:\n{result.stderr}"


@pytest.mark.parametrize("compiler", COMPILERS)
@pytest.mark.parametrize("mutation", ("forward teardown", "late guard", "missing ctor unwind"))
def test_mutations_fail_behaviorally(compiler: str, mutation: str) -> None:
    header, globals_source, ctor, dtor = installed_sources()
    if mutation == "forward teardown":
        changed = header.replace("for (int index = 10; index != 0; --index)",
                                 "for (int index = 0; index < 10; ++index)", 1)
    elif mutation == "late guard":
        changed = header.replace("        BufferSpecialTextsCleanupGuard |= 1;\n        for (int index = 10; index != 0; --index)",
                                 "        for (int index = 10; index != 0; --index)", 1)
        changed = changed.replace("            at(index - 1).~BufferSpecialText();\n        }",
                                  "            at(index - 1).~BufferSpecialText();\n        }\n        BufferSpecialTextsCleanupGuard |= 1;", 1)
    else:
        changed = header.replace("        } catch (...) {\n            while (constructed != 0) {\n                at(--constructed).~BufferSpecialText();\n            }\n            throw;\n        }",
                                 "        } catch (...) {\n            throw;\n        }", 1)
    assert changed != header, f"mutation anchor disappeared: {mutation}"
    result = run_fixture(compiler, changed, globals_source, instrumented_methods(ctor, dtor))
    assert result.returncode != 0, f"{compiler} mutation survived behavior control: {mutation}"
