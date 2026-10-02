#include "stdafx.h"
// ORIGINAL: 0x0045F710 ??_GInterlude@@UAEPAXI@Z 0x0045F710-0x0045F731 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original instruction/call evidence retained from src/deleting_thunks.cpp", "source:src/interlude.h: real GraphicWin base, embedded StringList and Buffer, compiler-generated deleting/adjustor lifecycle"], "reason": "Original member/callback/lifetime identity is owned by genuine Interlude C++ hierarchy; actual emitted symbol replaces the retired raw receiver shim, preserving historical spans and claims."}
// symbol    ??_GInterlude@@UAEPAXI@Z
// ORIGINAL: 0x0045F910 ??_GInterlude@@WEEE@AEPAXI@Z 0x0045F910-0x0045F91B FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original instruction/call evidence retained from src/deleting_thunks.cpp", "source:src/interlude.h: real GraphicWin base, embedded StringList and Buffer, compiler-generated deleting/adjustor lifecycle"], "reason": "Original member/callback/lifetime identity is owned by genuine Interlude C++ hierarchy; actual emitted symbol replaces the retired raw receiver shim, preserving historical spans and claims."}
// Original adjustor subtracts 0x444 (Buffer subobject) and jumps to 0x0045F710; catalogue ??3 was corrected to ??_G, while this compiler emits ??_E.
// symbol    ??_EInterlude@@WEEE@AEPAXI@Z
#include "interlude.h"
#include "messagewin.h"
#include "console.h"
#include "faction.h"
#include "game.h"
#include "base.h"
#include "alpha.h"
#include "strings.h"
#include "temp.h"
#include "xpops.h"
#include "font.h"

// Shared storage closure belongs to the coordinator's original owners.
// The pointer cell selects this observed NUL-terminated text-file identifier.
// The allocation extent of the pointed-to original string is unresolved.
// RECOVERY_OBJECT: v1 0x00691B70 _InterludeTextName
static char InterludeTextName[] = "INTERLUDEX";
// RECOVERY_OBJECT: v1 0x00691B08 ?InterludeTextFile@@3PADA
char *InterludeTextFile = InterludeTextName;
int __cdecl x_pop_caption_label(char *caption, const char *label,
                                int (__cdecl *callback)());

// ORIGINAL: 0x0045F200 ??0Interlude@@QAE@XZ 0x0045F200-0x0045F2DF;0x00655800-0x00655820 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/0045f200.cpp: original identity, complete spans and retained seed history", "source:src/interlude.h: genuine GraphicWin base and embedded member lifetime", "source:recovery/tasks/interlude-lifetime-owner-closure.json: bounded coordinator authorization"], "reason": "Original construction45F200 and its cleanup span establish real GraphicWin base plus RECT A14, StringList A24 and Buffer A54; natural member construction replaces the opaque hypothesis layout."}
Interlude::Interlude() {}
// ORIGINAL: 0x0045F740 ??1Interlude@@QAE@XZ 0x0045F740-0x0045F905;0x00655820-0x00655859
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0045f740.cpp: original identity, complete spans and retained seed history", "source:src/interlude.h: genuine GraphicWin base and embedded member lifetime", "source:recovery/tasks/interlude-lifetime-owner-closure.json: bounded coordinator authorization"], "reason": "Original destructor45F740 and cold cleanup establish real member/base teardown; declare genuine virtual lifecycle so compiler owns deleting destructors and receiver adjustments."}
// symbol    ??1Interlude@@UAE@XZ
Interlude::~Interlude() { close(); }

