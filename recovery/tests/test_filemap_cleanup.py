"""Live Filemap cleanup semantics and bounded original/caller evidence.

Host doubles do not establish the Win32 layout or exception-table ABI. LONG is
pointer-wide here solely to bridge close(end)'s original Win32 pointer casts;
all tested differences fit the original signed 32-bit range.
The two native checks require prepared recipes and an explicitly coordinated
VC6_PREFIX; ordinary host-only test runs do not use the shared default prefix.
"""

import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32

from match_recovery.compiler import Compiler
from match_recovery.measurement import Measurement
from match_recovery.model import Span
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.source import read, read_file
from match_recovery.source.asm import compare_unit, shared_spans
from match_recovery.source_inventory import image_pin


ROOT = Path(__file__).resolve().parents[2]


def braced(text, signature):
    start = text.index(signature)
    depth = 0
    for end in range(text.index("{", start), len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated source")


def live_source():
    def clean(path):
        return re.sub(r"/\*.*?\*/|//[^\n]*", "", path.read_text(), flags=re.S)
    header = clean(ROOT / "src/filemap.h")
    source = clean(ROOT / "src/filemap.cpp")
    # Expose actual members only for state assertions, without an invented
    # parallel Filemap layout or changes to the method bodies.
    declaration = braced(header, "class Filemap").replace("private:", "public:") + ";"
    signatures = ["void Filemap::close()", "void Filemap::close(LPVOID new_addr)",
                  "Filemap::Filemap()", "Filemap::~Filemap()"]
    if "inline void Filemap::close_impl()" in source:
        signatures.insert(0, "inline void Filemap::close_impl()")
    return declaration, "\n".join(braced(source, s) for s in signatures)


HARNESS = r"""
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <new>
#include <string>
#include <vector>
using std::uintptr_t;
typedef void *LPVOID;
typedef void *HANDLE;
typedef const char *LPCSTR;
typedef int BOOL;
typedef std::intptr_t LONG; // Explicit host-only pointer-width bridge.
#define INVALID_HANDLE_VALUE reinterpret_cast<HANDLE>(~uintptr_t(0))
#define INVALID_SET_FILE_POINTER 0xffffffffu
#define FILE_BEGIN 0
void check_at(bool ok, int line) {
    if (!ok) { std::fprintf(stderr,"check at %d\n",line); std::exit(91); }
}
#define check(x) check_at((x),__LINE__)
BOOL UnmapViewOfFile(LPVOID);
BOOL CloseHandle(HANDLE);
unsigned SetFilePointer(HANDLE,LONG,void *,unsigned);
BOOL SetEndOfFile(HANDLE);
// DECLARATION
Filemap *active;
struct Event { std::string kind; void *arg,*view,*mapping,*file; LONG offset; };
std::vector<Event> events;
bool fail_pointer, fail_api;
void event(const char *kind,void *arg,LONG offset=0) {
    events.push_back({kind,arg,active->map_view_addr_,active->file_map_,active->file_,offset});
}
BOOL UnmapViewOfFile(LPVOID p) { event("unmap",p); return !fail_api; }
BOOL CloseHandle(HANDLE h) { event("close",h); return !fail_api; }
unsigned SetFilePointer(HANDLE h,LONG n,void *high,unsigned origin) {
    check(!high && origin==FILE_BEGIN); event("seek",h,n);
    return fail_pointer ? INVALID_SET_FILE_POINTER : static_cast<unsigned>(n);
}
BOOL SetEndOfFile(HANDLE h) { event("end",h); return !fail_api; }
// BODIES
void *handle(int n) { return reinterpret_cast<void *>(uintptr_t(n)); }
void setup(Filemap &f,void *view,void *mapping,void *file) {
    active=&f; f.map_view_addr_=view; f.file_map_=mapping; f.file_=file;
    f.file_size_=0x12345678; events.clear(); fail_pointer=false; fail_api=false;
}
void normal(bool destructor) {
    char data[512];
    for(int v=0;v<2;++v) for(int m=0;m<2;++m) for(int h=0;h<3;++h)
    for(int failure=0;failure<2;++failure) {
        alignas(Filemap) unsigned char storage[sizeof(Filemap)];
        Filemap &f=*new(storage) Filemap;
        check(!f.map_view_addr_ && !f.file_map_ && f.file_==INVALID_HANDLE_VALUE);
        void *view=v ? data+100 : nullptr, *mapping=m ? handle(7) : nullptr;
        void *file=h==0 ? INVALID_HANDLE_VALUE : (h==1 ? nullptr : handle(9));
        setup(f,view,mapping,file); fail_api=failure;
        if(destructor) f.~Filemap(); else f.close();
        std::size_t i=0;
        if(v) {
            check(events[i].kind=="unmap" && events[i].arg==view);
            check(events[i].view==view && events[i].mapping==mapping && events[i].file==file); ++i;
        }
        if(m) {
            check(events[i].kind=="close" && events[i].arg==mapping);
            check(!events[i].view && events[i].mapping==mapping && events[i].file==file); ++i;
        }
        if(h!=0) {
            check(events[i].kind=="close" && events[i].arg==file);
            check(!events[i].view && !events[i].mapping && events[i].file==file); ++i;
        }
        check(events.size()==i);
        // Do not inspect members after lifetime end; callbacks above observe
        // the destructor's actual operations while the lifetime is active.
        if(!destructor) {
            check(!f.map_view_addr_ && !f.file_map_ && f.file_==(h ? nullptr : file));
            check(f.file_size_==0x12345678);
            events.clear(); f.close();
            check(events.size()==(h ? 1u : 0u));
            if(h) check(events[0].kind=="close" && !events[0].arg);
            f.~Filemap();
        }
    }
}
void truncate() {
    char data[512];
    for(int early=0;early<2;++early) for(int failed=0;failed<2;++failed)
    for(int h=0;h<3;++h) {
        Filemap f;
        void *file=h==0 ? nullptr : (h==1 ? INVALID_HANDLE_VALUE : handle(9));
        setup(f,data+100,handle(7),file); fail_pointer=failed;
        f.close(data+(early ? 90 : 130));
        check(events[0].kind=="unmap" && events[0].arg==data+100);
        check(events[1].kind=="close" && events[1].arg==handle(7) && !events[1].view);
        std::size_t i=2;
        if(!early) {
            check(events[i].kind=="seek" && events[i].arg==file && events[i].offset==30);
            check(!events[i].view && !events[i].mapping); ++i;
            if(!failed) { check(events[i].kind=="end" && events[i].arg==file); ++i; }
        }
        // Plain close uses != INVALID_HANDLE_VALUE. The successful truncation
        // tail instead uses != NULL; these are deliberately different cases.
        bool closes=(early || failed) ? h!=1 : h!=0;
        if(closes) { check(events[i].kind=="close" && events[i].arg==file); ++i; }
        check(events.size()==i && f.file_size_==0x12345678);
        check(!f.map_view_addr_ && !f.file_map_ && f.file_==(closes ? nullptr : file));
    }
}
int main() { normal(false); normal(true); truncate(); }
"""


def run_host(tmp_path, damage=None):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler required")
    declaration, bodies = live_source()
    if damage:
        old, new = damage
        assert old in bodies
        bodies = bodies.replace(old, new, 1)
    code = HARNESS.replace("// DECLARATION", declaration).replace("// BODIES", bodies)
    source, binary = tmp_path / "cleanup.cpp", tmp_path / "cleanup"
    source.write_text(code)
    build = subprocess.run([compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
                            "-fno-sanitize-recover=undefined", str(source), "-o", str(binary)],
                           capture_output=True, text=True)
    assert build.returncode == 0, build.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True)
    assert result.returncode == (91 if damage else 0), result.stderr


