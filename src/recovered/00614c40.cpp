// 0x00614C40  ?on_double_clicked@FileWin@@QAEXH@Z  ->  ?on_double_clicked@FileWin@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void FileWin::on_double_clicked(int a1) {
    UNK4();
}
