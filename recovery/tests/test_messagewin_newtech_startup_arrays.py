"""Source-driven host controls for two existing process-start array owners.

The probes extract real constructor/destructor bodies from product source and
replace native types with tiny event-recording C++98 doubles. They test source
ownership and ordinary lifetime order, not VC6 layout or generated-symbol
selection.
"""

from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path

import pytest
from match_recovery.model import Span
from match_recovery.object_source import read_objects
from match_recovery.pe_coff import PeImage
from match_recovery.source import Mode, read_file
from match_recovery.source_inventory import image_pin


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/messagewin.cpp").is_file())
HOST_COMPILERS = [path for path in (shutil.which("g++"), shutil.which("clang++")) if path]


@pytest.fixture(params=HOST_COMPILERS or [None],
                ids=[Path(p).name for p in HOST_COMPILERS] or ["no-host-compiler"])
def host_compiler(request):
    if request.param is None:
        pytest.skip("no GCC or Clang host compiler available")
    return request.param


def _mapped_record(path: str, address: int, name: str, span: tuple[int, int],
                   symbol: str, byte_exact: bool) -> None:
    records = [row for row in read_file(ROOT / path) if row.address == address]
    assert len(records) == 1
    row = records[0]
    assert row.name == name
    assert row.mode is Mode.FILE
    assert row.image_spans == (span,)
    assert row.symbol == symbol
    assert row.byte_exact is byte_exact


def _block(source: str, marker: str) -> str:
    start = source.index(marker)
    opening = source.index("{", start)
    depth = 0
    for pos in range(opening, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start:pos + 1]
    raise AssertionError(f"unterminated source block {marker}")


def _body(source: str, marker: str) -> str:
    block = _block(source, marker)
    return block[block.index("{"):]


def _compile_run(tmp_path: Path, compiler: str, source: str) -> subprocess.CompletedProcess[str]:
    tmp_path.mkdir(parents=True, exist_ok=True)
    cpp = tmp_path / "startup_array_probe.cpp"
    exe = tmp_path / "startup_array_probe"
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        text=True, capture_output=True, check=False, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], text=True, capture_output=True, check=False, timeout=30)


def _messagewin_probe(ctor_body: str, list_dtor_body: str) -> str:
    return r"""
#include <cstdlib>
static int constructed[16], construct_count;
static int removed[16], remove_count;
static int derived_done[16], derived_count;
static int base_done[16], base_count;
static int poison;
struct StringStruct {
  void* head_; void* current_; int entry_count_; int current_position_; void* allocator_;
  int test_id_;
  StringStruct() {
    head_ = &poison; current_ = &poison; entry_count_ = -17;
    current_position_ = -19; allocator_ = &poison;
""" + ctor_body[1:-1] + r"""
    test_id_ = construct_count;
    constructed[construct_count++] = test_id_;
  }
  virtual ~StringStruct() { base_done[base_count++] = test_id_; }
};
struct StringList : StringStruct {
  ~StringList() {
""" + list_dtor_body[1:-1] + r"""
    derived_done[derived_count++] = test_id_;
  }
  void remove_all() { removed[remove_count++] = test_id_; }
};
extern StringList MessageWinDeferredBaseMessages[8];
struct VerifyAtExit {
  ~VerifyAtExit() {
    if (remove_count != 8 || derived_count != 8 || base_count != 8) std::abort();
    for (int i = 0; i != 8; ++i) {
      const int reverse = 7 - i;
      if (removed[i] != reverse || derived_done[i] != reverse ||
          base_done[i] != reverse) std::abort();
    }
  }
} verify_at_exit;
StringList MessageWinDeferredBaseMessages[8];
int main() {
  if (construct_count != 8) return 41;
  for (int i = 0; i != 8; ++i) {
    const StringStruct& item = MessageWinDeferredBaseMessages[i];
    if (constructed[i] != i || item.head_ != 0 || item.current_ != 0 ||
        item.entry_count_ != 0 || item.current_position_ != 0 ||
        item.allocator_ != 0) return 42;
  }
  return 0;
}
"""


