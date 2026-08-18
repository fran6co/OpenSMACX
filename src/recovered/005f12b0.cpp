// ORIGINAL: 0x005F12B0 sub_5f12b0 0x005F12B0-0x005F12D6 BYTE_EXACT
// size      38 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x005F12B7 0x005F12CF
// 0x005F12B0  sub_5f12b0  ->  _sub_5f12b0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Recover the window's C++ object and forward the message to it.
Original Offset: 005F12B0
Return Value: the handler's result, or zero when the window has no object
Status: Complete
*/
// Catalogued as taking NO arguments; the disassembly reads three off the
// stack and calls GetWindowLongA(hwnd, GWL_USERDATA).
class WndVCall {
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
  virtual void slot067();
  virtual void slot068();
  virtual void slot069();
  virtual int slot070(int, int);
};

extern "C" int __cdecl sub_5f12b0(void *window, int a2, int a3) {
    typedef long(__stdcall * GetWindowLongFn)(void *, int);
    // ONE variable returned on both paths. Two separate returns re-zero eax
    // with an instruction of its own instead of merging to one epilogue.
    int object = (*reinterpret_cast<GetWindowLongFn *>(g_0066934c))(
        window, -0x15);
    if (object) {
        object = reinterpret_cast<WndVCall *>(object)->slot070(a2, a3);
    }
    return object;
}
