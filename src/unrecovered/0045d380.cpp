// ORIGINAL: 0x0045D380 ?release_iface_mode@SubInterface@@QAEXXZ 0x0045D380-0x0045D43F FILE
// TRIED: byte-exact - a stack-search-and-shift over a linear "iface mode" stack (0x7AE778) plus a parallel object table (0x7AE7F0); the found-and-shifted case does not break, it keeps scanning downward exactly like the disassembly, so no attempt used `break`. 59% mnemonic similarity; landed for the control-flow shape (nested for-loops, no early exit) rather than chase the remaining scheduling/frame differences.
// working copy - scaffold materialised by --work
// size      191 bytes
// prototype void (__thiscall ?release_iface_mode@SubInterface@@QAEXXZ)(SubInterface* this)
// callers   24   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D8000
// indirect  0x0045D417 0x0045D437

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?release_iface_mode@SubInterface@@QAEXXZ  at 0x0045D380  (191 bytes)
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
class SubInterface;

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
    int set_clip(RECT *);
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
// This body dispatches through slot(s): 0
class VCall { public:
    virtual void slot000();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_007ae778 = (int *)0x007AE778;
static int *const g_007ae7f0 = (int *)0x007AE7F0;
static int *const g_007aec64 = (int *)0x007AEC64;
static int *const g_007aec94 = (int *)0x007AEC94;
static int *const g_007d392c = (int *)0x007D392C;

// The two tables at 0x7AE778/0x7AE7F0 are indexed (lea/add of a scaled
// index), so the fixed-address const-pointer spelling above is wrong here -
// declared as our own extern arrays instead; the relocation is masked by
// the comparison.
extern int g_table_7ae778[];
extern int g_table_7ae7f0[];

// Indirect calls at 0x45D417/0x45D437 take an explicit int argument, so the
// scaffold's nullary VCall slot000 does not fit; a second shim with the
// right signature.
class VCallArg { public:
    virtual void slot000(int);
};

class SubInterface { public:
    uint32_t vtable_;
    uint32_t field_4_;

    void release_iface_mode();
};
void SubInterface::release_iface_mode() {
    int depth = *g_007d392c;
    if (depth != 0) {
        char *self = reinterpret_cast<char *>(this);
        int myid = *reinterpret_cast<int *>(self + 4);
        if (g_table_7ae778[depth] == myid) {
            depth -= 1;
            *g_007d392c = depth;
            int newtop = g_table_7ae778[depth];
            if (myid != newtop) {
                VCallArg *obj = reinterpret_cast<VCallArg *>(g_table_7ae7f0[myid]);
                if (obj != 0) {
                    obj->slot000(0);
                }
            }
            reinterpret_cast<Buffer *>(g_007aec64)->set_clip(reinterpret_cast<RECT *>(g_007aec94));
            VCallArg *obj2 = reinterpret_cast<VCallArg *>(g_table_7ae7f0[newtop]);
            if (obj2 != 0) {
                obj2->slot000(1);
            }
        } else {
            int i = depth - 1;
            if (i >= 1) {
                for (; i >= 1; i -= 1) {
                    if (g_table_7ae778[i] == myid) {
                        if (i < depth) {
                            for (int k = i; k < depth; k += 1) {
                                g_table_7ae778[k] = g_table_7ae778[k + 1];
                            }
                        }
                        depth -= 1;
                        *g_007d392c = depth;
                    }
                }
            }
        }
    }
}
