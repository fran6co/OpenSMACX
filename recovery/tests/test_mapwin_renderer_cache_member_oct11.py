"""Bounded renderer cache read controls, not complete renderer/game proof."""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_mapwin_cache import braced

ROOT = Path(__file__).resolve().parents[2]
BASE = "fc9d7909ee8fde8fcb93e454e65d57ed36587b43"
OLD = """                                delta = reinterpret_cast<char *>(
                                            window->owned_)
                                    [((MapLongitudeBounds >> 1) * a8 +
                                      local_14) *
                                         0xc +
                                     4 + corner];"""
NEW = """                                delta = window->owned_
                                    [(MapLongitudeBounds >> 1) * a8 + local_14]
                                        .altitude[corner];"""


def test_only_supported_member_read_changes():
    path = ROOT / "src/mapwin.cpp"
    old = subprocess.check_output(["git", "show", BASE + ":src/mapwin.cpp"],
                                  cwd=ROOT, text=True)
    current = path.read_text()
    assert old.count(OLD) == current.count(NEW) == 1
    assert old.replace(OLD, NEW) == current


def test_pinned_cache_reload_stride_and_signed_byte():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("pinned executable unavailable")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    rows = {i.address: (i.mnemonic, i.op_str) for i in
            Cs(CS_ARCH_X86, CS_MODE_32).disasm(
                image.read(Span(0x466B42, 0x466B80)), 0x466B42)}
    for address, expected in {
        0x466B4E: ("call", "0x46fe70"),
        0x466B56: ("mov", "ecx, dword ptr [eax + 4]"),
        0x466B59: ("test", "ecx, ecx"),
        0x466B61: ("mov", "eax, dword ptr [0x949870]"),
        0x466B69: ("sar", "eax, 1"),
        0x466B6B: ("imul", "eax, dword ptr [ebp + 0x24]"),
        0x466B6F: ("add", "eax, edx"),
        0x466B71: ("lea", "edx, [eax + eax*2]"),
        0x466B76: ("lea", "ecx, [ecx + edx*4]"),
        0x466B79: ("mov", "al, byte ptr [ecx + ebx + 4]"),
        0x466B7D: ("movsx", "eax, al"),
    }.items():
        assert rows[address] == expected


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_cache_read_reloads_and_sign_extends(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + " unavailable")
    text = (ROOT / "src/mapwin.cpp").read_text()
    # Retain the complete call, post-call null guard and both result paths.
    start = text.index("                            window->get_alt(a7, a8, corner);")
    end = text.index("\n                        }", start)
    block = text[start:end]
    pod = braced((ROOT / "src/mapwin.h").read_text(), "struct TerrainCacheEntry") + ";"
    fixture = r'''
#include <cassert>
#include <cstddef>
#include <initializer_list>
POD
struct Window {
    TerrainCacheEntry *owned_, *next;
    int calls;
    int get_alt(int,int,int) { ++calls; owned_=next; return 777; }
};
int read(Window *window,int MapLongitudeBounds,int a8,int local_14,int corner) {
    int delta=0,a7=local_14*2;
BLOCK
    return delta;
}
int main() {
    static_assert(sizeof(TerrainCacheEntry)==12,"stride");
    static_assert(offsetof(TerrainCacheEntry,altitude)==4,"member offset");
    TerrainCacheEntry old[128]={}, fresh[128]={};
    for(int i=0;i<128;++i)for(int c=0;c<5;++c) {
        old[i].altitude[c]=99;
        fresh[i].altitude[c]=static_cast<signed char>((i*17+c*43)%256-128);
    }
    for(int width : {4,6,8})for(int y : {-3,-1,0,1,3})
        for(int x : {-2,0,2})for(int corner=0;corner<5;++corner) {
            Window w={old+64,fresh+64,0};
            int index=(width/2)*y+x;
            assert(read(&w,width,y,x,corner)==fresh[64+index].altitude[corner]);
            assert(w.calls==1 && w.owned_==fresh+64);
            w={old+64,0,0};
            assert(read(&w,width,y,x,corner)==0 && w.calls==1 && !w.owned_);
        }
}
'''.replace("POD", pod).replace("BLOCK", block)
    source = tmp_path / "cache.cpp"
    source.write_text(fixture)
    binary = tmp_path / "cache"
    subprocess.run([compiler, "-std=c++11", "-fsanitize=undefined",
                    "-fno-sanitize-recover=all", str(source), "-o", str(binary)],
                   check=True, capture_output=True)
    subprocess.run([str(binary)], check=True, capture_output=True)
