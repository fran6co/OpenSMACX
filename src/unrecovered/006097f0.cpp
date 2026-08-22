// ORIGINAL: 0x006097F0 ?init@Dialog@@QAEHPAURECT@@PAUHeap@@@Z 0x006097F0-0x006098C8 FILE
// TRIED: separate width/height locals vs `right -= left; bottom -= top;` in place - both diverge at the same instruction (field_2c/30 store). This is the "this-adjusting thunk" shape (vtbl[1] holds a byte offset, not a function pointer) modelled with a single-virtual-slot Slot0Iface + raw pointer chasing.
// working copy - scaffold materialised by --work
// size      216 bytes
// prototype int (__thiscall ?init@Dialog@@QAEHPAURECT@@PAUHeap@@@Z)(Dialog* this, RECT*, Heap*)
// callers   4   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00608F50
// indirect  0x0060986D 0x00609881 0x0060989C

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?init@Dialog@@QAEHPAURECT@@PAUHeap@@@Z  at 0x006097F0  (216 bytes)
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
struct RECT;

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

struct RECT {
    long left;
    long top;
    long right;
    long bottom;
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
    int init(RECT *, Heap *);
};
class Slot0Iface { public:
    virtual void call0(int);
};

int Dialog::init(RECT * a1, Heap * a2) {
    char *self = reinterpret_cast<char *>(this);
    if (a1 == 0) {
        return 3;
    }
    int left = a1->left;
    int top = a1->top;
    int right = a1->right;
    int bottom = a1->bottom;
    int width = right - left;
    int height = bottom - top;
    close();
    *reinterpret_cast<int *>(self + 0x24) = left;
    *reinterpret_cast<unsigned int *>(self + 0x20) |= 3;
    *reinterpret_cast<int *>(self + 0x28) = top;
    *reinterpret_cast<int *>(self + 0x2C) = width;
    *reinterpret_cast<int *>(self + 0x30) = height;
    *reinterpret_cast<Heap **>(self + 0x18) = a2;

    if (*reinterpret_cast<void **>(self + 0xC4) != 0) {
        int count = *reinterpret_cast<int *>(self + 0xCC);
        int index = 0;
        if (count > 0) {
            do {
                char *head = *reinterpret_cast<char **>(self + 0xC4);
                *reinterpret_cast<int *>(self + 0xC8) = *reinterpret_cast<int *>(head + 0xC);
                void *field8 = *reinterpret_cast<void **>(head + 8);

                char *fbcVtbl = *reinterpret_cast<char **>(self + 0xBC);
                typedef void (__stdcall *FnT)(void *);
                FnT fn = *reinterpret_cast<FnT *>(fbcVtbl + 4);
                fn(field8);

                if (field8 != 0) {
                    char *vtbl = *reinterpret_cast<char **>(field8);
                    int offset = *reinterpret_cast<int *>(vtbl + 4);
                    Slot0Iface *adj = reinterpret_cast<Slot0Iface *>(reinterpret_cast<char *>(field8) + offset);
                    adj->call0(1);
                }

                head = *reinterpret_cast<char **>(self + 0xC4);
                *reinterpret_cast<int *>(head + 8) = 0;
                head = *reinterpret_cast<char **>(self + 0xC4);
                if (head != 0) {
                    char *vtbl2 = *reinterpret_cast<char **>(head);
                    int offset2 = *reinterpret_cast<int *>(vtbl2 + 4);
                    Slot0Iface *adj2 = reinterpret_cast<Slot0Iface *>(head + offset2);
                    adj2->call0(1);
                }

                index++;
                *reinterpret_cast<int *>(self + 0xC4) = *reinterpret_cast<int *>(self + 0xC8);
                count = *reinterpret_cast<int *>(self + 0xCC);
            } while (index < count);
        }
        *reinterpret_cast<int *>(self + 0xC4) = 0;
        *reinterpret_cast<int *>(self + 0xD0) = 0;
        *reinterpret_cast<int *>(self + 0xCC) = 0;
    }
    *reinterpret_cast<int *>(self + 0xD0) = 0;
    *reinterpret_cast<Heap **>(self + 0xD4) = a2;
    return 0;
}
