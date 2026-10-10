from pathlib import Path
import os
import shutil
import subprocess

import pytest
from match_recovery.source import Mode, read_file
from match_recovery.model import Span
from match_recovery.object_source import parse_objects
from match_recovery.pe_coff import PeImage


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/game.cpp"
IDENTITIES = {
    "sub_52a980": (0x0052A980, "int __cdecl sub_52a980() {"),
    "sub_58f4f0": (
        0x0058F4F0,
        'extern "C" char * __cdecl sub_58f4f0(char *a1, int a2, int a3, int a4) {',
    ),
    "say_fac_special": (
        0x0058F700,
        "void __cdecl say_fac_special(char *a1, char *a2, int a3) {",
    ),
    "sub_52db40": (0x0052DB40, "void sub_52db40() {"),
}


def test_distinct_start_only_token_bindings_match_pinned_strings() -> None:
    pin = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
    image = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image.is_file():
        pytest.skip("pinned original image unavailable for data evidence control")
    original = PeImage(image.read_bytes(), pin)
    rows = parse_objects(SOURCE.read_text(encoding="utf-8"), "src/game.cpp", pin,
                         address_space="pe:va")
    by_symbol = {row.symbol: row for row in rows}
    assert len(by_symbol) == len(rows), "duplicate original object symbol binding"
    names = ("Open", "Male", "Female", "Neutral", "One", "Two", "Sep", "Close")
    texts = ("$<", "M", "F", "N", "1", "2", ":", ">")
    for prefix, start in (("SayTok", 0x68FA70), ("SayFacTok", 0x68FAB0)):
        for index, (name, text) in enumerate(zip(names, texts)):
            row = by_symbol["_" + prefix + name]
            assert row.origin.address == start + index * 4
            # A short NUL-terminated read is not full allocation evidence.
            assert row.span is None and row.evidence is None
            assert original.read(Span(row.origin.address, row.origin.address + 4)).split(b"\0", 1)[0] == text.encode("ascii")


