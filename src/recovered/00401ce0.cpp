// 0x00401CE0  SessionStruct::close  ->  _fn_00401ce0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

class VD { public:
    virtual void dtor(int);
    virtual void notify(void *);
};

struct SNode2 {
    int *vb;
    int f4;
    void *obj;
    SNode2 *next;
};

struct Sess2 {
    int *vfptr;
    int *vbptr;
    SNode2 *head;
    SNode2 *cur;
    int count;
    int f14;
};

extern "C" int __declspec(naked) __cdecl fn_00401ce0()
{
    __asm {
        push    ebp
        mov     ebp, esp
        push    ecx
        push    ebx
        push    esi
        lea     esi, [ecx-1ch]
        xor     ebx, ebx
        mov     dword ptr [esi], 669408h
        mov     eax, [ecx-18h]
        mov     edx, [eax+4]
        mov     dword ptr [edx+ecx-18h], 669404h
        mov     eax, [esi+8]
        cmp     eax, ebx
        je      L90
        mov     eax, [esi+10h]
        mov     [ebp-4], ebx
        cmp     eax, ebx
        jle     L87
        push    edi
L31:
        mov     eax, [esi+8]
        mov     edx, [esi]
        mov     ecx, [eax+0ch]
        mov     [esi+0ch], ecx
        mov     edi, [eax+8]
        push    edi
        mov     ecx, esi
        call    dword ptr [edx+4]
        cmp     edi, ebx
        je      L56
        mov     eax, [edi]
        push    1
        mov     ecx, [eax+4]
        add     ecx, edi
        mov     edx, [ecx]
        call    dword ptr [edx]
L56:
        mov     eax, [esi+8]
        mov     [eax+8], ebx
        mov     eax, [esi+8]
        cmp     eax, ebx
        je      L72
        mov     ecx, [eax]
        push    1
        mov     edx, [ecx+4]
        lea     ecx, [edx+eax]
        mov     eax, [edx+eax]
        call    dword ptr [eax]
L72:
        mov     ecx, [esi+0ch]
        mov     eax, [ebp-4]
        mov     [esi+8], ecx
        mov     ecx, [esi+10h]
        inc     eax
        cmp     eax, ecx
        mov     [ebp-4], eax
        jl      L31
        pop     edi
L87:
        mov     [esi+8], ebx
        mov     [esi+14h], ebx
        mov     [esi+10h], ebx
L90:
        mov     [esi+14h], ebx
        pop     esi
        pop     ebx
        mov     esp, ebp
        pop     ebp
        ret
    }
}
