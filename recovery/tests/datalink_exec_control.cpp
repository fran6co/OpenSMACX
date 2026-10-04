// Private standalone fixture for an extracted actual Datalink::exec body.
// The pytest harness inserts product text at the token below; no body is
// duplicated in this fixture.
#include <assert.h>
#include <stdint.h>
#include <string>
#include <vector>

struct Font {};
struct Win {};
struct MainInterface : Win {};
struct Console { int faction_id_; };
struct ReportIf {
    uint32_t field_8_;
    int32_t report_faction_id_;
};
struct ReportWin {
    int field_FAB8_;
    int &shared_research_focus() { return field_FAB8_; }
};
struct Sprite {
    int width_;
    int width() const { return width_; }
};
struct FactionArt { Sprite datalink; };

struct RectObservation {
    int x;
    int y;
    int width;
    int height;
    Win *parent;
    void *list;
    int flags;
};

static std::vector<std::string> Events;
static RectObservation Rect;
static Font *FontArgs[4];
static int ColorArgs[3][4];
static int SelectedId;
static int ModalFlag;
static int (*ModalCallback)();
static Win *ModalOwner;
static int RedrawCount;
static int ReleaseCount;
static int MutateSavedAfterSetCat;
static int MutateSavedAfterModal;

int WinScreenWidth = 800;
ReportIf ReportIfGlobal = {0, 0};
ReportWin ReportWinGlobal = {0};
Console ConsoleGlobal = {0};
MainInterface MainInterfaceGlobal;
FactionArt FactionsArt[8];
int DatalinkWrongFocus = 0; // Used only by the wrong-owner damage mutant.

int pop_wait() { Events.push_back("pop_wait"); return 0; }

struct BufferCalls {
    void set_font(Font *a, Font *b, Font *c, Font *d) {
        FontArgs[0] = a; FontArgs[1] = b;
        FontArgs[2] = c; FontArgs[3] = d;
        Events.push_back("font");
    }
    void set_text_color(int a, int b, int c, int d) {
        ColorArgs[0][0] = a; ColorArgs[0][1] = b;
        ColorArgs[0][2] = c; ColorArgs[0][3] = d;
        Events.push_back("color1");
    }
    void set_text_color2(int a, int b, int c, int d) {
        ColorArgs[1][0] = a; ColorArgs[1][1] = b;
        ColorArgs[1][2] = c; ColorArgs[1][3] = d;
        Events.push_back("color2");
    }
    void set_text_color_hyper(int a, int b, int c, int d) {
        ColorArgs[2][0] = a; ColorArgs[2][1] = b;
        ColorArgs[2][2] = c; ColorArgs[2][3] = d;
        Events.push_back("hyper");
    }
};

struct StringBox : BufferCalls {
    void init(int x, int y, int width, int height, Win *parent,
              void *list, int flags) {
        Rect.x = x; Rect.y = y; Rect.width = width; Rect.height = height;
        Rect.parent = parent; Rect.list = list; Rect.flags = flags;
        Events.push_back("init");
    }
};
struct ListBox {
    void set_selected_id(int id) {
        SelectedId = id;
        Events.push_back("selected");
    }
};
struct SubInterface {
    void release_iface_mode() {
        ++ReleaseCount;
        Events.push_back("release");
    }
};
struct WinApi : Win {
    int set_modal(int flag, int (*callback)(), Win *owner);
    struct Datalink *current_window;
};

struct Datalink : WinApi, SubInterface {
    uint32_t field_A1C_;
    uint32_t field_A20_;
    uint32_t field_A24_;
    uint32_t field_29DC_;
    int32_t history_[21];
    uint32_t field_10394_;
    uint32_t field_10398_;
    uint32_t field_1039C_;
    uint32_t field_103A0_;
    Font secondary_body_font_;
    Font link_font_;
    StringBox stringBox_;
    ListBox listBox_;

    Datalink() : field_A1C_(0), field_A20_(0), field_A24_(0),
        field_29DC_(0), field_10394_(0), field_10398_(0),
        field_1039C_(0), field_103A0_(0) {
        current_window = this;
    }
    void exec(unsigned int topic, int index);
    void set_cat(unsigned int topic, int index) {
        field_29DC_ = topic;
        assert(index == 1);
        ReportWinGlobal.shared_research_focus() = 91;
        ReportIfGlobal.field_8_ = 92;
        ReportIfGlobal.report_faction_id_ = 93;
        if (MutateSavedAfterSetCat) {
            field_A1C_ = 31;
            field_A20_ = 32;
            field_A24_ = 33;
        }
        Events.push_back("set_cat");
    }
    void repaint_slot() {
        ++RedrawCount;
        Events.push_back("repaint");
    }
};

