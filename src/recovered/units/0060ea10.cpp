// ORIGINAL: 0x0060EA10 FILE
// name      ?flip_state_id@CheckBox@@QAEXH@Z
// size      312 bytes
// spans     0x0060EA10-0x0060EB48
// prototype void (__thiscall ?flip_state_id@CheckBox@@QAEXH@Z)(CheckBox* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0060EA10
// measured tier  MISMATCH
// divergence     1
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0060ea10/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?flip_state_id@CheckBox@@QAEXH@Z  at 0x0060EA10  (312 bytes)
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

class CheckBox;

class CheckBox { public:
    void flip_state_id(int);
};

void CheckBox::flip_state_id(int a1) {
    char *self = reinterpret_cast<char *>(this);

    {
        // vtable slot 2 (offset 8) holds this widget's own-data DELTA, the
        // same "add an offset out of my own vtable" idiom used elsewhere
        // (e.g. MapWin::on_button_clicked). base1 is this+delta+0xbc; the
        // fields touched below (+0xc/+0x10/+0x14 off base1) are absolute
        // this+delta+0xc8/0xcc/0xd0 - the SAME three fields the two blocks
        // below reach through a base with no +0xbc folded in.
        char *base1 = self + (*reinterpret_cast<int **>(self))[2] + 0xbc;
        int head = *reinterpret_cast<int *>(base1 + 8);
        if (head != 0) {
            int maxCount = *reinterpret_cast<int *>(base1 + 0x10);
            int i = 0;
            *reinterpret_cast<int *>(base1 + 0x14) = 0;
            *reinterpret_cast<int *>(base1 + 0xc) = head;
            if (maxCount > 0) {
                do {
                    int node = *reinterpret_cast<int *>(base1 + 0xc);
                    if (*reinterpret_cast<int *>(node + 4) == a1) {
                        break;
                    }
                    ++*reinterpret_cast<int *>(base1 + 0x14);
                    node = *reinterpret_cast<int *>(node + 0xc);
                    ++i;
                    *reinterpret_cast<int *>(base1 + 0xc) = node;
                } while (i < maxCount);
            }
        }
    }

    char *base2 = self + (*reinterpret_cast<int **>(self))[2];
    unsigned int bit = 1u << *reinterpret_cast<int *>(base2 + 0xd0);
    if (*reinterpret_cast<unsigned int *>(base2 + 0xec) & bit) {
        int head = *reinterpret_cast<int *>(base2 + 0xc4);
        if (head != 0) {
            int maxCount = *reinterpret_cast<int *>(base2 + 0xcc);
            int i = 0;
            *reinterpret_cast<int *>(base2 + 0xd0) = 0;
            *reinterpret_cast<int *>(base2 + 0xc8) = head;
            if (maxCount > 0) {
                do {
                    int node = *reinterpret_cast<int *>(base2 + 0xc8);
                    if (*reinterpret_cast<int *>(node + 4) == a1) {
                        break;
                    }
                    ++*reinterpret_cast<int *>(base2 + 0xd0);
                    node = *reinterpret_cast<int *>(node + 0xc);
                    ++i;
                    *reinterpret_cast<int *>(base2 + 0xc8) = node;
                } while (i < maxCount);
            }
        }
        char *base3 = self + (*reinterpret_cast<int **>(self))[2];
        unsigned int bit2 = 1u << *reinterpret_cast<int *>(base3 + 0xd0);
        *reinterpret_cast<unsigned int *>(base3 + 0xec) &= ~bit2;
    } else {
        int head = *reinterpret_cast<int *>(base2 + 0xc4);
        if (head != 0) {
            int maxCount = *reinterpret_cast<int *>(base2 + 0xcc);
            int i = 0;
            *reinterpret_cast<int *>(base2 + 0xd0) = 0;
            *reinterpret_cast<int *>(base2 + 0xc8) = head;
            if (maxCount > 0) {
                do {
                    int node = *reinterpret_cast<int *>(base2 + 0xc8);
                    if (*reinterpret_cast<int *>(node + 4) == a1) {
                        break;
                    }
                    ++*reinterpret_cast<int *>(base2 + 0xd0);
                    node = *reinterpret_cast<int *>(node + 0xc);
                    ++i;
                    *reinterpret_cast<int *>(base2 + 0xc8) = node;
                } while (i < maxCount);
            }
        }
        char *base3 = self + (*reinterpret_cast<int **>(self))[2];
        unsigned int bit2 = 1u << *reinterpret_cast<int *>(base3 + 0xd0);
        *reinterpret_cast<unsigned int *>(base3 + 0xec) |= bit2;
    }
}
