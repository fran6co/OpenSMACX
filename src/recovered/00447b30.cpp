// ORIGINAL: 0x00447B30
// 0x00447B30  ?new_base@BAmbience@@QAEXXZ  ->  ?new_base@BAmbience@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// Slot 67 at +0x10C takes one int (established by ten other byte-exact
// Ambience recoveries). Slot 69 at +0x114 is new here: the disassembly pushes
// the OLD value of +0x74 before the intervening call to slot 67's vtable
// fetch, then pushes 0xd immediately before `call [eax+0x114]` - two ints,
// (0xd, old field_74 value), with the second push hoisted early because
// ecx/eax get clobbered setting up the call.
class NewBaseBVCall {
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
  virtual void slot047();
  virtual void slot048();
  virtual void slot049();
  virtual void slot050();
  virtual void slot051();
  virtual void slot052();
  virtual void slot053();
  virtual void slot054();
  virtual void slot055();
  virtual void slot056();
  virtual void slot057();
  virtual void slot058();
  virtual void slot059();
  virtual void slot060();
  virtual void slot061();
  virtual void slot062();
  virtual void slot063();
  virtual void slot064();
  virtual void slot065();
  virtual void slot066();
  virtual void slot067(int);
  virtual void slot068();
  virtual void slot069(int, int);
};

void BAmbience::new_base() {
    char *const base = reinterpret_cast<char *>(this);
    const int reported = *reinterpret_cast<int *>(base + 0x74);
    *reinterpret_cast<int *>(base + 0x70) = *reinterpret_cast<int *>(base + 0x70) + 1;
    reinterpret_cast<NewBaseBVCall *>(this)->slot069(0xd, reported);
    reinterpret_cast<NewBaseBVCall *>(this)->slot067(0xd);
}
