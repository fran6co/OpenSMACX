// Ignored C++98 behavior fixture. The pytest proposal inserts production
// definitions extracted from src/popup.cpp at ACTUAL_SOURCE_BODIES below.
// This scaffolding models only the relevant typed owner graph and observers.
#include <cassert>
#include <cstring>

#ifndef __cdecl
#define __cdecl
#endif
#ifndef __stdcall
#define __stdcall
#endif

struct Win {
    int slot48_calls;
    int slot49_calls;
    int first;
    int second;
    Win() : slot48_calls(0), slot49_calls(0), first(0), second(0) {}
    virtual void vslot_48(int a, int b) {
        ++slot48_calls; first = a; second = b;
    }
    virtual void vslot_49(int a, int b) {
        ++slot49_calls; first = a; second = b;
    }
    virtual ~Win() {}
};

struct GraphicWin : Win {};
struct Dialog : virtual GraphicWin {};
struct ListBox : virtual GraphicWin, virtual Dialog {};
struct RadioButton : virtual GraphicWin, virtual Dialog {};
struct CheckBox : virtual GraphicWin, virtual Dialog {};
struct SpriteBox : virtual GraphicWin, virtual Dialog {};
struct EditGroup : virtual GraphicWin, virtual Dialog {};
// Mirrors Dialogs' five widget bases: they share one GraphicWin and Dialog
// virtual base, distinct from the Popup/BasePop GraphicWin subobject.
struct Dialogs : ListBox, RadioButton, CheckBox, SpriteBox, EditGroup {};

class BasePop : public GraphicWin {
public:
    Dialogs &dialogs() { return dialogs_; }
    void fallout();
    Dialogs dialogs_;
};

class Popup : public BasePop {
public:
    void vslot_48(int first, int second);
    void vslot_49(int first, int second);
    int on_link_clicked(int id, char *link);
};

struct Player {
    char filename[24];
    char search_key[24];
};

Player Players[8];
char PopupPendingLink[256];
int PopupPendingLinkFlag;
char PopupLinkMatchText[64] = "OPEN";
char PopupLinkOpenVerb[16] = "open";
char PopupLinkTarget[128] = "target";
int ExitTurnLoop;
void *HandleMain;

typedef long (__stdcall *func_popup_shell_execute)(void *, const char *,
                                                    const char *, const char *,
                                                    const char *, int);
func_popup_shell_execute &PopupShellExecuteSlot();

int __cdecl X_pop(char *, const char *, int (__cdecl *)());
int __cdecl X_pop(const char *, int (__cdecl *)());
void __cdecl popup_pending_link_dialog(const char *label);
int __cdecl popup_faction_datalinks(int faction_id);

static int fallout_calls;
void BasePop::fallout() { ++fallout_calls; }
void ShowWindow(void *, int) {}
static long shell_execute(void *, const char *, const char *, const char *,
                         const char *, int) { return 0; }
func_popup_shell_execute shell_slot = shell_execute;
func_popup_shell_execute &PopupShellExecuteSlot() { return shell_slot; }

static int same_case_insensitive(const char *left, const char *right) {
    while (*left && *right) {
        char a = *left++, b = *right++;
        if (a >= 'a' && a <= 'z') a = static_cast<char>(a - 'a' + 'A');
        if (b >= 'a' && b <= 'z') b = static_cast<char>(b - 'a' + 'A');
        if (a != b) return 0;
    }
    return *left == *right;
}
int _stricmp(const char *left, const char *right) {
    return same_case_insensitive(left, right) ? 0 : 1;
}

static char pop_labels[8][256];
static int pop_flags_before_call[8];
static int pop_count;
static char caption_seen[24];
static char label_seen[32];
static int callback_was_null;
static int pop_result;

int __cdecl X_pop(char *caption, const char *label,
                                int (__cdecl *callback)()) {
    std::strcpy(caption_seen, caption);
    std::strcpy(label_seen, label);
    callback_was_null = callback == 0;
    return pop_result;
}

// The scripted popup events exercise producer->consumer order. Initial menu
// selection queues "FIRST"; processing it queues "SECOND". The consumer must
// clear the flag before each popup so those producer writes survive.
int __cdecl X_pop(const char *label, int (__cdecl *)()) {
    assert(pop_count < 8);
    std::strncpy(pop_labels[pop_count], label, sizeof(pop_labels[0]) - 1);
    pop_labels[pop_count][sizeof(pop_labels[0]) - 1] = 0;
    pop_flags_before_call[pop_count] = PopupPendingLinkFlag;
    ++pop_count;
    if (pop_count == 1) {
        char first[] = "FIRST";
        Popup producer;
        producer.on_link_clicked(17, first);
    } else if (pop_count == 2) {
        char second[] = "SECOND";
        Popup producer;
        producer.on_link_clicked(29, second);
    }
    return 0;
}

// ACTUAL_SOURCE_BODIES

static void test_popup_virtual_forwarders() {
    Popup popup;
    popup.vslot_48(0x1234, -7);
    assert(popup.dialogs().slot48_calls == 1);
    assert(popup.dialogs().first == 0x1234 && popup.dialogs().second == -7);
    assert(popup.slot48_calls == 0);
    popup.vslot_49(0x2345, 91);
    assert(popup.dialogs().slot49_calls == 1);
    assert(popup.dialogs().first == 0x2345 && popup.dialogs().second == 91);
    assert(popup.slot49_calls == 0);
}

static void test_pending_link_consumer_and_producer() {
    const char initial[] = "MAIN";
    std::strcpy(PopupPendingLink, "stale");
    PopupPendingLinkFlag = 1;
    pop_count = 0;
    fallout_calls = 0;
    popup_pending_link_dialog(initial);
    assert(pop_count == 3);
    assert(std::strcmp(pop_labels[0], "MAIN") == 0);
    assert(std::strcmp(pop_labels[1], "FIRST") == 0);
    assert(std::strcmp(pop_labels[2], "SECOND") == 0);
    assert(pop_flags_before_call[0] == 0);
    assert(pop_flags_before_call[1] == 0);
    assert(pop_flags_before_call[2] == 0);
    assert(PopupPendingLinkFlag == 0);
    assert(fallout_calls == 2);
}

static void test_faction_datalinks_uses_player_filename_and_returns_result() {
    std::strcpy(Players[3].filename, "FACTION3.TXT");
    std::strcpy(Players[3].search_key, "WRONG-ALIAS");
    std::strcpy(caption_seen, "");
    std::strcpy(label_seen, "");
    callback_was_null = 0;
    pop_result = 0x1357;
    assert(popup_faction_datalinks(3) == 0x1357);
    assert(std::strcmp(caption_seen, "FACTION3.TXT") == 0);
    assert(std::strcmp(label_seen, "DATALINKS") == 0);
    assert(callback_was_null == 1);
}

int main() {
    test_popup_virtual_forwarders();
    test_pending_link_consumer_and_producer();
    test_faction_datalinks_uses_player_filename_and_returns_result();
    return 0;
}
