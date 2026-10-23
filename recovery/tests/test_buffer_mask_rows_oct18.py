"""Actual copy_mask body control; bounded host behavior is not game acceptance."""

import os
from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def body():
    path = ROOT / "src/buffer_mask.cpp"
    source = path.read_text()
    signature = "int Buffer::copy_mask("
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for offset in range(opening, len(source)):
        if source[offset] == "{":
            depth += 1
        elif source[offset] == "}":
            depth -= 1
            if depth == 0:
                result = source[start : offset + 1]
                break
    else:
        raise AssertionError("unterminated Buffer::copy_mask body")
    rows = {row.address: row for row in read_file(path)}
    row = rows[0x005E2B00]
    assert row.name == "?copy_mask@Buffer@@QAEHPAVBuffer@@HHHHHHH@Z"
    assert row.image_spans == ((0x005E2B00, 0x005E2DD3),)
    assert result in row.region
    return result


def scratch_definitions():
    source = (ROOT / "src/buffer_mask.cpp").read_text()
    result = {}
    for name in (
        "BufferMaskGroupCount",
        "BufferMaskSourceRowGap",
        "BufferMaskDestinationRowGap",
    ):
        marker = f"int {name} = 0;"
        assert source.count(marker) == 1
        result[name] = "static " + marker
    return result


