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
class Palette;

// Shared renderer camera scale, independently read from original readonly data.
extern const float CaviarCameraDistance;

struct VOX_Vect {
  float values[3];
};

struct VOX_Matrix {
  float values[3][3];
};

// The voxel engine installs these six callbacks as one contiguous table. The
// table's storage is also the beginning of the renderer's projection/message
// output view below; keep the two views in CaviarOutputMessageWindowCandidate
// rather than inventing independent globals for their overlapping bytes.
struct VoxIoCallbacks {
  void *(__cdecl *allocate)(unsigned long);
  void (__cdecl *release)(void *);
  int (__cdecl *read)(int, char *, long);
  int (__cdecl *write)(int, void *, long);
  long (__cdecl *seek)(int, long, int);
  long (__cdecl *tell)(int);
};

// Independently measured isometric projection views. The row structures keep
// the observed 16-byte strides; the fourth word remains uninterpreted.
struct CaviarProjectionCell {
  int32_t x_;
  int32_t y_;
  int32_t z_;
  uint32_t unobserved_word_;
};

struct CaviarFixedPointProjectionMatrix {
  int16_t word_00_;
  int16_t word_02_;
  int16_t word_04_;
  int16_t word_06_;
  int16_t word_08_;
  int16_t word_0A_;
  int32_t dword_0C_;
  int32_t dword_10_;
  int32_t dword_14_;
};

typedef char CaviarProjectionCell_stride_is_16[
    sizeof(CaviarProjectionCell) == 0x10 ? 1 : -1];

// Shared accessed windows reached by the current projection producer, scalar
// and shadow consumers, and error-message writer. These exact extents describe
// observed source accesses only, not original allocation bounds.
#pragma pack(push, 1)
struct CaviarKernelStepRowCandidate {
  int32_t x_;
  int32_t y_;
  int32_t z_;
  uint32_t unobserved_word_;
};
struct CaviarKernelAliasTailCandidate {
  float scratch_e0_;             // +0x00, 0x9C09E0
  float scratch_e4_;             // +0x04, 0x9C09E4
  float scratch_e8_;             // +0x08, 0x9C09E8
  float scratch_ec_;             // +0x0C, 0x9C09EC
  float scratch_f0_;             // +0x10, 0x9C09F0
  float scratch_f4_;             // +0x14, 0x9C09F4
  uint32_t vox_scene_extent_;     // +0x18, 0x9C09F8; prior local source view
  float scratch_fc_;             // +0x1C, 0x9C09FC
  float scratch_a00_;            // +0x20, 0x9C0A00
  float scratch_a04_;            // +0x24, 0x9C0A04
  uint8_t unknown_18_[0x38];     // +0x28..+0x5F, remains untyped
};
struct CaviarKernelKnownProjectionViewsCandidate {
  CaviarProjectionCell cells_[0x1A];
  CaviarKernelAliasTailCandidate tail_;
};
union CaviarKernelProjectionWindowCandidate {
  CaviarKernelKnownProjectionViewsCandidate current_views_;
  CaviarKernelStepRowCandidate raster_step_rows_[32];
  uint32_t accessed_words_[0x80];
};
struct CaviarOutputMessageWindowCandidate {
  // Preserve the observed callback/projection byte overlap without accessing
  // inactive union members: the prefix ends at the callback seek slot, then
  // the projection's ordinary uint32_t array occupies the same object bytes.
  // Callback getters copy pointer representations from this object via memcpy.
  uint8_t callback_prefix_[offsetof(VoxIoCallbacks, seek)];
  uint32_t projection_words_[0x6B];
  uint8_t unidentified_gap_[0x24];
  char message_[0x100];
};
#pragma pack(pop)
static_assert(sizeof(CaviarKernelStepRowCandidate) == 0x10,
              "raster stepper reads four dwords per row");
static_assert(sizeof(CaviarKernelAliasTailCandidate) == 0x60,
              "projection tail occupies 0x9C09E0..0x9C0A3F");
