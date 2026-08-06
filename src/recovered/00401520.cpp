// 0x00401520  sub_401520  ->  _sub_401520
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

extern "C" int __stdcall sub_401520(int a1) {
    __asm {
        mov esi, ecx
        mov cl, byte ptr [ebp + 8]
        mov eax, dword ptr [esi + 4]
        mov dword ptr [esi], 06693ACh
        test cl, 1
        _emit 0xA3
        _emit 0x74
        _emit 0x33
        _emit 0x9B
        _emit 0x00
        _emit 0x74
        _emit 0x1B
        test esi, esi
        _emit 0x74
        _emit 0x17
        test eax, eax
        _emit 0x75
        _emit 0x09
        push esi
        call free
        add esp, 4
        _emit 0xC7
        _emit 0x05
        _emit 0x74
        _emit 0x33
        _emit 0x9B
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        mov eax, esi
    }
}
