// ORIGINAL: 0x00629780 BYTE_EXACT FILE
// name      ?clear@StringBox@@QAEXXZ
// size      292 bytes
// spans     0x00629780-0x006298A4
// prototype void (__thiscall ?clear@StringBox@@QAEXXZ)(StringBox* this)
// callers   20   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005F7E90
// indirect  0x006297BE 0x006297D0 0x006297EC 0x00629828 0x0062983C 0x00629857 0x00629880 0x00629898
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00629780
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00629780/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?clear@StringBox@@QAEXXZ  at 0x00629780  (292 bytes)
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
class Font;
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
    int is_visible();
};

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 0, 1, 2, 62
class VCall { public:
    virtual void slot000(int);  // <-- used
    virtual void slot001(void *);  // <-- used
    virtual void slot002();  // <-- used
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

class StringBox { public:
    uint8_t pad_0_[0xA1C];
    uint32_t field_a1c_;

    void clear();
};

// StringBox::clear() walks a linked list of nodes (either an external
// StringList reached through field_2B6C_, or the StringList embedded at
// field_2B70_ when that pointer is null) and releases each node's data
// then the node itself. Both branches share the same relative layout:
// list+0x8 current node, list+0xc next-node staging, list+0x10 count.
// A node is list+0x8 current->[+0x8 data, +0xc next].
//
// The release call (`slot000(1)`) is not true polymorphism: the address
// is `obj + *(int*)(*(void**)obj + 4)` - a byte offset read out of the
// object's own vtable slot 1, not a compiler-emitted constant, so this
// is written as raw pointer-chasing (mirroring the existing
// deleting-thunk idiom in src/deleting_thunks.h) rather than a cast
// through a real base class.
void StringBox::clear() {
    char *self = reinterpret_cast<char *>(this);
    char *list = *reinterpret_cast<char **>(self + 0x2b6c);

    if (list != 0) {
        if (*reinterpret_cast<int32_t *>(list + 8) == 0) {
            goto tail;
        }
        int i = 0;
        if (*reinterpret_cast<int32_t *>(list + 0x10) <= 0) {
            goto zero_fields;
        }
        do {
            char *node = *reinterpret_cast<char **>(list + 8);
            char *next = *reinterpret_cast<char **>(node + 0xc);
            *reinterpret_cast<char **>(list + 0xc) = next;
            char *data = *reinterpret_cast<char **>(node + 8);
            reinterpret_cast<VCall *>(list)->slot001(data);
            if (data != 0) {
                reinterpret_cast<VCall *>(
                    data + *reinterpret_cast<int32_t *>(*reinterpret_cast<char **>(data) + 4)
                )->slot000(1);
            }
            *reinterpret_cast<int32_t *>(*reinterpret_cast<char **>(list + 8) + 8) = 0;
            node = *reinterpret_cast<char **>(list + 8);
            if (node != 0) {
                reinterpret_cast<VCall *>(
                    node + *reinterpret_cast<int32_t *>(*reinterpret_cast<char **>(node) + 4)
                )->slot000(1);
            }
            next = *reinterpret_cast<char **>(list + 0xc);
            i++;
            *reinterpret_cast<char **>(list + 8) = next;
        } while (i < *reinterpret_cast<int32_t *>(list + 0x10));
    } else {
        list = self + 0x2b70;
        if (*reinterpret_cast<int32_t *>(list + 8) == 0) {
            goto tail;
        }
        int i = 0;
        if (*reinterpret_cast<int32_t *>(list + 0x10) <= 0) {
            goto zero_fields;
        }
        do {
            char *node = *reinterpret_cast<char **>(list + 8);
            char *next = *reinterpret_cast<char **>(node + 0xc);
            *reinterpret_cast<char **>(list + 0xc) = next;
            char *data = *reinterpret_cast<char **>(node + 8);
            reinterpret_cast<VCall *>(list)->slot001(data);
            if (data != 0) {
                reinterpret_cast<VCall *>(
                    data + *reinterpret_cast<int32_t *>(*reinterpret_cast<char **>(data) + 4)
                )->slot000(1);
            }
            *reinterpret_cast<int32_t *>(*reinterpret_cast<char **>(list + 8) + 8) = 0;
            node = *reinterpret_cast<char **>(list + 8);
            if (node != 0) {
                reinterpret_cast<VCall *>(
                    node + *reinterpret_cast<int32_t *>(*reinterpret_cast<char **>(node) + 4)
                )->slot000(1);
            }
            next = *reinterpret_cast<char **>(list + 0xc);
            i++;
            *reinterpret_cast<char **>(list + 8) = next;
        } while (i < *reinterpret_cast<int32_t *>(list + 0x10));
    }

zero_fields:
    *reinterpret_cast<int32_t *>(list + 8) = 0;
    *reinterpret_cast<int32_t *>(list + 0x14) = 0;
    *reinterpret_cast<int32_t *>(list + 0x10) = 0;

tail:
    reinterpret_cast<VCall *>(self + 0xa20)->slot002();
    self = reinterpret_cast<char *>(this);
    *reinterpret_cast<int32_t *>(self + 0xa1c) = 0;
    if (reinterpret_cast<Win *>(self)->is_visible() != 0) {
        reinterpret_cast<VCall *>(self)->slot062();
    }
}