static_assert(offsetof(CaviarKernelKnownProjectionViewsCandidate, tail_) == 0x1A0,
              "26 projection cells end exactly at 0x9C09E0");
static_assert(offsetof(CaviarKernelAliasTailCandidate, vox_scene_extent_) == 0x18,
              "scene extent field follows six adjacent scratch floats");
static_assert(offsetof(CaviarKernelProjectionWindowCandidate, current_views_) == 0,
              "projection child paths start at the parent base");
static_assert(sizeof(CaviarKernelProjectionWindowCandidate) == 0x200,
              "projection owner covers the observed 0x200-byte access window");
#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(CaviarOutputMessageWindowCandidate) == 0x2E0,
              "Win32 shared callback/projection/message storage is 0x2e0 bytes");
static_assert(offsetof(CaviarOutputMessageWindowCandidate, projection_words_) == 0x10,
              "Win32 projection output begins over the seek slot");
static_assert(offsetof(CaviarOutputMessageWindowCandidate, message_) == 0x1E0,
              "Win32 message buffer remains at the observed 0x9C0D60 address");
#else
static_assert(sizeof(CaviarOutputMessageWindowCandidate) ==
                  offsetof(VoxIoCallbacks, seek) + 0x2D0,
              "host shared storage follows host callback pointer width");
static_assert(offsetof(CaviarOutputMessageWindowCandidate, projection_words_) ==
                  offsetof(VoxIoCallbacks, seek),
              "host projection view follows host callback pointer width");
#endif

extern CaviarKernelProjectionWindowCandidate CaviarKernelProjectionWindow;
extern CaviarOutputMessageWindowCandidate CaviarOutputMessageWindow;
typedef char CaviarFixedPointProjectionMatrix_size_is_24[
    sizeof(CaviarFixedPointProjectionMatrix) == 0x18 ? 1 : -1];


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
  // 0x477fff00 - the outer pair is the float -65535/+65535 clip range.
  float lead_[5];        // 0x00; at Caviar+0xD9
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

// Historical prefix-only qualification: the camera transform
// (0x0063A6F0) reads scale at +8 and six signed shorts at +0x14..+0x1f.
// Raster callers also prove the packed variable-tail accesses below; no
// complete allocation extent is inferred from either access family.
// The historical +0x20 interpretation remains a retained qualification
// and restoration obligation. Raster callers prove the directly following
// packed wire fields, but neither byte_size_ nor these accesses prove the
// allocation's complete extent or variable stream capacity.
#pragma pack(push, 1)
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
  int16_t raster_origin_[3]; // +0x20..+0x25, flat-path source vector
  uint32_t item_count_; // +0x26, observed dword traversal count (unaligned)
  uint8_t item_stream_[1]; // +0x2A, variable payload start; not a capacity
};
#pragma pack(pop)

static_assert(offsetof(CvrShape, raster_origin_) == 0x20,
              "CvrShape raster origin must follow the historical prefix");
static_assert(offsetof(CvrShape, item_count_) == 0x26,
              "CvrShape observed count is an unaligned dword at +0x26");
static_assert(offsetof(CvrShape, item_stream_) == 0x2A,
              "CvrShape variable stream begins at +0x2a");

struct CvrStep {
  uint32_t lead_;      // +0x00
  CvrShape *shape_;    // +0x04, the shape the transform scales this part by
};

struct CaviarData;

