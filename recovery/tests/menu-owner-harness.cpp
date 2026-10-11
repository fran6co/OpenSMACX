#include <cassert>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>
#include <cctype>
typedef unsigned int uint32_t;

struct RECT { int left, top, right, bottom; };
struct POINT { int x, y; };
typedef void *HWND;
HWND HandleMain = 0;
class Font;
static void (*AfterResize)() = 0;

struct Sound {
    Sound() : active_(0), plays_(0) {}
    int is_playing() const { return active_; }
    int play() { active_ = 1; ++plays_; return 0; }
    int active_;
    int plays_;
};

class Win {
public:
    explicit Win() : fill_color(0), text_color(0), writes(0), lines(0), redraws(0), hides(0), flushes(0), modal_releases(0), key_calls(0), last_key_input(-1), last_key(-1), win_parent_(0) { set_rect(0,0,200,100); }
    virtual ~Win() {}
    virtual void redraw() { ++redraws; }
    virtual void hide() { ++hides; }
    virtual int resize(int w, int h, int) { rect_.right = rect_.left + w; rect_.bottom = rect_.top + h; if (AfterResize) AfterResize(); return 0; }
    virtual int vslot_07() { return 0; }
    virtual int vslot_23() { return 0; }
    virtual int on_key_click(int a, int b) { ++key_calls; last_key_input=a; last_key=b; return 0; }
    virtual void release_modal() { ++modal_releases; }
    RECT const &outer_rect() const { return rect_; }
    void set_rect(int l, int t, int r, int b) { rect_.left=l; rect_.top=t; rect_.right=r; rect_.bottom=b; }
    Sound *event_window() { return &event_sound_; }
    void set_parent(Win *p) { win_parent_ = p; }
    void client_to_screen(int *, int *) {}
    int fill_color, text_color, writes, lines;
    void fill(int color) { fill_color = color; }
    void fill(int, int, int, int, int color) { fill_color = color; selected_fill_text_color = text_color; }
    int set_font(Font *, Font *, Font *, Font *) { return 0; }
    void set_text_color(int color, int, int, int) { text_color = color; }
    RECT clip_rect() const { return rect_; }
    int text_width(char *s);
    int text_width(char *s, int n);
    int text_height();
    int write_l(char *, RECT *, int) { ++writes; return 0; }
    int write_right_l(char *, RECT *, int) { ++writes; return 0; }
    int foreground_color() const { return text_color; }
    void hline(int, int, int, int) { ++lines; }
    void move(int x, int y) { rect_.right += x - rect_.left; rect_.bottom += y - rect_.top; rect_.left = x; rect_.top = y; }
    void flush_input() { ++flushes; }
    int redraws, hides, flushes, modal_releases, key_calls, last_key_input, last_key;
    int selected_fill_text_color;
    Sound event_sound_;
protected:
    Win *win_parent_;
    RECT rect_;
};

class Buffer {
public:
    Buffer() : markup_enabled(1) {}
    void set_markup_enabled(int enabled) { markup_enabled = enabled; last_markup_enabled = enabled; }
    int markup_enabled;
    static int last_markup_enabled;
};
int Buffer::last_markup_enabled = 1;

class GraphicWin : public Win, public Buffer {
public:
    GraphicWin() : init_result(0), init_calls(0), show_calls(0), last_flags(0),
        last_x(0), last_y(0), last_width(0), last_height(0) {}
    void show(int) { ++show_calls; }
    int init(int x, int y, int width, int height, char *, int flags, Win *parent,
             class Menu *, void *) {
        ++init_calls;
        last_x = x; last_y = y; last_width = width; last_height = height;
        last_flags = flags;
        if (init_result != 0) return init_result;
        set_rect(x, y, x + width, y + height);
        set_parent(parent);
        return 0;
    }
    int init_result, init_calls, show_calls, last_flags;
    int last_x, last_y, last_width, last_height;
};

class Font {
public:
    int height_;
    int internal_leading_;
    explicit Font(int h = 8) : height_(h), internal_leading_(1) {}
    int internal_leading() const { return internal_leading_; }
    int width(char *s) { return static_cast<int>(std::strlen(s)) * 5; }
    int width(char *s, int n) { (void)s; return n * 5; }
};
Font *MenuDefaultFont = 0;
Font *FontDefault = 0;
int Win::text_width(char *s) { return FontDefault->width(s); }
int Win::text_width(char *s, int n) { return FontDefault->width(s,n); }
int Win::text_height() { return FontDefault->height_; }

class Spot {
public:
    struct Item { RECT rect; int pos; int type; } items[15];
    int used;
    Spot() : used(0) {}
    void init(int) { used = 0; }
    int add(int pos, int type, int left, int top, int length, int width) {
        Item &i = items[used++]; i.rect.left = left; i.rect.top = top;
        i.rect.right = left + length; i.rect.bottom = top + width; i.pos = pos; i.type = type; return 0;
    }
    int check(int x, int y, int *index, int) {
        for (int n=0; n<used; ++n) if (x>=items[n].rect.left && x<items[n].rect.right && y>=items[n].rect.top && y<items[n].rect.bottom) { *index=items[n].pos; return n; }
        return -1;
    }
    void get_rect_at(int n, RECT *r) const { *r = items[n].rect; }
};

