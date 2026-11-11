
/* ---- source-owned scalar and shadow raster kernels ---- */
// Shared window types and parent globals are defined in caviar.h/caviar.cpp;
// fresh standalone CodeView/COFF resolves their v3 child paths, but not the
// original allocation bounds, lifetime, or final-linked placement.

// Seed provenance: build/recovery/next-game-coverage-oct5/product-closure-overlay-v6/src/caviar_scalar_kernel_candidate.inc
// Include after the Caviar renderer's real storage definitions. This code
// intentionally uses the current source owners (projection cells/output
// window, setup copies, active angle/palette and scene surfaces); the local
// struct is a non-owning invocation view, not another allocation or ABI type.
#include <float.h>
#include <limits.h>
#include <string.h>
#if !defined(_MSC_VER)
#include <fenv.h>
#endif
#include <math.h>


// The part and shadow renderers and their patchers are the _SELFMOD MASM
// modules (src/caviar_part_*.asm, src/caviar_shadow_*.asm); their former
// scalar C++ stand-ins are gone, and so are the invented box/projection
// helpers: the image writes that code out in each draw body.
extern int32_t vox_projection_prepared_;
extern const float CaviarProjectionDirectionScale;
extern const float CaviarProjectionRoundThreshold;
extern const float CaviarProjectionRoundBelow;
extern const float CaviarProjectionRoundAbove;
extern "C" void __cdecl sub_63fa80(const VOX_Matrix *);
void __cdecl caviar_direction_to_projection_index(
    const VOX_Matrix *, const VOX_Vect *, float, const VOX_Matrix *);
void __cdecl sub_6401b0(const int16_t *, const int16_t *, int *);
void __cdecl caviar_build_projection_tables();
void __cdecl vox_direction_to_octant(const VoxRenderRecord *);
void __cdecl vox_inverse_direction_to_octant(const VOX_Matrix *);
