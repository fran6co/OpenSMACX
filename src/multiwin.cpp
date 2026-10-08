// ORIGINAL: 0x0047A760 ??_GMultiWin@@WEEE@AEPAXI@Z 0x0047A760-0x0047A76B FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original ??_GMultiWin@@WEEE@AEPAXI@Z at 0x0047a760 is an 11-byte secondary scalar deleting adjustor; it subtracts 0x444 from ECX and tail-jumps to the complete-object MultiWin scalar deleting entry without touching stack or flags.", "Historical seed correction: the earlier `??3` deallocation interpretation was refuted; the original is a `??_G` scalar-deleting adjustor. `WEEE` demangles to adjustor{1092}, equal to 0x444.", "Pinned MultiWin constructor at 0x0047a6a1 stores Buffer-view vftable 0x0066c9e8 at this+0x444; the source owner is the typed MultiWin GraphicWin/Buffer multiple-inheritance view.", "Current src/multiwin.cpp COFF Buffer-view vftable first slot selects generated ??_EMultiWin@@WEEE@AEPAXI@Z; its body performs the same 0x444 adjustment and preserves the flag argument.", "Current MultiWin COFF weak external ??_EMultiWin@@UAEPAXI@Z records search mode 2 (library search) and fallback target ??_GMultiWin@@UAEPAXI@Z; this linked image resolves both primary labels to the same physical class deleting body. The object fallback and linked address are per-build evidence, not a general E/G equivalence or owner-selection rule.", "That measured current primary body calls MultiWin::~MultiWin, tests flags bit 0, calls operator delete only when bit 0 is set, returns the saved object pointer, and uses ret 4; this is the per-class measured E-to-G scalar path, not a general vector/scalar equivalence.", "The removed transitional C++ names were ?adjust_this_multi_win@@YIPAXPAX0I@Z and ?scalar_delete_multi_win@@YIPAXPAX0I@Z; no callers exist outside deleting_thunks.cpp/.h, and the current generated Buffer vftable uses the class-owned E adjustor instead.", "Historical seed metadata noted flags hidden;sp_ready;purged_ok, zero direct callers, and no direct call targets. These remain historical observations only; they do not establish virtual-dispatch completeness or acceptance.", "The old secondary record had no BYTE_EXACT or SEMANTIC claim. Its original identity, full span, kind and parsed history are moved unchanged; no matching claim is granted."], "reason": "Bind the pinned secondary scalar deleting identity for MultiWin to its measured genuine Buffer-view generated adjustor. Keep the original G identity and point its current symbol fact at the owner-emitted E adjustor; the per-object weak fallback and bit-0 path above establish why this exact current implementation preserves the original operation."}
// symbol    ??_EMultiWin@@WEEE@AEPAXI@Z
// kind      game
// ORIGINAL: 0x0047A6C0 ??_GMultiWin@@UAEPAXI@Z 0x0047A6C0-0x0047A6E1 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x0047A6C0-0x0047A6E1", "Pinned class constructor ??0MultiWin@@QAE@XZ at 0x0047A590 stores original class/view table VA 0x66c9f0 at 0x0047A69B (mov dword ptr [esi], 0x66c9f0); the current genuine owner vftable ??_7MultiWin@@6BWin@@@ selects generated ??_GMultiWin@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_EMultiWin@@UAEPAXI@Z (mode 2).", "Pinned ??_GMultiWin@@UAEPAXI@Z at 0x0047A6C0 calls the source-mapped ??1MultiWin@@QAE@XZ at 0x0047A430, then tests byte ptr [ebp + 8], 1; je 0x47a6da bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/multiwin.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original MultiWin scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GMultiWin@@UAEPAXI@Z
// kind      game
#include "stdafx.h"
#include "multiwin.h"

#include "netdaemon.h"
#include "netmsg.h"
#include "pulldown.h"
#include "infowin.h"
#include "reportif.h"
#include "game.h"
#include "console.h"
#include "faction.h"
#include "palette.h"
#include "wave.h"
#include "base.h"
#include "council.h"
#include "councwin.h"  // CouncilWindow's existing +0xA24/+0xA3C/+0xA54 storage
#include "lock.h"
#include "strings.h"
#include "alpha.h"
#include "messagewin.h"
#include "maininterface.h"
#include "general.h"
#include "statuswin.h"  // Shared source-owned layout initialization flags.
#include "fx.h"
#include "time.h"
#include "popup.h"
#include <stdlib.h>     // _itoa

// Existing source-owned storage, defined in netdaemon.cpp/game.cpp. The
// original body reads the wait accumulator and turn-control word directly.
extern int NetDaemonWaitTicks;
extern int TurnControlWord93A954;
extern int MultiWinState7FFF78;
extern uint32_t MultiWinState7FFF7C;

static_assert(offsetof(PlayerData, time_bonus_count) == 0x2054,
              "MultiWin time-bonus source offset");
static const char CaretSeparator[] = "^";  // original string at 0x006867C0