// Renderer-owned destinations, not loaded CVR models. Caviar::init allocates
// 1/12/36 bytes per entry; Caviar::close releases all three. The renderer's
// destructor tails close without a second array-destructor traversal.
struct VoxInstance {
  VoxInstance();
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
struct VoxRenderRecord;
typedef VoxRenderRecord VOX_ScnRenderHnd;
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

#pragma pack(push, 1)
struct CaviarSceneContext {
  VoxSceneState state_;       // Caviar+0x18, engine's existing scene prefix
  // RECOVERY_MEMBER: v2 CaviarSceneContext light_vector_ VOX_Vect 0x48 0x00616DE0
  VOX_Vect light_vector_;     // Caviar+0x60
  // RECOVERY_MEMBER: v2 CaviarSceneContext shadow_vector_ VOX_Vect 0x54 0x006181D0
  VOX_Vect shadow_vector_;    // Caviar+0x6C
  float field_78_;            // native scene projection scalar
  float field_7C_;
  float field_80_;
  float field_84_;
  float field_88_;
  float field_8C_;
  float field_90_;
  float field_94_;
  float field_98_;
  float field_9C_;
  float field_A0_;
  uint8_t field_A4_;
};
#pragma pack(pop)
static_assert(sizeof(CaviarSceneContext) == 0x8D,
              "Caviar scene context occupies +0x18 through +0xA4");
static_assert(offsetof(CaviarSceneContext, state_) == 0x00,
              "engine state prefix starts at the context pointer");
static_assert(offsetof(CaviarSceneContext, field_A4_) == 0x8C,
              "scene-state draw byte remains at Caviar+0xA4");

struct VoxCameraBlock {
  CaviarCamera camera_;         // +0x00, Caviar::camera_ at 0xA5
  CaviarChannelBounds bounds_;  // +0x34, Caviar::field_D9_ at 0xD9
};
#pragma pack(pop)

extern CaviarFixedPointProjectionMatrix CaviarFixedPointProjection;
extern float CaviarProjectionScaledMatrix[9];
extern int32_t vox_projection_render_stage_;
extern int32_t vox_projection_prepared_;
extern int32_t vox_projection_octant_mask_;
extern int32_t vox_projection_component_[3];
extern VOX_Matrix vox_projection_matrix_;
extern const float CaviarProjectionDirectionScale;
extern const float CaviarProjectionRoundThreshold;
extern const float CaviarProjectionRoundBelow;
extern const float CaviarProjectionRoundAbove;
extern const float CaviarBoundsRadiusScale;

bool __cdecl vox_part_behind(const VOX_Vect *position, float extent,
                             const VoxCameraBlock *view);
void __cdecl vox_direction_to_octant(const VoxRenderRecord *render_record);
void __cdecl vox_inverse_direction_to_octant(const VOX_Matrix *source);
void __cdecl caviar_build_projection_tables();

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
  // Loaded palette conversion and optional chunk release; names are descriptive.
  int apply_palette(Palette *destination, int unused);
  void release_palette_chunk();
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

// A borrowed view into Caviar's scene setup storage. On the original 32-bit
// target this is 0x24 bytes: 33 bytes are accessed, followed by three bytes of
// padding. Host-width pointer members intentionally make host layout wider.
// This is a source-owned view, not a recovered SDK class or allocation bound.
struct CaviarSceneSetupDescriptorCandidate {
  void *colour_buffer_;        // +0x00
  void *depth_buffer_;         // +0x04
  void *shadow_buffer_;        // +0x08
  int32_t colour_width_;       // +0x0C
  int32_t height_;             // +0x10; also ramp entry count
  int32_t colour_row_step_;    // +0x14
  int32_t depth_row_step_;     // +0x18
  int32_t shadow_row_step_;    // +0x1C
  uint8_t pixel_size_;         // +0x20
  uint8_t padding_[3];         // +0x21..+0x23
};
#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(CaviarSceneSetupDescriptorCandidate) == 0x24,
              "borrowed scene setup view spans 0x24 target bytes");
static_assert(offsetof(CaviarSceneSetupDescriptorCandidate, pixel_size_) == 0x20,
              "pixel-size discriminator remains at setup+0x20");
