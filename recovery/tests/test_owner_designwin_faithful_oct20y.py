import os
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
IMAGE = Path(os.environ.get(
    "OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"
))
SOURCE = ROOT / "src/designwin.cpp"
SIGNATURE = "void DesignWin::suggest_abil(int ability)"


def actual_body(*, bounded_bonus_loop=False):
    records = read_file(SOURCE)
    record, = [row for row in records if row.address == 0x00439D30]
    assert Path(record.path).resolve() == SOURCE.resolve()
    assert record.region and SIGNATURE in record.region
    text = SOURCE.read_text()
    start = text.index(SIGNATURE)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            body = text[start:end + 1]
            break
    else:
        raise AssertionError("unterminated actual DesignWin::suggest_abil")
    if bounded_bonus_loop:
        body = body.replace(
            "for (int i = 0; i < 0x29; ++i)",
            "for (int i = 0; i < local.faction_bonus_count && i < 8; ++i)",
            1,
        )
    return body


HARNESS = r'''#include <cassert>
#include <cstring>
typedef unsigned int uint32_t;
struct AbilityRecord { int preq_tech; };
struct Player {
  // Test-only widened tail: production's member arrays are eight entries,
  // but the pinned body intentionally reads a 0x29-dword window. Widening
  // avoids invoking host UB while testing that read-loop bound; this is not
  // evidence for production layout or a safe source contract.
  int faction_bonus_count;
  int faction_bonus_id[0x2A];
  int faction_bonus_val1[0x2A];
};
struct Console { int faction_id_; };
struct GameData { struct Fields { unsigned int game_preferences_; } fields_; };
struct RulesRecord { int tech_two_spec_abilities; };
static Player Players[1];
static Console ConsoleGlobal;
static GameData GameDataStateStorage;
static RulesRecord Rules;
static AbilityRecord Ability[29];
static int has_tech(int, int) { return 1; }
static int abil_cond(int, int, int, int) { return 1; }
static int bit_count(int value) {
  int count = 0;
  for (; value; value &= value - 1) ++count;
  return count;
}
static int abil_index(int value) {
  int index = 0;
  while ((value & 1) == 0) { value >>= 1; ++index; }
  return index;
}
class DesignWin {
public:
  uint32_t protoAbilityFlags_;
  int protoAbilityIDs_[2];
  int factionID_, protoChassisType_, protoWeaponType_, protoArmorType_;
  void check_abil() {}
  void suggest_abil(int ability);
};
/* ACTUAL_SUGGEST_ABIL */
int main() {
  DesignWin win;
  win.protoAbilityFlags_ = 0;
  win.protoAbilityIDs_[0] = win.protoAbilityIDs_[1] = -1;
  win.factionID_ = win.protoChassisType_ = win.protoWeaponType_ = win.protoArmorType_ = 0;
  ConsoleGlobal.faction_id_ = 0;
  GameDataStateStorage.fields_.game_preferences_ = 0x4000;
  Players[0].faction_bonus_count = 0;
  std::memset(Players[0].faction_bonus_id, 0, sizeof(Players[0].faction_bonus_id));
  std::memset(Players[0].faction_bonus_val1, 0, sizeof(Players[0].faction_bonus_val1));

  // A stale free-ability record beyond the declared eight slots still blocks
  // the requested mask in the original fixed 0x29-entry scan.
  Players[0].faction_bonus_id[8] = 15;
  Players[0].faction_bonus_val1[8] = 5;
  win.suggest_abil(1u << 5);
  assert(win.protoAbilityFlags_ == 0);

  Players[0].faction_bonus_id[8] = 0;
  Players[0].faction_bonus_id[0x28] = 15;
  Players[0].faction_bonus_val1[0x28] = 5;
  win.suggest_abil(1u << 5);
  assert(win.protoAbilityFlags_ == 0);

  Players[0].faction_bonus_id[0x28] = 0;
  Players[0].faction_bonus_id[0x29] = 15;
  Players[0].faction_bonus_val1[0x29] = 5;
  win.suggest_abil(1u << 5);
  assert(win.protoAbilityFlags_ == (1u << 5));
  return 0;
}
'''


def compile_and_run(tmp_path, compiler, *, bounded_bonus_loop=False,
                    expect_success=True):
    executable = shutil.which(compiler)
    if not executable:
        pytest.skip(compiler + " unavailable")
    suffix = "bounded" if bounded_bonus_loop else "actual"
    source = tmp_path / ("designwin-" + suffix + ".cpp")
    binary = tmp_path / ("designwin-" + suffix)
    source.write_text(HARNESS.replace(
        "/* ACTUAL_SUGGEST_ABIL */",
        actual_body(bounded_bonus_loop=bounded_bonus_loop),
    ))
    built = subprocess.run(
        [executable, "-std=c++03", "-O1", "-Wall", "-Wextra", "-Werror",
         "-Wno-unused-parameter", "-pedantic", str(source), "-o", str(binary)],
        capture_output=True, text=True, timeout=60,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True,
                            timeout=30)
    if expect_success:
        assert result.returncode == 0, result.stdout + result.stderr
    else:
        assert result.returncode != 0, "bounded bonus scan mutant survived"
        assert "protoAbilityFlags_ == 0" in result.stderr


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_suggestion_scans_original_stale_bonus_tail(compiler, tmp_path):
    compile_and_run(tmp_path, compiler)


def test_bounded_bonus_scan_mutant_misses_stale_original_entry(tmp_path):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("no host C++ compiler")
    compile_and_run(tmp_path, compiler, bounded_bonus_loop=True,
                    expect_success=False)


def test_pinned_suggestion_scans_41_slots_from_named_player_fields():
    import hashlib
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin

    assert IMAGE.is_absolute() and IMAGE.is_file()
    data = IMAGE.read_bytes()
    assert hashlib.sha256(data).hexdigest() == (
        "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
    )
    image = PeImage(data, image_pin(ROOT))
    span = Span(0x00439D30, 0x00439EA3)
    rows = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(image.read(span), span.start))
    assert rows and rows[0].address == span.start
    assert rows[-1].address + rows[-1].size == span.end
    by_address = {row.address: (row.mnemonic, row.op_str) for row in rows}
    assert by_address[0x00439E10] == (
        "cmp", "dword ptr [ecx*4 + 0x946f60], 0xf"
    )
    assert by_address[0x00439E31] == (
        "cmp", "dword ptr [edx*4 + 0x946f80], eax"
    )
    assert by_address[0x00439E40] == ("cmp", "edi, 0x29")
    assert by_address[0x00439E43] == ("jl", "0x439e0d")
