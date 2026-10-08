#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>

#define __stdcall
typedef short (__stdcall *KeyStateFn)(int);
enum { VK_SHIFT = 0x10, VK_CONTROL = 0x11, VK_SCROLL = 0x91 };

struct Win {
    virtual ~Win() {}
    virtual void on_mouse_move(int, int) = 0;
};

struct GraphicWin : Win {
    GraphicWin() : close_calls(0) {}
    void close();
    int close_calls;
};

struct MapWin : public virtual GraphicWin {
    MapWin();
    void on_sys_close();
    void close();
    void on_right_down(int, int);
    void on_mouse_move(int, int);
    uint32_t field_1DD74_, field_1DD78_, field_1DD7C_, field_1DD80_;
    uint32_t field_1DD84_, field_1DD88_, field_1DD8C_;
    int32_t field_1DD90_, field_1DD94_, field_1DE0C_;
    void *owned_;
    int move_calls, last_x, last_y;
    bool callback_saw_published_owners;
    Win *seen_owner3, *seen_owner4;
};

static MapWin *close_observed_map;
static int free_calls;
static std::vector<char> events;
void tracked_free(void *pointer) {
    ++free_calls;
    events.push_back('F');
    std::free(pointer);
}
#define free tracked_free

MapWin::MapWin() : field_1DD74_(0), field_1DD78_(0), field_1DD7C_(0),
    field_1DD80_(0), field_1DD84_(0), field_1DD88_(0), field_1DD8C_(0),
    field_1DD90_(0), field_1DD94_(0), field_1DE0C_(0), owned_(0),
    move_calls(0), last_x(0), last_y(0), callback_saw_published_owners(false),
    seen_owner3(0), seen_owner4(0) {}

void GraphicWin::close() {
    events.push_back('C');
    ++close_calls;
    assert(close_observed_map != 0);
    assert(close_observed_map->owned_ == 0);
}

static int MapInputEnabled;
struct StatusWindow {
    MapWin *mutate;
    int calls;
    void set_title(int title) {
        assert(title == 0);
        ++calls;
        events.push_back('T');
        if (mutate != 0) {
            mutate->field_1DD7C_ = 101;
            mutate->field_1DD80_ = 102;
            mutate->field_1DD84_ = 103;
            mutate->field_1DD88_ = 104;
            mutate->field_1DD8C_ = 105;
            mutate->field_1DD90_ = 106;
            mutate->field_1DD94_ = 107;
            mutate->field_1DE0C_ = 108;
        }
    }
} ConsoleStatusWin = {0, 0};

struct ConsoleState { int field_23D10_; } ConsoleGlobal = {0};
struct GameFields { uint32_t game_state_; };
struct GameState { GameFields fields_; } GameDataStateStorage = {{0}};
Win *WinPointerOwner3 = 0;
Win *WinPointerOwner4 = 0;

struct KeyReply { int key; short value; };
static short shift_reply;
static short ctrl_reply;
static short scroll_reply;
static std::vector<int> key_calls;
short __stdcall GetAsyncKeyState(int key) {
    events.push_back('A');
    key_calls.push_back(key);
    if (key == VK_SHIFT) return shift_reply;
    if (key == VK_CONTROL) return ctrl_reply;
    assert(false && "unexpected async key");
    return 0;
}
short __stdcall GetKeyState(int key) {
    events.push_back('K');
    assert(key == VK_SCROLL);
    key_calls.push_back(key);
    return scroll_reply;
}

/* ACTUAL_CLOSE */
/* ACTUAL_ON_SYS_CLOSE */
/* ACTUAL_ON_RIGHT_DOWN */
#undef free

void MapWin::on_mouse_move(int x, int y) {
    ++move_calls;
    last_x = x;
    last_y = y;
    events.push_back('M');
    seen_owner3 = WinPointerOwner3;
    seen_owner4 = WinPointerOwner4;
    callback_saw_published_owners = true;
}

static void reset_globals() {
    free_calls = 0;
    close_observed_map = 0;
    MapInputEnabled = 1;
    ConsoleGlobal.field_23D10_ = 0;
    GameDataStateStorage.fields_.game_state_ = 0;
    WinPointerOwner3 = 0;
    WinPointerOwner4 = 0;
    shift_reply = ctrl_reply = scroll_reply = 0;
    key_calls.clear();
    events.clear();
    ConsoleStatusWin.mutate = 0;
    ConsoleStatusWin.calls = 0;
}

static void seed_right_down(MapWin &map) {
    map.field_1DD7C_ = 1;
    map.field_1DD80_ = 2;
    map.field_1DD84_ = 3;
    map.field_1DD88_ = 4;
    map.field_1DD8C_ = 5;
    map.field_1DD90_ = 6;
    map.field_1DD94_ = 7;
    map.field_1DE0C_ = 8;
    map.move_calls = 0;
    map.callback_saw_published_owners = false;
}

static void assert_cleared(const MapWin &map, uint32_t final_drag_state = 0) {
    assert(map.field_1DD7C_ == 0 && map.field_1DD80_ == 0);
    assert(map.field_1DD84_ == 0 && map.field_1DD88_ == 0);
    assert(map.field_1DD8C_ == final_drag_state && map.field_1DE0C_ == -1);
}

