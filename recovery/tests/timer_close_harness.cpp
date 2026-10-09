// Actual Time declarations and definitions are inserted by pytest.
// OS cancellation and message draining are controlled doubles.
#include <assert.h>
#include <string.h>
#include <new>
typedef unsigned int uint32_t;
typedef unsigned int UINT_PTR;
typedef unsigned int DWORD_PTR;
typedef unsigned long DWORD;
typedef void *HWND;
#define CALLBACK __stdcall
#define MEASURED
#define UNUSED(name)
#define nullptr 0
const int TimeOneShot = 1;
const int TimeOneShotFired = 2;
HWND HandleMain = (HWND)0x1234;
void flush_timer();
unsigned int timeKillEvent(unsigned int);
int KillTimer(HWND, UINT_PTR);
#define private public
// LIVE_DECL
#undef private
// LIVE_HELPER
// LIVE_CTOR
// LIVE_CLOSE
// LIVE_DTOR

static Time *active;
static int calls[8], count;
static int expected_flush;
void callback1(int) {}
void callback2(int, void *) {}
unsigned int timeKillEvent(unsigned int id) {
    assert(active->id_event_ == 17 && id == 17);
    calls[count++] = 1;
    // stop must use the pre-cancellation one-shot state, not reread it.
    active->oneshot_state_ ^= TimeOneShot;
    return 0;
}
int KillTimer(HWND window, UINT_PTR id) {
    assert(window == HandleMain && active->id_event_ == 17 && id == 17);
    calls[count++] = 2;
    active->oneshot_state_ ^= TimeOneShot;
    return 1;
}
void flush_timer() {
    assert(expected_flush && active->id_event_ == 0);
    assert(active->resolution_ == 71 && active->tick_posted_ == 19);
    calls[count++] = 3;
    // Cleanup must clear state even if message processing changes it.
    active->cb_param1_ = 99;
    active->tick_posted_ = 27;
}

int main() {
    assert(sizeof(Time) == 0x28);
    for (int destructor = 0; destructor != 2; ++destructor) {
        for (int one_shot = 0; one_shot != 2; ++one_shot) {
            for (int mode = 0; mode != 4; ++mode) {
                Time timer;
                active = &timer;
                timer.oneshot_state_ = one_shot;
                timer.id_event_ = mode ? 17 : 0;
                timer.count_ = mode == 1 ? 49 : mode == 3 ? 0x80000000U : 50;
                timer.callback1_ = callback1;
                timer.callback2_ = callback2;
                int callback_context = 0;
                timer.cb_param2_ = &callback_context;
                timer.unk_2_ = -1;
                timer.resolution_ = 71;
                timer.tick_posted_ = 19;
                timer.cb_param1_ = 42;
                expected_flush = !one_shot;
                count = 0;
                if (destructor) timer.~Time(); else timer.close();
                assert(count == (mode != 0) + !one_shot);
                if (mode) assert(calls[0] == (mode == 1 ? 1 : 2));
                if (!one_shot) assert(calls[count - 1] == 3);
                // Inspect object representation, including after destruction.
                unsigned int words[10];
                memcpy(words, &timer, sizeof(words));
                for (int i = 0; i != 10; ++i) assert(words[i] == (i == 8 ? 5U : 0U));
                // Re-establish lifetime before the automatic destructor.
                if (destructor) new (&timer) Time;
                timer.oneshot_state_ = TimeOneShot;
                expected_flush = 0;
            }
        }
    }
    return 0;
}
