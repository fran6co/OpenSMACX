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
                   *, byte_exact: bool | None = None) -> None:
    records = [row for row in read_file(ROOT / path) if row.address == address]
    assert len(records) == 1
    row = records[0]
    assert row.name == name
    assert row.mode is Mode.FILE
    assert row.image_spans == (span,)
    # Generated callback counters vary with source grouping. Preserve the
    # current symbol's presence here; native COFF checks establish selection.
    assert row.symbol
    if byte_exact is True:
        assert row.byte_exact is True


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
  void remove_all() { removed[remove_count++] = test_id_; }
  virtual ~StringStruct() { base_done[base_count++] = test_id_; }
};
struct StringList : StringStruct {
  ~StringList() {
""" + list_dtor_body[1:-1] + r"""
    derived_done[derived_count++] = test_id_;
  }
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
                   "sub_51df80", (0x0051DF80, 0x0051DFA4))
    _mapped_record("src/messagewin.cpp", 0x0051DFB0,
                   "sub_51dfb0", (0x0051DFB0, 0x0051DFC4), byte_exact=True)
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

    shared = (ROOT / "src/stringstruct.h").read_text(encoding="utf-8")
    ctor_body = _body(shared, "StringStruct::StringStruct()")
    dtor_body = _body(shared, "~StringList()")
    assert "~StringList() {" in shared
    base_dtor = _body(shared, "~StringStruct()")
    assert "destroy_owned_entries(this);" in base_dtor
    assert "StringStruct::remove_all();" in dtor_body and "current_position_ = 0;" in dtor_body

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


def _sprite_probe(ctor_body: str, dtor_body: str, extent: int) -> str:
    sprite_header = (ROOT / "src/sprite.h").read_text()
    fields = sprite_header[sprite_header.index("  char *ppszFileName_;"):]
    fields = fields[:fields.index("\n};")]
    header = (ROOT / "src/newtechwin.h").read_text()
    declaration, = re.findall(r"^  static Sprite technology_sprites_\[6\];$", header, re.M)
    declaration = declaration.replace("Sprite", "HostSprite")
    owner = (ROOT / "src/newtechwin.cpp").read_text()
    definition, = re.findall(r"^Sprite NewTechWin::technology_sprites_\[6\];$", owner, re.M)
    definition = definition.replace("Sprite", "HostSprite")
    declaration = declaration.replace("[6]", f"[{extent}]")
    definition = definition.replace("[6]", f"[{extent}]")
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
struct HostSprite {
""" + fields + r"""
  int id;
  HostSprite();
  ~HostSprite() {
""" + dtor + r"""
  }
  void close() {
    closed[close_count++] = id;
  }
};
typedef HostSprite Sprite;
""" + body + r"""
struct VerifyAtExit {
  ~VerifyAtExit() {
    if (close_count != 6) std::abort();
    for (int i = 0; i != 6; ++i)
      if (closed[i] != 5 - i) std::abort();
  }
} verify_at_exit;
struct NewTechWin { """ + declaration + r""" };
""" + definition + r"""
int main() {
  if (constructed_count != 6) return 51;
  for (int i = 0; i != 6; ++i) {
    const HostSprite& sprite = NewTechWin::technology_sprites_[i];
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
    return source


def test_newtech_sprite_array_real_global_constructor_and_exit_cleanup(
    tmp_path: Path, host_compiler: str,
) -> None:
    impl = (ROOT / "src/sprite.cpp").read_text(encoding="utf-8")
    header = (ROOT / "src/sprite.h").read_text(encoding="utf-8")
    owner = (ROOT / "src/newtechwin.cpp").read_text(encoding="utf-8")
    use_header = (ROOT / "src/newtechwin.h").read_text(encoding="utf-8")
    declaration = re.search(r"^Sprite NewTechWin::technology_sprites_\[6\]\s*;",
                            owner, re.MULTILINE)
    assert declaration
    assert re.search(r"^  static Sprite technology_sprites_\[6\];$", use_header, re.MULTILINE)
    assert "NewTechSpriteLifetime" not in use_header
    assert "NewTechSpritesCleanupGuard" not in use_header
    compact_owner = re.sub(r"\s+", " ", owner)
    assert "set_report_icons(&technology_sprites_[0], &technology_sprites_[1], &technology_sprites_[2])" in compact_owner
    assert "set_report_icons(&technology_sprites_[3], &technology_sprites_[4], &technology_sprites_[5])" in compact_owner

    callback = [row for row in read_file(ROOT / "src/newtechwin.cpp")
                if row.address == 0x004838B0]
    assert len(callback) == 1
    assert callback[0].name == "??__Fg_NEWTECHWIN_SPRITES@@YAXXZ"
    assert callback[0].mode is Mode.FILE
    assert callback[0].image_spans == ((0x004838B0, 0x004838D8),)
    # This generated callback symbol and exactness are current compiler facts;
    # preserve original identity/span here and let native evidence qualify it.
    assert callback[0].symbol
    assert not (ROOT / "src/recovered/units/004838b0.cpp").exists()
    objects = [row for row in read_objects(ROOT, image_pin(ROOT))
               if row.origin.address == 0x00822610]
    assert len(objects) == 1
    assert objects[0].span is not None and objects[0].span.end == 0x00822718
    # `symbol` is the historical original identity. `target_symbol` is the
    # current emitted member; the object has no parent/member storage path.
    assert objects[0].symbol == "?g_NEWTECHWIN_SPRITES@@3PAVSprite@@A"
    assert objects[0].emitted_symbol == "?technology_sprites_@NewTechWin@@2PAVSprite@@A"
    assert objects[0].target_symbol == objects[0].emitted_symbol
    assert objects[0].parent_symbol is None and objects[0].member_path == ()
    assert objects[0].unit == "src/newtechwin.cpp"
    image_path = ROOT / ".opensmacx/game/terranx_original.exe"
    if image_path.is_file():
        image = PeImage(image_path.read_bytes(), image_pin(ROOT))
        assert image.read_initial(Span(0x00822610, 0x00822718)) == bytes(0x108)

    ctor = _body(impl, "\nSprite::Sprite()")
    dtor_match = re.search(r"~Sprite\(\)\s*(\{[^}]*\})", header)
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
