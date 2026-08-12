// ORIGINAL: 0x005F86A0 BYTE_EXACT
// name      sub_5f86a0
// size      31 bytes
// spans     0x005F86A0-0x005F86BF
// prototype 
// callers   1   call targets   0
// 0x005F86A0  sub_5f86a0  ->  _sub_5f86a0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Run the object's slot 7 step, but only when slot 23 reports it is
         needed.
Original Offset: 005F86A0
Return Value: n/a
Status: Complete
*/
// Returns void, not the catalogued int: nothing sets eax deliberately before
// `ret 4`. The only value there is slot023's result, used for the branch and
// never propagated.
// The shim must be at FILE SCOPE. A local class inside the function is
// C2599 in this dialect - every declared member needs an in-class body -
// and giving virtual slots bodies emits extra .text symbols the verifier
// then rejects.
class GateVCall {
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
  virtual int slot023();
};

extern "C" void __stdcall sub_5f86a0(int a1) {
    GateVCall *const self = reinterpret_cast<GateVCall *>(a1);
    if (self != 0) {
        if (self->slot023() == 0) {
            self->slot007();
        }
    }
}
