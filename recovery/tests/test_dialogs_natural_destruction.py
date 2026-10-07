"""Post-migration source/host control for Dialogs' real deleting owner.

This is intentionally not a VC6 ABI oracle. Fresh COFF/MAP/PE/PDB table and
thunk selection are checked by the coordinator's physical native qualifier.
"""
from __future__ import annotations

import hashlib
import re
import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest

from match_recovery.source import Mode, State, read, read_file


HERE = Path(__file__).resolve()
ROOT = next(parent for parent in HERE.parents if (parent / "src/IMAGE_PIN").is_file())
EXPECTED = {
    0x406910: {
        "name": "??1Dialogs@@QAE@XZ",
        "spans": ((0x406910, 0x406A74), (0x6509C0, 0x650AAA)),
        "kind": "game", "byte_exact": False, "semantic": False,
        "envelope": "0623668967ee859b60c23ba2cf76864eada15cf459e6cb98509f1c9279cdfc1e",
    },
    0x4070F0: {
        "name": "??_Gthunk1_Dialogs@@UAEPAXI@Z",
        "spans": ((0x4070F0, 0x4070F8),),
        "kind": "game", "byte_exact": False, "semantic": False,
        "envelope": "5f7ffc3faaa2a0e29883572eb19e16723aa0af57213e3366cc8f7d9839e49e22",
    },
    0x407100: {
        "name": "??_GDialogs@@UAEPAXI@Z",
        "spans": ((0x407100, 0x407141),),
        "kind": "game", "byte_exact": False, "semantic": False,
        "envelope": "9a50537c8cb580fe297a1bb99a02ca9d6e8b91326b73f7b388512cb5feed53f3",
    },
    0x407280: {
        "name": "??_Gthunk2_Dialogs@@UAEPAXI@Z",
        "spans": ((0x407280, 0x407291),),
        "kind": "game", "byte_exact": False, "semantic": False,
        "envelope": "c5f05ad9a164599b5f6fb5d03db0777734d00a84bd9fa1a6ce7258de73c9e158",
    },
    0x4072A0: {
        "name": "??_Gthunk3_Dialogs@@UAEPAXI@Z",
        "spans": ((0x4072A0, 0x4072AE),),
        "kind": "game", "byte_exact": False, "semantic": False,
        "envelope": "58b44dea54337d86068c5231ff885b7779001c7e715a3e4567ce08474766e7ac",
    },
}


