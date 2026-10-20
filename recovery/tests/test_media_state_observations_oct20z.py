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


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None, "runtime_gate", "protocol_gate", "state_three"))
def test_actual_source_stable_readiness_cases_and_mutants(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    compile_and_run(tmp_path, compiler, damage)
