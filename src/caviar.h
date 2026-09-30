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

// caviar.cpp includes buffer.h; this forward declaration is only for the
// renderer's own Buffer-taking members declared below.
class Buffer;

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

// SAME STORY ONE STRUCT LATER: the six-int group lives at 0xED inside
// `Caviar`, so the block that carries it must have alignment 1 too.
#pragma pack(push, 1)
struct CaviarChannelBounds {
  // 0xD9..0xEC, five engine limits init seeds: 0, 0xc77fff00, 0x477fff00, 0,
  // 0x477fff00 - the outer pair is a float -1e38/+1e38-style clip range.
  uint32_t lead_[5];     // 0x00; at Caviar+0xD9
  int32_t values_[6];    // 0x14; at Caviar+0xED, the six channel bounds init, set_buffer and
                         // render_scene all reset to {0,0,0xff,0xff,0x100,0x100}
  uint8_t tail_[3];      // 0x2C; at Caviar+0x105
};
#pragma pack(pop)

// ---------------------------------------------------------------------------
// THE CVR RECORD - the in-memory form one .cvr file loads to. Everything the
// add_object family reads comes out of one of these, and the offsets below are
// read off those bodies' own loads (0x006176E0..0x00618080) and confirmed by
// caviar_free_record (0x00638430), which releases exactly these arrays:
//
//   +0x20  step_count_   the rotation count every instance index is taken
//                        modulo - `a % record->step_count_`
//   +0x24  part_count_   the length of the three per-part arrays below
//   +0x28  slots_        an array of part_count_ dwords, each freed
//   +0x2C  steps_        an array of part_count_ 8-byte entries
//   +0x30  parts_        an array of part_count_ CvrPart, all three pointers
//                      of every entry freed
//   +0x34  shadows_      an array of part_count_ 12-byte entries, +4 and +8
//                      of each freed
//
// A part is three PARALLEL ARRAYS, one entry per rotation step: the colour
// bytes, the geometry vectors and the matrices. add_object indexes all three
// with the same step, 1 / 12 / 36 bytes at a time.
// ---------------------------------------------------------------------------
struct CvrPart {
  uint8_t *colours_;     // +0x00, one byte per step
  VOX_Vect *vectors_;    // +0x04, one 12-byte vector per step
  VOX_Matrix *matrices_; // +0x08, one 36-byte matrix per step
};

// What one step entry's second dword POINTS AT, named from the engine's own
// camera transform (0x0063A6F0), which is the only body that dereferences it:
// it loads a float at +8 (image 0x63A746 `fld dword ptr [esi + 8]`) and six
// signed 16-bit fields at +0x14..+0x1f (`movsx` off the same register), and
// multiplies all seven by one scale. The rest of the block belongs to the
// loader and nothing here reads it.
struct CvrShape {
  uint32_t field_0_;   // +0x00, serialized prefix; interpretation unresolved
  uint32_t byte_size_; // +0x04, current shape bytes moved by patch chunks
  float scale_;        // +0x08, the whole-shape scale
  uint8_t mid_[0x8];   // +0x0C
  int16_t axis_x_;     // +0x14, the three per-axis extents the AABB is built from
  int16_t axis_y_;     // +0x16
  int16_t axis_z_;     // +0x18
  int16_t offset_x_;   // +0x1A, the three per-axis offsets
  int16_t offset_y_;   // +0x1C
  int16_t offset_z_;   // +0x1E
};

static_assert(sizeof(CvrShape) == 0x20,
              "CvrShape must reach its six per-axis fields at +0x14..+0x1f");

struct CvrStep {
  uint32_t lead_;      // +0x00
  CvrShape *shape_;    // +0x04, the shape the transform scales this part by
};

struct CaviarData;