// ORIGINAL: 0x0045F640 ?close@Interlude@@QAEXXZ 0x0045F640-0x0045F6E5 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original instruction/call evidence retained from src/stringstruct.cpp", "source:src/interlude.h: real GraphicWin base, embedded StringList and Buffer, compiler-generated deleting/adjustor lifecycle", "Original45F640..45F6E5; retired seed git:117282ee:src/unrecovered/0045f640.cpp records the same identity and spans with its shim TRIED note kept below.", "The GraphicWin close chain is graphicwin.h's own close on the unadjusted receiver (the single-inheritance hypothesis layout), the caption list is the +0xA24 StringList block with head/next/count at +0xA2C/+0xA30/+0xA34, and the dialogue Buffer at +0xA54 closes through buffer.h's own close.", "The list's slot-1 release and each payload's deleting destructor run through member-fn-pointer unions because their classes are unnamed; the adjusted-receiver dispatch is the seed's vtbl+4 walk.", "Prior interpretation: Coverage transcription of the intermission teardown with the list drain kept at the seed's shape."], "reason": "Original member/callback/lifetime identity is owned by genuine Interlude C++ hierarchy; actual emitted symbol replaces the retired raw receiver shim, preserving historical spans and claims."}
// symbol    ?close@Interlude@@QAEXXZ
void Interlude::close() {
    GraphicWin::close();
    captions_.remove_all();
    captions_.current_position_ = 0;
    art_.close();
}

// ORIGINAL: 0x0045F6F0 ?on_key_down@Interlude@@QAEXH@Z 0x0045F6F0-0x0045F700 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original instruction/call evidence retained from src/delegation_thunks.cpp", "source:src/interlude.h: real GraphicWin base, embedded StringList and Buffer, compiler-generated deleting/adjustor lifecycle"], "reason": "Original member/callback/lifetime identity is owned by genuine Interlude C++ hierarchy; actual emitted symbol replaces the retired raw receiver shim, preserving historical spans and claims."}
// symbol    ?vslot_26@Interlude@@UAEHH@Z
int Interlude::vslot_26(int) {
    release_modal();
    return 1;
}

// ORIGINAL: 0x0045F700 ?on_left_down@Interlude@@QAEXHH@Z 0x0045F700-0x0045F70B FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original instruction/call evidence retained from src/delegation_thunks.cpp", "source:src/interlude.h: real GraphicWin base, embedded StringList and Buffer, compiler-generated deleting/adjustor lifecycle"], "reason": "Original member/callback/lifetime identity is owned by genuine Interlude C++ hierarchy; actual emitted symbol replaces the retired raw receiver shim, preserving historical spans and claims."}
// symbol    ?on_left_down@Interlude@@UAEXHH@Z
void Interlude::on_left_down(int, int) { release_modal(); }

// ORIGINAL: 0x0045F5C0 ?on_redraw@Interlude@@QAEHXZ 0x0045F5C0-0x0045F63E FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original instruction/call evidence retained from src/stringstruct.cpp", "source:src/interlude.h: real GraphicWin base, embedded StringList and Buffer, compiler-generated deleting/adjustor lifecycle", "Original45F5C0..45F63E; retired seed git:117282ee:src/unrecovered/0045f5c0.cpp records the same identity and spans.", "The receivers are the +0xA54 dialogue Buffer whose seven-int copy overload is buffer.h's own 0x005DFF00 blitter and the +0xA24 StringList write_strings draw; the frame numbers are read at +0x4C4/+0x4C8 and +0xAD4/+0xAD8 exactly as the seed measured them.", "Prior interpretation: Coverage transcription of the intermission repaint with the receiver spelled explicitly because Interlude has no header declaration yet."], "reason": "Original member/callback/lifetime identity is owned by genuine Interlude C++ hierarchy; actual emitted symbol replaces the retired raw receiver shim, preserving historical spans and claims."}
// symbol    ?on_redraw@Interlude@@UAEXXZ
void Interlude::on_redraw() {
    Buffer *canvas = static_cast<Buffer *>(this);
    const int width = art_.dib_.bmiHeader.biWidth;
    const int height = art_.dib_.bmiHeader.biHeight;
    art_.copy(canvas, 0, 0, (canvas->dib_.bmiHeader.biWidth - width) / 2,
              (height - canvas->dib_.bmiHeader.biHeight) / 2, width, -height);
    canvas->write_strings(&captions_, text_rect_.left, text_rect_.top,
                           text_rect_.right - text_rect_.left, 0);
}

