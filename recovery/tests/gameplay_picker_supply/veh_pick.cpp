// Actual-body control for the v4 proposal. Minimal typed mocks only: proves
// callback freshness and explicit/automatic close ordering, not product ABI,
// class layout, exhaustive native paths, or game behavior.
#include <cassert>
#include <cstring>
#define __cdecl
typedef unsigned char uint8_t;

int lifecycle[32], lifecycle_count;
void mark(int value) { lifecycle[lifecycle_count++] = value; }
struct Sprite {
    Sprite() { mark(2); }
    int create_blank(int w, int h, unsigned char) {
        assert(w == 100 && h == 75); mark(3); ++created; return 1;
    }
    void close() { mark(4); ++closed; }
    ~Sprite() { close(); mark(5); ++destroyed; }
    static int created, closed, destroyed;
};
int Sprite::created = 0; int Sprite::closed = 0; int Sprite::destroyed = 0;
struct GraphicWin {};
struct Veh { short proto_id, next_veh_id_stack, prev_veh_id_stack; };
struct VehPrototype { char veh_name[32]; };
Veh Vehs[8]; VehPrototype VehPrototypes[8]; char StringTemp[1032];
char PopupStartCaption[] = "SCRIPT"; const unsigned char WinFillColour = 7;
int PopupModalState, exec_result = 5, exec_calls, close_calls, destruct_calls;
int click_installed, double_click_installed, item_count, morale_count;
int listed_ids[8]; char listed_text[8][64];
int mutate_proto_after_morale = -1;
int mutate_next_after_sprite = -1;
typedef void (__cdecl *DrawCallback)(Sprite *, GraphicWin *, int, int, int);
typedef int (__cdecl *ClickCallback)(int);
int __cdecl veh_sprite_click(int);
int __cdecl veh_sprite_double_click(int);
struct SpriteBox {
    DrawCallback redraw;
    ClickCallback click, double_click;
    SpriteBox() : redraw(0), click(0), double_click(0) {}
    int sprite(Sprite *, char *text, int id) {
        listed_ids[item_count] = id;
        std::strcpy(listed_text[item_count++], text);
        if (id == 0 && mutate_next_after_sprite >= 0)
            Vehs[id].next_veh_id_stack = static_cast<short>(mutate_next_after_sprite);
        return 1;
    }
    void set_redraw_callback(DrawCallback fn) { redraw = fn; }
};
struct Dialogs : SpriteBox {};
struct Popup {
    Dialogs box;
    DrawCallback redraw;
    ClickCallback click, double_click;
    Popup() : redraw(0), click(0), double_click(0) { mark(1); }
    ~Popup() { close(); mark(7); ++destruct_calls; }
    Dialogs &dialogs() { return box; }
    int start(char *, const char *, int value, char *text, int, void *) {
        assert(value == -1 && text == 0); return 0;
    }
    void set_redraw_callback(DrawCallback fn) { redraw = fn; }
    void set_sprite_click_callback(ClickCallback fn) {
        click = fn; click_installed = fn == veh_sprite_click;
    }
    void set_sprite_double_click_callback(ClickCallback fn) {
        double_click = fn; double_click_installed = fn == veh_sprite_double_click;
    }
    int exec(int mode, int (__cdecl *callback)()) {
        assert(mode == 0 && callback == 0 && click == veh_sprite_click &&
               double_click == veh_sprite_double_click);
        assert(PopupModalState == 6); ++exec_calls; return exec_result;
    }
    void close() { mark(6); ++close_calls; }
};
void __cdecl say_morale(int id, int side) {
    assert(side == 0); ++morale_count;
    if (id == 0 && mutate_proto_after_morale >= 0)
        Vehs[id].proto_id = static_cast<short>(mutate_proto_after_morale);
}
void __cdecl veh_sprite_draw(Sprite *, GraphicWin *, int, int, int) {}
int __cdecl veh_sprite_click(int) { return 1; }
int __cdecl veh_sprite_double_click(int) { return 1; }
// ACTUAL_SOURCE_BODY

static void reset() {
    std::memset(Vehs, 0, sizeof(Vehs));
    std::memset(VehPrototypes, 0, sizeof(VehPrototypes));
    Sprite::created = Sprite::closed = Sprite::destroyed = 0;
    PopupModalState = exec_calls = close_calls = destruct_calls = 0;
    click_installed = double_click_installed = item_count = morale_count = 0;
    mutate_proto_after_morale = mutate_next_after_sprite = -1;
    exec_result = 5; lifecycle_count = 0;
}
int main() {
    reset();
    assert(veh_pick(-1, "PICKUNIT", 1) == -1);
    assert(exec_calls == 0 && Sprite::created == 1 && Sprite::closed == 2);
    assert(close_calls == 2 && destruct_calls == 1);
    assert(lifecycle_count == 9 && lifecycle[0] == 1 && lifecycle[1] == 2 &&
           lifecycle[2] == 3 && lifecycle[3] == 4 && lifecycle[4] == 6 &&
           lifecycle[5] == 4 && lifecycle[6] == 5 && lifecycle[7] == 6 &&
           lifecycle[8] == 7);

    reset();
    std::strcpy(VehPrototypes[0].veh_name, "Scout");
    std::strcpy(VehPrototypes[1].veh_name, "Colony");
    std::strcpy(VehPrototypes[2].veh_name, "Former");
    Vehs[0].prev_veh_id_stack = -1; Vehs[0].next_veh_id_stack = 1;
    Vehs[1].prev_veh_id_stack = 0;  Vehs[1].next_veh_id_stack = 2;
    Vehs[2].prev_veh_id_stack = 1;  Vehs[2].next_veh_id_stack = -1;
    Vehs[0].proto_id = 0; Vehs[1].proto_id = 1; Vehs[2].proto_id = 2;
    // Native reloads the prototype after say_morale and the next link after
    // SpriteBox::sprite; these mutations detect stale pre-callback snapshots.
    mutate_proto_after_morale = 1; mutate_next_after_sprite = 2;
    assert(veh_pick(1, "PICKUNIT", 0) == 5);
    assert(item_count == 2 && listed_ids[0] == 0 && listed_ids[1] == 2);
    assert(std::strcmp(listed_text[0], " Colony") == 0);
    assert(exec_calls == 1 && PopupModalState == 0);
    assert(click_installed && double_click_installed);
    assert(close_calls == 2 && Sprite::closed == 2 && destruct_calls == 1);
    // Success closes Popup before the local Sprite; then real automatic
    // destructors close Sprite and Popup in reverse declaration order.
    assert(lifecycle_count == 9 && lifecycle[0] == 1 && lifecycle[1] == 2 &&
           lifecycle[2] == 3 && lifecycle[3] == 6 && lifecycle[4] == 4 &&
           lifecycle[5] == 4 && lifecycle[6] == 5 && lifecycle[7] == 6 &&
           lifecycle[8] == 7);
}