// Renderer-owned destinations, not loaded CVR models. Caviar::init allocates
// 1/12/36 bytes per entry; Caviar::close releases all three. The renderer's
// destructor tails close without a second array-destructor traversal.
struct VoxInstance {
  VoxInstance() : instance_colour_(0), instance_vector_(0), instance_matrix_(0) { }
  uint8_t *instance_colour_;
  // RECOVERY_MEMBER: v3 VoxInstance instance_vector_ VOX_Vect* 0x4 0x00616DE0
  VOX_Vect *instance_vector_;
  // RECOVERY_MEMBER: v3 VoxInstance instance_matrix_ VOX_Matrix* 0x8 0x00616DE0
  VOX_Matrix *instance_matrix_;
};

static_assert(sizeof(VoxInstance) == 0xC, "renderer instance entries have 12-byte stride");

// 0x0076E890, defined in caviar.cpp. Declared here (not in a game.cpp static)
// so the 0x300-byte pointer table load_bases (0x00453A20) fills can name its
// end bound as this object instead of a second name for the same address -
// the duplicate-globals census counts that as two names for one address.
extern CaviarData g_VCTB_CAVIARDATA;
extern CaviarData g_VBP_CAVIARDATA;
extern CaviarData g_SSF_CAVIARDATA;
extern CaviarData g_SDP_CAVIARDATA;
extern CaviarData g_SAS_CAVIARDATA;
extern CaviarData g_SCD_CAVIARDATA;
extern CaviarData g_SCJ_CAVIARDATA;
extern CaviarData g_SAGS_CAVIARDATA;
extern CaviarData g_SFT_CAVIARDATA;
extern CaviarData g_VHR_CAVIARDATA1;
extern CaviarData g_SHT_CAVIARDATA;
extern CaviarData g_SRB_CAVIARDATA;
extern CaviarData g_ASAS_CAVIARDATA;
extern CaviarData g_RESLASER_CAVIARDATA;
extern CaviarData g_RESBOLT_CAVIARDATA;
extern CaviarData g_FUNLOAD_CAVIARDATA;
extern CaviarData g_TECLOAD_CAVIARDATA;
extern CaviarData g_SP_DISSWAVE_CAVIARDATA;
extern CaviarData g_SP_MARINED_CAVIARDATA;
extern CaviarData g_SP_NANOO_CAVIARDATA;
extern CaviarData g_SP_SOPORIFIC_CAVIARDATA;
extern CaviarData g_AA01_CAVIARDATA;
extern CaviarData g_AA_ROVER_CAVIARDATA;
extern CaviarData g_AA_WING_CAVIARDATA[2];
extern CaviarData g_ACP_CAVIARDATA[4];
extern CaviarData g_AX_CAVIARDATA;
extern CaviarData g_AA_CAVIARDATA;
extern CaviarData g_ACOLPOD_CAVIARDATA;
extern CaviarData g_AT_CAVIARDATA;
extern CaviarData g_VTA_CAVIARDATA;
extern CaviarData g_ATP_CAVIARDATA;
extern CaviarData g_SSFA_CAVIARDATA;
extern CaviarData g_SFTA_CAVIARDATA;
extern CaviarData g_VW00_CAVIARDATA;
extern CaviarData g_VIPTAWL_CAVIARDATA;
extern CaviarData g_VIPTASGN_CAVIARDATA;
extern CaviarData g_VIPTAPSI_CAVIARDATA;
extern CaviarData g_VIPTR_CAVIARDATA[4];
extern CaviarData g_VIPTA_CAVIARDATA[2];
extern CaviarData g_PTMOD_CAVIARDATA;
extern CaviarData g_VB_CAVIARDATA;
extern CaviarData g_VGMC_CAVIARDATA;
extern CaviarData g_VGMCP_CAVIARDATA;
extern CaviarData g_VLIGHTS_CAVIARDATA;
extern CaviarData g_VPT_CAVIARDATA;
extern CaviarData g_A_CAVIARDATA;
extern CaviarData g_APWALL_CAVIARDATA;
extern CaviarData g_ASGEN_CAVIARDATA;
extern CaviarData g_APSID_CAVIARDATA;
extern CaviarData g_VA01_CAVIARDATA;
extern CaviarData g_VR_CAVIARDATA[4];
extern CaviarData g_VHR_CAVIARDATA2;
extern CaviarData g_VRC_CAVIARDATA[4];
extern CaviarData g_VI_CAVIARDATA;
extern CaviarData g_VGMT_CAVIARDATA;
extern CaviarData g_VGMTP_CAVIARDATA;
extern CaviarData g_VWNTU_CAVIARDATA;
extern CaviarData g_VT_CAVIARDATA;
extern CaviarData g_DROP_CAVIARDATA;
extern CaviarData g_DROPLET_CAVIARDATA;
extern CaviarData g_VCL_CAVIARDATA;
extern CaviarData g_VCLT00_CAVIARDATA;
extern CaviarData g_VHT_VBP_CAVIARDATA;
extern CaviarData g_VHTP_CAVIARDATA;
extern CaviarData g_VHTA0_CAVIARDATA[2];
extern CaviarData g_VHTTP_CAVIARDATA;
extern CaviarData g_VHTTPA0_CAVIARDATA[3];
extern CaviarData g_VSP_CAVIARDATA;
extern CaviarData g_VSPA0_CAVIARDATA[2];
extern CaviarData g_VSPTF_CAVIARDATA;
extern CaviarData g_VSPTB_CAVIARDATA;
extern CaviarData g_VFL_CAVIARDATA;
extern CaviarData g_VGS_CAVIARDATA;
extern CaviarData g_VGSP_CAVIARDATA;
extern CaviarData g_VJTP_CAVIARDATA;
extern CaviarData g_VJT0_CAVIARDATA[2];
extern CaviarData g_VCU_CAVIARDATA;
extern CaviarData g_VCUP_CAVIARDATA;
extern CaviarData g_VCUA0_CAVIARDATA[2];
extern CaviarData g_VCUW_CAVIARDATA;
extern CaviarData g_VCT_CAVIARDATA;
extern CaviarData g_VCTP_CAVIARDATA;
extern CaviarData g_VCT0_CAVIARDATA[2];
extern CaviarData g_VWNTT_CAVIARDATA;
extern CaviarData g_VWNST_CAVIARDATA;
extern CaviarData g_VWNAA_CAVIARDATA;
extern CaviarData g_VW_CAVIARDATA[16];
extern CaviarData g_VM_CAVIARDATA;
extern CaviarData g_VM13_CAVIARDATA;
extern CaviarData g_VPBR0_CAVIARDATA[4];
extern CaviarData g_NW_CAVIARDATA;
extern CaviarData g_NI_CAVIARDATA;
extern CaviarData g_NLC_CAVIARDATA;
struct VOX_ScnRenderHnd;
struct VOX_Rect;
struct VoxRenderRecord;
struct VoxCameraBlock;