class PullDown : public Win {
public:
    struct Item { int id; uint32_t flags; char *mnemonic; } items_[64];
    int selected_, shows, inits, item_count_;
    PullDown() : selected_(-1), shows(0), inits(0), item_count_(0), update_old(0), update_new(0), updates(0) {}
    void init(class Menu *) { ++inits; }
    void show(int, int, int) { ++shows; }
    int get_selected() { return selected_; }
    int on_key_click(int key_input, int key);
    int get_selected_count() const { return item_count_; }
    uint32_t flags_[64];
    char *mnemonics_[64];
    void pulldown_update(int old_value, int new_value) { update_old = old_value; update_new = new_value; ++updates; }
    int update_old, update_new, updates;
};

struct MenuEntry { int id; char *text; unsigned char flags; unsigned char pad[3]; char *mnemonic; PullDown *pull_down; };
typedef void (*MenuProc)(int);
class Menu : public GraphicWin {
public:
    MenuProc proc_;
    int count_;
    unsigned int field_A1C_, field_A20_;
    unsigned int field_A24_;
    unsigned int field_A28_;
    Spot spot_;
    MenuEntry entries_[15];
    int pxwidth, pxheight;
    Menu() : proc_(0), count_(0), field_A1C_(0), field_A20_(0), field_A24_(0xffffffffu), field_A28_(0), pxwidth(200), pxheight(100) { std::memset(entries_, 0, sizeof(entries_)); for (int i=0;i<15;++i) entries_[i].id=-1; }
    int pixel_width() const { return pxwidth; }
    int pixel_height() const { return pxheight; }
    int text_width(char *s) { return FontDefault->width(s); }
    int text_width(char *s, int n) { return FontDefault->width(s,n); }
    void on_adjust_pulldown_pos(int *, int *) {}
    void hide_menus() { if (field_A24_ != 0xffffffffu) entries_[field_A24_].pull_down->hide(); field_A24_=0xffffffffu; }
    int add_menu(int, char *, int);
    int UNK10(int, char *);
    int init(Win *);
    void compute();
    void adjust();
    void on_left_down(int,int);
    void mouse_move(int,int);
    int on_key_click(int,int);
    void on_redraw();
};

static int allocation_fail_after = -1;
void *mem_get(std::size_t n) { if (allocation_fail_after==0) return 0; if (allocation_fail_after>0) --allocation_fail_after; return std::malloc(n); }
static int last_task_count = 0, draws = 0, last_callback_id = -999;
void do_all_tasks(Win *) { ++last_task_count; }
void do_all_draws() { ++draws; }
void ScreenToClient(HWND, POINT *) {}
void ClientToScreen(HWND, POINT *) {}
void callback(int id) { last_callback_id=id; }
int MenuSelection = -1;
int MenuDefaultColor1 = 247, MenuDefaultColor2 = 255, MenuDefaultColor3 = 7;
int MenuDefaultSelectionColor = 4, MenuDefaultSelectionTextColor = 6;
int MenuPullDownInputOrigin = 0;
int MenuLastMouseX = 0;
int MenuLastMouseY = 0;
char MenuEntryPadding[] = "  ";
char MenuComputeLeftSpace[] = " ";
char MenuComputeRightSpace[] = " ";
char MenuLeftDownMargin[] = " ";
char MenuMouseMoveMargin[] = " ";
char MenuMnemonicMargin[] = " ";
char MenuRedrawMargin[] = " ";

#include "GENERATED_MENU_SOURCE"

static Menu *ResizeMenu = 0;
static Win *ReplacementParent = 0;
static Font *ReplacementFont = 0;
static void replace_resize_owners() {
    AfterResize = 0;
    ResizeMenu->set_parent(ReplacementParent);
    MenuDefaultFont = ReplacementFont;
}

