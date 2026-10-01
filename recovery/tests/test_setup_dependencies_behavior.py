"""Host controls for the three reset helpers called by setup_game."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def source_function(signature: str) -> str:
    source = (ROOT / "src/game.cpp").read_text()
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for pos in range(opening, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start : pos + 1]
    raise AssertionError(f"unterminated function: {signature}")


HARNESS = r"""
#include <cassert>
#include <cstdint>
#include <cstring>
#define __cdecl
enum { MaxPlayerNum=8 };

BODY_MONUMENT_LAYOUT
MonumentFaction FactionMonuments[MaxPlayerNum];
namespace ProductOfferLayout {
BODY_OFFER_LAYOUT
}

int offer_write_log[8*8*8];
int offer_write_count;
struct TrackedInt {
  int value;
  int identity;
  TrackedInt &operator=(int next) {
    assert(next == 0 && offer_write_count < 8*8*8);
    value = next;
    offer_write_log[offer_write_count++] = identity;
    return *this;
  }
};
struct OfferResetCell {
  TrackedInt word_0_, word_1_, word_2_, word_3_;
  uint8_t opaque_[0x80];
};
OfferResetCell OfferResetCells[MaxPlayerNum][MaxPlayerNum];

int TamperState93A9AC, TamperState93A9B0, TamperState93A9B4;
int TamperDialogFaction[MaxPlayerNum], TamperDialogValue[MaxPlayerNum];

BODY_MONUMENT
BODY_OFFERS
BODY_TAMPER

static void check_offer_word(const TrackedInt &word, int expected_id) {
  assert(word.value == 0 && word.identity == expected_id);
}

int main() {
  std::memset(FactionMonuments, 0xA5, sizeof FactionMonuments);
  clear_monuments();
  for (int faction=0; faction<8; ++faction) {
    const unsigned char *bytes =
      reinterpret_cast<const unsigned char *>(&FactionMonuments[faction]);
    for (int offset=0; offset<0x4F4; ++offset) {
      bool cleared=false;
      for (int slot=0; slot<21; ++slot)
        if (offset>=slot*0x38+0x10 && offset<slot*0x38+0x14) cleared=true;
      if (cleared) assert(bytes[offset]==0);
      else if (offset>=0x498 && offset<0x49C) assert(bytes[offset]==0xFF);
      else assert(bytes[offset]==0xA5);
    }
    assert(reinterpret_cast<const char *>(&FactionMonuments[faction].latest_type_) -
           reinterpret_cast<const char *>(&FactionMonuments[faction]) == 0x498);
    if (faction != 0)
      assert(reinterpret_cast<const char *>(&FactionMonuments[faction]) -
             reinterpret_cast<const char *>(&FactionMonuments[faction-1]) == 0x4F4);
  }

  std::memset(OfferResetCells, 0x6B, sizeof OfferResetCells);
  offer_write_count = 0;
  for (int row=0; row<8; ++row) {
    for (int column=0; column<8; ++column) {
      OfferResetCell &cell = OfferResetCells[row][column];
      TrackedInt *words[4] = {&cell.word_0_, &cell.word_1_, &cell.word_2_, &cell.word_3_};
      for (int field=0; field<4; ++field) {
        words[field]->value = 1000 + row*32 + column*4 + field;
        words[field]->identity = (row*8 + column)*4 + field;
      }
    }
  }
  clear_all_offers();
  int expected_write = 0;
  for (int row=0; row<8; ++row) {
    for (int column=0; column<8; ++column) {
      const int flat = (row*8 + column)*4;
      const int transpose = (column*8 + row)*4;
      const int expected[8] = {flat, flat+1, transpose, transpose+1,
                               flat+2, flat+3, transpose+2, transpose+3};
      for (int i=0; i<8; ++i)
        assert(offer_write_log[expected_write++] == expected[i]);
    }
  }
  assert(offer_write_count == 8*8*8);
  for (int row=0; row<8; ++row) {
    for (int column=0; column<8; ++column) {
      OfferResetCell &cell = OfferResetCells[row][column];
      check_offer_word(cell.word_0_, (row*8+column)*4);
      check_offer_word(cell.word_1_, (row*8+column)*4+1);
      check_offer_word(cell.word_2_, (row*8+column)*4+2);
      check_offer_word(cell.word_3_, (row*8+column)*4+3);
      for (int byte=0; byte<0x80; ++byte) assert(cell.opaque_[byte] == 0x6B);
    }
  }

  TamperState93A9AC=11; TamperState93A9B0=22; TamperState93A9B4=33;
  for (int faction=0; faction<8; ++faction) {
    TamperDialogFaction[faction]=40+faction;
    TamperDialogValue[faction]=80+faction;
  }
  const int unrelated=0x13579BDF;
  clear_tamper();
  assert(TamperState93A9AC==0 && TamperState93A9B0==0 && TamperState93A9B4==0);
  for (int faction=0; faction<8; ++faction)
    assert(TamperDialogFaction[faction]==0 && TamperDialogValue[faction]==0);
  assert(unrelated==0x13579BDF);
}
"""


def test_setup_reset_helpers_actual_typed_storage_and_order(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    harness = HARNESS
    header = (ROOT / "src/game.h").read_text()
    monument = header[header.index("struct MonumentRecord {") :
                      header.index("extern MonumentFaction FactionMonuments")]
    offer = header[header.index("struct OfferResetCell {") :
                   header.index("extern OfferResetCell OfferResetCells")]
    harness = harness.replace("BODY_MONUMENT_LAYOUT", monument)
    harness = harness.replace("BODY_OFFER_LAYOUT", offer)
    harness = harness.replace(
        "BODY_MONUMENT",
        source_function("void __cdecl clear_monuments() {"),
    )
    harness = harness.replace(
        "BODY_OFFERS",
        source_function("void __cdecl clear_all_offers() {"),
    )
    harness = harness.replace(
        "BODY_TAMPER",
        source_function("void __cdecl clear_tamper() {"),
    )
    executable = tmp_path / "setup-reset-helpers-host-control"
    built = subprocess.run(
        [compiler, "-std=c++17", "-x", "c++", "-", "-o", str(executable)],
        input=harness,
        text=True,
        capture_output=True,
    )
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(executable)], capture_output=True)
    assert result.returncode == 0, result.stderr.decode()
