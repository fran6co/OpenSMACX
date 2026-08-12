// ORIGINAL: 0x00448340 BYTE_EXACT
// name      sub_448340
// size      15 bytes
// spans     0x00448340-0x0044834F
// prototype 
// callers   0   call targets   0
// 0x00448340  sub_448340  ->  _sub_448340
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Forward to the inner object's virtual slot 24, or return zero when
         there is no inner object.
Original Offset: 00448340
Return Value: the slot's result, or zero
Status: Complete
*/
// A __thiscall member returning int, not the catalogued
// `extern "C" int __cdecl sub_448340()`.
class InnerObj_448340 {
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
};

struct S_00448340 {
    char pad[0x3c];
    InnerObj_448340 *inner;
    int Method();
};

int S_00448340::Method() {
    // Guard the WORK, not the early return: `if (p == 0) return 0;` negates
    // to `jne` where the original has `je`.
    InnerObj_448340 *const p = inner;
    if (p != 0) {
        return p->slot024();
    }
    return 0;
}
