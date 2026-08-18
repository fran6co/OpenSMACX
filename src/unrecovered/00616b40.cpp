// ORIGINAL: 0x00616B40 sub_616b40 0x00616B40-0x00616BB6 FILE
// working copy - scaffold materialised by --work
// size      118 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_616b40  at 0x00616B40  (118 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" void free(void *);
class Obj616b40 {
 public:
  void sub_616b40();
};

void Obj616b40::sub_616b40() {
    int *self = reinterpret_cast<int *>(this);
    if (self[0] != 0) {
        int i = 0;
        if (0 < self[1]) {
            int offset = 0;
            do {
                int v0 = *(int *)(offset + self[0]);
                if (v0 != 0) {
                    free((void *)v0);
                }
                int v1 = *(int *)(offset + 4 + self[0]);
                if (v1 != 0) {
                    free((void *)v1);
                }
                int v2 = *(int *)(offset + 8 + self[0]);
                if (v2 != 0) {
                    free((void *)v2);
                }
                i = i + 1;
                offset = offset + 0xc;
            } while (i < self[1]);
        }
        if (self[0] != 0) {
            free((void *)self[0]);
        }
        self[0] = 0;
    }
    self[1] = 0;
    self[2] = 0;
}
