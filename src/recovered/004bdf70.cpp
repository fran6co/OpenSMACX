// ORIGINAL: 0x004BDF70 BYTE_EXACT
// 0x004BDF70  ?veh_draw@TutWin@@QAAXPAUSprite@@PAUGraphicWin@@HHH@Z  ->  ?veh_draw@TutWin@@QAAXPAUSprite@@PAUGraphicWin@@HHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

struct GraphicWin;
struct Sprite;
struct vehID;
struct Buffer;

// ---- callees, declared and never defined (a definition would be inlined) ----
int __cdecl veh_draw(Buffer*, int, int, int, int, int, int);

void __cdecl fn_004bdf70(Sprite* a1, GraphicWin* a2,
                         int a3, int a4, int a5) {
    Buffer *buffer = a2 != 0
        ? reinterpret_cast<Buffer *>(reinterpret_cast<char *>(a2) + 0x444)
        : 0;
    veh_draw(buffer, a3, a4, a5, 0, 0, 0);
}