def text(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8")


def extract_definition(source: str, signature: str) -> str:
    assert source.count(signature) == 1, signature
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    quote = None
    escaped = False
    line_comment = block_comment = False
    index = brace
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if line_comment:
            if char == "\n":
                line_comment = False
            index += 1
            continue
        if block_comment:
            if char == "*" and nxt == "/":
                block_comment = False
                index += 2
                continue
            index += 1
            continue
        if quote:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == quote:
                quote = None
            index += 1
            continue
        if char == "/" and nxt == "/":
            line_comment = True
            index += 2
            continue
        if char == "/" and nxt == "*":
            block_comment = True
            index += 2
            continue
        if char in ("'", '"'):
            quote = char
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
        index += 1
    raise AssertionError(f"unterminated source definition: {signature}")


def history_envelopes(source: str):
    pattern = re.compile(
        r"/\*\n \* RECOVERY_HISTORY: v1 address=0x[0-9A-F]{8} "
        r"source_sha256=[0-9a-f]{64} envelope_sha256=[0-9a-f]{64}\n"
        r"(?P<payload>(?: \* \[ARCHIVE\].*\n)*) \*/")
    for match in pattern.finditer(source):
        payload = []
        for line in match.group("payload").splitlines():
            if line == " * [ARCHIVE]":
                payload.append("")
            else:
                assert line.startswith(" * [ARCHIVE] "), line
                payload.append(line[len(" * [ARCHIVE] "):])
        restored = "\n".join(payload).replace("* /", "*/")
        yield restored.replace("// original marker:", "// ORIGINAL:")


def without_history(source: str) -> str:
    return re.sub(
        r"/\*\n \* RECOVERY_HISTORY: v1 address=0x[0-9A-F]{8} .*?\n \*/\n?",
        "", source, flags=re.DOTALL)


def test_all_original_dialogs_destructor_keys_move_to_real_owner_without_loss():
    owner = text("src/dialogs.cpp")
    rows = {}
    for row in read(ROOT / "src"):
        if row.address in EXPECTED:
            assert row.address not in rows
            rows[row.address] = row
    assert set(rows) == set(EXPECTED)
    archives = tuple(history_envelopes(owner))

    for address, expected in EXPECTED.items():
        row = rows[address]
        assert row.path == ROOT / "src/dialogs.cpp"
        assert row.name == expected["name"]
        assert row.image_spans == expected["spans"]
        assert row.mode is Mode.FILE and row.state is State.IMPLEMENTED
        assert row.kind == expected["kind"]
        assert row.byte_exact is expected["byte_exact"]
        assert row.semantic is expected["semantic"]
        # E/G symbol names are deliberately not guessed here; physical emitted
        # identities and selected table cells are an independent native check.
        assert any(hashlib.sha256(item.encode()).hexdigest() == expected["envelope"]
                   for item in archives), f"full original envelope lost: {address:08X}"


def test_no_manual_dialogs_scalar_deleting_cascade_remains():
    dialogs_h = text("src/dialogs.h")
    dialogs_cpp = text("src/dialogs.cpp")
    assert not (ROOT / "src/adjustor_thunks.cpp").exists()
    assert not (ROOT / "src/adjustor_thunks.h").exists()
    assert re.search(r"virtual\s+~Dialogs\s*\(\s*\)\s*;", dialogs_h)
    assert "Dialogs::~Dialogs()" in dialogs_cpp
    assert "HISTORICAL NOTE: the archived TRIED text predates the real Dialogs base/member ownership." in dialogs_cpp
    assert "SpriteBox::close and its Spot/SpriteList members now own this teardown" in dialogs_cpp
    assert "HISTORICAL TRIED (superseded by recovered Dialogs/SpriteBox ownership)" in dialogs_cpp
    assert '#include "original_seam.h"' not in dialogs_cpp
    assert '#include "original_seam.h"' not in dialogs_h
    assert 'func_dialogs_fwd1' not in dialogs_h and 'func_dialogs_fwd2' not in dialogs_h
    active = "\n".join(without_history(item) for item in
                       (dialogs_h, dialogs_cpp))
    for obsolete in (
        "Dialogs::destroy()", "dialogs_scalar_dtor_redirect",
        "DialogsDestructorAdjustment", "DialogsScalarDeleteTarget",
        "func_dialogs_teardown", "DialogsEditGroupDestructor",
        "DialogsSpriteBoxDestructor", "DialogsCheckBoxDestructor",
        "guarded_member(",
        "adjust_dialogs1_scalar_delete", "adjust_dialogs2_scalar_delete",
        "adjust_dialogs3_scalar_delete",
    ):
        assert obsolete not in active


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_dialogs_dtor_body_runs_real_reverse_base_lifetime(compiler):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} not installed")
    dialogs_h = text("src/dialogs.h")
    dialogs_cpp = text("src/dialogs.cpp")
    sprite_h = text("src/spritebox.h")
    sprite_cpp = text("src/spritebox.cpp")
    assert re.search(
        r"class\s+Dialogs\s*:\s*public\s+ListBox,\s*"
        r"public\s+RadioButton,\s*public\s+CheckBox,\s*"
        r"public\s+SpriteBox,\s*public\s+EditGroup", dialogs_h)
    assert sprite_h.index("Spot spots_;") < sprite_h.index("SpriteList list_;")
    assert "SpriteBox::~SpriteBox()" in sprite_cpp
    assert "SpriteListBase::~SpriteListBase()" in sprite_cpp
    spot_h = text("src/spot.h")
    assert "void shutdown()" in spot_h and "free(spots_)" in spot_h
    body = extract_definition(dialogs_cpp, "Dialogs::~Dialogs()")
    listbox_dtor = extract_definition(text("src/listbox.cpp"), "ListBox::~ListBox()")

    fixture = r'''#include <string>
static std::string events;
static void event(char value) { events += value; }
struct GraphicWin { virtual ~GraphicWin() { event('G'); } };
struct Dialog : virtual GraphicWin { virtual ~Dialog() { event('D'); } };
struct ListBox : virtual GraphicWin, virtual Dialog {
  virtual ~ListBox();
  void close() { event('C'); }
};
'''+listbox_dtor+r'''
struct RadioButton : virtual GraphicWin, virtual Dialog {
  virtual ~RadioButton() { event('R'); }
};
struct CheckBox : virtual GraphicWin, virtual Dialog {
  virtual ~CheckBox() { event('K'); }
};
struct SpriteBox : virtual GraphicWin, virtual Dialog {
  virtual ~SpriteBox() { event('S'); }
};
struct EditGroup : virtual GraphicWin, virtual Dialog {
  virtual ~EditGroup() { event('E'); }
};
struct Dialogs : ListBox, RadioButton, CheckBox, SpriteBox, EditGroup {
  virtual ~Dialogs();
  void close() {}
};
'''+body+r'''
int main() {
  { Dialogs value; }
  return events == "ESKRCDG" ? 0 : 1;
}
'''
    with tempfile.TemporaryDirectory(prefix="dialogs-natural-dtor-") as directory:
        cpp = Path(directory) / "control.cpp"
        exe = Path(directory) / "control"
        cpp.write_text(fixture)
        built = subprocess.run(
            [executable, "-std=c++98", "-O2", "-Wall", "-Wextra",
             "-Werror=return-type", str(cpp), "-o", str(exe)],
            capture_output=True, text=True, check=False)
        assert built.returncode == 0, built.stdout + built.stderr
        ran = subprocess.run([str(exe)], capture_output=True, text=True, check=False)
        assert ran.returncode == 0, (ran.returncode, ran.stdout, ran.stderr)
