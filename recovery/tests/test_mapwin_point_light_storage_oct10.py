"""Product storage declarations with synthetic bytes, not target-ABI proof."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
def test_actual_point_light_arrays(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip('host compiler unavailable')
    source = (ROOT / 'src/mapwin.cpp').read_text()
    definitions = []
    for bit in (1, 2):
        definition, = re.findall(
            rf'static const signed char AltLightBit{bit}\[113\] = \{{.*?\}};',
            source, re.S)
        definitions.append(definition)
        values = [-128, -1, 0, 127] * 28 + [-3]
        (tmp_path / f'terrain_point_light_bit{bit}.inc').write_text(','.join(map(str, values)))
    definition, = re.findall(r'static signed char AltLightBit200\[113\];', source)
    definitions.append(definition)
    cpp = tmp_path / 'storage.cpp'
    cpp.write_text('#include <cassert>\n' + '\n'.join(definitions) + r'''
int main() {
    static_assert(sizeof(AltLightBit1) == 113, "access window");
    static_assert(sizeof(AltLightBit2) == 113, "access window");
    static_assert(sizeof(AltLightBit200) == 113, "access window");
    for (int i = 0; i != 113; ++i) assert(AltLightBit200[i] == 0);
    assert(AltLightBit1[0] == -128 && AltLightBit1[1] == -1);
    assert(AltLightBit2[3] == 127 && AltLightBit2[112] == -3);
    AltLightBit200[112] = -9;
    assert(AltLightBit200[112] == -9);
}
''')
    exe = tmp_path / 'storage'
    subprocess.run([compiler, '-std=c++11', str(cpp), '-o', str(exe)], check=True,
                   capture_output=True, text=True)
    subprocess.run([str(exe)], check=True)
