// ORIGINAL: 0x0062D5F0 FILE
// RULED-OUT: calling `strcat(dst, src)` by name directly - VC6 recognises
//            it as an intrinsic and inlines it (repne scasb/not/sub), which
//            the original does NOT do (it's a real `call _strcat`).
//            Casting through a `StrcatFn` function-pointer type at the call
//            site defeats the substitution and lifts similarity from 0.78
//            to 0.855; a local variable holding the same cast changes the
//            prologue's register count instead, no better. MISMATCH #3
//            'push' vs 'mov' remains open (prologue register allocation).
// working copy - scaffold materialised by --work
// name      sub_62d5f0
// size      173 bytes
// spans     0x0062D5F0-0x0062D69D
// prototype 
// callers   1   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x0062F8A0 0x00644EF2 0x00645470

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_62d5f0  at 0x0062D5F0  (173 bytes)
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

class Net;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Net { public:
    int send_packet_type(void *, int, unsigned long, int, int);
};

extern "C" char *strcat(char *, const char *);
extern "C" void free(void *);
void * mem_get(int);
typedef char *(__cdecl *StrcatFn)(char *, const char *);

// No stack slot matches the values read at [ecx+N]; this is a
// __thiscall receiver, not the nullary cdecl the given head guessed.
// See PROPOSALS in the report.
class Sub62d5f0Recv { public:
    void sub_62d5f0();
};

void Sub62d5f0Recv::sub_62d5f0() {
    char *self = reinterpret_cast<char *>(this);

    if (*reinterpret_cast<int *>(self + 0x760) == *reinterpret_cast<int *>(self + 0x764)) {
        int count = *reinterpret_cast<int *>(self + 0x6dc);
        if (count != 0) {
            int size = count * 0x2c + 4;
            unsigned char *buf = reinterpret_cast<unsigned char *>(mem_get(size));
            *buf = static_cast<unsigned char>(count);
            if (count > 0) {
                unsigned char *dst = buf + 8;
                char *src = self + 0x188;
                int n = count;
                do {
                    *reinterpret_cast<int *>(dst - 4) = *reinterpret_cast<int *>(src - 0x34);
                    *dst = 0;
                    *src = 0;
                    // Cast through a function-pointer type so VC6 emits a
                    // real CALL instead of inlining strcat as an intrinsic.
                    (*reinterpret_cast<StrcatFn>(strcat))(reinterpret_cast<char *>(dst), src - 0x1f);
                    unsigned char flag = *reinterpret_cast<unsigned char *>(src - 0x20);
                    src += 0x58;
                    *reinterpret_cast<unsigned int *>(dst + 0x20) = flag;
                    dst += 0x28;
                    --n;
                } while (n != 0);
            }
            reinterpret_cast<Net *>(self)->send_packet_type(buf, size, 0, 0x10, 1);
            if (buf != 0) {
                free(buf);
            }
        }
    }
}
