"""Host controls extracting the current Caviar setup and raster bodies.

No product body is copied into this test. For staged review, point
OPENSMACX_CAVIAR_SOURCE_ROOT at this packet's staged-src tree. After root
integrates the proposal, the default checkout source is exercised directly.
"""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile


PACKET = Path(__file__).resolve().parent
SOURCE_ROOT = Path(os.environ.get(
    "OPENSMACX_CAVIAR_SOURCE_ROOT",
    next(p for p in PACKET.parents if (p / "src/caviar.cpp").is_file())
)).resolve()
KERNELS = SOURCE_ROOT / "src/caviar_raster_kernels.inc"
PRODUCT = SOURCE_ROOT / "src/caviar.cpp"
FIXTURE = PACKET / "recovery/tests/fixtures/caviar_setup_helpers_actual_source.cpp.in"
if not FIXTURE.is_file():
    FIXTURE = PACKET / "fixtures/caviar_setup_helpers_actual_source.cpp.in"

KERNELS_FUNCTIONS = (
    "static uint32_t caviar_kernel_u32_add(",
    "static int32_t caviar_kernel_i32_from_bits(",
    "static int caviar_kernel_rounding_mode_candidate(",
    "static int32_t caviar_kernel_fistp_dword_candidate(",
    "static int32_t caviar_kernel_sar8(",
    "static void caviar_kernel_project_coordinate(",
    "static int caviar_kernel_clip_coordinate(",
    "static void caviar_kernel_store_lane(",
    "static void caviar_kernel_draw_pixel(",
    "static void caviar_kernel_walk_stream(",
    "static CaviarRasterKernelViewCandidate caviar_make_kernel_view_candidate(",
    "static void caviar_kernel_contained_scalar_candidate(",
    "static void caviar_kernel_clipped_scalar_candidate(",
    "static int caviar_shadow_clip_repair_candidate(",
    "static int caviar_shadow_clip_inside_candidate(",
    "static int caviar_shadow_clip_half_open_candidate(",
    "static void caviar_shadow_kernel_project_candidate(",
    "static void caviar_shadow_kernel_walk_candidate(",
    "static void caviar_shadow_contained_scalar_candidate(",
    "static void caviar_shadow_clipped_scalar_candidate(",
    "static void caviar_part_output_bases_candidate(",
    "static void caviar_shadow_output_bases_candidate(",
    "static const CaviarKernelStepRowCandidate *caviar_kernel_steps_candidate(",
    "static const int32_t *caviar_kernel_row_offsets_candidate(",
    "static void caviar_prepare_part_source_bindings_candidate(",
    "static void caviar_prepare_part_destination_bindings_candidate(",
    "static void caviar_prepare_shadow_bindings_candidate(",
    "static void __cdecl caviar_setup_size1_aligned_candidate(",
    "static void __cdecl caviar_setup_size1_odd_candidate(",
    "static void __cdecl caviar_setup_size2_aligned_candidate(",
    "static void __cdecl caviar_setup_size2_odd_candidate(",
    "static void caviar_part_kernel_candidate(",
    "static void __cdecl caviar_part_s1a_contained_candidate(",
    "static void __cdecl caviar_part_s1a_clipped_candidate(",
    "static void __cdecl caviar_part_s1o_contained_candidate(",
    "static void __cdecl caviar_part_s1o_clipped_candidate(",
    "static void __cdecl caviar_part_s2a_contained_candidate(",
    "static void __cdecl caviar_part_s2a_clipped_candidate(",
    "static void __cdecl caviar_part_s2o_contained_candidate(",
    "static void __cdecl caviar_part_s2o_clipped_candidate(",
    "static void caviar_shadow_kernel_candidate(",
    "static void __cdecl caviar_shadow_clipped_callback_candidate(",
    "static void caviar_install_record_kernels_candidate(",
)
CALLBACKS = (
    "void __cdecl caviar_original_666fd0(", "void __cdecl caviar_original_666a80(",
    "void __cdecl caviar_original_6647f0(", "void __cdecl caviar_original_664000(",
    "void __cdecl caviar_original_665640(", "void __cdecl caviar_original_664e50(",
    "void __cdecl caviar_original_666440(", "void __cdecl caviar_original_665ca0(",
    "void __cdecl caviar_original_667b20(", "void __cdecl caviar_original_667380(",
)
SETUP_CALLBACKS = (
    "void __cdecl caviar_original_63aee0(", "void __cdecl caviar_original_63af00(",
    "void __cdecl caviar_original_63af20(", "void __cdecl caviar_original_63af40(",
)
HELPER_NAMES = (
    "void __cdecl caviar_original_666989(", "void __cdecl caviar_original_66638f(",
    "void __cdecl caviar_original_6646fb(", "void __cdecl caviar_original_664d51(",
    "void __cdecl caviar_original_668069(", "void __cdecl caviar_original_667a6f(",
    "void __cdecl caviar_original_665ba1(", "void __cdecl caviar_original_66554b(",
    "void __cdecl caviar_original_666f68(",
)


