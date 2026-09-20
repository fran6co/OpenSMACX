// Actual loader and role accessors with DLL/API doubles, not sound-driver proof.
#include <cassert>
#include <cstring>

typedef void *HMODULE;
typedef int (__stdcall *FARPROC)();
#define MAKEINTRESOURCEA(value) reinterpret_cast<const char *>(value)

// LIVE_EXPORT_TYPE
typedef unsigned int (__cdecl *func_sound_dll_version)();
HMODULE SoundDllModule;
int WaveDeviceReleaseGuard;
// LIVE_EXPORT_STORAGE

typedef void (__cdecl func_wave_device_release)(void *);
// LIVE_RELEASE_SLOT
// LIVE_CREATE_SLOT

static int requested, loads, frees, warnings, fail_ordinal;
static int creates, releases, alternate_releases, fake_device;
static bool fail_load;
static HMODULE driver = reinterpret_cast<HMODULE>(0x1234);

static int __cdecl create_device(void **slot, const char *name, int mode) {
    assert(strcmp(name, "sample.wav") == 0 && mode == 1);
    ++creates;
    *slot = &fake_device;
    return 17;
}

static void __cdecl release_device(void *device) {
    assert(device == &fake_device);
    ++releases;
}

static void __cdecl alternate_release(void *device) {
    assert(device == &fake_device);
    ++alternate_releases;
}

static unsigned int __cdecl driver_version() { return 0x100; }
static void __cdecl unused_export() {}

static HMODULE LoadLibraryA(const char *name) {
    assert(strcmp(name, ".\\soundx.dll") == 0);
    ++loads;
    return fail_load ? 0 : driver;
}

static FARPROC GetProcAddress(HMODULE module, const char *ordinal) {
    assert(module == driver);
    int index = reinterpret_cast<int>(ordinal);
    assert(index == ++requested && index >= 1 && index <= 11);
    if (index == fail_ordinal) return 0;
    if (index == 1) return reinterpret_cast<FARPROC>(create_device);
    if (index == 2) return reinterpret_cast<FARPROC>(release_device);
    if (index == 11) return reinterpret_cast<FARPROC>(driver_version);
    return reinterpret_cast<FARPROC>(unused_export);
}

static int FreeLibrary(HMODULE module) {
    assert(module == driver);
    ++frees;
    return 1;
}

extern "C" int __cdecl sound_header_version() { return 1; }
extern "C" void __cdecl sound_version_warning(const char *, const char *) { ++warnings; }

// LIVE_LOADER

static void reset() {
    SoundDllModule = 0;
    memset(SoundDllProcs, 0, sizeof(SoundDllProcs));
    WaveDeviceReleaseGuard = 1;
    requested = loads = frees = warnings = fail_ordinal = 0;
    creates = releases = alternate_releases = 0;
    fail_load = false;
}

int main() {
    reset();
    assert(load_sound_dll() == 0);
    assert(requested == 11 && loads == 1 && frees == 0 && warnings == 0);
    assert(SoundDllModule == driver);
    assert(reinterpret_cast<void *>(&WaveDeviceCreateSlot()) == &SoundDllProcs[0]);
    assert(reinterpret_cast<void *>(&WaveDeviceReleaseSlot()) == &SoundDllProcs[1]);
    assert(WaveDeviceCreateSlot() == create_device);
    assert(WaveDeviceReleaseSlot() == release_device);
    void *device = 0;
    assert(WaveDeviceCreateSlot()(&device, "sample.wav", 1) == 17);
    assert(device == &fake_device && creates == 1);
    WaveDeviceReleaseSlot()(device);
    assert(releases == 1);
    SoundDllProcs[1] = reinterpret_cast<void *>(alternate_release);
    WaveDeviceReleaseSlot()(device);
    assert(releases == 1 && alternate_releases == 1);
    assert(load_sound_dll() == 0 && loads == 1 && requested == 11);

    for (int missing = 1; missing <= 11; ++missing) {
        reset();
        fail_ordinal = missing;
        // A failed reload must clear old entries as well as its filled prefix.
        for (int i = 0; i < 11; ++i)
            SoundDllProcs[i] = reinterpret_cast<void *>(unused_export);
        assert(load_sound_dll() == 1);
        assert(requested == missing && loads == 1 && frees == 1 && warnings == 0);
        assert(SoundDllModule == 0 && WaveDeviceReleaseGuard == 0);
        for (int j = 0; j < 11; ++j) assert(SoundDllProcs[j] == 0);
        assert(WaveDeviceCreateSlot() == 0 && WaveDeviceReleaseSlot() == 0);
    }
    reset();
    fail_load = true;
    assert(load_sound_dll() == 1);
    assert(loads == 1 && requested == 0 && frees == 0 && SoundDllModule == 0);
    return 0;
}