def braced_definition(source: str, signature: str) -> str:
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for position in range(brace, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[start : position + 1]
    raise AssertionError(f"unterminated body for {signature}")


def actual_body(name: str) -> str:
    address, signature = IDENTITIES[name]
    rows = [row for row in read_file(SOURCE) if row.address == address]
    assert len(rows) == 1, f"expected one mapped identity at {address:#x}"
    expected_mode = Mode.BODY if name == "sub_52db40" else Mode.FILE
    assert rows[0].mode is expected_mode
    body = braced_definition(SOURCE.read_text(encoding="utf-8"), signature)
    assert rows[0].region.find(signature) >= 0
    return body


def actual_token_source(which: str) -> str:
    source = SOURCE.read_text(encoding="utf-8")
    if which == "ordinary":
        first = "static const char SayTokOpen[] = \"$<\";"
        end = "extern \"C\" char * __cdecl sub_58f4f0"
    else:
        first = "static const char SayFacTokOpen[] = \"$<\";"
        end = "void __cdecl say_fac_special"
    start = source.index(first)
    stop = source.index(end, start)
    return source[start:stop]


def source_fragments() -> tuple[str, str, str, str, str]:
    return (
        actual_token_source("ordinary"),
        actual_body("sub_58f4f0"),
        actual_token_source("faction"),
        actual_body("say_fac_special"),
        actual_body("sub_52a980"),
    )


HARNESS = r'''#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#define __cdecl

struct VehPrototype {
    char veh_name[32];
    uint32_t ability_flags;
    uint8_t chassis_id, weapon_id, armor_id, reactor_id;
    uint8_t carry_capacity, cost, plan;
    int8_t unk_1;
    uint8_t obsolete_factions;
    int8_t combat_factions;
    int8_t icon_offset;
    int8_t padding;
    uint16_t flags;
    int16_t preq_tech;
};
static_assert(sizeof(VehPrototype) == 0x34, "prototype stride");
static_assert(offsetof(VehPrototype, flags) == 0x30, "prototype flags offset");
static_assert(offsetof(VehPrototype, preq_tech) == 0x32, "adjacent tech offset");
static_assert(offsetof(VehPrototype, preq_tech) == offsetof(VehPrototype, flags) + 2,
              "DWORD access spans flags and prerequisite tech");
constexpr int MaxVehProtoNum = 512;
VehPrototype VehPrototypes[MaxVehProtoNum];
struct GameStateFields { int vehicle_current_count_, base_current_count_; };
struct GameState { GameStateFields fields_; };
GameState GameDataStateStorage;

struct Player {
    char adj_name_faction[64];
    int noun_gender;
    int is_noun_plural;
};
Player Players[8];
int GenderDefault = 0;
int PluralityDefault = 0;
std::vector<std::string> events;
std::string last_parse_input;
void parse_string(char *input, char *output) {
    events.push_back("parse");
    last_parse_input = input;
    std::strcpy(output, "parsed[");
    std::strcat(output, input);
    std::strcat(output, "]");
}

struct NetDaemonSaveProcessSlot {
    char name_[0x20];
    uint32_t pid_;
    int8_t identity_;
    uint8_t field_25_;
    uint8_t field_26_;
    int8_t field_27_;
    uint8_t status_bytes_[0x174];
};
static_assert(offsetof(NetDaemonSaveProcessSlot, identity_) == 0x24,
              "snapshot identity offset");
static_assert(offsetof(NetDaemonSaveProcessSlot, status_bytes_) == 0x28,
              "snapshot status offset");
static_assert(sizeof(NetDaemonSaveProcessSlot) == 0x19C, "snapshot stride");
struct NetDaemon {
    NetDaemonSaveProcessSlot saved_process_slots_[8];
    NetDaemonSaveProcessSlot process_slots_[8]; // live-table damage control
};
NetDaemon NetDaemonNet;
'''


def compile_and_run(compiler: str, cpp_source: str, tmp_path: Path) -> subprocess.CompletedProcess:
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} is unavailable")
    cpp = tmp_path / "actual_source.cpp"
    binary = tmp_path / "actual_source"
    cpp.write_text(cpp_source, encoding="utf-8")
    built = subprocess.run(
        [executable, "-std=c++17", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(binary)],
        capture_output=True,
        text=True,
        timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=45)


def require_success(compiler: str, program: str, tmp_path: Path) -> None:
    ran = compile_and_run(compiler, program, tmp_path)
    assert ran.returncode == 0, ran.stdout + ran.stderr


def require_mutant_rejected(compiler: str, program: str, tmp_path: Path) -> None:
    ran = compile_and_run(compiler, program, tmp_path)
    assert ran.returncode != 0, "damage control accepted a deliberate source mutant"


