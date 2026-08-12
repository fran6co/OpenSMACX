// ORIGINAL: 0x0046ED30 FILE
// name      ?on_left_down@MapWin@@QAEXHH@Z
// size      689 bytes
// spans     0x0046ED30-0x0046EFE1
// prototype void (__thiscall ?on_left_down@MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   9
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00463040 0x004E1F40 0x004E3C60 0x0050EF50 0x0050F170 0x005ABF20 0x005D5250 0x005ED9D0 0x00616350
// indirect  0x0046EE06 0x0046EE1A 0x0046EE26 0x0046EE35 0x0046EE45 0x0046EEBB 0x0046EEC9 0x0046EF88
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0046ED30
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0046ed30/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_left_down@MapWin@@QAEXHH@Z  at 0x0046ED30  (689 bytes)
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

class AutoSound;
struct BITMAPINFO;
typedef int BOOL;
class Buffer;
class Console;
class Font;
class GraphicWin;
typedef void * HBITMAP;
struct HCURSOR;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
class Heap;
typedef char * LPSTR;
typedef void * LPVOID;
class MapWin;
class Menu;
typedef void * PVOID;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class Time;
typedef unsigned int UINT_PTR;
class Win;

class AutoSound { public:
    PVOID vtable_;
    int val_1_;
    int val_2_;
    int val_3_;
    int val_4_;
    int val_5_;
    int val_6_;
    int val_7_;
    int val_8_;
    int val_9_;
    int val_10_;
    int val_11_;
    int val_12_;
    int val_13_;
    int val_14_;
    int val_15_;
    int val_16_;
    int val_17_;
    int val_18_;
    int val_19_;
    int val_20_;
    int val_21_;
    int val_22_;
    int val_23_;
    int val_24_;
    int val_25_;
    int val_26_;
    int val_27_;
    int val_28_;
    int val_29_;
    int val_30_;
    int val_31_;
    int val_32_;
    int val_33_;
    int val_34_;
    int val_35_;
    int val_36_;
    int val_37_;
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

class Heap { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
};

class Sprite { public:
    int ppszFileName_;
    int pcBits_;
    char cTransparentIndex_;
    char pad1_;
    char pad2_;
    char pad3_;
    int iSpriteWidth2_;
    int iSpriteWidth_;
    int iSpriteHeight_;
    int iWidth_;
    int iHeight_;
    int iLeftOffset_;
    int iTopOffset_;
    int fObj1Exists_;
};

// ---- callees, declared and never defined (a definition would be inlined) ----
class GraphicWin { public:
    AutoSound auto_sound_;
    uint32_t iFlags_;
    uint32_t iSomeFlag_;
    uint32_t field_A0_;
    uint32_t field_A4_;
    uint32_t poWinBase_;
    uint32_t iVertScaleDenom_;
    uint32_t iVertScaleNum_;
    Buffer * buffer1_;
    Buffer * buffer2_;
    Buffer * buffer3_;
    Buffer * buffer4_;
    Win * win_parent_;
    uint32_t field_C8_;
    uint32_t field_CC_;
    uint32_t field_D0_;
    uint32_t field_D4_;
    uint32_t field_D8_;
    Heap heap_;
    Menu * menu_;
    uint32_t field_F4_;
    uint32_t field_F8_;
    uint32_t field_FC_;
    uint32_t field_100_;
    uint32_t field_104_;
    uint32_t field_108_;
    uint32_t field_10C_;
    uint32_t field_110_;
    int caption_height_;
    int border_thickness_;
    int bottom_border_thickness_;
    uint32_t field_120_;
    uint32_t field_124_;
    uint32_t field_128_;
    uint32_t field_12C_;
    uint32_t field_130_;
    uint32_t field_134_;
    uint32_t field_138_;
    RECT outer_rect_;
    RECT client_rect_;
    uint32_t field_15C_;
    uint32_t field_160_;
    uint32_t field_164_;
    uint32_t field_168_;
    uint32_t field_16C_;
    uint32_t field_170_;
    uint32_t field_174_;
    uint32_t field_178_;
    uint32_t field_17C_;
    uint32_t field_180_;
    uint32_t field_184_;
    Sprite * cursor_sprite_;
    uint32_t field_18C_;
    uint32_t field_190_;
    HCURSOR * cursor_handle_;
    int cursor_name_;
    uint32_t field_19C_;
    uint32_t field_1A0_;
    Win * children_[150];
    int child_count_;
    uint32_t field_400_;
    uint32_t field_404_;
    uint32_t field_408_;
    uint32_t field_40C_;
    uint32_t field_410_;
    uint32_t field_414_;
    uint32_t field_418_;
    uint32_t field_41C_;
    uint32_t field_420_;
    uint32_t field_424_;
    uint32_t field_428_;
    uint32_t field_42C_;
    uint32_t field_430_;
    uint32_t field_434_;
    uint32_t field_438_;
    Scroll * scroll_vert_;
    Scroll * scroll_horz_;
    Buffer buffer_;
    uint32_t field_9CC_;
    uint32_t field_9D0_;
    uint32_t field_9D4_;
    uint32_t field_9D8_;
    uint32_t field_9DC_;
    uint32_t field_9E0_;
    uint32_t field_9E4_;
    uint32_t field_9E8_;
    uint32_t field_9EC_;
    uint32_t field_9F0_;
    uint32_t field_9F4_;
    uint32_t field_9F8_;
    uint32_t field_9FC_;
    uint32_t field_A00_;
    uint32_t field_A04_;
    uint32_t poCanvas_;
    uint32_t field_A0C_;
    uint32_t field_A10_;
    void fill(int);
};
class Console { public:
    uint8_t derived_storage_[0x23D94];
    GraphicWin virtual_base_;
    int edit_lock();
};
class Time { public:
    int unk_tgl_;
    UINT_PTR id_event_;
    void * callback1_;
    void * callback2_;
    int cb_param2_;
    int cb_param1_;
    uint32_t count_;
    int unk_1_;
    uint32_t resolution_;
    int unk_2_;
    void start(void (__cdecl *)(int), int, int, int);
};
class Win { public:
    AutoSound auto_sound_;
    uint32_t iFlags_;
    uint32_t iSomeFlag_;
    uint32_t field_A0_;
    uint32_t field_A4_;
    uint32_t poWinBase_;
    uint32_t iVertScaleDenom_;
    uint32_t iVertScaleNum_;
    Buffer * buffer1_;
    Buffer * buffer2_;
    Buffer * buffer3_;
    Buffer * buffer4_;
    Win * win_parent_;
    uint32_t field_C8_;
    uint32_t field_CC_;
    uint32_t field_D0_;
    uint32_t field_D4_;
    uint32_t field_D8_;
    Heap heap_;
    Menu * menu_;
    uint32_t field_F4_;
    uint32_t field_F8_;
    uint32_t field_FC_;
    uint32_t field_100_;
    uint32_t field_104_;
    uint32_t field_108_;
    uint32_t field_10C_;
    uint32_t field_110_;
    int caption_height_;
    int border_thickness_;
    int bottom_border_thickness_;
    uint32_t field_120_;
    uint32_t field_124_;
    uint32_t field_128_;
    uint32_t field_12C_;
    uint32_t field_130_;
    uint32_t field_134_;
    uint32_t field_138_;
    RECT outer_rect_;
    RECT client_rect_;
    uint32_t field_15C_;
    uint32_t field_160_;
    uint32_t field_164_;
    uint32_t field_168_;
    uint32_t field_16C_;
    uint32_t field_170_;
    uint32_t field_174_;
    uint32_t field_178_;
    uint32_t field_17C_;
    uint32_t field_180_;
    uint32_t field_184_;
    Sprite * cursor_sprite_;
    uint32_t field_18C_;
    uint32_t field_190_;
    HCURSOR * cursor_handle_;
    int cursor_name_;
    uint32_t field_19C_;
    uint32_t field_1A0_;
    Win * children_[150];
    int child_count_;
    uint32_t field_400_;
    uint32_t field_404_;
    uint32_t field_408_;
    uint32_t field_40C_;
    uint32_t field_410_;
    uint32_t field_414_;
    uint32_t field_418_;
    uint32_t field_41C_;
    uint32_t field_420_;
    uint32_t field_424_;
    uint32_t field_428_;
    uint32_t field_42C_;
    uint32_t field_430_;
    uint32_t field_434_;
    uint32_t field_438_;
    Scroll * scroll_vert_;
    Scroll * scroll_horz_;
    void show(int);
};
int __cdecl base_find(int, int, int);
void __cdecl auto_undo();
void __cdecl go_reset();
void __cdecl go_timer(int);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 17
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
    virtual void slot017();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0050f170 = (int *)0x0050F170;
static int *const g_0066932c = (int *)0x0066932C;
static int *const g_00669330 = (int *)0x00669330;
static int *const g_0068a5cc = (int *)0x0068A5CC;
static int *const g_0090d938 = (int *)0x0090D938;
static int *const g_009156b0 = (int *)0x009156B0;
static int *const g_00937118 = (int *)0x00937118;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00939288 = (int *)0x00939288;
static int *const g_009393c0 = (int *)0x009393C0;
static int *const g_00939e60 = (int *)0x00939E60;
static int *const g_0093a938 = (int *)0x0093A938;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009b7abc = (int *)0x009B7ABC;
static int *const g_009b7ac0 = (int *)0x009B7AC0;

class MapWin { public:
    uint32_t vbtable_pointer_;
    void * owned_;
    uint8_t derived_tail_[0x21A6C - 0x8];
    GraphicWin virtual_base_;

