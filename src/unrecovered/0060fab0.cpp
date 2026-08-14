// ORIGINAL: 0x0060FAB0 FILE
// RULED-OUT: reusing one `char *b = self - 0x1c` for both the vtable-slot
//            lookup and the null-check-then-adjust pattern collapses two
//            distinct registers (edi and ebp in the original, which hold the
//            same VALUE but are never merged) into one - the original keeps
//            a separate register alive purely for the "cast-to-virtual-base"
//            null check, this candidate CSEs it away. 69/84 mnemonics match,
//            no stack spill (ternary over if/else avoided the local-int
//            spill an earlier if/else form produced).
// working copy - scaffold materialised by --work
// name      ?on_redraw@CheckBox@@QAEXXZ
// size      221 bytes
// spans     0x0060FAB0-0x0060FB8D
// prototype void (__thiscall ?on_redraw@CheckBox@@QAEXXZ)(CheckBox* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DFF00 0x0060F030
// indirect  0x0060FAED

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_redraw@CheckBox@@QAEXXZ  at 0x0060FAB0  (221 bytes)
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
class CheckBox;
class Font;
typedef void * HBITMAP;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
typedef char * LPSTR;
typedef void * LPVOID;
struct RECT;
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
    int copy(Buffer *, int, int, int, int, int, int);
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
// This body dispatches through slot(s): 10
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
    virtual void slot010();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b7ab8 = (int *)0x009B7AB8;

class CheckBox { public:
    void draw_item(int, int, int);
    void on_redraw();
};
class VCallInt1 { public:
    int slotAny(int);
};
union Shim1 { int (VCallInt1::*fn)(int); void *raw; };

void CheckBox::on_redraw() {
    char *self = reinterpret_cast<char *>(this);
    char *b = self - 0x1c;

    int vbtag = (*reinterpret_cast<int **>(b))[1];
    int obj = *reinterpret_cast<int *>(self + vbtag + 0xa8);
    if (obj == 0) {
        return;
    }
    *g_009b7ab8 = obj;

    int adjEax = (b != 0) ? ((*reinterpret_cast<int **>(b))[1] + reinterpret_cast<int>(b)) : 0;

    Shim1 shim1;
    shim1.raw = (*reinterpret_cast<void ***>(obj))[0x28 / 4];
    int rc = (reinterpret_cast<VCallInt1 *>(obj)->*shim1.fn)(adjEax);

    if (rc == 0) {
        int edxVal = (b != 0)
            ? ((*reinterpret_cast<int **>(b))[1] + reinterpret_cast<int>(self) + 0x428)
            : 0;

        int *vbt = *reinterpret_cast<int **>(b);
        int vbtag4 = vbt[1];
        int vbt2 = vbt[2];
        char *baseA = reinterpret_cast<char *>(vbtag4 + reinterpret_cast<int>(self));
        char *baseB = reinterpret_cast<char *>(vbt2 + reinterpret_cast<int>(self));
        int field4ac = *reinterpret_cast<int *>(baseA + 0x4ac);
        int field4a8 = *reinterpret_cast<int *>(baseA + 0x4a8);
        int fieldC = *reinterpret_cast<int *>(baseB + 0xc);
        int field8 = *reinterpret_cast<int *>(baseB + 8);

        reinterpret_cast<Buffer *>(obj + 0x444)->copy(
            reinterpret_cast<Buffer *>(edxVal), field8, fieldC, 0, 0, field4a8, -field4ac);
    }

    int count = *reinterpret_cast<int *>((*reinterpret_cast<int **>(b))[2] + reinterpret_cast<int>(self) + 0xb0);
    for (int i = 0; i < count; ++i) {
        int selfM18 = *reinterpret_cast<int *>(self - 0x18);
        int vbt2b = (*reinterpret_cast<int **>(b))[2];
        int sameFlag = (selfM18 == i) ? 1 : 0;
        int flags = *reinterpret_cast<int *>(vbt2b + reinterpret_cast<int>(b) + 0xec);
        int bit = flags & (1 << i);
        reinterpret_cast<CheckBox *>(b)->draw_item(i, bit, sameFlag);
        count = *reinterpret_cast<int *>((*reinterpret_cast<int **>(b))[2] + reinterpret_cast<int>(self) + 0xb0);
    }
}
