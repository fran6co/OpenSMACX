// ORIGINAL: 0x006098D0 FILE
// working copy - scaffold materialised by --work
// name      ?init@Dialog@@QAEHPAUHeap@@@Z
// size      159 bytes
// spans     0x006098D0-0x0060996F
// prototype int (__thiscall ?init@Dialog@@QAEHPAUHeap@@@Z)(Dialog* this, Heap*)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00608F50
// indirect  0x00609915 0x00609927 0x00609943

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?init@Dialog@@QAEHPAUHeap@@@Z  at 0x006098D0  (159 bytes)
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

// A second vtable shim: the generated VCall above types every slot
// void(), but both slots here take one argument.
class VCall2 { public:
    virtual void slot000(int);
    virtual void slot001(void *);
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
    int init(Heap *);
};
int Dialog::init(Heap * a1) {
    close();
    field_1C_ = 0xffffffff;
    heap_ptr_ = a1;
    if (entry_head_ != 0) {
        int count = entry_count_;
        if (count > 0) {
            for (int i = 0; i < count; ++i) {
                DialogEntry *cur = entry_head_;
                current_entry_ = cur->next;
                void *payload = cur->payload;
                ((VCall2 *)&field_BC_)->slot001(payload);
                if (payload != 0) {
                    int *vt = *(int **)payload;
                    int adjustor = *(int *)((char *)vt + 4);
                    VCall2 *adj = (VCall2 *)((char *)payload + adjustor);
                    adj->slot000(1);
                }
                entry_head_->payload = 0;
                if (entry_head_ != 0) {
                    int *vt2 = *(int **)entry_head_;
                    int adjustor2 = *(int *)((char *)vt2 + 4);
                    VCall2 *adj2 = (VCall2 *)((char *)entry_head_ + adjustor2);
                    adj2->slot000(1);
                }
                entry_head_ = current_entry_;
            }
        }
        entry_head_ = 0;
        entry_position_ = 0;
        entry_count_ = 0;
    }
    entry_position_ = 0;
    field_D4_ = (uint32_t)a1;
    return 0;
}
