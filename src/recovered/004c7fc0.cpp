// ORIGINAL: 0x004C7FC0 sub_4c7fc0 0x004C7FC0-0x004C7FE7 BYTE_EXACT
// size      39 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7FD4
// 0x004C7FC0  sub_4c7fc0  ->  _sub_4c7fc0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Forward two arguments to the sound delegate's virtual slot 47.
Original Offset: 004C7FC0
Return Value: the slot's result, or 0x14 when there is no delegate
Status: Complete
*/
// Catalogued `extern "C" int __stdcall`; the receiver arrives in ecx with no
// stack push and `ret 8` pops only the two explicit arguments, which is
// __thiscall. The owning class is almost certainly the Midi family - same
// delegate at +0x3C, same address cluster - but no mangled name proves it,
// and the scaffolding does not expose `Midi` for these rows, so this stays a
// local overlay rather than inventing a member.
class MidiDelegate {
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
  virtual void slot035();
  virtual void slot036();
  virtual void slot037();
  virtual void slot038();
  virtual void slot039();
  virtual void slot040();
  virtual void slot041();
  virtual void slot042();
  virtual void slot043();
  virtual void slot044();
  virtual void slot045();
  virtual void slot046();
  virtual int slot047(int, int);
};

struct S_004C7FC0 {
    char pad[0x3c];
    MidiDelegate *other;
    int Call(int a1, int a2);
};

int S_004C7FC0::Call(int a1, int a2) {
    if (other) {
        return other->slot047(a1, a2);
    }
    return 0x14;
}
