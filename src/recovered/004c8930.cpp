// 0x004C8930  sub_4c8930  ->  _sub_4c8930
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Forward to the delegate at +0x3C through its virtual slot 38, or
         return zero when there is no delegate.
Original Offset: 004C8930
Return Value: the slot's result, or zero
Status: Complete
*/
class VObj_004C8930 {
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
  virtual int slot038();
};

struct S_004C8930 {
    char pad[0x3c];
    VObj_004C8930 *other;
    int Call();
};

int S_004C8930::Call() {
    if (other) {
        return other->slot038();
    }
    return 0;
}
