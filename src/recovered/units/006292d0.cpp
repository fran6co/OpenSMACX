// ORIGINAL: 0x006292D0 ?init@StringBox@@QAEXHHHHPAUWin@@PAUStringList@@H@Z 0x006292D0-0x00629486 FILE
// size      438 bytes
// prototype void (__thiscall ?init@StringBox@@QAEXHHHHPAUWin@@PAUStringList@@H@Z)(StringBox* this, int, int, int, int, Win*, StringList*, int)
// callers   6   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40 0x005D4EF0 0x00605370 0x00605890
// indirect  0x00629326 0x00629338 0x00629354 0x006293EE 0x00629400 0x0062941C 0x0062946D
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x006292D0
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/006292d0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?init@StringBox@@QAEXHHHHPAUWin@@PAUStringList@@H@Z  at 0x006292D0  (438 bytes)
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
struct BorderSizing;
class Buffer;
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
class Menu;
typedef void * PVOID;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class StringBox;
class StringList;
struct StringStructEntry;
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

class StringList { public:
    uint32_t primary_abi_word_;
    uint32_t virtual_base_abi_word_;
    StringStructEntry * head_;
    StringStructEntry * current_;
    int entry_count_;
    int current_position_;
    void * allocator_;
    uint32_t field_1C_;
    uint32_t field_20_;
    uint32_t field_24_;
    uint32_t allocation_base_abi_word_;
    void * allocation_owner_;
};

struct StringStructEntry {
    uint32_t abi_word;
    int id;
    int payload;
    StringStructEntry * next;
    StringStructEntry * previous;
    uint32_t secondary_abi_word;
    void * allocation_owner;
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
    void close();
    void init(int, int, int, int, char *, int, Win *, Menu *, BorderSizing *);
};
class Scroll { public:
    int init_vert(int, int, int, Win *, int);
    void close();
};

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 0, 1, 2
class VCall { public:
    virtual void slot000();  // <-- used
    virtual void slot001();  // <-- used
    virtual void slot002();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b8dd4 = (int *)0x009B8DD4;

class StringBox { public:
    uint8_t pad_0_[0xA1C];
    uint32_t field_a1c_;

    void init(int, int, int, int, Win *, StringList *, int);
};

// The element list at +0x2B70 is a polymorphic container: destroying an
// element goes through the element's OWN vtable at a vbtable-relative
// adjustment (the multiple-inheritance adjustor-thunk pattern), so it
// gets its own single-slot shim rather than the container's.
class VCallElem { public:
    virtual void slot000(int);
};
class VCallList { public:
    virtual void slot000();
    virtual void slot001(void *);  // <-- used
};
// The Scroll sub-object's third call (slot 2) has no name in the given
// context - close()/init_vert() are the only two this scaffold knows.
class VCallScroll { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();  // <-- used
};

static void destroyElement(void *p) {
    if (p != 0) {
        int off = *reinterpret_cast<int *>(*reinterpret_cast<int *>(p) + 4);
        reinterpret_cast<VCallElem *>(reinterpret_cast<char *>(p) + off)
            ->slot000(1);
    }
}

// The same list-clearing loop the original inlines twice (once
// unconditionally, once after a successful re-init with no explicit
// string list) - factored here since the two copies are identical, which
// the original's inlined instances are not obligated to be.
static void clearItems(char *self) {
    if (*reinterpret_cast<int *>(self + 0x2B78) != 0) {
        int i = 0;
        if (*reinterpret_cast<int *>(self + 0x2B80) > 0) {
            do {
                char *cur = *reinterpret_cast<char **>(self + 0x2B78);
                *reinterpret_cast<int *>(self + 0x2B7C) =
                    *reinterpret_cast<int *>(cur + 0xC);
                void *elem = *reinterpret_cast<void **>(cur + 8);
                reinterpret_cast<VCallList *>(self + 0x2B70)->slot001(elem);
                destroyElement(elem);
                *reinterpret_cast<int *>(cur + 8) = 0;
                destroyElement(cur);
                *reinterpret_cast<int *>(self + 0x2B78) =
                    *reinterpret_cast<int *>(self + 0x2B7C);
                ++i;
            } while (i < *reinterpret_cast<int *>(self + 0x2B80));
        }
        *reinterpret_cast<int *>(self + 0x2B78) = 0;
        *reinterpret_cast<int *>(self + 0x2B84) = 0;
        *reinterpret_cast<int *>(self + 0x2B80) = 0;
    }
}

void StringBox::init(int a1, int a2, int a3, int a4, Win * a5, StringList * a6, int a7) {
    char *self = reinterpret_cast<char *>(this);

    *reinterpret_cast<int *>(self + 0xA18) = -1;
    *reinterpret_cast<int *>(self + 0xA1C) = 0;
    *reinterpret_cast<int *>(self + 0x2B6C) = 0;
    *reinterpret_cast<int *>(self + 0xA14) = 0;
    reinterpret_cast<GraphicWin *>(this)->close();

    clearItems(self);
    *reinterpret_cast<int *>(self + 0x2B84) = 0;

    reinterpret_cast<Scroll *>(self + 0xA20)->close();

    int flags = a7;
    if (flags == -1) {
        flags = 0x1000120;
    }
    // NOT REPRODUCED: GraphicWin::init is catalogued `void` (the mangled
    // name's `X`), yet the original tests EAX right after the call and
    // skips the rest of this function on a nonzero result - so the real
    // callee returns something the catalogue's signature does not admit.
    // Changing that signature is a catalogue edit outside this file; the
    // call below drops the result and always takes the success path.
    reinterpret_cast<GraphicWin *>(this)->init(
        a1, a2, a3, a4, 0, flags, a5, 0, 0);
    {
        if (a6 != 0) {
            *reinterpret_cast<int *>(self + 0x2B6C) =
                reinterpret_cast<int>(a6);
        } else {
            clearItems(self);
        }
        *reinterpret_cast<int *>(self + 0x2B84) = 0;
        *reinterpret_cast<int *>(self + 0x2B88) = 0;

        int vshift = a4 - *g_009b8dd4;
        int field4c8 = *reinterpret_cast<int *>(self + 0x4C8);
        Scroll *scroll = reinterpret_cast<Scroll *>(self + 0xA20);
        scroll->init_vert(vshift, 0, -field4c8,
                          reinterpret_cast<Win *>(this), 0);
        reinterpret_cast<VCallScroll *>(scroll)->slot002();

        *reinterpret_cast<int *>(self + 0x1440) = 0;
        *reinterpret_cast<int *>(self + 0x1444) = 0;
    }
}
