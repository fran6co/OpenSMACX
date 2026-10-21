"""Stable-state host controls for the actual media readiness helper.

These controls verify branches and original load sites. They do not model
cross-thread interleavings, provide atomicity, or prove portable ordering.
"""
from pathlib import Path
import os
import shutil
import subprocess

import pytest
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage


def repository_root():
    for parent in Path(__file__).resolve().parents:
        if (parent / "pyproject.toml").is_file():
            return parent
    raise AssertionError("repository root not found")


ROOT = repository_root()
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def pinned_image():
    configured = os.environ.get("OPENSMACX_IMAGE")
    if configured:
        path = Path(configured)
        if path.is_file():
            return path
        return None
    for parent in (ROOT, *ROOT.parents):
        candidate = parent / ".opensmacx/game/terranx_original.exe"
        if candidate.is_file():
            return candidate
    return None


def readiness_body():
    source = (ROOT / "src/movie_media.cpp").read_text()
    start = source.index("extern \"C\" int __cdecl sub_5cb610(MediaDescriptor *d) {")
    end = source.index("\n}\n", start) + 3
    body = source[start:end]
    assert "const volatile LONG &observed_state = d->state;" in body
    return body


def stop_body():
    source = (ROOT / "src/movie_media.cpp").read_text()
    start = source.index("extern \"C\" int __cdecl sub_5cb660(MediaDescriptor *d) {")
    end = source.index("\n}\n", start) + 3
    return source[start:end]


def harness(body):
    return r'''
#include <stdint.h>
#define __cdecl
typedef int32_t LONG;
struct MediaRuntime { int protocol_gate; };
struct MediaDescriptor { MediaRuntime *runtime; LONG state; };
''' + body + r'''
int main() {
    MediaDescriptor d;
    MediaRuntime runtime;

    d.runtime=0; d.state=3;
    if (sub_5cb610(&d)!=0) return 1;

    d.runtime=&runtime; runtime.protocol_gate=1;
    d.state=6; if (sub_5cb610(&d)!=0) return 2;
    d.state=5; if (sub_5cb610(&d)!=1) return 3;
    d.state=2; if (sub_5cb610(&d)!=1) return 4;
    d.state=3; if (sub_5cb610(&d)!=1) return 5;

    runtime.protocol_gate=0;
    d.state=1; if (sub_5cb610(&d)!=0) return 6;
    d.state=2; if (sub_5cb610(&d)!=1) return 7;
    d.state=3; if (sub_5cb610(&d)!=1) return 8;
    d.state=6; if (sub_5cb610(&d)!=0) return 9;
    return 0;
}
'''


def compile_and_run(tmp_path, compiler, damage=None):
    body = readiness_body()
    mutations = {
        "runtime_gate": ("if (d->runtime != 0)", "if (d->runtime == 0)"),
        "protocol_gate": ("return observed_state != 6;", "return observed_state == 6;"),
        "state_three": ("observed_state == 2 || observed_state == 3",
                        "observed_state == 2"),
    }
    if damage is not None:
        before, after = mutations[damage]
        assert body.count(before) == 1
        body = body.replace(before, after)
    source = tmp_path / "media_state_observations.cpp"
    source.write_text(harness(body))
    executable = tmp_path / "media_state_observations"
    built = subprocess.run([compiler, "-std=c++98", str(source), "-o", str(executable)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(executable)], capture_output=True)
    assert (result.returncode == 0) == (damage is None), result.returncode


def test_pinned_original_helper_has_three_state_load_sites():
    path = pinned_image()
    if path is None:
        pytest.skip("pinned original unavailable; set OPENSMACX_IMAGE")
    image = PeImage(path.read_bytes(), PIN)

    def bytes_at(va, size):
        return image.read(Span(va, va + size))

    assert bytes_at(0x005CB626, 6) == bytes.fromhex("8b9198000000")
    assert bytes_at(0x005CB633, 6) == bytes.fromhex("8b8198000000")
    assert bytes_at(0x005CB63E, 6) == bytes.fromhex("8b8198000000")


def test_pinned_original_stop_helper_snapshots_null_gate_and_wait_targets():
    path = pinned_image()
    if path is None:
        pytest.skip("pinned original unavailable; set OPENSMACX_IMAGE")
    image = PeImage(path.read_bytes(), PIN)

    def bytes_at(va, size):
        return image.read(Span(va, va + size))

    # The gate register starts at zero; only a non-null runtime overwrites it.
    assert bytes_at(0x005CB666, 2) == bytes.fromhex("33ff")
    assert bytes_at(0x005CB668, 6) == bytes.fromhex("8b0185c07406")
    assert bytes_at(0x005CB66E, 6) == bytes.fromhex("8bb8d0030000")
    # The state4 exchange precedes one initial state read; zero gate branches
    # to the independent state5 polling loop rather than using state6.
    assert bytes_at(0x005CB67A, 8) == bytes.fromhex("6a0456ff15009166")
    # The gate check precedes that branch's initial shared-state load.
    assert bytes_at(0x005CB683, 9) == bytes.fromhex("85ff8b06741b83f806")
    assert bytes_at(0x005CB685, 9) == bytes.fromhex("8b06741b83f806742c")
    assert bytes_at(0x005CB6A4, 7) == bytes.fromhex("83f80574118b3d")
    body = stop_body()
    gate_branch = body.index("if (protocol_gate != 0)")
    assert body.index("InterlockedExchange(&d->state, 4);") < gate_branch
    assert body.index("while (observed_state != 6)", gate_branch) > gate_branch
    assert body.index("while (observed_state != 5)", gate_branch) > gate_branch
    assert "LONG state =" not in body