def test_messagewin_pending_list_array_has_real_ctor_and_reverse_cleanup(
    tmp_path: Path, host_compiler: str,
) -> None:
    owner = (ROOT / "src/messagewin.cpp").read_text(encoding="utf-8")
    header = (ROOT / "src/messagewin.h").read_text(encoding="utf-8")
    declaration = re.search(r"^StringList\s+MessageWinDeferredBaseMessages\s*\[(\d+)\]\s*;",
                            owner, re.MULTILINE)
    assert declaration and declaration.group(1) == "8"
    assert re.search(r"^extern StringList\s+MessageWinDeferredBaseMessages\s*\[8\]\s*;",
                     header, re.MULTILINE)
    assert "MessageWinDeferredBaseMessages[faction]" in owner

    _mapped_record("src/messagewin.cpp", 0x0051DF80,
                   "sub_51df80", (0x0051DF80, 0x0051DFA4),
                   "_$E26", False)
    _mapped_record("src/messagewin.cpp", 0x0051DFB0,
                   "sub_51dfb0", (0x0051DFB0, 0x0051DFC4),
                   "_$E24", True)
    objects = [row for row in read_objects(ROOT, image_pin(ROOT))
               if row.origin.address == 0x0093A7B0]
    assert len(objects) == 1
    assert objects[0].span is not None and objects[0].span.end == 0x0093A930
    assert objects[0].symbol == "?MessageWinDeferredBaseMessages@@3PAVStringList@@A"
    assert objects[0].unit == "src/messagewin.cpp"

    image_path = ROOT / ".opensmacx/game/terranx_original.exe"
    if image_path.is_file():
        image = PeImage(image_path.read_bytes(), image_pin(ROOT))
        assert image.read_initial(Span(0x0093A7B0, 0x0093A930)) == bytes(0x180)

    base = (ROOT / "src/stringstruct.cpp").read_text(encoding="utf-8")
    derived = (ROOT / "src/stringlist.cpp").read_text(encoding="utf-8")
    ctor_body = _body(base, "StringStruct::StringStruct()")
    dtor_body = _body(derived, "StringList::~StringList()")
    assert "StringList::~StringList()" in derived
    assert "finish_list_teardown(this);" in _body(base, "StringStruct::~StringStruct()")

    good = _messagewin_probe(ctor_body, dtor_body)
    result = _compile_run(tmp_path / "messagewin-good", host_compiler, good)
    assert result.returncode == 0, result.stdout + result.stderr

    no_head_init = ctor_body.replace("head_ = 0;", "/* mutation omitted head_ init */", 1)
    assert no_head_init != ctor_body
    mutant = _compile_run(tmp_path / "messagewin-no-head-init", host_compiler,
                          _messagewin_probe(no_head_init, dtor_body))
    assert mutant.returncode != 0, "omitting actual head_ initialization must fail the poisoned-state control"

    no_remove = dtor_body.replace("remove_all();", "/* mutation omitted remove_all */", 1)
    assert no_remove != dtor_body
    mutant = _compile_run(tmp_path / "messagewin-no-remove", host_compiler,
                          _messagewin_probe(ctor_body, no_remove))
    assert mutant.returncode != 0, "omitting actual StringList cleanup must fail"


