// ORIGINAL: 0x0063C6D0 BYTE_EXACT FILE
// LEVER: ecx/cdecl  the given `extern "C" int __cdecl sub_63c6d0()` head was
//   wrong - every field access reads `[ecx+N]`/`[esi+N]` with `esi=ecx` and no
//   stack argument, so the receiver is `this`. Changing it to a `__thiscall`
//   member of a local helper class is part of what made this match, so it is a
//   lever and not a ruled-out spelling.
// LEVER: reversing a `>=` comparison's operand order (`iVar2 <= *w` instead
//   of `*w >= iVar2`) flipped `jl` to the original's `jg`; and giving the
//   "abs bound exceeded" early-exit its own flag (skipping the trailing
//   `field36bc = iVar1` store) instead of relying on it falling out of a
//   plain if/else, matched the original NOT sharing that store between the
//   negative- and positive-`iVar1` branches - it stores separately in each.
// working copy - scaffold materialised by --work
// name      sub_63c6d0
// size      229 bytes
// spans     0x0063C6D0-0x0063C7B5
// prototype 
// callers   0   call targets   3
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00627200 0x0062E540 0x00644F3A

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_63c6d0  at 0x0063C6D0  (229 bytes)
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
struct SessionStruct;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Net { public:
    int join_session(SessionStruct *, char *, char *);
};

extern "C" int abs(int);
int __cdecl pop(char *, char *, int, int, int (__cdecl *)());

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00698c50 = (int *)0x00698C50;
static int *const g_00698c60 = (int *)0x00698C60;
static int *const g_009bb5e8 = (int *)0x009BB5E8;
#pragma function(abs)
// `this` is the receiver (see RULED-OUT above); no catalogued name exists
// for this address, so the class name and method name are ours to choose.
class T63C6D0 {
public:
    int connect();
};

int T63C6D0::connect() {
    char *self = reinterpret_cast<char *>(this);
    if (pop(reinterpret_cast<char *>(g_00698c60),
            reinterpret_cast<char *>(g_00698c50), 0x3c, 0x44, 0) == -1) {
        return 1;
    }

    char *a50 = *reinterpret_cast<char **>(self + 0xa50);
    char *inner = *reinterpret_cast<char **>(a50 + 8);

    int *field36b8 = reinterpret_cast<int *>(self + 0x36b8);
    int *field36b0 = reinterpret_cast<int *>(self + 0x36b0);
    int *field36b4 = reinterpret_cast<int *>(self + 0x36b4);
    int *field36bc = reinterpret_cast<int *>(self + 0x36bc);

    int iVar1 = *reinterpret_cast<int *>(inner + reinterpret_cast<int>(self) + 0xb3c);
    if (iVar1 <= *field36b8 - 1) {
        *field36b4 = *field36b0;
        int setField = 1;
        if (iVar1 < 0) {
            int iVar2 = abs(iVar1);
            if (iVar2 <= *field36b8) {
                iVar2 = abs(iVar1);
                if (iVar2 > 0) {
                    do {
                        --iVar2;
                        *field36b4 = *reinterpret_cast<int *>(*field36b4 + 0x10);
                    } while (iVar2 != 0);
                }
                iVar1 = iVar1 + *field36b8;
            } else {
                setField = 0;
            }
        } else if (iVar1 > 0) {
            int iVar2 = iVar1;
            do {
                --iVar2;
                *field36b4 = *reinterpret_cast<int *>(*field36b4 + 0xc);
            } while (iVar2 != 0);
        }
        if (setField) {
            *field36bc = iVar1;
        }
    }

    int arg1;
    if (*field36b0 != 0) {
        arg1 = *reinterpret_cast<int *>(*field36b4 + 8);
    } else {
        arg1 = 0;
    }

    Net *net = *reinterpret_cast<Net **>(self + 0x36a4);
    int result = net->join_session(
        reinterpret_cast<SessionStruct *>(arg1),
        reinterpret_cast<char *>(g_009bb5e8), 0);
    *reinterpret_cast<int *>(self + 0x37a4) = result;
    return result == 0;
}
