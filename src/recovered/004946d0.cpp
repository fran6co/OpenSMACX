// ORIGINAL: 0x004946D0
// 0x004946D0  ?UNK2@ProdPicker@@QAAXHPAUProdPicker@@@Z  ->  ?UNK2@ProdPicker@@QAAXHPAU1@@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl fn_004946d0(int a1, ProdPicker* a2) {
    if (a2 != 0) {
        char *object = reinterpret_cast<char *>(a2);
        ++*reinterpret_cast<int *>(object + 0xb64);
        a2->draw_item(*reinterpret_cast<int *>(object + 0xa774), 1);
    }
}
