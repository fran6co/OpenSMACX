/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#pragma once

struct VOX_Vect {
  float values[3];
};

struct VOX_Matrix {
  float values[3][3];
};

// UNALIGNED BY CONSTRUCTION: this group lives at 0xA5 inside `Caviar`, so it
// must have alignment 1 or the class grows. `pack(1)` is scoped to the struct
// and does not touch anything else.
#pragma pack(push, 1)
struct CaviarCamera {
  VOX_Vect position;    // +0  (0xA5)
  VOX_Matrix rotation;  // +12 (0xB1)
  float scaling;        // +48 (0xD5)
};
#pragma pack(pop)

class CaviarData {
 public:
  // IN-CLASS and `MEASURED`, not out-of-line in caviar.cpp: `Caviar::Caviar()`
  // has to be able to FOLD this into its 200-element array-construction loop,
  // which is what the image writes out inline there. `MEASURED` rather than a
  // plain `inline` keeps 0x00616BC0's own COMDAT measurable; its marker stays
  // in caviar.cpp with a `// body` fact.
  MEASURED CaviarData() : field_0_(0), fileDescriptor_(0), record_(nullptr) { }
  void close();

 private:
  // ALL THREE ARE OWNED ALLOCATIONS. `Caviar::close` frees and nulls every one
  // of them for all 200 slots - `[esi-4]`, `[esi]`, `[esi+4]` around 0x750 -
  // so modelling the first two as `uint32_t` said less than the image does.
  // `CaviarData::close` still only releases `record_`, through the helper;
  // the two teardowns are genuinely different and both are in the image.
  void *field_0_;
  void *fileDescriptor_;
  void *record_;   // 0x8, the renderer record close releases

  // The renderer owns these slots and tears them down in bulk.
  friend class Caviar;
};

class Caviar {
 public:
  void close();   // 0x00617020
  void UNK11(int *out1, int *out2, int *out3);
  Caviar();
  // No `??1Caviar@@...` is catalogued anywhere in the image; every embedding
  // class's own teardown (StatusWin::~StatusWin, and the still-forwarded
  // BaseWin/DesignWin/ProdPicker destructors) reaches Caviar through
  // close() instead of a destructor call. Giving Caviar member subobjects a
  // real (not raw-storage) presence needs SOME destructor for the compiler
  // to invoke automatically in reverse declaration order, so this one
  // forwards to the same close() the image itself calls - not a
  // byte-matching trick, a real reading of how Caviar tears down.
  ~Caviar() { close(); }
  void set_camera_direct(const VOX_Vect *camera, const VOX_Matrix *matrix);
  void set_scaling(float scaling);
  void set_scaling_bits(uint32_t scaling_bits);
  float get_scaling();

  int UNK3(int a, int b, int c);
  int UNK4(int a1, int a2, int a3, int a4);
  void UNK5(int a1, int a2);
  void UNK6(int a1, int a2);
  void UNK7(int a1, int a2);
  void UNK9(int a1, int a2, int a3);
  void UNK12(int a1, int a2, int a3);
  void UNK8(int menu_id);
  void UNK10(int a1, int a2, int a3);
  void set_scene_rotation(float x, float y, float z);

  // `static`, because the image's own names for these end in `QAA` - a public
  // member declared `__cdecl`, which takes no receiver in ecx and puts every
  // argument on the stack. WinMain's `call 0x6185a0` sets up no ecx at all, so
  // the only spelling a call site can use is `Caviar::init_class()`, and that
  // is `C2352: illegal call of non-static member function` unless it is
  // declared this way. Same rule tools/emit_translation_unit.py applies to
  // every other `*::init_class`, and the same one palette.h already follows
  // for `set_active_window`.
  static int init_class();
  static void close_class();
  // The six file-IO shims handed to the voxel engine (0x00618E10..
  // 0x00618E90). STATIC, each `__cdecl`, none touches a receiver: the
  // bodies read their parameters straight off [esp+4].. with no this -
  // measured on the image; the catalogue's QAA spellings on vx_seek and
  // vx_tell would shift every read +4 and the bytes do not have that.
  // One-line forwarders onto the allocator and the CRT: mem_get, free,
  // _read, _write, _lseek, _tell. vx_malloc is `void` yet the engine
  // reads its result - the body calls mem_get and returns without
  // touching eax, so the pointer survives in the return register.
  static void vx_malloc(unsigned long size);
  static void vx_free(void *block);
  static void vx_read(int fd, char *buffer, long count);
  static void vx_write(int fd, void *buffer, long count);
  static void vx_seek(int fd, long offset, int origin);
  static void vx_tell(int fd);

