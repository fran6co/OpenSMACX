#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define nullptr 0

static const char *DefaultFontFace = "fixture";

enum { MaxPlayerNum = 8, MaxSecretProjectNum = 64,
       RFLAG_ALIEN = 0x80, DTREATY_PACT = 1, DTREATY_TREATY = 2 };
struct RECT { int left, top, right, bottom; };
struct BoxSpriteParams { int unused; };
static BoxSpriteParams InterfaceBoxStyles[7];
static int TextColourTable[195];
static int ChatItemIdTable[8];
static int GenderDefault, PluralityDefault;
static char StringTemp[1024];
static int writes;
static char written[32][256];

struct Font {
    int init(const char *, int, int) { return 0; }
    int line_advance() const { return 14; }
};

struct Buffer {
    RECT rect1_;
    int set_clip(RECT *) { return 0; }
    int box_sprite(RECT *, BoxSpriteParams *) { return 0; }
    int set_font(Font *, Font *, Font *, Font *) { return 0; }
    int set_text_color(int, int, int, int) { return 0; }
    int write_cent_l(char *text, RECT *, int length) {
        return record(text, length);
    }
    int write_l(char *text, int, int, int length) {
        return record(text, length);
    }
    int write_l(char *text, RECT *, int length) {
        return record(text, length);
    }
    void soft_update(RECT *) {}
    void update(RECT *, void *) {}
    static int record(const char *text, int length) {
        assert(writes < 32);
        const int count = length < 255 ? length : 255;
        memcpy(written[writes], text, count);
        written[writes][count] = 0;
        return ++writes * 10;
    }
};

struct Player { int rule_flags, noun_gender, is_noun_plural; char noun_faction[32]; };
struct PlayerData {
    unsigned int flags;
    int diplo_treaties[8];
    int sanction_turns;
};
// Use the source's integer field widths without importing project headers.
struct Base { int faction_id_current; unsigned char facilities_built[9]; };
struct BaseSecretProject { int home_base[MaxSecretProjectNum]; };
struct GameDataStateFields {
    BaseSecretProject secret_project_;
    int base_current_count_;
};
struct GameDataStateWindow { GameDataStateFields fields_; };
static GameDataStateWindow GameDataStateStorage;
static Player Players[MaxPlayerNum];
static PlayerData PlayersData[MaxPlayerNum];
static Base Bases[16];
static Base *BaseCurrent;
static int BaseIDCurrentSelected;
static int BaseCommerceReceipts[7];
static int BaseCommercePartnerReceipts[7];
static Buffer MainInterfaceGlobal;

static char *label_get(int id) {
    static char labels[1200][16];
    snprintf(labels[id], sizeof(labels[id]), "L%d", id);
    return labels[id];
}
static char *CharUpperA(char *text) {
    for (char *p = text; *p; ++p) if (*p >= 'a' && *p <= 'z') *p -= 'a' - 'A';
    return text;
}
static char *_itoa(int value, char *out, int) {
    sprintf(out, "%d", value);
    return out;
}
struct BaseWin : Buffer {
    RECT commerce_outer_rect_, commerce_title_rect_, commerce_report_rect_;
    int focusedBaseID_;
    Font font1_;
    void draw_commerce(int update_mode);
};

/*__DRAW_COMMERCE_BODY__*/

int main(int argc, char **argv) {
    const int mode = argc > 1 ? atoi(argv[1]) : 0;
    BaseWin window;
    memset(&window, 0, sizeof(window));
    writes = 0;
    memset(Players, 0, sizeof(Players));
    memset(PlayersData, 0, sizeof(PlayersData));
    memset(Bases, 0, sizeof(Bases));
    memset(BaseCommerceReceipts, 0, sizeof(BaseCommerceReceipts));
    memset(BaseCommercePartnerReceipts, 0, sizeof(BaseCommercePartnerReceipts));
    memset(written, 0, sizeof(written));
    BaseCurrent = &Bases[0];
    GameDataStateStorage.fields_.base_current_count_ = 1;
    BaseIDCurrentSelected = 0;
    window.focusedBaseID_ = 0;
    Bases[0].faction_id_current = 1;
    Players[1].noun_gender = 1;
    Players[1].noun_faction[0] = 'o'; Players[1].noun_faction[1] = 'w';
    Players[1].noun_faction[2] = 'n'; Players[1].noun_faction[3] = 0;
    Players[2].noun_gender = 2;
    Players[2].is_noun_plural = 1;
    strcpy(Players[2].noun_faction, "PartnerTwo");
    PlayersData[1].diplo_treaties[2] = DTREATY_PACT;
    PlayersData[2].diplo_treaties[1] = DTREATY_PACT;
    PlayersData[1].sanction_turns = mode == 1 ? 0 : 3;
    PlayersData[2].sanction_turns = mode == 1 ? 0 : 7;
    BaseCommerceReceipts[1] = 5;
    BaseCommercePartnerReceipts[1] = mode == 2 ? 5 : 9;

    window.commerce_outer_rect_.right = 100;
    window.commerce_title_rect_.right = 100;
    window.commerce_report_rect_.right = 100;
    window.commerce_report_rect_.bottom = 120;
    window.draw_commerce(0);

    int partner_row = -1;
    int sanction_line = -1;
    int receipt_delta_line = -1;
    for (int i = 0; i < writes; ++i) {
        if (strstr(written[i], "PARTNERTWO")) partner_row = i;
        if (strstr(written[i], "L450") && strstr(written[i], "7")) sanction_line = i;
        if (strstr(written[i], "L648") && strstr(written[i], "9")) receipt_delta_line = i;
    }
    assert(partner_row >= 0);
    if (mode != 1) assert(sanction_line > partner_row);
    if (mode == 2) {
        assert(receipt_delta_line == -1);
    } else if (mode == 1) {
        assert(receipt_delta_line > partner_row);
        assert(receipt_delta_line > 0);
    } else {
        assert(receipt_delta_line > sanction_line);
    }
    assert(GenderDefault == 2 && PluralityDefault == 1);
    return 0;
}