#endif

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
  // Side-effect-free live reads for the renderer's protected Buffer
  // DIB fields; Caviar is already Buffer's friend.
  static void *render_color_dib_bits();
  static void *render_shadow_dib_bits();

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
  // The image's UNK2: it stores a borrowed colour-table source in the scene
  // record, raises field_4_'s bit 4 - the bit object_start tests to decide
  // where the next draw reads its colour table from - calls render, and drops
  // the bit again. centre_x/centre_y are the centre of the box
  // render builds 0x100 deep around, and render_flags is render's own mask.
  int render_with_colour_table(Buffer *buffer, int centre_x, int centre_y,
                               const void *colour_table, int render_flags);
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
  // ProdPicker::exec writes this existing one-byte state immediately after
  // initialization; keep the byte private while exposing that observed store.
  void set_field_A4(int value) { scene_context_.field_A4_ = static_cast<uint8_t>(value); }
  VOX_Vect &light_vector() { return scene_context_.light_vector_; }
  VOX_Vect &shadow_vector() { return scene_context_.shadow_vector_; }
  // 0x00618DA0. The engine's per-object callback: fills the handle's colour
  // table with 0x1800 bytes chosen by field_4_'s bits - the shared monochrome
  // source when bit 3 is on, the caller's table when bit 4 is on, otherwise
  // the instance table's entry for the callback's instance index.
  // The renderer installs this static __cdecl function in the engine's
  // six-argument callback slot. It has no implicit Caviar receiver; the body
  // obtains the active renderer from CaviarScene.
  static void __cdecl object_start(VOX_ScnRenderHnd *handle, long instance,
                                   VOX_Vect *position, VOX_Matrix *orientation,
                                   int32_t *bounds, VOX_Vect *direction);
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
  // Forward the six typed engine callbacks to the allocator and CRT.
  static void *vx_malloc(unsigned long size);
  static void vx_free(void *block);
  static int vx_read(int fd, char *buffer, long count);
  static int vx_write(int fd, void *buffer, long count);
  static long vx_seek(int fd, long offset, int origin);
  static long vx_tell(int fd);

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
  // The engine's first two arguments both receive this+0x18, and source
  // callers now pass the actual composed owner rather than downcasting its
  // VoxSceneState prefix. The state prefix and its Caviar tail share one
  // lifetime through +0xA4; camera_view_ remains at the pinned +0xA5.
  // RECOVERY_MEMBER: v2 Caviar scene_context_ CaviarSceneContext 0x18 0x006396A0
  CaviarSceneContext scene_context_;  // 0x18..0xA4

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
char __cdecl vox_init_callbacks(const VoxIoCallbacks *callbacks, int flag);

// 0x0063AF60. Fills the scene colour table with `count` entries of `value`
// (0xffff through 0x10000 entries here).
void __cdecl vox_fill_colour_table(void *table, unsigned long value,
                                   unsigned long count);

// 0x006393C0. Creates the scene render record from the descriptor block
// init_class fills at 0x009BB438; the created handle lands at 0x009BB478.
// The renderer record vox_create_record builds. 0x80 bytes are allocated and
// all of them are zeroed. Historical tail-as-padding interpretation is
// superseded by the dispatch/callsite evidence for five nonvirtual callbacks.
// Renderer-owned accumulated bounds. Unsigned cells preserve the original
// modulo arithmetic when consumers convert inclusive ends to exclusive ends.
struct VoxRenderClipBounds {
    uint32_t left_, top_, right_, bottom_, width_, height_;
};
static_assert(sizeof(VoxRenderClipBounds) == 0x18,
              "each renderer clip window contains six dwords");

