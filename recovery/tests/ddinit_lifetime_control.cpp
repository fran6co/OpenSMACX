// Named API substitutes observe actual source behavior, not native COM slots.
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <new>
typedef void *HWND;
typedef void *LPVOID;
typedef unsigned int uint32_t;
static int events[64], event_count;
static int window_token, bits_token, instance_token;
static void *WinInstance = &instance_token;
static const char DDInitClassName[] = "DirectDClass";
static void event(int value) { events[event_count++] = value; }
struct IDirectDraw {
    void RestoreDisplayMode() { event(2); }
    void Release() { event(3); }
};
struct IDirectDrawSurface {
    void Unlock(void *bits) { assert(bits == &bits_token); event(1); }
    void Release() { assert(false); }
};
static void DestroyWindow(void *window) { assert(window == &window_token); event(4); }
static void UnregisterClassA(const char *name, void *instance) {
    assert(std::strcmp(name, "DirectDClass") == 0 && instance == WinInstance); event(5);
}
static void DDInitRefreshScreenMetrics() { event(6); }
// @@DECLARATION@@
// @@BODIES@@
static bool main_finished;
struct ExitObserver {
    ~ExitObserver() {
        assert(main_finished && event_count == 3);
        assert(events[0] == 6 && events[1] == 6 && events[2] == 6);
    }
};
static ExitObserver observer; // Runs after the source-owned global's destructor.
// @@GLOBAL@@
static DDInit *expected_receiver;
int DDInit::init(int width, int height, int depth, int toggle) {
    assert(this == expected_receiver && width == 0 && height == 0 && depth == 24 && toggle == 1);
    return 19;
}
int main() {
    assert(!WinDisplayInit.hwnd_ && !WinDisplayInit.surf_ &&
           !WinDisplayInit.locked_surface_ && !WinDisplayInit.locked_bits_);
    assert(WinDisplayInit.pitch_ == 0 && WinDisplayInit.field_8_ == 0 &&
           WinDisplayInit.unknown_18_ == 0 && WinDisplayInit.unknown_1C_ == 0);
    // Placement constructs a real object in deliberately nonzero storage.
    // The original leaves the nonstatic pitch untouched; do not read it as int.
    void *storage = std::malloc(sizeof(DDInit)); assert(storage);
    std::memset(storage, 0xa5, sizeof(DDInit));
    DDInit *object = new (storage) DDInit;
    assert(!object->hwnd_ && !object->surf_ && !object->locked_surface_ && !object->locked_bits_);
    assert(object->field_8_ == 0 && object->unknown_18_ == 0 && object->unknown_1C_ == 0);
    const unsigned char *pitch = reinterpret_cast<const unsigned char *>(&object->pitch_);
    for (unsigned int i = 0; i < sizeof(object->pitch_); ++i) assert(pitch[i] == 0xa5);
    expected_receiver = object; assert(object->init_default_mode(24) == 19);
    object->~DDInit(); assert(event_count == 1 && events[0] == 6);
    std::free(storage);
    IDirectDraw device; IDirectDrawSurface surface;
    event_count = 0;
    {
        DDInit local;
        local.hwnd_ = &window_token; local.surf_ = &device;
        local.locked_surface_ = &surface; local.locked_bits_ = &bits_token;
    } // Exactly one automatic destructor owns this lifetime.
    assert(event_count == 6);
    for (int j = 0; j < 6; ++j) assert(events[j] == j + 1);
    event_count = 0;
    {
        DDInit local;
        local.hwnd_ = &window_token; local.surf_ = &device;
        local.locked_surface_ = &surface; local.locked_bits_ = &bits_token;
        local.teardown();
        assert(!local.hwnd_ && !local.surf_ && !local.locked_surface_ && !local.locked_bits_);
    }
    assert(event_count == 7);
    for (int k = 0; k < 6; ++k) assert(events[k] == k + 1);
    assert(events[6] == 6);
    event_count = 0;
    {
        DDInit local;
        local.surf_ = &device; local.locked_surface_ = &surface;
        local.teardown(); // Original repeated cleanup still refreshes metrics.
        assert(!local.surf_ && !local.locked_surface_);
    }
    assert(event_count == 4 && events[0] == 2 && events[1] == 3 && events[2] == 6 && events[3] == 6);
    event_count = 0;
    {
        DDInit local;
        local.locked_bits_ = &bits_token;
        local.teardown();
        assert(local.locked_bits_ == &bits_token); // No surface: original does not clear bits.
    }
    assert(event_count == 2 && events[0] == 6 && events[1] == 6);
    main_finished = true;
}
