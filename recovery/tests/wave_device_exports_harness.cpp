// Actual loader, genuine base/device declarations, source constructors and
// three lifecycle methods. DLL APIs and provider remain explicit host doubles.
#include <assert.h>
#include <string.h>
#include <new>
#define nullptr 0 // Same VC6 compatibility spelling as the product build.
#define static_assert(condition, message) \
    typedef char harness_static_assert_##__LINE__[(condition) ? 1 : -1]
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
typedef void *HWND;
typedef void *HMODULE;
typedef int (__stdcall *FARPROC)();
#define MAKEINTRESOURCEA(value) reinterpret_cast<const char *>(value)

class Wave;
struct EAX_REVERB_PROPERTIES;
struct IDirectSound;
// LIVE_DEVICE_BASE_DECLARATION
// LIVE_GROUP_TYPES
#define private public
// LIVE_DEVICE_DECLARATION
#undef private
// LIVE_ROLE_TYPES
// LIVE_SLOT_DISPATCH
}; // The source extraction deliberately stops before the view's closing token.
namespace SoundXDeviceAbi {
void release_device(void *device);
}
// LIVE_EXPORT_TYPE
// LIVE_TYPED_EXPORT_ACCESSORS
// LIVE_EXPORT_STORAGE
int WaveDeviceReleaseGuard;
HMODULE SoundDllModule;

// LIVE_BASE_CONSTRUCTOR
// LIVE_BASE_DESTRUCTOR
// LIVE_GROUP_CONSTRUCTOR
// LIVE_GROUP_DESTRUCTOR
// LIVE_WAVE_CONSTRUCTOR
// LIVE_GET_NDEVICES
// LIVE_GET_DESCRIPTION
// LIVE_INIT
// The exports harness never calls select; this test-only definition closes
// Wave_Device's inherited virtual surface without asserting product coverage.
int Wave_Device::select(unsigned long) { assert(false); return 0; }
Wave_Device::~Wave_Device() {}
void Wave_Device::unk_slot2() { assert(false); }

struct WaveDeviceProbe : Wave_Device {
    void *provider() const { return device_14_; }
    void provider(void *value) { device_14_ = value; }
    void **provider_slot() { return &device_14_; }
};

static WaveDeviceProbe *active;
static int creations, destructions, shutdowns, alternate_destructions;
static bool clear_device_on_shutdown, clear_hook_on_shutdown;
namespace SoundXDeviceAbi {
void release_device(void *) { assert(false); }
}
class WrappedDevice {
 public:
    virtual void slot0() { assert(false); }
    virtual void slot1() { assert(false); }
    virtual void slot2() { assert(false); }
    virtual void slot3() { assert(false); }
    virtual void shutdown() {
        ++shutdowns;
        if (clear_device_on_shutdown) active->provider(0);
        if (clear_hook_on_shutdown) SoundDllProcs[5] = 0;
    }
};
static WrappedDevice wrapped;

static int __cdecl factory(void **slot, unsigned long kind) {
    assert(slot == active->provider_slot() && kind == 7);
    ++creations;
    *slot = &wrapped;
    return 17;
}
static int __cdecl destroy() { ++destructions; return 0x41; }
static int __cdecl alternate_destroy() { ++alternate_destructions; return 0x42; }
static void __cdecl unused_export() {}
static unsigned int __cdecl version() { return 0x100; }
static int requested, fail_ordinal;
static HMODULE LoadLibraryA(const char *name) {
    assert(strcmp(name, ".\\soundx.dll") == 0);
    return reinterpret_cast<HMODULE>(0x1234);
}
static FARPROC GetProcAddress(HMODULE module, const char *ordinal) {
    assert(module == reinterpret_cast<HMODULE>(0x1234));
    int index = reinterpret_cast<int>(ordinal);
    assert(index == ++requested && index >= 1 && index <= 11);
    if (index == fail_ordinal) return 0;
    if (index == 5) return reinterpret_cast<FARPROC>(factory);
    if (index == 6) return reinterpret_cast<FARPROC>(destroy);
    if (index == 11) return reinterpret_cast<FARPROC>(version);
    return reinterpret_cast<FARPROC>(unused_export);
}
static int FreeLibrary(HMODULE module) {
    assert(module == reinterpret_cast<HMODULE>(0x1234));
    return 1;
}
extern "C" int __cdecl sound_header_version() { return 1; }
extern "C" void __cdecl sound_version_warning(const char *, const char *) { assert(false); }

// LIVE_LOADER
// LIVE_CREATE
// LIVE_DELETE
// LIVE_RELEASE

int main() {
    WaveDeviceProbe device;
    active = &device;
    assert(device.create_device(7) == 0x14 && creations == 0);
    assert(device.delete_device() == 0x14 && destructions == 0);
    assert(device.release() == 0 && shutdowns == 0);
    assert(load_sound_dll() == 0 && requested == 11);
    assert(device.create_device(7) == 17 && creations == 1 && device.provider() == &wrapped);
    assert(device.create_device(7) == 0xC && creations == 1);
    SoundDllProcs[5] = 0;
    assert(device.delete_device() == 0x14 && device.provider() == &wrapped);
    SoundDllProcs[5] = reinterpret_cast<void *>(destroy);
    assert(device.delete_device() == 0 && device.provider() == 0 && destructions == 1);
    assert(device.create_device(7) == 17);
    SoundDllProcs[5] = reinterpret_cast<void *>(alternate_destroy);
    assert(device.delete_device() == 0 && alternate_destructions == 1 && device.provider() == 0);

    SoundDllProcs[5] = reinterpret_cast<void *>(destroy);
    assert(device.create_device(7) == 17);
    assert(device.release() == 0 && shutdowns == 1 && destructions == 2 && device.provider() == 0);
    assert(device.create_device(7) == 17);
    clear_device_on_shutdown = true;
    assert(device.release() == 0 && shutdowns == 2 && destructions == 2 && device.provider() == 0);
    clear_device_on_shutdown = false;
    assert(device.create_device(7) == 17);
    clear_hook_on_shutdown = true;
    assert(device.release() == 0 && shutdowns == 3 && destructions == 2 && device.provider() == 0);
    clear_hook_on_shutdown = false;

    // A failed reload clears even previously populated factory/destroy slots.
    SoundDllModule = 0;
    requested = 0;
    fail_ordinal = 7;
    assert(load_sound_dll() == 1 && requested == 7);
    assert(SoundDllProcs[4] == 0 && SoundDllProcs[5] == 0);
    int before = creations;
    assert(device.create_device(7) == 0x14 && creations == before);
    device.provider(&wrapped);
    assert(device.delete_device() == 0x14 && device.provider() == &wrapped);
    assert(device.release() == 0 && shutdowns == 4 && destructions == 2 && device.provider() == 0);
    return 0;
}
