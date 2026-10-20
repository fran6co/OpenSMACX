"""Exercise recovered interface callbacks from their actual C++ definitions."""
from hashlib import sha256
from pathlib import Path
import os
import shutil
import struct
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
IMAGE = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
IMAGE_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def mouse_move_body():
    source = (ROOT / "src/maininterface.cpp").read_text()
    start = source.index("void MainInterface::on_iface_mouse_move(int a1, int a2) {")
    end = source.index("\n}\n", start) + 3
    return source[start:end]


def harness(body):
    return r'''
#include <cstring>
struct Spot {
    int result, position, type, calls, seen_x, seen_y;
    Spot() : result(-1), position(0), type(0), calls(0), seen_x(0), seen_y(0) {}
    int check(int x, int y, int *spot_pos, int *spot_type) {
        ++calls; seen_x=x; seen_y=y;
        if (result >= 0) {
            if (spot_pos) *spot_pos=position;
            if (spot_type) *spot_type=type;
        }
        return result;
    }
};
struct MainInterface {
    Spot spot_;
    int iface_redraw_pending_, redraws;
    MainInterface() : iface_redraw_pending_(0), redraws(0) {}
    void redraw_complete() { ++redraws; }
    void on_iface_mouse_move(int, int);
};
struct StatusWindow {
    Spot spot_;
    int hover_base_id_, hover_vehicle_id_, resets, draws;
    int draw_vehicle, draw_base, draw_mode;
    StatusWindow() : hover_base_id_(-1), hover_vehicle_id_(-1), resets(0),
                     draws(0), draw_vehicle(-2), draw_base(-2), draw_mode(-1) {}
    void reset_active() { ++resets; hover_base_id_=-1; hover_vehicle_id_=-1; }
    void draw_active(int vehicle, int base, int mode) {
        ++draws; draw_vehicle=vehicle; draw_base=base; draw_mode=mode;
    }
} ConsoleStatusWin;
''' + body + r'''
int main() {
    MainInterface window;
    // A non-type-5 hit writes position into a2 and type into a1. The original
    // second Spot call still receives the original x/y coordinates.
    window.spot_.result=0; window.spot_.position=17; window.spot_.type=29;
    ConsoleStatusWin.spot_.result=0;
    ConsoleStatusWin.spot_.position=23; ConsoleStatusWin.spot_.type=11;
    window.on_iface_mouse_move(100, 200);
    if (window.spot_.calls!=1 || window.spot_.seen_x!=100 || window.spot_.seen_y!=200) return 1;
    if (ConsoleStatusWin.spot_.calls!=1 || ConsoleStatusWin.spot_.seen_x!=100
        || ConsoleStatusWin.spot_.seen_y!=200) return 2;
    if (ConsoleStatusWin.hover_base_id_!=23 || ConsoleStatusWin.hover_vehicle_id_!=-1
        || ConsoleStatusWin.draws!=1 || ConsoleStatusWin.draw_vehicle!=-1
        || ConsoleStatusWin.draw_base!=23 || ConsoleStatusWin.draw_mode!=0) return 3;
    if (window.redraws!=0 || window.iface_redraw_pending_!=0) return 4;

    // The primary Spot's position 5 returns early and never probes the status Spot.
    window=MainInterface(); ConsoleStatusWin=StatusWindow();
    window.spot_.result=0; window.spot_.position=5; window.spot_.type=29;
    window.on_iface_mouse_move(100, 200);
    if (ConsoleStatusWin.spot_.calls!=0 || window.iface_redraw_pending_!=1
        || window.redraws!=1) return 5;

    // A miss does not write the outputs; the status Spot still sees the inputs.
    window=MainInterface(); ConsoleStatusWin=StatusWindow();
    ConsoleStatusWin.spot_.result=-1;
    window.on_iface_mouse_move(-31, 47);
    if (ConsoleStatusWin.spot_.calls!=1 || ConsoleStatusWin.spot_.seen_x!=-31
        || ConsoleStatusWin.spot_.seen_y!=47 || ConsoleStatusWin.resets!=1) return 6;
    return 0;
}
'''


def compile_and_run(tmp_path, compiler, damage=None):
    body = mouse_move_body()
    if damage == "reuse_mutated_arguments":
        old = "ConsoleStatusWin.spot_.check(original_x, original_y, &a2, &a1)"
        new = "ConsoleStatusWin.spot_.check(a1, a2, &a2, &a1)"
        assert body.count(old) == 1
        body = body.replace(old, new)
    source = tmp_path / "main_interface_mouse_move.cpp"
    source.write_text(harness(body))
    executable = tmp_path / "main_interface_mouse_move"
    built = subprocess.run([compiler, "-std=c++98", str(source), "-o", str(executable)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(executable)], capture_output=True)
    assert (result.returncode == 0) == (damage is None), result.returncode


def test_pinned_mouse_move_retries_original_coordinates():
    image = IMAGE.read_bytes()
    assert sha256(image).hexdigest() == IMAGE_SHA256
    pe_offset = struct.unpack_from("<I", image, 0x3C)[0]
    assert image[pe_offset:pe_offset + 4] == b"PE\0\0"
    coff = pe_offset + 4
    section_count = struct.unpack_from("<H", image, coff + 2)[0]
    optional_size = struct.unpack_from("<H", image, coff + 16)[0]
    optional = coff + 20
    image_base = struct.unpack_from("<I", image, optional + 28)[0]
    section_table = optional + optional_size

    def original_bytes(va, length):
        rva = va - image_base
        for index in range(section_count):
            section = section_table + index * 40
            virtual_size, virtual_address, raw_size, raw_offset = struct.unpack_from(
                "<IIII", image, section + 8)
            extent = max(virtual_size, raw_size)
            if virtual_address <= rva and rva + length <= virtual_address + extent:
                offset = raw_offset + rva - virtual_address
                return image[offset:offset + length]
        raise AssertionError("original VA is outside every PE section")

    # 45D00A prepares the first output slots, then pushes original x/y;
    # 45D058 repeats original x/y registers for the second Spot call.
    first_x = bytes.fromhex("8d4508")
    first_y = bytes.fromhex("8d4d0c")
    first_call = bytes.fromhex("505157538d8e20460200e8ddda1900")
    second_call = bytes.fromhex("8d55088d450c52505753b9486b8c00e894da1900")
    assert original_bytes(0x0045D00A, len(first_x)) == first_x
    assert original_bytes(0x0045D011, len(first_y)) == first_y
    assert original_bytes(0x0045D014, len(first_call)) == first_call
    assert original_bytes(0x0045D058, len(second_call)) == second_call


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None, "reuse_mutated_arguments"))
def test_actual_mouse_move_body_and_mutant(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    compile_and_run(tmp_path, compiler, damage)
