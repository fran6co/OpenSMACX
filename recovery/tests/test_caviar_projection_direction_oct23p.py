"""Actual-source controls for the pinned 0x63FCA0 global-state path."""

from pathlib import Path
import shutil
import subprocess

import pytest

from test_caviar_scene_product_source import extract_definition


HERE = Path(__file__).resolve().parent
ROOT = next(parent for parent in HERE.parents if (parent / "src/caviar.cpp").is_file())
FIXTURE = HERE / "fixtures/caviar_product_extraction.cpp"
SOURCE = ROOT / "src/caviar.cpp"
MARKER = "/* PRODUCT_EXTRACTS */"


def source_prefix():
    fixture = FIXTURE.read_text()
    assert fixture.count(MARKER) == 1
    return fixture.split(MARKER, 1)[0]


def direction_body():
    return extract_definition(
        SOURCE.read_text(), "caviar_direction_to_projection_index")


HARNESS_PREFIX = r"""
static int throw_on_nan_acos = 0;
static int throw_on_nan_atan2 = 0;
static int observed_nan_acos = 0;
static int observed_nan_atan2 = 0;
static double caviar_test_acos(double value) {
    if (projection_event_count < 8) projection_event_order[projection_event_count] = 'A';
    ++projection_event_count;
    if (value != value) {
        observed_nan_acos = 1;
        if (throw_on_nan_acos) throw 71;
    }
    return ::acos(value);
}
static double caviar_test_atan2(double y, double x) {
    if (projection_event_count < 8) projection_event_order[projection_event_count] = 'T';
    ++projection_event_count;
    const double value = ::atan2(y, x);
    if (value != value) {
        observed_nan_atan2 = 1;
        if (throw_on_nan_atan2) throw 72;
    }
    return value;
}
#define acos caviar_test_acos
#define atan2 caviar_test_atan2
"""

