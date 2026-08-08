// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00491C00
// name           ?on_redraw@PrefWin@@QAEXXZ
// size           494 bytes
// measured tier  MISMATCH
// divergence     8
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00491c00/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_redraw@PrefWin@@QAEXXZ  at 0x00491C00  (494 bytes)
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
struct BoxSpriteParams;
class Buffer;
class Font;
typedef void * HBITMAP;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
typedef char * LPSTR;
typedef void * LPVOID;
class PrefWin;
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

// ---- callees, declared and never defined (a definition would be inlined) ----
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
    int box_sprite(RECT *, BoxSpriteParams *);
};

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 62
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
    virtual void slot034();
    virtual void slot035();
    virtual void slot036();
    virtual void slot037();
    virtual void slot038();
    virtual void slot039();
    virtual void slot040();
    virtual void slot041();
    virtual void slot042();
    virtual void slot043();
    virtual void slot044();
    virtual void slot045();
    virtual void slot046();
    virtual void slot047();
    virtual void slot048();
    virtual void slot049();
    virtual void slot050();
    virtual void slot051();
    virtual void slot052();
    virtual void slot053();
    virtual void slot054();
    virtual void slot055();
    virtual void slot056();
    virtual void slot057();
    virtual void slot058();
    virtual void slot059();
    virtual void slot060();
    virtual void slot061();
    virtual void slot062();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0078d690 = (int *)0x0078D690;
static int *const g_0078d7f8 = (int *)0x0078D7F8;
static int *const g_008577d8 = (int *)0x008577D8;
static int *const g_008577dc = (int *)0x008577DC;
static int *const g_008577e0 = (int *)0x008577E0;
static int *const g_008577e4 = (int *)0x008577E4;
static int *const g_00857898 = (int *)0x00857898;
static int *const g_0085789c = (int *)0x0085789C;
static int *const g_008578a4 = (int *)0x008578A4;
static int *const g_008578a8 = (int *)0x008578A8;
static int *const g_00872ca8 = (int *)0x00872CA8;

class PrefWin { public:
    void hide_option_buttons();
    void update_pref_buttons();
    void on_redraw();
};

void PrefWin::on_redraw() {
    char *self = reinterpret_cast<char *>(this);
    Buffer *buf = reinterpret_cast<Buffer *>(self + 0x444);

    // A SINGLE RECT local, reused for both box_sprite calls: the disassembly
    // stores both rectangles into the same [ebp-0x10 .. ebp-4] slots (sub
    // esp, 0x10, not 0x20), and the Ghidra locals (local_14/c/8/10) are
    // likewise reassigned rather than duplicated.
    RECT rect;
    int top1 = *g_00857898;
    int rightBase1 = *g_008578a8;
    rect.left = *g_0085789c;
    rect.right = rightBase1 + rect.left;
    rect.bottom = *g_008578a4 + top1;
    rect.top = top1;
    buf->box_sprite(&rect, reinterpret_cast<BoxSpriteParams *>(g_0078d7f8));

    int top2 = *g_008577dc - *g_00872ca8;
    rect.top = top2;
    rect.left = *g_008577d8;
    rect.right = *g_008577e4 + rect.left;
    rect.bottom = *g_008577e0 + top2;
    if (*reinterpret_cast<int *>(self + 0xa14) == 4) {
        buf->box_sprite(&rect, reinterpret_cast<BoxSpriteParams *>(g_0078d690));
    }

    hide_option_buttons();
    update_pref_buttons();

    // Each of these fields holds a pointer to a virtual-base subobject's
    // vbtable; vbtable[1] is the delta from the field to that subobject,
    // whose own vtable pointer is read after applying the delta. Slot 62
    // (0xf8) is then called on the adjusted `this`.
    {
        char *base = self + 0x5974;
        int delta = (*reinterpret_cast<int **>(base))[1];
        reinterpret_cast<VCall *>(base + delta)->slot062();
    }
    {
        char *base = self + 0x649c;
        int delta = (*reinterpret_cast<int **>(base))[1];
        reinterpret_cast<VCall *>(base + delta)->slot062();
    }
    {
        char *base = self + 0x8614;
        int delta = (*reinterpret_cast<int **>(base))[1];
        reinterpret_cast<VCall *>(base + delta)->slot062();
    }
    {
        char *base = self + 0x913c;
        int delta = (*reinterpret_cast<int **>(base))[1];
        reinterpret_cast<VCall *>(base + delta)->slot062();
    }
    {
        char *base = self + 0x6fc4;
        int delta = (*reinterpret_cast<int **>(base))[1];
        reinterpret_cast<VCall *>(base + delta)->slot062();
    }
    {
        char *base = self + 0x7aec;
        int delta = (*reinterpret_cast<int **>(base))[1];
        reinterpret_cast<VCall *>(base + delta)->slot062();
    }
    {
        char *base = self + 0x9c64;
        int delta = (*reinterpret_cast<int **>(base))[1];
        reinterpret_cast<VCall *>(base + delta)->slot062();
    }

    reinterpret_cast<VCall *>(self + 0x14e08)->slot062();
    reinterpret_cast<VCall *>(self + 0x164a0)->slot062();
    reinterpret_cast<VCall *>(self + 0x15954)->slot062();
    reinterpret_cast<VCall *>(self + 0x16fec)->slot062();
    reinterpret_cast<VCall *>(self + 0x19d1c)->slot062();
    reinterpret_cast<VCall *>(self + 0x191d0)->slot062();
    reinterpret_cast<VCall *>(self + 0x18684)->slot062();
}
