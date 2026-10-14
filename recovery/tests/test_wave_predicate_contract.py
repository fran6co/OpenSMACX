"""Wave byte-result contract; host behavior is not a provider ABI proof."""
from pathlib import Path
import shutil
import subprocess
import pytest

from match_recovery.source import read_file
from test_spritelist_base_lifetime import extract_definition

ROOT = Path(__file__).resolve().parents[2]


def test_source_contract_and_original_identity():
    row, = [r for r in read_file(ROOT/'src/wave.cpp') if r.address == 0x4C7250]
    assert row.name == '?is_3d@Wave@@QAEHXZ'
    assert row.symbol == '?is_3d@Wave@@UAE_NXZ'
    assert row.image_spans == ((0x4C7250, 0x4C7262),)
    # Native recording may strengthen the historical semantic claim to exact.
    assert row.semantic or row.byte_exact
    assert 'virtual bool is_3d();' in (ROOT/'src/wave.h').read_text()
    assert 'virtual bool is_3d() = 0;' in (ROOT/'src/voice.h').read_text()


@pytest.mark.parametrize('compiler_name', ('g++', 'clang++'))
@pytest.mark.parametrize('damage', (False, True))
def test_actual_wrapper_null_and_provider_results(compiler_name, damage, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip('host compiler unavailable')
    body = extract_definition((ROOT/'src/wave.cpp').read_text(), 'bool Wave::is_3d() {')
    if damage:
        assert 'return false;' in body
        body = body.replace('return false;', 'return true;', 1)
    program = r'''
#include <cassert>
struct Provider { bool answer; int calls; bool is_3d() { ++calls; return answer; } };
struct Wave { void *device_; bool is_3d(); };
Provider *wave_playback_device(void *p) { return static_cast<Provider *>(p); }
''' + body + r'''
int main() {
  Wave w = {0};
  if (w.is_3d()) return 1;
  Provider p = {false, 0}; w.device_ = &p;
  if (w.is_3d() || p.calls != 1) return 2;
  p.answer = true;
  if (!w.is_3d() || p.calls != 2) return 3;
  return 0;
}
'''
    source = tmp_path/'predicate.cpp'
    source.write_text(program)
    exe = tmp_path/'predicate'
    subprocess.run([compiler, '-std=c++11', str(source), '-o', str(exe)], check=True)
    result = subprocess.run([str(exe)])
    assert result.returncode == (1 if damage else 0)