 private:
  float scene_scale_;
  // Fields are carved out of the opaque span rather than appended, keeping
  // the total at the pinned 0x13D0 - the static_assert below is what verifies
  // the carving did not shift anything.
  uint32_t field_4_;  // 0x4
  uint8_t field_8_[0xC];  // 0x8
  uint32_t field_14_;  // 0x14
  uint8_t field_18_[0x4];  // 0x18
  uint32_t field_1C_;  // 0x1C
  uint32_t field_20_;  // 0x20
  uint32_t field_24_;  // 0x24
  uint32_t field_28_;  // 0x28
  int32_t field_2C_;
  int32_t field_30_;
  int32_t field_34_;
  uint32_t field_38_;  // 0x38
  uint32_t field_3C_;  // 0x3C
  uint32_t field_40_;  // 0x40
  uint32_t field_44_;  // 0x44
  uint32_t field_48_;  // 0x48
  uint32_t field_4C_;  // 0x4C
  uint32_t field_50_;  // 0x50
  uint32_t field_54_;  // 0x54
  uint32_t field_58_;  // 0x58
  uint32_t field_5C_;  // 0x5C
  uint32_t field_60_;  // 0x60
  uint32_t field_64_;  // 0x64
  uint32_t field_68_;  // 0x68
  float field_6C_;  // 0x6C
  float field_70_;  // 0x70
  uint32_t field_74_;  // 0x74
  uint32_t field_78_;  // 0x78
  uint32_t field_7C_;  // 0x7C
  uint32_t field_80_;  // 0x80
  uint32_t field_84_;  // 0x84
  uint32_t field_88_;  // 0x88
  uint32_t field_8C_;  // 0x8C
  uint32_t field_90_;  // 0x90
  uint32_t field_94_;  // 0x94
  uint32_t field_98_;  // 0x98
  uint32_t field_9C_;  // 0x9C
  uint32_t field_A0_;  // 0xA0
  uint8_t field_A4_;  // 0xA4
  // NAMED THROUGH A PACKED STRUCT, because 0xA5 is not 4-aligned. The
  // arithmetic is exact: `set_camera_direct` memcpys a `VOX_Vect` to 0xA5 and
  // a `VOX_Matrix` to 0xB1, 0xA5 + 12 is 0xB1, and 0xB1 + 36 is 0xD5, which is
  // where `get_scaling` reads its float - the image does it in one
  // `fld dword ptr [ecx + 0xd5]`.
  //
  // Declaring those three as ordinary members makes VC6 pad to their natural
  // 4-byte alignment and `sizeof(Caviar)` stops being 0x13D0. `#pragma pack(1)`
  // on the group alone gives it alignment 1, so it sits at 0xA5 with no
  // padding and the class size is unchanged - which the static_assert below
  // checks rather than takes on trust.
  CaviarCamera camera_;      // 0xA5, 52 bytes: vector, matrix, scaling
  uint8_t field_D9_[0x2F];   // 0xD9
  int32_t field_108_;
  uint8_t field_10C_[0x640];  // 0x10C
  // THE OBJECT-DATA ARRAY. The image does NOT memset it - it runs a 200-turn
  // loop writing three zero dwords per element, which is `CaviarData`'s own
  // constructor folded into the implicit array construction. Declaring the
  // array means neither that nor `Caviar::close` has to reach it through a
  // cast, and `Caviar::Caviar()` writes nothing about it at all.
  // The arithmetic reconciles exactly: 200 * 0xC is 0x960, from 0x74C to
  // 0x10AC, and 0x960 + 0x320 is 0xC80, which is what the four fields this
  // replaces occupied (4 + 4 + 4 + 0xC74).
  CaviarData slots_[200];       // 0x74C
  uint8_t field_10AC_[0x320];   // 0x10AC
  uint32_t field_13CC_;  // 0x13CC
};

static_assert(sizeof(CaviarData) == 0xC,
              "CaviarData layout must match the original executable");
