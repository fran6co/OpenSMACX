// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00608F50
// name           ?close@Dialog@@QAEXXZ
// size           499 bytes
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00608f50/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?close@Dialog@@QAEXXZ  at 0x00608F50  (499 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
class Heap { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
    ~Heap();
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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00697098 = (int *)0x00697098;
static int *const g_0069709c = (int *)0x0069709C;
static int *const g_006970a0 = (int *)0x006970A0;
static int *const g_006970a4 = (int *)0x006970A4;
static int *const g_006970a8 = (int *)0x006970A8;
static int *const g_006970ac = (int *)0x006970AC;
static int *const g_006970b0 = (int *)0x006970B0;
static int *const g_006970b4 = (int *)0x006970B4;
static int *const g_006970b8 = (int *)0x006970B8;
static int *const g_006970bc = (int *)0x006970BC;
static int *const g_006970c0 = (int *)0x006970C0;
static int *const g_006970c4 = (int *)0x006970C4;
static int *const g_006970c8 = (int *)0x006970C8;
static int *const g_006970cc = (int *)0x006970CC;
static int *const g_006970d0 = (int *)0x006970D0;
static int *const g_006970d4 = (int *)0x006970D4;
static int *const g_006970d8 = (int *)0x006970D8;
static int *const g_006970dc = (int *)0x006970DC;
static int *const g_009b8eb4 = (int *)0x009B8EB4;
static int *const g_009b8eb8 = (int *)0x009B8EB8;
static int *const g_009b8ebc = (int *)0x009B8EBC;
static int *const g_009b8ec0 = (int *)0x009B8EC0;
static int *const g_009b8ec4 = (int *)0x009B8EC4;
static int *const g_009b8ec8 = (int *)0x009B8EC8;
static int *const g_009b8ecc = (int *)0x009B8ECC;
static int *const g_009b8ed0 = (int *)0x009B8ED0;
static int *const g_009b8ed4 = (int *)0x009B8ED4;
static int *const g_009b8ed8 = (int *)0x009B8ED8;

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
};

// This body's two delta-adjusted destroy calls and the list-object's
// remove call all pass an argument where VCall's slots are nullary.
class VCallW { public:
    virtual void slot000(int);    // <-- used, delta-adjusted destroy(flag)
    virtual void slot001(void *); // <-- used, list->remove(payload)
};

void Dialog::close() {
    int i;
    if (entry_head_ != 0) {
        if (entry_count_ > 0) {
            i = 0;
            do {
                DialogEntry *node = entry_head_;
                current_entry_ = node->next;
                void *payload = node->payload;
                reinterpret_cast<VCallW *>(&field_BC_)->slot001(payload);
                if (payload != 0) {
                    char *p = reinterpret_cast<char *>(payload);
                    int delta = *reinterpret_cast<int *>(*reinterpret_cast<int *>(p) + 4);
                    reinterpret_cast<VCallW *>(p + delta)->slot000(1);
                }
                entry_head_->payload = 0;
                if (entry_head_ != 0) {
                    char *p = reinterpret_cast<char *>(entry_head_);
                    int delta = *reinterpret_cast<int *>(*reinterpret_cast<int *>(p) + 4);
                    reinterpret_cast<VCallW *>(p + delta)->slot000(1);
                }
                entry_head_ = current_entry_;
                ++i;
            } while (i < entry_count_);
        }
        entry_head_ = 0;
        entry_position_ = 0;
        entry_count_ = 0;
    }
    entry_position_ = 0;
    heap_.~Heap();
    heap_ptr_ = 0;
    field_20_ = *g_006970dc;
    field_24_ = 0;
    field_28_ = 0;
    field_2C_ = *g_00697098;
    field_30_ = *g_009b8eb4;
    selected_position_ = 0;
    field_F0_ = -1;
    field_38_ = 0;
    field_3C_ = 0;
    field_40_ = *g_009b8ebc;
    field_44_ = *g_0069709c;
    field_48_ = -1;
    field_34_ = *g_009b8eb8;
    field_4C_ = *g_006970a0;
    field_50_ = *g_006970a4;
    field_54_ = 0;
    field_58_ = 0;
    field_5C_ = 0;
    field_60_ = 0;
    field_64_ = 0;
    field_68_ = 0;
    field_6C_ = *g_006970a8;
    field_AC_ = *g_009b8ecc;
    field_B0_ = *g_009b8ed0;
    field_B4_ = *g_009b8ed4;
    field_B8_ = *g_009b8ed8;
    font1_ = reinterpret_cast<Font *>(*g_009b8ec0);
    text_color_a_ = *g_006970ac;
    text_color_b_ = *g_006970b8;
    text_color_c_ = *g_006970c4;
    text_color_d_ = *g_006970d0;
    font2_ = reinterpret_cast<Font *>(*g_009b8ec4);
    text_color_2a_ = *g_006970b0;
    text_color_2b_ = *g_006970bc;
    text_color_2c_ = *g_006970c8;
    text_color_2d_ = *g_006970d4;
    font3_ = reinterpret_cast<Font *>(*g_009b8ec8);
    text_color_3a_ = *g_006970b4;
    text_color_3b_ = *g_006970c0;
    text_color_3c_ = *g_006970cc;
    text_color_3d_ = *g_006970d8;
    field_1C_ = -1;
}