// ---------------------------------------------------------------------------
// THE RENDERER AS THE ENGINE SEES IT. The engine's own draw (0x00639E60) and
// camera transform (0x0063A6F0) are handed this block as their first argument,
// and both call sites pass `this + 0x18` (image 0x61743F
// `lea eax, [ebp + 0x18]`), so every offset below lands on a Caviar member and
// the struct is the engine's name for the renderer's own state rather than a
// second object.
// ---------------------------------------------------------------------------
struct VoxSceneState {
  uint8_t lead_[0x4];          // +0x00  Caviar::field_18_
  int32_t instance_count_;     // +0x04  Caviar::field_1C_, the count render
                               //         stores before it draws
  uint32_t draw_flags_;        // +0x08  Caviar::field_20_
  // RECOVERY_MEMBER: v3 VoxSceneState instances_ VoxInstance* 0xC 0x0063A6F0
  VoxInstance *instances_;     // +0x0C  Caviar::field_24_, &slots_[0]
  CvrStep *instance_steps_;    // +0x10  Caviar::field_28_, &instance_steps_[0]
  VOX_Vect scene_offset_;      // +0x14  Caviar::field_2C_ .. field_34_
  VOX_Matrix scene_rotation_;  // +0x20  Caviar::rotation_ at 0x38
  float scale_;                // +0x44  Caviar::field_5C_, seeded 1.0f
};