// MultiWin::draw_time_bonus_markers, original 0x00479270. Add the supplied
// rectangle to the time-bonus hotspot and render its caret markers.
// The source header now declares the measured original member on its genuine owner.
/*
Purpose: Register the multiplayer time-bonus hotspot and draw its caret markers.
// ORIGINAL: 0x00479270 ?UNK1@MultiWin@@QAEXPAURECT@@@Z 0x00479270-0x0047932C FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned 0x00479270-0x0047932C is the original thiscall member ?UNK1@MultiWin@@QAEXPAURECT@@@Z; decoded body adds Spot position 3/type 2 before reading the local faction and writing centered text.", "Current MultiWin owns Spot as spot_ at +0x28AC; its GraphicWin base has the source-owned Buffer secondary base at +0x444. The original write_cent_l receiver adjustment is therefore static_cast<Buffer *>(this).", "PlayerData::time_bonus_count is the existing typed member at +0x2054 (asserted in multiwin.cpp); faction.h documents the time-control extra seconds, and faction/game/netdaemon sources produce and update it.", "The text uses source-owned StringTemp and BufferJoinSpace (0x00682820); the single caret byte at 0x006867C0 is only a strcat source, so the owner defines the same immutable character without relying on that image address."], "reason": "Move the original MultiWin member into its genuine class owner and replace raw object offsets with its existing Spot, Buffer base and PlayerData time-bonus members. Preserve the measured branch/order and historical match obligation."}
// symbol    ?draw_time_bonus_markers@MultiWin@@QAEXPAUtagRECT@@@Z
// SUPERSEDED_BINDING_HISTORY: v1 {"evidence": ["Original479270..47932C; retired seed git:117282ee:src/unrecovered/00479270.cpp records the same identity and spans.", "The receiver's Spot subobject at +0x28AC (spot.h's own add/3,2 overload) and the +0x444 Buffer (write_cent_l is buffer.h's own) carry the walk; the difficulty table is the 0x833-dword PlayerData stride faction.h pins, read at the +0x2054 record offset through the local-faction slot 0x00939284.", "The caret row is built into strings.h's StringTemp (0x009B86A0) from the one-space and caret .rdata strings exactly as the seed strcat-walks them."], "reason": "Coverage transcription of the multiplayer spot label with the receiver spelled explicitly because MultiWin has no header declaration yet."}
Status: Coverage transcription - unclaimed body
*/
void MultiWin::draw_time_bonus_markers(RECT *rect) {
    const int top = rect->top;
    const int height = rect->bottom - top;
    const int left = rect->left;
    const int width = rect->right - left;
    spot_.add(3, 2, left, top, width, height);

    const int time_bonus = PlayersData[ConsoleGlobal.faction_id_].time_bonus_count;
    if (time_bonus >= 0) {
        int carets = (time_bonus > 3) ? 3 : time_bonus;
        if (carets != 0) {
            StringTemp[0] = 0;
            strcat(StringTemp, BufferJoinSpace);
            for (; carets != 0; --carets) {
                strcat(StringTemp, CaretSeparator);
            }
            const unsigned int len = strlen(StringTemp);
            static_cast<Buffer *>(this)->write_cent_l(StringTemp, rect, len);
        }
    }
}



// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00479900 ?UNK2@MultiWin@@QAEXXZ 0x00479900-0x00479964 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00479900.cpp", "image:terranx_original.exe:0x00479900", "source:src/maininterface.h:InterfaceRegistry and mode-stack storage; source:src/multiwin.h:owned Sprite offsets; native callback-sensitive mode reload and downward repeated removal", "recovery/tasks/interface-contract-report-closure.json; owner expansion interface-registry-multiwin-small-20261001"], "reason": "Recover the original in its genuine C++ owner using source-owned interface registry and mode stack, real virtual callbacks, or owned MultiWin sprite geometry. Preserve historical seed obligations; this binding grants no current byte or runtime claim."}
void MultiWin::UNK2() {
    const int row_height = sprite5_.height();
    const int footer_height = sprite7_.height();
    const int header_height = sprite3_.height();
    int total = row_height * 7 + header_height + footer_height;
    if (IsMultiplayerNet != 0) {
        total += sprite4_.height() + footer_height + 0x0E;
    }
    field_A38_ = 0;
    field_A3C_ = 0;
    field_A40_ = 0x8A;
    field_A44_ = static_cast<unsigned int>(total);
    StatusFlags |= 0x40;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: MISMATCH #5, register-allocation-level divergence early in the function. Ghidra's param_2/param_3 initially looked swapped against a1/a2 (param_2 is actually a1, the player-slot index; param_3 is a2, the itoa'd number) - fixed via cross-check against the raw disassembly's ebp+8/ebp+0xc reads.
// ORIGINAL: 0x00478630 ?draw_player@MultiWin@@QAEXHHPAURECT@@@Z 0x00478630-0x00478BDB
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/00478630.cpp", "src/multiwin.h", "src/faction.h", "src/maininterface.h"], "reason": "The original row painter uses MultiWin's owned Spot, sprite array and first font, typed faction names and treaties, and shared chat colors. Network turn-state suffixes remain pending their global owners."}
// symbol    ?draw_player@MultiWin@@QAEXHHPAUtagRECT@@@Z
void MultiWin::draw_player(int faction_id, int row, RECT *bounds) {
  Buffer *canvas = static_cast<Buffer *>(this);
  canvas->set_font(&font1_, 0, 0, 0);

  const int row_height = bounds->bottom - bounds->top;
  const int icon_left = bounds->left + 7;
  const int text_left = icon_left + (IsMultiplayerNet ? row_height : 0) + 3;
  const int text_right = bounds->right - 10;
  const unsigned int faction_bit = 1u << faction_id;
  if (IsMultiplayerNet && (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_bit)) {
    spot_.add(faction_id, 0, icon_left, bounds->top, row_height, row_height);
    Sprite &icon = sprites2_[0];
    icon.draw(canvas, icon.transparent_index(), icon_left,
              bounds->top + (row_height - icon.height()) / 2, 1, 1);
  }
  spot_.add(faction_id, 1, text_left, bounds->top,
            text_right - text_left, row_height);
  canvas->set_text_color(ChatItemIdTable[faction_id],
                         ChatShadowColors[faction_id], 1, 1);

  StringTemp[0] = 0;
  char row_text[32];
  _itoa(row, row_text, 10);
  strcat(StringTemp, row_text);
  strcat(StringTemp, " ");
  GenderDefault = Players[faction_id].is_leader_female;
  PluralityDefault = 0;
  strcat(StringTemp, Players[faction_id].name_leader);
  RECT text_bounds = *bounds;
  text_bounds.left = text_left;
  text_bounds.right = text_right;
  canvas->write_l(StringTemp, &text_bounds, strlen(StringTemp));

  StringTemp[0] = 0;
  if (!IsMultiplayerNet) {
    const unsigned int treaties =
        PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[faction_id];
    int label_index = -1;
    if (treaties & 1) label_index = 0x34c / 4;
    else if (treaties & 2) label_index = 0x350 / 4;
    else if (treaties & 4) label_index = 0x354 / 4;
    else if (treaties & 0x10) label_index = 0x358 / 4;
    if (label_index >= 0) strcat(StringTemp, label_get(label_index));
  }
  // Pending: network control and countdown suffixes require the
  // unbound 0x0093E8xx turn state and its PlayerLock ownership.
  if (StringTemp[0]) {
    canvas->write_right_l(StringTemp, &text_bounds, strlen(StringTemp));
  }
}

// ORIGINAL: 0x00478BE0 ?draw_time@MultiWin@@QAEXPAURECT@@H@Z 0x00478BE0-0x00479269
// RECOVERY_BINDING: v1 {"evidence": ["historical rationale retained verbatim: Recover the clock region's owned hotspot, font and visible turn-state label; the countdown value and timer side effects await binding of the original shared time-control globals. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations.", "pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: draw_time performs the timer, council, multiplayer-label, warning, timeout, wave and FX paths over 0x00478BE0..0x00479269.", "source owners: existing NetDaemonWaitTicks, TurnRedrawPending, CouncilWindow fields +0xA24/+0xA3C/+0xA54, NetDaemonProgressInterval, NetDaemonNet lock-state words, Console, GameDataStateStorage, NetMsg and FX APIs.", "the 0x007FFF78 and 0x007FFF7C source objects retain separate original dword spans; no larger allocation or semantic-name claim."], "reason": "Recover the original clock, turn-control, countdown and timeout paths through existing typed state plus the shared NetDaemonProgressInterval and two original-span MultiWin state words. This updates the superseded scaffold rationale only; it grants no BYTE_EXACT or SEMANTIC claim, matching/restoration result, native build result or whole-project acceptance."}
// kind      game
// TRIED: transcribed from Ghidra's decompilation (which resolved the dataflow cleanly) with member calls fixed up (Spot::add, Buffer::set_font/set_text_color/write_cent_l, Strings::get, AlphaNet::who_2_pid, Net::get_player_name, NetMsg::pop, FX::play, message_data, wave_it) and the two `(x<0)?0:x` clamp bit-tricks written as plain clamps rather than neg/sbb/and. Compiles; diverges at instruction #3 (prologue - local frame size/shape, not chased further).
// symbol    ?draw_time@MultiWin@@QAEXPAUtagRECT@@H@Z
void MultiWin::draw_time(RECT *bounds, int draw_type) {
  Buffer *canvas = static_cast<Buffer *>(this);
  spot_.add(0, 2, bounds->left, bounds->top,
            bounds->right - bounds->left, bounds->bottom - bounds->top);
  canvas->set_font(&font2_, 0, 0, 0);

  // These locals are written only on the draw_type path in the original.
  // Keep the original uninitialized read behavior on the other path.
  int local_14;
  unsigned int local_10;
  int local_c;
  int minutes;
  unsigned int countdown_ms;
  char number[80];

  if (draw_type != 0) {
    ++MultiWinState7FFF7C;
    if (NetBusyCount == 0 &&
        NetDaemonNet.lock_state_[0].trailing_words_[4] == 0 &&
        NetDaemonProgressInterval == 0) {
      if (TurnRedrawPending == 0) {
        local_14 = 0;
        local_10 = NetDaemonWaitTicks > 0 ? 1u : 0u;
        union SignedDword {
          uint32_t bits;
          int32_t value;
        } after_tick;
        after_tick.bits = static_cast<uint32_t>(NetDaemonWaitTicks) - 500u;
        NetDaemonWaitTicks = after_tick.value < 0 ? 0 : after_tick.value;
      } else {
        local_14 = 0;
        const int council_time = static_cast<int32_t>(CouncilWindow.field_A3C_);
        local_10 = council_time > 0 ? 1u : 0u;
        union SignedDword {
          uint32_t bits;
          int32_t value;
        } after_tick;
        after_tick.bits = CouncilWindow.field_A3C_ - 500u;
        CouncilWindow.field_A3C_ = static_cast<uint32_t>(
            after_tick.value < 0 ? 0 : after_tick.value);
      }
    } else {
      local_14 = 1;
      if (static_cast<int32_t>(NetDaemonProgressInterval) > 0) {
        --NetDaemonProgressInterval;
      }
    }
  }

  int signed_time;
  if (TurnRedrawPending == 0) {
    signed_time = NetDaemonWaitTicks;
  } else {
    signed_time = static_cast<int32_t>(CouncilWindow.field_A3C_);
  }
  countdown_ms = signed_time < 0 ? 0u : static_cast<uint32_t>(signed_time);
  union RoundedTime {
    uint32_t bits;
    int32_t value;
  } rounded_seconds;
  rounded_seconds.bits = countdown_ms + 999u;
  const int total_seconds = rounded_seconds.value / 1000;
  minutes = total_seconds / 60;
  local_c = total_seconds % 60;

  if (TurnRedrawPending != 0) {
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x7e8 / 4));
    CharUpperA(StringTemp);
    strcat(StringTemp, ": ");
    _itoa(minutes, number, 10);
    strcat(StringTemp, number);
    strcat(StringTemp, ":");
    if (local_c < 10) strcat(StringTemp, "0");
    _itoa(local_c, number, 10);
    strcat(StringTemp, number);
    canvas->set_text_color(0xe4, -1, 1, 1);
    canvas->write_cent_l(StringTemp, bounds, strlen(StringTemp));
    if (local_10 == 0) {
      MultiWinState7FFF78 = local_c;
      return;
    }
    if (countdown_ms != 0) {
      MultiWinState7FFF78 = local_c;
      return;
    }
    if (CouncilWindow.field_A24_ != 0) {
      MultiWinState7FFF78 = local_c;
      return;
    }
    if (CouncilDialogActive == 0) {
      MultiWinState7FFF78 = local_c;
      return;
    }
    if (CouncilWindow.leader_label_kind_[ConsoleGlobal.faction_id_] != 0) {
      MultiWinState7FFF78 = local_c;
      return;
    }
    message_data(0x2601, 0, ConsoleGlobal.faction_id_, -3, 0, 0);
    MultiWinState7FFF78 = local_c;
    return;
  }

  int text_length;
  if (ConsoleControlTurnActive != 0) {
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x410 / 4));
    CharUpperA(StringTemp);
    strcat(StringTemp, " ");
    _itoa(NetUpkeepPhase, number, 10);
    strcat(StringTemp, number);
    text_length = static_cast<int>(strlen(StringTemp));
    goto write_clock_text;
  }

  if ((GameDataStateStorage.fields_.net_turn_flags_ & 0x10) != 0 &&
      (GameDataStateStorage.fields_.net_turn_faction_ !=
           ConsoleGlobal.faction_id_ || TurnControlWord93A954 != 0)) {
    const uint32_t faction_bit =
        1u << (GameDataStateStorage.fields_.net_turn_faction_ & 0x1f);
    if ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ &
         faction_bit) == 0) {
      MultiWinState7FFF78 = local_c;
      return;
    }
    StringTemp[0] = 0;
    const int player_id = NetDaemonNet.who_2_pid(
        GameDataStateStorage.fields_.net_turn_faction_);
    strcat(StringTemp, NetDaemonNet.get_player_name(player_id));
    canvas->set_text_color(0xe4, -1, 1, 1);
    text_length = static_cast<int>(strlen(StringTemp));
    goto write_clock_text;
  }

  if (draw_type != 0 &&
      NetDaemonNet.lock_state_[0].trailing_words_[1] != 0 &&
      NetDaemonNet.lock_state_[0].trailing_words_[4] == 0 &&
      GameDataStateStorage.fields_.time_controls_selection_ != 0) {
    const uint32_t local_faction_bit =
        1u << (ConsoleGlobal.faction_id_ & 0x1f);
    if (countdown_ms == 0 &&
        (ConsoleFactionsWaitingMask & local_faction_bit) == 0 &&
        MultiWinState7FFF78 != 0) {
      GameDataStateStorage.fields_.game_state_ |= 0x800;
      if (local_10 != 0) wave_it(0x21);
    }
    if (minutes == 0 && local_c != 0 && local_c < 0x10 &&
        local_c != MultiWinState7FFF78 && ConsoleControlTurnActive == 0 &&
        (GameDataStateStorage.fields_.game_state_ & 0x800) == 0 &&
        (ConsoleFactionsWaitingMask & local_faction_bit) == 0) {
      if (local_c == 0x0f) {
        NetMsgPrimary.pop("TIMEWARNING", -5000, 0, 0);
      }
      g_FX.play(0x23);
    }
  }

  StringTemp[0] = 0;
  strcat(StringTemp, label_get(0x40c / 4));
  strcat(StringTemp, ": ");
  if (NetDaemonNet.lock_state_[0].trailing_words_[4] == 0) {
    if (ConsoleControlTurnActive != 0) {
      StringTemp[0] = 0;
      strcat(StringTemp, label_get(0x988 / 4));
      strcat(StringTemp, ": ");
      strcat(StringTemp, label_get(0x410 / 4));
    } else if (GameDataStateStorage.fields_.time_controls_selection_ == 0) {
      StringTemp[0] = 0;
      strcat(StringTemp, label_get(0xa04 / 4));
    } else {
      _itoa(minutes, number, 10);
      strcat(StringTemp, number);
      strcat(StringTemp, ":");
      if (local_c < 10) strcat(StringTemp, "0");
      _itoa(local_c, number, 10);
      strcat(StringTemp, number);
    }
  } else {
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x988 / 4));
    strcat(StringTemp, ": ");
    strcat(StringTemp, label_get(0x444 / 4));
  }

  if (local_14 == 0 || (MultiWinState7FFF7C & 1u) == 0) {
    canvas->set_text_color(0xe4, -1, 1, 1);
  } else {
    canvas->set_text_color(0, -1, 1, 1);
  }
  text_length = static_cast<int>(strlen(StringTemp));