def stop_harness(body):
    return r'''
#include <stdint.h>
#include <stdlib.h>
#define __cdecl
typedef int32_t LONG;
typedef uint32_t DWORD;
struct MediaRuntime { DWORD protocol_gate; };
struct MediaDescriptor { MediaRuntime *runtime; LONG state; };
static int exchange_calls;
static int sleep_calls;
static LONG after_exchange;
static LONG after_sleep;
static LONG *sleep_target;
static MediaRuntime *runtime_to_mutate;
static DWORD gate_after_exchange;
LONG InterlockedExchange(LONG *target, LONG value) {
    LONG old = *target;
    ++exchange_calls;
    *target = value;
    *target = after_exchange;
    sleep_target = target;
    if (runtime_to_mutate != 0) runtime_to_mutate->protocol_gate = gate_after_exchange;
    return old;
}
void Sleep(DWORD milliseconds) {
    if (milliseconds != 10) exit(91);
    if (++sleep_calls > 3) exit(92);
    *sleep_target = after_sleep;
}
''' + body + r'''
static int check_case(MediaRuntime *runtime, LONG first_state, LONG next_state,
                      int mutate_gate, DWORD changed_gate, int expected_sleeps) {
    MediaDescriptor d;
    d.runtime = runtime;
    d.state = 1;
    exchange_calls = 0;
    sleep_calls = 0;
    after_exchange = first_state;
    after_sleep = next_state;
    sleep_target = 0;
    runtime_to_mutate = mutate_gate ? runtime : 0;
    gate_after_exchange = changed_gate;
    if (sub_5cb660(&d) != 0) return 0;
    if (exchange_calls != 1 || sleep_calls != expected_sleeps) return 0;
    if (d.state != (expected_sleeps ? next_state : first_state)) return 0;
    return 1;
}
int main() {
    MediaRuntime runtime;

    // A null runtime leaves the saved gate zero and therefore waits for 5.
    if (!check_case(0, 4, 5, 0, 0, 1)) return 1;
    if (!check_case(0, 5, 5, 0, 0, 0)) return 2;

    // The runtime gate is captured before InterlockedExchange; changing it in
    // the mock exchange must not switch the already-selected wait protocol.
    runtime.protocol_gate = 1;
    if (!check_case(&runtime, 4, 6, 1, 0, 1)) return 3;
    runtime.protocol_gate = 0;
    if (!check_case(&runtime, 4, 5, 1, 1, 1)) return 4;

    runtime.protocol_gate = 1;
    if (!check_case(&runtime, 6, 6, 0, 0, 0)) return 5;
    runtime.protocol_gate = 0;
    if (!check_case(&runtime, 5, 5, 0, 0, 0)) return 6;
    return 0;
}
'''


def stop_mutant(body, damage):
    if damage == "null_wait_target":
        if "DWORD expected = 5;" in body:
            return body.replace("DWORD expected = 5;", "DWORD expected = 6;", 1)
        if "if (protocol_gate != 0) {" in body:
            return body.replace("if (protocol_gate != 0) {", "if (protocol_gate == 0) {", 1)
        # The still-unfixed baseline already has this wrong null-runtime path.
        return body.replace("if (runtime != 0 && runtime->protocol_gate == 0) expected = 5;",
                            "if (runtime != 0 && runtime->protocol_gate != 0) expected = 5;", 1)
    if damage == "cached_state":
        if "state = observed_state;" in body:
            return body.replace("state = observed_state;", "state = state;")
        if "while (observed_state != 6) Sleep(10);" in body:
            return body.replace(
                "while (observed_state != 6) Sleep(10);",
                "LONG state = observed_state; while (state != 6) Sleep(10);", 1)
        return body.replace("while (d->state != (LONG)expected) Sleep(10);",
                            "LONG state = d->state; while (state != (LONG)expected) Sleep(10);", 1)
    raise AssertionError("unknown stop-helper mutant")


def compile_and_run_stop(tmp_path, compiler, damage=None):
    body = stop_body()
    if damage is not None:
        damaged = stop_mutant(body, damage)
        assert damaged != body
        body = damaged
    source = tmp_path / "media_stop_observations.cpp"
    source.write_text(stop_harness(body))
    executable = tmp_path / "media_stop_observations"
    built = subprocess.run([compiler, "-std=c++98", str(source), "-o", str(executable)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(executable)], capture_output=True)
    assert (result.returncode == 0) == (damage is None), result.returncode


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None, "runtime_gate", "protocol_gate", "state_three"))
def test_actual_source_stable_readiness_cases_and_mutants(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    compile_and_run(tmp_path, compiler, damage)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None, "null_wait_target", "cached_state"))
def test_actual_source_stop_wait_contract_and_mutants(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    compile_and_run_stop(tmp_path, compiler, damage)