static_assert(offsetof(VoxSceneState, instances_) == 0x0C,
              "VoxSceneState must alias Caviar's instance table at +0x0C");
static_assert(offsetof(VoxSceneState, scene_rotation_) == 0x20,
              "VoxSceneState must alias Caviar's rotation at +0x20");
static_assert(sizeof(VoxSceneState) == 0x48,
              "VoxSceneState must reach its scale at +0x44");

// THE THIRD BLOCK THE ENGINE IS HANDED. The draw reads the part's clip bounds
// at +0x48 and +0x58/+0x5c of this one, which are Caviar's six channel-bound
// ints at 0xED - so this is the camera and the bounds that follow it, passed
// as `this + 0xA5`.
#pragma pack(push, 1)
struct VoxCameraBlock {
  CaviarCamera camera_;         // +0x00, Caviar::camera_ at 0xA5
  CaviarChannelBounds bounds_;  // +0x34, Caviar::field_D9_ at 0xD9
};
#pragma pack(pop)

static_assert(offsetof(VoxCameraBlock, bounds_) == 0x34,
              "VoxCameraBlock must reach the channel bounds at +0x34");
static_assert(offsetof(VoxCameraBlock, bounds_.values_) == 0x48,
              "the bounds' six ints must start at +0x48 for the draw");

struct CvrRecord {
  uint32_t flags_;       // +0x00, bit 0 distinguishes explicit step sizes
  uint32_t state_;       // +0x04, set to 3 after the file is consumed
  float version_;       // +0x08, selects the old palette format below 7
  char *description_;   // +0x0C
  char *palette_name_;  // +0x10
  // RECOVERY_MEMBER: v1 CvrRecord palette_ 0x14 0x00638720
  uint8_t *palette_;    // +0x14, instance palette data begins 0x302 bytes in
  uint8_t *field_18_;   // +0x18, payload of palette chunk 0x01030000
  char *parts_name_;    // +0x1C
  // BOTH SIGNED, and the division proves it: every add_object takes the step
  // modulo +0x20 with `idiv` (`cdq` before it), and tests +0x24 with
  // `test esi, esi / jle`. Unsigned members would give `div` and `jbe`.
  int32_t step_count_;   // +0x20
  int32_t part_count_;   // +0x24
  char **slot_table_;    // +0x28, per-part names
  // RECOVERY_MEMBER: v3 CvrRecord steps_ CvrStep* 0x2C 0x00638AD0
  CvrStep *steps_;       // +0x2C
  // RECOVERY_MEMBER: v3 CvrRecord parts_ CvrPart* 0x30 0x006388C0
  CvrPart *parts_;       // +0x30
  // RECOVERY_MEMBER: v3 CvrRecord shadows_ CvrShadow* 0x34 0x00638F30
  struct CvrShadow *shadows_; // +0x34
  uint32_t field_38_;    // +0x38, zeroed by the loader; role unresolved
  uint32_t field_3C_;    // +0x3C, zeroed by the loader; role unresolved
  // RECOVERY_MEMBER: v1 CvrRecord user_length_ 0x40 0x006381F0
  int32_t user_length_;  // +0x40, callback's current chunk length minus 8
};

struct CvrShadow {
  uint32_t field_0_;    // +0x00, payload of chunk 0x04040401
  uint8_t *field_4_;    // +0x04
  uint8_t *field_8_;    // +0x08
};

struct CvrChunkHeader {
  int32_t tag;
  int32_t length;
};

