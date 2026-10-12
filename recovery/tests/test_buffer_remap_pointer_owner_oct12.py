"""Actual Buffer remap APIs and pinned argument use, not game/VC6 acceptance.

Host controls compile the two unchanged branch bodies and real declarations
with service/field doubles. Native product compilation, emitted symbols and
historical byte obligations are independently checked by the coordinator.
Synthetic surface installation executes field-transition branches; the real
inline Sprite accessor has no callback, so this is not callback reachability
evidence. Null-table forwarding is observed by doubles, not a real pixel/fault
test of the original's unguarded table reads.
"""

import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source.reader import read_file
from match_recovery.source_history import revision_files


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
METHODS = {"draw_dest": 0x5D8A30, "draw_multi_table_dest": 0x5D93C0}
BASE = "46a3d7771689f88454238c2359782d734a4ac10e"


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


@pytest.fixture(scope="module")
def baseline_text():
    # Installed history API reads immutable committed objects, never mutations.
    files = revision_files(ROOT, BASE, ("src",))
    return files["src/buffer.cpp"].decode(), files["src/buffer.h"].decode()


def assert_exact_continuity(cpp, header, baseline):
    before_cpp, before_header = baseline
    for name in METHODS:
        current_body = five_argument_body(cpp, name)
        previous_body = five_argument_body(before_cpp, name)
        assert cpp.count(current_body) == 1
        cpp = cpp.replace(current_body, previous_body)
        current_declaration = five_argument_declaration(header, name)
        previous_declaration = five_argument_declaration(before_header, name)
        assert header.count(current_declaration) == 1
        header = header.replace(current_declaration, previous_declaration)
        # Only these two compiler-observed fact transitions are authorized.
        previous_symbol = "// symbol    ?" + name + "@Buffer@@QAEHPAV1@HHHH@Z"
        observed_symbol = "// symbol    ?" + name + "@Buffer@@QAEHPAV1@HHHPAE@Z"
        assert before_cpp.count(previous_symbol) == cpp.count(observed_symbol) == 1
        cpp = cpp.replace(observed_symbol, previous_symbol)
    # Function heads/claims/lessons and all object bindings stay outside the
    # replaceable C++ bodies. No metadata or global symbol blanket waiver.
    assert cpp == before_cpp
    assert header == before_header


def test_exact_full_baseline_source_and_header_continuity(baseline_text):
    assert_exact_continuity((ROOT / "src/buffer.cpp").read_text(),
                            (ROOT / "src/buffer.h").read_text(), baseline_text)


@pytest.mark.parametrize("damage", ["outside_source", "identity", "span", "claim",
                                    "lesson", "object_binding", "other_symbol",
                                    "wrong_observed_symbol", "outside_header"])
def test_continuity_damage_is_refused(baseline_text, damage):
    cpp = (ROOT / "src/buffer.cpp").read_text()
    header = (ROOT / "src/buffer.h").read_text()
    if damage == "outside_source":
        old, new = 'const char BufferExtTgf[] = ".tgf";', 'const char BufferExtTgf[] = ".bad";'
    elif damage == "identity":
        old, new = "0x005D8A30 ?draw_dest@Buffer@@QAEHPAVBuffer@@HHHH@Z", "0x005D8A30 wrong_identity"
    elif damage == "span":
        old, new = "0x005D8A30-0x005D8B1E", "0x005D8A30-0x005D8B1D"
    elif damage == "claim":
        old, = (line for line in cpp.splitlines() if line.startswith("// ORIGINAL: 0x005D8A30 "))
        new = old.rsplit(" ", 1)[0]
    elif damage == "lesson":
        old, new = "// function's own a5 is `int`, so the tail call casts it.", "// deleted historical lesson"
    elif damage == "object_binding":
        old, new = "member ?BufferSpriteOwner@@3UBufferSpriteLifetime@@A storage_.bytes", "member ?BufferSpriteOwner@@3UBufferSpriteLifetime@@A wrong_member"
    elif damage == "other_symbol":
        old, new = "// symbol    ?release_buff_sprite@Buffer@@SAXXZ", "// symbol    wrong_unrelated_symbol"
    elif damage == "wrong_observed_symbol":
        old, new = "// symbol    ?draw_dest@Buffer@@QAEHPAV1@HHHPAE@Z", "// symbol    ?draw_dest@Buffer@@QAEHPAV1@HHHHHPAE@Z"
    else:
        old, new = "  int draw_unk2(Buffer *destination, int transparent, int x, int y, int table);", "  int draw_unk2(Buffer *destination, int transparent, int x, int y, unsigned char *table);"
        assert header.count(old) == 1
        header = header.replace(old, new)
        with pytest.raises(AssertionError):
            assert_exact_continuity(cpp, header, baseline_text)
        return
    assert cpp.count(old) == 1
    cpp = cpp.replace(old, new)
    with pytest.raises(AssertionError):
        assert_exact_continuity(cpp, header, baseline_text)