    int pixel_to_tile(int, int, int *, int *);
    void on_left_down(int, int);
};

// Vtable slot 0x44/4 = 17, called with two int args - the emitter's slot
// is nullary, so a widened shim is needed.
class VCall17 { public:
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
    virtual void slot017(int, int);  // <-- used, widened
};

typedef short(__stdcall *KeyStateFn)(int);
typedef unsigned char(__stdcall *AsyncKeyFn)(int);

void MapWin::on_left_down(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);

    if (*g_0090d938 == 0) {
        goto end;
    }

    go_reset();
    *reinterpret_cast<int *>(self - 0x3cf0) = 0;
    *reinterpret_cast<int *>(self - 0x3cec) = 0;
    *reinterpret_cast<int *>(self - 0x3ce0) = 0;
    *reinterpret_cast<int *>(self - 0x3c60) = -1;
    *reinterpret_cast<int *>(self - 0x3ce4) = 0;
    *reinterpret_cast<int *>(self - 0x3ce8) = 0;

    if (*g_0093a938 != 0) {
        goto end;
    }
    if (*reinterpret_cast<int *>(self - 0x3cf4) != 0) {
        goto end;
    }
    if (*g_009393c0 == 0) {
        goto label_locked;
    }
    if (*g_009393c0 != 8) {
        goto end;
    }

