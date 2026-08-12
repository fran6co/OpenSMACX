// ORIGINAL: 0x00625310 BYTE_EXACT
// name      ??1ImageButton@@QAE@XZ
// size      21 bytes
// spans     0x00625310-0x00625325
// prototype void (__thiscall ??1ImageButton@@QAE@XZ)(ImageButton* this)
// callers   1   call targets   0
// 0x00625310  ??1ImageButton@@QAE@XZ  ->  ??1ImageButton@@QAE@XZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Re-install this class's two vtables - its own and the secondary base
         subobject's at +0x444 - then run the base destructor.
Original Offset: 00625310
Return Value: n/a
Status: Complete
*/
// An empty `~ImageButton() {}` compiles to a bare `ret`: the scaffolding declares
// ImageButton with no base, so the vtable resets and the base call have to be
// written out. The trailing call becomes the original's tail `jmp`.
class BaseButton { public: ~BaseButton(); };

ImageButton::~ImageButton() {
    *reinterpret_cast<int **>(this) = g_00670a94;
    *reinterpret_cast<int **>(reinterpret_cast<char *>(this) + 0x444) =
        g_00670a8c;
    reinterpret_cast<BaseButton *>(this)->~BaseButton();
}
