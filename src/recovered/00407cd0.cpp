// ORIGINAL: 0x00407CD0 ?on_button_clicked@AlphaSave@@QAEXH@Z 0x00407CD0-0x00407CE6 BYTE_EXACT
// size      22 bytes
// prototype void (__thiscall ?on_button_clicked@AlphaSave@@QAEXH@Z)(AlphaSave* this, int)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00614B90
// 0x00407CD0  ?on_button_clicked@AlphaSave@@QAEXH@Z  ->  ?on_button_clicked@AlphaSave@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void AlphaSave::on_button_clicked(int a1) {
    char *self = reinterpret_cast<char *>(this);
    reinterpret_cast<FileWin *>(self + 0xa14)->on_button_clicked(a1);
}
