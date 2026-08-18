// ORIGINAL: 0x00638090 Video_unk14 0x00638090-0x00638128 FILE
// RULED-OUT: MISMATCH #0 mov/push, prologue register choice differs
// PROPOSAL: extern "C" int __stdcall Video_unk14(int) -> VideoUnk14Recv::Video_unk14(int),
//           __thiscall member: ecx reads (0x4b0,0x1008,0xff8,...) with no matching stack slot
// working copy - scaffold materialised by --work
// size      152 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: Video_unk14  at 0x00638090  (152 bytes)
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
// ecx reads with no matching stack slot -> this is a __thiscall receiver,
// not the stdcall(int) IDA guessed. See PROPOSALS in the report.
class VideoUnk14Recv { public:
    int Video_unk14(int a1);
};

int VideoUnk14Recv::Video_unk14(int a1) {
    char *self = (char *)this;
    int *state = *(int **)(self + 0x4b0);
    if (state == 0) {
        return -1;
    }
    int *rec = *(int **)(self + 0x1008);
    if (rec == 0) {
        return -1;
    }
    int count = *(int *)(self + 0xff8);
    if (a1 >= count || a1 < 0) {
        return -1;
    }

    int marker = *(int *)(self + 0x598);
    int *cur = rec;
    while (*cur != marker) {
        cur = cur + 4;
    }

    int idx = a1;
    if (a1 > 0) {
        do {
            int *scan = cur;
            while (scan[4] != marker) {
                scan = scan + 4;
            }
            cur = scan + 4;
            idx = idx - 1;
        } while (idx != 0);

        while (a1 > 0 && (*((unsigned char *)cur + 4) & 0x10) == 0) {
            int *scan = cur;
            while (scan[-4] != marker) {
                scan = scan - 4;
            }
            cur = scan - 4;
            a1 = a1 - 1;
        }
    }

    if (a1 == 0) {
        return -1;
    }
    *(int **)(self + 0x100c) = cur;
    return a1;
}