// ORIGINAL: 0x0045F2E0 ?exec@Interlude@@QAEXPAD@Z 0x0045F2E0-0x0045F5C0
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/0045f2e0.cpp: original identity, complete spans and retained seed history", "source:src/interlude.h: genuine GraphicWin base and embedded member lifetime", "source:recovery/tasks/interlude-lifetime-owner-closure.json: bounded coordinator authorization"], "reason": "Original45F2E0 configures Interlude art, captions, clip rectangle and modal lifecycle on actual embedded Buffer/StringList members; source-owned borrowed fonts preserve shared producer relationships."}
void Interlude::exec(char *section) {
    close();
    GraphicWin::init(0, 0, WinScreenWidth, WinScreenHeight, 0, 0, 0, 0, 0);
    Buffer *canvas = static_cast<Buffer *>(this);
    canvas->set_text_color(MessageWinTextColor, -1, 1, 1);
    canvas->set_text_color2(MessageWinTextColor, -1, 1, 1);
    canvas->set_text_color3(MessageWinTextColor, -1, 1, 1);
    canvas->set_text_color_hyper(2, -1, 1, 1);
    canvas->set_font(InterludeNormalFont, InterludeSecondFont,
                     InterludeThirdFont, 0);
    art_.load_pcx(WinScreenWidth == 800 ? "credits.pcx" : "credits2.pcx",
                  nullptr, 10, 0xec);
    captions_.load((Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN)
                      ? const_cast<char *>("INTERLUDEA") : InterludeTextFile,
                    section, 1, nullptr);
    text_rect_ = canvas->clip_rect();
    const int inset_height = (text_rect_.bottom - text_rect_.top) / -6;
    const int inset_width = (text_rect_.right - text_rect_.left) / -6;
    text_rect_.left -= inset_width;
    text_rect_.right += inset_width;
    text_rect_.top -= inset_height;
    text_rect_.bottom += inset_height;
    text_rect_.right -= (text_rect_.right - text_rect_.left) / 3;
    if (text_rect_.left > 24) text_rect_.left = 25;
    if (strcmp(section, "INTERLUDE21") == 0
        || strcmp(section, "INTERLUDE22") == 0) {
        text_rect_.top += 50;
        text_rect_.bottom += 50;
    }
    const int height = canvas->write_strings_height(
        &captions_, text_rect_.right - text_rect_.left, 0);
    const int offset = ((text_rect_.bottom - text_rect_.top) - height) / 2;
    text_rect_.top += offset;
    text_rect_.bottom += offset;
    set_modal(0, nullptr, nullptr);
    close();
    do_all_draws();
}

