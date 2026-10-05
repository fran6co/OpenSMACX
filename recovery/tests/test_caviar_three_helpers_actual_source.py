import pathlib
import shutil
import subprocess
import tempfile


ROOT = pathlib.Path(__file__).resolve().parents[2]
PRODUCT = ROOT / "src/caviar.cpp"

FIXTURE = r'''#include <stdint.h>
#include <math.h>
#ifndef __cdecl
#define __cdecl
#endif
struct VOX_Vect { float values[3]; };
struct VOX_Matrix { float values[3][3]; };
struct CaviarChannelBounds { float lead_[5]; int32_t values_[6]; };
struct VoxCameraBlock { CaviarChannelBounds bounds_; };
struct VoxRenderRecord { int32_t setup_id_; int32_t clip_max_a_x_; };
VOX_Matrix vox_projection_matrix_ = {0};
int32_t vox_projection_component_[3] = {0};
int32_t vox_projection_render_stage_ = 0;
int32_t vox_projection_prepared_ = 0;
int32_t vox_projection_octant_mask_ = 0;
const float CaviarProjectionDirectionScale = 256.0f;
const float CaviarProjectionRoundThreshold = 0.0f;
const float CaviarProjectionRoundBelow = -0.5f;
const float CaviarProjectionRoundAbove = 0.5f;
const float CaviarBoundsRadiusScale = 128.0f;
float vox_lod_cut_ = 1.0f;
void vox_normalise_vector(const float *, float *);
void leaf_006281b0_redirect(const float *, float, float *);
void vox_matrix_inverse(const float (*)[3], float (*)[3]);
'''


def extract_definition(source: str, signature: str) -> str:
    start = source.find(signature)
    assert start >= 0, "actual product definition missing: " + signature
    opening = source.find("{", start)
    assert opening >= 0
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError("unbalanced product definition: " + signature)


def run_fixture(compiler: str, source_text: str, expected: bool):
    body = "\n\n".join(
        extract_definition(source_text, signature)
        for signature in (
            "bool __cdecl vox_part_behind(",
            "void __cdecl vox_direction_to_octant(",
            "void __cdecl vox_inverse_direction_to_octant(",
        )
    )
    main = r'''
void vox_normalise_vector(const float *s, float *o) {
    const float n = (float)sqrt(s[0]*s[0] + s[1]*s[1] + s[2]*s[2]);
    o[0] = s[0]/n; o[1] = s[1]/n; o[2] = s[2]/n;
}
void leaf_006281b0_redirect(const float *s, float k, float *o) {
    o[0]=s[0]*k; o[1]=s[1]*k; o[2]=s[2]*k;
}
void vox_matrix_inverse(const float (*s)[3], float (*o)[3]) {
    for (int r=0; r<3; ++r) for (int c=0; c<3; ++c) o[r][c]=s[r][c];
}
''' + r'''
int main() {
    VoxCameraBlock view = {};
    view.bounds_.lead_[3] = -10.0f;
    view.bounds_.lead_[4] = 10.0f;
    view.bounds_.values_[4] = 12;
    view.bounds_.values_[5] = 13;
    VOX_Vect p = {{0.0f, 0.0f, 0.0f}};
    if (vox_part_behind(&p, 0.0f, &view)) return 1;
    p.values[2] = -10.0f;
    if (vox_part_behind(&p, 0.0f, &view)) return 2;
    p.values[2] = 10.01f;
    if (!vox_part_behind(&p, 0.0f, &view)) return 3;
    p.values[2] = 0.0f; p.values[0] = 12.01f;
    if (!vox_part_behind(&p, 0.0f, &view)) return 4;
    p.values[0] = 0.0f; p.values[1] = 13.01f;
    if (!vox_part_behind(&p, 0.0f, &view)) return 5;

    VoxRenderRecord rec = {0};
    vox_projection_matrix_.values[0][0]=1.0f;
    vox_projection_matrix_.values[1][1]=1.0f;
    vox_projection_matrix_.values[2][2]=1.0f;
    vox_direction_to_octant(&rec);
    if (vox_projection_component_[2] != 256 ||
        vox_projection_prepared_ != 1 ||
        vox_projection_octant_mask_ != (1 << 9)) return 6;
    rec.setup_id_ = 1;
    vox_direction_to_octant(&rec);
    if (vox_projection_component_[2] != -256 ||
        vox_projection_octant_mask_ != (1 << 25)) return 7;
    vox_projection_matrix_.values[0][2] = 1.0f;
    vox_projection_matrix_.values[1][2] = 2.0f;
    vox_projection_matrix_.values[2][2] = 3.0f;
    rec.setup_id_ = 0;
    vox_direction_to_octant(&rec);
    if (vox_projection_component_[0] != 68 ||
        vox_projection_component_[1] != 137 ||
        vox_projection_component_[2] != 205 ||
        vox_projection_octant_mask_ != (1 << 1)) return 8;
    rec.setup_id_ = 0;
    VOX_Matrix input = {};
    input.values[0][0]=1.0f; input.values[1][1]=1.0f;
    input.values[0][2]=-1.0f; input.values[1][2]=2.0f;
    input.values[2][2]=-3.0f;
    vox_inverse_direction_to_octant(&input);
    if (vox_projection_render_stage_ != 1 ||
        vox_projection_prepared_ != 1 ||
        vox_projection_component_[0] != -68 ||
        vox_projection_component_[1] != 137 ||
        vox_projection_component_[2] != -205 ||
        vox_projection_octant_mask_ != (1 << 21)) return 9;
    return 0;
}
'''
    text = FIXTURE + "\n" + body + "\n" + main
    with tempfile.TemporaryDirectory() as directory:
        cpp = pathlib.Path(directory) / "actual.cpp"
        exe = pathlib.Path(directory) / "actual"
        cpp.write_text(text)
        subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-pedantic",
             str(cpp), "-o", str(exe)],
            check=True,
        )
        result = subprocess.run([str(exe)], check=False)
        assert (result.returncode == 0) == expected


def test_actual_product_bodies_with_gcc_and_clang():
    source = PRODUCT.read_text()
    compilers = [shutil.which("g++"), shutil.which("clang++")]
    assert all(compilers), "both GCC and Clang are required"
    for compiler in compilers:
        run_fixture(compiler, source, True)


def test_mutant_clip_boundary_is_rejected():
    source = PRODUCT.read_text()
    original = extract_definition(source, "bool __cdecl vox_part_behind(")
    mutant = original.replace("position->values[2] < clip.lead_[3]",
                              "position->values[2] <= clip.lead_[3]", 1)
    assert mutant != original
    mutated_source = source.replace(original, mutant, 1)
    compiler = shutil.which("g++")
    assert compiler
    run_fixture(compiler, mutated_source, False)


def test_mutant_flag_at_clip_offset_is_rejected():
    source = PRODUCT.read_text()
    original = extract_definition(source, "void __cdecl vox_direction_to_octant(")
    mutant = original.replace("render_record->setup_id_ & 1",
                              "render_record->clip_max_a_x_ & 1", 1)
    assert mutant != original
    run_fixture(shutil.which("g++"), source.replace(original, mutant, 1), False)


def test_mutant_raw_yz_instead_of_scaled_values_is_rejected():
    source = PRODUCT.read_text()
    original = extract_definition(source, "void __cdecl vox_direction_to_octant(")
    mutant = original.replace("scaled[1]", "vector[1]").replace(
        "scaled[2]", "vector[2]")
    assert mutant != original
    run_fixture(shutil.which("g++"), source.replace(original, mutant, 1), False)