def extract_definition(source: str, signature: str) -> str:
    start = source.find(signature)
    assert start >= 0, f"missing actual configured definition {signature!r} in {SOURCE_ROOT}"
    while start >= 0:
        opening = source.find("{", start)
        semicolon = source.find(";", start)
        if opening >= 0 and (semicolon < 0 or opening < semicolon):
            break
        start = source.find(signature, start + len(signature))
    assert start >= 0 and opening >= 0, f"definition lacks body: {signature}"
    depth, state, i = 0, "code", opening
    while i < len(source):
        c, n = source[i], source[i + 1] if i + 1 < len(source) else ""
        if state == "code":
            if c == '"': state = "string"
            elif c == "'": state = "char"
            elif c == "/" and n == "/": state = "line"; i += 1
            elif c == "/" and n == "*": state = "block"; i += 1
            elif c == "{": depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0: return source[start:i + 1]
        elif state in ("string", "char"):
            if c == "\\": i += 1
            elif (state == "string" and c == '"') or (state == "char" and c == "'"): state = "code"
        elif state == "line" and c == "\n": state = "code"
        elif state == "block" and c == "*" and n == "/": state = "code"; i += 1
        i += 1
    raise AssertionError(f"unbalanced actual definition: {signature}")


def assembled(trace=False, mutant=None):
    kernels, product = KERNELS.read_text(), PRODUCT.read_text()
    if mutant == "rampA-for-size1":
        before = "if (view->pixel_size_bytes_ == 1) {\n        // Pinned size-1 path: Ramp-B contributes a word-index row offset;\n        // the descriptor-owned lane base is scaled by the native [ECX*2].\n        depth_at = view->depth_lane_base_[lane] + pixel_index;\n    } else {"
        after = before.replace("view->depth_lane_base_[lane] + pixel_index",
                               "view->depth_rows_[screen_y + lane / 2] + screen_x + lane % 2")
        assert kernels.count(before) == 1
        kernels = kernels.replace(before, after, 1)
    elif mutant == "descriptor-for-size2":
        before = "depth_at = view->depth_rows_[screen_y + lane / 2] +\n            screen_x + lane % 2;"
        after = "depth_at = view->depth_lane_base_[lane] + pixel_index;"
        assert kernels.count(before) == 1
        kernels = kernels.replace(before, after, 1)
    elif mutant == "drop-step-y":
        before = "y_fixed = static_cast<int32_t>(\n        static_cast<uint32_t>(y_fixed) + static_cast<uint32_t>(step.y_));"
        after = "y_fixed = static_cast<int32_t>(static_cast<uint32_t>(y_fixed));"
        assert kernels.count(before) == 1
        kernels = kernels.replace(before, after, 1)

    definitions = [extract_definition(kernels, sig) for sig in KERNELS_FUNCTIONS]
    definitions += [extract_definition(product, sig) for sig in CALLBACKS]
    definitions += [extract_definition(product, sig) for sig in SETUP_CALLBACKS]
    if trace:
        definitions += [
            "static int setup_trace[4]; static int setup_trace_count;\n" +
            "void __cdecl caviar_original_666989(){setup_trace[setup_trace_count++]=1;}\n"
            "void __cdecl caviar_original_66638f(){setup_trace[setup_trace_count++]=2;}\n"
            "void __cdecl caviar_original_6646fb(){setup_trace[setup_trace_count++]=3;}\n"
            "void __cdecl caviar_original_664d51(){setup_trace[setup_trace_count++]=4;}\n"
            "void __cdecl caviar_original_668069(){setup_trace[setup_trace_count++]=5;}\n"
            "void __cdecl caviar_original_667a6f(){setup_trace[setup_trace_count++]=6;}\n"
            "void __cdecl caviar_original_665ba1(){setup_trace[setup_trace_count++]=7;}\n"
            "void __cdecl caviar_original_66554b(){setup_trace[setup_trace_count++]=8;}\n"
            "void __cdecl caviar_original_666f68(){setup_trace[setup_trace_count++]=9;}"
        ]
    else:
        definitions += [extract_definition(product, sig) for sig in HELPER_NAMES]
    template = FIXTURE.read_text()
    return template.replace("/* ACTUAL_CAVIAR_DEFINITIONS */", "\n\n".join(definitions)).replace(
        "/* TRACE_SETUP */", "#define TRACE_SETUP 1" if trace else "")


def compile_run(compiler, source, expected):
    with tempfile.TemporaryDirectory(prefix="caviar-setup-r2-actual-") as folder:
        cpp, exe = Path(folder) / "actual.cpp", Path(folder) / "actual"
        cpp.write_text(source)
        built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra",
                                "-pedantic", str(cpp), "-o", str(exe)],
                               capture_output=True, text=True)
        assert built.returncode == 0, built.stdout + built.stderr
        result = subprocess.run([str(exe)], check=False)
        assert (result.returncode == 0) == expected, (result.returncode, expected)


def test_actual_setup_helpers_and_all_depth_profiles_gcc_clang():
    compilers = [shutil.which(c) for c in ("g++", "clang++")]
    compilers = [c for c in compilers if c]
    assert compilers, "host GCC or Clang is required"
    for compiler in compilers:
        compile_run(compiler, assembled(), True)


def test_setup_order_trace_and_order_mutant():
    compiler = shutil.which("g++") or shutil.which("clang++")
    assert compiler
    compile_run(compiler, assembled(trace=True), True)
    kernels = KERNELS.read_text()
    before = "caviar_original_666989(); caviar_original_66638f(); caviar_original_666f68();"
    after = "caviar_original_66638f(); caviar_original_666989(); caviar_original_666f68();"
    assert kernels.count(before) == 1
    compile_run(compiler, assembled(trace=True).replace(before, after, 1), False)


def test_wrong_depth_mapping_and_missing_step_y_mutants_fail_behaviorally():
    compiler = shutil.which("g++") or shutil.which("clang++")
    assert compiler
    for mutant in ("rampA-for-size1", "descriptor-for-size2", "drop-step-y"):
        compile_run(compiler, assembled(mutant=mutant), False)


if __name__ == "__main__":
    import pytest
    raise SystemExit(pytest.main([__file__]))
