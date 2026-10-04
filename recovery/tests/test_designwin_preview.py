"""Compile the actual preview implementation against bounded host controls."""
from pathlib import Path
import os
import shutil
import subprocess

import pytest
from recovery.tests.test_pact_ascent import definition


ROOT = Path(__file__).resolve().parents[2]
HARNESS = r'''
#include <cassert>
#define __cdecl
struct Win { char prefix[0x444]; };
struct Buffer { int marker; };
struct GraphicWin : Win, Buffer {};
struct Prototype { unsigned short flags; unsigned char obsolete_factions; };
struct Vehicle { signed char order; };
Prototype VehPrototypes[512];
Vehicle Vehs[2049];
int WorkshopFaction=1, WorkshopPreviewZoom=-17;
int MapWinCaptionScale=-1, MapWinCaptionDivisor=-1;
int SpriteScaleNumerator=1, SpriteScaleDenominator=1;
int fake_count=0, draw_count=0, fake_faction=-1;
int got_x=0, got_y=0, got_zoom=0, got_flags=0, got_frame=0;
int draw_numerator=0, draw_denominator=0;
Buffer *got_buffer=0;
int veh_fake(int prototype, int faction) {
    assert(prototype==7); ++fake_count; fake_faction=faction;
    WorkshopFaction=3; WorkshopPreviewZoom=99;
    return 2048;
}
int veh_draw(Buffer *buffer, int vehicle, int x, int y, int zoom, int flags, int frame) {
    assert(vehicle==2048); ++draw_count; got_buffer=buffer;
    got_x=x; got_y=y; got_zoom=zoom; got_flags=flags; got_frame=frame;
    draw_numerator=SpriteScaleNumerator;
    draw_denominator=SpriteScaleDenominator;
    return 0;
}
// ACTUAL_PREVIEW
int main() {
    GraphicWin window;
    VehPrototypes[7].flags=1;
    VehPrototypes[7].obsolete_factions=1u<<3;
    hq_list_draw(&window,7,1,10,20,4);
    assert(fake_faction==1 && Vehs[2048].order==1);
    assert(got_buffer==static_cast<Buffer *>(&window));
    assert(static_cast<void *>(got_buffer)!=static_cast<void *>(&window));
    assert(got_zoom==-17 && got_flags==0x2044 && got_frame==0);
    assert(got_x==2 && got_y==24);
    assert(draw_numerator==-1 && draw_denominator==16);
    assert(SpriteScaleNumerator==1 && SpriteScaleDenominator==1);
    WorkshopPreviewZoom=-14;
    hq_list_draw(0,7,0,0,0,0);
    assert(got_buffer==0 && got_y==-4);
    WorkshopPreviewZoom=-6; MapWinCaptionScale=3; MapWinCaptionDivisor=2;
    hq_list_draw(&window,7,1,5,10,120);
    assert(got_x==203 && got_y==14 && got_zoom==-6);
    assert(draw_numerator==3 && draw_denominator==2);
    VehPrototypes[7].flags=0;
    const int before=draw_count;
    SpriteScaleNumerator=9; SpriteScaleDenominator=7;
    hq_list_draw(&window,7,1,0,0,0);
    assert(draw_count==before && fake_count==before);
    assert(SpriteScaleNumerator==9 && SpriteScaleDenominator==7);
}
'''


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_preview(compiler, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"host compiler unavailable: {compiler}")
    source = (ROOT / "src/designwin.cpp").read_text()
    actual = "\n".join(definition(source, signature) for signature in (
        r"static int workshop_signed_bits\(",
        r"static int workshop_scaled_dimension\(",
        r"void __cdecl hq_list_draw\("))
    cpp = tmp_path / "preview.cpp"
    cpp.write_text(HARNESS.replace("// ACTUAL_PREVIEW", actual))
    binary = tmp_path / "preview"
    subprocess.run([executable, "-std=c++03", "-Wall", "-Wextra", "-Werror",
                    "-fsanitize=address,undefined", "-fno-pie", "-no-pie",
                    str(cpp), "-o", str(binary)], check=True, capture_output=True)
    environment = dict(os.environ, ASAN_OPTIONS="detect_leaks=0")
    # LSan cannot run under this sandbox's ptrace setup; ASan/UBSan remain on.
    subprocess.run([str(binary)], env=environment, check=True, capture_output=True)