static_assert(sizeof(CvrRecord) == 0x44, "CVR loader allocates and clears 0x44 bytes");
static_assert(sizeof(CvrShadow) == 0xC, "CVR shadow entries have three words");

struct CaviarData {
 public:
  // Loaded models have their own lifetime: original array initialization
  // supplies 0x616BC0 and destructor 0x456100 to the EH iterator.
  MEASURED CaviarData() : field_0_(0), fileDescriptor_(0), record_(nullptr) { }
  ~CaviarData() { close(); }
  void close();
  // 0x00616BD0. Loads a .cvr file into this slot: releases whatever record
  // it still holds, opens the file, hands the descriptor to the loader and
  // closes it again once the record is built.
  int init(char *name);
  const CvrRecord *record() const { return record_; }

 private:
  uint32_t field_0_;         // +0x0, zeroed; role unresolved
  // RECOVERY_MEMBER: v1 CaviarData fileDescriptor_ 0x4 0x00616BD0
  int fileDescriptor_;       // +0x4, CRT _open result, including -1
  // RECOVERY_MEMBER: v3 CaviarData record_ CvrRecord* 0x8 0x00616C60
  CvrRecord *record_;        // +0x8, released by close and the destructor

  // add_object reads loaded definitions into separate VoxInstance storage.
  friend class Caviar;
};

class SelectPartWin;

class Caviar {
 public:
  friend class SelectPartWin;
  friend class DesignWin;
  // Vehicle assembly starts a new instance list without releasing any of
  // the renderer's allocated slots or loaded model definitions.
  void reset_objects() { field_13CC_ = 0; }
  void close();   // 0x00617020
  void UNK11(float *out1, float *out2, float *out3);
  Caviar();
  // 0x00616DE0. Builds the 200-instance table and seeds the scene state: the
  // three allocations per instance, the rotation, the clipping defaults and
  // the channel bounds, then switches the renderer on.
  void init();
  // Original 0x4210F0 tails close; instance entries have no additional
  // destructor work. Loaded CaviarData models have a separate destructor.
  ~Caviar() { close(); }
  void set_camera_direct(const VOX_Vect *camera, const VOX_Matrix *matrix);
  void set_scaling(float scaling);
  void set_scaling_bits(uint32_t scaling_bits);
  float get_scaling();

