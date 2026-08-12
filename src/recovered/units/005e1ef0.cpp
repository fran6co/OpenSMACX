// ORIGINAL: 0x005E1EF0 FILE
// name      ?dotted_vline@Buffer@@QAEXHHHH@Z
// size      384 bytes
// spans     0x005E1EF0-0x005E2070
// prototype void (__thiscall ?dotted_vline@Buffer@@QAEXHHHH@Z)(Buffer* this, int, int, int, int)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005E1FBD 0x005E205B
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005E1EF0
// measured tier  MISMATCH
// divergence     28
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005e1ef0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?dotted_vline@Buffer@@QAEXHHHH@Z  at 0x005E1EF0  (384 bytes)
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
class Spot;

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

struct RECT {
    long left;
    long top;
    long right;
    long bottom;
};

class Spot { public:
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
};

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 25, 32
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
    virtual void slot025();  // <-- used
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_005e201a = (int *)0x005E201A;

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
    uint8_t field_4BC_[80];
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

    void dotted_vline(int, int, int, int);
};

typedef int (__stdcall *DottedVLineSlot025)(void *, int, void *, int, int);
typedef int (__stdcall *DottedVLineSlot032)(void *, int);

void Buffer::dotted_vline(int a1, int a2, int a3, int a4) {
    char *self = reinterpret_cast<char *>(this);
    int vptr1 = *reinterpret_cast<int *>(self + 0x54);

    if (vptr1 == 0 && *reinterpret_cast<int *>(self + 0x58) == 0) return;
    if (a1 < *reinterpret_cast<int *>(self + 0x20)) return;
    if (a1 >= *reinterpret_cast<int *>(self + 0x28)) return;
    if (a2 == a3) return;

    if (a2 > a3) {
        a2 ^= a3;
        a3 ^= a2;
        a2 ^= a3;
    }

    int bottom = *reinterpret_cast<int *>(self + 0x2c);
    if (a2 >= bottom) return;
    int top = *reinterpret_cast<int *>(self + 0x24);
    if (a3 < top) return;
    if (a2 < top) a2 = top;
    if (a3 >= bottom) a3 = bottom - 1;

    if (a1 >= *reinterpret_cast<int *>(self + 0x80)) return;
    int negHeight = -*reinterpret_cast<int *>(self + 0x84);
    if (a2 >= negHeight) return;

    int *piVar3 = *reinterpret_cast<int **>(self + 0x58);
    int state;
    if (piVar3 == 0) {
        *reinterpret_cast<int *>(self + 0x50) = vptr1;
        if (vptr1 != 0) {
            (*reinterpret_cast<int *>(self + 0x6c))++;
        }
        state = vptr1;
    } else if (*reinterpret_cast<int *>(self + 0x50) == 0) {
        int callBuf[10];
        callBuf[0] = 0x6c;
        DottedVLineSlot025 fn25 = reinterpret_cast<DottedVLineSlot025>((*reinterpret_cast<int ***>(piVar3))[25]);
        int callResult = fn25(piVar3, 0, callBuf, 1, 0);
        if (callResult != 0) {
            return;
        }
        *reinterpret_cast<int *>(self + 0x4a8) = callBuf[4];
        (*reinterpret_cast<int *>(self + 0x6c))++;
        *reinterpret_cast<int *>(self + 0x50) = callBuf[9];
        state = callBuf[9];
    } else {
        (*reinterpret_cast<int *>(self + 0x6c))++;
        state = *reinterpret_cast<int *>(self + 0x50);
    }

    if (state == 0) return;

    int stride = *reinterpret_cast<int *>(self + 0x4a8);
    char *pixelPtr = reinterpret_cast<char *>(stride * a2 + a1 + *reinterpret_cast<int *>(self + 0x50));
    if (pixelPtr == 0) return;

    unsigned int count = (static_cast<unsigned int>(a3 - a2) + 1) >> 1;
    do {
        *pixelPtr = static_cast<char>(a4);
        pixelPtr += stride * 2;
        --count;
    } while (count != 0);

    if (piVar3 == 0) {
        int refs = *reinterpret_cast<int *>(self + 0x6c) - 1;
        *reinterpret_cast<int *>(self + 0x6c) = refs;
        if (refs < 1) {
            *reinterpret_cast<int *>(self + 0x50) = 0;
            *reinterpret_cast<int *>(self + 0x6c) = 0;
        }
        return;
    }

    int refs = *reinterpret_cast<int *>(self + 0x6c) - 1;
    *reinterpret_cast<int *>(self + 0x6c) = refs;
    if (*reinterpret_cast<int *>(self + 0x50) != 0 && refs < 1) {
        DottedVLineSlot032 fn32 = reinterpret_cast<DottedVLineSlot032>((*reinterpret_cast<int ***>(piVar3))[32]);
        fn32(piVar3, *reinterpret_cast<int *>(self + 0x50));
        *reinterpret_cast<int *>(self + 0x50) = 0;
        *reinterpret_cast<int *>(self + 0x6c) = 0;
    }
}