HARNESS = r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
#include <type_traits>
#include <utility>
static void require(bool value) { if (!value) std::exit(42); }
struct Buffer;
struct Surface { int Unlock(void *); };
struct Dib { struct Header { int biWidth, biHeight; } bmiHeader; };
struct Sprite {
    char *pcBits_;
    int iSpriteWidth2_, iSpriteWidth_, iSpriteHeight_, iWidth_, iHeight_;
    int iLeftOffset_, iTopOffset_;
    unsigned char cTransparentIndex_;
    int fObj1Exists_;
    int draw_dest(Buffer *, int, int, int, unsigned char *);
    int draw_multi_table_dest(Buffer *, int, int, int, unsigned char *);
};
struct Buffer {
    void *locked_bits_, *dib_bits_;
    Surface *surface_;
    int surface_lock_count_, stride_;
    Dib dib_;
    ACTUAL_DECLARATIONS
};
static Buffer *source;
static Surface surface;
static bool install_surface, replace_bits;
static int accesses, unlocks, calls, seen_kind;
static Buffer *seen_destination;
static int seen_transparent, seen_x, seen_y;
static unsigned char *seen_table;
static Sprite seen_sprite;
static char replaced_pixel;
struct SpriteDoubleOwner {
    Sprite value;
    Sprite &sprite() {
        if (install_surface && accesses++ == 0) source->surface_ = &surface;
        return value;
    }
};
static SpriteDoubleOwner BufferSpriteOwner;
static unsigned char WinFillColour = 0xD7;
int Surface::Unlock(void *bits) {
    require(bits == source->dib_bits_ && source->locked_bits_ == bits);
    require(source->surface_lock_count_ == 0);
    require(BufferSpriteOwner.value.pcBits_ == bits);
    ++unlocks;
    return 0;
}
static int observe(Sprite *sprite, int kind, Buffer *destination,
                   int transparent, int x, int y, unsigned char *table) {
    require(sprite == &BufferSpriteOwner.value);
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
                                 unsigned char *table) {
    return observe(this, 2, d, t, x, y, table);
}
ACTUAL_BODIES
using Remap = int (Buffer::*)(Buffer *, int, int, int, unsigned char *);
static Remap plain = static_cast<Remap>(&Buffer::draw_dest);
static Remap multi = static_cast<Remap>(&Buffer::draw_multi_table_dest);
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
    unsigned char pixels[16] = {}, table[65536] = {};
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
    BufferSpriteOwner.value = {&replaced_pixel, 71, 72, 73, 74, 75, 76, 77, 78, 79};
    const Sprite before = BufferSpriteOwner.value;
    install_surface = reenter; replace_bits = replace;
    accesses = unlocks = calls = seen_kind = 0;
    unsigned char *pointer = null_table ? nullptr : table;
    const int result = (owner.*(kind == 1 ? plain : multi))(
        &destination, transparent, -19, 23, pointer);
    require(result == 0x4230 + kind && calls == 1 && seen_kind == kind);
    require(seen_destination == &destination && seen_transparent == transparent);
    require(seen_x == -19 && seen_y == 23 && seen_table == pointer);
    require(BufferSpriteOwner.value.pcBits_ == nullptr);
    require(BufferSpriteOwner.value.iSpriteWidth2_ == seen_sprite.iSpriteWidth2_);
    require(BufferSpriteOwner.value.iSpriteWidth_ == seen_sprite.iSpriteWidth_);
    require(BufferSpriteOwner.value.iSpriteHeight_ == seen_sprite.iSpriteHeight_);
    require(BufferSpriteOwner.value.iWidth_ == seen_sprite.iWidth_);
    require(BufferSpriteOwner.value.iHeight_ == seen_sprite.iHeight_);
    require(BufferSpriteOwner.value.iLeftOffset_ == seen_sprite.iLeftOffset_);
    require(BufferSpriteOwner.value.iTopOffset_ == seen_sprite.iTopOffset_);
    require(BufferSpriteOwner.value.cTransparentIndex_ == seen_sprite.cTransparentIndex_);
    require(BufferSpriteOwner.value.fObj1Exists_ == seen_sprite.fObj1Exists_);
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
        const bool retain = remaining > 0 || (reenter && !have_bits);
        require(owner.surface_lock_count_ == (retain ? remaining : 0));
        require(owner.locked_bits_ == (retain ? owner.dib_bits_ : nullptr));
        require(unlocks == (reenter && have_bits && remaining <= 0 ? 1 : 0));
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
     "destination, transparent_in, x, y, reinterpret_cast<unsigned char *>(static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(table))))"),
    ("surface_lock_count_ = surface_lock_count_ + 1;", "/* missing increment */"),
    ("surface_lock_count_ = surface_lock_count_ - 1;", "surface_lock_count_ = surface_lock_count_ - 0;"),
    ("(int)surface_lock_count_ > 0", "static_cast<unsigned int>(surface_lock_count_) > 0"),
    ("BufferSpriteOwner.sprite().iSpriteWidth_ = dib_.bmiHeader.biWidth;",
     "BufferSpriteOwner.sprite().iSpriteWidth_ = stride_;"),
    ("BufferSpriteOwner.sprite().pcBits_ = 0;", "/* missing cleanup */"),
    ("BufferSpriteOwner.sprite().pcBits_ = 0;",
     "if (BufferSpriteOwner.sprite().pcBits_ == nullptr) BufferSpriteOwner.sprite().pcBits_ = 0;"),
    ("return result;", "BufferSpriteOwner.sprite().iSpriteWidth_ = 0; return result;"),
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
    assert declaration.count("unsigned char *table") == 1
    if damage == "integer_last":
        declarations[name] = declaration.replace("unsigned char *table", "int table")
    elif damage == "integer_compatibility":
        declarations[name] += "\n" + declaration.replace("unsigned char *table", "int table")
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
