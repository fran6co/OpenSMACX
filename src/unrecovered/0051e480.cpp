// ORIGINAL: 0x0051E480 sub_51e480 0x0051E480-0x0051E523 FILE
// TRIED: named `adj2` local for the second this-adjusted vtable call (mismatched: original uses `lea` there, not `add`, unlike the first call which does match named). MISMATCH #41 is the sole remaining divergence at 98.5% mnemonic similarity.
// working copy - scaffold materialised by --work
// size      163 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0051E4B8 0x0051E4CA 0x0051E4E6

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_51e480  at 0x0051E480  (163 bytes)
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
static int *const g_0093a7b0 = (int *)0x0093A7B0;
// g_0093a96c/g_0093a98c are INDEXED table bases (`[eax*4+ADDR]`), not plain
// loads - the const-pointer spelling folds the `lea` into a separate `add`.
// Declared as arrays instead so the address itself does the indexing work.
extern int g_0093a96c[];
extern int g_0093a98c[];

// A second vtable shim: the generated VCall above types every slot
// void(), but both slots here take one argument.
class VCall2 { public:
    virtual void slot000(int);
    virtual void slot001(void *);
};

struct Entry51e480 {
    uint32_t vtable;
    int id;
    void * payload;
    Entry51e480 * next;
    Entry51e480 * previous;
    uint32_t secondary_vtable;
    void * heap;
};

struct EntryListMgr {
    uint32_t vtable_or_ops;
    uint32_t field_04_;
    Entry51e480 * entry_head_;
    Entry51e480 * current_entry_;
    int entry_count_;
    uint32_t field_14_;
    uint32_t field_18_;
};

extern "C" void __cdecl sub_51e480(int a1) {
    EntryListMgr *rec = reinterpret_cast<EntryListMgr *>(reinterpret_cast<char *>(g_0093a7b0) + a1 * 0x30);
    if (rec->entry_head_ != 0) {
        int i = 0;
        if (rec->entry_count_ > 0) {
            do {
                rec->current_entry_ = rec->entry_head_->next;
                void *payload = rec->entry_head_->payload;
                ((VCall2 *)rec)->slot001(payload);
                if (payload != 0) {
                    int *vt = *(int **)payload;
                    int adjustor = *(int *)((char *)vt + 4);
                    VCall2 *adj = (VCall2 *)((char *)payload + adjustor);
                    adj->slot000(1);
                }
                rec->entry_head_->payload = 0;
                if (rec->entry_head_ != 0) {
                    int *vt2 = *(int **)rec->entry_head_;
                    int adjustor2 = *(int *)((char *)vt2 + 4);
                    ((VCall2 *)((char *)rec->entry_head_ + adjustor2))->slot000(1);
                }
                rec->entry_head_ = rec->current_entry_;
                ++i;
            } while (i < rec->entry_count_);
        }
        rec->entry_head_ = 0;
        rec->field_14_ = 0;
        rec->entry_count_ = 0;
    }
    rec->field_14_ = 0;
    rec->field_18_ = 0;
    g_0093a98c[a1] = 0;
    g_0093a96c[a1] = 0;
}
