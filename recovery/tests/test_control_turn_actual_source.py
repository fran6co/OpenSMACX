"""Host controls extracting and compiling the real game.cpp control_turn body.

Mocks supply dependency boundaries only. This is neither a native ABI check nor
game-runtime acceptance.
"""
from pathlib import Path
import os
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/game.cpp"

FIXTURE = r'''
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <new>
#define __cdecl
typedef int BOOL;
enum { MaxPlayerNum = 8 };
struct GameDataLockMaskView { uint32_t enable_mask_; };
struct GameDataStateWindowFields {
    int base_current_count_, diff_level_current_, turn_current_num_;
    int vehicle_current_count_, net_turn_faction_, governor_faction_;
    uint32_t game_rules_, game_more_preferences_, game_state_, net_turn_flags_;
    GameDataLockMaskView lock_masks_;
};
struct GameDataStateWindow { GameDataStateWindowFields fields_; };
GameDataStateWindow GameDataStateStorage;
struct RECT { long left, top, right, bottom; };
struct Buffer { int draws; void box_sprite(RECT *, int *) { ++draws; } };
struct GraphicWin : Buffer { void update(GraphicWin *) {} };
struct Win { int shows, hides; void hide() { ++hides; } void show(int) { ++shows; } };
struct StringBox : Win { int clears; void clear() { ++clears; } };
struct ButtonGroup { int field_84_; int sets; void set(int, int id) { field_84_ = id; ++sets; } };
struct MainInterface : GraphicWin {
    RECT status_left_rect_, status_right_rect_, status_hit_rect_;
    RECT tutorial_bounds_, message_bounds_, date_rect_;
    StringBox string_box_;
    ButtonGroup groups_[1];
    ButtonGroup &button_group(int i) { return groups_[i]; }
};
struct ListBox : Win {};
struct MessageWin : Win { ListBox listBox_; int clears; void clear() { ++clears; } };
struct AlphaMenu { int hides; void hide() { ++hides; } };
struct Console : Win {
    int faction_id_, center_calls, cursor_calls, update_calls, human_calls;
    AlphaMenu alpha_menu_;
    void human_turn();
    void set_center(int, int, int) { ++center_calls; }
    void set_cursor(int, int) { ++cursor_calls; }
    void update_data(int) { ++update_calls; }
};
struct SubInterface { int releases; void release_iface_mode() { ++releases; } };
struct BattleWin : SubInterface { int stops; void stop_timer() { ++stops; } };
struct BaseWindow { int exits; void exit() { ++exits; } };
struct MultiWin : Win {};
struct WorldWin : GraphicWin {
    int hides, shows, redraws;
    void hide_all() { ++hides; }
    void show_all() { ++shows; }
    void redraw() { ++redraws; }
};
struct CoverWin : Win { int fills; void fill(int) { ++fills; } };
struct StatusWin { int redraws; void on_redraw() { ++redraws; } };
struct InfoWin {
    int timer_suppression, set_calls;
    void set_timer_suppression(int value) {
        timer_suppression = value; ++set_calls;
    }
};
InfoWin ConsoleInfoWin;
int popup_heap_shutdowns, popup_scroll_closes, popup_closes;
int PopupInputState;
struct Heap {
    void shutdown() { ++popup_heap_shutdowns; }
    // The native BasePop member-cleanup edge is Heap::shutdown (0x5D45B0).
    // Model its observable idempotent cleanup effect at the host-language
    // member destruction boundary; this fixture does not assert the target
    // symbol identity or emitted call spelling.
    ~Heap() { shutdown(); }
};
struct FlatButton { ~FlatButton() {} };
struct Scroll : GraphicWin {
    FlatButton flat_button_left_, flat_button_right_;
    void close() { ++popup_scroll_closes; }
    ~Scroll() { close(); }
};
struct Sprite { ~Sprite() {} };
struct StringList { ~StringList() {} };
struct Dialogs { ~Dialogs() {} };
struct Spot { ~Spot() {} };
struct BasePop : GraphicWin {
    Heap heap_;
    FlatButton flat_button1_, flat_button2_;
    Sprite sprite_;
    StringList item_ids_, unit_ids_;
    Dialogs dialogs_;
    Spot spot_;
    void close() {}
};
struct Popup : BasePop {
    Scroll scroll_; int input_state;
    Popup() : input_state(PopupInputState) {}
    ~Popup() { close(); }
    void close() { ++popup_closes; scroll_.close(); BasePop::close(); }
    void start(char *, const char *, int, char *, int, GraphicWin *) {}
    int exec(int, int) { return 0; }
    uint32_t input_poll_state() const { return static_cast<uint32_t>(input_state); }
};
struct Base { int faction_id_current; int x, y; };
struct Veh { int faction_id, x, y; };
struct Player {
    int is_leader_female;
    char title_leader[32], name_leader[32], noun_faction[32];
    int noun_gender, is_noun_plural;
};
struct PlayerData { int unk_102, earned_techs_saved; };
struct OfferResetCell {
    int count_[2], state_[2], type_[2][8], value_[2][8];
};
struct RulesProposal { char *name; };

MainInterface MainInterfaceGlobal;
MessageWin InterfaceMessageWin;
Console ConsoleGlobal;
BattleWin g_BattleWin;
BaseWindow TutWinBaseWindow;
MultiWin MultiWindow;
WorldWin WorldClimateWorldWin;
CoverWin CoverWindow;
StatusWin ConsoleStatusWin;
int InterfaceBoxStyles[22];
int IfaceIdToSlotStorage[30], ActiveIfaceIdStorage;
int UnkBitfield1;
uint32_t FactionDiplomacyFlags[8];
OfferResetCell OfferResetCells[8][8];
char ControlTurnMailTemplate[1];
char ControlTurnPasswordInitialText[1];
uint32_t PendingDiplomacyFlags[8][8];
Player Players[8];
PlayerData PlayersData[8];
RulesProposal Proposal[11];
Base Bases[16]; Veh Vehs[16];
int ControlTurnPhase, IsMultiplayerPBEM;
int ExitTurnLoop, ConsoleControlTurnActive;
int MapBaseDisplayOverride, GameRestartQueued, IsMultiplayerNet;
int FactionTurnCombatWins[8], FactionTurnCombatLosses[8];
int TamperDialogFaction[8], TamperDialogValue[8];
int CouncilPbemGate, CouncilNotifyPluralSlots[8], CouncilNotifyFormSlots[8];
int GovernorFaction;
int GenderDefault, PluralityDefault;
int UnkState;
int ScriptPopupFileStorage;
char *ScriptPopupFile = (char *)&ScriptPopupFileStorage;
char PopupStartCaption[] = "caption";
struct ParsedString { char str[64]; };
ParsedString ParseStrBuffer[1];
int endgame_initial, endgame_later, turn_upkeep_calls, faction_upkeep_calls;
int display_calls, communicate_calls, selected_center, checksum_value;
int parse_calls, parsed_ids[16];
int stop_timer_calls, alpha_hide_calls, show_message_calls, player_message_result;
int popup_result, save_result = 1, human_exit_after = 1;
int timer_starts, map_draws, all_draws, tech_calls, enemy_calls;
int x_pop_ask_overload;
int x_pop_ask_initial_limit;

void Console::human_turn() { ++human_calls; if (human_exit_after) ExitTurnLoop = 1; }
int __cdecl save_game(int) { return save_result; }
int __cdecl display_player_messages(int) { ++display_calls; return player_message_result; }
void __cdecl mon_winning_unify(int, int) {}
void __cdecl communicate(int, int, int) { ++communicate_calls; }
void __cdecl diplo(int, int) {}
void __cdecl council(int, int, int) {}
void __cdecl draw_map(int) { ++map_draws; }
void __cdecl turn_upkeep() { ++turn_upkeep_calls; }
void __cdecl faction_upkeep(int) { ++faction_upkeep_calls; }
int __cdecl end_of_game(int check_victory) {
    if (check_victory != 0 && endgame_initial > 0) {
        --endgame_initial; return 1;
    }
    if (check_victory == 0 && endgame_later > 0) {
        --endgame_later; return 1;
    }
    return 0;
}
void __cdecl crash_landing(int) {}
int __cdecl parse_says(int id, char *, int, int) {
    parsed_ids[parse_calls++] = id; return 0;
}
int __cdecl parse_say(int, int, int, int) { return 0; }
int __cdecl parse_num(int, int) { return 0; }
int __cdecl checksum_password(char *) { return checksum_value; }
typedef int (__cdecl *PopupCallback)();
int __cdecl X_pop_ask(char *, const char *, int, char *, PopupCallback, int) {
    x_pop_ask_overload = 1; return 0;
}
int __cdecl X_pop_ask(char *, const char *, int, PopupCallback, int) {
    x_pop_ask_overload = 2; return 0;
}
int __cdecl X_pop_ask(char *, const char *, char *, PopupCallback, int) {
    x_pop_ask_overload = 3; return 0;
}
int __cdecl X_pop_ask(const char *, int value, char *, PopupCallback, int) {
    x_pop_ask_overload = 4;
    x_pop_ask_initial_limit = value;
    return 0;
}
int __cdecl X_pop_ask(const char *, int, PopupCallback, int) {
    x_pop_ask_overload = 5; return 0;
}
int __cdecl X_pop(const char *, int) { return 0; }
int __cdecl popp(char *, const char *, int, const char *, int) { return 0; }
void __cdecl stop_timers() { timer_starts = 0; }
void __cdecl start_timers() { ++timer_starts; }
void __cdecl do_all_draws() { ++all_draws; }
void __cdecl enemy_turn(int) { ++enemy_calls; }
int __cdecl bit_count(unsigned int value) {
    int count = 0; while (value) { count += value & 1u; value >>= 1; } return count;
}
void __cdecl tech_advance(int) { ++tech_calls; }
void __cdecl clear_council_notify(int) {}
void __cdecl rankings(int) {}
void __cdecl set_current_faction(int faction) { ConsoleGlobal.faction_id_ = faction; }
int __cdecl pending_unused();
unsigned int &pending_diplomacy_flag(int i) { return PendingDiplomacyFlags[i / 8][i % 8]; }
static int &control_turn_relation_first_word(int i) {
    OfferResetCell &cell = OfferResetCells[0][i / 4];
    switch (i % 4) {
    case 0: return cell.state_[0];
    case 1: return cell.type_[0][7];
    case 2: return cell.value_[0][0];
    default: return cell.value_[1][1];
    }
}
int __cdecl unused_symbol_anchor() { return 0; }

/* ACTUAL_CONTROL_TURN_BODY */

int main() {
    // Empty active mask still executes upkeep and the ordinary common tail.
    ExitTurnLoop = 1;
    endgame_initial = endgame_later = 0;
    control_turn();
    if (turn_upkeep_calls != 1 || ConsoleGlobal.human_calls != 1 ||
        popup_heap_shutdowns != 1) return 1;

    // The early upkeep victory takes the direct Popup destructor route; its
    // BasePop member teardown still performs the native Heap cleanup effect.
    ConsoleGlobal.human_calls = 0;
    endgame_initial = 1;
    int prior_shutdowns = popup_heap_shutdowns;
    control_turn();
    if (popup_closes < 1 || popup_scroll_closes < 1 ||
        popup_heap_shutdowns != prior_shutdowns + 1) return 2;
    prior_shutdowns = popup_heap_shutdowns;

    // One reported faction exercises the actual mail/password/UI/faction
    // path. The fixture ends the loop at the ordinary human_turn boundary.
    ExitTurnLoop = 0; human_exit_after = 1;
    endgame_initial = endgame_later = 0;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0x0101;
    ConsoleGlobal.faction_id_ = 1;
    PlayersData[0].unk_102 = 0;
    checksum_value = 123;
    player_message_result = 1;
    GameDataStateStorage.fields_.base_current_count_ = 1; Bases[0].faction_id_current = 7;
    GameDataStateStorage.fields_.vehicle_current_count_ = 1; Vehs[0].faction_id = 0;
    int previous_mail_parse = display_calls;
    int previous_center_calls = ConsoleGlobal.center_calls;
    control_turn();
    if (display_calls != previous_mail_parse + 1 ||
        ConsoleGlobal.faction_id_ != 0 || timer_starts != 1 ||
        popup_heap_shutdowns != prior_shutdowns + 1 ||
        ConsoleGlobal.center_calls != previous_center_calls + 1 ||
        parse_calls != 4 || parsed_ids[0] != 0 || parsed_ids[1] != 1 ||
        parsed_ids[2] != 2 || parsed_ids[3] != 3 ||
        MainInterfaceGlobal.draws != 11 || x_pop_ask_overload != 4 ||
        x_pop_ask_initial_limit != 0x50 ||
        ConsoleInfoWin.timer_suppression != 0 ||
        ConsoleInfoWin.set_calls != 2) return 3;

    // Pinned retry-limit exit writes the existing distinct globals.
    ExitTurnLoop = 0; human_exit_after = 1;
    ControlTurnPhase = 0; IsMultiplayerPBEM = 1; GameDataStateStorage.fields_.net_turn_flags_ = 0;
    GameDataStateStorage.fields_.turn_current_num_ = 1; GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0x0101;
    ConsoleGlobal.faction_id_ = 1;
    PlayersData[0].unk_102 = 7;
    checksum_value = 123; GameRestartQueued = 1;
    control_turn();
    if (ExitTurnLoop != 1 || GameRestartQueued != 0) return 6;

    // A nonzero end_of_game(0) after faction upkeep takes the retained
    // function's ordinary Popup scope-exit chain rather than continuing into
    // the next human-turn dispatch. This exercises the formerly omitted
    // 0x52821A -> 0x5285A4 native edge at the source-body boundary.
    ExitTurnLoop = 0; human_exit_after = 1;
    ControlTurnPhase = 0; IsMultiplayerPBEM = 1; GameDataStateStorage.fields_.net_turn_flags_ = 0x40;
    GameDataStateStorage.fields_.turn_current_num_ = 1; GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0x0101;
    ConsoleGlobal.faction_id_ = 1;
    PlayersData[0].unk_102 = 0; PopupInputState = 0;
    checksum_value = 321; endgame_initial = 0; endgame_later = 1;
    int previous_upkeep = faction_upkeep_calls;
    int previous_shutdowns = popup_heap_shutdowns;
    int previous_human_calls = ConsoleGlobal.human_calls;
    control_turn();
    if (faction_upkeep_calls != previous_upkeep + 1 ||
        popup_heap_shutdowns != previous_shutdowns + 1 ||
        ConsoleGlobal.human_calls != previous_human_calls ||
        endgame_later != 0) return 4;
    return 0;
}
'''


