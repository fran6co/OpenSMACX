"""Host controls for Buffer's direct ten-element static member.

The host fixture extracts the real element constructor/destructor and uses a
typed static array receiver. It checks element field behavior and the C++ array
prefix-unwind/reverse-destruction rules. It does not simulate or qualify the
VC6-generated process-start guard; that requires fresh native COFF/PE evidence.
"""
from __future__ import annotations

import shutil
import re
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
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated method body: {signature}")


PRELUDE = r"""
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <new>
struct RECT { int left; int top; int right; int bottom; };
#include "buffer_special_text.h"
struct Buffer { static BufferSpecialText special_texts_[10]; };
BufferSpecialText Buffer::special_texts_[10];
static bool observe_lifetime = false;
static int fail_after = -1;
static int attempts = 0, destroyed = 0;
static const BufferSpecialText *constructed[10];
static const BufferSpecialText *destruction_order[10];
"""

MAIN = r"""
static void test_element_fields_and_untouched_suffix() {
    assert(sizeof(Buffer::special_texts_) == 10 * 0x30);
    for (int i = 0; i < 10; ++i) {
        const BufferSpecialText &item = Buffer::special_texts_[i];
        assert(item.bounds[0].left == 0 && item.bounds[0].top == 0);
        assert(item.bounds[0].right == 0 && item.bounds[0].bottom == 0);
        assert(item.bounds[1].left == 0 && item.bounds[1].top == 0);
        assert(item.bounds[1].right == 0 && item.bounds[1].bottom == 0);
        assert(item.field_20_ == 0);
        assert(item.field_24_ == 0 && item.field_28_ == 0 && item.color_ == 0);
    }
    alignas(BufferSpecialText) unsigned char bytes[sizeof(BufferSpecialText)] = {};
    const uint32_t sentinel = 0xA5A5A5A5;
    for (int offset = 0x24; offset <= 0x2C; offset += 4)
        std::memcpy(bytes + offset, &sentinel, sizeof(sentinel));
    BufferSpecialText *item = new (bytes) BufferSpecialText;
    for (int offset = 0x24; offset <= 0x2C; offset += 4) {
        uint32_t value = 0;
        std::memcpy(&value, bytes + offset, sizeof(value));
        assert(value == sentinel);
    }
    item->~BufferSpecialText();
}

static void test_language_array_prefix_unwind() {
    for (int failure = 0; failure < 10; ++failure) {
        attempts = destroyed = 0;
        fail_after = failure;
        observe_lifetime = true;
        bool caught = false;
        try {
            BufferSpecialText local[10];
            (void)local;
        } catch (int value) {
            caught = value == failure;
        }
        observe_lifetime = false;
        assert(caught && attempts == failure + 1 && destroyed == failure);
        for (int i = 0; i < failure; ++i)
            assert(destruction_order[i] == constructed[failure - 1 - i]);
    }
    attempts = destroyed = 0;
    fail_after = -1;
    observe_lifetime = true;
    {
        BufferSpecialText local[10];
        assert(attempts == 10);
    }
    observe_lifetime = false;
    assert(destroyed == 10);
    for (int i = 0; i < 10; ++i)
        assert(destruction_order[i] == constructed[9 - i]);
}

int main() {
    test_element_fields_and_untouched_suffix();
    test_language_array_prefix_unwind();
}
"""


def installed_sources() -> tuple[str, str, str]:
    root = repository_root()
    header = (root / "src/buffer_special_text.h").read_text(encoding="utf-8")
    buffer_header = (root / "src/buffer.h").read_text(encoding="utf-8")
    cpp = (root / "src/buffer.cpp").read_text(encoding="utf-8")
    assert "static BufferSpecialText special_texts_[10];" in buffer_header
    assert "BufferSpecialText Buffer::special_texts_[10];" in cpp
    combined = cpp + buffer_header + header
    assert not re.search(
        r"^\s*(?:uint8_t|unsigned char|char)\s+BufferSpecialTextsCleanupGuard\s*;",
        combined, re.MULTILINE)
    assert not re.search(
        r"^\s*(?:BufferSpecialTextArrayOwner|BufferSpecialTextsOwner)\s+\w+\s*;",
        combined, re.MULTILINE)
    assert not re.search(r"^\s*(?:struct|class)\s+BufferSpecialTextArrayOwner\b",
                         combined, re.MULTILINE)
    assert "special_texts_[special]" in cpp
    ctor = extract_definition(cpp, "BufferSpecialText::BufferSpecialText()")
    dtor = extract_definition(cpp, "BufferSpecialText::~BufferSpecialText()")
    return header, ctor, dtor


def instrument(ctor: str, dtor: str) -> str:
    ctor_open = ctor.index("{")
    ctor = ctor[:ctor_open + 1] + r"""
    if (observe_lifetime) {
        const int index = attempts++;
        if (index < 10) constructed[index] = this;
        if (index == fail_after) throw index;
    }
""" + ctor[ctor_open + 1:]
    dtor_open = dtor.index("{")
    dtor = dtor[:dtor_open + 1] + r"""
    if (observe_lifetime) destruction_order[destroyed++] = this;
""" + dtor[dtor_open + 1:]
    return ctor + "\n" + dtor + "\n"


@pytest.mark.parametrize("compiler", [name for name in ("g++", "clang++")
                                       if shutil.which(name)])
def test_direct_static_array_element_behavior_and_language_unwind(compiler: str) -> None:
    header, ctor, dtor = installed_sources()
    with tempfile.TemporaryDirectory(prefix="buffer-static-array-") as directory:
        temp = Path(directory)
        (temp / "buffer_special_text.h").write_text(header, encoding="utf-8")
        executable = temp / "array-control"
        source = PRELUDE + instrument(ctor, dtor) + MAIN
        compiled = subprocess.run(
            [compiler, "-std=c++11", "-O0", "-Wall", "-Wextra", "-Werror",
             "-I", str(temp), "-x", "c++", "-", "-o", str(executable)],
            input=source, text=True, capture_output=True, timeout=30, check=False)
        assert compiled.returncode == 0, f"{compiler} compile failed:\n{compiled.stderr}"
        result = subprocess.run([str(executable)], text=True, capture_output=True,
                                timeout=10, check=False)
        assert result.returncode == 0, f"{compiler} runtime control failed:\n{result.stderr}"
