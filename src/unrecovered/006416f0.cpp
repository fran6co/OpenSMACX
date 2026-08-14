// ORIGINAL: 0x006416F0 FILE
// working copy - scaffold materialised by --work
// name      sub_6416f0
// size      209 bytes
// spans     0x006416F0-0x006417C1
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_6416f0  at 0x006416F0  (209 bytes)
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
// Signature changed from the fallback `extern "C" int __cdecl(void)`: the
// disassembly reads three stack args before any push ([esp+4], and after
// the prologue [esp+0x18]=arg3, [esp+0x1c]=arg2/arg1 slot) and never sets
// eax on return, matching IDA's 3-arg guess with a void return - a heap
// sift-down over parallel arrays at param_1+0xb50/+0xb54/+0x144c.
extern "C" void __cdecl sub_6416f0(int param_1, int param_2, int param_3) {
    int n = *reinterpret_cast<int *>(param_1 + 0x1444);
    int val = *reinterpret_cast<int *>(param_1 + 0xb50 + param_3 * 4);
    int child = param_3 * 2;

    if (child > n) {
        *reinterpret_cast<int *>(param_1 + 0xb50 + param_3 * 4) = val;
        return;
    }

    for (;;) {
        int best = child;
        if (child < n) {
            int left = *reinterpret_cast<int *>(param_1 + 0xb54 + child * 4);
            int right = *reinterpret_cast<int *>(param_1 + 0xb50 + child * 4);
            unsigned short leftKey = *reinterpret_cast<unsigned short *>(param_2 + left * 4);
            unsigned short rightKey = *reinterpret_cast<unsigned short *>(param_2 + right * 4);
            if (leftKey < rightKey ||
                (leftKey == rightKey &&
                 *reinterpret_cast<unsigned char *>(param_1 + 0x144c + left) <=
                 *reinterpret_cast<unsigned char *>(param_1 + 0x144c + right))) {
                best = child + 1;
            }
        }
        int candidate = *reinterpret_cast<int *>(param_1 + 0xb50 + best * 4);
        unsigned short valKey = *reinterpret_cast<unsigned short *>(param_2 + val * 4);
        unsigned short candKey = *reinterpret_cast<unsigned short *>(param_2 + candidate * 4);
        if (valKey < candKey) {
            break;
        }
        if (valKey == candKey &&
            *reinterpret_cast<unsigned char *>(param_1 + 0x144c + val) <=
            *reinterpret_cast<unsigned char *>(param_1 + 0x144c + candidate)) {
            *reinterpret_cast<int *>(param_1 + 0xb50 + param_3 * 4) = val;
            return;
        }
        *reinterpret_cast<int *>(param_1 + 0xb50 + param_3 * 4) = candidate;
        param_3 = best;
        child = best * 2;
        n = *reinterpret_cast<int *>(param_1 + 0x1444);
        if (child > n) {
            break;
        }
    }
    *reinterpret_cast<int *>(param_1 + 0xb50 + param_3 * 4) = val;
}
