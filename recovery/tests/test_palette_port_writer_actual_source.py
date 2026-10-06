from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest

from test_sprite_list_lifecycle_actual_source import extract_definition, repo_root


ROOT = repo_root()


def harness(function: str) -> str:
    return r'''#include <cassert>
#include <vector>
struct PortEvent { int kind; int port; int value; };
static std::vector<PortEvent> events;
static void save_event(int kind, int port, int value) {
    PortEvent event;
    event.kind = kind;
    event.port = port;
    event.value = value;
    events.push_back(event);
}
static void fake_disable() { save_event('D', 0, 0); }
static int fake_outp(int port, int value) {
    save_event('O', port, value);
    return 0;
}
static void fake_enable() { save_event('E', 0, 0); }
#define _disable fake_disable
#define _outp fake_outp
#define _enable fake_enable
''' + function + r'''
#undef _disable
#undef _outp
#undef _enable
int main() {
    unsigned char colors[256 * 4];
    for (int i = 0; i != 256; ++i) {
        colors[i * 4 + 0] = static_cast<unsigned char>(i);
        colors[i * 4 + 1] = static_cast<unsigned char>(255 - i);
        colors[i * 4 + 2] = static_cast<unsigned char>(i * 29 + 11);
        colors[i * 4 + 3] = static_cast<unsigned char>(i ^ 0xA5);
    }
    sub_5d4240(colors);
    assert(events.size() == 771);
    assert(events[0].kind == 'D');
    assert(events[1].kind == 'O' && events[1].port == 0x3c8
           && events[1].value == 0);
    for (int i = 0; i != 256; ++i) {
        for (int channel = 0; channel != 3; ++channel) {
            const PortEvent &e = events[2 + i * 3 + channel];
            assert(e.kind == 'O' && e.port == 0x3c9);
            assert(e.value == (colors[i * 4 + channel] >> 2));
        }
    }
    assert(events[770].kind == 'E');
    return 0;
}
'''


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_installed_source_uses_safe_fake_ports_and_preserves_order(
        compiler_name: str, tmp_path: Path) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is not installed")
    source = (ROOT / "src/general.cpp").read_text()
    function = extract_definition(source, "void __cdecl sub_5d4240(unsigned char *palette_rgb)")
    assert "#pragma intrinsic(_disable, _enable, _outp)" in source
    assert "// ORIGINAL: 0x005D4240 sub_5d4240 0x005D4240-0x005D427C" in source
    assert not (ROOT / "src/recovered/units/005d4240.cpp").exists()

    positive = harness(function)
    mutant_function = function.replace("palette_rgb += 4;", "palette_rgb += 3;")
    assert mutant_function != function
    mutant = harness(mutant_function)
    for label, program in (("positive", positive), ("stride-mutant", mutant)):
        cpp = tmp_path / f"{label}.cpp"
        exe = tmp_path / label
        cpp.write_text(program)
        built = subprocess.run(
            [compiler, "-std=c++98", "-D__cdecl=", str(cpp), "-o", str(exe)],
            capture_output=True, text=True, timeout=30)
        assert built.returncode == 0, built.stdout + built.stderr
        ran = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
        if label == "positive":
            assert ran.returncode == 0, ran.stdout + ran.stderr
        else:
            assert ran.returncode != 0, "stride mutant escaped the input/output oracle"

