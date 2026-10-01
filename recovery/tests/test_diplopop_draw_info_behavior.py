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


BODY = source_body("void DiploPop::draw_info() {", "src/diplopop.cpp")


HARNESS = r'''#include <algorithm>
#include <cassert>
#include <cctype>
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
constexpr int DTREATY_TRUCE = 4;
constexpr int DTREATY_VENDETTA = 0x10;
struct RECT { int left, top, right, bottom; };
struct Font {
    int advance = 10;
    int line_advance() const { return advance; }
};
struct BoxSpriteParams {};
BoxSpriteParams InterfaceBoxStyles[51];
struct Draw { int id, mono, x, y, color; };
std::vector<Draw> sprite_draws;
struct Sprite {
    int id = -1;
    int w = 20;
    int h = 12;
    int width() const { return w; }
    int height() const { return h; }
    int transparent_index() const { return id + 20; }
    int draw_mono(class Buffer *, int, int x, int y, int color, int, int) {
        sprite_draws.push_back({id, 1, x, y, color}); return 0;
    }
    int draw(class Buffer *, int, int x, int y, int, int) {
        sprite_draws.push_back({id, 0, x, y, 0}); return 0;
    }
};
std::vector<int> spy_calls;
std::vector<int> spy_values;
struct Buffer {
    std::vector<std::string> events;
    std::vector<RECT> frames;
    std::vector<int> colors;
    std::vector<Font *> fonts;
    std::vector<std::string> wrapped;
    std::vector<std::string> written;
    int wrap_width = -1;
    void set_wrap_width(uint32_t value) { wrap_width = static_cast<int>(value); events.push_back("width"); }
    int box_sprite(RECT *rect, BoxSpriteParams *style) {
        assert(style == &InterfaceBoxStyles[0]); frames.push_back(*rect); events.push_back("frame"); return 0;
    }
    void set_text_color(int color, int, int, int) { colors.push_back(color); events.push_back("color"); }
    int set_font(Font *font, Font *, Font *, Font *) { fonts.push_back(font); events.push_back("font"); return 0; }
    int wrap_cent(char *text, RECT *area) {
        wrapped.emplace_back(text); events.push_back("center"); return area->bottom + 7;
    }
    int wrap(char *text, int, int y, int) {
        wrapped.emplace_back(text); events.push_back("wrap"); return y + 4;
    }
    int write_l(char *text, int, int y, int) {
        written.emplace_back(text); events.push_back("write"); return y + 10;
    }
};
struct MainInterface : Buffer {};
MainInterface MainInterfaceGlobal;
struct Player {
    char name_leader[32] = {};
    char noun_faction[32] = {};
    int noun_gender = 0;
    int is_noun_plural = 0;
};
struct PlayerData {
    uint32_t diplo_treaties[8] = {};
    uint32_t ranking = 0;
    int energy_reserves = 0;
    uint32_t hurry_cost_total = 0;
};
Player Players[8];
PlayerData PlayersData[8];
struct FactionArt { Sprite diplomacy_logo; } FactionsArt[8];
int ChatItemIdTable[8] = {10, 11, 12, 13, 14, 15, 16, 17};
int WinScreenWidth = 1024;
struct Console { int faction_id_ = 0; } ConsoleGlobal;
char StringTemp[1032] = {};
char label_text[2048] = {};
struct RulesMight { char *adj_start = nullptr; };
RulesMight Might[7];
char might_text[7][32] = {};
char mood_text[9][32] = {};
char *label_get(int index) {
    switch (index) {
        case 0x3A6: return const_cast<char *>("STATUS");
        case 0x285: return const_cast<char *>("PACT");
        case 0x284: return const_cast<char *>("TREATY");
        case 0x283: return const_cast<char *>("TRUCE");
        case 0x287: return const_cast<char *>("WAR");
        case 0x19: return const_cast<char *>("NONE");
        case 0x3A7: return const_cast<char *>("SPY");
        case 0x3A8: return const_cast<char *>("CLEAR");
        case 0x185: return const_cast<char *>("INFILTRATED");
        case 0x24: return const_cast<char *>("ENERGY");
        case 0xDF: return const_cast<char *>("MIGHT");
        default: assert(false && "unexpected label id"); return label_text;
    }
}
struct Strings {
    char *get(int address) {
        for (int i = 0; i < 7; ++i) if (address == 0x2000 + i) return might_text[i];
        assert(false && "unexpected string pointer"); return nullptr;
    }
} StringTable;
int spying(int faction) {
    spy_calls.push_back(faction);
    const int index = static_cast<int>(spy_calls.size()) - 1;
    return index < static_cast<int>(spy_values.size()) ? spy_values[index] : 0;
}
void say_special(char *output, char *text, int faction) {
    assert(faction >= 0 && faction < 8);
    std::strcat(output, "RANK-"); std::strcat(output, text);
}
char *_itoa(int value, char *output, int radix) {
    assert(radix == 10); std::sprintf(output, "%d", value); return output;
}
char *CharUpperA(char *text) {
    for (char *p = text; *p; ++p) *p = static_cast<char>(std::toupper(static_cast<unsigned char>(*p)));
    return text;
}
struct DiploPop : Buffer {
    RECT rect1_{0, 10, 200, 90};
    Font font2_{8};
    Font font3_{12};
    int counterpart_faction_ = 2;
    void draw_info();
};
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
        result = subprocess.run([str(exe)], capture_output=True, text=True)
        assert result.returncode == 0, result.stdout + result.stderr


def test_actual_body_draws_two_panels_and_repeats_native_spying_query() -> None:
    run_case(r'''
        for (int faction = 0; faction < 8; ++faction) {
            std::sprintf(Players[faction].name_leader, "leader-%d", faction);
            PlayersData[faction].energy_reserves = 500 + faction;
            PlayersData[faction].hurry_cost_total = 20 + faction;
            PlayersData[faction].ranking = faction == 2 ? 9 : 3;
            FactionsArt[faction].diplomacy_logo.id = faction;
            FactionsArt[faction].diplomacy_logo.w = 20 + faction;
            FactionsArt[faction].diplomacy_logo.h = 10 + faction;
            std::sprintf(might_text[faction < 7 ? faction : 6], "might-%d", faction < 7 ? faction : 6);
            Might[faction < 7 ? faction : 6].adj_start = reinterpret_cast<char *>(static_cast<intptr_t>(0x2000 + (faction < 7 ? faction : 6)));
        }
        ConsoleGlobal.faction_id_ = 3;
        PlayersData[3].diplo_treaties[2] = DTREATY_VENDETTA;
        spy_values = {0, 1, 0};
        DiploPop popup;
        popup.draw_info();
        assert(popup.wrap_width == 15 && MainInterfaceGlobal.wrap_width == -1);
        assert(MainInterfaceGlobal.frames.size() == 2);
        assert(MainInterfaceGlobal.frames[0].left == 0 && MainInterfaceGlobal.frames[0].right == 98);
        assert(MainInterfaceGlobal.frames[1].left == 101 && MainInterfaceGlobal.frames[1].right == 199);
        assert(sprite_draws.size() == 2);
        assert(sprite_draws[0].id == 2 && sprite_draws[0].mono == 0 && sprite_draws[0].x == 3 && sprite_draws[0].y == 28);
        assert(sprite_draws[1].id == 3 && sprite_draws[1].mono == 0);
        assert((MainInterfaceGlobal.colors == std::vector<int>{12, 160, 13, 160}));
        assert(MainInterfaceGlobal.fonts.size() == 4);
        assert(MainInterfaceGlobal.fonts[0] == &popup.font3_ && MainInterfaceGlobal.fonts[1] == &popup.font2_);
        assert(MainInterfaceGlobal.wrapped[0] == "LEADER-2");
        assert(MainInterfaceGlobal.wrapped[1] == "STATUS: WAR");
        assert(MainInterfaceGlobal.wrapped[2] == "SPY: CLEAR");
        assert(MainInterfaceGlobal.wrapped[3] == "LEADER-3");
        assert(MainInterfaceGlobal.wrapped[4] == "SPY: CLEAR");
        assert(spy_calls == std::vector<int>({2, 2, 3}));
        assert(MainInterfaceGlobal.written.size() == 6);
        assert(MainInterfaceGlobal.written[0] == "ENERGY: 480");
        assert(MainInterfaceGlobal.written[1] == "MIGHT: ");
        assert(MainInterfaceGlobal.written[2] == "RANK-MIGHT-0");
        assert(MainInterfaceGlobal.written[3] == "ENERGY: 480");
        assert(MainInterfaceGlobal.written[4] == "MIGHT: ");
        assert(MainInterfaceGlobal.written[5] == "RANK-MIGHT-4");
        assert(popup.events.front() == "width");
        assert(MainInterfaceGlobal.events.front() == "frame");
    ''')


def test_800_width_uses_centered_monochrome_portraits_and_clamps_might() -> None:
    run_case(r'''
        for (int faction = 0; faction < 8; ++faction) {
            std::strcpy(Players[faction].name_leader, "LEADER");
            PlayersData[faction].ranking = faction == 2 ? 99 : 0;
            FactionsArt[faction].diplomacy_logo.id = faction;
            FactionsArt[faction].diplomacy_logo.w = 20;
            FactionsArt[faction].diplomacy_logo.h = 10;
        }
        for (int i = 0; i < 7; ++i) {
            std::sprintf(might_text[i], "might-%d", i);
            Might[i].adj_start = reinterpret_cast<char *>(static_cast<intptr_t>(0x2000 + i));
        }
        ConsoleGlobal.faction_id_ = 3;
        WinScreenWidth = 800;
        spy_values = {0, 0};
        DiploPop popup;
        popup.draw_info();
        assert(sprite_draws.size() == 2);
        assert(sprite_draws[0].mono == 1 && sprite_draws[0].color == 0x41);
        assert(sprite_draws[0].x == 39 && sprite_draws[0].y == 45);
        assert(MainInterfaceGlobal.wrapped[0] == "LEADER");
        assert(MainInterfaceGlobal.written[1] == "RANK-MIGHT-0");
        assert(MainInterfaceGlobal.written.back() == "RANK-MIGHT-6");
    ''')
