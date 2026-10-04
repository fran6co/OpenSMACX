"""Actual-source rendering and Font lifetime controls, not native/game proof."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import pytest
from recovery.tests.test_pact_ascent import definition

ROOT = Path(__file__).resolve().parents[2]


@pytest.mark.parametrize('name', ['g++', 'clang++'])
def test_actual_design_info_and_scan_damage(name):
    compiler = shutil.which(name)
    if not compiler:
        pytest.skip('host compiler unavailable')
    body = definition((ROOT / 'src/designwin.cpp').read_text(), re.escape('void DesignWin::draw_info('))
    fixture = Path(__file__).with_name('design_info_control.cpp').read_text()
    token = '// @@ACTUAL_DESIGN_INFO@@'
    assert fixture.count(token) == 1
    anchor = 'found_replacement = true;\n                        continue;'
    assert body.count(anchor) == 1
    damaged = body.replace(anchor, 'found_replacement = true;\n                        break;', 1)
    with tempfile.TemporaryDirectory(prefix='osmx-design-info-') as directory:
        cpp, binary = Path(directory) / 'control.cpp', Path(directory) / 'control'
        for index, variant in enumerate((body, damaged)):
            cpp.write_text(fixture.replace(token, variant))
            built = subprocess.run([compiler, '-std=c++98', '-Wall', '-Wextra',
                                    '-Werror', '-pedantic', str(cpp), '-o', str(binary)],
                                   capture_output=True, text=True, timeout=45)
            assert built.returncode == 0, built.stdout + built.stderr
            run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=15)
            assert (run.returncode == 0) == (index == 0), (name, index, run.stdout, run.stderr)
