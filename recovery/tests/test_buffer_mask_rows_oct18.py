"""Actual copy_mask body control; bounded host behavior is not game acceptance."""

import os
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def body():
    source = (ROOT / "src/buffer.cpp").read_text()
    start = source.index("int Buffer::copy_mask(")
    return source[start:source.index("\n}", start) + 2]


HARNESS = r'''
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <vector>
struct RECT { int left, top, right, bottom; };
bool IntersectRect(RECT *out, const RECT *a, const RECT *b) {
    RECT r = {std::max(a->left,b->left),std::max(a->top,b->top),
              std::min(a->right,b->right),std::min(a->bottom,b->bottom)};
    *out = r;
    return r.left < r.right && r.top < r.bottom;
}
std::vector<int> events;
void require(bool condition, const char *witness) {
    if (!condition) { std::fprintf(stderr,"WITNESS: %s\n",witness); std::exit(17); }
}
struct Buffer {
    void *dib_bits_, *surface_;
    struct DIB { struct Header { int biWidth,biHeight; } bmiHeader; } dib_;
    RECT rect1_;
    int stride_, id, locks;
    bool fail;
    std::vector<unsigned char> pixels;
    Buffer(int width,int height,int pitch,int identity): dib_bits_(this),
        surface_(0),stride_(pitch),id(identity),locks(0),fail(false),
        pixels(pitch*(height+2),8) {
        dib_.bmiHeader.biWidth=width; dib_.bmiHeader.biHeight=-height;
        rect1_.left=rect1_.top=0; rect1_.right=width; rect1_.bottom=height;
    }
    unsigned char *get_data(int x,int y) {
        events.push_back(id*10+1);
        if(fail) return 0;
        ++locks;
        return &pixels[0]+y*stride_+x;
    }
    void free_data(int count) { events.push_back(id*10+2); locks-=count; }
    int copy_mask(Buffer *,int,int,int,int,int,int,int);
};
@BODY@

// Independent coordinate specification: visit requested source coordinates in
// increasing row/column order, admitting only source bounds and destination clip.
// In-place writes remain live so forward overlap preserves the original scan.
void reference(Buffer &source,Buffer &dest,int sx,int sy,int dx,int dy,
               int width,int height,int color) {
    for(int r=0;r<height;++r) for(int c=0;c<width;++c) {
        const int rx=sx+c, ry=sy+r, wx=dx+c, wy=dy+r;
        if(rx<0 || ry<0 || rx>=source.dib_.bmiHeader.biWidth ||
           ry>=-source.dib_.bmiHeader.biHeight || wx<dest.rect1_.left ||
           wx>=dest.rect1_.right || wy<dest.rect1_.top || wy>=dest.rect1_.bottom)
            continue;
        unsigned char &target=dest.pixels[wy*dest.stride_+wx];
        if(target==static_cast<unsigned char>(color))
            target=source.pixels[ry*source.stride_+rx];
    }
}
void patterned(Buffer &source,Buffer &dest) {
    for(unsigned i=0;i<source.pixels.size();++i)
        source.pixels[i]=static_cast<unsigned char>(20+(i*7)%193);
    for(unsigned i=0;i<dest.pixels.size();++i)
        dest.pixels[i]=(i%5==2)?99:8;
}
void scenario(int sx,int sy,int dx,int dy,int w,int h,RECT clip) {
    Buffer source(10,8,13,1),dest(12,10,17,2);
    patterned(source,dest); dest.rect1_=clip;
    Buffer expected_source=source,expected_dest=dest;
    reference(expected_source,expected_dest,sx,sy,dx,dy,w,h,0x108);
    events.clear();
    require(source.copy_mask(&dest,sx,sy,dx,dy,w,h,0x108)==0,"valid return");
    require(dest.pixels==expected_dest.pixels,"rectangle pixels and untouched padding");
    require(source.pixels==expected_source.pixels,"source remains read-only");
    require(source.locks==0 && dest.locks==0,"balanced acquisition");
    const int order[]={21,11,12,22};
    require(events==std::vector<int>(order,order+4),"destination acquire, source acquire, source release, destination release");
}
void errors() {
    Buffer source(10,8,13,1),dest(12,10,17,2);
    events.clear(); source.dib_bits_=0;
    require(source.copy_mask(0,0,0,0,0,3,3,8)==7,"missing storage precedes null destination");
    require(events.empty(),"storage failure no acquisition");
    source.surface_=&source;
    require(source.copy_mask(0,0,0,0,0,3,3,8)==3,"null destination");
    require(events.empty(),"null destination no acquisition");
    require(source.copy_mask(&dest,11,0,0,0,3,3,8)==0,"source x beyond bounds");
    require(source.copy_mask(&dest,0,9,0,0,3,3,8)==0,"source y beyond bounds");
    require(source.copy_mask(&dest,10,0,0,0,3,3,8)==0,"source x equal bounds empty");
    require(source.copy_mask(&dest,0,8,0,0,3,3,8)==0,"source y equal bounds empty");
    require(source.copy_mask(&dest,0,0,20,20,3,3,8)==0,"destination empty intersection");
    require(events.empty(),"empty rectangles no acquisition");
    dest.fail=true;
    require(source.copy_mask(&dest,0,0,0,0,3,3,8)==3,"destination acquisition failure");
    require(events.size()==1 && events[0]==21,"destination failure skips source and release");
    events.clear(); dest.fail=false; source.fail=true;
    require(source.copy_mask(&dest,0,0,0,0,3,3,8)==3,"source acquisition failure");
    require(events.size()==2 && events[0]==21 && events[1]==11,"source failure retains destination acquisition");
    require(dest.locks==1 && source.locks==0,"historical destination acquisition leak");
}
void overlap() {
    Buffer actual(10,8,13,1);
    for(unsigned i=0;i<actual.pixels.size();++i) actual.pixels[i]=8;
    actual.pixels[0]=51; actual.pixels[1]=62; actual.pixels[2]=73;
    Buffer expected=actual;
    reference(expected,expected,0,0,1,1,6,5,0x108);
    events.clear();
    require(actual.copy_mask(&actual,0,0,1,1,6,5,0x108)==0,"forward overlap return");
    require(actual.pixels==expected.pixels,"forward overlap keeps live source reads");
    const int order[]={11,11,12,12};
    require(events==std::vector<int>(order,order+4) && actual.locks==0,"same-buffer double acquire and release");
}
int main() {
    RECT full={0,0,12,10}, inset={3,3,7,6};
    const int widths[]={1,3,4,5};
    for(unsigned i=0;i<4;++i) scenario(2,1,4,2,widths[i],4,full);
    scenario(0,0,1,1,9,7,inset); // simultaneous clipping on all four edges
    scenario(-2,-1,1,1,8,6,full); // source negative origins shift destination
    scenario(7,6,2,2,8,6,full); // source right/bottom clamp
    scenario(1,1,-2,-1,8,6,full); // destination left/top clip
    errors(); overlap();
    std::puts("copy_mask controls passed");
}
'''