write_clock_text:
  canvas->write_cent_l(StringTemp, bounds, text_length);
  MultiWinState7FFF78 = local_c;
}

// ORIGINAL: 0x00479330 ?draw@MultiWin@@QAEXH@Z 0x00479330-0x004798EC
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00479330.cpp", "src/multiwin.h", "src/faction.h", "src/spot.h"], "reason": "Move the panel painter from its fixed-address seed onto MultiWin's owned sprites, Spot and GraphicWin buffer; the two separately mapped row painters remain explicit dependencies."}
// TRIED: raw self+offset field reads (MultiWin left opaque) with reinterpret_cast to Spot/GraphicWin/Buffer sub-objects at their disassembly-derived offsets; diverges at insn #11
void MultiWin::draw(int draw_type) {
  spot_.kill_type(2);
  spot_.kill_type(0);
  spot_.kill_type(1);
  GraphicWin::fill(9);

  Buffer *canvas = static_cast<Buffer *>(this);
  int y = (!IsMultiplayerNet || field_A24_)
              ? 0 : sprite5_.height() * 7 + sprite7_.height() * 2;
  sprite3_.draw(canvas, sprite3_.transparent_index(), 0, y);
  y += sprite3_.height();

  RECT rect;
  if (IsMultiplayerNet) {
    sprite4_.draw(canvas, sprite4_.transparent_index(), 0, y);
    rect.left = 0;
    rect.top = y;
    rect.right = dib_.bmiHeader.biWidth;
    rect.bottom = y + sprite4_.height();
    draw_time(&rect, draw_type);

    rect.top = rect.bottom;
    rect.bottom = rect.top + 14;
    y = rect.bottom;
    sprite5_.draw(canvas, sprite5_.transparent_index(), 0, rect.top);
    canvas->set_clip(&rect);
    spot_.add(3, 2, &rect);

    // The seed reads PlayerData +0x2054: the local faction's time bonus.
    const int time_bonus = PlayersData[ConsoleGlobal.faction_id_].time_bonus_count;
    if (time_bonus > 0) {
      int carets = time_bonus > 3 ? 3 : time_bonus;
      StringTemp[0] = 0;
      strcat(StringTemp, " ");
      while (carets-- > 0) strcat(StringTemp, "^");
      canvas->write_cent_l(StringTemp, &rect, strlen(StringTemp));
    }
    canvas->set_clip(&rect2_);
    y = val_2_;
  }

  const int saved_top = y;
  const int saved_bottom = y + sprite5_.height();
  const int saved_right = dib_.bmiHeader.biWidth;
  bool drew_player = false;
  if (!IsMultiplayerNet || field_A24_) {
    int row = 0;
    for (int faction = 1; faction < 8; ++faction) {
      if (faction == ConsoleGlobal.faction_id_) continue;
      const unsigned int mask = 1u << faction;
      if (!(GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (mask << 8))) continue;
      if (!(GameDataStateStorage.fields_.game_state_ & 0x80) &&
          !(PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[faction] & 8) &&
          !(GameDataStateStorage.fields_.secret_project_.empath_guild() >= 0 && Bases[GameDataStateStorage.fields_.secret_project_.empath_guild()].faction_id_current == ConsoleGlobal.faction_id_) &&
          !((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & mask) && auto_contact())) continue;

      ++row;
      drew_player = true;
      Sprite &background = hovered_spot_ == faction ? sprite6_ : sprite5_;
      background.draw(canvas, background.transparent_index(), 0, y);
      rect.left = 0;
      rect.top = y;
      rect.right = saved_right;
      rect.bottom = y + sprite5_.height();
      draw_player(faction, row, &rect);
      y += sprite5_.height();
    }
    for (; row < 7; ++row) {
      sprite5_.draw(canvas, sprite5_.transparent_index(), 0, y);
      y += sprite5_.height();
    }

    canvas->set_font(&font1_, 0, 0, 0);
    y += sprites1_[0].height() - sprite5_.height();
    sprite5_.draw(canvas, sprite5_.transparent_index(), 0, y);
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x80c / 4));
    strcat(StringTemp, ": ");
    strcat(StringTemp, " ");
    char votes_text[84];
    _itoa(council_votes(ConsoleGlobal.faction_id_), votes_text, 10);
    strcat(StringTemp, votes_text);
    canvas->set_text_color(0xff, -1, 1, 1);
    if (!(Players[ConsoleGlobal.faction_id_].rule_flags & 0x80)) {
      canvas->write_cent_l(StringTemp, 0, y + 10, 0x8a, strlen(StringTemp));
      y += sprite5_.height();
    }
    if (IsMultiplayerNet) {
      sprite7_.draw(canvas, sprite7_.transparent_index(), 0, y);
      y += sprite7_.height();
    }
    sprite7_.draw(canvas, sprite7_.transparent_index(), 0, y);
  }

  if (!drew_player) {
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x9d0 / 4));
    canvas->set_text_color(MessageWinTextColor, MessageWinTextBackground, 1, 1);
    rect.left = 0;
    rect.top = saved_top;
    rect.right = saved_right;
    rect.bottom = saved_bottom;
    canvas->set_font(find_font(12, 1), 0, 0, 0);
    canvas->write_cent_l(StringTemp, &rect, strlen(StringTemp));
  }
  if (!MultiWindowRedrawSuppressed) soft_update();
}

