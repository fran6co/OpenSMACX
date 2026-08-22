// ORIGINAL: 0x005E2330 ?put_pixel@Buffer@@QAEHHHH@Z 0x005E2330-0x005E2457
// TRIED: the two indirect calls are DirectDraw surface Lock/Unlock through the surface pointer stored at field_58_ - stdcall COM methods, not thiscall members of this class, so they are reached through a plain typed function pointer read from the surface's own vtable rather than a member-pointer shim; this matches the original's own indirect-call instruction shape. Ruled out a named lock-descriptor struct in favour of a raw byte buffer sized 0x6c with the two fields it actually reads (lpSurface at +0x10, pitch at +0x24) addressed by offset, since the real DDSURFACEDESC layout is not declared here. Landing the closest MISMATCH; divergence starts at instruction #5, `push` in the original for the DDraw lock call setup versus `xor` in the rebuilt.
// size      295 bytes
// prototype int (__thiscall ?put_pixel@Buffer@@QAEHHHH@Z)(Buffer* this, int, int, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005E23C3 0x005E243F

int Buffer::put_pixel(int a1, int a2, int a3) {
  int local_48;
  int result;
  unsigned char desc[0x6c];
  void *surface;

  int cur = (int)ppv_bits_;
  if (cur == 0 && field_58_ == 0) {
    return 7;
  }
  if (a1 < 0) {
    return 0;
  }
  if (a2 < 0) {
    return 0;
  }
  if ((int)width_ <= a1) {
    return 0;
  }
  if (-(int)height_ <= a2) {
    return 0;
  }

  surface = (void *)field_58_;
  if (surface == 0) {
    field_50_ = cur;
    if (cur == 0) {
      goto after_write;
    }
    field_6C_ = field_6C_ + 1;
    local_48 = cur;
  } else if (field_50_ == 0) {
    typedef int(__stdcall * LockFn)(void *, void *, void *, int, int);
    LockFn lock;
    *(int *)desc = 0x6c;
    lock = (LockFn)(*(void ***)surface)[0x64 / 4];
    result = lock(surface, 0, desc, 1, 0);
    if (result != 0) {
      goto after_write;
    }
    field_4A8_ = *(uint32_t *)(desc + 0x10);
    field_6C_ = field_6C_ + 1;
    field_50_ = *(int *)(desc + 0x24);
    local_48 = field_50_;
  } else {
    field_6C_ = field_6C_ + 1;
    local_48 = field_50_;
  }

  if (local_48 != 0) {
    unsigned char *p = (unsigned char *)(field_4A8_ * a2 + local_48 + a1);
    if (p != 0) {
      *p = (unsigned char)a3;
    }
  }

after_write:
  surface = (void *)field_58_;
  if (surface == 0) {
    int n = field_6C_ - 1;
    field_6C_ = n;
    if (n < 1) {
      field_50_ = 0;
      field_6C_ = 0;
      return 0;
    }
  } else {
    int n = field_6C_ - 1;
    field_6C_ = n;
    if (field_50_ != 0 && n < 1) {
      typedef int(__stdcall * UnlockFn)(void *, int);
      UnlockFn unlock = (UnlockFn)(*(void ***)surface)[0x80 / 4];
      unlock(surface, field_50_);
      field_50_ = 0;
      field_6C_ = 0;
    }
  }
  return 0;
}
