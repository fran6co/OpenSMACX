"""Compile host controls against definitions extracted from src/faction.cpp."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "faction.cpp"
HARNESS = r'''
#define __cdecl
#include <cassert>
#include <cstdio>
#include <cstring>
#include <stdint.h>
#include <string>
#include <vector>
#define StringTemp string_temp
#define nullptr 0

const int MaxVehProtoFactionNum = 64;
const int MaxPlayerNum = 8;
const uint16_t PROTO_UNK_10 = 0x10;
const uint16_t PROTO_UNK_20 = 0x20;
const int DTREATY_PACT = 1;

struct Sprite { int id; };
struct Player {
    const char *title_leader;
    const char *name_leader;
    const char *noun_faction;
    const char *adj_name_faction;
    int noun_gender, is_noun_plural, is_leader_female;
};
// Fields touched by the actual extracted producer bodies, typed as in
// src/faction.h; this narrow fixture is not a layout/ABI claim.
struct PlayerData {
    uint32_t diplo_treaties[MaxPlayerNum];
    char diplo_patience[MaxPlayerNum];
    int diplo_spoke[MaxPlayerNum];
    int global_reputation;
};
struct GameDataStateFieldsOwner { int diff_level_current_, turn_current_num_; };
struct GameDataStateOwner { GameDataStateFieldsOwner fields_; };
struct VehPrototype { const char *veh_name; uint16_t flags; };
struct NetMock {
    void await_diplo(int n);
    void pop(const char *, int, unsigned int, int (__cdecl *)());
};

static std::vector<std::string> events;
static std::vector<int> parsed_slots;
struct ParseRecord { int slot, gender, plural; std::string text; };
static std::vector<ParseRecord> parse_records;
static std::vector<std::string> popup_labels;
static std::vector<int> wanted_ids;
static std::vector<int> wanted_factions;
static std::vector<int> achieved;
static std::vector<int> message_opcodes, message_payloads;
static std::vector<int> await_codes;
static std::vector<int> caption_null_callbacks;
static std::vector<int> noun_phrase_factions, noun_phrase_ids;
static int wants_speak_calls, speak_result, speak_replacement = -1;
static int popup_result = 1, multiplayer, tutorial_result, difficulty;
static int territory_result, territory_calls, random_result = 1, turn_number = 17;
static int pact_calls, map_calls, treaty_code, await_code, caption_calls;
static int popup_calls, net_pop_calls, scan_calls;
static int scan_best_proto = -1;
static int last_message_opcode, last_message_payload;
static bool has_proto[512];
static char string_temp[256];

Player Players[MaxPlayerNum];
PlayerData PlayersData[MaxPlayerNum];
GameDataStateOwner GameDataStateStorage;
VehPrototype VehPrototypes[MaxPlayerNum * MaxVehProtoFactionNum];
Sprite sprites[MaxPlayerNum];
Sprite *FactionDiplomacyPortraits[MaxPlayerNum];
int PopupDialogFactionID, BestProtoForTrade, BestProtoToMention;
int DiploScratchFA70, IsMultiplayerNet;
int PluralityDefault, GenderDefault;
const char *PopupStartCaption = "CAPTION";
const char *DiploLabelSendingTECH = "SENDINGTECH";
const char *DiploLabelSendingTREATYON = "SENDINGTREATYON";
const char *DiploLabelSendingTRADEMAPS = "SENDINGMAPS";
const char *DiploLabelPROTO = "PROTO";
const char *DiploLabelPACT = "PACT";
const char *DiploLabelRIVAL = "RIVAL";
const char *DiploLabelMAKEPACT = "MAKEPACT";
const char *DiploLabelHERESTHEMAP = "HERESTHEMAP";
const char *DiploLabelMAKETREATY = "MAKETREATY";
const char *DiploLabelCOMMERCE = "COMMERCE";
const char *DiploLabelTIMETOLEAVE = "TIMETOLEAVE";
const char *DiploLabelMAKETRUCE = "MAKETRUCE";
NetMock NetDaemonNet;
struct NetMsgMock {
    void pop(const char *s, int t, unsigned int f, int (__cdecl *cb)()) {
        events.push_back(std::string("netpop:") + s);
        ++net_pop_calls;
    }
} NetMsgPrimary;

void __cdecl reset_fixture() {
    events.clear(); parsed_slots.clear(); parse_records.clear(); popup_labels.clear();
    wanted_ids.clear(); wanted_factions.clear(); achieved.clear();
    message_opcodes.clear(); message_payloads.clear();
    await_codes.clear();
    caption_null_callbacks.clear(); noun_phrase_factions.clear(); noun_phrase_ids.clear();
    std::memset(Players, 0, sizeof(Players));
    std::memset(PlayersData, 0, sizeof(PlayersData));
    std::memset(VehPrototypes, 0, sizeof(VehPrototypes));
    std::memset(has_proto, 0, sizeof(has_proto));
    for (int i = 0; i < MaxPlayerNum; ++i) {
        Players[i].title_leader = "Title";
        Players[i].name_leader = "Leader";
        Players[i].noun_faction = "Faction";
        Players[i].adj_name_faction = "AdjName";
        FactionDiplomacyPortraits[i] = &sprites[i];
    }
    wants_speak_calls = 0; speak_result = 1; speak_replacement = -1;
    popup_result = 1; multiplayer = 0; tutorial_result = 1; difficulty = 1;
    territory_result = territory_calls = 0; random_result = 1; turn_number = 17;
    pact_calls = map_calls = treaty_code = await_code = caption_calls = 0;
    popup_calls = net_pop_calls = scan_calls = 0;
    scan_best_proto = -1;
    last_message_opcode = last_message_payload = 0;
    PopupDialogFactionID = BestProtoForTrade = BestProtoToMention = -1;
    DiploScratchFA70 = IsMultiplayerNet = GameDataStateStorage.fields_.diff_level_current_ = 0;
    GameDataStateStorage.fields_.turn_current_num_ = turn_number;
    PluralityDefault = GenderDefault = 0;
    std::memset(string_temp, 0, sizeof(string_temp));
}

int __cdecl pop_wait() { events.push_back("wait"); return 1; }
int __cdecl wants_to_speak(int a, int b) {
    ++wants_speak_calls; events.push_back("wants_speak");
    if (speak_replacement >= 0) BestProtoToMention = speak_replacement;
    return speak_result;
}
int __cdecl wants_prototype(int proto, int faction) {
    wanted_ids.push_back(proto);
    wanted_factions.push_back(faction);
    events.push_back("wants_proto");
    return proto >= 0 && proto < 512 && has_proto[proto];
}
void __cdecl tech_achieved(int faction, int tech, int with, int mode) {
    achieved.push_back(tech); events.push_back("tech");
}
void __cdecl log_say(const char *s, int, int, int) {
    events.push_back(std::string("log:") + s);
}
void __cdecl message_data(int op, int, int, int, int payload, int) {
    last_message_opcode = op;
    last_message_payload = payload;
    message_opcodes.push_back(op);
    message_payloads.push_back(payload);
    events.push_back("message");
}
void NetMock::await_diplo(int n) {
    await_code = n; events.push_back("await");
    await_codes.push_back(n);
}
int __cdecl treaty_on(int a, int b, int treaty) {
    ++pact_calls; treaty_code = treaty; events.push_back("treaty"); return 0;
}
void __cdecl diplomacy_check(int, int, int) { events.push_back("check"); }
void __cdecl diplomacy_caption(int, int) { ++caption_calls; events.push_back("caption"); }
void __cdecl parse_says(int slot, const char *text, int, int) {
    parsed_slots.push_back(slot); events.push_back("parse");
    ParseRecord record;
    record.slot = slot; record.gender = GenderDefault; record.plural = PluralityDefault;
    record.text = text ? text : "";
    parse_records.push_back(record);
}
void __cdecl say_stats_2(char *out, int proto) {
    std::sprintf(out, "stats%d", proto); events.push_back("stats");
}
int __cdecl X_pops(const char *s, int, Sprite *, int (__cdecl *cb)()) {
    ++popup_calls; popup_labels.push_back(s);
    events.push_back(std::string("popup:") + s); return popup_result;
}
int __cdecl X_pop(const char *, const char *s, int (__cdecl *cb)()) {
    ++popup_calls; popup_labels.push_back(s);
    caption_null_callbacks.push_back(cb == 0);
    events.push_back(std::string("caption_popup:") + s); return 0;
}
void __cdecl scan_prototypes(int, int) {
    ++scan_calls; events.push_back("scan");
    if (scan_best_proto >= 0) BestProtoToMention = scan_best_proto;
}
void __cdecl trade_maps(int, int) { ++map_calls; events.push_back("trade_maps"); }
int __cdecl tut_check2(int) { events.push_back("tutorial"); return tutorial_result; }
int __cdecl territory(int actor, int partner, int mode, int *, int *) {
    ++territory_calls;
    events.push_back("territory");
    assert(actor == 2 && partner == 1 && mode == 2);
    return territory_result;
}
const char *__cdecl get_pact(int) { events.push_back("get_pact"); return "PACTTEXT"; }
const char *__cdecl get_phrase(int, int) { events.push_back("get_phrase"); return "PHRASE"; }
const char *__cdecl get_noun_phrase(int faction, int phrase) {
    noun_phrase_factions.push_back(faction); noun_phrase_ids.push_back(phrase);
    GenderDefault = Players[faction].noun_gender;
    PluralityDefault = Players[faction].is_noun_plural;
    return faction == 1 ? "ActorNounPhrase" : "PartnerNounPhrase";
}
int __cdecl random(int, int) { events.push_back("random"); return random_result; }
char *__cdecl _itoa(int value, char *out, int) {
    std::sprintf(out, "%d", value); return out;
}

@@ACTUAL_FACTION_BODIES@@

int main() {
    reset_fixture();
    BestProtoToMention = -1;
    mention_prototypes(1, 2);
    assert(PopupDialogFactionID == 2 && wants_speak_calls == 0);

    reset_fixture();
    BestProtoToMention = 1;
    speak_replacement = 2;
    PlayersData[1].diplo_treaties[2] = DTREATY_PACT;
    mention_prototypes(1, 2);
    assert(Players[2].name_leader != 0 && VehPrototypes[2].flags & PROTO_UNK_20);
    assert(BestProtoToMention == 2 && BestProtoForTrade == -1);
    assert(wants_speak_calls == 1 && parsed_slots.size() == 2);

    reset_fixture();
    BestProtoToMention = 3;
    speak_result = 1;
    IsMultiplayerNet = 1;
    has_proto[69] = true;
    has_proto[133] = true;
    mention_prototypes(1, 2);
    assert(wanted_ids.size() == 128 && wanted_ids[10] == 69 && wanted_factions[10] == 2);
    assert(wanted_ids[11] == 133 && wanted_factions[11] == 1);
    assert(last_message_opcode == 0x244B && await_code == 0x44B);
    assert(VehPrototypes[69].flags & PROTO_UNK_20);
    assert(VehPrototypes[133].flags & PROTO_UNK_20);
    assert(BestProtoForTrade == -1 && BestProtoToMention == -1);

    reset_fixture();
    BestProtoToMention = 3;
    popup_result = 0;
    mention_prototypes(1, 2);
    assert(BestProtoToMention == -1 && BestProtoForTrade == -1);
    assert(VehPrototypes[128].flags & PROTO_UNK_10);

    reset_fixture();
    BestProtoToMention = -1;
    multiplayer = IsMultiplayerNet = 1;
    pact_of_brotherhood(1, 2);
    assert(message_opcodes.size() >= 2 && message_opcodes[0] == 0x2441);
    assert(message_payloads[0] == 0x01000001);
    assert(await_codes.size() == 1 && await_codes[0] == 0x448);
    assert(PlayersData[1].global_reputation == 1 && PlayersData[2].global_reputation == 1);
    assert(PlayersData[2].diplo_patience[1] == 0);
    assert(events[0] == "log:Sending TREATY ON" && events[1] == "message");

    reset_fixture();
    GameDataStateStorage.fields_.diff_level_current_ = 3;
    tutorial_result = 0;
    pact_of_brotherhood(1, 2);
    assert(map_calls == 1 && scan_calls == 1);
    assert(popup_labels.size() == 2 && popup_labels[0] == "MAKEPACT");
    assert(popup_labels[1] == "HERESTHEMAP");
    assert(caption_calls == 1 && territory_calls == 0 && net_pop_calls == 0);

    reset_fixture();
    territory_result = 1;
    pact_of_brotherhood(1, 2);
    assert(territory_calls == 0 && net_pop_calls == 0);
    assert(popup_labels.size() == 3 && popup_labels[2] == "PACT");
    assert(caption_null_callbacks.size() == 1 && caption_null_callbacks[0] == 1);
    assert(parsed_slots.size() == 11);
    assert(parse_records[7].text == "Faction" && parse_records[8].text == "Title");
    assert(parse_records[9].text == "Leader" && parse_records[10].text == "AdjName");

    reset_fixture();
    IsMultiplayerNet = 1;
    make_treaty(1, 2);
    assert(message_opcodes.size() == 1 && message_opcodes[0] == 0x2441);
    assert(message_payloads[0] == 0x01000202 && await_codes.size() == 1 && await_codes[0] == 0x441);
    assert(parsed_slots.size() == 8 && parsed_slots[0] == 0 && parsed_slots[5] == 5);
    assert(parse_records[6].text == "ActorNounPhrase" && parse_records[7].text == "PartnerNounPhrase");
    assert(noun_phrase_factions.size() == 2 && noun_phrase_factions[0] == 1 && noun_phrase_factions[1] == 2);
    assert(noun_phrase_ids.size() == 2 && noun_phrase_ids[0] == 9 && noun_phrase_ids[1] == 9);
    assert(PlayersData[1].diplo_spoke[2] == turn_number);
    assert(PlayersData[1].global_reputation == 1 && PlayersData[2].global_reputation == 1);

    reset_fixture();
    random_result = 2;
    GameDataStateStorage.fields_.diff_level_current_ = 3;
    tutorial_result = 0;
    territory_result = 1;
    make_treaty(1, 2);
    assert(popup_labels.size() == 1 && popup_labels[0] == "MAKETREATY2");
    assert(net_pop_calls == 1 && events[events.size() - 1] == "netpop:TIMETOLEAVE");
    assert(parsed_slots.size() == 10);
    assert(noun_phrase_factions.size() == 2 && noun_phrase_factions[0] == 1 && noun_phrase_factions[1] == 2);
    assert(parse_records[8].text == "Faction" && parse_records[8].gender == Players[2].noun_gender);
    assert(parse_records[8].plural == Players[2].is_noun_plural);
    assert(parse_records[9].text == "AdjName");

    reset_fixture();
    Players[2].noun_gender = 7;
    Players[2].is_noun_plural = 1;
    make_treaty(1, 2);
    assert(popup_labels.size() == 2 && popup_labels[1] == "COMMERCE");
    assert(parsed_slots.size() == 9 && parsed_slots[8] == 0);
    assert(parse_records[8].text == "Faction" && parse_records[8].gender == 7 && parse_records[8].plural == 1);
    assert(noun_phrase_factions.size() == 2 && noun_phrase_factions[0] == 1 && noun_phrase_factions[1] == 2);

    reset_fixture();
    pledge_truce(1, 2, 0);
    assert(treaty_code == 0x204 && PopupDialogFactionID == 2);
    assert(PlayersData[1].diplo_spoke[2] == turn_number);
    assert(caption_calls == 0 && parsed_slots.empty());

    reset_fixture();
    pledge_truce(1, 2, 1);
    assert(treaty_code == 0x204 && PopupDialogFactionID == 2 && caption_calls == 1);
    assert(parsed_slots.size() == 6 && parsed_slots[0] == 0 && parsed_slots[5] == 5);
    return 0;
}

'''
SIGNATURES = (
    r"void\s+__cdecl\s+mention_prototypes\s*\(",
    r"void\s+__cdecl\s+pact_of_brotherhood\s*\(",
    r"void\s+__cdecl\s+make_treaty\s*\(",
    r"void\s+__cdecl\s+pledge_truce\s*\(",
)


def _closing_brace(text: str, opening: int) -> int:
    depth = 0
    state = "code"
    i = opening
    while i < len(text):
        char = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""
        if state == "line":
            if char == "\n":
                state = "code"
        elif state == "block":
            if char == "*" and nxt == "/":
                state = "code"
                i += 1
        elif state in ("string", "char"):
            if char == "\\":
                i += 1
            elif (state == "string" and char == '"') or (state == "char" and char == "'"):
                state = "code"
        elif char == "/" and nxt == "/":
            state = "line"
            i += 1
        elif char == "/" and nxt == "*":
            state = "block"
            i += 1
        elif char == '"':
            state = "string"
        elif char == "'":
            state = "char"
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return i
        i += 1
    raise AssertionError("unterminated function definition")


def _actual_bodies() -> str:
    text = SOURCE.read_text()
    bodies = []
    for signature in SIGNATURES:
        match = re.search(signature, text)
        assert match, "missing actual source definition: " + signature
        opening = text.find("{", match.end())
        semicolon = text.find(";", match.end())
        assert opening >= 0 and (semicolon < 0 or opening < semicolon), signature
        closing = _closing_brace(text, opening)
        body = text[match.start():closing + 1]
        assert "BODY GOES HERE" not in body and "PENDING_BODY" not in body
        assert len(body) > 300, "body is incomplete: " + signature
        bodies.append(body.replace("nullptr", "0"))
    return "\n\n".join(bodies)


def _check_bodies(compiler: str, tmp_path: Path, bodies: str, *, reject: bool = False) -> None:
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(compiler + " is unavailable")
    fixture = HARNESS
    marker = "@@ACTUAL_FACTION_BODIES@@"
    assert fixture.count(marker) == 1
    source = fixture.replace(marker, bodies)
    cpp = tmp_path / "actual_diplomacy_producer_control.cpp"
    binary = tmp_path / "actual_diplomacy_producer_control"
    cpp.write_text(source)
    compile_result = subprocess.run(
        [executable, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         "-Wno-unused-parameter", "-Wno-c++11-compat", "-D__cdecl=",
         "-fsanitize=undefined", "-fno-sanitize-recover=undefined",
         str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=60,
    )
    assert compile_result.returncode == 0, compile_result.stderr
    run_result = subprocess.run(
        [str(binary)], capture_output=True, text=True, timeout=30,
    )
    if reject:
        assert run_result.returncode != 0, "behavior damage escaped the controls"
    else:
        assert run_result.returncode == 0, run_result.stderr


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_diplomacy_producer_bodies(compiler: str, tmp_path: Path) -> None:
    _check_bodies(compiler, tmp_path, _actual_bodies())


# These deliberately damaged copies must compile but fail the native-derived
# host expectations. They never replace product source or its annotations.
DAMAGE = {
    "missing_truce_popup_owner": (
        "void __cdecl pledge_truce",
        "    PopupDialogFactionID = faction_id_with;",
        "    /* deliberately removed owner write */", 1),
    "invented_pact_wait": (
        "void __cdecl pact_of_brotherhood",
        "message_data(0x2441, 0, faction_id_with, faction_id,\n                     0x01000001, 0);",
        "message_data(0x2441, 0, faction_id_with, faction_id,\n                     0x01000001, 0); NetDaemonNet.await_diplo(0x441);", 1),
    "omitted_noun_phrase": (
        "void __cdecl make_treaty",
        "    parse_says(6, get_noun_phrase(faction_id, 9), -1, -1);",
        "    /* deliberately omitted native parser callback */", 1),
    "extra_tutorial_phrase_load": (
        "void __cdecl make_treaty",
        "parse_says(0, Players[faction_id_with].noun_faction, -1, -1);",
        "parse_says(0, get_noun_phrase(faction_id_with, 9), -1, -1);", 2),
}


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("damage", sorted(DAMAGE))
def test_controls_reject_behavior_damage(compiler: str, damage: str, tmp_path: Path) -> None:
    bodies = _actual_bodies()
    owner, old, new, count = DAMAGE[damage]
    start = bodies.index(owner)
    end = bodies.find("\n\nvoid __cdecl", start + len(owner))
    if end < 0:
        end = len(bodies)
    body = bodies[start:end]
    assert body.count(old) == count, "control anchor changed: " + damage
    changed = bodies[:start] + body.replace(old, new, 1) + bodies[end:]
    _check_bodies(compiler, tmp_path, changed, reject=True)
