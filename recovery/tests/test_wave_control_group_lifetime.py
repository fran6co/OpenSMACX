"""Actual mapped group member constructor/destructor and damage controls."""
from pathlib import Path
import shutil
import subprocess
import pytest
from recovery.tests.test_pact_ascent import ROOT, definition


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_wave_group_stores_and_teardown(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    header = (ROOT / "src/wave_device.h").read_text()
    declarations = header[header.index("struct WaveGroupNode {"):header.index(" /*\n  * Wave_Device")]
    source = (ROOT / "src/wave_device.cpp").read_text()
    pieces = [definition(source, r"WaveControlGroup::WaveControlGroup\(\)"),
              definition(source, r"WaveControlGroup::~WaveControlGroup\(\)")]
    mutations = {
        "missing_head": (0, "this->list_.head = 0;", "(void)this->list_.head;"),
        "missing_tail": (0, "this->list_.tail = 0;", "(void)this->list_.tail;"),
        "missing_cursor": (0, "this->list_.cursor = 0;", "(void)this->list_.cursor;"),
        "missing_count": (0, "this->list_.count = 0;", "(void)this->list_.count;"),
        "missing_volume": (0, "this->volume = 0x7f;", "(void)this->volume;"),
        "missing_enabled": (0, "this->enabled = 1;", "(void)this->enabled;"),
        "wrong_enabled": (0, "this->enabled = 1;", "this->enabled = 0;"),
        "clears_cursor": (1, "this->list_.count -= 1;", "this->list_.count -= 1; this->list_.cursor = 0;"),
        "ignores_null_wave": (1, "if (!wave)", "if (false)"),
        "bad_next_backlink": (1, "next->prev = 0;", "(void)next->prev;"),
        "wrong_count_direction": (1, "this->list_.count -= 1;", "this->list_.count += 1;"),
        "deletes_wave": (1, "operator delete(node);", "operator delete(wave);"),
    }
    fixture = Path(__file__).with_name("wave_control_group_lifetime_control.cpp").read_text()
    for name in ("actual", *mutations):
        bodies = list(pieces)
        if name != "actual":
            index, before, after = mutations[name]
            assert bodies[index].count(before) == 1, name
            bodies[index] = bodies[index].replace(before, after, 1)
        text = fixture.replace("// @@DECLARATIONS@@", declarations).replace("// @@BODIES@@", "\n".join(bodies))
        cpp, binary = tmp_path / (name + ".cpp"), tmp_path / name
        cpp.write_text(text)
        build = subprocess.run([compiler, "-std=c++98", "-fsanitize=undefined",
                                "-fno-sanitize-recover=all", str(cpp), "-o", str(binary)],
                               capture_output=True, text=True, timeout=45)
        assert build.returncode == 0, name + build.stderr
        run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
        assert (run.returncode == 0) == (name == "actual"), name + run.stderr
