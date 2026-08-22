// ORIGINAL: 0x005EE330 ?resize_event@Win@@QAEHHH@Z 0x005EE330-0x005EE454
// TRIED: a named-global spelling for 0x9b7ab8, i.e. a static Win-pointer global assigned from `this`, reordered the entry register allocation - mismatch at instr #0; a plain store through a cast fixed address matches through instr #16-19 depending on how the vtable-slot-3/13 calls are spelled. The two indirect vtable calls, slot 3 at +0xc and slot 13 at +0x34, cannot use a virtual-shim helper class declared ahead of the definition: a second top-level type before the function truncates the region this file's own extractor reads (brace-depth returns to zero at the helper's own close, before the real body), and a LOCAL shim class needs every declared virtual defined, C2599, which VC6 then emits as extra external symbols and the unit fails the exactly-one-symbol check. A raw thiscall function pointer draws C4234. The working spelling: an empty LOCAL class with no members at all, so it carries no vtable and needs no symbol, used purely for its member-pointer representation, populated at runtime through a union from the vtable slot read as a plain pointer - legitimate member-pointer call syntax, so it invokes with the receiver in ECX like the original, and it is one self-contained region for this file's own extractor. 0.857 mnemonic similarity, first divergence at instr #19; landing the closest MISMATCH. NOTE: literal brace characters in this comment block will truncate the region the same way - keep this prose brace-free.
// size      292 bytes
// prototype int (__thiscall ?resize_event@Win@@QAEHHH@Z)(Win* this, int, int)
// callers   3   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED7D0 0x005F2760
// indirect  0x005EE3AA 0x005EE401 0x005EE422 0x005EE439 0x005EE444

int Win::resize_event(int a1, int a2) {
  class MfpBase {};
  typedef int (MfpBase::*Fn3)(int, int, int);
  union Conv3 {
    Fn3 asMfp;
    void *asPtr;
  };

  *(Win **)0x9b7ab8 = this;

  if ((iFlags_ & 0x40) == 0) {
    if (scroll_vert_ != 0) {
      move(outer_rect_.right - outer_rect_.left, 0);
      char *sv = (char *)scroll_vert_;
      char *sh = (char *)scroll_horz_;
      int iVar2;
      if (sh == 0) {
        iVar2 = outer_rect_.bottom - outer_rect_.top;
      } else {
        iVar2 = *(int *)(sh + 0x4c8) - outer_rect_.top + outer_rect_.bottom;
      }
      Conv3 c;
      c.asPtr = (*(void ***)sv)[0xc / 4];
      (reinterpret_cast<MfpBase *>(scroll_vert_)->*c.asMfp)(
          *(int *)(sv + 0x4c4), iVar2, 0);
    }
    if (scroll_horz_ != 0) {
      move(0, outer_rect_.bottom - outer_rect_.top);
      char *sh = (char *)scroll_horz_;
      char *sv = (char *)scroll_vert_;
      Conv3 c;
      c.asPtr = (*(void ***)sh)[0xc / 4];
      if (sv == 0) {
        (reinterpret_cast<MfpBase *>(scroll_horz_)->*c.asMfp)(
            outer_rect_.right - outer_rect_.left, -*(int *)(sh + 0x4c8), 0);
      } else {
        (reinterpret_cast<MfpBase *>(scroll_horz_)->*c.asMfp)(
            (outer_rect_.right - *(int *)(sv + 0x4c4)) - outer_rect_.left,
            -*(int *)(sh + 0x4c8), 0);
      }
    }
  }
  if (field_400_ != 0) {
    typedef void(__cdecl * Fn2)(int, int);
    Fn2 fn = (Fn2)field_400_;
    fn(a1, a2);
  }
  {
    typedef int (MfpBase::*Fn2m)(int, int);
    union Conv2 {
      Fn2m asMfp;
      void *asPtr;
    } c2;
    c2.asPtr = (*(void ***)this)[0x34 / 4];
    (reinterpret_cast<MfpBase *>(this)->*c2.asMfp)(a1, a2);
  }
  redo_caption_buttons();
  return 0;
}