def test_actual_cleanup_and_truncation(tmp_path):
    run_host(tmp_path)


@pytest.mark.parametrize("damage", [
    ("if (file_ != INVALID_HANDLE_VALUE)", "if (file_)"),
    ("map_view_addr_ = 0;", "map_view_addr_ = 0; file_size_ = 0;"),
    ("UnmapViewOfFile(map_view_addr_);", "map_view_addr_ = 0; UnmapViewOfFile(map_view_addr_);"),
    ("file_map_ = 0;", "/* missing clear */"),
    ("if (file_) {", "if (file_ != INVALID_HANDLE_VALUE) {"),
    ("SetEndOfFile(file_);", "/* missing truncation */"),
])
def test_cleanup_rejects_runtime_damage(tmp_path, damage):
    run_host(tmp_path, damage)


def test_pinned_external_close_and_local_cleanup_sites():
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original image required")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    for address in (0x422FD2, 0x423E0F, 0x423EB0, 0x423F45, 0x4240B2,
                    0x4240CF, 0x4240F9, 0x424283, 0x42435C, 0x5FE0F0, 0x539069):
        op, = decoder.disasm(image.read(Span(address, address + 5)), address)
        assert (op.mnemonic, op.op_str) == ("call", "0x6287c0")
    for address in (0x628452, 0x628562, 0x628673, 0x628826):
        op, = decoder.disasm(image.read(Span(address, address + 6)), address)
        assert (op.mnemonic, op.op_str) == ("call", "dword ptr [0x669150]")


