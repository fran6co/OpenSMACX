// ORIGINAL: 0x004E31E0 ?setup@AlphaNet@@QAEHH@Z 0x004E31E0-0x004E32BA FILE
// TRIED: outer `if (a1 == 0) { big-init-block } else { field_D8_ |= 1 }` (ghidra's polarity, not the naive `!= 0`) plus a duplicated success-tail (field_48_/4C_ writes + return 0) inside the `if (rc >= 0)` arm rather than one shared tail reached via a unified `if (r2 == 0) return 1;` after the join/create if-else - 0.73 -> 0.98 mnemonic similarity. AlphaNet reaches Net::close()/ init() through reinterpret_cast<Net*>(this) (a zero-offset base).
// TRIED: one remaining `test/je` original has on the join() path only (create() path just has `test/jne`) that a single shared `if (r2 != 0)` after the join/create if-else does not reproduce; not chased further.
// working copy - scaffold materialised by --work
// size      218 bytes
// prototype int (__thiscall ?setup@AlphaNet@@QAEHH@Z)(AlphaNet* this, int)
// callers   1   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E26E0 0x004E2A30 0x004E2E50 0x005BF420 0x0062D8E0 0x0062E010

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?setup@AlphaNet@@QAEHH@Z  at 0x004E31E0  (218 bytes)
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

class AlphaNet;
class Net;
struct _GUID;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Net { public:
    int init(_GUID *, int, int);
    void close();
};

int __cdecl X_pop(char *, const char *, int, int (__cdecl *)());

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00689218 = (int *)0x00689218;
static int *const g_00689240 = (int *)0x00689240;
static int *const g_0068925c = (int *)0x0068925C;
static int *const g_0093a95c = (int *)0x0093A95C;

class AlphaNet { public:
    uint32_t field_0_;
    uint32_t field_4_;
    uint32_t field_8_;
    uint32_t field_C_;
    uint32_t field_10_;
    uint32_t field_14_;
    uint32_t field_18_;
    uint32_t field_1C_;
    uint32_t field_20_;
    uint32_t field_24_;
    uint32_t field_28_;
    uint8_t field_2C_[0x18];
    uint32_t field_44_;
    uint32_t field_48_;
    uint32_t field_4C_;
    uint32_t field_50_;
    uint32_t field_54_;
    uint32_t field_58_;
    uint8_t field_5C_[0x54];
    uint32_t field_B0_;
    uint32_t field_B4_;
    uint32_t field_B8_;
    uint32_t field_BC_;
    uint32_t field_C0_;
    uint32_t field_C4_;
    uint32_t field_C8_;
    uint32_t field_CC_;
    uint32_t field_D0_;
    uint32_t field_D4_;
    uint32_t field_D8_;
    uint32_t field_DC_;
    uint32_t field_E0_;
    uint32_t field_E4_;
    uint8_t field_E8_[0x6C];
    uint32_t field_154_;
    uint32_t field_158_;
    uint32_t field_15C_;
    uint8_t field_160_[0x4];
    uint32_t field_164_;
    uint8_t field_168_;
    uint8_t field_169_[0x56B];
    uint32_t field_6D4_;
    uint8_t field_6D8_[0x4];
    uint32_t field_6DC_;
    uint32_t field_6E0_;
    uint32_t field_6E4_;
    uint32_t field_6E8_;
    uint8_t field_6EC_[0x10];
    uint32_t field_6FC_;
    uint32_t field_700_;
    uint32_t field_704_;
    uint32_t field_708_;
    uint32_t field_70C_;
    uint32_t field_710_;
    uint32_t field_714_;
    uint32_t field_718_;
    uint32_t field_71C_;
    uint32_t field_720_;
    uint8_t field_724_[0x8];
    uint32_t field_72C_;
    uint32_t field_730_;
    uint32_t field_734_;
    uint32_t field_738_;
    uint32_t field_73C_;
    uint32_t field_740_;
    uint32_t field_744_;
    uint8_t field_748_[0x8];
    uint32_t field_750_;
    uint32_t field_754_;
    uint32_t field_758_;
    uint32_t field_75C_;
    uint32_t field_760_;
    uint32_t field_764_;
    uint8_t field_768_[0x18];
    uint8_t data_[0xD20];

    int pick_service(int *);
    int create();
    int join();
    int setup(int);
};
int AlphaNet::setup(int a1) {
    char *self = reinterpret_cast<char *>(this);

    int *p = reinterpret_cast<int *>(self + 0x78c);
    int count = 8;
    do {
        *p = 0;
        p += 0x67;
        count--;
    } while (count != 0);

    *reinterpret_cast<int *>(self + 0x768) = 0;

    reinterpret_cast<Net *>(this)->close();
    *g_0093a95c = 0;

    if (a1 == 0) {
        reinterpret_cast<Net *>(this)->init(reinterpret_cast<_GUID *>(g_00689218), 3, 9);
        int result = pick_service(g_0093a95c);
        if (result != 0) {
            return result;
        }
        if (*g_0093a95c != 0) {
            reinterpret_cast<Net *>(this)->close();
            return 0;
        }
        int rc = X_pop(reinterpret_cast<char *>(g_0068925c), reinterpret_cast<const char *>(g_00689240), 0x50, 0);
        if (rc >= 0) {
            int r2;
            if (rc == 1) {
                r2 = join();
            } else {
                r2 = create();
            }
            if (r2 != 0) {
                *reinterpret_cast<int *>(self + 0x48) = 0xdc;
                *reinterpret_cast<int *>(self + 0x4c) = 0x100dc;
                return 0;
            }
        }
        return 1;
    } else {
        *reinterpret_cast<unsigned int *>(self + 0xd8) |= 1;
    }

    *reinterpret_cast<int *>(self + 0x48) = 0xdc;
    *reinterpret_cast<int *>(self + 0x4c) = 0x100dc;
    return 0;
}