HARNESS_SUFFIX = r"""
#undef acos
#undef atan2

static CaviarProjectionAngleRowCandidate test_rows[24];

static void reset_trace() {
    projection_inverse_calls = 0;
    projection_vector_mul_calls = 0;
    projection_event_count = 0;
    projection_stage_at_inverse = -1;
    projection_stage_at_vector_mul[0] = projection_stage_at_vector_mul[1] = -1;
    vox_projection_render_stage_ = 0;
    vox_projection_selected_angle_ = 0;
    projection_override_second_vector = 0;
}

static void require(bool value, const char *message) {
    if (!value) {
        fprintf(stderr, "FAIL: %s\n", message);
        exit(1);
    }
}

static void init_rows() {
    for (int row = 0; row != 24; ++row) {
        for (int byte = 0; byte != 0x860; ++byte)
            test_rows[row][byte] = static_cast<uint8_t>(row ^ byte);
        CaviarProjectionAngleRows[row] =
            reinterpret_cast<const CaviarProjectionAngleRowCandidate *>(
                &test_rows[row]);
    }
}

static void verify_live_output(const VOX_Matrix &part, const VOX_Vect &direction) {
    require(projection_inverse_calls == 1, "inverse call count");
    require(projection_vector_mul_calls == 2, "matrix-vector call count");
    require(projection_event_count >= 3 &&
            projection_event_order[0] == 'I' &&
            projection_event_order[1] == 'M' &&
            projection_event_order[2] == 'M', "inverse/multiply call order");
    require(projection_stage_at_inverse == 0, "stage before inverse");
    require(projection_stage_at_vector_mul[0] == 1 &&
            projection_stage_at_vector_mul[1] == 1 &&
            vox_projection_render_stage_ == 1, "stage publication order");
    require(projection_inverse_source == &part.values[0][0], "inverse input");
    require(projection_inverse_output == &vox_projection_matrix_.values[0][0],
            "inverse output is persistent global matrix");
    require(projection_vector_matrices[1] == &vox_projection_matrix_.values[0][0],
            "second multiply consumes persistent matrix");
    require(projection_vector_outputs[1] ==
            &CaviarProjectionDirectionCandidate.values[0],
            "second multiply writes original persistent output window");
    require(projection_vector_outputs[1] != projection_vector_inputs[1],
            "output is not aliased to transform input");
    for (int row = 0; row != 3; ++row)
        for (int col = 0; col != 3; ++col)
            require(vox_projection_matrix_.values[row][col] == part.values[row][col],
                    "persistent inverse matrix contents");
    for (int col = 0; col != 3; ++col)
        require(CaviarProjectionDirectionCandidate.values[col] ==
                    projection_vector_outputs[1][col],
                "persistent direction output contents");
}

static int expected_index(double value, double scale) {
    return static_cast<int>(0.5 - value * scale);
}

static void finite_case(float x, float y, float z,
                        int expected_row, int expected_offset) {
    VOX_Matrix part = {};
    VOX_Matrix camera = {};
    part.values[0][0] = 2.0f;
    part.values[1][1] = 3.0f;
    part.values[2][2] = 4.0f;
    set_identity(camera);
    VOX_Vect direction = {{x / 2.0f, y / 3.0f, z / 4.0f}};
    CaviarSceneProjectionScratch.projected_scale_ = 1.0f;
    CaviarSceneProjectionScratch.viewport_aspect_ = 1.0f;
    reset_trace();
    caviar_direction_to_projection_index(&part, &direction, 1.0f, &camera);
    verify_live_output(part, direction);

    float final_x = x, final_y = y, final_z = z;
    if (final_z < 0.0f) {
        final_x = -final_x;
        final_y = -final_y;
        final_z = -final_z;
    }
    double polar = final_z < 1.0f ? ::acos(final_z) : 0.0;
    double azimuth = ::atan2(final_y, final_x);
    if (!(azimuth >= 0.0)) azimuth -= -6.283185482025146484375;
    int polar_index = expected_index(polar, -14.64225482940673828125);
    int azimuth_index = expected_index(azimuth, -14.1647891998291015625);
    if (azimuth_index == 0x59) azimuth_index = 0;
    require(polar_index == expected_row, "polar row witness");
    require(0x59 - azimuth_index == expected_offset, "azimuth offset witness");
    require(vox_projection_selected_angle_ == test_rows[expected_row] + expected_offset,
            "selected pointer matches source row and offset");
    require(CaviarProjectionDirectionCandidate.values[0] == final_x &&
            CaviarProjectionDirectionCandidate.values[1] == final_y &&
            CaviarProjectionDirectionCandidate.values[2] == final_z,
            "global transformed vector contents");
}

static void test_finite_paths() {
    init_rows();
    // With positive X: zero azimuth remains zero; positive azimuth is not
    // wrapped; negative azimuth is wrapped by subtracting negative 2*pi.
    finite_case(1.0f, 0.0f, 0.0f, 23, 0x59);
    finite_case(1.0f, 0.25f, 0.0f, 23, 0x59 - 3);
    finite_case(1.0f, -0.25f, 0.0f, 23, 3);
    // Below zero is negated; z==0 is not. At and above one, acos is skipped.
    finite_case(1.0f, 0.25f, -0.25f, 19, 41);
    finite_case(1.0f, 0.0f, 0.5f, 15, 0x59);
    finite_case(1.0f, 0.0f, 1.0f, 0, 0x59);
    finite_case(1.0f, 0.0f, 1.25f, 0, 0x59);
}

static void test_unordered_prefix_paths() {
    init_rows();
    volatile float zero = 0.0f;
    const float nan_value = zero / zero;
    VOX_Matrix part = {};
    VOX_Matrix camera = {};
    set_identity(part);
    set_identity(camera);
    CaviarSceneProjectionScratch.projected_scale_ = 1.0f;
    CaviarSceneProjectionScratch.viewport_aspect_ = 1.0f;

    // NaN z takes the original C0-set vector-negation and acos branches. Throw
    // in acos before the later float-to-int conversion/table lookup.
    VOX_Vect direction = {{1.0f, 0.25f, nan_value}};
    projection_forced_vector[0] = 1.0f;
    projection_forced_vector[1] = 0.25f;
    projection_forced_vector[2] = nan_value;
    projection_override_second_vector = 1;
    reset_trace();
    projection_forced_vector[0] = 1.0f;
    projection_forced_vector[1] = 0.25f;
    projection_forced_vector[2] = nan_value;
    projection_override_second_vector = 1;
    throw_on_nan_acos = 1;
    int caught = 0;
    try { caviar_direction_to_projection_index(&part, &direction, 1.0f, &camera); }
    catch (int value) { caught = value; }
    throw_on_nan_acos = 0;
    require(caught == 71 && observed_nan_acos, "unordered z reaches acos prefix");
    require(CaviarProjectionDirectionCandidate.values[0] == -1.0f &&
            CaviarProjectionDirectionCandidate.values[1] == -0.25f &&
            CaviarProjectionDirectionCandidate.values[2] !=
                CaviarProjectionDirectionCandidate.values[2],
            "unordered z follows original negate branch");

    // A NaN azimuth is observed at atan2 and throws before the subsequent
    // wrapping predicate and unsafe integer/table-index tail.
    direction.values[0] = nan_value;
    direction.values[1] = 0.25f;
    direction.values[2] = 0.5f;
    reset_trace();
    projection_forced_vector[0] = nan_value;
    projection_forced_vector[1] = 0.25f;
    projection_forced_vector[2] = 0.5f;
    projection_override_second_vector = 1;
    throw_on_nan_atan2 = 1;
    caught = 0;
    try { caviar_direction_to_projection_index(&part, &direction, 1.0f, &camera); }
    catch (int value) { caught = value; }
    throw_on_nan_atan2 = 0;
    require(caught == 72 && observed_nan_atan2,
            "unordered azimuth stops at atan2 before integer/table tail");
}

int main() {
    test_finite_paths();
    test_unordered_prefix_paths();
    return 0;
}
"""