  // 0x00618520 and 0x00618560 - the image's UNK13 and UNK14, named from what
  // both bodies do: they read the created render record's clip bounds, one to
  // fill the engine's clip rectangle and return its address, the other to
  // copy the same four values into a caller's array. The catalogue spells
  // the first `X` (void) but the body returns the rectangle's address in eax,
  // so the declaration says so.
  void *clip_rect();
  void fill_clip_rect(int destination);
  // 0x00618430. Copies 0x100 bytes of shadow table into the created record.
  int set_shadow_table(unsigned char *table);
  // 0x006183D0. Rejects a draw with no Buffer and resets the six-int group.
  int set_buffer(Buffer *buffer);
  // The image's UNK2, named from its own stores: it hands the scene record a
  // colour table, raises field_4_'s bit 4 - the bit object_start tests to
  // decide where the next draw reads its colour table from - calls render,
  // and drops the bit again. centre_x/centre_y are the centre of the box
  // render builds 0x100 deep around, and render_flags is render's own mask.
  int render_with_colour_table(Buffer *buffer, int centre_x, int centre_y,
                               int colour_table, int render_flags);
  // 0x00617230. The renderer's own draw. Not homed yet - pending_bodies.cpp
  // forwards it - and the declaration is what keeps the wrapper's call a
  // plain `call rel32` like the image. centre_x/centre_y are the centre of
  // the 0x100-deep box it builds (its own body subtracts 0x80 from each) and
  // flags is its bit mask.
  int render(Buffer *buffer, int centre_x, int centre_y, int flags);
  // 0x00617920..0x00618080. The eight add_object overloads append the parts of
  // one .cvr definition to the renderer's instance table: each part's three
  // arrays are indexed with the caller's step, the geometry is accumulated
  // into the instance's own vector and matrix, and the instance's palette and step
  // entry are copied out of the record. Every one returns 0, 1 when the table
  // is full (200), 7 when a definition carries no record, or 0x10 on a null
  // argument.
  int add_object(CaviarData *definition, VOX_Vect *offset, int step);
  int add_object(CaviarData *definition, CaviarData *geometry,
                 VOX_Vect *offset, int step);
  int add_object(CaviarData *definition, uint8_t *object,
                 CaviarData *geometry, VOX_Vect *offset, int step);
  int add_object(CaviarData *definition, uint8_t *object, VOX_Vect *offset,
                 int step);
  int add_object(CaviarData *definition, CaviarData *geometry, int step);
  int add_object(CaviarData *definition, uint8_t *object,
                 CaviarData *geometry, int step);
  int add_object(CaviarData *definition, int step);
  int add_object(CaviarData *definition, uint8_t *object, int step);
  int UNK3(int a, int b, int c);
  // 0x00617620. Renders with the engine's monochrome table: fills the shared
  // 0x1800-byte colour source with one byte repeated, raises field_4_'s bit 3
  // - the "read the colour from the shared source" bit - draws, and drops it.
  // colour is truncated to a byte, the same table index render's own output
  // goes through.
  // 0x00617080, the catalogue's UNK1 and the renderer's own top-level draw:
  // reset the channel bounds, hand the record and the camera to the engine,
  // union the clip rectangle the draw produced into the viewport, clear.
  int render_scene(Buffer *buffer, int flags);
  // 0x00618470. Clears the two scene surfaces and, when the argument is zero,
  // the viewport's slice of the shared colour table.
  void clear_buffers(int fill);
  // 0x006171A0. Renders and blits the result into surface A as a multi-table
  // destination; the bits pointer is the blit's source table.
  void render_multi_table_dest(Buffer *buffer, int centre_x, int centre_y,
                               int fill, unsigned char *bits, int flags);
  void render_mono(Buffer *buffer, int centre_x, int centre_y, int colour,
                   int flags);
  int UNK4(int a1, int a2, int a3, int a4);
  void UNK5(int a1, int a2);
  void UNK6(int a1, int a2);
  void UNK7(int a1, int a2);
  void UNK9(int a1, int a2, int a3);
  void UNK12(int a1, int a2, int a3);
  void UNK8(int menu_id);
  void UNK10(float offset_x, float offset_y, float offset_z);
  void set_scene_rotation(float x, float y, float z);
  // 0x00618DA0. The engine's per-object callback: fills the handle's colour
  // table with 0x1800 bytes chosen by field_4_'s bits - the shared monochrome
  // source when bit 3 is on, the caller's table when bit 4 is on, otherwise
  // the instance table's own entry for the step.
  // `__cdecl` MEMBER, because the image's own name spells `QAA`: a member
  // declared __cdecl takes no receiver, which is what lets init_class store
  // its address as the engine's per-object callback. The five engine
  // arguments are the callback's protocol and the body reads two of them.
  // The last four arguments are the engine's callback protocol - position,
  // orientation, bounds, direction, the packet the engine's other per-object
  // callbacks take - and this body reads none of them; only the handle and
  // the step reach the colour table.
  void __cdecl object_start(VOX_ScnRenderHnd *handle, long step,
                            VOX_Vect *position, VOX_Matrix *orientation,
                            VOX_Rect *bounds, VOX_Vect *direction);
  // 0x006181D0. The shadow direction as two angles: the three floats it
  // writes are the rotation's second column.
  void set_shadow_vector(float elevation, float azimuth);

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
  // 0x14. The Buffer set_buffer stores and render draws into - named from
  // the setter that writes it (0x006183F0 `mov dword ptr [ecx + 0x14], eax`
  // with the Buffer * in eax).
  Buffer *buffer_;
  // 0x18..0x5F, THE BLOCK THE ENGINE IS HANDED. Both engine entry points -
  // the draw 0x00639E60 and the camera transform 0x0063A6F0 - take it as
  // their first argument, and both of this class's own call sites pass
  // `this + 0x18` (image 0x61743F `lea eax, [ebp + 0x18]`), so the ten
  // members that used to be carved out one at a time here are one struct:
  // the instance count render stores, the instance and step tables init
  // seeds, the scene offset the draw adjusts, the scene rotation
  // set_scene_rotation fills, and the scale seeded 1.0f. VoxSceneState above
  // is that block, with each offset's Caviar member named on it.
  VoxSceneState scene_state_;  // 0x18, 0x48 bytes
 public:
  // init normalises the light and copies it to the shadow direction;
  // turn_start supplies both vectors directly for the vehicle renderer.
  // RECOVERY_MEMBER: v2 Caviar light_vector_ VOX_Vect 0x60 0x00616DE0
  VOX_Vect light_vector_;
  // RECOVERY_MEMBER: v2 Caviar shadow_vector_ VOX_Vect 0x6C 0x006181D0
  VOX_Vect shadow_vector_;

