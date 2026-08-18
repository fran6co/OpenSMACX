// ORIGINAL: 0x005EAF8F ?save@Sprite@@QAEHH@Z 0x005EAF8F-0x005EB0B2;0x00662C30-0x00662C42
// RULED-OUT: a stack-local `Filemap fm;` (RAII) reproduces the SEH try/finally scaffold (0x662C30/0x662C38, __CxxFrameHandler) automatically - no hand-written exception frame needed. Landed at MISMATCH #44, well past the whole prologue, both early-return cleanup paths and the `create` call.
// size      309 bytes
// prototype int (__thiscall ?save@Sprite@@QAEHH@Z)(Sprite* this, int)
// callers   0   call targets   5
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00628380 0x006283E0 0x00628650 0x006287C0 0x00645930

int Sprite::save(int a1) {
    Filemap fm;
    if (pcBits_ == 0) {
        return 7;
    }
    char *buf = reinterpret_cast<char *>(
        fm.create(reinterpret_cast<char *>(a1), iSpriteWidth_ * iSpriteHeight_ + 0x2c, 1));
    if (buf == 0) {
        return 4;
    }
    *reinterpret_cast<int *>(buf) = iSpriteWidth_;
    buf += 4;
    *reinterpret_cast<int *>(buf) = iSpriteHeight_;
    buf += 4;
    *reinterpret_cast<int *>(buf) = iWidth_;
    buf += 4;
    *reinterpret_cast<int *>(buf) = iHeight_;
    buf += 4;
    *reinterpret_cast<int *>(buf) = iLeftOffset_;
    buf += 4;
    *reinterpret_cast<int *>(buf) = iTopOffset_;
    buf += 4;
    *buf = cTransparentIndex_;
    buf += 1;
    *reinterpret_cast<int *>(buf) = iSpriteWidth2_;
    buf += 4;
    memcpy(buf, reinterpret_cast<void *>(pcBits_), iSpriteHeight_ * iSpriteWidth2_);
    fm.close();
    return 0;
}
