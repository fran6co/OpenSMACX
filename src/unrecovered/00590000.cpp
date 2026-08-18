// ORIGINAL: 0x00590000 ?lock@PlayerLock@@QAEHHHHHHHH@Z 0x00590000-0x00590093 FILE
// RULED-OUT: real SquareLock member calls (first_/second_/flag_, lock/unlock) already declared by the scaffold; store order flag_,second_, first_ (not declaration order) matches the disassembly's init loop. MISMATCH #3 'push' vs 'mov' remains open.
// working copy - scaffold materialised by --work
// size      147 bytes
// prototype int (__thiscall ?lock@PlayerLock@@QAEHHHHHHHH@Z)(PlayerLock* this, int, int, int, int, int, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0058FD90 0x0058FE80

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?lock@PlayerLock@@QAEHHHHHHHH@Z  at 0x00590000  (147 bytes)
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

class PlayerLock;
class SquareLock;

// ---- callees, declared and never defined (a definition would be inlined) ----
class SquareLock { public:
    int32_t first_;
    int32_t second_;
    int32_t flag_;
    int lock(int, int, int, int);
    void unlock(int);
};


class PlayerLock { public:
    int lock(int, int, int, int, int, int, int);
};
int PlayerLock::lock(int a1, int a2, int a3, int a4, int a5, int a6, int a7) {
    char *self = reinterpret_cast<char *>(this);
    SquareLock *lock1 = reinterpret_cast<SquareLock *>(self + 4);
    SquareLock *lock2 = reinterpret_cast<SquareLock *>(self + 0x10);

    lock1->flag_ = 0;
    lock1->second_ = -1;
    lock1->first_ = -1;
    lock2->flag_ = 0;
    lock2->second_ = -1;
    lock2->first_ = -1;

    *self = 0;

    if (lock1->lock(a1, a2, a3, a4) == 0) {
        if (lock2->lock(a1, a5, a6, a7) == 0) {
            return 0;
        }
    }

    lock1->unlock(a1);
    lock2->unlock(a1);

    *self = 0;
    return 1;
}
