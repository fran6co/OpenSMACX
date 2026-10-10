"""Bounded actual constructor/member controls, not native layout proof."""
import re
import shutil
import subprocess

import pytest

from match_recovery.source import read_text
from test_sprite_asset_copy import ROOT, braced
from test_texture_raster_actual_source import _continuity_fields


SOURCE = ROOT / "src/sprite.cpp"
BASELINE = "52f2d6989240f5db25e08e072cf9d255bfee4119"


def test_all_sprite_identities_claims_and_symbols_remain_intact():
    old = subprocess.run(
        ["git", "show", f"{BASELINE}:src/sprite.cpp"], cwd=ROOT,
        check=True, text=True, capture_output=True,
    ).stdout
    before, after = read_text(old, SOURCE), read_text(SOURCE.read_text(), SOURCE)
    assert len(before) == len(after) == 435
    assert [_continuity_fields(row) for row in before] == [
        _continuity_fields(row) for row in after
    ]
    assert [(row.symbol, row.function_fragment) for row in before] == [
        (row.symbol, row.function_fragment) for row in after
    ]


def _probe(body):
    header = (ROOT / "src/sprite.h").read_text()
    fields = header[header.index("  char *ppszFileName_;"):]
    fields = fields[:fields.index("\n};")]
    return r'''
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <new>
#include <limits.h>
int SpriteMemoryUsed;
static char poison;
struct Sprite {
''' + fields + "\nSprite();\n};\n" + body + r'''
union Storage { char bytes[sizeof(Sprite)]; void *alignment; };
int main() {
    Storage storage;
    volatile unsigned char *bytes = reinterpret_cast<volatile unsigned char *>(storage.bytes);
    for (unsigned index = 0; index < sizeof(storage.bytes); ++index) bytes[index] = 0xa5;
    SpriteMemoryUsed = INT_MAX - 10;
    uint32_t expected = static_cast<uint32_t>(SpriteMemoryUsed) + sizeof(Sprite);
    Sprite *sprite = new (storage.bytes) Sprite;
    assert(sprite->ppszFileName_ == 0 && sprite->pcBits_ == 0);
    assert(sprite->iSpriteWidth2_ == 0 && sprite->iSpriteWidth_ == 0);
    assert(sprite->iSpriteHeight_ == 0 && sprite->iWidth_ == 0);
    assert(sprite->iHeight_ == 0 && sprite->iLeftOffset_ == 0);
    assert(sprite->iTopOffset_ == 0 && sprite->fObj1Exists_ == 0);
    assert(sprite->cTransparentIndex_ == 9);
    assert(*reinterpret_cast<volatile unsigned char *>(&sprite->pad1_) == 0xa5);
    assert(*reinterpret_cast<volatile unsigned char *>(&sprite->pad2_) == 0xa5);
    assert(*reinterpret_cast<volatile unsigned char *>(&sprite->pad3_) == 0xa5);
    assert(static_cast<uint32_t>(SpriteMemoryUsed) == expected);
}
'''


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None, "pointer", "palette", "accounting"))
def test_actual_constructor_clears_typed_members_without_padding_changes(
    compiler, damage, tmp_path,
):
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(f"{compiler} unavailable")
    body = braced(SOURCE.read_text(), "Sprite::Sprite()")
    assert "reinterpret_cast" not in body
    if damage == "pointer":
        needle = "receiver->pcBits_ = 0;"
        replacement = "receiver->pcBits_ = &poison;"
    elif damage == "palette":
        needle, replacement = "receiver->cTransparentIndex_ = 9;", "receiver->cTransparentIndex_ = 0;"
    elif damage == "accounting":
        needle, replacement = "+ sizeof(Sprite)", "+ 1"
    if damage:
        assert body.count(needle) == 1
        body = body.replace(needle, replacement)
    cpp, binary = tmp_path / "constructor.cpp", tmp_path / "constructor"
    cpp.write_text(_probe(body))
    result = subprocess.run(
        [compiler_path, "-std=c++98", "-O2", "-fstrict-aliasing", "-Wall", "-Wextra",
         "-Werror", "-Wno-unused-variable", str(cpp), "-o", str(binary)], capture_output=True, text=True,
        timeout=30,
    )
    assert result.returncode == 0, result.stdout + result.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=20)
    assert (result.returncode == 0) is (damage is None), result.stdout + result.stderr


def test_constructor_retains_pinned_store_sequence():
    body = braced(SOURCE.read_text(), "Sprite::Sprite()")
    stores = re.findall(r"receiver->(\w+)\s*=", body)
    assert stores == ["ppszFileName_", "pcBits_", "iSpriteWidth2_", "iSpriteWidth_",
                      "iSpriteHeight_", "iWidth_", "iHeight_", "iLeftOffset_",
                      "iTopOffset_", "cTransparentIndex_", "fObj1Exists_"]
    assert body.index("receiver->ppszFileName_") < body.index("SpriteMemoryUsed =")
    assert body.index("SpriteMemoryUsed =") < body.index("receiver->pcBits_")
