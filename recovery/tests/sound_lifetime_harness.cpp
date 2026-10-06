// Actual declarations, constructor and fade setters. Other virtual methods
// are explicit link doubles: this checks initialization, not audio playback.
#include <assert.h>
#include <string.h>
#include <new>
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned char uint8_t;

class OriginalObject {};
#define ORIGINAL(object) reinterpret_cast<OriginalObject *>(object)
template<class T> T original_slot(void *) { assert(false); return T(); }

#define protected public
#define private public
// LIVE_FADE_DECLARATION
// LIVE_SOUND_DECLARATION
struct _MMIOINFO;
// LIVE_VOICE_DECLARATION
#undef private
#undef protected

// LIVE_FADE_INTERVAL
// LIVE_ROOT_FADE
// LIVE_SOUND_FADE
// LIVE_SOUND_CONSTRUCTOR
// LIVE_NEXT
// LIVE_PREV
// LIVE_TYPE
// LIVE_SET_TYPE
// LIVE_VOICE_CONSTRUCTOR

// These definitions only make the actual declaration's vtable linkable.
Sound::~Sound() {}
int Sound::unk_slot24() { return 0; }
int Sound::set_attrib(unsigned long) { assert(false); return 0; }
int Sound::get_attrib() { assert(false); return 0; }
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
int Sound::get_game_hwnd() { assert(false); return 0; }
void Sound::set_volume(int) { assert(false); }
void Sound::set_pan(int) { assert(false); }
void Sound::set_loop_state(long) { assert(false); }
void Sound::set_delay(unsigned int) { assert(false); }
int Sound::set_fade_in(unsigned int) { assert(false); return 0; }
int Sound::is_looping() { assert(false); return 0; }
int Sound::is_playing() { assert(false); return 0; }
int Sound::get_time() { assert(false); return 0; }
int Sound::attach() { assert(false); return 0; }
int Sound::detach() { assert(false); return 0; }
VoiceTx::~VoiceTx() { assert(device_ == 0 && fname_ == 0); }
int VoiceTx::unload() { assert(false); return 0; }
int VoiceTx::stop() { assert(false); return 0; }
int VoiceTx::release() { assert(false); return 0; }
int VoiceTx::init(unsigned long, void *, unsigned long) { assert(false); return 0; }
_MMIOINFO *VoiceTx::get_next_buffer() { assert(false); return 0; }
int VoiceTx::return_buffer(struct _MMIOINFO *) { assert(false); return 0; }
int VoiceTx::start() { assert(false); return 0; }
int VoiceTx::get_nbuffers() { assert(false); return 0; }

int main() {
    union Storage { double alignment; unsigned char bytes[sizeof(Sound) + 8]; } storage;
    memset(storage.bytes, 0xA5, sizeof(storage.bytes));
    Sound *sound = new (storage.bytes) Sound;
    assert(sizeof(SoundFadeState) == 0x3C && sizeof(Sound) == 0x54);
    assert(sound->volume_ == 127 && sound->pan_8_ == 0);
    for (int i = 0; i < 9; ++i) assert(sound->state_C_[i] == 0);
    assert(sound->loop_flag_30_ == 0);
    const unsigned char untouched_delay[4] = {0xA5, 0xA5, 0xA5, 0xA5};
    assert(!memcmp(&sound->delay_, untouched_delay, sizeof(untouched_delay)));
    assert(sound->fade_38_ == 1000 && sound->device_ == 0 && sound->flags_40_ == 0);
    assert(sound->get_next() == 0 && sound->get_prev() == 0);
    assert(sound->fname_ == 0 && sound->get_type() == 0);
    for (unsigned int j = sizeof(Sound); j < sizeof(storage.bytes); ++j)
        assert(storage.bytes[j] == 0xA5);

    assert(sound->set_fade_interval(0) == 10 && sound->fade_38_ == 1000);
    assert(sound->set_fade_interval(0xFFFFFFFFUL) == 0 && sound->fade_38_ == 0xFFFFFFFFUL);
    assert(sound->SoundFadeState::set_fade(0) == 10 && sound->fade_38_ == 0xFFFFFFFFUL);
    assert(sound->SoundFadeState::set_fade(17) == 0 && sound->fade_38_ == 17);
    assert(sound->set_fade(23) == 0 && sound->fade_38_ == 23);
    sound->~Sound();

    union VoiceStorage { double alignment; unsigned char bytes[sizeof(VoiceTx) + 8]; } voice_storage;
    memset(voice_storage.bytes, 0xA5, sizeof(voice_storage.bytes));
    VoiceTx *voice = new (voice_storage.bytes) VoiceTx;
    assert(sizeof(VoiceTx) == 0x58 && voice->flags_54_ == 0);
    assert(voice->device_ == 0 && voice->get_next() == 0 && voice->get_prev() == 0);
    assert(!memcmp(&voice->delay_, untouched_delay, sizeof(untouched_delay)));
    assert(voice->fade_38_ == 1000 && voice->get_type() == 7 && voice->flags_40_ == 0x80);
    assert(!strcmp(static_cast<const char *>(voice->fname_), "Voice Tx"));
    for (unsigned int k = sizeof(VoiceTx); k < sizeof(voice_storage.bytes); ++k)
        assert(voice_storage.bytes[k] == 0xA5);
    ::operator delete(voice->fname_);
    voice->fname_ = 0;
    voice->~VoiceTx();
    return 0;
}
