"""Compile/run host controls from installed production source snippets.

These controls use a small mock Sprite only to observe constructor/destructor
ordering. The owner union and owner/accessor definitions are extracted from installed
source. The Sprite double observes lifecycle only, not the target ABI or game.
"""
from __future__ import annotations

import hashlib
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
def installed_sources():
    return ((ROOT / "src/sprite.h").read_text(encoding="utf-8"),
            (ROOT / "src/sprite.cpp").read_text(encoding="utf-8"))


def braced_definition(text: str, signature: str) -> str:
    start = text.find(signature)
    if start < 0 or text.find(signature, start + 1) >= 0:
        raise SystemExit(f"expected exactly one proposed definition: {signature}")
    opening = text.find("{", start)
    if opening < 0:
        raise SystemExit(f"missing body for {signature}")
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[start:index + 1]
    raise SystemExit(f"unterminated body for {signature}")


def build_control(header: str, cpp: str, mutation: str = "") -> str:
    union_start = header.index("union BlankSpriteStorage {")
    owner_end_text = '"blank Sprite owner must retain the original object extent");'
    owner_end = header.index(owner_end_text, union_start) + len(owner_end_text)
    owner_layout = header[union_start:owner_end]
    owner_layout = re.sub(r"static_assert\([\s\S]*?\);", "", owner_layout)
    owner_layout += '''
typedef char blank_storage_extent_control[
    sizeof(BlankSpriteStorage) == sizeof(Sprite) ? 1 : -1];
typedef char blank_owner_extent_control[
    sizeof(BlankSpriteStorageOwner) == sizeof(Sprite) ? 1 : -1];
'''
    snippets = [braced_definition(cpp, signature) for signature in (
        "Sprite &blank_sprite()", "BlankSpriteStorageOwner::BlankSpriteStorageOwner()",
        "BlankSpriteStorageOwner::~BlankSpriteStorageOwner()",
        "Sprite &BlankSpriteStorageOwner::sprite()",
    )]
    implementations = "\n".join(snippets)
    if mutation == "late_guard":
        before = "    BlankSpriteCleanupGuard |= 1;\n    sprite().~Sprite();"
        after = "    sprite().~Sprite();\n    BlankSpriteCleanupGuard |= 1;"
        if implementations.count(before) != 1:
            raise SystemExit("could not apply reentrancy mutant")
        implementations = implementations.replace(before, after, 1)
    elif mutation == "clobber_guard_bits":
        if implementations.count("BlankSpriteCleanupGuard |= 1;") != 1:
            raise SystemExit("could not apply guard-bit mutant")
        implementations = implementations.replace("BlankSpriteCleanupGuard |= 1;",
                                                  "BlankSpriteCleanupGuard = 1;", 1)
    elif mutation == "skip_construction":
        if implementations.count("new (&rawstorage_.bytes[0]) Sprite();") != 1:
            raise SystemExit("could not apply construction mutant")
        implementations = implementations.replace("new (&rawstorage_.bytes[0]) Sprite();",
                                                  "/* omitted construction */", 1)
    elif mutation:
        raise SystemExit(f"unknown mutation {mutation}")

    return f'''#include <new>
#include <stdint.h>
#include <assert.h>

struct BlankSpriteStorageOwner;
extern BlankSpriteStorageOwner g_BLANK_SPRITE_OWNER;
class Sprite {{
public:
    static int constructed;
    static int destroyed;
    static bool reenter;
    uint32_t mock_sprite_storage[11];
    Sprite() {{ ++constructed; }}
    ~Sprite();
}};
int Sprite::constructed = 0;
int Sprite::destroyed = 0;
bool Sprite::reenter = false;

{owner_layout}
extern uint8_t BlankSpriteCleanupGuard;
extern BlankSpriteStorageOwner g_BLANK_SPRITE_OWNER;
Sprite &blank_sprite();

Sprite::~Sprite() {{
    ++destroyed;
    if (reenter) g_BLANK_SPRITE_OWNER.~BlankSpriteStorageOwner();
}}

uint8_t BlankSpriteCleanupGuard = 0;
BlankSpriteStorageOwner g_BLANK_SPRITE_OWNER;
{implementations}

int main() {{
    assert(Sprite::constructed == 1);
    assert(&blank_sprite() == reinterpret_cast<Sprite *>(g_BLANK_SPRITE_OWNER.rawstorage_.bytes));
    assert(reinterpret_cast<uintptr_t>(&blank_sprite()) % 4 == 0);
    BlankSpriteCleanupGuard = 0xA0;
    Sprite::reenter = true;
    g_BLANK_SPRITE_OWNER.~BlankSpriteStorageOwner();
    assert(BlankSpriteCleanupGuard == 0xA1);
    assert(Sprite::destroyed == 1);
    g_BLANK_SPRITE_OWNER.~BlankSpriteStorageOwner();
    assert(Sprite::destroyed == 1);
    return 0;
}}
'''


import pytest

@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("mutation", ["", "late_guard", "clobber_guard_bits", "skip_construction"])
def test_installed_blank_sprite_lifetime(compiler_name, mutation, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    header, cpp = installed_sources()
    source = build_control(header, cpp, mutation)
    binary = tmp_path / "blank_sprite_lifetime"
    compiled = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-x", "c++", "-", "-o", str(binary)], input=source, text=True, capture_output=True)
    assert compiled.returncode == 0, compiled.stderr
    ran = subprocess.run([str(binary)], capture_output=True, timeout=10)
    assert (ran.returncode == 0) == (mutation == ""), ran.stderr.decode(errors="replace")