// The original show/hide pair publishes its visible state here. The zero
// initializer and both writers are retained in the pinned image and seeds.
// RECOVERY_OBJECT: v2 0x007CFF04 0x007CFF08 ?MultiWindowVisibilityState@@3HA 0x00479970
int MultiWindowVisibilityState;

// Shared animation frame interval (20 ms in the pinned image). Other window
// animations also read this original data object.
// RECOVERY_OBJECT: v2 0x00691AF8 0x00691AFC ?WindowAnimationFrameDelay@@3HA 0x00479C00
int WindowAnimationFrameDelay = 20;

// The original draw suppresses its soft update while show calls the redraw callback.
// The original show writes this dword on both sides of that callback;
// MultiWin::draw at 0x00479330 reads it before soft_update.
// RECOVERY_OBJECT: v2 0x007FFF70 0x007FFF74 ?MultiWindowRedrawSuppressed@@3HA 0x00479970
int MultiWindowRedrawSuppressed;

// NetDaemonProgressInterval is the shared 0x93F668 dword, declared and
// defined once by netdaemon. These two independent MultiWin latch/frame dwords
// are address-reflecting provisional identities, not semantic names or a
// larger allocation claim. Original PE initial data is four zero bytes at
// each address; the frame test uses only its low bit.
// RECOVERY_OBJECT: v2 0x007FFF78 0x007FFF7C ?MultiWinState7FFF78@@3HA 0x00478BE0
int MultiWinState7FFF78;
// RECOVERY_OBJECT: v2 0x007FFF7C 0x007FFF80 ?MultiWinState7FFF7C@@3IA 0x00478BE0
uint32_t MultiWinState7FFF7C;

// 0x0047A590 and 0x0047A430. The compiler owns construction, exception
// cleanup and reverse destruction of the subobjects declared in multiwin.h.
// Historical mapped seed units remain under src/recovered/units/.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0047A590 ??0MultiWin@@QAE@XZ 0x0047A590-0x0047A6B9;0x00656310-0x006563DE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0047a590.cpp; docs/recovery/subobject-sites.csv; src/multiwin.h"], "reason": "The original constructor builds a GraphicWin front and the owned Buffer, Sprite, FlatButton, Font and Spot subobjects in the observed order."}
MultiWin::MultiWin() {}
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0047A430 ??1MultiWin@@QAE@XZ 0x0047A430-0x0047A58A;0x00656230-0x0065630E
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0047a430.cpp; docs/recovery/subobject-sites.csv; src/multiwin.h"], "reason": "The original destructor tears down the Spot, Fonts, FlatButtons, Sprites, Buffer and GraphicWin front in reverse ownership order."}
// symbol    ??1MultiWin@@UAE@XZ
MultiWin::~MultiWin() {}

// 0x00477E70: Spot::check returns the hovered position and type through its
// two output arguments. Only type 1 changes the highlighted player.
// ORIGINAL: 0x00477E70 ?on_mouse_move@MultiWin@@QAEXHH@Z 0x00477E70-0x00477EDB BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/recovered/00477e70.cpp; docs/recovery/member-accesses.csv"], "reason": "The original checks the owned Spot for a player hover and redraws when the selected position changes."}
// TRIED: Historical exact-seed spelling: Spot::check used the addresses of both incoming stack parameters as its output arguments (lea of ebp+8/ebp+0xc), overwriting them with spot position/type; writing the result != -1 arm first produced the original je branch polarity. The typed source owner uses distinct output locals and a real virtual override; this observation preserves seed restoration evidence, not a current matching claim.
// symbol    ?on_mouse_move@MultiWin@@UAEXHH@Z
void MultiWin::on_mouse_move(int x, int y) {
  int position;
  int type;
  if (spot_.check(x, y, &position, &type) != -1) {
    if (type == 1 && hovered_spot_ != position) {
      hovered_spot_ = position;
      draw(0);
    }
  } else if (hovered_spot_ != -1) {
    hovered_spot_ = -1;
    draw(0);
  }
}

