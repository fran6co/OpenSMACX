"""Host controls for the actual localized Buffer TGA/BMP path bodies."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def actual_body(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated Buffer loader")


@pytest.fixture(scope="module")
def loaders(tmp_path_factory):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source = (ROOT / "src/buffer.cpp").read_text()
    tga = actual_body(source, "int __stdcall Buffer::load_tga(const char * a1")
    bmp = actual_body(source, "int __stdcall Buffer::load_bmp(const char * a1")
    harness = r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#define __stdcall
using HANDLE = void *;
struct Palette {};
uint32_t Language;
const char BufferExtTgf[] = ".tgf", BufferExtTgg[] = ".tgg", BufferExtTga[] = ".tga";
const char BufferExtBmf[] = ".bmf", BufferExtBmg[] = ".bmg", BufferExtBmp[] = ".bmp";
std::string events, opened_path;
bool open_ok;
int file_size_calls;
struct Filemap {
    Filemap() { events += 'C'; }
    ~Filemap() { events += 'D'; }
    void *open_read(const char *path, int) {
        events += 'O'; opened_path = path; return open_ok ? this : nullptr;
    }
    HANDLE get_handle() { return this; }
};
unsigned long GetFileSize(HANDLE, unsigned long *) { events += 'F'; ++file_size_calls; return 12; }
struct Buffer {
    int __stdcall load_tga(const char *, Palette *, int, int);
    int __stdcall load_bmp(const char *, Palette *, int, int);
};
'''
    harness += tga + "\n" + bmp + r'''
void check(bool value, int line) { if (!value) { std::fprintf(stderr,"failed %d\n",line); std::exit(1); } }
#define CHECK(x) check((x),__LINE__)
void reset(int lang, bool succeeds) { Language=lang; open_ok=succeeds; events.clear(); opened_path.clear(); file_size_calls=0; }
int main() {
    Buffer b;
    const char *tga_ext[] = {".tga", ".tgf", ".tgg", ".tga"};
    const char *bmp_ext[] = {".bmp", ".bmf", ".bmg", ".bmp"};
    for (int lang : {0,1,2,7}) {
        reset(lang,true);
        CHECK(b.load_tga("asset",nullptr,0,0)==8);
        CHECK(opened_path==std::string("asset")+tga_ext[lang==7?3:lang]);
        CHECK(events=="COFD" && file_size_calls==1);
        reset(lang,true);
        CHECK(b.load_bmp("asset",nullptr,0,0)==8);
        CHECK(opened_path==std::string("asset")+bmp_ext[lang==7?3:lang]);
        CHECK(events=="COFD" && file_size_calls==1);
    }
    reset(1,true); CHECK(b.load_tga("dir.v1/asset",nullptr,0,0)==8);
    CHECK(opened_path=="dir.v1/asset");
    reset(2,true); CHECK(b.load_bmp("dir.v1/asset",nullptr,0,0)==8);
    CHECK(opened_path=="dir.v1/asset");
    reset(2,true); CHECK(b.load_tga(".hidden",nullptr,0,0)==8);
    CHECK(opened_path==".hidden.tgg");
    reset(1,true); CHECK(b.load_bmp(".hidden",nullptr,0,0)==8);
    CHECK(opened_path==".hidden.bmf");
    reset(0,true); CHECK(b.load_bmp("",nullptr,0,0)==8);
    CHECK(opened_path==".bmp");
    reset(2,true); CHECK(b.load_tga("",nullptr,0,0)==8);
    CHECK(opened_path==".tgg");
    reset(0,true); CHECK(b.load_tga("asset",nullptr,0,0)==8);
    reset(0,false); CHECK(b.load_bmp("asset",nullptr,0,0)==6);
    CHECK(events=="COD" && file_size_calls==0);
    reset(0,false); CHECK(b.load_tga("asset",nullptr,0,0)==6);
    CHECK(events=="COD" && file_size_calls==0);
    reset(0,true); CHECK(b.load_tga(nullptr,nullptr,0,0)==3);
    CHECK(events=="CD" && file_size_calls==0);
}
'''
    work = tmp_path_factory.mktemp("buffer-localized-paths")
    cpp, exe = work / "control.cpp", work / "control"
    cpp.write_text(harness)
    subprocess.run([compiler, "-std=c++17", str(cpp), "-o", str(exe)], check=True)
    return exe


def test_actual_localized_loader_paths_and_unsupported_result(loaders):
    subprocess.run([str(loaders)], check=True)
