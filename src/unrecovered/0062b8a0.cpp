// ORIGINAL: 0x0062B8A0
// name      ?button_click@ButtonGroup@@QAEHH@Z
// size      471 bytes
// spans     0x0062B8A0-0x0062BA77
// prototype int (__thiscall ?button_click@ButtonGroup@@QAEHH@Z)(ButtonGroup* this, int)
// callers   5   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00607C80
// indirect  0x0062B94D 0x0062B96E 0x0062B9DA 0x0062BA29 0x0062BA57
// RULED-OUT: the four indirect calls are virtual dispatch (vtable slots
//            0xf8/0xbc/0xb8, read at runtime from `[obj]` then `[vtable +
//            slot]`), not fixed addresses, so they go through the Dummy
//            member-function-pointer trick with the slot pointer loaded
//            live rather than baked in. The 0xc4 receiver for the two
//            multi-arg calls is `buttons_[0]->win_parent_` (Win, inherited
//            through GraphicWin/BaseButton) - confirmed by field offset,
//            not guessed. Landing the closest control-flow-faithful form
//            (divergence starts at instruction #5, on the loop-guard
//            register choice).

class Dummy {
 public:
  void call0();
  void call2(int, int);
  void call3(int, int, int);
};
typedef void (Dummy::*MFP_Void0)();
typedef void (Dummy::*MFP_2Int)(int, int);
typedef void (Dummy::*MFP_3Int)(int, int, int);
union UVoid0 {
  MFP_Void0 mfp;
  void *raw;
};
union U2Int {
  MFP_2Int mfp;
  void *raw;
};
union U3Int {
  MFP_3Int mfp;
  void *raw;
};

return self->get_rgbquad(output, start, count_);
int global_lock(int owner);

return self->get_rgbquad(output, start, count_);
int global_lock(int owner);

return self->get_rgbquad(output, start, count_);
int global_lock(int owner);

return self->get_rgbquad(output, start, count_);
int global_lock(int owner);

int global_lock(int owner);

int global_lock(int owner);

int global_lock(int owner);

int global_lock(int owner);

int ButtonGroup::button_click(int a1) {
  int foundActive = 0;
  int foundClicked = 0;
  int activeIndex = 0;
  int clickedIndex = 0;

  int count = count_;
  if (count > 0) {
    int activeId = field_84_;
    for (int i = 0; i < count; ++i) {
      int buttonId = buttons_[i]->field_A78_;
      if (buttonId == activeId) {
        activeIndex = i;
        foundActive = 1;
      }
      if (buttonId == a1) {
        clickedIndex = i;
        foundClicked = 1;
      }
    }
  }

  unsigned char flags = static_cast<unsigned char>(field_88_);
  if ((flags & 2) != 0) {
    if (static_cast<int>(field_84_) == a1) {
      field_84_ = 0xffffffff;
      foundClicked = 0;
    }
  }

  if (static_cast<int>(field_84_) != a1 || (flags & 1) == 0) {
    if (foundActive) {
      buttons_[activeIndex]->set(0);
      void *vtable = *reinterpret_cast<void **>(buttons_[activeIndex]);
      UVoid0 u;
      u.raw = *reinterpret_cast<void **>(reinterpret_cast<char *>(vtable) +
                                          0xf8);
      (reinterpret_cast<Dummy *>(buttons_[activeIndex])->*u.mfp)();
    }

    if (foundClicked) {
      buttons_[clickedIndex]->set(1);
      void *vtable = *reinterpret_cast<void **>(buttons_[clickedIndex]);
      UVoid0 u;
      u.raw = *reinterpret_cast<void **>(reinterpret_cast<char *>(vtable) +
                                          0xf8);
      (reinterpret_cast<Dummy *>(buttons_[clickedIndex])->*u.mfp)();
      field_84_ = a1;
    } else {
      field_84_ = 0xffffffff;
    }

    if (field_90_ == 0 && count_ != 0) {
      int idB = foundClicked ? buttons_[clickedIndex]->field_A78_ : -1;
      int idA = foundActive ? buttons_[activeIndex]->field_A78_ : -1;
      Win *owner = buttons_[0]->win_parent_;
      void *ownerVtable = *reinterpret_cast<void **>(owner);
      U2Int u2;
      u2.raw = *reinterpret_cast<void **>(
          reinterpret_cast<char *>(ownerVtable) + 0xbc);
      (reinterpret_cast<Dummy *>(owner)->*u2.mfp)(idA, idB);

      if (count_ != 0) {
        int idB2 = foundClicked ? buttons_[clickedIndex]->field_A78_ : -1;
        int idA2 = foundActive ? buttons_[activeIndex]->field_A78_ : -1;
        Win *owner2 = buttons_[0]->win_parent_;
        void *owner2Vtable = *reinterpret_cast<void **>(owner2);
        U3Int u3;
        u3.raw = *reinterpret_cast<void **>(
            reinterpret_cast<char *>(owner2Vtable) + 0xb8);
        (reinterpret_cast<Dummy *>(owner2)->*u3.mfp)(idA2, idB2, field_8C_);
      }
    }
  }

  int cnt2 = count_;
  for (int j = 0; j < cnt2; ++j) {
    if (buttons_[j]->field_A74_ != 0) {
      buttons_[j]->field_A74_ = 0;
      void *vtable = *reinterpret_cast<void **>(buttons_[j]);
      UVoid0 u;
      u.raw = *reinterpret_cast<void **>(reinterpret_cast<char *>(vtable) +
                                          0xf8);
      (reinterpret_cast<Dummy *>(buttons_[j])->*u.mfp)();
    }
    cnt2 = count_;
  }
  return 0;
}
