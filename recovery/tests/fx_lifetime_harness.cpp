// Inserts the actual FX declaration and bodies. Effect is a lifetime/dispatch
// double: this does not exercise the real audio stack or establish game safety.
#include <assert.h>
typedef unsigned int uint32_t;

int attempts, live, destroyed, fail_at, last_id, last_operation;
struct Effect {
    int id;
    char reserved[104];
    Effect() {
        id = attempts++;
        if (id == fail_at) throw id;
        ++live;
    }
    ~Effect() {
        assert(id == live - 1);
        --live;
        ++destroyed;
    }
    void stop() { last_id = id; last_operation = 1; }
    void release() { last_id = id; last_operation = 2; }
    void fade() { last_id = id; last_operation = 3; }
};

// LIVE_FX_DECLARATION
// LIVE_FX_CONSTRUCTOR
// LIVE_FX_DESTRUCTOR
// LIVE_FX_STOP
// LIVE_FX_RELEASE
// LIVE_FX_FADE

void reset(int failure) {
    attempts = live = destroyed = 0;
    fail_at = failure;
    last_id = last_operation = -1;
}

int main() {
    reset(-1);
    {
        FX bank;
        assert(sizeof(bank) == 0x28F0);
        assert(attempts == 97 && live == 97 && destroyed == 0);
        bank.stop(96);
        assert(last_id == 96 && last_operation == 1);
        bank.release(0);
        assert(last_id == 0 && last_operation == 2);
        bank.fade(48);
        assert(last_id == 48 && last_operation == 3);
    }
    assert(live == 0 && destroyed == 97);

    // A failed element itself is not destroyed; only earlier complete
    // elements unwind, each once, in reverse construction order.
    for (int failure = 0; failure != 97; ++failure) {
        reset(failure);
        try {
            FX bank;
            assert(false);
        } catch (int index) {
            assert(index == failure);
        }
        assert(attempts == failure + 1 && live == 0 && destroyed == failure);
    }
    return 0;
}
