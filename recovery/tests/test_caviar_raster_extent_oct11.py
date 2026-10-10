"""Actual extent parser at packed addresses; not complete CVR format proof."""
from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest

from match_recovery.source import read_text
from test_caviar_shape_chain_oct11 import assert_caviar_owner_continuity
from test_caviar_draw_callbacks_actual_source import extract

ROOT = Path(__file__).resolve().parents[2]
BASE = "51eec0d6e95b648f7527d4b78e33f73ec199fb38"


def test_original_identities_claims_symbols_and_history_preserved():
    path = ROOT / "src/caviar.cpp"
    old = subprocess.check_output(["git", "show", BASE + ":src/caviar.cpp"],
                                  cwd=ROOT, text=True)
    assert_caviar_owner_continuity(read_text(old, path))


def program():
    header = (ROOT / "src/caviar_render_types.h").read_text()
    source = (ROOT / "src/caviar.cpp").read_text()
    declaration = extract(header, "struct CvrRasterItemHeaderCandidate {")
    helper = extract(source, "static uint32_t caviar_raster_payload_extent(")
    assert source.count("caviar_raster_payload_extent(item)") == 3
    assert "*reinterpret_cast<const uint32_t *>(item->inline_payload_)" not in source
    return r'''#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#pragma pack(push,1)
''' + declaration + ";\n#pragma pack(pop)\n" + helper + r'''
int main() {
    assert(offsetof(CvrRasterItemHeaderCandidate,inline_payload_)==0x20);
    unsigned char bytes[64];
    for (unsigned offset=0; offset!=4; ++offset) {
        memset(bytes,0,sizeof(bytes));
        CvrRasterItemHeaderCandidate *item=
            reinterpret_cast<CvrRasterItemHeaderCandidate*>(bytes+offset);
        item->item_count_=0xFFFFFFFFU;
        item->flags_=0;
        assert(caviar_raster_payload_extent(item)==0xFFFFFFFDU);
        item->flags_=2;
        assert(caviar_raster_payload_extent(item)==0xFFFFFFFDU);
        item->item_count_=0;
        assert(caviar_raster_payload_extent(item)==0);
        const unsigned char encoded[4]={0x78,0x56,0x34,0x12};
        memcpy(item->inline_payload_,encoded,4);
        item->flags_=1;
        assert(caviar_raster_payload_extent(item)==0x12345678U);
        item->flags_=3;
        assert(caviar_raster_payload_extent(item)==0x12345678U);
    }
    // Clear flag must not touch an encoded-size dword, even speculatively.
    const size_t page=static_cast<size_t>(sysconf(_SC_PAGESIZE));
    unsigned char *mapping=static_cast<unsigned char*>(mmap(0,page*2,
        PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0));
    assert(mapping!=MAP_FAILED);
    assert(mprotect(mapping+page,page,PROT_NONE)==0);
    CvrRasterItemHeaderCandidate *item=
        reinterpret_cast<CvrRasterItemHeaderCandidate*>(mapping+page-0x20);
    memset(item,0,0x20);
    item->item_count_=7;
    assert(caviar_raster_payload_extent(item)==21);
    assert(munmap(mapping,page*2)==0);
}
'''


def run(text, succeeds):
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    assert any(compilers), "an actual-source host compiler is required"
    with tempfile.TemporaryDirectory(prefix="caviar-raster-extent-") as directory:
        cpp = Path(directory) / "control.cpp"
        cpp.write_text(text)
        for compiler in filter(None, compilers):
            exe = Path(directory) / Path(compiler).name.replace("+", "p")
            subprocess.run([compiler, "-std=c++11", "-O2", "-fsanitize=undefined",
                            "-fno-sanitize-recover=undefined", str(cpp), "-o", str(exe)],
                           check=True, capture_output=True, timeout=60)
            result = subprocess.run([str(exe)], capture_output=True, timeout=30)
            assert (result.returncode == 0) == succeeds, result.stderr.decode()


def test_actual_extent_read_is_unaligned_safe_and_branch_gated():
    run(program(), True)


@pytest.mark.parametrize("old,new", (
    ("memcpy(&extent, item->inline_payload_, sizeof(extent));",
     "extent = bad_aligned_load(reinterpret_cast<const uint32_t *>(item->inline_payload_));"),
    ("item->item_count_ * 3u", "item->item_count_ * 4u"),
    ("return extent;", "return extent + 4u;"),
    ("if ((item->flags_ & 1) == 0)", "if (item->flags_ == 0)"),
))
def test_alignment_count_header_and_flag_damage_is_rejected(old, new):
    text = program()
    assert old in text
    if "bad_aligned_load" in new:
        # Clang propagates a packed member's reduced alignment through the
        # direct cast and omits that diagnostic. An opaque ordinary uint32
        # consumer gives both sanitizers an explicit aligned-load contract.
        bad = "static __attribute__((noinline)) uint32_t bad_aligned_load(const uint32_t *p) { return *p; }\n"
        text = text.replace("static uint32_t caviar_raster_payload_extent(",
                            bad + "static uint32_t caviar_raster_payload_extent(", 1)
    run(text.replace(old, new, 1), False)
