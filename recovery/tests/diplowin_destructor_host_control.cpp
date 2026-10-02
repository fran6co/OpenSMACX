#include <cassert>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

static std::vector<std::string> events;
static std::string throw_on;
static int outer_button_index;
static void record(const std::string &event) {
    events.push_back(event);
    if (event == throw_on) throw std::runtime_error(event);
}
static std::string indexed(const char *prefix, int index) {
    char value[32];
    std::sprintf(value, "%s%d", prefix, index);
    return value;
}

struct GraphicWinFake {
    std::string name;
    explicit GraphicWinFake(const char *value = "GraphicWin::~DiploWin")
        : name(value) {}
    ~GraphicWinFake() { record(name); }
};
struct InterfaceFake {};
struct TimeFake { ~TimeFake() { record("EditBox.Time::~Time"); } };

struct EditBoxFake : GraphicWinFake {
    TimeFake time;
    EditBoxFake() : GraphicWinFake("EditBox.GraphicWin::~") {}
    ~EditBoxFake() { record("EditBox.close"); }
};

struct DialogFake {
    ~DialogFake() { record("Dialog.close/~Dialog"); }
};
struct ListBoxFake : virtual GraphicWinFake, virtual DialogFake {
    ListBoxFake()
        : GraphicWinFake("ListBox.GraphicWin::~"), DialogFake() {}
    ~ListBoxFake() {
        record("ListBox.close");
        record("ListBox.GraphicWin.close");
        record("ListBox.Dialog.close");
    }
};

struct ButtonBaseFake {
    std::string name;
    explicit ButtonBaseFake(const std::string &value) : name(value) {}
    ~ButtonBaseFake() { record(name + ".BaseButton::~"); }
};
struct FlatButtonFake : ButtonBaseFake {
    explicit FlatButtonFake(const std::string &value) : ButtonBaseFake(value) {}
    FlatButtonFake() : ButtonBaseFake(indexed("outer.FlatButton", outer_button_index++)) {}
    ~FlatButtonFake() { record(name + ".FlatButton.close"); }
};

struct ScrollFake : GraphicWinFake {
    FlatButtonFake left;
    FlatButtonFake right;
    ScrollFake()
        : GraphicWinFake("Scroll.GraphicWin::~"), left("Scroll.left"),
          right("Scroll.right") {}
    ~ScrollFake() {
        record("Scroll.close");
        record("Scroll.left.close");
        record("Scroll.right.close");
        record("Scroll.GraphicWin.close");
    }
};

struct AllocationBaseFake {
    ~AllocationBaseFake() { record("StringAllocationBase::~"); }
};
struct StringStructFake : virtual AllocationBaseFake {
    ~StringStructFake() { record("StringStruct::~"); }
};
struct StringListFake : StringStructFake {
    ~StringListFake() {
        record("StringList.remove_all");
        record("StringList.current_position=0");
    }
};

struct StringBoxFake : GraphicWinFake {
    ScrollFake scroll;
    StringListFake strings;
    StringBoxFake() : GraphicWinFake("StringBox.GraphicWin::~") {}
    ~StringBoxFake() {
        record("StringBox.close");
        record("StringBox.GraphicWin.close");
        record("StringBox.StringList.remove_all");
        record("StringBox.StringList.current_position=0");
        record("StringBox.Scroll.close");
        record("StringBox.Scroll.left.close");
        record("StringBox.Scroll.right.close");
        record("StringBox.Scroll.GraphicWin.close");
    }
};

struct SpotFake { ~SpotFake() { record("Spot.shutdown/~Spot"); } };

struct DiploWin {
    GraphicWinFake graphic;
    InterfaceFake subinterface;
    SpotFake spot;
    FlatButtonFake flat_buttons[13];
    StringBoxFake string_box;
    ListBoxFake list_box;
    EditBoxFake edit_box;
    ~DiploWin();
};

#include "diplowin_destructor_candidate.cpp"

static std::vector<std::string> normal_expected() {
    std::vector<std::string> result;
    result.push_back("EditBox.close");
    result.push_back("EditBox.Time::~Time");
    result.push_back("EditBox.GraphicWin::~");
    result.push_back("ListBox.close");
    result.push_back("ListBox.GraphicWin.close");
    result.push_back("ListBox.Dialog.close");
    result.push_back("Dialog.close/~Dialog");
    result.push_back("ListBox.GraphicWin::~");
    result.push_back("StringBox.close");
    result.push_back("StringBox.GraphicWin.close");
    result.push_back("StringBox.StringList.remove_all");
    result.push_back("StringBox.StringList.current_position=0");
    result.push_back("StringBox.Scroll.close");
    result.push_back("StringBox.Scroll.left.close");
    result.push_back("StringBox.Scroll.right.close");
    result.push_back("StringBox.Scroll.GraphicWin.close");
    result.push_back("StringList.remove_all");
    result.push_back("StringList.current_position=0");
    result.push_back("StringStruct::~");
    result.push_back("StringAllocationBase::~");
    result.push_back("Scroll.close");
    result.push_back("Scroll.left.close");
    result.push_back("Scroll.right.close");
    result.push_back("Scroll.GraphicWin.close");
    result.push_back("Scroll.right.FlatButton.close");
    result.push_back("Scroll.right.BaseButton::~");
    result.push_back("Scroll.left.FlatButton.close");
    result.push_back("Scroll.left.BaseButton::~");
    result.push_back("Scroll.GraphicWin::~");
    result.push_back("StringBox.GraphicWin::~");
    for (int index = 12; index >= 0; --index) {
        const std::string name = indexed("outer.FlatButton", index);
        result.push_back(name + ".FlatButton.close");
        result.push_back(name + ".BaseButton::~");
    }
    result.push_back("Spot.shutdown/~Spot");
    result.push_back("GraphicWin::~DiploWin");
    return result;
}

int main() {
    outer_button_index = 0;
    {
        DiploWin object;
    }
    assert(events == normal_expected());

    events.clear();
    outer_button_index = 0;
    throw_on = "ListBox.close";
    bool caught = false;
    try {
        DiploWin object;
    } catch (const std::runtime_error &) {
        caught = true;
    }
    assert(caught);
    assert(events.size() > 8);
    assert(events[0] == "EditBox.close");
    assert(events[3] == "ListBox.close");
    assert(events[4] == "Dialog.close/~Dialog");
    assert(events[5] == "ListBox.GraphicWin::~");
    assert(events[6] == "StringBox.close");
    assert(events.back() == "GraphicWin::~DiploWin");
    return 0;
}
