// ORIGINAL: 0x0063E540 sub_63e540 0x0063E540-0x0063E58F FILE
// size      79 bytes
// prototype 
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0063E540
// measured tier  MISMATCH
// divergence     10
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0063e540/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_63e540  at 0x0063E540  (79 bytes)
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

// SIGNATURE CHANGE (proposal): the default guess (`__stdcall`, no
// receiver) doesn't fit the disassembly - `ecx` is copied to `esi` and
// dereferenced as an object pointer throughout (`cmp dword ptr [esi],
// ebx`, `mov dword ptr [esi],edi`, ...), and `ret 4` pops exactly the one
// stack argument a __thiscall member with one `int` parameter would leave
// behind. Modeled here as a `__thiscall` method on a shim class, matching
// that evidence, since the address has no mangled name to pin a real
// class to. BODY: walks a singly-linked list at *this (each node's
// dword+8 freed, then the node itself), zeroes the list head as it goes,
// then resets fields +4/+8 and stores `a1` at +0xc. Returns `a1`, which is
// still live in eax after the final store - matches the disassembly, which
// never overwrites eax between that store and `ret`.
//
// CLOSEST reached: 0.9855 mnemonic similarity, MISMATCH. The one residual
// divergence (an extra `mov ecx,eax` register copy the original takes
// before reading `node->8`, which this backend's register allocator does
// not reproduce for any spelling of the two reads tried - shared local,
// re-dereferencing `*self` twice, `node[2]` indexing) is INSTRUCTION
// SCHEDULING / register allocation, not a source-form bug.
class Sub63e540Shim { public:
    int sub_63e540(int a1);
};

int Sub63e540Shim::sub_63e540(int a1) {
    char *self = reinterpret_cast<char *>(this);
    while (*reinterpret_cast<char **>(self) != 0) {
        int next = *reinterpret_cast<int *>(*reinterpret_cast<char **>(self) + 0x10);
        int field8 = *reinterpret_cast<int *>(*reinterpret_cast<char **>(self) + 8);
        if (field8 != 0) {
            free(reinterpret_cast<void *>(field8));
            *reinterpret_cast<int *>(*reinterpret_cast<char **>(self) + 8) = 0;
        }
        if (*reinterpret_cast<char **>(self) != 0) {
            free(*reinterpret_cast<void **>(self));
        }
        *reinterpret_cast<int *>(self) = next;
    }
    *reinterpret_cast<int *>(self + 4) = 0;
    *reinterpret_cast<int *>(self + 8) = 0;
    *reinterpret_cast<int *>(self + 0xc) = a1;
    return a1;
}
