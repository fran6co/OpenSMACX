"""Bounded live vehicle-setup bodies, not renderer, x87, or ABI acceptance.

Original 4BF740 publishes the first Euler triple before its second rotation;
4BF8A1 negates only matrix row two. Its four AH40 tests also take the zero arm
for unordered inputs. Original 4C1B17 snapshots the Console baseline, then
walks 7**4 entries with four cross products followed by four normalizations.
Geometry doubles check that contract; they do not validate Caviar itself.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]


def braced(source, start):
    depth = 0
    for end in range(source.index("{", start), len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated actual body")


def live(name):
    path = ROOT / "src/veh.cpp"
    source = re.sub(r"/\*.*?\*/|//[^\n]*", "", path.read_text(), flags=re.S)
    match, = re.finditer(r"void\s+(?:__cdecl\s+)?" + name + r"\([^;{]*\)\s*\{", source)
    body = braced(source, match.start())
    return body


def types():
    header = (ROOT / "src/caviar.h").read_text()
    return "\n".join(braced(header, header.index("struct " + name + " {")) + ";"
                     for name in ("VOX_Vect", "VOX_Matrix"))


COMMON = r"""
#include <cmath>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <limits>
#define __cdecl
void check(bool ok) { if (!ok) std::exit(91); }
bool near(float a, float b) {
    return (std::isnan(a) && std::isnan(b)) || std::fabs(a-b) < 0.000002f;
}
void vector_is(const float *actual, float x, float y, float z) {
    check(near(actual[0], x) && near(actual[1], y) && near(actual[2], z));
}
// TYPES
"""


CAMERA = r"""
VOX_Vect VehDrawCameraAngles;
VOX_Vect input, input_before, transformed;
VOX_Matrix output;
VOX_Matrix *first_matrix, *last_matrix;
float expected_x, expected_z;
int events, rotations;
int atan_calls;
double atan_arguments[2];
void event(char c) { check("RVRLRM"[events] == c); ++events; }
void tag(VOX_Matrix *m, int serial) {
    for (int r=0; r<3; ++r) for (int c=0; c<3; ++c)
        m->values[r][c] = float(serial*100 + r*3 + c);
}
void tagged(const float (*m)[3], int serial) {
    for (int r=0; r<3; ++r) for (int c=0; c<3; ++c)
        check(m[r][c] == float(serial*100 + r*3 + c));
}
void caviar_apply_rotation(const float *angles, VOX_Matrix *matrix) {
    event('R');
    check(matrix != &output);
    if (rotations == 0) {
        std::uint32_t bits; std::memcpy(&bits, &angles[2], sizeof(bits));
        check(angles[0] == 0 && angles[1] == 0 && bits == 0xBF85F92Du);
        vector_is(VehDrawCameraAngles.values, 51, 52, 53);
        first_matrix = matrix;
    } else if (rotations == 1) {
        check(matrix == first_matrix);
        vector_is(angles, expected_x, 0, 0);
        vector_is(VehDrawCameraAngles.values, expected_x, 0, 0);
    } else {
        check(rotations == 2 && matrix != first_matrix);
        vector_is(angles, 0, 0, expected_z);
        vector_is(VehDrawCameraAngles.values, expected_x, 0, 0);
        last_matrix = matrix;
    }
    tag(matrix, ++rotations);
}
void vox_matrix_vector_mul(const float (*matrix)[3], const float *normal, float *out) {
    event('V'); check(matrix == first_matrix->values && normal == input.values);
    check(out != normal && out != VehDrawCameraAngles.values);
    tagged(matrix, 1);
    for (int i=0; i<3; ++i) out[i] = transformed.values[i];
}
float leaf_006281e0_redirect(const float *xy) {
    event('L');
    vector_is(xy, transformed.values[0], transformed.values[1], 0);
    return std::sqrt(xy[0]*xy[0] + xy[1]*xy[1]);
}
void vox_matrix_matrix_mul(const float (*left)[3], const float (*right)[3], float (*out)[3]) {
    event('M'); check(left == first_matrix->values && right == last_matrix->values);
    check(out == output.values); tagged(left, 2); tagged(right, 3);
    tag(&output, 4);
}
double capture_atan(double argument) {
    check(events == 2 || events == 4);
    const int slot = events == 2 ? 0 : 1;
    check((atan_calls & (1 << slot)) == 0);
    atan_calls |= 1 << slot;
    atan_arguments[slot] = argument;
    return std::atan(argument);
}
// Rebind only the extracted body's calls. The double and oracle std::atan
// calls are outside this macro's lifetime, so neither is accidentally rebound.
#define atan capture_atan
// BODY
#undef atan
void run(float x, float y, float z, float angle_x, float angle_z,
         int expected_atan_calls = 0, double arg_x = 0, double arg_z = 0) {
    input = VOX_Vect{{17, -19, 23}}; input_before = input;
    transformed = VOX_Vect{{x,y,z}};
    VehDrawCameraAngles = VOX_Vect{{51,52,53}};
    expected_x = angle_x; expected_z = angle_z; events = rotations = 0;
    atan_calls = 0;
    compute_camera(&input, &output);
    check(events == 6 && rotations == 3);
    check(atan_calls == expected_atan_calls);
    if (atan_calls & 1) check(atan_arguments[0] == arg_x);
    if (atan_calls & 2) check(atan_arguments[1] == arg_z);
    check(std::memcmp(&input, &input_before, sizeof(input)) == 0);
    vector_is(VehDrawCameraAngles.values, expected_x, 0, 0);
    for (int r=0; r<3; ++r) for (int c=0; c<3; ++c)
        check(output.values[r][c] == (r == 2 ? -1 : 1) * float(400+r*3+c));
}
int main() {
    const float nan = std::numeric_limits<float>::quiet_NaN();
    run(0,0,0,0,0);
    run(0,3,4,0,float(std::atan(0.75)),2,0,0.75);
    run(3,0,0,1.57f,1.57f);
    run(-3,4,-5,float(std::atan(-0.75)),float(std::atan(-1.0)),3,-0.75,-1.0);
    run(-0.0f,-0.0f,9,0,0);
    run(nan,4,5,0,0);
    run(3,nan,5,1.57f,0);
    run(3,4,nan,float(std::atan(0.75)),1.57f,1,0.75);
    run(0,0,nan,0,0);
    // 1/3 distinguishes a double argument from float division rounded before
    // promotion, even when both paths produce the same final float angle.
    run(1,3,0,float(std::atan(1.0/3.0)),1.57f,1,1.0/3.0);
    run(0,1,3,0,float(std::atan(1.0/3.0)),2,0,1.0/3.0);
    return 0;
}
"""


NORMALS = r"""
struct Console { std::uint32_t field_1DD6C_; } ConsoleGlobal;
VOX_Vect VehDrawFacetNormals0[2401], VehDrawFacetNormals1[2401];
VOX_Vect VehDrawFacetNormals2[2401], VehDrawFacetNormals3[2401];
VOX_Vect *tables[] = {VehDrawFacetNormals0, VehDrawFacetNormals1,
                     VehDrawFacetNormals2, VehDrawFacetNormals3};