// 0x00477EE0 and 0x004798F0: Win callbacks selected by MultiWin's table.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00477EE0 ?on_mouse_leave@MultiWin@@QAEXHH@Z 0x00477EE0-0x00477EFE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/00477ee0.cpp; docs/recovery/member-accesses.csv"], "reason": "The original resets the hovered player position and redraws on mouse leave."}
// symbol    ?on_mouse_leave@MultiWin@@UAEXHH@Z
void MultiWin::on_mouse_leave(int, int) {
  if (hovered_spot_ >= 0) {
    hovered_spot_ = -1;
    draw(0);
  }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004798F0 ?on_redraw@MultiWin@@QAEXXZ 0x004798F0-0x004798F8 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/004798f0.cpp; src/win.h"], "reason": "The original Win redraw callback forwards the zero draw mode to MultiWin draw."}
// symbol    ?on_redraw@MultiWin@@UAEXXZ
void MultiWin::on_redraw() { draw(0); }

// 0x00477F00. The chat branch is restored; the council dialog remains an
// explicitly unfinished, separately mapped dependency.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00477F00 ?on_button_clicked@MultiWin@@QAEXH@Z 0x00477F00-0x00477F30 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/00477f00.cpp", "src/console.cpp", "src/council.cpp", "src/win.h"], "reason": "The original button callback branches on chat and council button IDs. The typed override now preserves both calls; the separately mapped council dialog remains explicitly unfinished."}
// symbol    ?on_button_clicked@MultiWin@@UAEXH@Z
void MultiWin::on_button_clicked(int button_id) {
  switch (button_id) {
    case 0: ConsoleGlobal.chat(0); break;
    case 1: call_council(ConsoleGlobal.faction_id_); break;
  }
}

// 0x00479970. The panel is redrawn into its owned buffer and optionally
// slides upward over the screen before Win publishes it in solo play.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00479970 ?show@MultiWin@@QAEXXZ 0x00479970-0x00479BF1;0x006563F5-0x0065640A
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00479970.cpp; src/multiwin.h; src/faction.h; src/console.h; src/wave.cpp"], "reason": "The original show uses MultiWin's owned controls and GraphicWin buffer, the local Console faction, Player rule flags, process window animation state, and the source-owned wave, screen, visibility and frame-delay objects."}
void MultiWin::show() {
  Buffer background;
  if (IsMultiplayerNet == 0 && field_A24_ != 0) return;

  hovered_spot_ = -1;
  // The seed sets each button's Win visibility bit directly. Use the real
  // control interface here; reconcile any callback difference with draw().
  if (ExpansionEnabled == 0 ||
      (Players[ConsoleGlobal.faction_id_].rule_flags & 0x80) == 0) {
    flat_button1_.show(1);
  }
  if (IsMultiplayerNet != 0) flat_button2_.show(1);

  if (GameDataStateStorage.fields_.game_preferences_ & 0x400) g_MULTIWIN_WAVE.play();
  MultiWindowVisibilityState = 2;
  field_A24_ = 1;
  MultiWindowRedrawSuppressed = 1;
  on_redraw();
  MultiWindowRedrawSuppressed = 0;

  const int width = dib_.bmiHeader.biWidth;
  const int height = -dib_.bmiHeader.biHeight;
  background.resize(width, height);
  background.fill(WinFillColour);
  update_window_to_buffer(&background);

  RECT screen_rect = rect2_;
  client_to_screen(&screen_rect);
  int position = screen_rect.bottom - sprite3_.height();
  if (IsMultiplayerNet != 0) position -= 14 + sprite4_.height();
  const int slide = (position - screen_rect.top) / 5;
  ScreenBuffer.set_clip(&screen_rect);

  if ((GameDataStateStorage.fields_.game_preferences_ & 0x8000000) != 0 && IsMultiplayerNet == 0) {
    WinFlipClipped = 1;
    for (int frame = 0; frame < 6; ++frame) {
      const DWORD start = timeGetTime();
      ScreenBuffer.draw(&background, 9, screen_rect.left, position, 1, 1);
      Win::flip(&screen_rect);
      position -= slide;
      do_sound();
      while (timeGetTime() - start < static_cast<DWORD>(WindowAnimationFrameDelay)) {
        do_sound();
      }
    }
    ScreenBuffer.set_clip(&WinScreenClipRect);
    WinFlipClipped = 0;
  }

  if (GameDataStateStorage.fields_.game_preferences_ & 0x400) g_MULTIWIN_WAVE.fade(50);
  if (IsMultiplayerNet == 0) Win::show(3);
}

// 0x00479C00. The Win slot-2 override hides the multi-player panel, restores
// its background and runs the original optional slide-out animation.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00479C00 ?hide@MultiWin@@QAEXXZ 0x00479C00-0x00479F8E;0x0065640A-0x0065641F
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00479c00.cpp; src/win.h; src/buffer.h; src/wave.cpp"], "reason": "The original panel teardown uses its real GraphicWin Buffer extent, owned animation buffer and sprites, typed console/screen globals, and Win/GraphicWin callbacks. The retained seed preserves the historical byte obligation until remeasurement."}
// symbol    ?hide@MultiWin@@UAEXXZ
void MultiWin::hide() {
  Buffer background;
  if (IsMultiplayerNet == 0 && field_A24_ == 0) return;

  if (GameDataStateStorage.fields_.game_preferences_ & 0x400) g_MULTIWIN_WAVE.play();
  hovered_spot_ = -1;
  MultiWindowVisibilityState = 0;

  const int width = dib_.bmiHeader.biWidth;
  const int height = -dib_.bmiHeader.biHeight;
  background.resize(width, height);
  background.fill(WinFillColour);
  update_window_to_buffer(&background);
  on_redraw();

  RECT screen_rect = rect2_;
  client_to_screen(&screen_rect);
  RECT console_rect = screen_rect;
  ConsoleGlobal.screen_to_client(&console_rect);

  buffer_.init(width, height, 0, 0);
  static_cast<Buffer *>(&ConsoleGlobal)->copy(
      &buffer_, console_rect.left, console_rect.top, 0, 0, width, height);

  int limit = screen_rect.bottom - sprite3_.height();
  if (IsMultiplayerNet != 0) limit -= 14 + sprite4_.height();
  const int slide = (limit - screen_rect.top) / 5;
  ScreenBuffer.set_clip(&screen_rect);

  if ((GameDataStateStorage.fields_.game_preferences_ & 0x8000000) != 0 && IsMultiplayerNet == 0) {
    WinFlipClipped = 1;
    int position = screen_rect.top;
    for (int frame = 0; frame < 6; ++frame) {
      const DWORD start = timeGetTime();
      buffer_.copy(&ScreenBuffer, 0, 0, screen_rect.left, screen_rect.top,
                   width, height);
      background.draw(&ScreenBuffer, 9, screen_rect.left, position, 1, 1);
      Win::flip(&screen_rect);
      position += slide;
      if (position >= limit) position = limit;
      do_sound();
      while (timeGetTime() - start < static_cast<DWORD>(WindowAnimationFrameDelay)) {
        do_sound();
      }
    }
    if (position < limit) {
      buffer_.copy(&ScreenBuffer, 0, 0, screen_rect.left, screen_rect.top,
                   width, height);
      static_cast<Buffer *>(this)->draw(
          &ScreenBuffer, 9, screen_rect.left, limit, 1, 1);
      Win::flip(&screen_rect);
    }
    ScreenBuffer.set_clip(&WinScreenClipRect);
    WinFlipClipped = 0;
  }

  if (GameDataStateStorage.fields_.game_preferences_ & 0x400) g_MULTIWIN_WAVE.fade(50);
  field_A24_ = 0;
  if (IsMultiplayerNet == 0) {
    Win::hide();
  } else {
    flat_button2_.hide();
    flat_button1_.hide();
    on_redraw();
    on_update();
  }
  buffer_.close();
}

// Separate contact/display faction bits; provisional descriptive name.
// Native click writers and535xxx/537xxx readers prove a DWORD, not its full lifecycle.
// RECOVERY_OBJECT: v2 0x007FFF74 0x007FFF78 ?MultiWindowContactStatusMask@@3HA 0x00477F30
int MultiWindowContactStatusMask;

// ORIGINAL: 0x00477F30 ?on_left_click@MultiWin@@QAEXHH@Z 0x00477F30-0x00478148
// RECOVERY_BINDING: v1 {"evidence": ["task:multiwin-contact-owner", "historical seed:src/recovered/units/00477f30.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:complete original mapped span and native call/branch order", "native:478500 subtracts item ID1; jump-table indices7..10 select items8renounce,9withdrawal,10contact,11chat; withdrawal arguments local then clicked", "source:existing typed subobjects, faction/player/parser/network and separate bounded contact-state storage; automatic PullDown/Buffer lifetime"], "reason": "Recover the complete original in its genuine MultiWin or diplomacy C++ owner, retaining identity, auxiliary spans and historical seed findings; no current match or runtime claim."}
// symbol    ?on_left_click@MultiWin@@UAEXHH@Z
void MultiWin::on_left_click(int x, int y) {
    ConsoleInfoWin.reset();
    ConsoleStatusWin.reset();

    int faction = -1;
    int kind = -1;
    if (spot_.check(x, y, &faction, &kind) < 0) return;

    if (!is_alive(static_cast<uint32_t>(faction))) {
        GenderDefault = Players[faction].is_leader_female;
        PluralityDefault = 0;
        parse_says(0, Players[faction].title_leader, -1, -1);
        GenderDefault = Players[faction].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[faction].name_leader, -1, -1);
        GenderDefault = Players[faction].noun_gender;
        PluralityDefault = Players[faction].is_noun_plural;
        parse_says(2, Players[faction].noun_faction, -1, -1);
        NetMsgPrimary.pop("ELIMINATED", 5000, 0, 0);
        return;
    }

    if (kind == 1) commlink_attempt(faction);
    if (!IsMultiplayerNet) return;

    if (kind == 2) {
        ConsoleInfoWin.reset();
        ConsoleStatusWin.reset();
        if (faction == 0) {
            ConsoleGlobal.time_controls();
            return;
        }
        if (faction == 3) {
            ConsoleGlobal.use_time_bonus();
            return;
        }
    } else if (kind == 0) {
        const uint32_t bit = 1u << faction;
        if (MultiWindowContactStatusMask & bit) {
            MultiWindowContactStatusMask &= ~bit;
            NetDaemonNet.lock_state_[2].trailing_words_[6] |= bit;
            draw(0);
            return;
        }
        if (NetDaemonNet.lock_state_[2].trailing_words_[6] & bit) {
            NetDaemonNet.lock_state_[2].trailing_words_[6] &= ~bit;
            draw(0);
            return;
        }
        MultiWindowContactStatusMask |= bit;
        draw(0);
    }
}

