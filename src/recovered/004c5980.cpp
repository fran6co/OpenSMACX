// ORIGINAL: 0x004C5980 BYTE_EXACT
// name      ??1Wave_In_Device@@QAE@XZ
// size      19 bytes
// spans     0x004C5980-0x004C5993
// prototype void (__thiscall ??1Wave_In_Device@@QAE@XZ)(Wave_In_Device* this)
// callers   0   call targets   0
// 0x004C5980  ??1Wave_In_Device@@QAE@XZ  ->  ??1Wave_In_Device@@QAE@XZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Re-install the shared device vtable and tear down the owned
         sub-object at +0x14 through its own virtual slot 4.
Original Offset: 004C5980
Return Value: n/a
Status: Complete
*/
class Wave_In_DeviceOwned {
 public:
  virtual void slot000();
  virtual void slot001();
  virtual void slot002();
  virtual void slot003();
  virtual void slot004();
};

Wave_In_Device::~Wave_In_Device() {
    *reinterpret_cast<int **>(this) = g_0066e098;
    Wave_In_DeviceOwned *const owned = reinterpret_cast<Wave_In_DeviceOwned *>(field_14_);
    if (owned) {
        owned->slot004();
    }
}