int phase, entry, baseline;
float *cross_outputs[4];
void heights(int index, float *h) {
    // Original nested-loop counters are 30,60,...,210 with l innermost.
    const int divisors[4] = {343,49,7,1};
    for (int q=0; q<4; ++q) h[q] = float((30*(index/divisors[q]%7+1))/4 - baseline);
}
void vox_cross_product(const float *a, const float *b, float *out) {
    check(entry < 2401 && phase < 4);
    float h[4]; heights(entry,h);
    const float v[4][3] = {{0,50,h[1]}, {-50,0,h[0]},
                           {50,0,h[2]}, {0,-50,h[3]}};
    const int pairs[4][2] = {{0,1},{2,0},{3,2},{1,3}};
    const float *ea = v[pairs[phase][0]], *eb = v[pairs[phase][1]];
    vector_is(a,ea[0],ea[1],ea[2]); vector_is(b,eb[0],eb[1],eb[2]);
    check(a != out && b != out);
    for (int n=0; n<phase; ++n) check(out != cross_outputs[n]);
    cross_outputs[phase] = out;
    out[0] = a[1]*b[2]-a[2]*b[1];
    out[1] = a[2]*b[0]-a[0]*b[2];
    out[2] = a[0]*b[1]-a[1]*b[0];
    ++phase;
    // 4C1B17 captures this once before any geometry callback.
    ConsoleGlobal.field_1DD6C_ = 123;
}
void vox_normalise_vector(const float *in, float *out) {
    check(phase >= 4 && phase < 8 && in == out && out == cross_outputs[phase-4]);
    const float length = std::sqrt(in[0]*in[0]+in[1]*in[1]+in[2]*in[2]);
    for (int n=0; n<3; ++n) out[n] /= length;
    if (++phase == 8) { phase = 0; ++entry; }
}
// BODY
void run(int initial_baseline) {
    baseline = initial_baseline; ConsoleGlobal.field_1DD6C_ = initial_baseline;
    phase = entry = 0;
    for (int t=0; t<4; ++t) for (int i=0; i<2401; ++i)
        for (int c=0; c<3; ++c) tables[t][i].values[c] = -9999;
    vehdraw_compute_facet_normals();
    check(entry == 2401 && phase == 0 && ConsoleGlobal.field_1DD6C_ == 123);
    // Independent analytic normals, not a replay of the source cross calls.
    for (int i=0; i<2401; ++i) {
        float h[4]; heights(i,h);
        const float want[4][3] = {{50*h[0],-50*h[1],2500},
                                  {-50*h[2],-50*h[1],2500},
                                  {-50*h[2],50*h[3],2500},
                                  {50*h[0],50*h[3],2500}};
        for (int t=0; t<4; ++t) {
            const float length = std::sqrt(want[t][0]*want[t][0]+want[t][1]*want[t][1]+6250000);
            vector_is(tables[t][i].values, want[t][0]/length, want[t][1]/length, 2500/length);
        }
    }
}
int main() { run(25); run(8); return 0; }
"""


def run_host(tmp_path, body, harness):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host g++ required")
    source, executable = tmp_path / "setup.cpp", tmp_path / "setup"
    source.write_text(COMMON.replace("// TYPES", types()) + harness.replace("// BODY", body))
    compiled = subprocess.run([compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
        "-fno-sanitize-recover=undefined", str(source), "-o", str(executable)],
        text=True, capture_output=True, timeout=30)
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    return subprocess.run([str(executable)], text=True, capture_output=True, timeout=10)


def camera_body():
    return live("compute_camera")


def normals_body():
    return live("vehdraw_compute_facet_normals")


@pytest.mark.parametrize("address,end", [(0x4BF740, 0x4BF8C1), (0x4C1B10, 0x4C1CF7)])
def test_vehicle_setup_native_identity(address, end):
    records = [record for record in read_file(ROOT / "src/veh.cpp") if record.address == address]
    record, = records
    assert record.image_spans == ((address, end),)


@pytest.mark.parametrize("kind", ["camera", "normals"])
def test_live_vehicle_setup(tmp_path, kind):
    body, harness = (camera_body(), CAMERA) if kind == "camera" else (normals_body(), NORMALS)
    result = run_host(tmp_path, body, harness)
    assert result.returncode == 0, result.stdout + result.stderr


def replace_once(body, old, new):
    assert body.count(old) == 1, old
    return body.replace(old, new)


@pytest.mark.parametrize("kind", [
    "late_publication", "wrong_negated_row", "unordered_ignored", "swapped_matrices",
    "rounded_x_quotient", "rounded_z_quotient",
])
def test_camera_runtime_damage(tmp_path, kind):
    body = camera_body()
    if kind == "late_publication":
        body = replace_once(body, "VehDrawCameraAngles = angles;", "")
        call = "caviar_apply_rotation(angles.values, &second_rotation);"
        body = replace_once(body, call, call + "\n    VehDrawCameraAngles = angles;")
    elif kind == "wrong_negated_row":
        body = replace_once(body, "matrix->values[2][2] = -matrix->values[2][2];",
                            "matrix->values[1][2] = -matrix->values[1][2];")
    elif kind == "unordered_ignored":
        body, count = re.subn(r" \|\| (rotated\.values\[[012]\]|length) != \1", "", body)
        assert count == 4
    elif kind == "rounded_x_quotient":
        body = replace_once(body, "atan((double)rotated.values[0] / rotated.values[1])",
                            "atan((double)(rotated.values[0] / rotated.values[1]))")
    elif kind == "rounded_z_quotient":
        body = replace_once(body, "atan((double)length / rotated.values[2])",
                            "atan((double)(length / rotated.values[2]))")
    else:
        body = replace_once(body,
            "vox_matrix_matrix_mul(first_rotation.values, second_rotation.values, matrix->values);",
            "vox_matrix_matrix_mul(second_rotation.values, first_rotation.values, matrix->values);")
    result = run_host(tmp_path, body, CAMERA)
    assert result.returncode == 91, result.stdout + result.stderr


@pytest.mark.parametrize("kind", [
    "constant_baseline", "reread_baseline", "fractional_quarter", "reversed_cross", "missing_last_entry",
])
def test_normals_runtime_damage(tmp_path, kind):
    body = normals_body()
    if kind == "constant_baseline":
        body = replace_once(body, "(int)ConsoleGlobal.field_1DD6C_", "25")
    elif kind == "reread_baseline":
        assert body.count("- baseline") == 4
        body = body.replace("- baseline", "- (int)ConsoleGlobal.field_1DD6C_")
    elif kind == "fractional_quarter":
        body = replace_once(body, "(i >> 2)", "(i / 4.0f)")
    elif kind == "reversed_cross":
        body = replace_once(body, "vox_cross_product(a.values, b.values, n0.values);",
                            "vox_cross_product(b.values, a.values, n0.values);")
    else:
        assignment = "VehDrawFacetNormals0[index] = n0;"
        body = replace_once(body, assignment, "if (index == 2400) return;\n" + assignment)
    result = run_host(tmp_path, body, NORMALS)
    # Only a compiled, explicit behavioral assertion counts as rejected damage.
    assert result.returncode == 91, result.stdout + result.stderr
