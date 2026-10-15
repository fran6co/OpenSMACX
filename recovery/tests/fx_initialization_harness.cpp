// The actual FX initializer and declaration, with explicit audio-call doubles.
// Expected calls are decoded afresh from the pinned original by the test.
#include <assert.h>
#include <string.h>
typedef unsigned int uint32_t;
class Sound;
// LIVE_DEFAULTS_DECLARATION
void observe(const char *, Sound *, const char *, int);
int call_result;
class Sound {
 public:
  void set_pan(int value) { observe("pan", this, "", value); }
};
class Wave : public Sound {
 public:
  int load(char *name, unsigned long flags) { observe("load", this, name, flags); return call_result; }
  void init(char *name, unsigned long flags) { observe("wave_init", this, name, flags); }
  void set_volume(int value) { observe("volume", this, "", value); }
  virtual int set_pitch(int value) { observe("pitch", this, "", value); return call_result; }
};
class Effect : public Wave {
 public:
  void init(char *name, long flags) { observe("effect_init", this, name, flags); }
  int set_pitch(int value) { observe("virtual_pitch", this, "", value); return call_result; }
};
class Wave_Device {
 public:
  int add_to_group(unsigned int group, Wave *wave);
};
Wave_Device WaveDeviceGlobal;
Wave g_OK_WAVE, g_ALPHAMENU_WAVE, g_SCOOT_WAVE, g_PASSOVER_WAVE;
Wave g_MENU_UP_WAVE, g_MENU_DOWN_WAVE, g_CPU_WAVES[45];

// LIVE_TEXT_BLOCK
typedef void (__cdecl *BasePopConfigCallback)(BasePopTextBlock *, int);
void __cdecl popup_wave_callback(BasePopTextBlock *, int) { assert(false); }
BasePopConfigCallback BasePopDefaultField3094;
struct PopupDefaults { uint32_t before, words[70], after; } popup_defaults;
struct AudioDefaults {
  uint32_t before;
  AutoSoundDefaultsBlock words;
  uint32_t after;
} audio_defaults;
#define BasePopDefaultBlock2E64 popup_defaults.words
#define AutoSoundDefaults audio_defaults.words

#define private public
// LIVE_FX_DECLARATION
#undef private
FX::FX() {}
FX::~FX() {}
FX *current_bank;
// LIVE_DEFAULT_VALUE
// LIVE_INITIALIZER

struct ExpectedCall {
    const char *op;
    int family, index;
    const char *name;
    int value, reset;
};
const ExpectedCall expected[] = {
// ORIGINAL_CALL_CONTRACT
};
int next_call;

void observe(const char *op, Sound *sound, const char *name, int value) {
    assert(next_call < sizeof(expected) / sizeof(expected[0]));
    const ExpectedCall &call = expected[next_call++];
    assert(!strcmp(op, call.op) && !strcmp(name, call.name) && value == call.value);
    Wave *globals[] = {&g_OK_WAVE, &g_ALPHAMENU_WAVE, &g_SCOOT_WAVE,
                      &g_PASSOVER_WAVE, &g_MENU_UP_WAVE, &g_MENU_DOWN_WAVE};
    Sound *receiver = call.family == 1 ? &current_bank->effects_[call.index] :
                      call.family == 2 ? &g_CPU_WAVES[call.index] : globals[call.index];
    assert(sound == receiver);
    assert(current_bank->engine_effect_index_ == -1);
    assert(BasePopDefaultField3094 == popup_wave_callback);
    // Independent original block contents, not the candidate constant itself.
    BasePopTextBlock block = {"", -1, 127, 50, 0, 0};
    assert(!memcmp(BasePopDefaultBlock2E64, &block, sizeof(block)));
    const unsigned char *audio_bytes =
        reinterpret_cast<const unsigned char *>(&AutoSoundDefaults);
    for (unsigned int i = 0; i < sizeof(AutoSoundDefaults); ++i)
        assert(audio_bytes[i] == (call.reset ? 0 : 0xab));
    assert(popup_defaults.before == 0xabababab && popup_defaults.after == 0xabababab);
    assert(audio_defaults.before == 0xabababab && audio_defaults.after == 0xabababab);
}

int Wave_Device::add_to_group(unsigned int group, Wave *wave) {
    assert(this == &WaveDeviceGlobal);
    observe("group", wave, "", group);
    return call_result;
}

int main() {
    FX bank;
    current_bank = &bank;
    // Original ignores the callees' return values: both runs must complete.
    for (int iteration = 0; iteration < 2; ++iteration) {
        call_result = iteration ? -11 : 0;
        memset(&popup_defaults, 0xab, sizeof(popup_defaults));
        memset(&audio_defaults, 0xab, sizeof(audio_defaults));
        BasePopDefaultField3094 = 0;
        bank.engine_effect_index_ = 17;
        next_call = 0;
        bank.init();
        assert(next_call == sizeof(expected) / sizeof(expected[0]));
    }
    return 0;
}
