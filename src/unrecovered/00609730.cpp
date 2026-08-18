// ORIGINAL: 0x00609730 ?init@Dialog@@QAEHHHHHPAUHeap@@@Z 0x00609730-0x006097EE FILE
// RULED-OUT: MISMATCH, closest at 187/190 bytes (mnemonic_similarity 0.97). Same embedded-manager close-and-relink shape as SetupWin::close (0x004AD9C0): a single mutated `char *p` (starts as `this`, then `p += 0xbc` to become the manager) reproduces the whole prologue, field stores AND the loop body byte-for-byte - matching original's single-register reuse (esi: this, then mgr) instead of a separate named `mgr` local, which cost an extra register/push. The two vtable-adjustment "deleting dtor" calls (vtbl[1] read as a byte displacement, this-adjusted, then that pointer's own vtbl[0] called with arg 1) still diverge by a few instructions each - same register-allocator nuance as 0x004AD9C0's second call site, not chased further.
// working copy - scaffold materialised by --work
// size      190 bytes
// prototype int (__thiscall ?init@Dialog@@QAEHHHHHPAUHeap@@@Z)(Dialog* this, int, int, int, int, Heap*)
// callers   2   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00608F50
// indirect  0x00609793 0x006097A7 0x006097C2

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?init@Dialog@@QAEHHHHHPAUHeap@@@Z  at 0x00609730  (190 bytes)
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

typedef int BOOL;
class Dialog;
struct DialogEntry;
class Font;
typedef void * HFONT;
class Heap;
typedef char * LPSTR;
typedef void * LPVOID;

// ---- callees, declared and never defined (a definition would be inlined) ----
struct DialogEntry {
    uint32_t vtable;
    int id;
    void * payload;
    DialogEntry * next;
    DialogEntry * previous;
    uint32_t secondary_vtable;
    void * heap;
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

class Dialog { public:
    LPVOID vtable_;
    Heap heap_;
    Heap * heap_ptr_;
    uint32_t field_1C_;
    uint32_t field_20_;
    uint32_t field_24_;
    uint32_t field_28_;
    uint32_t field_2C_;
    uint32_t field_30_;
    uint32_t field_34_;
    uint32_t field_38_;
    uint32_t field_3C_;
    uint32_t field_40_;
    uint32_t field_44_;
    uint32_t field_48_;
    uint32_t field_4C_;
    uint32_t field_50_;
    uint32_t field_54_;
    uint32_t field_58_;
    uint32_t field_5C_;
    uint32_t field_60_;
    uint32_t field_64_;
    uint32_t field_68_;
    uint32_t field_6C_;
    Font * font1_;
    Font * font2_;
    Font * font3_;
    uint32_t text_color_a_;
    uint32_t text_color_2a_;
    uint32_t text_color_3a_;
    uint32_t text_color_b_;
    uint32_t text_color_2b_;
    uint32_t text_color_3b_;
    uint32_t text_color_c_;
    uint32_t text_color_2c_;
    uint32_t text_color_3c_;
    uint32_t text_color_d_;
    uint32_t text_color_2d_;
    uint32_t text_color_3d_;
    uint32_t field_AC_;
    uint32_t field_B0_;
    uint32_t field_B4_;
    uint32_t field_B8_;
    uint32_t field_BC_;
    uint32_t field_C0_;
    DialogEntry * entry_head_;
    DialogEntry * current_entry_;
    int entry_count_;
    int entry_position_;
    uint32_t field_D4_;
    uint32_t field_D8_;
    uint32_t field_DC_;
    uint32_t field_E0_;
    uint32_t field_E4_;
    uint32_t field_E8_;
    int selected_position_;
    uint32_t field_F0_;

    void close();
    int init(int, int, int, int, Heap *);
};
class MgrVCall { public:
    virtual void slot0();
    virtual void slot1(void *);  // <-- used
};

class AdjVCall { public:
    virtual void slot0(int);  // <-- used
};

int Dialog::init(int a1, int a2, int a3, int a4, Heap * a5) {
    close();
    char *p = reinterpret_cast<char *>(this);
    *reinterpret_cast<int *>(p + 0x28) = a2;
    *reinterpret_cast<int *>(p + 0x24) = a1;
    *reinterpret_cast<uint32_t *>(p + 0x20) |= 3;
    *reinterpret_cast<int *>(p + 0x2c) = a3;
    *reinterpret_cast<int *>(p + 0x30) = a4;
    *reinterpret_cast<Heap **>(p + 0x18) = a5;

    p += 0xbc;
    if (*reinterpret_cast<DialogEntry **>(p + 8) != 0) {
        int i;
        for (i = 0; i < *reinterpret_cast<int *>(p + 0x10); ++i) {
            DialogEntry *item = *reinterpret_cast<DialogEntry **>(p + 8);
            *reinterpret_cast<DialogEntry **>(p + 0xc) = item->next;
            void *payload = item->payload;
            reinterpret_cast<MgrVCall *>(p)->slot1(payload);
            if (payload != 0) {
                char *vtbl = *reinterpret_cast<char **>(payload);
                int disp = *reinterpret_cast<int *>(vtbl + 4);
                char *adjusted = reinterpret_cast<char *>(payload) + disp;
                reinterpret_cast<AdjVCall *>(adjusted)->slot0(1);
            }
            (*reinterpret_cast<DialogEntry **>(p + 8))->payload = 0;
            DialogEntry *item2 = *reinterpret_cast<DialogEntry **>(p + 8);
            if (item2 != 0) {
                char *vtbl2 = *reinterpret_cast<char **>(item2);
                int disp2 = *reinterpret_cast<int *>(vtbl2 + 4);
                char *adjusted2 = reinterpret_cast<char *>(item2) + disp2;
                reinterpret_cast<AdjVCall *>(adjusted2)->slot0(1);
            }
            *reinterpret_cast<DialogEntry **>(p + 8) = *reinterpret_cast<DialogEntry **>(p + 0xc);
        }
        *reinterpret_cast<DialogEntry **>(p + 8) = 0;
        *reinterpret_cast<int *>(p + 0x14) = 0;
        *reinterpret_cast<int *>(p + 0x10) = 0;
    }
    *reinterpret_cast<int *>(p + 0x14) = 0;
    *reinterpret_cast<uint32_t *>(p + 0x18) = (uint32_t)a5;
    return 0;
}
