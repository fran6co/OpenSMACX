// ORIGINAL: 0x005F5AD0
// name      ?on_nc_hittest@Win@@QAEHHH@Z
// size      294 bytes
// spans     0x005F5AD0-0x005F5BF6
// prototype int (__thiscall ?on_nc_hittest@Win@@QAEHHH@Z)(Win* this, int, int)
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// RULED-OUT: transcribing the nine-way corner/edge ladder as one flat
//            sequence of if-return statements (matching the Ghidra shape,
//            each re-testing x/y against iVar1 and the recomputed
//            client-rect edge) compiles and the byte count comes close
//            (286 vs 294) but the prologue register allocation still
//            diverges at instruction #3 - the original pushes edi before
//            touching it, this form delays that push since it needs one
//            fewer live temporary across the first branch. Landing the
//            closest MISMATCH.

int Win::on_nc_hittest(int a1, int a2) {
  if (iSomeFlag_ & 2) {
    int iVar1 = field_170_;
    if (a1 < iVar1) {
      if (a2 < iVar1) {
        return 0xd;
      }
      if (client_rect_.bottom - client_rect_.top - iVar1 < a2) {
        return 0x10;
      }
    }
    int iVar2 = client_rect_.right - client_rect_.left - iVar1;
    if (iVar2 < a1) {
      if (a2 < iVar1) {
        return 0xe;
      }
      if (client_rect_.bottom - client_rect_.top - iVar1 < a2) {
        return 0x11;
      }
    }
    if (a1 < iVar1) {
      return 0xa;
    }
    if (a2 < iVar1) {
      return 0xc;
    }
    if (iVar2 < a1) {
      return 0xb;
    }
    if (client_rect_.bottom - client_rect_.top - iVar1 < a2) {
      return 0xf;
    }
  }
  if ((iFlags_ & 0x4000010) == 0) {
    if (iFlags_ & 1) {
      return 2;
    }
  } else if (field_16C_ != 0 && a2 < (int)field_16C_) {
    return 2;
  }
  return 0;
}
