"""Actual-source PCX dimension controls; pinned quirks, not full-game proof."""
from pathlib import Path
import re
import shutil
import subprocess
import pytest
from recovery.tests.test_pact_ascent import definition

ROOT = Path(__file__).resolve().parents[2]

HARNESS = r'''
#include <stdint.h>
#include <string.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#define MAX_PATH 260
#define TRUE 1
static int Language, close_calls, prefix_calls;
static bool opens;
static const char *view;
static std::string events, opened;
@@PREFIX@@
@@HEADER@@
struct Filemap {
 Filemap(){events+='C';} ~Filemap(){events+='D';}
 const char *open_read(const char *p,int){events+='O';opened=p;return opens?view:0;}
 const char *get_view(){events+='V';return view;}
 int get_handle(){events+='H';return 9;}
 void close(){events+='X';++close_calls;}
};
int GetFileSize(int h,void*){if(h!=9)abort();events+='F';return 0;}
size_t observed_strlen(const char *s){if(s==PcxAssetPrefix)++prefix_calls;return strlen(s);}
struct Buffer {static int get_pcx_dimensions(const char*,int*,int*);};
#define strlen observed_strlen
@@BODY@@
#undef strlen
#define CHECK(x) do {if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);return 1;}}while(0)
void reset(){events.clear();opened.clear();opens=true;close_calls=prefix_calls=0;Language=0;}
int main(){
 unsigned char raw[128]={10,0,1,0};
 // Deliberately invalid version and depth remain accepted by this utility.
 raw[4]=1;raw[5]=1;raw[6]=2;raw[7]=2;raw[8]=4;raw[9]=1;raw[10]=4;raw[11]=2;
 view=(char*)raw;int w=-7,h=-8;
 reset();CHECK(Buffer::get_pcx_dimensions(0,&w,&h)==3);CHECK(events=="CD"&&w==-7&&h==-8);
 reset();CHECK(Buffer::get_pcx_dimensions("asset",0,&h)==3);CHECK(events=="CD");
 reset();CHECK(Buffer::get_pcx_dimensions("asset",&w,0)==3);CHECK(events=="CD");
 reset();opens=false;CHECK(Buffer::get_pcx_dimensions("asset",&w,&h)==6);CHECK(events=="COD"&&opened=="asset.pcx");
 const char *suffix[]={".pcx",".pcf",".pcg",".pcx"};
 for(int l=0;l<4;++l){reset();Language=l;CHECK(Buffer::get_pcx_dimensions("asset",&w,&h)==0);CHECK(opened==std::string("asset")+suffix[l]);CHECK(events=="COVHFXD"&&w==4&&h==3&&prefix_calls==1&&close_calls==1);}
 reset();Language=1;CHECK(Buffer::get_pcx_dimensions(".hidden",&w,&h)==0);CHECK(opened==".hidden.pcf");
 reset();CHECK(Buffer::get_pcx_dimensions("dir.ext/asset",&w,&h)==0);CHECK(opened=="dir.ext/asset");
 reset();CHECK(Buffer::get_pcx_dimensions("",&w,&h)==0);CHECK(opened==".pcx");
 char prefixed[sizeof(PcxAssetPrefix)+128];memcpy(prefixed,PcxAssetPrefix,sizeof(PcxAssetPrefix));memcpy(prefixed+sizeof(PcxAssetPrefix),raw,128);view=prefixed;
 reset();CHECK(Buffer::get_pcx_dimensions("asset",&w,&h)==0);CHECK(w==4&&h==3&&prefix_calls==3);
 reset();int both=-1;CHECK(Buffer::get_pcx_dimensions("asset",&both,&both)==0);CHECK(both==3);
 view=(char*)raw;raw[0]=11;reset();w=77;h=88;CHECK(Buffer::get_pcx_dimensions("asset",&w,&h)==3);CHECK(events=="COVHFD"&&w==77&&h==88&&close_calls==0);
 raw[0]=10;raw[2]=2;reset();CHECK(Buffer::get_pcx_dimensions("asset",&w,&h)==3);CHECK(close_calls==0);
 raw[2]=1;raw[8]=0;raw[9]=0;raw[10]=0;raw[11]=0;reset();CHECK(Buffer::get_pcx_dimensions("asset",&w,&h)==0);CHECK(w==-256&&h==-513);
 return 0;
}
'''

@pytest.mark.parametrize('compiler_name',['g++','clang++'])
@pytest.mark.parametrize('damage',['none','cached_prefix','view_after_size','reject_version','close_invalid'])
def test_actual_dimension_paths_and_damage(tmp_path,compiler_name,damage):
    compiler=shutil.which(compiler_name)
    if not compiler: pytest.skip(compiler_name+' unavailable')
    text=(ROOT/'src/buffer.cpp').read_text()
    body=definition(text,r'int\s+Buffer::get_pcx_dimensions\s*\(')
    if damage=='cached_prefix':
        anchor='        (void)strlen(PcxAssetPrefix);'
        assert anchor in body
        body=body.replace(anchor,'',1)
    elif damage=='view_after_size':
        anchor='    const char *content = static_cast<const char *>(map.get_view());\n    GetFileSize(map.get_handle(), 0);'
        assert anchor in body
        body=body.replace(anchor,'    GetFileSize(map.get_handle(), 0);\n    const char *content = static_cast<const char *>(map.get_view());',1)
    elif damage=='reject_version':
        body=body.replace("header.manufacturer == '\\n'", "header.version >= 4 && header.manufacturer == '\\n'",1)
    elif damage=='close_invalid':
        anchor='\n    return 3;\n}'
        assert anchor in body
        body=body.replace(anchor,'\n    map.close();\n    return 3;\n}',1)
    header=re.search(r'struct PcxHeaderView\s*\{.*?\};',text,re.S).group()
    prefix=re.search(r'const char PcxAssetPrefix\[\]\s*=\s*"[^"]*";',text).group()
    code=HARNESS.replace('@@PREFIX@@',prefix).replace('@@HEADER@@',header).replace('@@BODY@@',body)
    cpp=tmp_path/'control.cpp';exe=tmp_path/'control';cpp.write_text(code)
    build=subprocess.run([compiler,'-std=c++17','-O0','-Wall','-Wextra','-Werror',str(cpp),'-o',str(exe)],capture_output=True,text=True)
    assert build.returncode==0,build.stderr
    run=subprocess.run([str(exe)],capture_output=True,text=True)
    assert (run.returncode==0)==(damage=='none'),run.stdout+run.stderr
