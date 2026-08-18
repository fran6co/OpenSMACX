// ORIGINAL: 0x004C88C0 sub_4c88c0 0x004C88C0-0x004C88E0 BYTE_EXACT
// size      32 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C88D0
// 0x004C88C0  sub_4c88c0  ->  _sub_4c88c0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Forward to the sound delegate at +0x3C through its virtual slot 35.
Original Offset: 004C88C0
Return Value: the slot's result, or zero when there is no delegate
Status: Complete
*/
// Catalogued `extern "C" int __stdcall sub_4c88c0(int)`; the receiver arrives
// in ecx with no stack load and `ret 4` pops the single explicit argument, so
// it is a __thiscall member. The null path is `xor eax,eax` (33 C0), not
// `xor al,al`, so the return really is int here.
class SndDelegate {
 public:
  virtual void slot000();
  virtual void slot001();
  virtual void slot002();
  virtual void slot003();
  virtual void slot004();
  virtual void slot005();
  virtual void slot006();
  virtual void slot007();
  virtual void slot008();
  virtual void slot009();
  virtual void slot010();
  virtual void slot011();
  virtual void slot012();
  virtual void slot013();
  virtual void slot014();
  virtual void slot015();
  virtual void slot016();
  virtual void slot017();
  virtual void slot018();
  virtual void slot019();
  virtual void slot020();
  virtual void slot021();
  virtual void slot022();
  virtual void slot023();
  virtual void slot024();
  virtual void slot025();
  virtual void slot026();
  virtual void slot027();
  virtual void slot028();
  virtual void slot029();
  virtual void slot030();
  virtual void slot031();
  virtual void slot032();
  virtual void slot033();
  virtual void slot034();
  virtual int slot035(int);
};

struct SoundDelegateHost {
    char pad[0x3c];
    SndDelegate *delegate;
    int forward(int a1);
};

int SoundDelegateHost::forward(int a1) {
    if (delegate) {
        return delegate->slot035(a1);
    }
    return 0;
}
