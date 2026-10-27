"""Execute the actual panel drawer, including guard/no-mask paths, as void."""
from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.local_variants import _function_body_bytes
from match_recovery.source import read_file

ROOT = Path(__file__).resolve().parents[2]

PRELUDE = r'''
#include <cassert>
#include <vector>
struct RECT { int left, top, right, bottom; };
struct Sprite;
struct BoxSpriteParams;
struct Event { int kind, x, y, w, h; };
std::vector<Event> events;
void emit(int k,int x,int y,int w,int h) { events.push_back(Event{k,x,y,w,h}); }
struct Buffer {
    void box_sprite(RECT *, BoxSpriteParams *);
    int tile(Sprite *,int,int,int x,int y,int w,int h) { emit(2,x,y,w,h); return 77; }
    int tile(Buffer *,int,int,int x,int y,int w,int h) { emit(4,x,y,w,h); return 88; }
    int tile_mask(Buffer *,int,int,int x,int y,int w,int h,int) { emit(3,x,y,w,h); return 99; }
};
struct Sprite {
    int iWidth_, iHeight_;
    unsigned char cTransparentIndex_;
    void draw(Buffer *,int,int x,int y,int,int) { emit(1,x,y,iWidth_,iHeight_); }
};
struct BoxSpriteParams {
    Sprite *corner0_, *corner1_, *corner2_, *corner3_;
    Sprite *edge0_, *edge1_, *edge2_, *edge3_;
    Buffer *mask_;
    int mask_color_;
};
'''
MAIN = r'''
void check(unsigned int i,int k,int x,int y,int w,int h) {
    assert(i < events.size());
    Event e=events[i];
    assert(e.kind==k && e.x==x && e.y==y && e.w==w && e.h==h);
}
int main() {
    Buffer canvas, mask;
    RECT r={10,20,110,100};
    BoxSpriteParams p={};
    canvas.box_sprite(0,&p); canvas.box_sprite(&r,0);
    canvas.box_sprite(&r,&p);
    assert(events.empty());
    Sprite s={2,3,255};
    p.corner0_=p.corner1_=p.corner2_=p.corner3_=&s;
    p.edge0_=p.edge1_=p.edge2_=p.edge3_=&s;
    canvas.box_sprite(&r,&p);
    assert(events.size()==8);
    check(0,1,10,20,2,3); check(1,1,108,20,2,3);
    check(2,1,10,97,2,3); check(3,1,108,97,2,3);
    check(4,2,12,20,96,3); check(5,2,12,97,96,3);
    check(6,2,10,23,2,74); check(7,2,108,23,2,74);
    events.clear(); p.mask_=&mask;
    canvas.box_sprite(&r,&p);
    assert(events.size()==17);
    for (unsigned int i=0;i<8;++i) {
        assert(events[i*2].kind==(i<4?1:2));
        assert(events[i*2+1].kind==3);
        assert(events[i*2].x==events[i*2+1].x);
        assert(events[i*2].y==events[i*2+1].y);
        assert(events[i*2].w==events[i*2+1].w);
        assert(events[i*2].h==events[i*2+1].h);
    }
    check(16,4,12,23,96,74);
}
'''


@pytest.mark.parametrize("mutation", (None, "missing-return", "omit-centre"))
def test_panel_drawing_and_return_contract(tmp_path, mutation):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    row, = (r for r in read_file(ROOT / "src/buffer.cpp") if r.address == 0x005E2DD3)
    lo, hi = _function_body_bytes(row.region, "Buffer::box_sprite")
    body = "void Buffer::box_sprite(RECT *rect, BoxSpriteParams *params) " + row.region.encode()[lo:hi].decode()
    prelude = PRELUDE
    if mutation == "missing-return":
        body = body.replace("void Buffer::box_sprite", "int Buffer::box_sprite")
        prelude = prelude.replace("void box_sprite", "int box_sprite")
    elif mutation == "omit-centre":
        anchor = "tile(params->mask_, cx, cy, cx, cy, w, h);"
        assert body.count(anchor) == 1
        body = body.replace(anchor, "(void)0;")
    source = tmp_path / "panel.cpp"
    source.write_text(prelude + body + MAIN)
    program = tmp_path / "panel"
    built = subprocess.run([compiler, "-std=c++11", "-O0", "-Werror=return-type",
                            str(source), "-o", str(program)], capture_output=True)
    if mutation == "missing-return":
        assert built.returncode != 0 and b"return" in built.stderr
    else:
        assert built.returncode == 0, built.stderr.decode()
        result = subprocess.run([str(program)], capture_output=True)
        assert (result.returncode == 0) == (mutation is None)