HARNESS = r'''
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <vector>
@GLOBALS@
struct RECT { int left, top, right, bottom; };
bool IntersectRect(RECT *out, const RECT *a, const RECT *b) {
    RECT r = {std::max(a->left,b->left),std::max(a->top,b->top),
              std::min(a->right,b->right),std::min(a->bottom,b->bottom)};
    *out = r;
    return r.left < r.right && r.top < r.bottom;
}
std::vector<int> events;
struct ScratchSnapshot { int groups, source_gap, destination_gap; };
std::vector<ScratchSnapshot> scratch_at_release;
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
    void free_data(int count) {
        events.push_back(id*10+2);
        ScratchSnapshot snapshot = {
            BufferMaskGroupCount, BufferMaskSourceRowGap,
            BufferMaskDestinationRowGap};
        scratch_at_release.push_back(snapshot);
        locks-=count;
    }
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
    scratch_at_release.clear();
    require(source.copy_mask(&dest,sx,sy,dx,dy,w,h,0x108)==0,"valid return");
    require(dest.pixels==expected_dest.pixels,"rectangle pixels and untouched padding");
    require(source.pixels==expected_source.pixels,"source remains read-only");
    require(source.locks==0 && dest.locks==0,"balanced acquisition");
    const int order[]={21,11,12,22};
    require(events==std::vector<int>(order,order+4),"destination acquire, source acquire, source release, destination release");
    int source_x=sx, destination_x=dx, columns=w;
    if(source_x<0) { columns+=source_x; destination_x-=source_x; source_x=0; }
    if(source_x+columns>source.dib_.bmiHeader.biWidth)
        columns=source.dib_.bmiHeader.biWidth-source_x;
    const int clipped_columns=std::min(destination_x+columns,clip.right)-
                              std::max(destination_x,clip.left);
    const int expected_groups=(clipped_columns+3)/4;
    require(scratch_at_release.size()==2,"scratch observed at both releases");
    for(unsigned i=0;i<scratch_at_release.size();++i) {
        require(scratch_at_release[i].groups==expected_groups,
                "scratch group count is ceil clipped width over four");
        require(scratch_at_release[i].source_gap==13-clipped_columns,
                "source scratch gap is stride minus clipped width");
        require(scratch_at_release[i].destination_gap==17-clipped_columns,
                "destination scratch gap is stride minus clipped width");
    }
}
void set_scratch(int groups,int source_gap,int destination_gap) {
    BufferMaskGroupCount=groups;
    BufferMaskSourceRowGap=source_gap;
    BufferMaskDestinationRowGap=destination_gap;
}
void require_scratch(int groups,int source_gap,int destination_gap,const char *label) {
    require(BufferMaskGroupCount==groups && BufferMaskSourceRowGap==source_gap &&
            BufferMaskDestinationRowGap==destination_gap,label);
}
void errors() {
    Buffer source(10,8,13,1),dest(12,10,17,2);
    events.clear(); source.dib_bits_=0;
    require(source.copy_mask(0,0,0,0,0,3,3,8)==7,"missing storage precedes null destination");
    require(events.empty(),"storage failure no acquisition");
    source.surface_=&source;
    require(source.copy_mask(0,0,0,0,0,3,3,8)==3,"null destination");
    require(events.empty(),"null destination no acquisition");
    set_scratch(31,41,59);
    require(source.copy_mask(&dest,11,0,0,0,3,3,8)==0,"source x beyond bounds");
    require(source.copy_mask(&dest,0,9,0,0,3,3,8)==0,"source y beyond bounds");
    require(source.copy_mask(&dest,10,0,0,0,3,3,8)==0,"source x equal bounds empty");
    require(source.copy_mask(&dest,0,8,0,0,3,3,8)==0,"source y equal bounds empty");
    require(source.copy_mask(&dest,0,0,20,20,3,3,8)==0,"destination empty intersection");
    require(events.empty(),"empty rectangles no acquisition");
    require_scratch(31,41,59,"empty/error paths leave poisoned scratch unchanged");
    set_scratch(31,41,59);
    dest.fail=true;
    require(source.copy_mask(&dest,0,0,0,0,3,3,8)==3,"destination acquisition failure");
    require(events.size()==1 && events[0]==21,"destination failure skips source and release");
    require_scratch(31,41,59,"destination acquisition failure leaves scratch unchanged");
    events.clear(); dest.fail=false; source.fail=true;
    require(source.copy_mask(&dest,0,0,0,0,3,3,8)==3,"source acquisition failure");
    require(events.size()==2 && events[0]==21 && events[1]==11,"source failure retains destination acquisition");
    require(dest.locks==1 && source.locks==0,"historical destination acquisition leak");
    require_scratch(31,41,59,"source acquisition failure leaves scratch unchanged");
}
void overlap() {
    Buffer actual(10,8,13,1);
    for(unsigned i=0;i<actual.pixels.size();++i) actual.pixels[i]=8;
    actual.pixels[0]=51; actual.pixels[1]=62; actual.pixels[2]=73;
    Buffer expected=actual;
    reference(expected,expected,0,0,1,1,6,5,0x108);
    events.clear();
    scratch_at_release.clear();
    require(actual.copy_mask(&actual,0,0,1,1,6,5,0x108)==0,"forward overlap return");
    require(actual.pixels==expected.pixels,"forward overlap keeps live source reads");
    const int order[]={11,11,12,12};
    require(events==std::vector<int>(order,order+4) && actual.locks==0,"same-buffer double acquire and release");
    require(scratch_at_release.size()==2 && scratch_at_release[0].groups==2 &&
            scratch_at_release[0].source_gap==7 && scratch_at_release[0].destination_gap==7,
            "overlap scratch values observed at both releases");
}
int main() {
    require_scratch(0,0,0,"three source scratch globals start at zero");
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
        assert "BufferMaskSourceRowGap = read_stride - columns;" in actual
        assert "BufferMaskDestinationRowGap = write_stride - columns;" in actual
        actual = actual.replace("BufferMaskSourceRowGap = read_stride - columns;",
                                "BufferMaskSourceRowGap = read_stride - columns + 1;")
        actual = actual.replace("BufferMaskDestinationRowGap = write_stride - columns;",
                                "BufferMaskDestinationRowGap = write_stride - columns + 1;")
    elif mutation == "reversed":
        assert actual.count("*write_ptr = *read_ptr;") == 4
        actual = actual.replace("*write_ptr = *read_ptr;", "*read_ptr = *write_ptr;")
    elif mutation == "mask":
        assert actual.count("*write_ptr == mask") == 4
        actual = actual.replace("*write_ptr == mask", "*write_ptr != mask")
    cpp = tmp_path / "copy_mask.cpp"
    exe = tmp_path / "copy_mask"
    scratch = scratch_definitions()
    harness = HARNESS.replace("@GLOBALS@", "\n".join(scratch.values()))
    harness = harness.replace("@BODY@", actual)
    cpp.write_text(harness)
    build = subprocess.run([compiler_path,"-std=c++98","-Wall","-Wextra","-Werror",
                            "-Wno-implicit-fallthrough",
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