static std::vector<int> expected_keys(bool shift, bool ctrl, bool scroll,
                                     bool game_flag) {
    std::vector<int> expected;
    expected.push_back(VK_SHIFT);
    bool at_check_flag = false;
    if (shift) {
        expected.push_back(VK_CONTROL);
        at_check_flag = ctrl;
    }
    if (!at_check_flag) {
        expected.push_back(VK_SCROLL);
        if (!scroll) {
            expected.push_back(VK_CONTROL); // abort label's final poll
            return expected;
        }
        expected.push_back(VK_CONTROL); // second control test
        if (ctrl) {
            expected.push_back(VK_CONTROL); // abort label's final poll
            return expected;
        }
        at_check_flag = true;
    }
    if (at_check_flag && !game_flag)
        expected.push_back(VK_CONTROL); // abort label's final poll
    return expected;
}

static void test_close_lifecycle() {
    reset_globals();
    MapWin map;
    assert(static_cast<void *>(static_cast<GraphicWin *>(&map)) !=
           static_cast<void *>(&map));
    close_observed_map = &map;
    map.field_1DD78_ = 0;
    map.field_1DD74_ = 0xA5;
    map.owned_ = std::malloc(16);
    assert(map.owned_ != 0);
    map.on_sys_close();
    assert(map.field_1DD74_ == 0xA5);
    assert(free_calls == 0 && map.close_calls == 0);
    std::free(map.owned_);
    map.owned_ = 0;

    map.field_1DD78_ = 1;
    map.field_1DD74_ = 0xA5;
    map.owned_ = std::malloc(16);
    assert(map.owned_ != 0);
    map.on_sys_close();
    assert(map.field_1DD74_ == 0);
    assert(map.owned_ == 0 && free_calls == 1 && map.close_calls == 1);
    assert(events.size() == 2 && events[0] == 'F' && events[1] == 'C');
    map.on_sys_close();
    assert(free_calls == 1 && map.close_calls == 2);
}

static void test_right_down_truth_matrix() {
    for (int input = 0; input != 2; ++input)
    for (int console_gate = 0; console_gate != 2; ++console_gate)
    for (int shift = 0; shift != 2; ++shift)
    for (int ctrl = 0; ctrl != 2; ++ctrl)
    for (int scroll = 0; scroll != 2; ++scroll)
    for (int game = 0; game != 2; ++game) {
        reset_globals();
        MapWin map;
        seed_right_down(map);
        MapInputEnabled = input;
        ConsoleGlobal.field_23D10_ = console_gate;
        shift_reply = shift ? static_cast<short>(0x8000) : 0;
        ctrl_reply = ctrl ? static_cast<short>(0x8000) : 0;
        scroll_reply = scroll ? 1 : 0;
        GameDataStateStorage.fields_.game_state_ = game ? 0x40 : 0;
        map.on_right_down(-123, 456);

        if (!input) {
            assert(map.field_1DD7C_ == 1 && map.field_1DD80_ == 2);
            assert(map.field_1DD84_ == 3 && map.field_1DD88_ == 4);
            assert(map.field_1DD8C_ == 5 && map.field_1DE0C_ == 8);
            assert(key_calls.empty() && map.move_calls == 0);
            continue;
        }

        const bool route_flag = (shift && ctrl) || (scroll && !ctrl);
        const bool expect_move = !console_gate && route_flag && game;
        assert_cleared(map, expect_move ? 2 : 0);
        if (console_gate) {
            assert(key_calls.empty() && map.move_calls == 0);
            continue;
        }

        const std::vector<int> expected = expected_keys(shift, ctrl, scroll, game);
        assert(key_calls == expected);
        assert(map.move_calls == (expect_move ? 1 : 0));
        if (expect_move) {
            assert(map.last_x == -123 && map.last_y == 456);
            assert(map.field_1DD8C_ == 2);
            assert(map.field_1DD94_ == -1 && map.field_1DD90_ == -1);
            assert(map.callback_saw_published_owners);
            assert(WinPointerOwner3 == static_cast<Win *>(&map));
            assert(WinPointerOwner4 == 0);
            assert(map.seen_owner3 == static_cast<Win *>(&map));
            assert(map.seen_owner4 == 0);
        } else {
            assert(map.field_1DD8C_ == 0);
        }
    }
}

static void test_status_callback_and_existing_owner() {
    reset_globals();
    MapWin map;
    seed_right_down(map);
    map.field_1DD80_ = 1;
    ConsoleStatusWin.mutate = &map;
    shift_reply = static_cast<short>(0x8000);
    ctrl_reply = static_cast<short>(0x8000);
    GameDataStateStorage.fields_.game_state_ = 0x40;
    WinPointerOwner3 = reinterpret_cast<Win *>(static_cast<uintptr_t>(0x1234));
    WinPointerOwner4 = reinterpret_cast<Win *>(static_cast<uintptr_t>(0x5678));
    Win *const old_owner3 = WinPointerOwner3;
    Win *const old_owner4 = WinPointerOwner4;
    map.on_right_down(17, -29);
    assert(ConsoleStatusWin.calls == 1);
    assert(events.size() >= 2 && events[0] == 'T' && events[1] == 'A');
    assert_cleared(map, 2);
    assert(map.move_calls == 1 && map.last_x == 17 && map.last_y == -29);
    assert(map.callback_saw_published_owners);
    assert(WinPointerOwner3 == old_owner3 && WinPointerOwner4 == old_owner4);
    assert(map.seen_owner3 == old_owner3 && map.seen_owner4 == old_owner4);
    ConsoleStatusWin.mutate = 0;
}

int main() {
    test_close_lifecycle();
    test_right_down_truth_matrix();
    test_status_callback_and_existing_owner();
    std::puts("MapWin input owner actual-source controls passed");
}
