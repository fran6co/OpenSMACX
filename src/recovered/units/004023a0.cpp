// ORIGINAL: 0x004023A0 FILE
// name      sub_4023a0
// size      365 bytes
// spans     0x004023A0-0x004024F8;0x00650580-0x00650595
// prototype 
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00402401 0x00402415 0x00402434 0x00402497 0x004024A9 0x004024C9
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004023A0
// measured tier  MISMATCH
// divergence     126
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004023a0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_4023a0  at 0x004023A0  (365 bytes)
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

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 0, 1
class VCall { public:
    virtual void slot000();  // <-- used
    virtual void slot001();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0065058b = (int *)0x0065058B;
static int *const g_006693f0 = (int *)0x006693F0;
static int *const g_006693f4 = (int *)0x006693F4;
static int *const g_00669418 = (int *)0x00669418;
static int *const g_0066941c = (int *)0x0066941C;
static int *const g_00671860 = (int *)0x00671860;

extern "C" __declspec(naked) int __cdecl sub_4023a0() {
    __asm {
        push ebp
        mov ebp, esp
        push -1
        push 0x65058b
        mov eax, dword ptr fs:[0]
        push eax
        mov dword ptr fs:[0], esp
        sub esp, 8
        push ebx
        mov ebx, ecx
        push esi
        push edi
        lea esi, [ebx - 0x24]
        mov dword ptr [ebp - 0x14], ebx
        mov dword ptr [esi], 0x66941c
        mov eax, dword ptr [ebx - 0x20]
        mov ecx, dword ptr [eax + 4]
        mov dword ptr [ecx + ebx - 0x20], 0x669418
        mov ecx, dword ptr [esi + 8]
        xor eax, eax
        cmp ecx, eax
        mov dword ptr [ebp - 4], eax
        je end1
        mov ecx, dword ptr [esi + 0x10]
        mov dword ptr [ebp - 0x10], eax
        cmp ecx, eax
        jle skip_loop1
loop1:
        mov eax, dword ptr [esi + 8]
        mov ecx, esi
        mov edx, dword ptr [eax + 0xc]
        mov dword ptr [esi + 0xc], edx
        mov edi, dword ptr [eax + 8]
        mov eax, dword ptr [esi]
        push edi
        call dword ptr [eax + 4]
        test edi, edi
        je skip1a
        mov ecx, dword ptr [edi]
        push 1
        mov edx, dword ptr [ecx + 4]
        mov eax, dword ptr [edx + edi]
        lea ecx, [edx + edi]
        call dword ptr [eax]
skip1a:
        mov ecx, dword ptr [esi + 8]
        mov dword ptr [ecx + 8], 0
        mov eax, dword ptr [esi + 8]
        test eax, eax
        je skip1b
        mov edx, dword ptr [eax]
        push 1
        mov ecx, dword ptr [edx + 4]
        mov edx, dword ptr [ecx + eax]
        add ecx, eax
        call dword ptr [edx]
skip1b:
        mov eax, dword ptr [esi + 0xc]
        mov ecx, dword ptr [esi + 0x10]
        mov dword ptr [esi + 8], eax
        mov eax, dword ptr [ebp - 0x10]
        inc eax
        cmp eax, ecx
        mov dword ptr [ebp - 0x10], eax
        jl loop1
        xor eax, eax
skip_loop1:
        mov dword ptr [esi + 8], eax
        mov dword ptr [esi + 0x14], eax
        mov dword ptr [esi + 0x10], eax
end1:
        mov dword ptr [esi + 0x14], eax
        lea esi, [ebx - 0x24]
        mov dword ptr [ebp - 4], 0xffffffff
        mov dword ptr [esi], 0x6693f4
        mov ecx, dword ptr [ebx - 0x20]
        mov edx, dword ptr [ecx + 4]
        mov dword ptr [edx + ebx - 0x20], 0x6693f0
        mov ecx, dword ptr [esi + 8]
        cmp ecx, eax
        je end2
        mov ecx, dword ptr [esi + 0x10]
        xor ebx, ebx
        cmp ecx, eax
        jle skip_loop2
loop2:
        mov eax, dword ptr [esi + 8]
        mov edx, dword ptr [esi]
        mov ecx, dword ptr [eax + 0xc]
        mov dword ptr [esi + 0xc], ecx
        mov edi, dword ptr [eax + 8]
        push edi
        mov ecx, esi
        call dword ptr [edx + 4]
        test edi, edi
        je skip2a
        mov eax, dword ptr [edi]
        push 1
        mov ecx, dword ptr [eax + 4]
        add ecx, edi
        mov edx, dword ptr [ecx]
        call dword ptr [edx]
skip2a:
        mov eax, dword ptr [esi + 8]
        mov dword ptr [eax + 8], 0
        mov eax, dword ptr [esi + 8]
        test eax, eax
        je skip2b
        mov ecx, dword ptr [eax]
        push 1
        mov edx, dword ptr [ecx + 4]
        lea ecx, [edx + eax]
        mov eax, dword ptr [edx + eax]
        call dword ptr [eax]
skip2b:
        mov eax, dword ptr [esi + 0x10]
        mov ecx, dword ptr [esi + 0xc]
        inc ebx
        mov dword ptr [esi + 8], ecx
        cmp ebx, eax
        jl loop2
        xor eax, eax
skip_loop2:
        mov dword ptr [esi + 8], eax
        mov dword ptr [esi + 0x14], eax
        mov dword ptr [esi + 0x10], eax
end2:
        mov ecx, dword ptr [ebp - 0xc]
        mov dword ptr [esi + 0x14], eax
        pop edi
        pop esi
        pop ebx
        mov dword ptr fs:[0], ecx
        mov esp, ebp
        pop ebp
        ret
        mov ecx, dword ptr [ebp - 0x14]
        sub ecx, 8
        _emit 0xE9
        _emit 0x55
        _emit 0x16
        _emit 0xDB
        _emit 0xFF
        _emit 0xB8
        _emit 0x60
        _emit 0x18
        _emit 0x67
        _emit 0x00
        _emit 0xE9
        _emit 0x41
        _emit 0x4A
        _emit 0xFF
        _emit 0xFF
    }
}
