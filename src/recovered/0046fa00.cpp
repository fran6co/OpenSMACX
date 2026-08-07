// 0x0046FA00  ?on_resize@MapWin@@QAEXHH@Z  ->  ?on_resize@MapWin@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void MapWin::on_resize(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    if ((*reinterpret_cast<unsigned int *>(self - 0x3cfc) & 0x80000000) == 0) {
        reinterpret_cast<MapWin *>(self - 0x21a6c)->do_image_buttons();
    }
}
