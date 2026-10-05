#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#define __cdecl

enum { MaxTechnologyNum = 89 };
struct RulesTechnology {
    int growth_value;
    int tech_value;
    int wealth_value;
};
struct Sprite { int identity; };
struct SpriteList { int entry_count_; };
struct SpriteBox {
    SpriteList list_;
    int sprite_calls;
    Sprite *last_sprite;
    SpriteBox() : sprite_calls(0), last_sprite(0) { list_.entry_count_ = 0; }
    int sprite(Sprite *picture, int, int) {
        ++sprite_calls;
        last_sprite = picture;
        return 0;
    }
};
struct Dialogs : public SpriteBox {};
struct PopupStart {
    std::string file;
    std::string label;
    int value;
    std::string override_text;
    int flags;
    bool has_owner;
};
struct Popup {
    Dialogs dialogs_;
    Sprite *caller_picture;
    std::vector<PopupStart> starts;
    int exec_calls;
    int exec_flags;
    int exec_result;
    Popup() : caller_picture(0), exec_calls(0), exec_flags(0), exec_result(0) {}
    Dialogs &dialogs() { return dialogs_; }
    void set_caller_picture(Sprite *picture) { caller_picture = picture; }
    int start(char *file, const char *label, int value, char *override_text,
              int flags, void *owner) {
        PopupStart event;
        event.file = file;
        event.label = label;
        event.value = value;
        event.override_text = override_text == 0 ? "" : override_text;
        event.flags = flags;
        event.has_owner = owner != 0;
        starts.push_back(event);
        return 0;
    }
    int exec(int flags, int (*)()) {
        ++exec_calls;
        exec_flags = flags;
        return exec_result;
    }
};

RulesTechnology Technology[MaxTechnologyNum] = {};
Sprite g_TECH_ICON_SPRITES[MaxTechnologyNum] = {};
char StringTemp[1032] = {};
char *TechWizardPopupFile = const_cast<char *>("WIZARD");
static int parser_calls = 0;
static int parser_ids[4] = {};
static char parser_text[4][128] = {};
static int random_calls = 0;
static int random_result = 1;
static int itoa_calls = 0;

void say_tech(char *output, int tech, int) {
    std::sprintf(output, "TECH%d", tech);
}
int parse_says(int slot, char *text, int, int) {
    parser_ids[parser_calls] = slot;
    std::strcpy(parser_text[parser_calls], text);
    ++parser_calls;
    return 0;
}
unsigned int random(int, int) {
    ++random_calls;
    return static_cast<unsigned int>(random_result);
}
char *_itoa(int value, char *buffer, int) {
    ++itoa_calls;
    std::sprintf(buffer, "%d", value);
    return buffer;
}

// Actual-source controls replace this include with the extracted product body.
// PRODUCT_BODY_INSERTED_BY_TEST

static void reset(Popup &first, Popup &second) {
    std::memset(Technology, 0, sizeof Technology);
    parser_calls = 0;
    random_calls = 0;
    random_result = 1;
    itoa_calls = 0;
    StringTemp[0] = '\0';
    first = Popup();
    second = Popup();
}

static bool check_common(const Popup &first, const Popup &second,
                         const char *roll_text, const char *result_text) {
    return first.starts.size() == 2 && second.starts.size() == 2 &&
           first.starts[0].file == "WIZARD" &&
           first.starts[0].label == "TECHMIL" &&
           first.starts[0].flags == 0x100 &&
           first.starts[1].label == roll_text &&
           first.starts[1].flags == 0x80 &&
           second.starts[0].label == "TECHINF" &&
           second.starts[0].flags == 0x100 &&
           second.starts[1].label == result_text &&
           second.starts[1].flags == 0x80 &&
           first.exec_calls == 1 && second.exec_calls == 1;
}

