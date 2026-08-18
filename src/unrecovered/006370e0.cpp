// ORIGINAL: 0x006370E0 ?play@Video@@QAEHXZ 0x006370E0-0x0063718F FILE
// RULED-OUT: own VCall2 shim (35 slots) for slot34(int) since the scaffold's VCall types it nullary; cached TimeGetTimeFn across both timeGetTime() calls. MISMATCH #2 mov/push, early prologue register-save ordering only.
// working copy - scaffold materialised by --work
// size      175 bytes
// prototype int (__thiscall ?play@Video@@QAEHXZ)(Video* this)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00626250 0x00637910 0x00637C30
// indirect  0x0063712F 0x00637159 0x00637173

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?play@Video@@QAEHXZ  at 0x006370E0  (175 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.

typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
// `char`, NOT `signed char`. They are distinct MSVC types and mangle
// differently - D against C - and the catalogue's `int8` means the first:
// counted over every catalogued mangled name, `PAD` appears 508 times and
// `PAC` once. Spelling it `signed char` made 150 derived prototypes emit a
// symbol no target object holds. `int8_t` keeps its C meaning below; neither
// catalogue ever uses it.
typedef char int8;
typedef unsigned char uint8;

class Video;

// ---- callees, declared and never defined (a definition would be inlined) ----
void log_say(char *, char *, int, int, int);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 34
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();
    virtual void slot012();
    virtual void slot013();
    virtual void slot014();
    virtual void slot015();
    virtual void slot016();
    virtual void slot017();
    virtual void slot018();
    virtual void slot019();
    virtual void slot020();
    virtual void slot021();
    virtual void slot022();
    virtual void slot023();
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();
    virtual void slot033();
    virtual void slot034();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669368 = (int *)0x00669368;
static int *const g_006987fc = (int *)0x006987FC;
static int *const g_00698808 = (int *)0x00698808;

class Video { public:
    int decode_frame(int);
    int read_frame();
    int play();
};
class VCall2 { public:
    virtual void s00(); virtual void s01(); virtual void s02(); virtual void s03();
    virtual void s04(); virtual void s05(); virtual void s06(); virtual void s07();
    virtual void s08(); virtual void s09(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15();
    virtual void s16(); virtual void s17(); virtual void s18(); virtual void s19();
    virtual void s20(); virtual void s21(); virtual void s22(); virtual void s23();
    virtual void s24(); virtual void s25(); virtual void s26(); virtual void s27();
    virtual void s28(); virtual void s29(); virtual void s30(); virtual void s31();
    virtual void s32(); virtual void s33();
    virtual void s34(int);
};

int Video::play() {
    char *self = reinterpret_cast<char *>(this);
    if (*reinterpret_cast<int *>(self + 0x4b0) == 0) {
        return 7;
    }
    if ((*reinterpret_cast<unsigned char *>(self + 0x4ac) & 0x10) != 0) {
        read_frame();
    } else {
        while (read_frame() != 0) {
        }
        decode_frame(0);
    }
    typedef unsigned int (__stdcall *TimeGetTimeFn)();
    TimeGetTimeFn fn = *reinterpret_cast<TimeGetTimeFn *>(g_00669368);
    unsigned int t = fn();
    log_say((char *)g_00698808, (char *)g_006987fc, (int)t, 0, 0);
    if ((*reinterpret_cast<unsigned char *>(self + 0x4ac) & 1) != 0) {
        unsigned int t2 = fn();
        *reinterpret_cast<unsigned int *>(self + 0xffc) = t2;
        *reinterpret_cast<int *>(self + 0x4ac) |= 0x8000;
        return 0;
    }
    reinterpret_cast<VCall2 *>(this)->s34(*reinterpret_cast<int *>(self + 0x1004));
    *reinterpret_cast<int *>(self + 0x4ac) |= 0x8000;
    return 0;
}