def extract_braced(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated function: {signature}")


def compile_and_run(body: str):
    compiler = os.environ.get("CXX", "g++")
    source = FIXTURE.replace("/* ACTUAL_CONTROL_TURN_BODY */", body, 1)
    with tempfile.TemporaryDirectory(prefix="osmx-control-turn-host-") as directory:
        path = Path(directory) / "control.cpp"
        executable = Path(directory) / "control"
        path.write_text(source)
        built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra",
                                str(path), "-o", str(executable)],
                               text=True, capture_output=True)
        assert built.returncode == 0, built.stdout + built.stderr
        return subprocess.run([str(executable)], text=True, capture_output=True)


def test_actual_control_turn_source_and_vehicle_fallback_mutant():
    source = SOURCE.read_text()
    body = extract_braced(source, "void __cdecl control_turn()")
    result = compile_and_run(body)
    assert result.returncode == 0, result.stdout + result.stderr

    mutant = body.replace("if (!found_owned_base && GameDataStateStorage.fields_.vehicle_current_count_ > 0)",
                          "if (found_owned_base && GameDataStateStorage.fields_.vehicle_current_count_ > 0)", 1)
    assert mutant != body
    result = compile_and_run(mutant)
    assert result.returncode != 0, "vehicle-focus fallback mutant unexpectedly passed"


