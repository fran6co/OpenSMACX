// Real Sound/Wave declarations and destructor/list bodies. Construction,
// unrelated virtuals, group removal and the DLL callback are explicit doubles.
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <new>
#define nullptr 0
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned char uint8_t;
#define protected public
#define private public
// LIVE_FADE_DECLARATION
// LIVE_SOUND_DECLARATION
// LIVE_WAVE_DECLARATION
#undef private
#undef protected
// LIVE_CHAIN_STORAGE

static int events[32], event_count, releases, name_frees, callback_frees;
static bool throw_group, throw_release, relink, replace_name;
static Sound *destroying;
static void *initial_name, *callback_name;
static void event(int n) { assert(event_count < 32); events[event_count++] = n; }
void __cdecl operator delete(void *p) {
    if (p && p == initial_name) { ++name_frees; event(20); }
    if (p && p == callback_name) { ++callback_frees; event(21); }
    free(p);
}

Sound::Sound() : device_(0), flags_40_(0), chain_prev_(0), chain_next_(0), fname_(0), type_(0) {}
Wave::Wave() : flags_54_(0), pitch_(0), reverb_mix_(1), ms_length_(0), start_time_(0), group_slot_(16) {}
int SoundFadeState::set_fade(unsigned long) { assert(false); return 0; }
int SoundFadeState::set_fade_interval(unsigned long) { assert(false); return 0; }
int Sound::set_fade(unsigned long) { assert(false); return 0; }
int Sound::UNK1(int) { assert(false); return 0; }
int Sound::load(const char *) { assert(false); return 0; }
int Sound::unload() { assert(false); return 0; }
int Sound::play() { return 33; } // Witness the active base destruction stage.
int Sound::play(unsigned int) { assert(false); return 0; }
int Sound::stop() { assert(false); return 0; }
int Sound::fade() { assert(false); return 0; }
void Sound::fade(unsigned long) { assert(false); }
int Sound::fade_in() { assert(false); return 0; }
void Sound::fade_in(unsigned int) { assert(false); }
void Sound::ramp(int, int, unsigned int) { assert(false); }
int Sound::release() { assert(false); return 0; }
int Sound::get_game_hwnd() { assert(false); return 0; }
void Sound::set_volume(int) { assert(false); }
void Sound::set_pan(int) { assert(false); }
void Sound::set_loop_state(long) { assert(false); }
void Sound::set_delay(unsigned int) { assert(false); }
void Sound::set_type(unsigned int) { assert(false); }
int Sound::set_fade_in(unsigned int) { assert(false); return 0; }
int Sound::is_looping() { assert(false); return 0; }
int Sound::is_playing() { assert(false); return 0; }
void Sound::unk_slot24() { assert(false); }
int Sound::set_attrib(unsigned long) { assert(false); return 0; }
int Sound::get_attrib() { assert(false); return 0; }
int Sound::get_time() { assert(false); return 0; }
uint32_t Sound::get_type() { assert(false); return 0; }
void Wave::set_volume(int) { assert(false); }
int Wave::unload() { assert(false); return 0; }
int Wave::is_playing() { assert(false); return 0; }
int Wave::get_game_hwnd() { assert(false); return 0; }
int Wave::get_attrib() { assert(false); return 0; }
int Wave::set_attrib(unsigned long) { assert(false); return 0; }
int Wave::reload() { assert(false); return 0; }
void Wave::init(char *, unsigned long) { assert(false); }
int Wave::load(char *, unsigned long) { assert(false); return 0; }
int Wave::load() { assert(false); return 0; }
int Wave::load(const char *) { assert(false); return 0; }
int Wave::play(int) { assert(false); return 0; }
int Wave::play() { return 77; }
int Wave::set_pitch(int) { assert(false); return 0; }
int Wave::set_bufflimit(unsigned int) { assert(false); return 0; }
int Wave::set_asdr() { assert(false); return 0; }
int Wave::set_attack(unsigned int,unsigned int,unsigned int) { assert(false); return 0; }
int Wave::set_sustain(unsigned int,unsigned int,unsigned int) { assert(false); return 0; }
int Wave::set_decay(unsigned int,unsigned int,unsigned int) { assert(false); return 0; }
int Wave::set_release(unsigned int,unsigned int,unsigned int) { assert(false); return 0; }
int Wave::get_time(uint32_t) { assert(false); return 0; }
int Wave::get_current_marker() { assert(false); return 0; }
int Wave::get_ndevices() { assert(false); return 0; }
int Wave::get_device_description(char *,int,int) { assert(false); return 0; }
int Wave::get_ms_length() { assert(false); return 0; }
int Wave::is_hwbuffer() { assert(false); return 0; }
int Wave::set_position3d(float,float,float) { assert(false); return 0; }
int Wave::set_xpos(float) { assert(false); return 0; }
int Wave::set_ypos(float) { assert(false); return 0; }
int Wave::set_zpos(float) { assert(false); return 0; }
int Wave::is_3d() { assert(false); return 0; }
int Wave::set_reverb_mix(float) { assert(false); return 0; }

