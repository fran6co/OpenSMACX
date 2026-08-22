// ORIGINAL: 0x005D2B20 sub_5d2b20 0x005D2B20-0x005D2C0A FILE
// TRIED: BYTE_EXACT is likely structurally unreachable here without `__asm` (banned). The disassembly reads `esi` as an 8-int input array from instruction #1 (`mov ebx,[esi+0x14]`) and NEVER writes esi before that first read anywhere in the function - Ghidra names it `unaff_ESI` for exactly this reason. That means the real calling convention hands this function its input array already sitting in esi, which none of __cdecl/__stdcall/__fastcall/__thiscall can express - every standard convention either uses the stack or ecx/edx for the first argument, never esi with zero setup. This is plausible as a compiler-chosen convention specific to this internal, 2-caller helper (proven safe only in the context of its real callers, which this recovery unit does not see), but nothing in valid VC6 C++ can request it. Landed body takes the input array as a normal second parameter instead (MISMATCH from the very first instruction, since the prologue differs immediately) so the arithmetic shape is on record: an 8-point butterfly (pairwise sum/difference of src[0],src[2],src[4], src[6] and separately src[1],src[3],src[5],src[7]) combining one `__int64` fixed-point multiply-by-g_009c9594 (the `imul dword ptr [addr]` high-dword-times-2 pattern) with float arithmetic against g_009c9598/g_009c959c/g_009c95a0, each float result added to g_009c95a4 and reduced to an int via `fstp qword` followed by reading only the low dword - this looks like the classic "magic-constant" double-to-int rounding idiom (add a bias, read the mantissa's low bits) but a plain `(int)(double)expr` cast was NOT enough to reproduce it; the exact bias/rounding trick was not reverse-engineered.
// size      234 bytes
// prototype 
// callers   2   call targets   0
// kind      
// flags     
// calls     (none)
// working copy - scaffold materialised by --work

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5d2b20  at 0x005D2B20  (234 bytes)
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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009c9594 = (int *)0x009C9594;
static int *const g_009c9598 = (int *)0x009C9598;
static int *const g_009c959c = (int *)0x009C959C;
static int *const g_009c95a0 = (int *)0x009C95A0;
static int *const g_009c95a4 = (int *)0x009C95A4;
extern "C" void __cdecl sub_5d2b20(int *dst, int *src) {
    int iVar9 = src[3] + src[5];
    int iVar8 = src[7] + src[1];
    float fVar2 = (float)(src[5] - src[3]);
    float fVar3 = (float)(src[1] - src[7]);
    float fVar1 = (fVar3 + fVar2) * *(float *)g_009c95a0;
    int iVar6 = (int)(((__int64)(iVar8 - iVar9) * (__int64)*g_009c9594) >> 32) * 2;
    int iStack8 = (int)(double)(fVar2 * *(float *)g_009c9598 + fVar1 + *(float *)g_009c95a4);
    int iStack10 = (int)(double)((fVar3 * *(float *)g_009c959c - fVar1) + *(float *)g_009c95a4);
    int iVar4 = iStack8 + iVar6;
    iVar6 = iVar6 + iStack10;
    iStack10 = iStack10 + iVar9 + iVar8;
    iVar9 = src[4] + src[0];
    int iVar5 = src[0] - src[4];
    int iVar7 = (int)(((__int64)(src[2] - src[6]) * (__int64)*g_009c9594) >> 32);
    iVar8 = iVar7 * 2;
    int iVar10 = src[6] + src[2] + iVar8;
    iVar8 = iVar8 + iVar5;
    iVar5 = iVar5 + iVar7 * -2;
    iVar7 = iVar10 + iVar9;
    iVar9 = iVar9 - iVar10;
    dst[2] = iVar5 + iVar4;
    dst[5] = iVar5 - iVar4;
    dst[3] = iVar9 + iStack8;
    dst[4] = iVar9 - iStack8;
    dst[0] = iVar7 + iStack10;
    dst[1] = iVar6 + iVar8;
    dst[6] = iVar8 - iVar6;
    dst[7] = iVar7 - iStack10;
}