 private:
  uint32_t field_78_;  // 0x78
  uint32_t field_7C_;  // 0x7C
  uint32_t field_80_;  // 0x80
  uint32_t field_84_;  // 0x84
  uint32_t field_88_;  // 0x88
  uint32_t field_8C_;  // 0x8C
  float field_90_;     // 0x90, seeded 1.0f
  uint32_t field_94_;  // 0x94
  uint32_t field_98_;  // 0x98
  float field_9C_;     // 0x9C, seeded 1.0f
  float field_A0_;     // 0xA0, seeded 1.0f
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
  // 0xA5, 0x63 bytes: the camera and the channel bounds that follow it, which
  // is what the engine's draw is handed as its third argument (the call sites
  // push `this + 0xA5`). CaviarCamera is the first member, so every
  // `camera_view_.camera_` spelling below lands on the offsets set_camera_direct
  // measured.
  VoxCameraBlock camera_view_;
  int32_t field_108_;
  // THE STEP TABLE. Two dwords per drawn instance, copied out of the record's
  // +0x2C entry the instance was built from; 0x10C + 200*8 is 0x74C, so it is
  // exactly 200 long and 0x13CC is the count in use.
  CvrStep instance_steps_[200];  // 0x10C
  // Original construction clears three pointers per entry; close frees
  // those allocations directly, never passing them to model-record cleanup.
  // RECOVERY_MEMBER: v4 Caviar slots_ VoxInstance[200] 0x74C 0x960 0x00616DA0
  VoxInstance slots_[200];      // 0x74C
  // Per-instance colour sources: add_object selects the record's palette
  // after its 0x302-byte header, or the supplied byte buffer. object_start
  // copies 0x1800 bytes from the selected pointer, not from a numeric ID.
  // RECOVERY_MEMBER: v1 Caviar instance_palettes_ 0x10AC 0x00617920
  uint8_t *instance_palettes_[200]; // 0x10AC; 200 Win32 pointers end at 0x13CC
  // RECOVERY_MEMBER: v1 Caviar field_13CC_ 0x13CC 0x004C1D20
  uint32_t field_13CC_;  // 0x13CC
};

static_assert(sizeof(CaviarData) == 0xC,
              "CaviarData layout must match the original executable");
static_assert(sizeof(Caviar) == 0x13D0,
              "Caviar layout must match the original executable");


// The loader and its matching release walk own the CVR record allocations.
CvrRecord *__cdecl vox_load_record(int fd);
void __cdecl caviar_free_record(CvrRecord *record);

