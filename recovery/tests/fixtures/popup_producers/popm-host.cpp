// Host behavior control for the source-extracted 0x0059D3B0 body.
#include <cassert>

#define __cdecl

char PopupStartCaption[] = "POPUP";

struct PopMenu {
    static int constructors;
    static int destructors;
    static int start_calls;
    static int exec_calls;
    static int result;
    static bool throw_from_exec;
    static const char *last_file;
    static const char *last_label;
    static int last_value;
    static char *last_override;
    static int last_flags;
    static int last_x;
    static int last_y;
    static int (*last_poll)();

    PopMenu() { ++constructors; }
    ~PopMenu() { ++destructors; }
    void start(char *file, const char *label, int value, char *override_text,
               int flags, void *parent) {
        ++start_calls;
        last_file = file;
        last_label = label;
        last_value = value;
        last_override = override_text;
        last_flags = flags;
        assert(parent == 0);
    }
    int exec(int x, int y, int (*poll)()) {
        ++exec_calls;
        last_x = x;
        last_y = y;
        last_poll = poll;
        if (throw_from_exec) throw 11;
        return result;
    }
};

int PopMenu::constructors = 0;
int PopMenu::destructors = 0;
int PopMenu::start_calls = 0;
int PopMenu::exec_calls = 0;
int PopMenu::result = 0;
bool PopMenu::throw_from_exec = false;
const char *PopMenu::last_file = 0;
const char *PopMenu::last_label = 0;
int PopMenu::last_value = 0;
char *PopMenu::last_override = 0;
int PopMenu::last_flags = 0;
int PopMenu::last_x = 0;
int PopMenu::last_y = 0;
int (*PopMenu::last_poll)() = 0;

/*__POPM_BODY__*/

static int poll_callback() { return 7; }

int main() {
    char file[] = "MENUFILE";
    char label[] = "ITEMS";
    PopMenu::result = 3;
    assert(popm(file, label, -17, 29, poll_callback) == 3);
    assert(PopMenu::constructors == 1 && PopMenu::destructors == 1);
    assert(PopMenu::start_calls == 1 && PopMenu::exec_calls == 1);
    assert(PopMenu::last_file == file && PopMenu::last_label == label);
    assert(PopMenu::last_value == -1 && PopMenu::last_override == 0);
    assert(PopMenu::last_flags == 0);
    assert(PopMenu::last_x == -17 && PopMenu::last_y == 29);
    assert(PopMenu::last_poll == poll_callback);
    PopMenu::result = -1;
    assert(popm(file, label, 4, 5, 0) == -1);
    assert(PopMenu::constructors == 2 && PopMenu::destructors == 2);
    assert(PopMenu::start_calls == 2 && PopMenu::exec_calls == 2);
    PopMenu::throw_from_exec = true;
    bool caught = false;
    try {
        popm(file, label, 1, 2, poll_callback);
    } catch (int value) {
        caught = value == 11;
    }
    assert(caught);
    assert(PopMenu::constructors == 3 && PopMenu::destructors == 3);
}
