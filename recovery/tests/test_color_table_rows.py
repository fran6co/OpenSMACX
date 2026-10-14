"""Actual multi-table Sprite body with bounded raster/helper doubles.

Unit/shrink/expand, nonzero rows, destination255 and signed transparency.
Not real helper/DirectDraw integration, invalid-index proof or game acceptance.
"""
from pathlib import Path
import re
import shutil
import subprocess
import pytest

ROOT = Path(__file__).resolve().parents[2]


def actual_body():
    source = (ROOT / 'src/sprite.cpp').read_text()
    matches = [m for m in re.finditer(
        r'int Sprite::draw_multi_table_dest\([^)]*\)\s*\{', source)
        if m.group().count(',') == 4]
    assert len(matches) == 1
    start = matches[0].start()
    depth = 0
    for end in range(source.index('{', start), len(source)):
        depth += (source[end] == '{') - (source[end] == '}')
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError('unterminated actual Sprite body')


HARNESS = r'''
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "jackal/color_table.h"
struct RECT {int left,top,right,bottom;};
bool IntersectRect(RECT *out, const RECT *left, const RECT *right) {
    RECT r = {std::max(left->left,right->left),std::max(left->top,right->top),
              std::min(left->right,right->right),std::min(left->bottom,right->bottom)};
    *out = r;
    return r.left < r.right && r.top < r.bottom;
}
struct Buffer {
    unsigned int stride_; void *locked_bits_; RECT rect1_; int frees;
    void *get_data() {return locked_bits_;}
    void free_data(int mode) {if(mode!=1) std::exit(80); ++frees;}
};
struct Sprite {
    unsigned char *pcBits_;
    int iSpriteWidth2_,iSpriteWidth_,iSpriteHeight_,iTopOffset_,iLeftOffset_;
    DECLARATION
    void fill_scaling_table() {}
    int *get_clipped_x_table_shrink(RECT*,RECT*,int*,int*,int*,int*);
    int *get_clipped_y_table_shrink(RECT*,RECT*,int*,int*,int*,int*);
    int *get_clipped_x_table_expand(RECT*,RECT*,int*,int*,int*,int*);
    int *get_clipped_y_table_expand(RECT*,RECT*,int*,int*,int*,int*);
};
int SpriteScaleNumerator,SpriteScaleDenominator;
int SpriteCachedScaleNumerator,SpriteCachedScaleDenominator;
int SpriteMinimumScaledWidth=1,SpriteMinimumScaledHeight=1;
float SpriteScalingRatio;
int steps[2]={2,2};
int *shrink(int *ds,int *ss,int *dc,int *sc) {*ds=*ss=0; *dc=2; *sc=4; return steps;}
int *expand(int *ss,int *ds,int *sc,int *dc) {*ss=*ds=0; *sc=2; *dc=4; return steps;}
int *Sprite::get_clipped_x_table_shrink(RECT*,RECT*,int*a,int*b,int*c,int*d) {return shrink(a,b,c,d);}
int *Sprite::get_clipped_y_table_shrink(RECT*,RECT*,int*a,int*b,int*c,int*d) {return shrink(a,b,c,d);}
int *Sprite::get_clipped_x_table_expand(RECT*,RECT*,int*a,int*b,int*c,int*d) {return expand(a,b,c,d);}
int *Sprite::get_clipped_y_table_expand(RECT*,RECT*,int*a,int*b,int*c,int*d) {return expand(a,b,c,d);}
ACTUAL_BODY
int main() {
    ColorTable table[24];
    for(int row=0;row<24;++row) for(int col=0;col<256;++col)
        table[row][col]=static_cast<unsigned char>(row*13+col*7+11);
    for(int mode=0;mode<3;++mode) for(int keycase=0;keycase<2;++keycase) {
        unsigned char pixels[16],output[64],expected[64];
        std::memset(pixels,2,sizeof(pixels));
        for(int i=0;i<64;++i) output[i]=expected[i]=static_cast<unsigned char>(255-i%3);
        const int transparent=keycase ? -1 : 9;
        const unsigned char key=static_cast<unsigned char>(transparent);
        const int width=mode==1 ? 4 : 2;
        const int rendered=mode==2 ? 4 : 2;
        const int step=mode==1 ? 2 : 1;
        pixels[0]=3; pixels[step]=key; pixels[step*width]=7; pixels[step*width+step]=2;
        Sprite sprite={pixels,width,width,width,0,0};
        Buffer buffer={8,output,{0,0,8,8},0};
        SpriteScaleNumerator=mode==2 ? 2 : 1;
        SpriteScaleDenominator=mode==1 ? 2 : 1;
        SpriteCachedScaleNumerator=SpriteCachedScaleDenominator=0;
        SpriteScalingRatio=1;
        for(int y=0;y<rendered;++y) for(int x=0;x<rendered;++x) {
            const int sx=mode==2 ? x/2 : x*step;
            const int sy=mode==2 ? y/2 : y*step;
            const unsigned char source=pixels[sy*width+sx];
            if(source!=key) {
                // Independent arithmetic oracle, not the row-index expression.
                const int at=y*8+x;
                expected[at]=static_cast<unsigned char>(source*13+expected[at]*7+11);
            }
        }
        if(sprite.draw_multi_table_dest(&buffer,transparent,0,0,table)!=1 || buffer.frees!=1)
            return 81;
        if(std::memcmp(output,expected,sizeof(output))) {
            std::fprintf(stderr,"wrong pixels mode=%d key=%d\n",mode,transparent);
            return 82;
        }
    }
    return 0;
}
'''


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
@pytest.mark.parametrize('damage', [None, 'unit_row', 'shrink_row', 'expand_row', 'destination'])
def test_actual_multi_table_paths(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + ' unavailable')
    body = actual_body()
    mutations = {
        'unit_row': ('multi_table[source_row[column]][destination_row[column]]',
                     'multi_table[0][destination_row[column]]'),
        'shrink_row': ('multi_table[*source][destination_row[column]]',
                       'multi_table[0][destination_row[column]]'),
        'expand_row': ('multi_table[pixel_value][target]', 'multi_table[0][target]'),
        'destination': ('multi_table[pixel_value][target]', 'multi_table[pixel_value][0]'),
    }
    if damage:
        old,new=mutations[damage]
        assert body.count(old)==1
        body=body.replace(old,new)
    header=(ROOT/'src/sprite.h').read_text()
    declarations=[m.group() for m in re.finditer(r'int draw_multi_table_dest\([^;]+\);',header)
                  if m.group().count(',')==4]
    assert len(declarations)==1
    harness=HARNESS.replace('DECLARATION',declarations[0]).replace('ACTUAL_BODY',body)
    path=tmp_path/'multi_table.cpp'
    path.write_text(harness)
    binary=tmp_path/'multi_table'
    built=subprocess.run([compiler,'-std=c++98','-Wall','-Wextra',
        '-iquote',str(ROOT/'src'),str(path),'-o',str(binary)],capture_output=True,text=True)
    assert built.returncode==0,built.stdout+built.stderr
    ran=subprocess.run([str(binary)],capture_output=True,text=True)
    assert ran.returncode==(82 if damage else 0),ran.stdout+ran.stderr
