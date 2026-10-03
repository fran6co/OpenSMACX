#include <assert.h>
#include <string.h>

#if !defined(_MSC_VER)
#define __cdecl
#define __stdcall
#endif

struct RECT { int left, top, right, bottom; };

struct CallLog {
    int redraws;
    int visible;
    int info_draws;
    int updates;
    RECT *updated_rect;
    int synch_calls;
    short opcode;
    int a, b, c, d;
    char *text;
    short flags;
} log_state;

class StatusWin {
public:
    void redraw();
};
StatusWin ConsoleStatusWin;
class Win {
public:
    int is_visible();
};
class GraphicWin : public Win {
public:
    void soft_update(RECT *r);
};
class DiploPop : public GraphicWin {
public:
    void draw_info();
    RECT rect1_;
};
DiploPop DiplomacyPopup;
class NetDaemon {
public:
    void synch(short op, int aa, int bb, int cc, char *s, int dd, short f);
};
NetDaemon NetDaemonNet;

void StatusWin::redraw() { ++log_state.redraws; }
int Win::is_visible() { return log_state.visible; }
void GraphicWin::soft_update(RECT *r) {
    assert(log_state.visible != 0);
    assert(log_state.info_draws == log_state.updates + 1);
    ++log_state.updates;
    log_state.updated_rect = r;
}
void DiploPop::draw_info() {
    assert(log_state.visible != 0);
    assert(log_state.info_draws == log_state.updates);
    ++log_state.info_draws;
}
void NetDaemon::synch(short op, int aa, int bb, int cc, char *s, int dd, short f) {
    ++log_state.synch_calls;
    log_state.opcode = op; log_state.a = aa; log_state.b = bb;
    log_state.c = cc; log_state.text = s; log_state.d = dd;
    log_state.flags = f;
}

// ACTUAL_SOURCE_BODIES

int main() {
    sub_50f630(42);
    assert(log_state.redraws == 1);

    log_state.visible = 0;
    sub_539970();
    assert(log_state.info_draws == 0 && log_state.updates == 0);
    log_state.visible = 1;
    sub_539970();
    assert(log_state.info_draws == 1 && log_state.updates == 1);
    assert(log_state.updated_rect == &DiplomacyPopup.rect1_);

    sub_593370();
    assert(log_state.synch_calls == 1 && log_state.opcode == 1);
    assert(log_state.a == 0 && log_state.b == 0 && log_state.c == 0);
    assert(log_state.text == 0 && log_state.d == 1 && log_state.flags == 0x2101);
    sub_593480(7);
    assert(log_state.synch_calls == 2 && log_state.opcode == 0x17);
    assert(log_state.a == 7 && log_state.b == 0 && log_state.c == 0);
    assert(log_state.text == 0 && log_state.d == 1 && log_state.flags == 0x2101);

    char dest[8];
    memset(dest, 0x55, sizeof(dest));
    assert(sub_584d30(dest, const_cast<char *>("abcdef"), 4) == dest);
    assert(strcmp(dest, "abc") == 0);
    memset(dest, 0x55, sizeof(dest));
    assert(sub_584d30(dest, const_cast<char *>("xy"), 5) == dest);
    assert(dest[0] == 'x' && dest[1] == 'y' && dest[2] == 0 && dest[4] == 0);
    return 0;
}
