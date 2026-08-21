// ORIGINAL: 0x004C9390 sub_4c9390 0x004C9390-0x004C93A2 BYTE_EXACT
// symbol    ?Call@S_004C9390@@QAEHXZ
// size      18 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// 0x004C9390  sub_4c9390  ->  _sub_4c9390
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Forward to the sub-object at +0x3C through its virtual slot 37, or
         return zero when there is none.
Original Offset: 004C9390
Return Value: the slot's result, or zero
Status: Complete
*/
// A nullary __thiscall member, not `extern "C" int __cdecl sub_4c9390()`.
// NOTE: writing `__thiscall` explicitly on an out-of-line member definition
// is a hard error in this dialect (C4234); it is already implicit.
class VObj_004C9390 {
 public:
  virtual int slot000();
  virtual int slot001();
  virtual int slot002();
  virtual int slot003();
  virtual int slot004();
  virtual int slot005();
  virtual int slot006();
  virtual int slot007();
  virtual int slot008();
  virtual int slot009();
  virtual int slot010();
  virtual int slot011();
  virtual int slot012();
  virtual int slot013();
  virtual int slot014();
  virtual int slot015();
  virtual int slot016();
  virtual int slot017();
  virtual int slot018();
  virtual int slot019();
  virtual int slot020();
  virtual int slot021();
  virtual int slot022();
  virtual int slot023();
  virtual int slot024();
  virtual int slot025();
  virtual int slot026();
  virtual int slot027();
  virtual int slot028();
  virtual int slot029();
  virtual int slot030();
  virtual int slot031();
  virtual int slot032();
  virtual int slot033();
  virtual int slot034();
  virtual int slot035();
  virtual int slot036();
  virtual int slot037();
};

struct S_004C9390 {
    char pad[0x3c];
    VObj_004C9390 *other;
    int Call();
};

int S_004C9390::Call() {
    if (other) {
        return other->slot037();
    }
    return 0;
}