def test_control_turn_source_identity_and_canonical_owner():
    from match_recovery.source import read_file

    owner = read_file(SOURCE)
    rows = [row for row in owner if row.address == 0x005275B0]
    assert len(rows) == 1
    record = rows[0]
    assert record.name == "?control_turn@@YAXXZ"
    assert record.image_spans == ((0x005275B0, 0x00528E94),
                                  (0x0065D6DD, 0x0065DB67))
    assert not record.byte_exact and not record.semantic
    assert record.ruled_out == (
        "mismatch #11 near prologue (xor vs push) - popup/end_game_pop declared as real C++ locals rather than the original's staggered same-slot vtable-poke construction; the \"big dialog\" tail (Dialogs/Spot/ListBox/CheckBox/EditGroup/ SpriteBox/StringStruct x2/Sprite) is reached by raw offset casts into BasePop's own dialogs_/spot_/sprite_ members, not by a modelled derived class - faithful call order/args, not byte fidelity. Main loop, faction dispatch, diplomacy/ tech/council logic transcribed directly from the disasm.",
    )
    source = SOURCE.read_text()
    body = extract_braced(source, "void __cdecl control_turn()")
    assert "Popup popup;" in body
    assert "popup.heap_.shutdown()" not in body
    assert "ControlTurnPopupLifetime" not in source
    header = (ROOT / "src/game.h").read_text()
    assert header.count("void __cdecl control_turn();") == 1