// ORIGINAL: 0x00478150 ?on_right_click@MultiWin@@QAEXHH@Z 0x00478150-0x004785FA;0x006563E0-0x006563F5
// RECOVERY_BINDING: v1 {"evidence": ["task:multiwin-contact-owner", "historical seed:src/recovered/units/00478150.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:complete original mapped span and native call/branch order", "native:478500 subtracts item ID1; jump-table indices7..10 select items8renounce,9withdrawal,10contact,11chat; withdrawal arguments local then clicked", "source:existing typed subobjects, faction/player/parser/network and separate bounded contact-state storage; automatic PullDown/Buffer lifetime"], "reason": "Recover the complete original in its genuine MultiWin or diplomacy C++ owner, retaining identity, auxiliary spans and historical seed findings; no current match or runtime claim."}
// symbol    ?on_right_click@MultiWin@@UAEXHH@Z
void MultiWin::on_right_click(int x, int y) {
    PullDown menu;
    int faction = -1;
    int kind = -1;
    if (spot_.check(x, y, &faction, &kind) < 0 || faction < 0 || kind != 1)
        return;

    Win::client_to_screen(&x, &y);
    menu.init(0);

    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x9F0 / 4));
    strcat(StringTemp, " ");
    strcat(StringTemp, Players[faction].title_leader);
    strcat(StringTemp, " ");
    strcat(StringTemp, Players[faction].name_leader);
    menu.add_item(1, StringTemp);
    menu.add_item(2, label_get(0x9F4 / 4));

    const unsigned int treaties =
        PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[faction];
    if (treaties & 7) {
        menu.add_separator(0x63);
        if (treaties & 1)
            menu.add_item(8, label_get(0xE20 / 4));
        else
            menu.add_item(9, label_get(0xE24 / 4));
    }

    menu.add_separator(0x63);
    menu.add_item(3, label_get(0x9F8 / 4));
    if (IsMultiplayerNet) {
        menu.add_separator(0x63);
        menu.add_item(0xA, label_get(0x9FC / 4));
        menu.add_item(0xB, label_get(0xA00 / 4));
    }

    menu.show(x, y, pop_expire);
    if (GameDataStateStorage.fields_.game_state_ & 0x800) return;

    const int selected = menu.get_selected();
    if (selected >= 0x40) return;
    const int action = menu.item_id(selected);
    if (action < 0) return;

    if (!is_alive(static_cast<uint32_t>(faction))) {
        GenderDefault = Players[faction].is_leader_female;
        PluralityDefault = 0;
        parse_says(0, Players[faction].title_leader, -1, -1);
        GenderDefault = Players[faction].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[faction].name_leader, -1, -1);
        GenderDefault = Players[faction].noun_gender;
        PluralityDefault = Players[faction].is_noun_plural;
        parse_says(2, Players[faction].noun_faction, -1, -1);
        NetMsgPrimary.pop("ELIMINATED", 5000, 0, 0);
        return;
    }

    switch (action) {
    case 1:
        commlink_attempt(faction);
        break;
    case 2:
        report_intel(faction);
        break;
    case 3:
        call_council(ConsoleGlobal.faction_id_);
        break;
    case 8:
        renounce_pact(ConsoleGlobal.faction_id_, faction);
        break;
    case 9:
        demands_withdrawal(ConsoleGlobal.faction_id_, faction);
        break;
    case 10: {
        const unsigned int bit = 1u << faction;
        if (MultiWindowContactStatusMask & bit) {
            MultiWindowContactStatusMask &= ~bit;
            NetDaemonNet.lock_state_[2].trailing_words_[6] |= bit;
            draw(0);
        } else if (NetDaemonNet.lock_state_[2].trailing_words_[6] & bit) {
            NetDaemonNet.lock_state_[2].trailing_words_[6] &= ~bit;
            draw(0);
        } else {
            MultiWindowContactStatusMask |= bit;
            draw(0);
        }
        break;
    }
    case 11:
        ConsoleGlobal.chat(faction);
        break;
    default:
        break;
    }
}

