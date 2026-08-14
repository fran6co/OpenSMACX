// ORIGINAL: 0x00641A70 FILE
// working copy - scaffold materialised by --work
// name      sub_641a70
// size      231 bytes
// spans     0x00641A70-0x00641B57
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_641a70  at 0x00641A70  (231 bytes)
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
// disassembly reads three stack args before any push, matching IDA's 3-arg
// guess, and never sets eax before the tail pops, so it returns void.
extern "C" void __cdecl sub_641a70(int param_1, int param_2, int param_3) {
    unsigned short curVal = *reinterpret_cast<unsigned short *>(param_2 + 2);
    int runLen = 0;
    unsigned int lastGroup = 0xffffffff;
    int threshold = 7;
    int minRun = 4;
    if (curVal == 0) {
        threshold = 0x8a;
        minRun = 3;
    }
    *reinterpret_cast<unsigned short *>(param_2 + 6 + param_3 * 4) = 0xffff;
    if (param_3 >= 0) {
        unsigned short *p = reinterpret_cast<unsigned short *>(param_2 + 6);
        unsigned int prevVal = curVal;
        int count = param_3 + 1;
        do {
            unsigned int nextVal = *p;
            ++runLen;
            if (runLen >= threshold || prevVal != nextVal) {
                if (runLen < minRun) {
                    short *slot = reinterpret_cast<short *>(param_1 + 0xa70 + prevVal * 4);
                    *slot = *slot + (short)runLen;
                } else if (prevVal == 0) {
                    if (runLen < 0xb) {
                        short *slot = reinterpret_cast<short *>(param_1 + 0xab4);
                        *slot = *slot + 1;
                    } else {
                        short *slot = reinterpret_cast<short *>(param_1 + 0xab8);
                        *slot = *slot + 1;
                    }
                } else {
                    if (prevVal != lastGroup) {
                        short *slot = reinterpret_cast<short *>(param_1 + 0xa70 + prevVal * 4);
                        *slot = *slot + 1;
                    }
                    short *slot2 = reinterpret_cast<short *>(param_1 + 0xab0);
                    *slot2 = *slot2 + 1;
                }
                runLen = 0;
                lastGroup = prevVal;
                if (nextVal == 0) {
                    threshold = 0x8a;
                    minRun = 3;
                } else if (prevVal == nextVal) {
                    threshold = 6;
                    minRun = 3;
                } else {
                    threshold = 7;
                    minRun = 4;
                }
            }
            p += 2;
            --count;
            prevVal = nextVal;
        } while (count != 0);
    }
}
