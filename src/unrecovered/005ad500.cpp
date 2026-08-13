// ORIGINAL: 0x005AD500 BYTE_EXACT FILE
// working copy - scaffold materialised by --work
// name      sub_5ad500
// size      96 bytes
// spans     0x005AD500-0x005AD560
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5ad500  at 0x005AD500  (96 bytes)
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

// `ecx+0xa24` with no stack access -> __thiscall member; `ret 0xc` pops
// the 3 explicit ints (a1, a2, a3), `this` rides in ecx uncounted. A
// table of 0x200 3-int records starting at this+0xA20 (key1, key2,
// value); search for key1==a1 && key2==a2, then invalidate the record.
struct Slot5AD500 { int key1; int key2; int value; };

class Table5AD500 { public:
    uint8_t pad_0_[0xA20];
    Slot5AD500 slots_[0x200];

    void remove(int a1, int a2, int a3);
};

void Table5AD500::remove(int a1, int a2, int a3) {
    for (int i = 0; i < 0x200; ++i) {
        if (slots_[i].key1 == a1 && slots_[i].key2 == a2) {
            slots_[i].value = -1;
            slots_[i].key2 = -1;
            slots_[i].key1 = -1;
            return;
        }
    }
}
