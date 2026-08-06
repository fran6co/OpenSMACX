// 0x00401CE0  SessionStruct::close  ->  _fn_00401ce0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

extern "C" __declspec(naked) int __cdecl fn_00401ce0() {
    __asm {
        push ebp
        mov ebp, esp
        push ecx
        push ebx
        push esi
        lea esi, [ecx - 0x1c]
        xor ebx, ebx
        mov dword ptr [esi], 0x669408
        mov eax, dword ptr [ecx - 0x18]
        mov edx, dword ptr [eax + 4]
        mov dword ptr [edx + ecx - 0x18], 0x669404
        mov eax, dword ptr [esi + 8]
        cmp eax, ebx
        je done
        mov eax, dword ptr [esi + 0x10]
        mov dword ptr [ebp - 4], ebx
        cmp eax, ebx
        jle after_loop
        push edi
    loop_top:
        mov eax, dword ptr [esi + 8]
        mov edx, dword ptr [esi]
        mov ecx, dword ptr [eax + 0xc]
        mov dword ptr [esi + 0xc], ecx
        mov edi, dword ptr [eax + 8]
        push edi
        mov ecx, esi
        call dword ptr [edx + 4]
        cmp edi, ebx
        je skip_release1
        mov eax, dword ptr [edi]
        push 1
        mov ecx, dword ptr [eax + 4]
        add ecx, edi
        mov edx, dword ptr [ecx]
        call dword ptr [edx]
    skip_release1:
        mov eax, dword ptr [esi + 8]
        mov dword ptr [eax + 8], ebx
        mov eax, dword ptr [esi + 8]
        cmp eax, ebx
        je skip_release2
        mov ecx, dword ptr [eax]
        push 1
        mov edx, dword ptr [ecx + 4]
        lea ecx, [edx + eax]
        mov eax, dword ptr [edx + eax]
        call dword ptr [eax]
    skip_release2:
        mov ecx, dword ptr [esi + 0xc]
        mov eax, dword ptr [ebp - 4]
        mov dword ptr [esi + 8], ecx
        mov ecx, dword ptr [esi + 0x10]
        inc eax
        cmp eax, ecx
        mov dword ptr [ebp - 4], eax
        jl loop_top
        pop edi
    after_loop:
        mov dword ptr [esi + 8], ebx
        mov dword ptr [esi + 0x14], ebx
        mov dword ptr [esi + 0x10], ebx
    done:
        mov dword ptr [esi + 0x14], ebx
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}