class Wave_Device { public: int pull_from_group(Wave *); };
Wave_Device WaveDeviceGlobal;
int Wave_Device::pull_from_group(Wave *wave) {
    assert(wave == destroying && wave->group_slot_ < 16);
    assert(destroying->play() == 77 && destroying->fname_ == initial_name);
    event(10);
    if (throw_group) throw 10;
    wave->group_slot_ = 16;
    return 0;
}
int WaveDeviceReleaseGuard;
static void __cdecl release_device(void *p) {
    assert(p == reinterpret_cast<void *>(17));
    assert(destroying->play() == 33); // Original has published Sound's table.
    assert(name_frees == 1 && destroying->fname_ == 0);
    assert(bool(destroying->flags_40_ & 2) == throw_group);
    assert(destroying->device_ == p);
    ++releases; event(30);
    if (relink) destroying->Sound::attach();
    if (replace_name) destroying->fname_ = callback_name;
    if (throw_release) throw 30;
}
typedef void(__cdecl func_wave_device_release)(void *);
static func_wave_device_release *release_hook = release_device;
inline func_wave_device_release *&WaveDeviceReleaseSlot() { return release_hook; }

// LIVE_ATTACH
// LIVE_DETACH
// LIVE_NEXT
// LIVE_PREV
// LIVE_SOUND_DESTRUCTOR
// LIVE_WAVE_DESTRUCTOR

int main() {
    // Normal, no-group, guard-off, no-device, group-exception, and release-exception.
    for (int scenario = 0; scenario != 6; ++scenario) {
        event_count = releases = name_frees = callback_frees = 0;
        throw_group = scenario == 4; throw_release = scenario == 5;
        relink = scenario < 2; replace_name = scenario < 2;
        WaveDeviceReleaseGuard = scenario == 2 ? 0 : 1;
        initial_name = malloc(8); callback_name = malloc(8);
        assert(initial_name && callback_name);
        Sound before, after;
        Wave *wave = new Wave;
        destroying = wave;
        wave->fname_ = initial_name;
        wave->device_ = scenario == 3 ? 0 : reinterpret_cast<void *>(17);
        wave->group_slot_ = scenario == 1 ? 16 : 0;
        before.attach(); wave->attach(); after.attach();
        int caught = 0;
        try { wave->~Wave(); }
        catch (int n) { caught = n; }
        assert(caught == (throw_group ? 10 : throw_release ? 30 : 0));
        assert(name_frees == 1 && callback_frees == 0);
        assert(releases == ((scenario == 2 || scenario == 3) ? 0 : 1));
        assert(SoundChainHead == &before && SoundChainTail == &after);
        assert(before.chain_next_ == &after && after.chain_prev_ == &before);
        int i = 0;
        if (scenario != 1) assert(events[i++] == 10);
        assert(events[i++] == 20);
        if (releases) assert(events[i++] == 30);
        assert(event_count == i);
        before.detach(); after.detach();
        assert(SoundChainHead == 0 && SoundChainTail == 0);
        ::operator delete(wave); // Storage only; the destructor already ran.
        free(callback_name); // Callback result must not be cleaned a second time.
        initial_name = callback_name = 0;
    }
    return 0;
}
