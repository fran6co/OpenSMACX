#include <assert.h>
#include <stdint.h>
#include <vector>
#ifndef __cdecl
#define __cdecl
#endif

struct BasePopTextBlock {
    int field_104_;
    int field_108_;
};
struct GameDataFields { uint32_t game_preferences_; };
struct GameDataOwner { GameDataFields fields_; };
static GameDataOwner GameDataStateStorage;

struct Base { int queue_production_id[1]; };
static Base TestBase;
static Base *BaseCurrent = &TestBase;
static const int FacilitySPStart = 70;
static int PopupWaveLastIndex;
static int ClockValue;
static bool ClearPreferencesAtClock;
static std::vector<int> Events;

struct Wave {
    int id;
    int playing;
    int load_count;
    int play_count;
    int is_playing() {
        Events.push_back(1000 + id);
        return playing;
    }
    void load() { ++load_count; Events.push_back(2000 + id); }
    void play() { ++play_count; Events.push_back(3000 + id); }
};
static Wave g_WAVE_GENERAL{-1, 0, 0, 0};
static Wave g_CPU_WAVES[45];

struct FactionAmbience {
    virtual void production() { Events.push_back(4000); }
    virtual void general() { Events.push_back(5000); }
};
struct FactionAmbienceSpy : FactionAmbience {
    int production_calls = 0;
    int general_calls = 0;
    void production() override { ++production_calls; Events.push_back(4000); }
    void general() override { ++general_calls; Events.push_back(5000); }
};
static FactionAmbience *FactionMusic;

struct FX {
    int play_count;
    int last_effect;
    void play(int effect) {
        ++play_count;
        last_effect = effect;
        Events.push_back(6000 + effect);
    }
};
static FX g_FX;

static uint32_t timeGetTime() {
    if (ClearPreferencesAtClock)
        GameDataStateStorage.fields_.game_preferences_ &= ~0x400u;
    return static_cast<uint32_t>(ClockValue);
}

/* ACTUAL_POPUP_WAVE_CALLBACK */

static FactionAmbienceSpy MusicSpy;
static BasePopTextBlock Block;

static void reset_state() {
    Events.clear();
    GameDataStateStorage.fields_.game_preferences_ = 0x400;
    TestBase.queue_production_id[0] = 0;
    PopupWaveLastIndex = 2;
    ClockValue = 0;
    ClearPreferencesAtClock = false;
    MusicSpy.production_calls = 0;
    MusicSpy.general_calls = 0;
    FactionMusic = &MusicSpy;
    g_WAVE_GENERAL = Wave{-1, 0, 0, 0};
    for (int i = 0; i < 45; ++i) g_CPU_WAVES[i] = Wave{i, 0, 0, 0};
    g_FX = FX{0, 0};
    Block = BasePopTextBlock{0, 1};
}

int main() {
    // The preference, null, negative-index, unarmed and out-of-range gates.
    reset_state();
    GameDataStateStorage.fields_.game_preferences_ = 0;
    Block.field_104_ = 19;
    popup_wave_callback(&Block, 0);
    assert(Events.empty() && MusicSpy.production_calls == 0);
    reset_state();
    popup_wave_callback(nullptr, 0);
    assert(Events.empty());
    reset_state();
    Block.field_104_ = -1;
    popup_wave_callback(&Block, 0);
    assert(Events.empty());
    reset_state();
    Block.field_108_ = 0;
    popup_wave_callback(&Block, 0);
    assert(Events.empty());
    reset_state();
    Block.field_104_ = 45;
    popup_wave_callback(&Block, 0);
    assert(Events.empty());

    // Secret-project remap changes 0x19 to 0x25 before later dispatch gates.
    reset_state();
    Block.field_104_ = 0x19;
    TestBase.queue_production_id[0] = -FacilitySPStart - 1;
    popup_wave_callback(&Block, 0);
    assert(Block.field_104_ == 0x25);
    assert(g_CPU_WAVES[0x25].load_count == 1 && g_CPU_WAVES[0x25].play_count == 1);
    assert(MusicSpy.production_calls == 0 && MusicSpy.general_calls == 0);

    // Even when audio is busy, the original wave-0x19 event still dispatches.
    reset_state();
    Block.field_104_ = 0x19;
    g_WAVE_GENERAL.playing = 1;
    popup_wave_callback(&Block, 0);
    assert(g_CPU_WAVES[0x19].load_count == 0 && g_CPU_WAVES[0x19].play_count == 0);
    assert(PopupWaveLastIndex == 2);
    assert(MusicSpy.production_calls == 1 && MusicSpy.general_calls == 0);

    reset_state();
    Block.field_104_ = 0x19;
    g_CPU_WAVES[PopupWaveLastIndex].playing = 1;
    popup_wave_callback(&Block, 0);
    assert(g_CPU_WAVES[0x19].load_count == 0 && g_CPU_WAVES[0x19].play_count == 0);
    assert(PopupWaveLastIndex == 2);
    assert(MusicSpy.production_calls == 1 && MusicSpy.general_calls == 0);

    // The owner-null check suppresses only its event; wave playback remains.
    reset_state();
    Block.field_104_ = 0x19;
    FactionMusic = 0;
    popup_wave_callback(&Block, 0);
    assert(g_CPU_WAVES[0x19].load_count == 1 && g_CPU_WAVES[0x19].play_count == 1);
    assert(MusicSpy.production_calls == 0 && MusicSpy.general_calls == 0);

    // Idle audio preserves call ordering: tests, load/play, then production.
    reset_state();
    Block.field_104_ = 0x19;
    popup_wave_callback(&Block, 0);
    assert(g_CPU_WAVES[0x19].load_count == 1 && g_CPU_WAVES[0x19].play_count == 1);
    assert(PopupWaveLastIndex == 0x19);
    assert(MusicSpy.production_calls == 1 && MusicSpy.general_calls == 0);
    assert(Events.size() == 5);
    assert(Events[0] == 999 && Events[1] == 1002);
    assert(Events[2] == 2025 && Events[3] == 3025 && Events[4] == 4000);

    // Clock rejection for index 0x2B returns before any wave query; an allowed
    // clock can clear the preference before the second sound gate is read.
    reset_state();
    Block.field_104_ = 0x2B;
    ClockValue = 0;
    popup_wave_callback(&Block, 0);
    assert(Events.empty());
    reset_state();
    Block.field_104_ = 0x2B;
    ClockValue = 1;
    popup_wave_callback(&Block, 0);
    assert(g_CPU_WAVES[0x2B].load_count == 1 && g_CPU_WAVES[0x2B].play_count == 1);
    assert(PopupWaveLastIndex == 0x2B && MusicSpy.production_calls == 0);
    reset_state();
    Block.field_104_ = 0x2B;
    ClockValue = 1;
    ClearPreferencesAtClock = true;
    popup_wave_callback(&Block, 0);
    assert(g_CPU_WAVES[0x2B].load_count == 0 && g_CPU_WAVES[0x2B].play_count == 0);
    assert(PopupWaveLastIndex == 2 && MusicSpy.production_calls == 0);

    // Effect 0x38 remains independent of the busy-sound branch.
    reset_state();
    Block.field_104_ = 0x10;
    g_WAVE_GENERAL.playing = 1;
    popup_wave_callback(&Block, 0);
    assert(g_FX.play_count == 1 && g_FX.last_effect == 0x38);
    assert(g_CPU_WAVES[0x10].load_count == 0 && g_CPU_WAVES[0x10].play_count == 0);
    assert(MusicSpy.production_calls == 0 && MusicSpy.general_calls == 0);
    return 0;
}
