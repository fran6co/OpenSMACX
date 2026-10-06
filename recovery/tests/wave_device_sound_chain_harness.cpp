// Actual Sound/Wave_Device declarations, chain operations and switch body.
// Unrelated virtuals, construction and the external driver are explicit doubles.
#include <assert.h>
#include <string.h>
#include <new>
#define nullptr 0
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned char uint8_t;
class OriginalObject {};
#define ORIGINAL(object) reinterpret_cast<OriginalObject *>(object)
#define protected public
// LIVE_DEVICE_BASE_DECL
// LIVE_FADE_DECLARATION
// LIVE_SOUND_DECLARATION
#undef protected
// LIVE_CHAIN_STORAGE
// LIVE_SOUND_DOUBLES
// LIVE_FADE_CONSTRUCTOR
// LIVE_ATTACH
// LIVE_DETACH
// LIVE_DESTRUCTOR
// LIVE_NEXT
// LIVE_PREV

class Wave;
struct EAX_REVERB_PROPERTIES;
// LIVE_GROUP_TYPES
#define private public
// LIVE_DEVICE_DECLARATION
#undef private
// LIVE_SLOT_DISPATCH
SoundDeviceBase::SoundDeviceBase()
    : field_4_(0), volume_8_(0), field_C_(0), field_10_(0), device_14_(0),
      field_18_(0), field_1C_(0) {}
SoundDeviceBase::~SoundDeviceBase() {}
Wave_Device::Wave_Device() { device_14_ = 0; }
Wave_Device::~Wave_Device() {}
int Wave_Device::init(void *, unsigned long) { return 0; }
int Wave_Device::release() { return 0; }
int Wave_Device::get_ndevices() { return 0; }
int Wave_Device::get_description(unsigned long, char *, unsigned long) { return 0; }
int Wave_Device::create_device(unsigned long) { assert(false); return 0; }
int Wave_Device::delete_device() { assert(false); return 0; }
void Wave_Device::unk_slot2() { assert(false); }
// LIVE_SELECT

static int events[64], event_count;
static void event(int value) { assert(event_count < 64); events[event_count++] = value; }
static const char normal_name[] = "normal";
static const char changed_name[] = "changed by previous-node callback";
static Wave_Device *switching_device;
static void *replacement_device;

class TestSound : public Sound {
 public:
    int id, first_attributes, later_attributes, queries, unloads, loads;
    bool rename_on_previous;
    TestSound(int identity, int first, int later) : id(identity),
        first_attributes(first), later_attributes(later), queries(0), unloads(0), loads(0),
        rename_on_previous(false) { fname_ = const_cast<char *>(normal_name); }
    ~TestSound() { fname_ = 0; } // Names are borrowed by this test double.
    int get_game_hwnd() { return 0; }
    int set_attrib(unsigned long) { return 11; }
    int get_attrib() {
        event(10 + id);
        return ++queries == 1 ? first_attributes : later_attributes;
    }
    Sound *get_next() { event(20 + id); return chain_next_; }
    Sound *get_prev() {
        event(50 + id);
        if (rename_on_previous) fname_ = const_cast<char *>(changed_name);
        return chain_prev_;
    }
    int unload() {
        event(30 + id); ++unloads;
        if (id == 4) switching_device->device_14_ = replacement_device;
        return Sound::detach();
    }
    int load(const char *name) {
        event(60 + id);
        assert(chain_prev_ == 0 && chain_next_ == 0);
        assert(name == (rename_on_previous ? changed_name : normal_name));
        ++loads;
        return Sound::attach();
    }
};
class AnotherSound : public TestSound {
 public: AnotherSound(int id, int first, int later) : TestSound(id, first, later) {}
};
static Sound *remaining_head, *remaining_tail;
static int driver_switches;
class WrappedDevice {
 public:
    virtual void slot0() { assert(false); }
    virtual void slot1() { assert(false); }
    virtual void slot2() { assert(false); }
    virtual void slot3() { assert(false); }
    virtual void slot4() { assert(false); }
    virtual void slot5() { assert(false); }
    virtual int select(unsigned long output) {
        assert(this == replacement_device);
        assert(output == 7 && SoundChainHead == remaining_head && SoundChainTail == remaining_tail);
        ++driver_switches;
        event(40);
        return 23; // The actual wrapper deliberately ignores this result.
    }
};

int main() {
    Wave_Device device;
    assert(device.select(7) == 2 && event_count == 0);
    WrappedDevice driver, replacement;
    switching_device = &device;
    replacement_device = &replacement;
    device.device_14_ = &driver;
    TestSound first(1, 1, 1), protected_sound(2, 1, 5);
    AnotherSound idle(3, 0, 0), last(4, 1, 1);
    last.rename_on_previous = true;
    first.attach(); protected_sound.attach(); idle.attach(); last.attach();
    remaining_head = &protected_sound;
    remaining_tail = &idle;
    assert(device.select(7) == 0 && driver_switches == 1);
    const int expected[] = {11,11,21,31,12,12,22,13,23,14,14,24,34,40,54,64,51,61};
    assert(event_count == sizeof(expected)/sizeof(expected[0]));
    assert(!memcmp(events, expected, sizeof(expected)));
    assert(first.loads == 1 && last.loads == 1 && first.unloads == 1 && last.unloads == 1);
    assert(protected_sound.unloads == 0 && idle.unloads == 0);
    assert(protected_sound.queries == 2 && idle.queries == 1);
    assert(SoundChainHead == &protected_sound && SoundChainTail == &first);
    assert(protected_sound.chain_next_ == &idle && idle.chain_next_ == &last);
    assert(last.chain_next_ == &first && first.chain_prev_ == &last);
    while (SoundChainHead) SoundChainHead->detach();
    remaining_head = remaining_tail = 0;
    event_count = 0;
    assert(device.select(7) == 0 && driver_switches == 2 && event_count == 1 && events[0] == 40);
    return 0;
}