def compile_run(tmp_path, compiler, body, name):
    tmp_path.mkdir(parents=True, exist_ok=True)
    prefix = source_prefix()
    translation_unit = prefix + "\n" + HARNESS_PREFIX + body + "\n" + HARNESS_SUFFIX
    source = tmp_path / (name + ".cpp")
    exe = tmp_path / name
    source.write_text(translation_unit)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", str(source), "-o", str(exe)],
        text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=60)
    assert built.returncode == 0, built.stdout
    run = subprocess.run([str(exe)], text=True, stdout=subprocess.PIPE,
                         stderr=subprocess.STDOUT, timeout=30)
    return run


def test_actual_direction_source_and_mutants(tmp_path):
    body = direction_body()
    assert "!(CaviarProjectionDirectionCandidate.values[2] >=" in body
    assert "CaviarProjectionDirectionCandidate.values[2] >=\n          CaviarProjectionAngleLimit" in body
    assert "!(azimuth_angle >= CaviarProjectionRoundThreshold)" in body
    compilers = [shutil.which("g++"), shutil.which("clang++")]
    assert all(compilers), "GCC and Clang are required"

    # Establish the unmodified actual-source witness before running mutants.
    for index, compiler in enumerate(compilers):
        run = compile_run(tmp_path / ("good-" + str(index)), compiler, body,
                          "actual-good")
        assert run.returncode == 0, run.stdout

    mutants = {}
    stack_matrix = body.replace(
        "    vox_matrix_inverse(part_matrix->values, vox_projection_matrix_.values);",
        "    VOX_Matrix local_inverse;\n"
        "    vox_matrix_inverse(part_matrix->values, local_inverse.values);", 1)
    stack_matrix = stack_matrix.replace(
        "vox_matrix_vector_mul(vox_projection_matrix_.values, camera_direction.values,",
        "vox_matrix_vector_mul(local_inverse.values, camera_direction.values,", 1)
    assert stack_matrix != body
    mutants["stack-matrix"] = stack_matrix

    stack_vector = body
    opening = stack_vector.find("{", stack_vector.find("caviar_direction_to_projection_index"))
    stack_vector = (stack_vector[:opening + 1] + "\n    VOX_Vect local_direction;" +
                    stack_vector[opening + 1:])
    stack_vector = stack_vector.replace(
        "CaviarProjectionDirectionCandidate.values", "local_direction.values")
    assert stack_vector != body
    mutants["stack-vector"] = stack_vector

    ordered = body.replace(
        "!(CaviarProjectionDirectionCandidate.values[2] >=\n"
        "          CaviarProjectionRoundThreshold)",
        "CaviarProjectionDirectionCandidate.values[2] <\n"
        "          CaviarProjectionRoundThreshold", 1)
    ordered = ordered.replace(
        "!(CaviarProjectionDirectionCandidate.values[2] >=\n"
        "          CaviarProjectionAngleLimit)",
        "CaviarProjectionDirectionCandidate.values[2] <\n"
        "          CaviarProjectionAngleLimit", 1)
    assert ordered != body
    mutants["ordered-gates"] = ordered

    reversed_wrap = body.replace(
        "!(azimuth_angle >= CaviarProjectionRoundThreshold)",
        "azimuth_angle >= CaviarProjectionRoundThreshold", 1)
    assert reversed_wrap != body
    mutants["reversed-wrap"] = reversed_wrap

    # Every damage control is compiled only after the actual body passed with
    # both host compilers above; each must fail its runtime witness.
    for compiler_index, compiler in enumerate(compilers):
        for name, mutant in mutants.items():
            run = compile_run(tmp_path / ("mutant-" + str(compiler_index) + "-" + name),
                              compiler, mutant, "actual-mutant")
            assert run.returncode != 0, name + " mutant unexpectedly passed"

