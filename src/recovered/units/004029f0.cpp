// ORIGINAL: 0x004029F0 FILE
// name      ??1StringStruct@@QAE@H@Z
// size      102 bytes
// spans     0x004029F0-0x00402A56
// prototype void (__thiscall ??1StringStruct@@QAE@H@Z)(StringStruct* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00402970 0x00644EF2
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004029F0
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004029f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??1StringStruct@@QAE@H@Z  at 0x004029F0  (102 bytes)
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

class StringStruct;
struct StringStructEntry;

struct StringStructEntry {
    uint32_t abi_word;
    int id;
    int payload;
    StringStructEntry * next;
    StringStructEntry * previous;
    uint32_t secondary_abi_word;
    void * allocation_owner;
};

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" int __cdecl fn_00402970();
extern "C" int __cdecl free();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006693a0 = (int *)0x006693A0;
static int *const g_006693a4 = (int *)0x006693A4;
static int *const g_006693ac = (int *)0x006693AC;
static int *const g_009b3374 = (int *)0x009B3374;

class StringStruct { public:
    uint32_t primary_abi_word_;
    uint32_t virtual_base_abi_word_;
    StringStructEntry * head_;
    StringStructEntry * current_;
    int entry_count_;
    int current_position_;
    void * allocator_;
    uint32_t allocation_base_abi_word_;
    void * allocation_owner_;

    ~StringStruct();
};

__declspec(naked) StringStruct::~StringStruct() {
    __asm {
        push ebp
        mov ebp, esp
        push esi
        lea esi, [ecx-1Ch]
        push edi
        lea edi, [esi+0]
        mov dword ptr [edi], 06693A4h
        mov eax, [esi+4]
        mov ecx, [eax+4]
        mov dword ptr [ecx+esi+4], 06693A0h
        mov ecx, edi
        call fn_00402970
        mov cl, [ebp+8]
        mov dword ptr [edi+14h], 0
        mov eax, [esi+20h]
        mov dword ptr [esi+1Ch], 06693ACh
        test cl, 1
        mov dword ptr ds:[09B3374h], eax
        je asmlbl_common
        test esi, esi
        je asmlbl_common
        test eax, eax
        jne asmlbl_skipfree
        push esi
        call free
        add esp, 4
asmlbl_skipfree:
        mov dword ptr ds:[09B3374h], 0
asmlbl_common:
        mov eax, esi
        pop edi
        pop esi
        pop ebp
        ret 4
    }
}
