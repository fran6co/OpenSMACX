"""Actual Buffer remap APIs and pinned argument use, not game/VC6 acceptance.

Host controls compile the two unchanged branch bodies and real declarations
with service/field doubles. Native product compilation, emitted symbols and
historical byte obligations are independently checked by the coordinator.
The Sprite double can change the Buffer surface at the call boundary to check
that the already-completed setup/cleanup path is not retroactively changed.
This is a host state-transition control, not callback reachability evidence.
Null-table forwarding is observed by doubles, not a real pixel/fault test of
the original's unguarded table reads.
"""

import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.source.reader import read_file
from match_recovery.pe_coff import PeImage
from match_recovery.model import Span


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
METHODS = {"draw_dest": 0x5D8A30, "draw_multi_table_dest": 0x5D93C0}


def actual_parts():
    header = (ROOT / "src/buffer.h").read_text()
    rows = {row.address: row for row in read_file(ROOT / "src/buffer.cpp")}
    declarations, bodies = {}, {}
    for name, address in METHODS.items():
        matches = [match.group(0).strip() for match in re.finditer(
            r"\bint\s+" + name + r"\s*\([^;]+\);", header)
                   if match.group(0).count(",") == 4]
        assert len(matches) == 1, name
        declarations[name] = matches[0]
        region = rows[address].region
        start = re.search(r"\bint\s+Buffer::" + name + r"\s*\(", region)
        assert start is not None, name
        opening = region.index("{", start.start())
        depth = 0
        for end in range(opening, len(region)):
            depth += (region[end] == "{") - (region[end] == "}")
            if depth == 0:
                bodies[name] = region[start.start():end + 1]
                break
        assert name in bodies, "unterminated actual body"
    return declarations, bodies


def five_argument_body(cpp, name):
    matches = [match for match in re.finditer(
        r"\bint\s+Buffer::" + name + r"\s*\([^)]*\)", cpp)
               if match.group(0).count(",") == 4]
    assert len(matches) == 1, name
    start = matches[0].start()
    opening = cpp.index("{", matches[0].end())
    depth = 0
    for end in range(opening, len(cpp)):
        depth += (cpp[end] == "{") - (cpp[end] == "}")
        if depth == 0:
            return cpp[start:end + 1]
    raise AssertionError("unterminated actual body")


def five_argument_declaration(header, name):
    declarations = [match.group(0) for match in re.finditer(
        r"\bint\s+" + name + r"\s*\([^;]+\);", header)
                    if match.group(0).count(",") == 4]
    assert len(declarations) == 1, name
    return declarations[0]


HISTORICAL = {
    "draw_dest": ("// ORIGINAL: 0x005D8A30 ?draw_dest@Buffer@@QAEHPAVBuffer@@HHHH@Z 0x005D8A30-0x005D8B1E BYTE_EXACT",
                   "// symbol    ?draw_dest@Buffer@@QAEHPAV1@HHHPAE@Z"),
    "draw_multi_table_dest": (
        "// ORIGINAL: 0x005D93C0 ?draw_multi_table_dest@Buffer@@QAEHPAVBuffer@@HHHH@Z 0x005D93C0-0x005D94AE BYTE_EXACT",
        "// symbol    ?draw_multi_table_dest@Buffer@@QAEHPAV1@HHHPAY0BAA@E@Z"),
}
LESSONS = {
    "draw_dest": "The ten Blit slots this wrapper",
    "draw_multi_table_dest": "same shape and same landing as draw_dest above",
}


def assert_scoped_source(cpp, header):
    for name in METHODS:
        original, symbol = HISTORICAL[name]
        assert cpp.count(original) == 1
        assert cpp.count(symbol) == 1
        block_start = cpp.index(original)
        block_end = cpp.index(f"int Buffer::{name}(", block_start)
        provenance = cpp[block_start:block_end]
        assert "// LEVER:" in provenance or "// TRIED:" in provenance
        assert LESSONS[name] in provenance
        declaration = five_argument_declaration(header, name)
        expected_type = "unsigned char *table" if name == "draw_dest" else "ColorTable *table"
        assert expected_type in declaration and "int table" not in declaration
        body = five_argument_body(cpp, name)
        assert "shared_sprite_" in body and "BufferSpriteOwner" not in body
        assert "shared_sprite_.pcBits_ = 0;" in body
    assert "static Sprite shared_sprite_;" in header