@pytest.fixture(scope="module")
def compiled():
    from recovery.environment.wine import VC6_ROOT
    if not os.environ.get("VC6_PREFIX"):
        pytest.skip("native checks require a coordinator-provided isolated VC6_PREFIX")
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None:
        pytest.skip("VC6 and Wine32 required; run under a coordinator compiler lease")
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    raw = {(unit, tuple(flags)): compiler(ROOT / unit, (), flags + settings.layout_flags)
           for unit in ("src/filemap.cpp", "src/game.cpp") for flags in settings.flag_sets}
    return settings, raw


def test_native_nine_filemap_historical_bodies(compiled):
    settings, raw = compiled
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original image required")
    records = read_file(ROOT / "src/filemap.cpp")
    assert {r.address for r in records if r.byte_exact} == {
        0x628380, 0x6283A0, 0x6283C0, 0x6283E0, 0x628430,
        0x628540, 0x628650, 0x6287C0, 0x628810}
    result = compare_unit(records, path, ROOT / "src/filemap.cpp", [], settings.flag_sets,
                          shared_spans(read(ROOT / "src")),
                          compiler=lambda unit, args, flags: raw[(unit.relative_to(ROOT).as_posix(), tuple(flags))])
    assert len(result.results) == 9
    assert all(not isinstance(value, str) and value.verdict == "BYTE_EXACT"
               for _, value in result.results), result.results
    for flags in settings.flag_sets:
        obj = Coff(raw[("src/filemap.cpp", tuple(flags))])
        obj.function("?close@Filemap@@QAEXXZ")  # Genuine standalone owner.
        assert not any("close_impl" in s.name for s in obj.symbols.values())


def test_native_external_call_targets(compiled):
    settings, raw = compiled
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    for flags in settings.flag_sets:
        obj = Coff(raw[("src/game.cpp", tuple(flags))])
        for symbol in ("?compare_palettes@@YAHXZ", "?colortables_init@@YAXXZ"):
            data, relocs, _ = obj.subject(obj.function(symbol))
            calls = [op for op in decoder.disasm(data, 0) if op.mnemonic == "call"]
            close_refs = [rel for rel in relocs
                          if obj.symbols[int(rel.target)].name == "?close@Filemap@@QAEXXZ"]
            assert close_refs
            if symbol == "?compare_palettes@@YAHXZ":
                assert len(close_refs) == 1
            # The unclaimed producer may share call sites between branches.
            # Its actual-body behavior tests check cleanup execution/order;
            # static site counts cannot stand in for that control-flow proof.
            assert all(any(op.address + 1 == rel.offset and op.size == 5 for op in calls)
                       for rel in close_refs)
            assert not any(obj.symbols[int(rel.target)].name == "__imp__UnmapViewOfFile@4"
                           for rel in relocs)
