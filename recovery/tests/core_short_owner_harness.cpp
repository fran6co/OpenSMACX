#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#ifndef __cdecl
#define __cdecl
#endif

struct RulesTechnology { const char *name; };
struct Player { const char *adj_name_faction; };
struct Console { int faction_id_; };
struct MSG { unsigned int message; };
struct Net { void handle_sys_msg(); };

extern RulesTechnology Technology[89];
extern Player Players[8];
extern Console ConsoleGlobal;
extern char StringTemp[1032];
extern Net *ActiveNet;
extern uint32_t MsgStatus;
int parse_say(int, intptr_t, int, int);
int parse_says(int, const char *, int, int);
void say_num(int);
int X_pop(const char *, int (*)());
void do_video();
void do_net();
void check_net();
int PeekMessage(MSG *, void *, unsigned int, unsigned int, unsigned int);
int TranslateMessage(const MSG *);
long DispatchMessage(const MSG *);

#define PM_REMOVE 1
#define WM_PAINT 0x000F
#define WM_USER 0x0400
#define PeekMessageA PeekMessage
#define DispatchMessageA DispatchMessage

// __ACTUAL_BODIES_SLOT__

RulesTechnology Technology[89];
Player Players[8];
Console ConsoleGlobal;
char StringTemp[1032];
Net *ActiveNet;
uint32_t MsgStatus;

static Net live_net;
static std::vector<std::string> calls;
static std::vector<unsigned int> queue_messages;
static intptr_t expected_technology_name;
static const char *expected_faction_name;
static std::string expected_popup;
static int expected_number;
static bool watch_do_net_status;
static bool mutate_msg_status_in_do_net;
static uint32_t last_do_net_status;

static void reset_fixture() {
    calls.clear();
    queue_messages.clear();
    std::memset(StringTemp, 0, sizeof(StringTemp));
    MsgStatus = 99;
    ActiveNet = &live_net;
    watch_do_net_status = false;
    mutate_msg_status_in_do_net = false;
    last_do_net_status = 0xFFFFFFFFu;
}

void Net::handle_sys_msg() { calls.push_back("sys"); }

int parse_say(int index, intptr_t name, int, int) {
    assert(index == 0);
    assert(name == expected_technology_name);
    calls.push_back("tech");
    return 1;
}

int parse_says(int index, const char *name, int, int) {
    assert(index == 1);
    assert(name == expected_faction_name);
    calls.push_back("faction");
    return 1;
}

void say_num(int number) {
    assert(number == expected_number);
    char digit[2] = {static_cast<char>('0' + number), '\0'};
    std::strcat(StringTemp, digit);
    calls.push_back("number");
}

int X_pop(const char *text, int (*callback)()) {
    assert(callback == nullptr);
    assert(expected_popup == text);
    calls.push_back("popup");
    return 0;
}

void do_video() { calls.push_back("video"); }
void do_net() {
    calls.push_back("net");
    if (watch_do_net_status) last_do_net_status = MsgStatus;
    if (mutate_msg_status_in_do_net) MsgStatus = 77;
}
void check_net() { calls.push_back("check"); }

int PeekMessage(MSG *out, void *, unsigned int low, unsigned int high,
                unsigned int remove) {
    assert(remove == PM_REMOVE);
    calls.push_back("peek:" + std::to_string(low) + ":" +
                    std::to_string(high));
    for (std::vector<unsigned int>::iterator it = queue_messages.begin();
         it != queue_messages.end(); ++it) {
        if (*it >= low && *it <= high) {
            out->message = *it;
            queue_messages.erase(it);
            return 1;
        }
    }
    return 0;
}

int TranslateMessage(const MSG *message) {
    calls.push_back("translate:" + std::to_string(message->message));
    return 1;
}

long DispatchMessage(const MSG *message) {
    calls.push_back("dispatch:" + std::to_string(message->message));
    return 0;
}

static bool has_call(const char *name) {
    for (const std::string &call : calls) if (call == name) return true;
    return false;
}

