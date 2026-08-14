// ORIGINAL: 0x00619710
// name      ?extract@Texture@@QAEHPAUBuffer@@HHHHPAUTexHeap@@@Z
// size      462 bytes
// spans     0x00619710-0x006198DE
// prototype int (__thiscall ?extract@Texture@@QAEHPAUBuffer@@HHHHPAUTexHeap@@@Z)(Texture* this, Buffer*, int xCoord, int yCoord, int height, int width, #120*)
// callers   3   call targets   6
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x005E33F3 0x005E34A3 0x00635370 0x00644EF2 0x00645930
// RULED-OUT: the extra locals this body needs (widthMinus1, heightMinus1,
//            stride, etc.) push the prologue to a `sub esp` before the
//            register pushes, so divergence starts at instruction #1
//            (push vs mov). Landing the closest control-flow-faithful form.

int Texture::extract(Buffer * a1, int a2, int a3, int a4, int a5, TexHeap * a6) {
  if (a1 == 0) {
    return 0x10;
  }

  if (pixels_ != 0 && borrowed_ == 0) {
    if (pixels_) {
      free(pixels_);
    }
    pixels_ = 0;
  }
  iWidth_ = 0;
  iHeight_ = 0;
  borrowed_ = 0;

  if (a2 + a4 > (int)a1->width_) {
    return 3;
  }
  if (a3 + a5 > -(int)a1->height_) {
    return 3;
  }

  iHeight_ = a5;
  iWidth_ = a4;

  int srcData = a1->get_data(a2, a3);
  if (srcData == 0) {
    return 7;
  }

  int stride = a1->field_4A8_;
  pixels_ = 0;
  if (a6 != 0) {
    pixels_ = reinterpret_cast<void *>(a6->get_tex_mem(iHeight_ << 8));
    if (pixels_ != 0) {
      borrowed_ = 1;
    }
  }
  if (pixels_ == 0) {
    pixels_ = mem_get(iHeight_ * iWidth_);
    if (pixels_ == 0) {
      iWidth_ = 0;
      iHeight_ = 0;
      borrowed_ = 0;
      a1->free_data(1);
      return 4;
    }
    borrowed_ = 0;
  }

  char *dest = reinterpret_cast<char *>(pixels_);
  int src = srcData;
  if (a5 != 0) {
    int count = a5;
    do {
      memcpy(dest, reinterpret_cast<void *>(src), iWidth_);
      if (borrowed_ == 0) {
        dest += iWidth_;
      } else {
        dest += 0x100;
      }
      src += stride;
      --count;
    } while (count != 0);
  }

  a1->free_data(1);

  int *self = reinterpret_cast<int *>(this);
  self[3] = 0;
  self[4] = 0;
  int widthMinus1 = iWidth_ - 1;
  int heightMinus1 = iHeight_ - 1;
  self[5] = widthMinus1;
  int widthHalfMinus1 = (int)iWidth_ / 2 - 1;
  self[7] = widthMinus1;
  self[0xd] = widthHalfMinus1;
  self[0xf] = widthMinus1;
  self[0x11] = widthMinus1;
  self[0x13] = widthMinus1;
  int heightHalfMinus1 = (int)iHeight_ / 2 - 1;
  self[8] = heightMinus1;
  self[0xa] = heightMinus1;
  self[0x14] = heightMinus1;
  self[0x16] = heightMinus1;
  self[0x18] = heightMinus1;
  self[6] = 0;
  self[9] = 0;
  self[0xb] = 0;
  self[0xc] = 0;
  self[0xe] = 0;
  self[0x10] = 0;
  self[0x12] = heightHalfMinus1;
  self[0x15] = widthHalfMinus1;
  self[0x17] = 0;
  self[0x19] = 0;
  self[0x1a] = heightHalfMinus1;
  return 0;
}