def execute(tmp_path, compiler, mutation=None):
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(f"{compiler} unavailable")
    actual = body()
    if mutation == "gap":
        assert "read_ptr += read_stride;" in actual
        actual = actual.replace("read_ptr += read_stride;", "read_ptr += read_stride - columns;")
        actual = actual.replace("write_ptr += write_stride;", "write_ptr += write_stride - columns;")
    elif mutation == "reversed":
        assert "write_ptr[column] = read_ptr[column];" in actual
        actual = actual.replace("write_ptr[column] = read_ptr[column];", "read_ptr[column] = write_ptr[column];")
    elif mutation == "mask":
        assert "write_ptr[column] == mask" in actual
        actual = actual.replace("write_ptr[column] == mask", "write_ptr[column] != mask")
    cpp = tmp_path / "copy_mask.cpp"
    cpp.write_text(HARNESS.replace("@BODY@", actual))
    exe = tmp_path / "copy_mask"
    build = subprocess.run([compiler_path,"-std=c++98","-Wall","-Wextra","-Werror",
                            str(cpp),"-o",str(exe)], capture_output=True,text=True,timeout=30)
    assert build.returncode == 0, build.stderr
    return subprocess.run([str(exe)],capture_output=True,text=True,timeout=10)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_complete_actual_body(tmp_path, compiler):
    result = execute(tmp_path,compiler)
    assert result.returncode == 0, result.stderr
    assert "copy_mask controls passed" in result.stdout


@pytest.mark.parametrize("mutation", ["gap", "reversed", "mask"])
def test_compiling_damage_controls(tmp_path, mutation):
    result = execute(tmp_path,"g++",mutation)
    assert result.returncode == 17, (result.stdout,result.stderr)
    assert "WITNESS: rectangle pixels and untouched padding" in result.stderr


def test_pinned_complete_original_row_scan():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    configured = os.environ.get("OPENSMACX_IMAGE")
    paths = ([Path(configured)] if configured else []) + [
        parent / ".opensmacx/game/terranx_original.exe" for parent in (ROOT,*ROOT.parents)]
    path = next((path for path in paths if path.is_file()), None)
    if path is None:
        pytest.skip("pinned proprietary image unavailable")
    image = PeImage(path.read_bytes(),PIN)
    start,end = 0x5E2B00,0x5E2DD3
    decoded = list(Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(start,end)),start))
    cursor = start
    for instruction in decoded:
        assert instruction.address == cursor
        cursor += instruction.size
    assert cursor == end
    instructions = {i.address:(i.mnemonic,i.op_str) for i in decoded}
    anchors = {
        0x5E2C71:("call","0x5e33f3"), # destination get_data
        0x5E2C94:("call","0x5e33f3"), # source get_data
        0x5E2CAF:("mov","ecx, dword ptr [eax + 0x4a8]"),
        0x5E2CBB:("mov","eax, dword ptr [edx + 0x4a8]"),
        0x5E2CE4:("sub","ecx, dword ptr [ebp - 0x10]"),
        0x5E2CE7:("mov","dword ptr [0x9b3a84], ecx"),
        0x5E2CF0:("sub","edx, dword ptr [ebp - 0x10]"),
        0x5E2CF3:("mov","dword ptr [0x9b37d8], edx"),
        0x5E2CFB:("mov","esi, dword ptr [ebp - 0xc]"),
        0x5E2CFE:("mov","edi, dword ptr [ebp - 8]"),
        0x5E2D0A:("mov","al, byte ptr [ebp + 0x24]"),
        0x5E2D55:("cmp","byte ptr [edi], al"),
        0x5E2D59:("mov","ah, byte ptr [esi]"),
        0x5E2D5B:("mov","byte ptr [edi], ah"),
        0x5E2DA3:("add","esi, dword ptr [0x9b3a84]"),
        0x5E2DA9:("add","edi, dword ptr [0x9b37d8]"),
        0x5E2DB9:("call","0x5e34a3"), # source free_data first
        0x5E2DC3:("call","0x5e34a3"), # destination second
        0x5E2DD0:("ret","0x20"),
    }
    for address in (0x5E2D5D,0x5E2D67,0x5E2D71,0x5E2D7B,
                    0x5E2D86,0x5E2D8C,0x5E2D92,0x5E2D98):
        anchors[address] = ("inc","esi")
        anchors[address+1] = ("inc","edi")
    for address,expected in anchors.items():
        assert instructions[address] == expected, hex(address)