int main() {
    Font default_font(8); FontDefault=&default_font;
    Menu m;
    Win parent; parent.set_rect(4, 7, 204, 80);
    m.field_A28_ = 5;
    assert(m.init(0) == 3 && m.field_A28_ == 5);
    FontDefault = 0;
    assert(m.init(&parent) == 7 && m.field_A28_ == 0);
    assert(m.init_calls == 0);
    FontDefault = &default_font;
    assert(m.init(&parent) == 0);
    assert(m.init_calls == 1 && m.last_flags == 0x8020);
    assert(m.last_x == 4 && m.last_y == -9 && m.last_width == 200 && m.last_height == 16);
    assert(m.markup_enabled == 0 && Buffer::last_markup_enabled == 0 && m.show_calls == 1);
    assert(m.outer_rect().left == 4 && m.outer_rect().top == -9);
    m.init_result = 9;
    assert(m.init(&parent) == 9 && m.show_calls == 1);
    m.init_result = 0;

    m.proc_ = callback;
    char open_text[] = "&Open";
    assert(m.add_menu(10, open_text, 0) == 0);
    assert(m.count_ == 1 && m.entries_[0].id == 10);
    assert(std::strcmp(m.entries_[0].text, "Open  ") == 0);
    assert(m.entries_[0].mnemonic == m.entries_[0].text);
    assert(m.entries_[0].flags == 1 && m.entries_[0].pull_down->inits == 1);
    char again_text[] = "again";
    assert(m.add_menu(10, again_text, 0) == 1);
    assert(m.add_menu(11, 0, 0) == 3);

    char save_text[] = "Save && Exit";
    assert(m.UNK10(10, save_text) == 0);
    assert(std::strcmp(m.entries_[0].text, "Save & Exit  ") == 0);
    assert(m.entries_[0].mnemonic == 0);
    char x_text[] = "x";
    assert(m.UNK10(999, x_text) == 11);
    char edit_text[] = "&Edit";
    char view_text[] = "View";
    assert(m.add_menu(20, edit_text, 1) == 0);
    assert(m.add_menu(21, view_text, 0) == 0);

    m.compute();
    assert(m.spot_.used == 3);
    assert(m.spot_.items[0].rect.left == 5);
    assert(m.spot_.items[1].rect.left == 165);
    assert(m.spot_.items[2].rect.left == 70);
    m.set_parent(&parent);
    m.adjust();
    assert(m.outer_rect().left == 4 && m.outer_rect().top == -9);

    Win replacement_parent; replacement_parent.set_rect(99, 30, 399, 90);
    Font replacement_font(12);
    ResizeMenu = &m; ReplacementParent = &replacement_parent;
    ReplacementFont = &replacement_font; AfterResize = replace_resize_owners;
    m.adjust();
    assert(m.outer_rect().left == 99 && m.outer_rect().top == 6);
    assert(MenuDefaultFont == &replacement_font && m.spot_.items[0].rect.bottom == 24);
    MenuDefaultFont = &default_font; m.set_parent(&parent); m.adjust();

    m.on_left_down(166, 2);
    assert(m.field_A24_ == 1 && m.entries_[1].pull_down->shows == 1);
    m.mouse_move(10, 2);
    assert(m.field_A24_ == 0);
    assert(m.on_key_click(0, 'e') == 1);
    assert(m.field_A24_ == 1);

    m.field_A24_ = 0;
    m.entries_[0].pull_down->selected_ = 0;
    m.entries_[0].pull_down->items_[0].id = 77;
    assert(m.on_key_click(0, 0x0d) == 1);
    assert(last_callback_id == 77);
    m.entries_[1].flags = 7;
    m.field_A24_ = 1;
    m.on_redraw();
    assert(m.fill_color == MenuDefaultSelectionColor);
    assert(m.selected_fill_text_color == MenuDefaultColor1);
    assert(m.writes == 3 && m.lines == 1);

    PullDown pd;
    // Supply valid empty rows before selectively populating the keyboard fixture.
    // Otherwise the mnemonic scan can dereference an uninitialized test pointer.
    for (int i = 0; i < 64; ++i) {
        pd.items_[i].id = 0;
        pd.items_[i].flags = 0;
        pd.items_[i].mnemonic = 0;
    }
    Win keyboard_parent;
    pd.set_parent(&keyboard_parent);
    pd.item_count_ = 4;
    pd.items_[0].flags = 1;
    pd.items_[1].flags = 5;
    pd.items_[2].flags = 0;
    pd.items_[3].flags = 1;
    pd.selected_ = 0;
    MenuPullDownInputOrigin = 0;
    assert(pd.on_key_click('q', 0x28) == 1);
    assert(pd.selected_ == 3 && pd.update_old == 0 && pd.update_new == 3);
    assert(MenuPullDownInputOrigin == 1 && pd.updates == 1);
    assert(pd.on_key_click('q', 0x26) == 1);
    assert(pd.selected_ == 0 && pd.update_old == 3 && pd.update_new == 0);
    // Native down-key jge rejects stale indices above count as well as count.
    pd.selected_ = 8;
    assert(pd.on_key_click('q', 0x28) == 1);
    assert(pd.selected_ == -1 && pd.update_old == 8 && pd.update_new == -1);
    char mnemonic[] = "z";
    pd.items_[3].mnemonic = mnemonic;
    assert(pd.on_key_click('z', 0) == 1);
    assert(pd.selected_ == 3 && pd.redraws == 1 && pd.modal_releases == 1);
    assert(pd.on_key_click('q', 0x1B) == 1);
    assert(pd.selected_ == -1 && pd.modal_releases == 2);
    assert(pd.on_key_click('q', 0x6B) == 0);
    assert(keyboard_parent.key_calls == 1);
    assert(keyboard_parent.last_key_input == 'q' && keyboard_parent.last_key == 0x6B);

    Menu fail_menu;
    char one_text[] = "one";
    assert(fail_menu.add_menu(1, one_text, 0) == 0);
    allocation_fail_after = 0;
    char fail_text[] = "failure";
    assert(fail_menu.UNK10(1, fail_text) == 4);
    assert(fail_menu.count_ == 0);
    allocation_fail_after = -1;
    std::puts("menu owner draft controls passed (host mock ABI only)");
}