static void test_tech_secret() {
    expected_technology_name = reinterpret_cast<intptr_t>(Technology[7].name);
    expected_faction_name = Players[2].adj_name_faction;
    expected_number = 0;
    expected_popup = "THESECRET0";
    reset_fixture();
    tech_secret(7, 2);
    assert(StringTemp == expected_popup);
    assert((calls == std::vector<std::string>{"tech", "faction", "number", "popup"}));

    expected_faction_name = Players[3].adj_name_faction;
    expected_number = 1;
    expected_popup = "THESECRET1";
    reset_fixture();
    tech_secret(7, 3);
    assert(StringTemp == expected_popup);
    assert((calls == std::vector<std::string>{"tech", "faction", "number", "popup"}));
}

static void test_do_task_special_message_and_poll_range() {
    reset_fixture();
    queue_messages.push_back(0x000A);
    queue_messages.push_back(0x000F);
    assert(do_task() == 1);
    assert(calls.size() >= 4);
    assert(calls[0] == "video" && calls[1] == "net" && calls[2] == "check");
    assert(calls[3] == "video" && calls[4] == "check" && calls[5] == "net");
    assert(calls[6] == "peek:15:15");
    assert(has_call("dispatch:15") && has_call("dispatch:10"));
    assert(!has_call("sys") && !has_call("dispatch:1026"));
    assert(MsgStatus == 0);
}

static void test_do_task_special_message() {
    reset_fixture();
    queue_messages.push_back(0x0402);
    assert(do_task() == 1);
    assert(has_call("sys"));
    assert(!has_call("dispatch:1026") && !has_call("translate:1026"));
}

static void test_null_network_dispatch() {
    reset_fixture();
    ActiveNet = nullptr;
    queue_messages.push_back(0x0402);
    assert(do_task() == 1);
    assert(!has_call("sys"));
    assert(!has_call("translate:1026") && !has_call("dispatch:1026"));
}

static void test_wait_task_first_poll_range_and_initial_order() {
    reset_fixture();
    queue_messages.push_back(0x000A);
    queue_messages.push_back(0x000F);
    wait_task();
    assert(calls.size() >= 7);
    assert(calls[0] == "video" && calls[1] == "net" && calls[2] == "check");
    assert(calls[3] == "video" && calls[4] == "check" && calls[5] == "net");
    assert(calls[6] == "peek:15:15");
    assert(has_call("dispatch:15") && has_call("dispatch:10"));
    assert(!has_call("sys") && !has_call("dispatch:1026"));
    assert(MsgStatus == 0);
}

static void test_wait_task_special_message() {
    reset_fixture();
    queue_messages.push_back(0x0402);
    wait_task();
    assert(has_call("sys"));
    assert(!has_call("dispatch:1026") && !has_call("translate:1026"));
}

static void test_wait_task_null_network() {
    reset_fixture();
    ActiveNet = nullptr;
    queue_messages.push_back(0x0402);
    wait_task();
    assert(!has_call("sys"));
    assert(!has_call("translate:1026") && !has_call("dispatch:1026"));
}

static void test_do_all_tasks_queue_cleanup_and_special_message() {
    reset_fixture();
    queue_messages.push_back(0x000F);
    queue_messages.push_back(0x0200);
    queue_messages.push_back(0x0402);
    watch_do_net_status = true;
    mutate_msg_status_in_do_net = true;
    do_all_tasks();
    assert(has_call("dispatch:15"));
    assert(has_call("dispatch:512"));
    assert(has_call("sys"));
    assert(!has_call("dispatch:1026") && !has_call("translate:1026"));
    assert(!queue_messages.size());
    assert(calls.back() == "net");
    assert(calls[calls.size() - 2] == "peek:1024:65535");
    assert(last_do_net_status == 0);
    assert(MsgStatus == 77);
}

int main() {
    Technology[7].name = "TECH-7";
    Players[2].adj_name_faction = "FACTION-2";
    Players[3].adj_name_faction = "FACTION-3";
    ConsoleGlobal.faction_id_ = 2;

    test_tech_secret();
    test_do_task_special_message_and_poll_range();
    test_do_task_special_message();
    test_null_network_dispatch();
    test_wait_task_first_poll_range_and_initial_order();
    test_wait_task_special_message();
    test_wait_task_null_network();
    test_do_all_tasks_queue_cleanup_and_special_message();
}
