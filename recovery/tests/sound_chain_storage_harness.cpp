// Real declarations, chain storage and attach/detach/destruction bodies.
// Constructor, unrelated virtuals and DLL release are explicit test doubles.
#include <assert.h>
#include <string.h>
#include <new>
#define nullptr 0
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned char uint8_t;
typedef void *HMODULE;
// LIVE_EXPORT_ABIS
#define protected public
// LIVE_FADE_DECLARATION
// LIVE_SOUND_DECLARATION
#undef protected
// LIVE_CHAIN_STORAGE
// LIVE_EXPORT_STORAGE

// SOUND_LINK_DOUBLES_BEGIN
Sound::Sound() : device_(0), flags_40_(0), chain_prev_(0), chain_next_(0), fname_(0), type_(0) {}
int SoundFadeState::set_fade(unsigned long) { assert(false); return 0; }
int SoundFadeState::set_fade_interval(unsigned long) { assert(false); return 0; }
int Sound::set_fade(unsigned long) { assert(false); return 0; }
int Sound::unk_slot24() { assert(false); return 0; }
int Sound::UNK1(int) { assert(false); return 0; }
int Sound::load(const char *) { assert(false); return 0; }
int Sound::unload() { assert(false); return 0; }
int Sound::play() { assert(false); return 0; }
int Sound::play(unsigned int) { assert(false); return 0; }
int Sound::stop() { assert(false); return 0; }
int Sound::fade() { assert(false); return 0; }
void Sound::fade(unsigned long) { assert(false); }
int Sound::fade_in() { assert(false); return 0; }
void Sound::fade_in(unsigned int) { assert(false); }
void Sound::ramp(int, int, unsigned int) { assert(false); }
int Sound::release() { assert(false); return 0; }
void Sound::set_volume(int) { assert(false); }
void Sound::set_pan(int) { assert(false); }
void Sound::set_loop_state(long) { assert(false); }
void Sound::set_delay(unsigned int) { assert(false); }
void Sound::set_type(unsigned int) { assert(false); }
uint32_t Sound::get_type() { assert(false); return 0; }
int Sound::set_fade_in(unsigned int) { assert(false); return 0; }
int Sound::is_looping() { assert(false); return 0; }
int Sound::is_playing() { assert(false); return 0; }
int Sound::get_time() { assert(false); return 0; }
static int releases;
static int __cdecl release_device(void *device) {
    assert(device == reinterpret_cast<void *>(17));
    ++releases;
    return 0x61;
}
int WaveDeviceReleaseGuard;
// SOUND_LINK_DOUBLES_END

// LIVE_FADE_CONSTRUCTOR
// LIVE_ATTACH
// LIVE_DETACH
// LIVE_DESTRUCTOR
// LIVE_NEXT
// LIVE_PREV

class OtherSound : public Sound {
 public:
    int get_game_hwnd() { return 0; }
    int set_attrib(unsigned long) { return 11; }
    int get_attrib() { return 0; }
};

int main() {
    SoundDllProcs[1] = reinterpret_cast<void *>(&release_device);
    assert(SoundChainHead == 0 && SoundChainTail == 0);
    OtherSound first, middle, last;
    first.flags_40_ = middle.flags_40_ = last.flags_40_ = 0x81;
    assert(first.attach() == 0 && SoundChainHead == &first && SoundChainTail == &first);
    assert(first.flags_40_ == 0x83 && first.get_next() == 0 && first.get_prev() == 0);
    assert(middle.attach() == 0 && last.attach() == 0);
    assert(SoundChainHead == &first && SoundChainTail == &last);
    assert(first.get_next() == &middle && middle.get_prev() == &first);
    assert(middle.get_next() == &last && last.get_prev() == &middle);
    assert(middle.attach() == 0 && SoundChainTail == &last); // Already linked: no duplication.
    assert(middle.detach() == 0 && middle.flags_40_ == 0x81);
    assert(middle.get_next() == 0 && middle.get_prev() == 0);
    assert(first.get_next() == &last && last.get_prev() == &first);
    assert(last.detach() == 0 && SoundChainTail == &first && first.get_next() == 0);
    assert(middle.detach() == 0 && SoundChainHead == &first); // Unchained no-op.
    assert(first.detach() == 0 && SoundChainHead == 0 && SoundChainTail == 0);

    first.attach();
    OtherSound *doomed = new OtherSound;
    doomed->attach();
    last.attach();
    WaveDeviceReleaseGuard = 1;
    doomed->device_ = reinterpret_cast<void *>(17);
    delete doomed;
    assert(releases == 1 && SoundChainHead == &first && SoundChainTail == &last);
    assert(first.get_next() == &last && last.get_prev() == &first);
    first.detach();
    assert(SoundChainHead == &last && SoundChainTail == &last && last.get_prev() == 0);
    last.detach();
    assert(SoundChainHead == 0 && SoundChainTail == 0);
    return 0;
}