struct VoxRenderRecord {
    uint32_t setup_id_;        // 0x00 - first argument, stored verbatim
    CaviarSceneSetupDescriptorCandidate *setup_block_; // 0x04 borrowed scene view
    VoxRenderClipBounds primary_clip_;   // +0x08, bounds and extents
    VoxRenderClipBounds secondary_clip_; // +0x20, bounds and extents
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
    // 0x58: six-argument per-instance callback installed by init_class.
    // 0x5C: post-part callback; vox_draw_scene reloads it after vox_draw_part
    // and calls it with this record and the same instance index.
    typedef void (__cdecl *ObjectStartCallback)(VoxRenderRecord *, long,
        VOX_Vect *, VOX_Matrix *, int32_t *, VOX_Vect *);
    typedef void (__cdecl *PostPartCallback)(VoxRenderRecord *, int32_t);
    ObjectStartCallback object_start_callback_; // +0x58
    PostPartCallback post_part_callback_;       // +0x5C
    // +0x60 is an array of depth-row pointers. +0x64 is an array of signed
    // relative row offsets, not pointers. sub_63f9b0 frees/reallocates both
    // arrays and preserves the descriptor's live rereads while filling them.
    uint16_t **ramp_a_ptr_; // 0x60 depth-row pointers
    int32_t *ramp_b_ptr_;   // 0x64 relative row offsets
    uint8_t setup_size_code_;  // 0x68 - low byte of the last argument
    uint8_t field_69_[0x6C - 0x69];
    // Nonvirtual callback data in the original 0x6c..0x7f record tail.
    // Slot identity/signature remains subject to the pending fresh serializer
    // parent/member proof; these types reflect the observed call protocols.
    typedef void (__cdecl *PartContainedCallback)(int32_t, int32_t, int32_t,
        uint32_t, const uint8_t *, int32_t);
    typedef void (__cdecl *PartClippedCallback)(int32_t, int32_t, int32_t,
        const int32_t *, uint32_t, const uint8_t *, int32_t);
    typedef void (__cdecl *ShadowContainedCallback)(int32_t, int32_t,
        uint32_t, const uint8_t *, int32_t);
    typedef void (__cdecl *ShadowClippedCallback)(int32_t, int32_t,
        const int32_t *, uint32_t, const uint8_t *, int32_t);
    typedef void (__cdecl *SetupCallback)(void);
    PartContainedCallback part_contained_;    // +0x6c
    PartClippedCallback part_clipped_;        // +0x70
    ShadowContainedCallback shadow_contained_; // +0x74
    ShadowClippedCallback shadow_clipped_;     // +0x78
    SetupCallback setup_callback_;            // +0x7c
};

#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(VoxRenderRecord) == 0x80,
              "renderer record must be the 0x80 bytes the engine allocates");
static_assert(offsetof(VoxRenderRecord, primary_clip_) == 0x08,
              "primary clip bounds start at +0x08");
static_assert(offsetof(VoxRenderRecord, secondary_clip_) == 0x20,
              "secondary clip bounds start at +0x20");
static_assert(offsetof(VoxRenderRecord, draw_left_) == 0x38 &&
              offsetof(VoxRenderRecord, draw_top_) == 0x3C,
              "draw origin follows the two clip windows");
static_assert(offsetof(VoxRenderRecord, colour_table_) == 0x50,
              "the callback handle's colour table is at +0x50");
static_assert(offsetof(VoxRenderRecord, object_start_callback_) == 0x58,
              "the six-argument callback slot is at +0x58");
static_assert(offsetof(VoxRenderRecord, post_part_callback_) == 0x5C,
              "the post-part callback slot is at +0x5C");
#endif

VoxRenderRecord *__cdecl vox_create_record(
    int setup_id, void *setup_data, void *shadow_data,
    CaviarSceneSetupDescriptorCandidate *setup_block, int size_code);

// 0x0063A6F0, the engine's camera transform - homed in caviar.cpp. Takes the
// renderer's own state block (Caviar::scene_state_, which is what the image's
// call site passes as `this + 0x18`), the step index, and the two vectors the
// union of everything drawn is accumulated into.
void __cdecl vox_transform_camera(VoxSceneState *state, int step,
                                  VOX_Vect *position, VOX_Vect *extent);

extern Caviar g_VEHDRAW_CAVIAR;

static_assert(sizeof(VOX_ScnRenderHnd) == 0x80,
              "the callback handle is the same 0x80-byte renderer record");