def _sprite_probe(ctor_body: str, dtor_body: str, extent: int,
                  guard_damage: str | None = None, already_guarded: bool = False) -> str:
    sprite_header = (ROOT / "src/sprite.h").read_text()
    fields = sprite_header[sprite_header.index("  char *ppszFileName_;"):]
    fields = fields[:fields.index("\n};")]
    header = (ROOT / "src/newtechwin.h").read_text()
    aggregate = _block(header, "struct NewTechSpriteArray") + ";"
    assert "Sprite elements_[6];" in aggregate
    aggregate = aggregate.replace("elements_[6]", f"elements_[{extent}]", 1)
    storage = _block(header, "union NewTechSpriteStorage") + ";"
    # Host ABI projection only: source target storage is aligned for Win32.
    storage = storage.replace("{", "{\n  void *host_alignment;", 1)
    lifetime = _block(header, "struct NewTechSpriteLifetime") + ";"
    if guard_damage == "publish":
        needle, replacement = "NewTechSpritesCleanupGuard |= 1;", "/* omitted guard publication */"
    elif guard_damage == "upper_bits":
        needle, replacement = "NewTechSpritesCleanupGuard |= 1;", "NewTechSpritesCleanupGuard = 1;"
    elif guard_damage == "skip":
        needle = "if ((NewTechSpritesCleanupGuard & 1) != 0) return;"
        replacement = "/* omitted guard early return */"
    if guard_damage:
        assert lifetime.count(needle) == 1
        lifetime = lifetime.replace(needle, replacement, 1)
    body = ctor_body.replace("sizeof(Sprite)", "sizeof(HostSprite)")
    body = "HostSprite::HostSprite() " + body[:-1] + "\n  id = constructed_count; constructed[constructed_count++] = id;\n}"
    dtor = dtor_body[1:-1]
    source = r"""
#include <cstdlib>
#include <stdint.h>
#include <new>
static int constructed[12], constructed_count;
static int closed[12], close_count;
int SpriteMemoryUsed;
uint8_t NewTechSpritesCleanupGuard = INITIAL_GUARD;
struct HostSprite {
""" + fields + r"""
  int id;
  HostSprite();
  ~HostSprite() {
""" + dtor + r"""
  }
  void close() {
    if ((NewTechSpritesCleanupGuard & 1) == 0) std::abort();
    closed[close_count++] = id;
  }
};
typedef HostSprite Sprite;
""" + body + "\n" + aggregate + "\n" + storage + "\n" + lifetime + r"""
struct VerifyAtExit {
  ~VerifyAtExit() {
    if (close_count != EXPECTED_CLEANUPS || NewTechSpritesCleanupGuard != 0x81) std::abort();
    for (int i = 0; i != EXPECTED_CLEANUPS; ++i)
      if (closed[i] != 5 - i) std::abort();
  }
} verify_at_exit;
static NewTechSpriteLifetime NewTechSpritesOwner;
int main() {
  if (constructed_count != 6 || NewTechSpritesCleanupGuard != INITIAL_GUARD) return 51;
  for (int i = 0; i != 6; ++i) {
    const HostSprite& sprite = NewTechSpritesOwner.at(i);
    if (constructed[i] != i || sprite.ppszFileName_ != 0 || sprite.pcBits_ != 0 ||
        sprite.iSpriteWidth2_ != 0 || sprite.iSpriteWidth_ != 0 ||
        sprite.iSpriteHeight_ != 0 || sprite.iWidth_ != 0 || sprite.iHeight_ != 0 ||
        sprite.iLeftOffset_ != 0 || sprite.iTopOffset_ != 0 ||
        sprite.cTransparentIndex_ != 9 || sprite.fObj1Exists_ != 0) return 52;
  }
  if (SpriteMemoryUsed != 6 * static_cast<int>(sizeof(HostSprite))) return 53;
  return 0;
}
"""
    return source.replace("INITIAL_GUARD", "0x81" if already_guarded else "0x80").replace(
        "EXPECTED_CLEANUPS", "0" if already_guarded else "6")


