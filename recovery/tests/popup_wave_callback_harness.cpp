// Actual shared block, callback typedef and recovered callback body are inserted.
// Wave, FX, base state, clock and owner notification are explicit test doubles.
// This checks the callback's decisions, not its unrecovered dependency seams.
#include <assert.h>
#include <stddef.h>
#include <string.h>
typedef unsigned char uint8_t;
typedef int int32_t;

// LIVE_TEXT_BLOCK
// LIVE_CALLBACK_TYPE

int GamePreferences;
const int FacilitySPStart = 70;
struct BaseState { int queue_production_id[1]; } base_state;
BaseState *BaseCurrent = &base_state;

struct Wave {
    int busy, loads, plays;
    int is_playing() { return busy; }
    int load() { ++loads; return 1; }
    int play() { ++plays; return 1; }
};
Wave g_WAVE_GENERAL, g_CPU_WAVES[45];
struct FX {
    int calls, last;
    void play(int effect) { ++calls; last = effect; }
} g_FX;

unsigned long now;
// SDK-signature double: real import/operand identity is checked separately.
extern "C" unsigned long __stdcall timeGetTime() { return now; }

// The old owner seam remains in product code. Its interpretation is NOT
// certified by this double; only whether the callback requests notification.
int owner_calls;
class OriginalObject {
public:
    unsigned long *table;
    void notify() { ++owner_calls; }
};
unsigned long owner_table[79];
OriginalObject owner;
void *owner_pointer;
void *&PopupWaveOwnerSlot() { return owner_pointer; }
template <class Method> Method original_method(unsigned long) {
    return &OriginalObject::notify;
}
#define ORIGINAL(pointer) static_cast<OriginalObject *>(pointer)

// LIVE_CALLBACK
BasePopConfigCallback BasePopDefaultField3094 = popup_wave_callback;

void reset() {
    GamePreferences = 0x400;
    base_state.queue_production_id[0] = 0;
    memset(g_CPU_WAVES, 0, sizeof(g_CPU_WAVES));
    memset(&g_WAVE_GENERAL, 0, sizeof(g_WAVE_GENERAL));
    memset(&g_FX, 0, sizeof(g_FX));
    PopupWaveLastIndex = 0;
    owner_calls = 0;
    owner_pointer = 0;
    owner.table = owner_table;
    now = 1;
}

int total_plays() {
    int sum = 0;
    for (int i = 0; i < 45; ++i) sum += g_CPU_WAVES[i].plays;
    return sum;
}

int main() {
    assert(sizeof(BasePopTextBlock) == 0x118);
    BasePopTextBlock block;
    memset(&block, 0, sizeof(block));
    for (int enabled = 0; enabled < 2; ++enabled) {
        for (int armed = 0; armed < 2; ++armed) {
            for (int index = -1; index <= 45; ++index) {
                reset();
                GamePreferences = enabled ? 0x400 : 0;
                block.field_104_ = index;
                block.field_108_ = armed ? -7 : 0;
                BasePopDefaultField3094(&block, 1);
                const int plays = enabled && armed && index >= 0 && index < 45;
                assert(total_plays() == plays);
                assert(block.field_104_ == index);
                if (plays) {
                    assert(g_CPU_WAVES[index].loads == 1);
                    assert(PopupWaveLastIndex == index);
                }
            }
        }
    }
    reset();
    BasePopDefaultField3094(0, 0);
    assert(total_plays() == 0);
    block.field_108_ = 1;
    for (int mode = 0; mode < 3; ++mode) {
        reset();
        now = mode;
        block.field_104_ = 43;
        BasePopDefaultField3094(&block, 0);
        assert(total_plays() == (mode == 1));
    }
    for (int busy = 1; busy <= 2; ++busy) {
        reset();
        block.field_104_ = 16;
        g_WAVE_GENERAL.busy = busy == 1;
        g_CPU_WAVES[0].busy = busy == 2;
        BasePopDefaultField3094(&block, 0);
        assert(total_plays() == 0);
        assert(g_FX.calls == 1 && g_FX.last == 56);
    }
    reset();
    block.field_104_ = 25;
    owner_pointer = &owner;
    BasePopDefaultField3094(&block, 1);
    assert(owner_calls == 1 && g_CPU_WAVES[25].plays == 1);
    reset();
    block.field_104_ = 25;
    base_state.queue_production_id[0] = -FacilitySPStart - 1;
    owner_pointer = &owner;
    BasePopDefaultField3094(&block, 0);
    assert(block.field_104_ == 37);
    assert(g_CPU_WAVES[37].plays == 1 && owner_calls == 0);
    assert(block.field_10C_ == 0 && block.field_110_ == 0 && block.field_114_ == 0);
    return 0;
}
