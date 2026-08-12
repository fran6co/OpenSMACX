// ORIGINAL: 0x005FFD80
// name      ??0MCIVideo@@QAE@XZ
// size      33 bytes
// spans     0x005FFD80-0x005FFDA1
// prototype void (__thiscall ??0MCIVideo@@QAE@XZ)(MCIVideo* this)
// callers   2   call targets   1
// 0x005FFD80  ??0MCIVideo@@QAE@XZ  ->  ??0MCIVideo@@QAE@XZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

MCIVideo::MCIVideo() {
    Palette *palette = reinterpret_cast<Palette *>(reinterpret_cast<char *>(this) + 0x10);
    palette->Palette::Palette();
    short *self16 = reinterpret_cast<short *>(this);
    self16[2] = 0;
    int *self32 = reinterpret_cast<int *>(this);
    self32[2] = 0;
    self32[3] = 0;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x468) = 0;
}