def test_actual_mapped_identities_and_token_address_evidence() -> None:
    source = SOURCE.read_text(encoding="utf-8")
    for name, (address, _) in IDENTITIES.items():
        rows = [row for row in read_file(SOURCE) if row.address == address]
        expected_mode = Mode.BODY if name == "sub_52db40" else Mode.FILE
        assert len(rows) == 1 and rows[0].mode is expected_mode, name
        assert f"// ORIGINAL: 0x{address:08X}" in rows[0].region
    assert "0x0068FA70..8C" in source
    assert "0x0068FAB0..CC" in source
    assert "0x009AB898" not in actual_body("sub_52a980")
    assert "0x0093E99E" not in actual_body("sub_52db40")
    assert "0x0068FA70" not in actual_token_source("ordinary")
    assert "0x0068FAB0" not in actual_token_source("faction")
    assert "no shared original allocation/pooling is established" in source

    reset_body = actual_body("sub_52a980")
    ordered_prototype_writes = [
        reset_body.index("memcpy(&flags_and_preq_tech"),
        reset_body.index("flags_and_preq_tech &= ~uint32_t(1)"),
        reset_body.index("memcpy(prototype_bytes + offsetof(VehPrototype, flags)"),
        reset_body.index("prototype.icon_offset = -1"),
        reset_body.index("vehicle_current_count_ = 0"),
        reset_body.index("base_current_count_ = 0"),
    ]
    assert ordered_prototype_writes == sorted(ordered_prototype_writes)
    save_body = actual_body("sub_52db40")
    ordered_save_writes = [
        save_body.index("record.identity_ = -1"),
        save_body.index("record.field_25_ = 0"),
        save_body.index("record.field_26_ = 0"),
        save_body.index("record.field_27_ = -1"),
        save_body.index("record.status_bytes_[0] = 2"),
    ]
    assert ordered_save_writes == sorted(ordered_save_writes)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_prototype_reset_keeps_full_window_and_all_512_rows(compiler, tmp_path):
    reset = actual_body("sub_52a980")
    program = HARNESS + reset + r'''
int main() {
    unsigned char expected[MaxVehProtoNum][sizeof(VehPrototype)];
    for (int i = 0; i < MaxVehProtoNum; ++i) {
        std::memset(&VehPrototypes[i], 0xA5, sizeof(VehPrototypes[i]));
        VehPrototypes[i].flags = static_cast<uint16_t>(0xA501u ^ (i * 13u));
        const uint16_t tech_bits = static_cast<uint16_t>(0xD000u ^ (i * 31u));
        std::memcpy(&VehPrototypes[i].preq_tech, &tech_bits, sizeof(tech_bits));
        VehPrototypes[i].icon_offset = static_cast<int8_t>(i & 0x7F);
        std::memcpy(expected[i], &VehPrototypes[i], sizeof(VehPrototype));
        uint32_t expected_flags_and_tech;
        std::memcpy(&expected_flags_and_tech,
                    expected[i] + offsetof(VehPrototype, flags),
                    sizeof(expected_flags_and_tech));
        expected_flags_and_tech &= ~uint32_t(1);
        std::memcpy(expected[i] + offsetof(VehPrototype, flags),
                    &expected_flags_and_tech, sizeof(expected_flags_and_tech));
        expected[i][offsetof(VehPrototype, icon_offset)] = 0xFF;
    }
    GameDataStateStorage.fields_.vehicle_current_count_ = 1234;
    GameDataStateStorage.fields_.base_current_count_ = 4321;
    assert(sub_52a980() == 0);
    assert(GameDataStateStorage.fields_.vehicle_current_count_ == 0);
    assert(GameDataStateStorage.fields_.base_current_count_ == 0);
    for (int i = 0; i < MaxVehProtoNum; ++i) {
        assert(std::memcmp(&VehPrototypes[i], expected[i], sizeof(VehPrototype)) == 0);
    }
}
'''
    require_success(compiler, program, tmp_path)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_save_initializer_touches_only_snapshot_prefix(compiler, tmp_path):
    reset = actual_body("sub_52db40")
    program = HARNESS + reset + r'''
int main() {
    std::memset(&NetDaemonNet, 0x5A, sizeof(NetDaemonNet));
    for (int i = 0; i < 8; ++i) {
        NetDaemonNet.saved_process_slots_[i].identity_ = static_cast<int8_t>(i);
        NetDaemonNet.saved_process_slots_[i].field_25_ = 0x31 + i;
        NetDaemonNet.saved_process_slots_[i].field_26_ = 0x41 + i;
        NetDaemonNet.saved_process_slots_[i].field_27_ = static_cast<int8_t>(0x50 + i);
        NetDaemonNet.saved_process_slots_[i].status_bytes_[0] = 0x61 + i;
        NetDaemonNet.process_slots_[i].identity_ = static_cast<int8_t>(0x70 + i);
    }
    NetDaemon before = NetDaemonNet;
    sub_52db40();
    for (int i = 0; i < 8; ++i) {
        const NetDaemonSaveProcessSlot &before_row = before.saved_process_slots_[i];
        const NetDaemonSaveProcessSlot &after_row = NetDaemonNet.saved_process_slots_[i];
        assert(after_row.identity_ == -1);
        assert(after_row.field_25_ == 0);
        assert(after_row.field_26_ == 0);
        assert(after_row.field_27_ == -1);
        assert(after_row.status_bytes_[0] == 2);
        assert(std::memcmp(before_row.name_, after_row.name_, sizeof(before_row.name_)) == 0);
        assert(before_row.pid_ == after_row.pid_);
        assert(std::memcmp(before_row.status_bytes_ + 1, after_row.status_bytes_ + 1,
                           sizeof(before_row.status_bytes_) - 1) == 0);
        assert(NetDaemonNet.process_slots_[i].identity_ == before.process_slots_[i].identity_);
    }
    assert(std::memcmp(before.process_slots_, NetDaemonNet.process_slots_,
                       sizeof(before.process_slots_)) == 0);
}
'''
    require_success(compiler, program, tmp_path)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_markup_helpers_preserve_branches_defaults_parse_and_append(compiler, tmp_path):
    tokens, phrase, fac_tokens, fac, _reset = source_fragments()
    program = HARNESS + tokens + phrase + fac_tokens + fac + r'''
static std::string expected_markup(int gender, int plural, const char *text) {
    std::string value = "$<";
    if (gender == 0) value += "M";
    else if (gender == 1) value += "F";
    else if (gender == 2) value += "N";
    if (plural == 0) value += "1";
    else if (plural == 1) value += "2";
    value += ":";
    value += text;
    value += ">";
    return value;
}
int main() {
    std::strcpy(Players[3].adj_name_faction, "AdjName");
    std::strcpy(Players[4].adj_name_faction, "OtherAdj");
    char output[1024];
    for (int gender = 0; gender <= 3; ++gender) {
        for (int plural = 0; plural <= 2; ++plural) {
            std::strcpy(output, "prefix:");
            events.clear();
            const std::string expected = expected_markup(gender, plural, "AdjName");
            char *returned = sub_58f4f0(output, 3, plural, gender);
            assert(returned == output);
            assert(last_parse_input == expected);
            assert(std::string(output) == "prefix:parsed[" + expected + "]");
            assert(events.size() == 1 && events[0] == "parse");
        }
    }
    GenderDefault = 2;
    PluralityDefault = 1;
    std::strcpy(output, "D:");
    const std::string defaulted = expected_markup(2, 1, "AdjName");
    assert(sub_58f4f0(output, 3, -1, -1) == output);
    assert(last_parse_input == defaulted);
    assert(std::string(output) == "D:parsed[" + defaulted + "]");
    // Invalid positive selectors omit only their optional token.
    std::strcpy(output, "I:");
    const std::string invalid = expected_markup(9, 7, "AdjName");
    sub_58f4f0(output, 3, 7, 9);
    assert(last_parse_input == invalid);

    for (int gender = 0; gender <= 2; ++gender) {
        for (int plural = 0; plural <= 1; ++plural) {
            std::strcpy(output, "F:");
            Players[5].noun_gender = gender;
            Players[5].is_noun_plural = plural;
            say_fac_special(output, const_cast<char *>("Word"), 5);
            const std::string faction_markup = expected_markup(gender, plural, "Word");
            assert(last_parse_input == faction_markup);
            assert(std::string(output) == "F:parsed[" + faction_markup + "]");
        }
    }
    Players[5].noun_gender = 8;
    Players[5].is_noun_plural = 8;
    std::strcpy(output, "X:");
    const std::string faction_invalid = expected_markup(8, 8, "Noun");
    say_fac_special(output, const_cast<char *>("Noun"), 5);
    assert(last_parse_input == faction_invalid);
    assert(std::string(output) == "X:parsed[" + faction_invalid + "]");
}
'''
    require_success(compiler, program, tmp_path)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_damage_controls_reject_loop_owner_and_token_mutants(compiler, tmp_path):
    tokens, phrase, fac_tokens, fac, reset = source_fragments()
    full_reset = HARNESS + tokens + reset + r'''
int main() {
    for (int i = 0; i < MaxVehProtoNum; ++i) {
        VehPrototypes[i].flags = 3;
        VehPrototypes[i].preq_tech = static_cast<int16_t>(0x5A5A);
    }
    sub_52a980();
    assert((VehPrototypes[MaxVehProtoNum - 1].flags & 1u) == 0);
}
'''
    loop_mutant = reset.replace(
        "prototype_id < MaxVehProtoNum", "prototype_id < MaxVehProtoNum - 1", 1
    )
    assert loop_mutant != reset
    require_mutant_rejected(
        compiler,
        HARNESS + loop_mutant + full_reset[len(HARNESS + tokens + reset):],
        tmp_path,
    )

    snapshot_test = r'''
int main() {
    std::memset(&NetDaemonNet, 0x5A, sizeof(NetDaemonNet));
    NetDaemonNet.saved_process_slots_[0].identity_ = 12;
    NetDaemonNet.process_slots_[0].identity_ = 13;
    sub_52db40();
    assert(NetDaemonNet.saved_process_slots_[0].identity_ == -1);
    assert(NetDaemonNet.process_slots_[0].identity_ == 13);
}
'''
    owner_mutant = actual_body("sub_52db40").replace(
        "NetDaemonNet.saved_process_slots_[slot]",
        "NetDaemonNet.process_slots_[slot]",
        1,
    )
    assert owner_mutant != actual_body("sub_52db40")
    require_mutant_rejected(compiler, HARNESS + owner_mutant + snapshot_test, tmp_path)

    changed_tokens = tokens.replace('SayTokTwo[] = "2"', 'SayTokTwo[] = "X"', 1)
    assert changed_tokens != tokens
    markup_test = r'''
int main() {
    std::strcpy(Players[0].adj_name_faction, "Adj");
    char output[128] = "";
    sub_58f4f0(output, 0, 1, 0);
    assert(last_parse_input == "$<M2:Adj>");
}
'''
    require_mutant_rejected(compiler, HARNESS + changed_tokens + phrase + markup_test, tmp_path)

    save_body = actual_body("sub_52db40")
    save_field_mutant = save_body.replace("record.field_25_ = 0", "record.field_25_ = 1", 1)
    assert save_field_mutant != save_body
    save_field_test = r'''
int main() {
    std::memset(&NetDaemonNet, 0x5A, sizeof(NetDaemonNet));
    sub_52db40();
    assert(NetDaemonNet.saved_process_slots_[0].field_25_ == 0);
}
'''
    require_mutant_rejected(compiler, HARNESS + save_field_mutant + save_field_test, tmp_path)

    default_mutant = phrase.replace("a4 = GenderDefault", "a4 = PluralityDefault", 1)
    assert default_mutant != phrase
    default_test = r'''
int main() {
    std::strcpy(Players[0].adj_name_faction, "Adj");
    GenderDefault = 2;
    PluralityDefault = 1;
    char output[128] = "";
    sub_58f4f0(output, 0, -1, -1);
    assert(last_parse_input == "$<N2:Adj>");
}
'''
    require_mutant_rejected(compiler, HARNESS + tokens + default_mutant + default_test, tmp_path)

    order_mutant = fac.replace(
        "strcat(buf1, SayFacTokSep);\n    strcat(buf1, a2);",
        "strcat(buf1, a2);\n    strcat(buf1, SayFacTokSep);",
        1,
    )
    assert order_mutant != fac
    order_test = r'''
int main() {
    Players[0].noun_gender = 0;
    Players[0].is_noun_plural = 1;
    char output[128] = "";
    say_fac_special(output, const_cast<char *>("Word"), 0);
    assert(last_parse_input == "$<M2:Word>");
}
'''
    require_mutant_rejected(compiler, HARNESS + fac_tokens + order_mutant + order_test, tmp_path)
