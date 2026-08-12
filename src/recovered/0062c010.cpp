// ORIGINAL: 0x0062C010 BYTE_EXACT
// name      ??1PushButton@@QAE@XZ
// size      21 bytes
// spans     0x0062C010-0x0062C025
// prototype void (__thiscall ??1PushButton@@QAE@XZ)(PushButton* this)
// callers   7   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// 0x0062C010  ??1PushButton@@QAE@XZ  ->  ??1PushButton@@QAE@XZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Re-install this class's two vtables - its own and the secondary base
         subobject's at +0x444 - then run the base destructor.
Original Offset: 0062C010
Return Value: n/a
Status: Complete
*/
// An empty `~PushButton() {}` compiles to a bare `ret`: the scaffolding declares
// PushButton with no base, so the vtable resets and the base call have to be
// written out. The trailing call becomes the original's tail `jmp`.
class BaseButton { public: ~BaseButton(); };

PushButton::~PushButton() {
    *reinterpret_cast<int **>(this) = g_00670c60;
    *reinterpret_cast<int **>(reinterpret_cast<char *>(this) + 0x444) =
        g_00670c58;
    reinterpret_cast<BaseButton *>(this)->~BaseButton();
}