int main(int argc, char **argv) {
    const char *mode = argc > 1 ? argv[1] : "rank2";
    Popup first;
    Popup second;
    reset(first, second);

    if (std::strcmp(mode, "negative") == 0 ||
        std::strcmp(mode, "second-negative") == 0) {
        const bool second_negative = std::strcmp(mode, "second-negative") == 0;
        tech_wizard_unk1(&first, &second, second_negative ? 2 : -1,
                        second_negative ? -1 : 4, 0);
        return parser_calls == 0 && random_calls == 0 &&
                       first.starts.empty() && second.starts.empty()
                   ? 0 : 1;
    }

    int first_tech = 2;
    int second_tech = 3;
    int force_roll = 0;
    const char *expected_result = "INF2";
    Technology[3].growth_value = 1;
    Technology[3].tech_value = 1;
    Technology[3].wealth_value = 1;
    if (std::strcmp(mode, "rank1") == 0) {
        Technology[3].growth_value = 1;
        Technology[3].tech_value = 2;
        Technology[3].wealth_value = 5;
        expected_result = "INF1";
    } else if (std::strcmp(mode, "rank0") == 0) {
        Technology[3].growth_value = 5;
        Technology[3].tech_value = 1;
        Technology[3].wealth_value = 2;
        expected_result = "INF0";
    } else if (std::strcmp(mode, "growth-ahead-tech") == 0) {
        Technology[3].growth_value = 5;
        Technology[3].tech_value = 2;
        Technology[3].wealth_value = 1;
        expected_result = "INF0";
    } else if (std::strcmp(mode, "equal-growth-wealth") == 0) {
        Technology[3].growth_value = 5;
        Technology[3].tech_value = 2;
        Technology[3].wealth_value = 5;
        expected_result = "INF2";
    } else if (std::strcmp(mode, "forced") == 0) {
        force_roll = 1;
        expected_result = "INF3";
    } else if (std::strcmp(mode, "exec-results") == 0) {
        first.exec_result = 7;
        second.exec_result = 23;
    } else if (std::strcmp(mode, "sum-zero") == 0) {
        Technology[3].growth_value = -1;
        Technology[3].tech_value = 0;
        Technology[3].wealth_value = 0;
        expected_result = "INF9";
    } else if (std::strcmp(mode, "same-tech") == 0) {
        second_tech = first_tech;
        expected_result = "INF9";
    } else if (std::strcmp(mode, "upper-bound") == 0) {
        second_tech = MaxTechnologyNum;
        expected_result = "INF2";
    } else if (std::strcmp(mode, "last-tech") == 0) {
        second_tech = MaxTechnologyNum - 1;
        expected_result = "INF9";
    } else if (std::strcmp(mode, "first-high") == 0) {
        first_tech = MaxTechnologyNum;
        expected_result = "INF2";
    } else if (std::strcmp(mode, "occupied-sprite-list") == 0) {
        first.dialogs_.SpriteBox::list_.entry_count_ = 1;
        second.dialogs_.SpriteBox::list_.entry_count_ = 1;
    }

    if (first_tech < MaxTechnologyNum)
        g_TECH_ICON_SPRITES[first_tech].identity = first_tech;
    if (second_tech < MaxTechnologyNum)
        g_TECH_ICON_SPRITES[second_tech].identity = second_tech;
    tech_wizard_unk1(&first, &second, first_tech, second_tech, force_roll);
    const char *roll_text = force_roll != 0 ? "MIL3" : "MIL1";
    char expected_first_tech[32];
    char expected_second_tech[32];
    std::sprintf(expected_first_tech, "TECH%d", first_tech);
    std::sprintf(expected_second_tech, "TECH%d", second_tech);
    if (parser_calls != 2 || parser_ids[0] != 0 ||
        parser_ids[1] != 1 || std::strcmp(parser_text[0], expected_first_tech) != 0 ||
        std::strcmp(parser_text[1], expected_second_tech) != 0 ||
        !check_common(first, second, roll_text, expected_result))
        return 2;
    if (random_calls != (force_roll != 0 ? 0 : 1) || itoa_calls != 2)
        return 3;

    if (first_tech < MaxTechnologyNum) {
        Sprite *const expected_first = &g_TECH_ICON_SPRITES[first_tech];
        if (first.dialogs_.sprite_calls != 1 ||
            first.dialogs_.last_sprite != expected_first)
            return 4;
        const bool occupied = std::strcmp(mode, "occupied-sprite-list") == 0;
        if (first.caller_picture != (occupied ? 0 : expected_first)) return 5;
    } else if (first.dialogs_.sprite_calls != 0 || first.caller_picture != 0) {
        return 5;
    }
    if (second_tech < MaxTechnologyNum) {
        Sprite *const expected_second = &g_TECH_ICON_SPRITES[second_tech];
        if (second.dialogs_.sprite_calls != 1 ||
            second.dialogs_.last_sprite != expected_second)
            return 6;
        const bool occupied = std::strcmp(mode, "occupied-sprite-list") == 0;
        if (second.caller_picture != (occupied ? 0 : expected_second)) return 7;
    } else if (second.dialogs_.sprite_calls != 0) {
        return 8;
    }
    return std::strcmp(StringTemp, expected_result) == 0 ? 0 : 9;
}