def test_newtech_sprite_array_real_global_constructor_and_exit_cleanup(
    tmp_path: Path, host_compiler: str,
) -> None:
    impl = (ROOT / "src/sprite.cpp").read_text(encoding="utf-8")
    header = (ROOT / "src/sprite.h").read_text(encoding="utf-8")
    owner = (ROOT / "src/newtechwin.cpp").read_text(encoding="utf-8")
    use_header = (ROOT / "src/newtechwin.h").read_text(encoding="utf-8")
    declaration = re.search(r"^NewTechSpriteLifetime\s+NewTechSpritesOwner\s*;",
                            owner, re.MULTILINE)
    assert declaration
    assert "extern NewTechSpriteLifetime NewTechSpritesOwner;" in use_header
    assert "Sprite elements_[6];" in use_header
    compact_owner = re.sub(r"\s+", " ", owner)
    assert "set_report_icons(&NewTechSpritesOwner.at(0), &NewTechSpritesOwner.at(1), &NewTechSpritesOwner.at(2))" in compact_owner
    assert "set_report_icons(&NewTechSpritesOwner.at(3), &NewTechSpritesOwner.at(4), &NewTechSpritesOwner.at(5))" in compact_owner

    _mapped_record("src/newtechwin.cpp", 0x004838B0,
                   "??__Fg_NEWTECHWIN_SPRITES@@YAXXZ", (0x004838B0, 0x004838D8),
                   "_$E24", False)
    assert not (ROOT / "src/recovered/units/004838b0.cpp").exists()
    objects = [row for row in read_objects(ROOT, image_pin(ROOT))
               if row.origin.address == 0x00822610]
    assert len(objects) == 1
    assert objects[0].span is not None and objects[0].span.end == 0x00822718
    assert objects[0].symbol == "?g_NEWTECHWIN_SPRITES@@3PAVSprite@@A"
    assert objects[0].unit == "src/newtechwin.cpp"
    guard = [row for row in read_objects(ROOT, image_pin(ROOT))
             if row.origin.address == 0x00822604]
    assert len(guard) == 1 and guard[0].symbol == "?NewTechSpritesCleanupGuard@@3EA"

    image_path = ROOT / ".opensmacx/game/terranx_original.exe"
    if image_path.is_file():
        image = PeImage(image_path.read_bytes(), image_pin(ROOT))
        assert image.read_initial(Span(0x00822610, 0x00822718)) == bytes(0x108)

    ctor = _body(impl, "\nSprite::Sprite()")
    dtor_match = re.search(r"MEASURED\s+~Sprite\(\)\s*(\{[^}]*\})", header)
    assert dtor_match and dtor_match.group(1).strip() == "{ close(); }"
    baseline = _compile_run(tmp_path / "newtech-good", host_compiler,
                            _sprite_probe(ctor, dtor_match.group(1), 6))
    assert baseline.returncode == 0, baseline.stdout + baseline.stderr

    no_type_init = ctor.replace("= 9;", "= 0;", 1)
    mutant = _compile_run(tmp_path / "newtech-init-mutant", host_compiler,
                          _sprite_probe(no_type_init, dtor_match.group(1), 6))
    assert mutant.returncode != 0, "mutating Sprite's actual type-byte initialization must fail"

    no_close = dtor_match.group(1).replace("close();", "/* mutation omitted close */", 1)
    mutant = _compile_run(tmp_path / "newtech-dtor-mutant", host_compiler,
                          _sprite_probe(ctor, no_close, 6))
    assert mutant.returncode != 0, "omitting actual Sprite destructor delegation must fail"

    extent_mutant = _compile_run(tmp_path / "newtech-extent-mutant", host_compiler,
                                 _sprite_probe(ctor, dtor_match.group(1), 5))
    assert extent_mutant.returncode != 0, "changing the six-Sprite object extent must fail"

    for damage in ("publish", "upper_bits"):
        mutant = _compile_run(tmp_path / f"newtech-guard-{damage}", host_compiler,
                              _sprite_probe(ctor, dtor_match.group(1), 6, damage))
        assert mutant.returncode != 0, "guard publication and upper-bit preservation are load-bearing"


def test_newtech_guarded_exit_skips_cleanup_without_resetting_guard(
    tmp_path: Path, host_compiler: str,
) -> None:
    impl = (ROOT / "src/sprite.cpp").read_text()
    header = (ROOT / "src/sprite.h").read_text()
    ctor = _body(impl, "\nSprite::Sprite()")
    dtor, = re.findall(r"MEASURED\s+~Sprite\(\)\s*(\{[^}]*\})", header)
    source = _sprite_probe(ctor, dtor, 6, already_guarded=True)
    result = _compile_run(tmp_path / "newtech-already-guarded", host_compiler, source)
    assert result.returncode == 0, result.stdout + result.stderr
    mutant = _sprite_probe(ctor, dtor, 6, "skip", already_guarded=True)
    result = _compile_run(tmp_path / "newtech-guard-skip-mutant", host_compiler, mutant)
    assert result.returncode != 0, "published cleanup bit must suppress element destruction"
