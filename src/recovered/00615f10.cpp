// ORIGINAL: 0x00615F10
// 0x00615F10  ?on_mouse_move@EditBox@@QAEXHH@Z  ->  ?on_mouse_move@EditBox@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// PROPOSAL: the catalogue mangles text_position as returning void
// (?text_position@EditBox@@QAEXH@Z, `X`), but the caller here stores
// `eax` into [esi+0xb44] immediately after `call text_position` with
// no reload in between - text_position's real (ABI) return value is
// an int, even though its declared return type is void. Reached
// through a member-function-pointer reinterpret_cast rather than by
// editing the given declaration.
typedef int (EditBox::*TextPositionIntFn)(int);

void EditBox::on_mouse_move(int a1, int a2) {
    if (*g_009b7abc == reinterpret_cast<int>(this) ||
        (*g_009b7ac0 == reinterpret_cast<int>(this) && *g_009b7abc != 0 && *g_009b7ac0 != 0)) {
        TextPositionIntFn fn = reinterpret_cast<TextPositionIntFn>(&EditBox::text_position);
        *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0xB44) = (this->*fn)(a1);
        reinterpret_cast<VCall *>(this)->slot062();
    }
}