int WinApi::set_modal(int flag, int (*callback)(), Win *owner) {
    ModalFlag = flag;
    ModalCallback = callback;
    ModalOwner = owner;
    Events.push_back("modal");
    if (MutateSavedAfterModal) {
        ReportWinGlobal.shared_research_focus() = 81;
        ReportIfGlobal.field_8_ = 82;
        ReportIfGlobal.report_faction_id_ = 83;
        current_window->field_A1C_ = 41;
        current_window->field_A20_ = 42;
        current_window->field_A24_ = 43;
    }
    return 0;
}

// @@DATALINK_EXEC_DEFINITION@@

static void reset_observations(int width) {
    Events.clear();
    WinScreenWidth = width;
    ReportWinGlobal.shared_research_focus() = 11;
    ReportIfGlobal.field_8_ = 7;
    ReportIfGlobal.report_faction_id_ = 4;
    ConsoleGlobal.faction_id_ = 2;
    FactionsArt[1].datalink.width_ = 42;
    SelectedId = -1;
    ModalFlag = -1;
    ModalCallback = 0;
    ModalOwner = 0;
    RedrawCount = 0;
    ReleaseCount = 0;
    MutateSavedAfterSetCat = 0;
    MutateSavedAfterModal = 0;
}

static void test_800_geometry_and_owner_identity() {
    reset_observations(0x320);
    Datalink window;
    window.field_10394_ = 100;
    window.field_10398_ = 20;
    window.field_1039C_ = 300;
    window.field_103A0_ = 200;
    window.exec(6, 13);

    assert(Rect.x == 150 && Rect.y == 23);
    assert(Rect.width == 147 && Rect.height == 171);
    assert(Rect.parent == static_cast<Win *>(&window));
    assert(Rect.list == 0 && Rect.flags == -1);
    assert(FontArgs[0] == 0 && FontArgs[1] == &window.secondary_body_font_);
    assert(FontArgs[2] == 0 && FontArgs[3] == &window.link_font_);
    assert(ColorArgs[0][0] == 0x98 && ColorArgs[1][0] == 0x96);
    assert(ColorArgs[2][0] == 0x96);
    assert(SelectedId == 60013 && window.history_[0] == 13);
    assert(ModalFlag == 1 && ModalCallback == pop_wait);
    assert(ModalOwner == static_cast<Win *>(&MainInterfaceGlobal));
    assert(RedrawCount == 1 && ReleaseCount == 1);
    const char *expected[] = {"init", "font", "color1", "color2", "hyper",
        "set_cat", "selected", "repaint", "modal", "release"};
    assert(Events.size() == sizeof(expected) / sizeof(expected[0]));
    for (unsigned int i = 0; i < Events.size(); ++i)
        assert(Events[i] == expected[i]);
}

static void test_restores_after_mutating_callbacks() {
    reset_observations(0x320);
    MutateSavedAfterSetCat = 1;
    MutateSavedAfterModal = 1;
    Datalink window;
    window.field_10394_ = 10;
    window.field_10398_ = 20;
    window.field_1039C_ = 500;
    window.field_103A0_ = 400;
    window.exec(4, 9);

    // These are the saved owner fields after callbacks deliberately mutate
    // them. Final shared cells must be restored from the source-owned members.
    assert(window.field_A1C_ == 41 && window.field_A20_ == 42);
    assert(window.field_A24_ == 43);
    assert(ReportWinGlobal.shared_research_focus() == 41);
    assert(ReportIfGlobal.field_8_ == 42);
    assert(ReportIfGlobal.report_faction_id_ == 43);
    assert(SelectedId == 40009 && window.history_[0] == 9);
    assert(RedrawCount == 1 && ReleaseCount == 1);
}

static void test_non_800_skips_box_setup() {
    reset_observations(1024);
    Datalink window;
    window.exec(3, 5);
    assert(Events.size() == 5);
    assert(Events[0] == "set_cat");
    assert(SelectedId == 30005 && window.history_[0] == 5);
    assert(RedrawCount == 1 && ReleaseCount == 1);
    assert(ModalOwner == static_cast<Win *>(&MainInterfaceGlobal));
}

int main() {
    test_800_geometry_and_owner_identity();
    test_restores_after_mutating_callbacks();
    test_non_800_skips_box_setup();
}
