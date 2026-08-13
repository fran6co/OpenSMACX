// ORIGINAL: 0x005F8BE0 FILE
// name      ?add_separator@PullDown@@QAEHH@Z
// size      161 bytes
// spans     0x005F8BE0-0x005F8C81
// prototype int (__thiscall ?add_separator@PullDown@@QAEHH@Z)(PullDown* this, int)
// callers   10   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005F8BE0
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005f8be0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?add_separator@PullDown@@QAEHH@Z  at 0x005F8BE0  (161 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.

typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;

class PullDown { public:
    int add_separator(int);
};

int PullDown::add_separator(int a1) {
    char *self = reinterpret_cast<char *>(this);
    int new_count = ++(*reinterpret_cast<int *>(self + 0xf20));
    if (new_count > 0x40) {
        *reinterpret_cast<int *>(self + 0xf20) = new_count - 1;
        return 1;
    }
    if (a1 == -1) {
        return 3;
    }
    for (int i = 0; i < 0x40; ++i) {
        if (*reinterpret_cast<int *>(self + 0xa20 + i * 0x14) == -1) {
            int idx = new_count * 5 - 5;
            char *base = self + idx * 4;
            if (*reinterpret_cast<int *>(base + 0xa18) != 0) {
                return 0xc;
            }
            *reinterpret_cast<int *>(base + 0xa20) = a1;
            *reinterpret_cast<int *>(base + 0xa24) = 5;
            ++(*reinterpret_cast<int *>(self + 0xf2c));
            *reinterpret_cast<unsigned char *>(self + 0xf34) = 1;
            return 0;
        }
    }
    return 1;
}