def test_scoped_original_identity_claim_and_current_abi():
    assert_scoped_source((ROOT / "src/buffer.cpp").read_text(),
                         (ROOT / "src/buffer.h").read_text())


@pytest.mark.parametrize("damage", ["identity", "span", "claim", "lesson",
                                    "member", "symbol", "declaration"])
def test_scoped_source_damage_is_refused(damage):
    cpp = (ROOT / "src/buffer.cpp").read_text()
    header = (ROOT / "src/buffer.h").read_text()
    if damage == "identity":
        cpp = cpp.replace(HISTORICAL["draw_dest"][0],
                          HISTORICAL["draw_dest"][0].replace("?draw_dest@Buffer@@", "?wrong@Buffer@@"), 1)
    elif damage == "span":
        cpp = cpp.replace("0x005D8A30-0x005D8B1E", "0x005D8A30-0x005D8B1D", 1)
    elif damage == "claim":
        cpp = cpp.replace("0x005D8A30-0x005D8B1E BYTE_EXACT",
                          "0x005D8A30-0x005D8B1E MISMATCH", 1)
    elif damage == "lesson":
        cpp = cpp.replace(LESSONS["draw_dest"], "the removed slots this wrapper", 1)
    elif damage == "member":
        header = header.replace("static Sprite shared_sprite_;", "static Sprite changed_sprite_;", 1)
    elif damage == "symbol":
        cpp = cpp.replace(HISTORICAL["draw_dest"][1], "// symbol    ?draw_dest_wrong", 1)
    else:
        declaration = five_argument_declaration(header, "draw_dest")
        assert "unsigned char *table" in declaration
        header = header.replace(declaration,
                                declaration.replace("unsigned char *table", "int table", 1), 1)
    with pytest.raises(AssertionError):
        assert_scoped_source(cpp, header)


HARNESS = r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
#include <type_traits>
#include <utility>
#define require(value) do { if (!(value)) { \
    std::fprintf(stderr, "host fixture require failed at line %d\n", __LINE__); \
    std::exit(42); } } while (0)
struct Buffer;
using ColorTable = unsigned char[256];
struct Surface { int Unlock(void *); };
struct Dib { struct Header { int biWidth, biHeight; } bmiHeader; };
struct Sprite {
    unsigned char *pcBits_;
    int iSpriteWidth2_, iSpriteWidth_, iSpriteHeight_, iWidth_, iHeight_;
    int iLeftOffset_, iTopOffset_;
    unsigned char cTransparentIndex_;
    int fObj1Exists_;
    int draw_dest(Buffer *, int, int, int, unsigned char *);
    int draw_multi_table_dest(Buffer *, int, int, int, ColorTable *);
};
struct Buffer {
    static Sprite shared_sprite_;
    void *locked_bits_, *dib_bits_;
    Surface *surface_;
    int surface_lock_count_, stride_;
    Dib dib_;
    ACTUAL_DECLARATIONS
};
Sprite Buffer::shared_sprite_;
static Buffer *source;
static Surface surface;
static bool install_surface, replace_bits;
static int unlocks, calls, seen_kind;
static Buffer *seen_destination;
static int seen_transparent, seen_x, seen_y;
static const void *seen_table;
static Sprite seen_sprite;
static unsigned char replaced_pixel;
static unsigned char WinFillColour = 0xD7;
int Surface::Unlock(void *bits) {
    require(bits == source->dib_bits_ && source->locked_bits_ == bits);
    require(source->surface_lock_count_ == 0);
    require(Buffer::shared_sprite_.pcBits_ == bits);
    ++unlocks;
    return 0;
}
static int observe(Sprite *sprite, int kind, Buffer *destination,
                   int transparent, int x, int y, const void *table) {
    require(sprite == &Buffer::shared_sprite_);
    if (install_surface) source->surface_ = &surface;
    ++calls; seen_kind = kind; seen_destination = destination;
    seen_transparent = transparent; seen_x = x; seen_y = y; seen_table = table;
    seen_sprite = *sprite;
    if (replace_bits) sprite->pcBits_ = &replaced_pixel;
    return 0x4230 + kind;
}
int Sprite::draw_dest(Buffer *d, int t, int x, int y, unsigned char *table) {
    return observe(this, 1, d, t, x, y, table);
}
int Sprite::draw_multi_table_dest(Buffer *d, int t, int x, int y,
                                 ColorTable *table) {
    return observe(this, 2, d, t, x, y, table);
}
ACTUAL_BODIES
using PlainRemap = int (Buffer::*)(Buffer *, int, int, int, unsigned char *);
using MultiRemap = int (Buffer::*)(Buffer *, int, int, int, ColorTable *);
static PlainRemap plain = static_cast<PlainRemap>(&Buffer::draw_dest);
static MultiRemap multi = static_cast<MultiRemap>(&Buffer::draw_multi_table_dest);
template<class T, class = void> struct OldPlain : std::false_type {};
template<class T> struct OldPlain<T, std::void_t<decltype(
    static_cast<int (T::*)(Buffer *, int, int, int, int)>(&T::draw_dest))>>
    : std::true_type {};
