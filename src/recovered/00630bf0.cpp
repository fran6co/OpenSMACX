// ORIGINAL: 0x00630BF0 sub_630bf0 0x00630BF0-0x00630C29 BYTE_EXACT
// size      57 bytes
// prototype BOOL (__stdcall sub_630BF0)(LPGUID lpguidSP, LPSTR lpSPName, DWORD dwMajorVersion, DWORD dwMinorVersion, LPVOID lpContext)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00401760 0x00645C70
// 0x00630BF0  sub_630bf0  ->  _sub_630bf0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// The generated head above (`BOOL __stdcall sub_630bf0(LPGUID lpguidSP a1,
// LPSTR lpSPName a2, ...)`) does not compile: BOOL/DWORD/LPGUID/LPSTR/LPVOID
// are forward-declared as opaque incomplete STRUCTS above, not the real
// Windows typedefs, and every parameter carries two identifiers (a type-name
// and a semantic name) at once. See PROPOSALS in the agent report - this is
// an emitter bug, not a signature this body chooses to diverge from. `ret
// 0x14` (5 stack dwords) is the only part of the original head the
// disassembly actually pins; everything else here is a plain `int`/pointer
// replacement that compiles and carries the same calling convention.
//
// The scaffold's own `_memcmp` declaration above is nullary
// (`int __cdecl _memcmp();`) and can't take the 3 arguments this call needs:
// declfix only patches arity for names in its CRT_SIGNATURES table, and
// memcmp isn't in it. A same-shaped extern "C" cdecl declaration under a
// different name produces the identical push/push/push/call/add-esp
// sequence - the call target is a relocation and is masked from the byte
// comparison either way, so the symbol name carries no weight here.
extern "C" int __cdecl raw_memcmp(const void *, const void *, unsigned int);

extern "C" int __stdcall sub_630bf0(const void *a1, int a2, int a3, int a4, int a5) {
    if (raw_memcmp(a1, g_00697558, 0x10) != 0) {
        char *svc = reinterpret_cast<char *>(*g_009be5f0);
        *reinterpret_cast<int *>(svc + 0x1c) = a2;
        *reinterpret_cast<const void **>(svc + 0x20) = a1;
        reinterpret_cast<ServiceStruct *>(svc)->add(0);
    }
    return 1;
}