// Shared voxel geometry uses the matrices' real three-float row bounds.
void __cdecl caviar_apply_rotation(const float *angles, VOX_Matrix *matrix);
void __cdecl vox_matrix_vector_mul(const float (*)[3], const float *, float *);
void __cdecl vox_matrix_matrix_mul(const float (*)[3], const float (*)[3],
                                  float (*)[3]);
void __cdecl vox_matrix_inverse(const float (*)[3], float (*)[3]);
void __cdecl vox_cross_product(const float *, const float *, float *);
void __cdecl vox_normalise_vector(const float *, float *);

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
    uint32_t zero_b_[2];       // 0x30
    int32_t draw_left_;        // 0x38 - the draw rectangle's left and top,
    int32_t draw_top_;         // 0x3C - both zero until the first draw
    // 0x40..0x4C: the rectangle the last draw covered and its extent. Created
    // as {0xff, 0xff, 0x100, 0x100} - a right/bottom one short of the extent,
    // which is why the body that reads them adds one.
    int32_t draw_right_;       // 0x40
    int32_t draw_bottom_;      // 0x44
    int32_t draw_width_;       // 0x48 - width, or 0 when a side is negative
    int32_t draw_height_;      // 0x4C
    void *colour_table_;       // 0x50
    void *shadow_table_;       // 0x54
    // 0x58, 0x5C. The engine's two per-object callbacks: +0x58 is the one
    // init_class fills with the renderer's own object_start (0x00618DA0), and
    // the draw calls both - +0x58 with the part, +0x5C after it with the step.
    uint32_t object_callback_[2];
    // 0x60, 0x64. The two per-entry ramp pointers. Created zeroed, then
    // owned by sub_63f9b0: it frees each, reallocates count<<2 bytes (count
    // = the descriptor's entry count at setup_block+0x10), and fills ramp A
    // from *(setup_block+4) stepped *(setup_block+0x18)*2 per entry, ramp B
    // as a running accumulation of *(setup_block+0x14) per entry.
    // init_class fills the same slots with its own two 256-entry ramps.
    // The two ramp tables: arrays of shade-row POINTERS (0x100 bytes per
    // shade level - half a scene row). init_class fills them from the scene
    // memory; sub_63f9b0 reallocates and refills them per descriptor count.
    void **ramp_a_ptr_;  // 0x60
    void **ramp_b_ptr_;  // 0x64
    uint8_t setup_size_code_;  // 0x68 - low byte of the last argument
    uint8_t field_69_[0x7C - 0x69];
    // 0x7C. The setup hook the draw calls once per frame, before the walk -
    // the third of the record's three callbacks.
    void *setup_callback_;
};

static_assert(sizeof(VoxRenderRecord) == 0x80,
              "renderer record must be the 0x80 bytes the engine allocates");

VoxRenderRecord *__cdecl vox_create_record(int a1, void *a2, void *a3, void *a4,
                                        int a5);

// 0x0063A6F0, the engine's camera transform - homed in caviar.cpp. Takes the
// renderer's own state block (Caviar::scene_state_, which is what the image's
// call site passes as `this + 0x18`), the step index, and the two vectors the
// union of everything drawn is accumulated into.
void __cdecl vox_transform_camera(VoxSceneState *state, int step,
                                  VOX_Vect *position, VOX_Vect *extent);

// The scene render handle the engine hands back to the renderer's callbacks.
// +0x50 is the colour table the next draw reads, which is what object_start
// fills; the same 0x80-byte block is modelled as VoxRenderRecord just above,
// where the same slot is colour_table_.
struct VOX_ScnRenderHnd {
  uint8_t lead_[0x50];     // +0x00
  void *colour_table_;     // +0x50
};

extern Caviar g_VEHDRAW_CAVIAR;

static_assert(sizeof(VOX_ScnRenderHnd) == 0x54,
              "the render handle must reach its colour table at +0x50");
