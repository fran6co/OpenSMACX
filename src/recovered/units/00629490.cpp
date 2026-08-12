// ORIGINAL: 0x00629490 FILE
// name      ?add_fixup@StringBox@@QAEXXZ
// size      627 bytes
// spans     0x00629490-0x00629703
// prototype void (__thiscall ?add_fixup@StringBox@@QAEXXZ)(StringBox* this)
// callers   21   call targets   7
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00402500 0x00402530 0x005DAD70 0x005DC360 0x005DCAB0 0x005F7E90 0x00605D20
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00629490
// measured tier  MISMATCH
// divergence     63
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00629490/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?add_fixup@StringBox@@QAEXXZ  at 0x00629490  (627 bytes)
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
class StringList;
class StringStruct;
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
    int text_line_height();
    int wrap_height(char *, int);
    int write_strings_height(StringList *, int, int);
};
class Scroll { public:
    void set_pos(int);
};
class StringStruct { public:
    uint32_t primary_abi_word_;
    uint32_t virtual_base_abi_word_;
    StringStructEntry * head_;
    StringStructEntry * current_;
    int entry_count_;
    int current_position_;
    void * allocator_;
    uint32_t allocation_base_abi_word_;
    void * allocation_owner_;
    int current_entry();
    int next_entry();
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
    int is_visible();
};

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 0, 1, 2, 62
class VCall { public:
    virtual void slot000();  // <-- used
    virtual void slot001();  // <-- used
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

    void add_fixup();
};

// The emitter's VCall shim declares slot000/slot001 nullary; the two
// destructor-like calls at 0x00629571/0x0062958A push an int (1) before
// `call [edx]`, and the calls at 0x0062955F/0x00629623 push an int before
// `call [edx+4]`/`call [eax+4]`, so this shim widens just those two slots.
class VCall2 { public:
    virtual void slot000(int);  // <-- used, widened
    virtual void slot001(int);  // <-- used, widened
    virtual void slot002();     // <-- used
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
    virtual void slot062();     // <-- used
};

void StringBox::add_fixup() {
    char *self = reinterpret_cast<char *>(this);
    Buffer *buf = reinterpret_cast<Buffer *>(self + 0x444);

    int lineHeight = buf->text_line_height();
    if (lineHeight == 0) {
        lineHeight = 0xc;
    }

    StringStruct *list = *reinterpret_cast<StringStruct **>(self + 0x2b6c);
    if (list == 0) {
        list = reinterpret_cast<StringStruct *>(self + 0x2b70);
    }

    if (*reinterpret_cast<int *>(self + 0xa18) > 0) {
        int extra = *reinterpret_cast<int *>(self + 0x4c4);
        if (*reinterpret_cast<uint8_t *>(self + 0xabc) & 1) {
            extra -= *reinterpret_cast<int *>(self + 0x1480);
        }

        int written = buf->write_strings_height(reinterpret_cast<StringList *>(list), extra, 0);
        if (written / lineHeight > *reinterpret_cast<int *>(self + 0xa18)) {
            do {
                if (list->entry_count_ - 1 >= 0) {
                    list->current_ = list->head_;
                    list->current_position_ = 0;
                }

                if (list->head_ != 0) {
                    list->current_->next->previous = list->current_->previous;
                    list->current_->previous->next = list->current_->next;

                    StringStructEntry *entry = list->current_;
                    if (entry == list->head_) {
                        list->head_ = entry->next;
                    }
                    list->current_ = list->current_->next;

                    void *payload = reinterpret_cast<void *>(entry->payload);
                    reinterpret_cast<VCall2 *>(list)->slot001(reinterpret_cast<int>(payload));
                    if (payload != 0) {
                        void **vtbl = *reinterpret_cast<void ***>(payload);
                        char *adjusted = reinterpret_cast<char *>(payload)
                                       + reinterpret_cast<long>(vtbl[1]);
                        reinterpret_cast<VCall2 *>(adjusted)->slot000(1);
                    }

                    entry->payload = 0;
                    if (entry != 0) {
                        void **vtbl = *reinterpret_cast<void ***>(entry);
                        char *adjusted = reinterpret_cast<char *>(entry)
                                       + reinterpret_cast<long>(vtbl[1]);
                        reinterpret_cast<VCall2 *>(adjusted)->slot000(1);
                    }

                    if (--list->entry_count_ == 0) {
                        list->head_ = 0;
                    }
                    if (list->current_position_ > list->entry_count_ - 1) {
                        list->current_position_ = list->entry_count_ - 1;
                    }
                }

                written = buf->write_strings_height(reinterpret_cast<StringList *>(list), extra, 0);
            } while (written / lineHeight > *reinterpret_cast<int *>(self + 0xa18));
        }
    }

    *reinterpret_cast<int *>(self + 0x1444) = list->entry_count_;
    *reinterpret_cast<int *>(self + 0x1460) =
        -1 - (*reinterpret_cast<int *>(self + 0x4c8) / lineHeight);

    if (!(*reinterpret_cast<uint8_t *>(self + 0xabc) & 1)) {
        int written2 = buf->write_strings_height(reinterpret_cast<StringList *>(list),
                                                  *reinterpret_cast<int *>(self + 0x4c4), 0);
        if (written2 > -*reinterpret_cast<int *>(self + 0x4c8)) {
            reinterpret_cast<VCall2 *>(self + 0xa20)->slot001(0);
        } else {
            reinterpret_cast<VCall2 *>(self + 0xa20)->slot002();
        }
    }

    if (reinterpret_cast<Win *>(self)->is_visible()) {
        if (*reinterpret_cast<uint8_t *>(self + 0xabc) & 1) {
            if (list->entry_count_ - 1 >= 0) {
                list->current_ = list->head_;
                list->current_position_ = 0;
            }

            if (-(buf->text_line_height() + *reinterpret_cast<int *>(self + 0x4c8)) >= 0) {
                int accum = 0;
                do {
                    if (list->head_ != 0) {
                        list->current_ = list->current_->previous;
                        if (--list->current_position_ == -1) {
                            list->current_position_ = list->entry_count_ - 1;
                        }
                    }

                    int idField = (list->current_ != 0)
                        ? reinterpret_cast<StringStructEntry *>(list->current_entry())->id
                        : 0;

                    accum += buf->wrap_height(reinterpret_cast<char *>(idField),
                                               *reinterpret_cast<int *>(self + 0x4c4));
                } while (accum <= -(buf->text_line_height() + *reinterpret_cast<int *>(self + 0x4c8)));
            }

            list->next_entry();
            reinterpret_cast<Scroll *>(self + 0xa20)->set_pos(list->current_position_);
            *reinterpret_cast<int *>(self + 0xa1c) = *reinterpret_cast<int *>(self + 0x144c);
        }
        reinterpret_cast<VCall2 *>(self)->slot062();
    }
}