static_assert(sizeof(Caviar) == 0x13D0,
              "Caviar layout must match the original executable");


// The 413-byte helper that walks the record and frees its members is not yet
// source-owned, so close reaches it through a rebindable dependency.
// 0x00638430, a pending_bodies forwarder. A POINTER here cost CaviarData::close
// a `call dword ptr [...]` where the image has `call rel32`.
void __cdecl caviar_free_record(void *record);

// The rotation is applied by a helper that is not recovered yet.
// 0x00627D00, a pending_bodies forwarder, for the same reason as
// `caviar_free_record` above.
void __cdecl caviar_apply_rotation(float *angles, void *matrix);

// ---------------------------------------------------------------------------
// The voxel (CAVIAR) engine seam init_class reaches. None of the three is
// recovered yet; each is a pending_bodies forwarder, for the same reason as
// `caviar_free_record` above. Both 0x0063AF60 and 0x006393C0 are __cdecl -
// the image cleans their arguments with one `add esp, 0x20` that covers the
// pair of calls - and 0x006392E0's answer is tested with `test al, al`.
// ---------------------------------------------------------------------------

// 0x006392E0. Hands the six vx_* file-IO callbacks (the table init_class
// builds, 0x00618E10..0x00618E90) to the voxel engine and reports whether it
// initialised; a false answer skips the CAVIAR_INVALIDCPU popup. char, not
// int: the image's call site tests the answer with `test al, al` - a byte
// -sized value (init_class: image instruction 25).
char __cdecl vox_init_callbacks(unsigned long *callbacks, int flag);

// 0x0063AF60. Fills the scene colour table with `count` entries of `value`
// (0xffff through 0x10000 entries here).
void __cdecl vox_fill_colour_table(void *table, unsigned long value,
                                   unsigned long count);

// 0x006393C0. Creates the scene render record from the descriptor block
// init_class fills at 0x009BB438; the created handle lands at 0x009BB478.
// The renderer record vox_create_record builds. 0x80 bytes are allocated and
// all of them are zeroed, so the tail is unknown padding.
struct VoxRenderRecord {
    uint32_t setup_id_;        // 0x00 - first argument, stored verbatim
    uint32_t setup_block_;     // 0x04 - the descriptor block
    uint32_t clip_max_a_x_;    // 0x08  0x7fff
    uint32_t clip_max_a_y_;    // 0x0C  0x7fff
    uint32_t clip_min_a_x_;    // 0x10  0xffff8001 (-32767)
    uint32_t clip_min_a_y_;    // 0x14  0xffff8001
    uint32_t zero_a_[2];       // 0x18
    uint32_t clip_max_b_x_;    // 0x20  0x7fff
    uint32_t clip_max_b_y_;    // 0x24  0x7fff
    uint32_t clip_min_b_x_;    // 0x28  0xffff8001
    uint32_t clip_min_b_y_;    // 0x2C  0xffff8001
    uint32_t zero_b_[4];       // 0x30
    int32_t width_minus_1_;    // 0x40
    int32_t height_minus_1_;   // 0x44
    uint32_t width_;           // 0x48 - width, or 0 when a side is negative
    uint32_t height_;          // 0x4C
    void *colour_table_;       // 0x50
    void *shadow_table_;       // 0x54
    uint32_t field_58_[2];     // 0x58 - init_class stores &object_start at +0x58
    // 0x60, 0x64. The two per-entry ramp pointers. Created zeroed, then
    // owned by sub_63f9b0: it frees each, reallocates count<<2 bytes (count
    // = the descriptor's entry count at setup_block+0x10), and fills ramp A
    // from *(setup_block+4) stepped *(setup_block+0x18)*2 per entry, ramp B
    // as a running accumulation of *(setup_block+0x14) per entry.
    // init_class fills the same slots with its own two 256-entry ramps.
    void *ramp_a_ptr_;         // 0x60
    void *ramp_b_ptr_;         // 0x64
    uint8_t setup_size_code_;  // 0x68 - low byte of the last argument
    uint8_t field_69_[0x80 - 0x69];
};

static_assert(sizeof(VoxRenderRecord) == 0x80,
              "renderer record must be the 0x80 bytes the engine allocates");

VoxRenderRecord *__cdecl vox_create_record(int a1, void *a2, void *a3, void *a4,
                                        int a5);