// ORIGINAL: 0x005230E0 ?interlude@@YAXHPADHH@Z 0x005230E0-0x00523B4E;0x0065D4F0-0x0065D6C8
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/005230e0.cpp: original identity, complete spans and retained seed history", "source:src/interlude.h: genuine GraphicWin base and embedded member lifetime", "source:recovery/tasks/interlude-lifetime-owner-closure.json: bounded coordinator authorization"], "reason": "Original5230E0 owns exactly one real Interlude local and its inline member destruction, not additional fabricated Buffer/StringStruct/RunResult objects; third/fourth arguments and saved-turn calculations follow original behavior."}
void __cdecl interlude(int id, char *text, int mode, int ending) {
    Interlude popup;
    if (IsMultiplayerNet || DiplomacyActive) return;
    const unsigned int shift = static_cast<unsigned int>(id) & 31u;
    if (GameInterludeFlags & (1u << shift)) return;
    int faction = ConsoleGlobal.faction_id_;
    const bool alien = (Players[faction].rule_flags & RFLAG_ALIEN) != 0;
    if (id < 32 ? alien : !alien) return;
    if ((GameState & 0x10) || (GameRules & 0x200000)) return;
    StringTemp[0] = 0;
    strcat(StringTemp, Players[faction].filename);
    CharUpperA(StringTemp);
    if (strcmp(StringTemp, "FUNGBOY") == 0) return;
    if (id < 32) GameInterludeFlags |= 1u << shift;
    if (id == 6) GameInterludeFlags |= 0x12;
    if (GamePreferences & 0x40000) return;

    const int saved_turn = TurnCurrentNum;
    parse_num(0, game_year(saved_turn));
    parse_num(1, game_year(saved_turn));
    parse_num(2, game_year(saved_turn));
    StringTemp[0] = 0;
    if (mode != 0) {
        strcat(StringTemp, mode == 2 ? "EPILOGUE" : mode == 3 ? "EPILOGUE2"
                           : mode == 4 ? "EPILOGUE3" : "INTERLUDE");
        x_pop_caption_label(InterludeTextFile, StringTemp, nullptr);
    }
    parse_num(1, static_cast<int>(game_year(saved_turn) - game_year(0)));
    if (id == 20) {
        parse_num(1, static_cast<int>(game_year(saved_turn) - game_year(0)) / 100);
    }

    faction = ConsoleGlobal.faction_id_;
    PluralityDefault = 0;
    GenderDefault = Players[faction].is_leader_female;
    parse_says(0, Players[faction].title_leader, -1, -1);
    faction = ConsoleGlobal.faction_id_;
    PluralityDefault = 0;
    GenderDefault = Players[faction].is_leader_female;
    parse_says(1, Players[faction].name_leader, -1, -1);
    parse_says(8, Players[ConsoleGlobal.faction_id_].assistant_name, -1, -1);
    parse_says(9, Players[ConsoleGlobal.faction_id_].scientist_name, -1, -1);
    faction = ConsoleGlobal.faction_id_;
    StringTemp[0] = 0;
    PluralityDefault = 0;
    GenderDefault = Players[faction].is_leader_female;
    strcat(StringTemp, Players[faction].name_leader);
    CharLowerA(StringTemp);
    parse_says(3, StringTemp, -1, -1);

    int best_value = 0;
    // The original writes this local only when a positive best-base score
    // is found; the later tie-selection loop reads the same local.
    int best_base;
    for (int i = 0; i < BaseCurrentCount; ++i) {
        if (Bases[i].faction_id_current == ConsoleGlobal.faction_id_) {
            int value = Bases[i].population_size;
            int offset, mask;
            bitmask(1, &offset, &mask);
            if (Bases[i].facilities_built[offset] & mask) value += 999;
            if (value > best_value) {
                parse_says(2, Bases[i].name_string, -1, -1);
                best_value = value;
                best_base = i;
            }
        }
    }
    if (ending == 0) {
        int best = 0;
        for (int i = 0; i < BaseCurrentCount; ++i) {
            if (Bases[i].faction_id_current == ConsoleGlobal.faction_id_) {
                int value = Bases[i].population_size + Bases[i].energy_surplus * 2;
                if (i == best_base) value = 1;
                if (value >= best) {
                    best = value;
                    parse_says(4, Bases[i].name_string, -1, -1);
                }
            }
        }
    }
    if (text) parse_says(5, text, -1, -1);
    StringTemp[0] = 0;
    strcat(StringTemp, Players[ConsoleGlobal.faction_id_].search_key);
    CharUpperA(StringTemp);
    if (strcmp(StringTemp, "CARETAKE") == 0) {
        parse_says(6, label_get(1128), -1, -1);
        parse_says(7, label_get(1129), -1, -1);
    } else {
        parse_says(6, label_get(1127), -1, -1);
        parse_says(7, label_get(1130), -1, -1);
    }
    if (id < 32) {
        if (Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN) return;
    } else {
        id -= 32;
        if (id == 3) GameInterludeFlags |= 0x10;
    }
    GameInterludeFlags |= 1u << (static_cast<unsigned int>(id) & 31u);
    StringTemp[0] = 0;
    strcat(StringTemp, "INTERLUDE");
    char number[20];
    _itoa(id, number, 10);
    strcat(StringTemp, number);
    popup.exec(StringTemp);
}