template<class T, class = void> struct OldMulti : std::false_type {};
template<class T> struct OldMulti<T, std::void_t<decltype(
    static_cast<int (T::*)(Buffer *, int, int, int, int)>(&T::draw_multi_table_dest))>>
    : std::true_type {};
static_assert(!OldPlain<Buffer>::value && !OldMulti<Buffer>::value,
              "no integer compatibility overload");
static_assert(sizeof(void *) > sizeof(std::uint32_t), "full-width host control");
static void route(int kind, bool existing_surface, bool have_bits, int count,
                  int transparent, bool reenter, bool replace, bool null_table) {
    unsigned char pixels[16] = {};
    ColorTable table[256] = {};
    require(reinterpret_cast<std::uintptr_t>(table) > UINT32_MAX);
    Buffer owner = {}, destination = {};
    source = &owner;
    owner.dib_bits_ = have_bits ? pixels : nullptr;
    owner.locked_bits_ = &replaced_pixel;
    owner.surface_ = existing_surface ? &surface : nullptr;
    owner.surface_lock_count_ = count;
    owner.stride_ = 37;
    owner.dib_.bmiHeader.biWidth = 5;
    owner.dib_.bmiHeader.biHeight = -3;
    Buffer::shared_sprite_ = {&replaced_pixel, 71, 72, 73, 74, 75, 76, 77, 78, 79};
    const Sprite before = Buffer::shared_sprite_;
    install_surface = reenter; replace_bits = replace;
    unlocks = calls = seen_kind = 0;
    unsigned char *plain_pointer = null_table ? nullptr : &table[0][0];
    ColorTable *multi_pointer = null_table ? nullptr : table;
    const int result = kind == 1
        ? (owner.*plain)(&destination, transparent, -19, 23, plain_pointer)
        : (owner.*multi)(&destination, transparent, -19, 23, multi_pointer);
    require(result == 0x4230 + kind && calls == 1 && seen_kind == kind);
    require(seen_destination == &destination && seen_transparent == transparent);
    require(seen_x == -19 && seen_y == 23 &&
            seen_table == (kind == 1 ? static_cast<void *>(plain_pointer)
                                     : static_cast<void *>(multi_pointer)));
    require(Buffer::shared_sprite_.pcBits_ == nullptr);
    require(Buffer::shared_sprite_.iSpriteWidth2_ == seen_sprite.iSpriteWidth2_);
    require(Buffer::shared_sprite_.iSpriteWidth_ == seen_sprite.iSpriteWidth_);
    require(Buffer::shared_sprite_.iSpriteHeight_ == seen_sprite.iSpriteHeight_);
    require(Buffer::shared_sprite_.iWidth_ == seen_sprite.iWidth_);
    require(Buffer::shared_sprite_.iHeight_ == seen_sprite.iHeight_);
    require(Buffer::shared_sprite_.iLeftOffset_ == seen_sprite.iLeftOffset_);
    require(Buffer::shared_sprite_.iTopOffset_ == seen_sprite.iTopOffset_);
    require(Buffer::shared_sprite_.cTransparentIndex_ == seen_sprite.cTransparentIndex_);
    require(Buffer::shared_sprite_.fObj1Exists_ == seen_sprite.fObj1Exists_);
    if (existing_surface) {
        require(seen_sprite.pcBits_ == before.pcBits_ && unlocks == 0);
        require(seen_sprite.iSpriteWidth2_ == before.iSpriteWidth2_);
        require(seen_sprite.iSpriteWidth_ == before.iSpriteWidth_);
        require(seen_sprite.iSpriteHeight_ == before.iSpriteHeight_);
        require(seen_sprite.iWidth_ == before.iWidth_ && seen_sprite.iHeight_ == before.iHeight_);
        require(seen_sprite.iLeftOffset_ == 76 && seen_sprite.iTopOffset_ == 77);
        require(seen_sprite.cTransparentIndex_ == 78 && seen_sprite.fObj1Exists_ == 79);
        require(owner.locked_bits_ == &replaced_pixel && owner.surface_lock_count_ == count);
    } else {
        require(seen_sprite.pcBits_ == owner.dib_bits_);
        require(seen_sprite.iSpriteWidth2_ == 37 && seen_sprite.iSpriteWidth_ == 5);
        require(seen_sprite.iSpriteHeight_ == 3 && seen_sprite.iWidth_ == 5);
        require(seen_sprite.iHeight_ == 3 && seen_sprite.iLeftOffset_ == 0);
        require(seen_sprite.iTopOffset_ == 0 && seen_sprite.fObj1Exists_ == 0);
        require(seen_sprite.cTransparentIndex_ ==
                (transparent == -1 ? WinFillColour : static_cast<unsigned char>(transparent)));
        const int remaining = count + (have_bits ? 1 : 0) - 1;
        const bool retain = remaining > 0;
        require(owner.surface_lock_count_ == (retain ? remaining : 0));
        require(owner.locked_bits_ == (retain ? owner.dib_bits_ : nullptr));
        // Surface installation occurs inside the Sprite call, after this
        // wrapper's lock-count cleanup, so it must not cause a late Unlock.
        require(unlocks == 0);
    }
    require(owner.surface_ == (existing_surface || reenter ? &surface : nullptr));
    require(owner.dib_bits_ == (have_bits ? pixels : nullptr) && owner.stride_ == 37);
}
int main() {
    for (int kind : {1, 2}) {
        for (int count : {-2, 0, 1, 3}) {
            for (bool bits : {false, true}) {
                for (int transparent : {-1, 0x131}) {
                    route(kind, false, bits, count, transparent, false, false, false);
                    route(kind, false, bits, count, transparent, false, true, true);
                }
            }
        }
        route(kind, true, true, 3, -1, false, true, false);
        route(kind, false, true, 0, -1, true, true, false);
        route(kind, false, true, 3, -1, true, false, false);
        route(kind, false, false, 0, -1, true, true, true);
    }
}
'''


def host_source(declarations, bodies):
    return (HARNESS.replace("ACTUAL_DECLARATIONS", "\n".join(declarations.values()))
            .replace("ACTUAL_BODIES", "\n".join(bodies.values())))


def compile_host(tmp_path, compiler, declarations, bodies):
    executable = tmp_path / "remap"
    unit = tmp_path / "remap.cpp"
    unit.write_text(host_source(declarations, bodies))
    result = subprocess.run([compiler, "-std=c++17", "-O1", "-Wall", "-Wextra",
                             "-Werror", "-fsanitize=undefined", "-fno-sanitize-recover=all",
                             str(unit), "-o", str(executable)],
                            capture_output=True, text=True, timeout=40)
    return result, executable


@pytest.fixture(params=["g++", "clang++"])
def compiler(request):
    path = shutil.which(request.param)
    if path is None:
        pytest.skip(request.param + " unavailable")
    return path


def test_actual_typed_remap_bodies(tmp_path, compiler):
    declarations, bodies = actual_parts()
    result, executable = compile_host(tmp_path, compiler, declarations, bodies)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stderr


RUNTIME_DAMAGE = [
    ("destination, transparent_in, x, y, table)", "destination, transparent, x, y, table)"),
    ("destination, transparent_in, x, y, table)", "destination, transparent_in, y, x, table)"),
    ("destination, transparent_in, x, y, table)",
     "(static_cast<void>(destination), this), transparent_in, x, y, table)"),
    ("destination, transparent_in, x, y, table)",
     "destination, transparent_in, x, y, reinterpret_cast<decltype(table)>(static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(table))))"),
    ("surface_lock_count_ = surface_lock_count_ + 1;", "/* missing increment */"),
    ("surface_lock_count_ = surface_lock_count_ - 1;", "surface_lock_count_ = surface_lock_count_ - 0;"),
    ("(int)surface_lock_count_ > 0", "static_cast<unsigned int>(surface_lock_count_) > 0"),
    ("shared_sprite_.iSpriteWidth_ = dib_.bmiHeader.biWidth;",
     "shared_sprite_.iSpriteWidth_ = stride_;"),
    ("shared_sprite_.pcBits_ = 0;", "/* missing cleanup */"),
    ("shared_sprite_.pcBits_ = 0;",
     "if (shared_sprite_.pcBits_ == nullptr) shared_sprite_.pcBits_ = 0;"),
    ("return result;", "shared_sprite_.iSpriteWidth_ = 0; return result;"),
    ("return result;", "(void)result; return 0;"),
]


@pytest.mark.parametrize("name", METHODS)
@pytest.mark.parametrize("old,new", RUNTIME_DAMAGE)
def test_executed_body_damage_is_rejected(tmp_path, compiler, name, old, new):
    declarations, bodies = actual_parts()
    assert old in bodies[name]
    bodies[name] = bodies[name].replace(old, new)
    result, executable = compile_host(tmp_path, compiler, declarations, bodies)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 42, result.stderr


@pytest.mark.parametrize("name", METHODS)
@pytest.mark.parametrize("damage", ["integer_last", "integer_compatibility", "pointer_scalar"])
def test_actual_declaration_damage_is_rejected(tmp_path, compiler, name, damage):
    declarations, bodies = actual_parts()
    declaration = declarations[name]
    table_type = "unsigned char *table" if name == "draw_dest" else "ColorTable *table"
    assert declaration.count(table_type) == 1
    if damage == "integer_last":
        declarations[name] = declaration.replace(table_type, "int table")
    elif damage == "integer_compatibility":
        declarations[name] += "\n" + declaration.replace(table_type, "int table")
    else:
        declarations[name] = declaration.replace("int transparent", "unsigned char *transparent")
    result, _ = compile_host(tmp_path, compiler, declarations, bodies)
    assert result.returncode != 0, "damaged real declaration unexpectedly compiled"


def test_pinned_original_distinguishes_pointer_from_scalar_arguments():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original required; set OPENSMACX_IMAGE")
    image = PeImage(path.read_bytes(), PIN)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    witnesses = {}
    for start, end in ((0x5D8A30, 0x5D8B1E), (0x5D93C0, 0x5D94AE),
                       (0x5E5833, 0x5E6652), (0x5E8FF5, 0x5E9D44)):
        instructions = list(decoder.disasm(image.read(Span(start, end)), start))
        assert sum(item.size for item in instructions) == end - start
        witnesses.update({item.address: (item.mnemonic, item.op_str) for item in instructions})
    expected = {
        0x5D8AF3: ("mov", "eax, dword ptr [esp + 0x20]"),
        0x5D8AFF: ("push", "eax"), 0x5D8B0D: ("call", "0x5e5833"),
        0x5D9483: ("mov", "eax, dword ptr [esp + 0x20]"),
        0x5D948F: ("push", "eax"), 0x5D949D: ("call", "0x5e8ff5"),
        0x5E58E3: ("mov", "eax, dword ptr [ebp + 0xc]"),
        0x5E58E6: ("and", "eax, 0xff"),
        0x5E5939: ("mov", "ecx, dword ptr [ebp + 0x14]"),
        0x5E593C: ("add", "ecx, dword ptr [eax + 0x24]"),
        0x5E5945: ("mov", "eax, dword ptr [ebp + 0x10]"),
        0x5E5948: ("add", "eax, dword ptr [edx + 0x20]"),
        0x5E5A40: ("mov", "edx, dword ptr [ebp + 0x18]"),
        0x5E5A46: ("mov", "ebp, edx"),
        0x5E5AF0: ("mov", "bl, byte ptr [edi]"),
        0x5E5AF2: ("mov", "bl, byte ptr [ebp + ebx]"),
        0x5E90A5: ("mov", "eax, dword ptr [ebp + 0xc]"),
        0x5E90A8: ("and", "eax, 0xff"),
        0x5E90FB: ("mov", "ecx, dword ptr [ebp + 0x14]"),
        0x5E90FE: ("add", "ecx, dword ptr [eax + 0x24]"),
        0x5E9107: ("mov", "eax, dword ptr [ebp + 0x10]"),
        0x5E910A: ("add", "eax, dword ptr [edx + 0x20]"),
        0x5E9202: ("mov", "edx, dword ptr [ebp + 0x18]"),
        0x5E9208: ("mov", "ebp, edx"),
        0x5E9285: ("cmp", "byte ptr [esi], al"),
        0x5E9289: ("mov", "bh, byte ptr [esi]"),
        0x5E928B: ("mov", "bl, byte ptr [edi]"),
        0x5E928D: ("mov", "bl, byte ptr [ebp + ebx]"),
    }
    assert {address: witnesses[address] for address in expected} == expected