    *reinterpret_cast<int *>(self - 0x3cec) = 1;
    if (*g_009b7abc == 0) {
        int *vtbl = *reinterpret_cast<int **>(self - 0x21a6c);
        int voff = vtbl[1];
        *g_009b7ac0 = 0;
        *g_009b7abc = reinterpret_cast<int>(self - 0x21a6c + voff);
    }

    reinterpret_cast<GraphicWin *>(g_00937118)->fill(9);
    reinterpret_cast<Win *>(g_00937118)->show(0);

    {
        int *vtbl = *reinterpret_cast<int **>(self - 0x21a6c);
        int voff = vtbl[1];
        char *vbase = self - 0x21a6c + voff;
        reinterpret_cast<VCall17 *>(vbase)->slot017(a1, a2);
    }
    return;

label_locked : {
    KeyStateFn getKeyState = *reinterpret_cast<KeyStateFn *>(g_00669330);
    AsyncKeyFn getAsyncKey =
        *reinterpret_cast<AsyncKeyFn *>(g_0066932c);

    unsigned char shiftDown =
        static_cast<unsigned char>(getKeyState(0x10) >> 8);
    if (shiftDown == 0) {
        unsigned char ctrlDown =
            static_cast<unsigned char>(getKeyState(0x11) >> 8);
        if (ctrlDown != 0) {
            goto label_check_undo;
        }
    }

    if ((getAsyncKey(0x91) & 1) == 0) {
        goto label_after_undo;
    }
    {
        unsigned char ctrlDown2 =
            static_cast<unsigned char>(getKeyState(0x11) >> 8);
        if (ctrlDown2 != 0) {
            goto label_after_undo;
        }
    }

label_check_undo:
    if ((*reinterpret_cast<unsigned char *>(g_009a64c0) & 0x40) == 0) {
        goto label_after_undo;
    }

    auto_undo();
    *reinterpret_cast<int *>(self - 0x3ce0) = 1;
    *reinterpret_cast<int *>(self - 0x3cd8) = -1;
    *reinterpret_cast<int *>(self - 0x3cdc) = -1;

    if (*g_009b7abc == 0) {
        int *vtbl = *reinterpret_cast<int **>(self - 0x21a6c);
        int voff = vtbl[1];
        *g_009b7ac0 = 0;
        *g_009b7abc = reinterpret_cast<int>(self - 0x21a6c + voff);
    }

    {
        int *vtbl = *reinterpret_cast<int **>(self - 0x21a6c);
        int voff = vtbl[1];
        char *vbase = self - 0x21a6c + voff;
        reinterpret_cast<VCall17 *>(vbase)->slot017(a1, a2);
    }
    return;

label_after_undo : {
    unsigned char ctrlDown3 =
        static_cast<unsigned char>(getKeyState(0x11) >> 8);
    if (ctrlDown3 != 0) {
        goto end;
    }
}

    if (reinterpret_cast<Console *>(g_009156b0)->edit_lock() != 0) {
        goto end;
    }

    {
        int tileX, tileY;
        bool haveTile = false;

        if (*g_00939288 == 0 ||
            (*reinterpret_cast<int *>(self - 0x3cfc) & 0x40000) != 0) {
            int rc = reinterpret_cast<MapWin *>(self - 0x21a6c)
                         ->pixel_to_tile(a1, a2, &tileY, &tileX);
            if (rc != 0) {
                goto end;
            }
            if (tileY < 0 || tileY >= *g_00949874) {
                goto end;
            }
            if (tileX < 0 || tileX >= *g_00949870) {
                goto end;
            }
            haveTile = true;
        }
        (void)haveTile;

        if ((*reinterpret_cast<int *>(self - 0x3cfc) & 0x40000) != 0) {
            int found = base_find(tileX, tileY, *g_00939284);
            *reinterpret_cast<int *>(self - 0x3c48) = found;
            if (found < 0) {
                goto end;
            }
        }
    }

    *g_0068a5cc = *reinterpret_cast<int *>(self - 0x3cf4);

    {
        unsigned char shiftDown2 =
            static_cast<unsigned char>(getKeyState(0x10) >> 8);
        if (shiftDown2 != 0 &&
            (*reinterpret_cast<unsigned char *>(g_009a64c0) & 0x40) == 0) {
            go_timer(3);
            return;
        }
    }

    {
        int guard = (*g_00939288 != 0) ? -1 : 0;
        int edxVal = (guard & 0xfffffd65) + 0x3e8;
        reinterpret_cast<Time *>(g_00939e60)
            ->start(reinterpret_cast<void(__cdecl *)(int)>(g_0050f170), 3,
                    edxVal, 5);
    }
}

end:
    return;
}
