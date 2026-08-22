// ORIGINAL: 0x004C86D0 sub_4c86d0 0x004C86D0-0x004C87A2;0x004C8450-0x004C8457;0x00659F70-0x00659F8A FILE
// TRIED: literal `call 0x64557f`/SEH __try/__finally reproduction - the span at 0x004C8450-0x004C8457 is COMDAT-folded with src/wave.cpp's Wave::~Wave and src/ambience.cpp's Ambience::~Ambience (both list it in their own spans too), so classify_body reports SHARED_TAIL for this address regardless of body content - no per-function verdict is well defined here, confirmed by running the bare verifier against the empty placeholder before writing anything.
// size      243 bytes
// prototype
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0064557F
// indirect  0x004C8706 0x004C873C

// GENERATED SKELETON (hand-written, following tools/emit_translation_unit.py
// conventions) - a VERIFICATION ARTIFACT, not product source: classes are
// opaque and globals are bound to fixed addresses.
//
// This destructor is the fourth Sound-rooted class in the image (after
// Sound, Wave, Ambience - see src/sound.h and src/ambience.h): its own
// vtable stage is 0x0066E790, and it descends through the same two shared
// stages every Sound-rooted destructor does - 0x0066E3C0 (Sound) then
// 0x0066E444 (the ultimate base) - through the SAME fixed globals
// (0x0090DB20/0x0090DB1C chain ends, 0x0090DB28/0x0090DB7C device release
// hook) that src/sound.cpp, src/wave.cpp and src/ambience.cpp already use.
// What is unique here is the own-stage teardown of the field at +0x3C: it
// is released through ITS OWN vtable (slot 2, one int argument) before the
// generic Sound-stage guarded release runs over it again (a no-op in the
// normal case, kept because the original keeps it).
//
// The registered SEH frame (push -1 / push 0x659F80 / fs:[0] install) is
// omitted, matching the precedent in src/wave.cpp, src/sound.cpp and
// src/ambience.cpp: none of the calls in the body throws in practice, so
// the frame is unreachable and the three sibling destructors already carry
// the same omission under "Status: Complete".

typedef unsigned int uint32_t;
typedef unsigned char uint8_t;

// Vtable shim. VC6 rejects a free `__thiscall` function pointer (C4234),
// so an indirect virtual call is spelled by calling the Nth virtual of a
// class that is never defined and never instantiated. This body dispatches
// through slot 2 only.
class VCall {
public:
    virtual void slot0();
    virtual void slot1();
    virtual void slot2(int);  // <-- used, offset 8
};

// The object's Sound-shaped tail, viewed at fixed offsets - the same shape
// src/ambience.cpp's AmbienceSoundView uses.
struct SoundView {
    uint32_t vtable_storage_;   // 0x00
    uint8_t pad_04_[0x38];      // 0x04..0x3B
    VCall *device_;             // 0x3C
    uint32_t flags_40_;         // 0x40, bit 1 = chained
    SoundView *chain_prev_;     // 0x44, toward the chain head
    SoundView *chain_next_;     // 0x48, toward the chain tail
    void *fname_;               // 0x4C
};

typedef void (__cdecl DeleteFn)(void *);
typedef void (__cdecl DeviceReleaseFn)(void *);

static DeleteFn *const g_operator_delete = (DeleteFn *)0x0064557F;
static DeviceReleaseFn **const g_device_release_slot =
    (DeviceReleaseFn **)0x0090DB28;
static int *const g_device_release_guard = (int *)0x0090DB7C;
static SoundView **const g_chain_head = (SoundView **)0x0090DB20;
static SoundView **const g_chain_tail = (SoundView **)0x0090DB1C;

class SoundOwner {
public:
    ~SoundOwner();
};

SoundOwner::~SoundOwner() {
    SoundView volatile *const self = reinterpret_cast<SoundView volatile *>(this);
    self->vtable_storage_ = 0x0066E790;
    {
        VCall *const device = self->device_;
        if (device) {
            device->slot2(1);
            self->device_ = 0;
        }
    }
    self->vtable_storage_ = 0x0066E3C0;
    {
        void *const name = self->fname_;
        if (name) {
            g_operator_delete(name);
            self->fname_ = 0;
        }
    }
    {
        VCall *const device = self->device_;
        if (device) {
            if (*g_device_release_guard) {
                (*g_device_release_slot)(device);
            }
            self->device_ = 0;
        }
    }
    if (self->flags_40_ & 2) {
        SoundView *const prev = self->chain_prev_;
        if (prev) {
            reinterpret_cast<SoundView volatile *>(prev)->chain_next_ =
                self->chain_next_;
        } else {
            *g_chain_head = self->chain_next_;
        }
        SoundView *const next = self->chain_next_;
        if (next) {
            reinterpret_cast<SoundView volatile *>(next)->chain_prev_ =
                self->chain_prev_;
        } else {
            *g_chain_tail = self->chain_prev_;
        }
        self->chain_next_ = 0;
        self->chain_prev_ = 0;
        self->flags_40_ &= ~2u;
    }
    self->vtable_storage_ = 0x0066E444;
}