// ORIGINAL: 0x00479F90 ?init@MultiWin@@QAEXXZ 0x00479F90-0x0047A429;0x0065641F-0x00656434
// RECOVERY_BINDING: v1 {"evidence": ["task:multiwin-contact-owner", "historical seed:src/recovered/units/00479f90.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:complete original mapped span and native call/branch order", "native:478500 subtracts item ID1; jump-table indices7..10 select items8renounce,9withdrawal,10contact,11chat; withdrawal arguments local then clicked", "source:existing typed subobjects, faction/player/parser/network and separate bounded contact-state storage; automatic PullDown/Buffer lifetime"], "reason": "Recover the complete original in its genuine MultiWin or diplomacy C++ owner, retaining identity, auxiliary spans and historical seed findings; no current match or runtime claim."}
void MultiWin::init() {
    Buffer bitmap;
    field_A4C_ = 0;
    field_A50_ = 0;
    field_A48_ = 0;

    font1_.init(DefaultFontFace, 10, 0);
    font2_.init(DefaultFontFace, 14, 1);
    spot_.init(16);
    bitmap.load_pcx("multi.pcx", 0, 10, 0xEC);

    sprite3_.extract(&bitmap, WinFillColour, 1, 1, 0x8A, 6, 0);
    sprite4_.extract(&bitmap, WinFillColour, 1, 8, 0x8A, 0x1C, 0);
    sprite5_.extract(&bitmap, WinFillColour, 1, 0x25, 0x8A, 0x12, 0);
    sprite7_.extract(&bitmap, WinFillColour, 1, 0x38, 0x8A, 0x18, 0);
    sprite6_.extract(&bitmap, WinFillColour, 1, 0x51, 0x8A, 0x12, 0);

    for (int i = 0; i < 3; ++i) {
        const int y = 1 + i * 0x0F + (i == 1 ? 0x1E : 0);
        sprites2_[i].extract(&bitmap, WinFillColour, 0x106, y, 0x0E, 0x0E, 0);
    }
    for (int sprite_index = 0; sprite_index < 3; ++sprite_index) {
        sprites1_[sprite_index].extract(&bitmap, WinFillColour, 0x8C,
                            1 + sprite_index * 0x13, 0x79, 0x12, 0);
    }

    int height = sprite5_.height() * 7 + sprite7_.height() + sprite3_.height();
    if (IsMultiplayerNet) {
        height += 0x0E + sprite7_.height() + sprite4_.height();
    }
    RECT bounds = {0, 0, 0x8A, height};
    StatusFlags |= 0x40;
    GraphicWin::init(&bounds, 0, 0x01000000, &MainInterfaceGlobal, 0, 0);
    GraphicWin::fill(9);

    const int button_height = sprites1_[0].height();
    int top = height - (sprite7_.height() + button_height) / 2;
    if (IsMultiplayerNet) {
        flat_button2_.init(label_get(0x110), 0, 10, top,
                           sprites1_[0].width(), button_height,
                           static_cast<Win *>(this), 0);
        flat_button2_.vslot_59();
        flat_button2_.set_report_icons(&sprites1_[0], &sprites1_[1], &sprites1_[2]);
        flat_button2_.set_slice_colours(0x65, 0x41, 0x41);
        flat_button2_.redraw();
        top -= sprite7_.height();
    }
    top -= sprite5_.height();

    const int faction = ConsoleGlobal.faction_id_;
    if (!ExpansionEnabled || !(Players[faction].rule_flags & RFLAG_ALIEN)) {
        flat_button1_.init(label_get(0x1FA), 1, 10, top,
                           sprites1_[0].width(), button_height,
                           static_cast<Win *>(this), 0);
        flat_button1_.vslot_59();
        flat_button1_.set_report_icons(&sprites1_[0], &sprites1_[1], &sprites1_[2]);
        flat_button1_.set_slice_colours(0x65, 0x41, 0x41);
        flat_button1_.redraw();
    }

    int move_x;
    int move_y;
    if (WinScreenWidth == 800) {
        move_y = dib_.bmiHeader.biHeight + 0x166;
        move_x = 800 - dib_.bmiHeader.biWidth;
    } else {
        move_y = dib_.bmiHeader.biHeight - 0xFF + WinScreenHeight;
        move_x = (WinScreenWidth + 0x400) / 2 - dib_.bmiHeader.biWidth;
    }
    Win::move(move_x, move_y);
    field_A24_ = 0;
    if (IsMultiplayerNet) Win::show(0);

    g_MULTIWIN_WAVE.init(const_cast<char *>("fx\\mmenu.wav"), 4);
    g_MULTIWIN_WAVE.set_pan(0x20);
    g_MULTIWIN_WAVE.set_volume(0x28);
}
