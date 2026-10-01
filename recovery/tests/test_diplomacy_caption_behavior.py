from pathlib import Path
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[2]


def source_body(signature: str, source_name: str) -> str:
    source = (ROOT / source_name).read_text()
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for pos in range(brace, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start:pos + 1]
    raise AssertionError(f"unterminated source body: {signature}")


BODY = source_body(
    "void __cdecl diplomacy_caption(int faction_id, int faction_id_with) {",
    "src/faction.cpp",
)


HARNESS = r'''#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
using std::intptr_t;
#define __cdecl
using BOOL = int;
using LPSTR = char *;
constexpr int DTREATY_PACT = 1;
constexpr int DTREATY_TREATY = 2;
constexpr int DTREATY_VENDETTA = 0x10;
constexpr int DTREATY_HAVE_SURRENDERED = 0x02000000;
struct Player { char noun_faction[32] = {}; int noun_gender = 0; int is_noun_plural = 0; };
struct PlayerData { uint32_t diplo_treaties[8] = {}; };
Player Players[8];
PlayerData PlayersData[8];
char mood_text[9][20] = {};
LPSTR Mood[9] = {};
int DiploFriction = 0;
int DiploScratchFA98 = 0;
int GenderDefault = 0;
BOOL PluralityDefault = 0;
std::vector<int> calls;
std::string parsed;
int parsed_slot = -1;
int special_calls = 0;
int special_mood = -1;
int special_faction = -1;
int special_label_calls = 0;
int visible = 0;
int visible_after_parse = -1;
int draw_info_calls = 0;
int soft_update_calls = 0;
const void *soft_update_rect = nullptr;
struct Rect { int left, top, right, bottom; };
struct Popup {
    Rect rect1_{1, 2, 101, 42};
    int is_visible() { calls.push_back(40); return visible; }
    void draw_info() { calls.push_back(41); ++draw_info_calls; }
    void soft_update(Rect *rect) { calls.push_back(42); ++soft_update_calls; soft_update_rect = rect; }
};
Popup DiplomacyPopup;
struct Strings {
    char *get(int address) {
        if (address >= 0x1000 && address < 0x1009) return mood_text[address - 0x1000];
        assert(false && "unexpected StringTable key");
        return nullptr;
    }
} StringTable;
char special_label[] = "SPECIAL";
char *label_get(int index) {
    assert(index == 0x1E8);
    ++special_label_calls;
    return special_label;
}
void say_fac_special(char *output, char *mood, int faction) {
    calls.push_back(20);
    ++special_calls;
    special_mood = -1;
    for (int i = 0; i < 9; ++i) if (mood == mood_text[i]) special_mood = i;
    special_faction = faction;
    std::strcat(output, "<");
    std::strcat(output, mood);
    std::strcat(output, ">");
}
int parse_says(int slot, char *text, int, int) {
    calls.push_back(30);
    parsed_slot = slot;
    parsed = text;
    if (visible_after_parse >= 0) visible = visible_after_parse;
    return 0;
}
'''


def run_case(test_code: str) -> None:
    with tempfile.TemporaryDirectory() as directory:
        cpp = Path(directory) / "fixture.cpp"
        exe = Path(directory) / "fixture"
        cpp.write_text(HARNESS + BODY + "\nint main() {\n" + test_code + "\n}\n")
        subprocess.run(
            ["g++", "-std=c++17", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
            check=True,
            capture_output=True,
            text=True,
        )
        subprocess.run([str(exe)], check=True)


def test_mood_ranges_and_friction_clamp_are_taken_from_actual_body() -> None:
    run_case(r'''
        std::strcpy(Players[2].noun_faction, "Faction Two");
        Players[2].noun_gender = 2;
        Players[2].is_noun_plural = 1;
        for (int i = 0; i < 9; ++i) {
            std::sprintf(mood_text[i], "mood-%d", i);
            Mood[i] = reinterpret_cast<char *>(static_cast<intptr_t>(0x1000 + i));
        }
        struct Case { int friction; int expected; } cases[] = {
            {-1, 0}, {0, 0}, {1, 1}, {2, 1}, {3, 2}, {4, 2},
            {5, 3}, {8, 3}, {9, 4}, {12, 4}, {13, 5}, {15, 5},
            {16, 6}, {17, 6}, {18, 7}, {19, 7}, {20, 8},
        };
        for (const Case &item : cases) {
            DiploFriction = item.friction;
            special_calls = special_label_calls = 0;
            diplomacy_caption(1, 2);
            assert(special_calls == 1 && special_faction == 2);
            assert(special_mood == item.expected);
            assert(parsed == std::string("Faction Two - <mood-") +
                                 std::to_string(item.expected) + ">");
            assert(parsed_slot == 7);
            assert(GenderDefault == 2 && PluralityDefault == 1);
        }
        PlayersData[1].diplo_treaties[2] = DTREATY_VENDETTA;
        DiploFriction = 14;
        special_calls = 0;
        diplomacy_caption(1, 2);
        assert(DiploFriction == 15 && special_mood == 5 && special_calls == 1);
        DiploFriction = 15;
        diplomacy_caption(1, 2);
        assert(DiploFriction == 15 && special_mood == 5);
    ''')


def test_special_label_predicates_and_post_parse_redraw_order() -> None:
    run_case(r'''
        std::strcpy(Players[2].noun_faction, "Faction Two");
        for (int i = 0; i < 9; ++i) {
            std::sprintf(mood_text[i], "mood-%d", i);
            Mood[i] = reinterpret_cast<char *>(static_cast<intptr_t>(0x1000 + i));
        }
        DiploFriction = 0;
        DiploScratchFA98 = 1;
        PlayersData[1].diplo_treaties[2] = 0;
        special_calls = special_label_calls = 0;
        visible = 0;
        diplomacy_caption(1, 2);
        assert(parsed == "Faction Two - SPECIAL");
        assert(special_calls == 0 && special_label_calls == 1);
        assert(draw_info_calls == 0 && soft_update_calls == 0);

        DiploScratchFA98 = 0;
        PlayersData[1].diplo_treaties[2] = DTREATY_TREATY;
        PlayersData[2].diplo_treaties[1] = DTREATY_PACT | DTREATY_HAVE_SURRENDERED;
        special_label_calls = 0;
        diplomacy_caption(1, 2);
        assert(parsed == "Faction Two - SPECIAL");
        assert(special_label_calls == 1 && special_calls == 0);

        PlayersData[2].diplo_treaties[1] = 0;
        visible = 0;
        visible_after_parse = 1;
        calls.clear();
        diplomacy_caption(1, 2);
        assert(calls.size() >= 4);
        assert(calls[calls.size() - 4] == 30);
        assert(calls[calls.size() - 3] == 40);
        assert(calls[calls.size() - 2] == 41);
        assert(calls[calls.size() - 1] == 42);
        assert(soft_update_rect == &DiplomacyPopup.rect1_);
        const int draws = draw_info_calls;
        visible_after_parse = 0;
        diplomacy_caption(1, 2);
        assert(draw_info_calls == draws);
    ''')
