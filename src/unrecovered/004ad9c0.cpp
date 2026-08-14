// ORIGINAL: 0x004AD9C0 FILE
// RULED-OUT: MISMATCH, closest at 188/187 bytes (mnemonic_similarity 0.92).
//            SetupWin has no scaffold fields, so everything is raw offset
//            arithmetic on `this`. The two "deleting-dtor-style" indirect
//            calls (vtbl[1] read as a byte displacement, added to the
//            object, then that adjusted pointer's OWN vtbl[0] called with
//            arg 1 - a vbtable/virtual-base adjustment thunk, not a plain
//            vcall) are expressed as a second `AdjVCall` shim distinct from
//            the embedded manager's own `MgrVCall` shim. First call site
//            (via an explicit `adjusted` local) reproduces byte-for-byte;
//            the second (`adjusted2`, structurally identical source) still
//            compiles to add+mov[ecx] where the original used lea+mov via
//            a differently-addressed load - register-allocator nuance at
//            the tail of the if-scope, not chased further. No flag set
//            reproduces the original's exact push-ebp framed prologue
//            AND register choice (esi=this, ebx=0) simultaneously; the
//            frameless /O2 build won on mnemonic_similarity despite not
//            matching the prologue shape.
// working copy - scaffold materialised by --work
// name      ?close@SetupWin@@QAEXXZ
// size      187 bytes
// spans     0x004AD9C0-0x004ADA7B
// prototype void (__thiscall ?close@SetupWin@@QAEXXZ)(SetupWin* this)
// callers   5   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D7470 0x005FA830
// indirect  0x004ADA24 0x004ADA36 0x004ADA52

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?close@SetupWin@@QAEXXZ  at 0x004AD9C0  (187 bytes)
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

struct BITMAPINFO;
typedef int BOOL;
class Buffer;
class Font;
typedef void * HBITMAP;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
typedef char * LPSTR;
typedef void * LPVOID;
struct RECT;
class SetupWin;
class Spot;

// ---- callees, declared and never defined (a definition would be inlined) ----
struct RECT {
    long left;
    long top;
    long right;
    long bottom;
};

class Spot { public:
    struct SpotInternal {
        RECT rect;
        int type;
        int position;
    };
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
    void shutdown();
};

class Buffer { public:
    LPVOID vtable_;
    uint32_t poOwner_;
    uint32_t field_8_;
    uint32_t field_C_;
    uint32_t field_10_;
    uint32_t field_14_;
    uint32_t field_18_;
    uint32_t field_1C_;
    RECT rect1_;
    RECT rect2_;
    uint32_t field_40_[4];
    uint32_t field_50_;
    LPVOID * ppv_bits_;
    uint32_t field_58_;
    uint32_t field_5C_;
    HDC hdc2_;
    HDC hdc_;
    uint32_t field_68_;
    uint32_t field_6C_;
    HRGN field_70_;
    uint32_t field_74_;
    HBITMAP bitmap_handle_;
    const BITMAPINFO * bitmap_info_;
    uint32_t width_;
    uint32_t height_;
    uint16_t field_88_;
    uint16_t field_8A_;
    uint32_t field_8C_;
    uint32_t field_90_;
    uint32_t field_94_;
    uint32_t field_98_;
    uint32_t field_9C_;
    uint32_t field_A0_;
    int32_t dib_[256];
    uint32_t field_4A4_;
    uint32_t field_4A8_;
    uint32_t field_4AC_;
    Spot spot_;
    uint32_t field_4BC_;
    uint8_t field_4C0_[0x4C];
    uint32_t field_50C_;
    uint32_t field_510_;
    uint32_t field_514_;
    uint32_t field_518_;
    uint32_t field_51C_;
    uint32_t field_520_;
    uint32_t field_524_;
    uint32_t field_528_;
    Font * font1_;
    Font * font2_;
    Font * font3_;
    Font * font4_;
    uint32_t color_val_1_;
    uint32_t color_2_val_1_;
    uint32_t color_3_val_1_;
    uint32_t color_hyper_val_1_;
    uint32_t color_val_2_;
    uint32_t color_2_val_2_;
    uint32_t color_3_val_2_;
    uint32_t color_hyper_val_2_;
    uint32_t color_val_3_;
    uint32_t color_2_val_3_;
    uint32_t color_3_val_3_;
    uint32_t color_hyper_val_3_;
    uint32_t color_val_4_;
    uint32_t color_2_val_4_;
    uint32_t color_3_val_4_;
    uint32_t color_hyper_val_4_;
    uint32_t field_57C_;
    int8_t field_580_;
    uint32_t field_584_;
    void close();
};

class Font { public:
    int unk_1_;
    BOOL is_fot_set_;
    HFONT font_obj_;
    int line_height_;
    int height_;
    int internal_leading_;
    int ascent_;
    int descent_;
    int pad_;
    LPSTR fot_file_name_;
};


// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 0, 1
class VCall { public:
    virtual void slot000();  // <-- used
    virtual void slot001();  // <-- used
};

class SetupWin { public:
    void close();
};

class MgrVCall { public:
    virtual void slot0();
    virtual void slot1(void *);  // <-- used
};

class AdjVCall { public:
    virtual void slot0(int);  // <-- used
};

void SetupWin::close() {
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0xfd0) = 0;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0xfcc) = 0;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0xa14) = 0;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0xa18) = 0;
    reinterpret_cast<Buffer *>(reinterpret_cast<char *>(this) + 0xa44)->close();
    reinterpret_cast<Spot *>(reinterpret_cast<char *>(this) + 0xfd4)->shutdown();

    char *mgr = reinterpret_cast<char *>(this) + 0xfe0;
    if (*reinterpret_cast<int *>(mgr + 8) != 0) {
        int i;
        for (i = 0; i < *reinterpret_cast<int *>(mgr + 0x10); ++i) {
            char *item = *reinterpret_cast<char **>(mgr + 8);
            *reinterpret_cast<int *>(mgr + 0xc) = *reinterpret_cast<int *>(item + 0xc);
            void *payload = *reinterpret_cast<void **>(item + 8);
            reinterpret_cast<MgrVCall *>(mgr)->slot1(payload);
            if (payload != 0) {
                char *vtbl = *reinterpret_cast<char **>(payload);
                int disp = *reinterpret_cast<int *>(vtbl + 4);
                char *adjusted = reinterpret_cast<char *>(payload) + disp;
                reinterpret_cast<AdjVCall *>(adjusted)->slot0(1);
            }
            *reinterpret_cast<int *>(*reinterpret_cast<char **>(mgr + 8) + 8) = 0;
            char *item2 = *reinterpret_cast<char **>(mgr + 8);
            if (item2 != 0) {
                char *vtbl2 = *reinterpret_cast<char **>(item2);
                int disp2 = *reinterpret_cast<int *>(vtbl2 + 4);
                char *adjusted2 = item2 + disp2;
                reinterpret_cast<AdjVCall *>(adjusted2)->slot0(1);
            }
            *reinterpret_cast<int *>(mgr + 8) = *reinterpret_cast<int *>(mgr + 0xc);
        }
        *reinterpret_cast<int *>(mgr + 8) = 0;
        *reinterpret_cast<int *>(mgr + 0x14) = 0;
        *reinterpret_cast<int *>(mgr + 0x10) = 0;
    }
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0xff4) = 0;
}
