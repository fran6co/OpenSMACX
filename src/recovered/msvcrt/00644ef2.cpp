// ORIGINAL: 0x00644EF2 BYTE_EXACT
// name      _free
// size      72 bytes
// spans     0x00644EF2-0x00644F3A
// prototype 
// callers   170   call targets   4
// kind      library
// flags     lib;hidden;sp_ready;purged_ok
// calls     0x00647D04 0x00647D65 0x00647DF4 0x00647E1F
// indirect  0x00644F32
// 0x00644EF2  _free  ->  _free
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// The emitted declarations for these CRT internals above are nullary (no
// prototype survives for unmangled C-linkage names), so fresh, correctly
// typed shadow declarations are used instead (rule 4: shadow declarations
// above the definition are kept; the identifiers below do not collide
// with the emitted nullary ones).
extern "C" void *__cdecl sbh_find_block_impl(void *);
extern "C" void __cdecl sbh_free_block_impl(void *, void *);
extern "C" void __cdecl lock_impl(int);
extern "C" void __cdecl unlock_impl(int);

extern "C" void __cdecl free(void *pBlock) {
    if (pBlock != 0) {
        lock_impl(9);
        void *header = sbh_find_block_impl(pBlock);
        if (header != 0) {
            sbh_free_block_impl(header, pBlock);
            unlock_impl(9);
        } else {
            unlock_impl(9);
            typedef void *(__stdcall * HeapFreeFn)(void *, unsigned int, void *);
            reinterpret_cast<HeapFreeFn>(*g_00669198)(
                *reinterpret_cast<void **>(g_009c11c4), 0, pBlock);
        }
    }
}
