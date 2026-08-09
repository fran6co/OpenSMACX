// ORIGINAL: 0x006043D0
// 0x006043D0  ?set_def_file@BasePop@@QAAHPAD@Z  ->  ?set_def_file@BasePop@@QAAHPAD@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// The mangled name (?set_def_file@BasePop@@QAAHPAD@Z, QAA = public
// non-static __cdecl) makes `this` a hidden stack argument pushed
// BEFORE a1, at [esp+8] after the prologue's one push - which is
// exactly the slot this function reads. `a1` is unused: the string
// this validates/copies is `this` itself, reinterpreted as char*.
// `#pragma function` forces real calls to strlen/strcat instead of
// their /O2 inline expansion (repne scasb / rep movsd) while keeping
// /O2's `mov`+`add esp` epilogue style, which is what the target uses.
#pragma function(strlen, strcat)

int __cdecl BasePop::set_def_file(char * a1) {
    char *self_str = reinterpret_cast<char *>(this);
    if (self_str != 0 && strlen(self_str) <= 0xC) {
        *reinterpret_cast<char *>(g_009b8aa8) = 0;
        strcat(reinterpret_cast<char *>(g_009b8aa8), self_str);
        return 0;
    }
    return 3;
}
