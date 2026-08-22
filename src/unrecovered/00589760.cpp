// ORIGINAL: 0x00589760 sub_589760 0x00589760-0x0058986A;0x00660240-0x00660255
// TRIED: no mangled name, so the emitter never declared the receiver as a class member - reads at offsets 0x9c8..0xa38 with the thiscall receiver cached to esi and no matching stack slot mean this is a method on a class derived from GraphicWin (the fields past 0xA14 belong to none of the catalogued classes' real layouts, so this is a new derived type, not Scroll despite the address range overlapping it). Declaring that class here as its own top-level type, with the method defined out of line so VC6 actually emits it, truncates the region this file's own extractor reads (a second top-level type ahead of the definition returns the brace-depth to zero before the real body) or, if the method stays inline in the class body, the compiler drops it as an unreferenced inline entirely - "found 0" either way. Landing it as a plain stdcall function taking the receiver as an explicit pointer keeps the whole thing as ONE top-level construct and one external symbol; the cost is the callee-pop count on the final `ret`, since the original is thiscall and pops two words for itself, not three. A local Buffer object reproduces the SEH prologue, the push -1 / handler / fs colon zero chain, that the original's stack-allocated Buffer constructor and destructor need, with no inline asm required. The one indirect vtable call, slot 1 at offset 4, goes through an empty local class used purely for its member-pointer representation, which draws no C4234 and no extra symbol. 94% mnemonic similarity was reached with the thiscall-member spelling, first divergence at instruction 23 - a cmp/jl versus a reload before the same compare; this stdcall spelling is close behind and the one that actually extracts. Landing the closest MISMATCH. NOTE: literal brace characters in this comment block truncate the region the same way a helper type would - keep it brace-free.
// size      287 bytes
// prototype
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D50A0 0x005D5250 0x005D7210 0x005D7410
// indirect  0x00589844

extern "C" void __stdcall sub_589760(GraphicWin *self, int a1, int a2) {
  class MfpBase {};
  typedef int (MfpBase::*Fn1)(int);
  union Conv1 {
    Fn1 asMfp;
    void *asPtr;
  } c;

  Buffer localBuf;
  char *e = (char *)self;

  *(int *)(e + 0xa34) = a1;
  *(int *)(e + 0xa38) = a2;
  *(int *)(e + 0xa14) = 5;
  *(int *)(e + 0xa18) = 0x2d;
  *(int *)(e + 0xa1c) = 0x226;
  *(int *)(e + 0xa20) = 0x12c;
  if (*g_00939e5c >= 0x3e8) {
    *(int *)(e + 0xa1c) = 0x2c0;
    *(int *)(e + 0xa20) = 0x180;
  }
  RECT *rect = (RECT *)(e + 0xa24);
  rect->left = 5;
  rect->top = 0x2d;
  rect->right = *(int *)(e + 0xa1c) + 5;
  rect->bottom = *(int *)(e + 0xa20) + 0x2d;

  self->init(rect, (char *)g_0068f5b4, 0x1400000, (Win *)*g_00945824, 0, 0);
  self->fill(9);
  c.asPtr = (*(void ***)self)[4 / 4];
  (reinterpret_cast<MfpBase *>(self)->*c.asMfp)(0);
}
