"""Installed-source controls for the eight MovieMedia owner identities.

This is a proposal test, not native matching or acceptance. It reads installed
C++ only; no projected source or emitter output is used as a body.
"""
from __future__ import annotations

import os
import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.source import Mode, function_line, read_file, read_text


PACKET = Path(__file__).resolve().parent
ROOT = next(p for p in Path(__file__).resolve().parents if (p / 'src/movie_media.cpp').is_file())
BASE = "5fe79d193062cacf6b9eb9c5b901f9a0501cd2d8"

# Address -> (owning source, native-lexer definition name, expected reader mode).
OWNERS = {
    0x005CA230: ("src/movie_media.cpp", "MediaRuntime::sub_5ca230", Mode.BODY),
    0x005CD3B0: ("src/movie_media.cpp", "sub_5cd3b0", Mode.BODY),
    0x005CDAD0: ("src/movie_media.cpp", "sub_5cdad0", Mode.BODY),
    0x005CEE80: ("src/movie_media_decoder.cpp", "sub_5cee80", Mode.BODY),
    0x005CEF8A: ("src/movie_media_decoder.cpp", "sub_5cef8a", Mode.BODY),
    0x005CED1D: ("src/movie_media_decoder.cpp", "sub_5ced1d", Mode.BODY),
    0x005CBBC0: ("src/movie_media.cpp", "MediaRuntime::sub_5cbbc0", Mode.FILE),
    0x005CC430: ("src/movie_media.cpp", "media_component_mask_bits_31", Mode.FILE),
}
PRESERVED_FIELDS = (
    "name", "image_spans", "kind", "byte_exact", "semantic",
    "levers", "ruled_out", "unrecoverable", "deferred",
)

# Self-contained copies of the existing reviewed host fixtures. Keeping these
# here avoids making the proposed permanent test depend on ignored packet files.
SIX_HARNESS = r'''#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#define __fastcall
#define __cdecl
#define __int64 long long
typedef uint32_t DWORD; typedef int32_t LONG; typedef unsigned char BYTE; typedef char *HPSTR;
typedef void *HMMIO; typedef long HRESULT; typedef unsigned long ULONG;
#define DD_OK 0
#define DDPF_FOURCC 4
struct GUID { DWORD data[4]; }; static const GUID IID_IDirectDrawSurface2={{2,0,0,0}};
struct MediaQueueRecord { unsigned char *payload; DWORD field_04,field_08,field_0c; };
struct MediaDescriptor { HMMIO stream; };
struct MediaRuntime {
  int __fastcall sub_5ca230(int,int);
  MediaDescriptor *descriptor; unsigned char *input_ring_base,*input_ring_current;
  MediaQueueRecord read_records[24]; LONG read_producer_cursor,pending_read_records,read_counter,stream_eof;
};
static LONG mmio_result=8; static void *last_stream=0; static char *last_read_ptr=0;
static int release_count=0, create_result=0, query_result=0;
static int mmioRead(HMMIO h,HPSTR p,LONG n){last_stream=h;last_read_ptr=p;return mmio_result;}
static LONG InterlockedIncrement(LONG *p){return ++*p;}
struct DDPIXELFORMAT { DWORD dwSize,dwFlags,dwFourCC; DWORD spare[5]; };
struct DDSCAPS { DWORD dwCaps; };
struct DDSURFACEDESC {
 DWORD dwSize,dwFlags,dwHeight,dwWidth;
 DWORD spare_a[14]; DDPIXELFORMAT ddpfPixelFormat; DDSCAPS ddsCaps;
};
struct IDirectDrawSurface2 {
 virtual HRESULT QueryInterface(const GUID &,void **)=0;
 virtual ULONG AddRef()=0;
 virtual ULONG Release()=0;
};
struct IDirectDrawSurface : IDirectDrawSurface2 {};
struct IDirectDraw2 {
 virtual HRESULT CreateSurface(DDSURFACEDESC *,IDirectDrawSurface **,void *)=0;
};
struct FakeSurface: IDirectDrawSurface {
 HRESULT QueryInterface(const GUID &,void **out){if(query_result==0)*out=(IDirectDrawSurface2*)this;return query_result;}
 ULONG AddRef(){return 1;} ULONG Release(){++release_count;return 0;}
};
struct FakeDraw: IDirectDraw2 {
 FakeSurface surface; DDSURFACEDESC seen; int calls;
 HRESULT CreateSurface(DDSURFACEDESC *d,IDirectDrawSurface **out,void *){
   seen=*d;++calls;if(create_result==0)*out=&surface;return create_result;
 }
};
struct MediaConversionTableStorage { DWORD block_scale_entries[64]; DWORD packed_component_words[384]; };
MediaConversionTableStorage movie_media_conversion_tables={};
'''

SIX_MAIN = r'''static void ck(bool b,const char *label){if(!b){fprintf(stderr,"FAIL %s\n",label);exit(1);}}
static void *lowmap(size_t n){void*p=mmap(0,n,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_32BIT,-1,0);return p==MAP_FAILED?0:p;}
int main(){
  unsigned char *ring=(unsigned char*)lowmap(1200200); ck(ring!=0,"low ring mapping");
  MediaDescriptor desc={reinterpret_cast<void*>(0x1234)}; MediaRuntime rt={};
  rt.descriptor=&desc;rt.input_ring_base=ring;rt.input_ring_current=ring+1199995;
  rt.read_producer_cursor=26;rt.read_counter=41;rt.pending_read_records=5;
  mmio_result=77;ck(rt.sub_5ca230(10,6)==1,"successful read");
  ck(last_stream==desc.stream&&last_read_ptr==(char*)ring,"wrap before mmioRead");
  ck(rt.read_records[2].payload==ring&&rt.read_records[2].field_08==41&&
     rt.read_records[2].field_0c==6,"typed read-record publication");
  ck(rt.input_ring_current==ring+10&&rt.read_producer_cursor==27&&
     rt.pending_read_records==6&&rt.read_counter==42,"advance and interlocked state");
  rt.stream_eof=0;mmio_result=0;LONG before=rt.read_counter;
  ck(rt.sub_5ca230(3,2)==-1&&rt.stream_eof==1&&rt.read_counter==before,"EOF path");
  mmio_result=-1;rt.stream_eof=0;
  ck(rt.sub_5ca230(3,2)==-2&&rt.stream_eof==0&&rt.read_counter==before,"error path");

  FakeDraw draw; IDirectDrawSurface2 *output=0; create_result=0;query_result=0;
  release_count=0;ck(sub_5cdad0(&draw,&output,320,200,0x32595559)==0,"surface create success");
  ck(draw.calls==1&&draw.seen.dwSize==0x6c&&draw.seen.dwFlags==0x1007&&
     draw.seen.dwWidth==320&&draw.seen.dwHeight==200&&
     draw.seen.ddpfPixelFormat.dwSize==0x20&&draw.seen.ddpfPixelFormat.dwFlags==4&&
     draw.seen.ddpfPixelFormat.dwFourCC==0x32595559&&draw.seen.ddsCaps.dwCaps==0x4040,
     "surface descriptor fields");
  ck(output==(IDirectDrawSurface2*)&draw.surface&&release_count==0,"QI publish/leak preserved");
  output=0;query_result=-7;release_count=0;
  ck(sub_5cdad0(&draw,&output,4,3,9)==1&&output==0&&release_count==1,"QI failure releases original");
  query_result=0;create_result=-9;release_count=0;
  ck(sub_5cdad0(&draw,&output,4,3,9)==1&&release_count==0,"CreateSurface failure no release");

  movie_media_conversion_tables.block_scale_entries[0]=65536;
  for(int i=0;i<384;++i)movie_media_conversion_tables.packed_component_words[i]=i;
  int pixels[320];for(int i=0;i<320;++i)pixels[i]=-1;
  sub_5cd3b0(2,4,6,8,4,6,pixels,67);
  for(int row=0;row<8;++row)for(int col=0;col<8;++col){
    ck(pixels[row*16+col]==390,"first 8x8 luma tile");
    ck(pixels[row*16+8+col]==391,"second 8x8 luma tile");
    ck(pixels[134+row*16+col]==392,"third 8x8 luma tile");
    ck(pixels[142+row*16+col]==393,"fourth 8x8 luma tile");
  }
  puts("installed MovieMedia controls PASS");return 0;
}
'''

TWO_HARNESS = r'''#include <cassert>
#include <stdint.h>
#define __fastcall
typedef unsigned char BYTE;
typedef unsigned int DWORD;
typedef void IDirectDrawSurface2;
struct MediaDescriptor {
  IDirectDrawSurface2 *surface_1c;
  IDirectDrawSurface2 *surface_20;
  DWORD flags;
  BYTE *provided_output_base_4c;
  BYTE *provided_output_alternate_50;
};
struct MediaRuntime {
  MediaDescriptor *descriptor;
  DWORD queue_record_kind;
  void sub_5cbbc0();
};
__RUNTIME_BODY__
__MASK_BODY__
int main() {
  assert(media_component_mask_bits_31(0U) == 0);
  assert(media_component_mask_bits_31(0x80000000U) == 0);
  assert(media_component_mask_bits_31(0x80000001U) == 1);
  assert(media_component_mask_bits_31(0x7fffffffU) == 31);
  MediaDescriptor descriptor = {};
  MediaRuntime runtime = {&descriptor, 1};
  descriptor.surface_1c = reinterpret_cast<void *>(0x10);
  descriptor.surface_20 = reinterpret_cast<void *>(0x20);
  runtime.sub_5cbbc0();
  assert(descriptor.surface_1c == reinterpret_cast<void *>(0x20));
  assert(descriptor.surface_20 == reinterpret_cast<void *>(0x10));
  descriptor.surface_1c = reinterpret_cast<void *>(0x30);
  descriptor.surface_20 = reinterpret_cast<void *>(0x40);
  runtime.queue_record_kind = 2;
  runtime.sub_5cbbc0();
  assert(descriptor.surface_1c == reinterpret_cast<void *>(0x30));
  assert(descriptor.surface_20 == reinterpret_cast<void *>(0x40));
  descriptor.flags = 0x800;
  BYTE first_output[16],second_output[16];
  descriptor.provided_output_base_4c = first_output;
  descriptor.provided_output_alternate_50 = second_output;
  runtime.queue_record_kind = 0;
  runtime.sub_5cbbc0();
  assert(descriptor.provided_output_base_4c == second_output);
  assert(descriptor.provided_output_alternate_50 == first_output);
  return 0;
}
'''


def historical_rows():
    """Read pre-transfer identities with the package's source reader."""
    env = os.environ.copy()
    env["UV_CACHE_DIR"] = "/tmp/osmx-uv-cache"
    raw = subprocess.check_output(
        ["git", "show", f"{BASE}:src/leaf_recoveries.cpp"], cwd=ROOT, env=env
    ).decode("utf-8")
    return {
        row.address: row
        for row in read_text(raw, ROOT / "src/leaf_recoveries.cpp")
        if row.address in OWNERS
    }


def _code_mask(text: str) -> str:
    return COMMENTS_OR_STRINGS.sub(
        lambda match: "".join(
            "\n" if char == "\n" else " " for char in match.group(0)
        ),
        text,
    )


def installed_definition(path: Path, name: str) -> str:
    """Extract one live definition, anchored by the package's C++ lexer."""
    source = path.read_text(encoding="utf-8")
    line = function_line(path, name)
    start = sum(len(item) for item in source.splitlines(keepends=True)[:line - 1])
    masked = _code_mask(source)
    opening = masked.find("{", start)
    assert opening >= 0, f"native lexer found {name}, but its body is absent"
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError(f"unterminated installed definition: {name}")


def compile_and_run(compiler: str, source_text: str, tmp_path: Path, *, succeeds: bool):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} unavailable")
    source = tmp_path / f"installed-{compiler.replace('+', 'p')}.cpp"
    binary = tmp_path / f"installed-{compiler.replace('+', 'p')}"
    source.write_text(source_text)
    built = subprocess.run(
        [executable, "-std=c++98", "-O0", str(source), "-o", str(binary)],
        capture_output=True, text=True,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    if succeeds:
        assert ran.returncode == 0, ran.stdout + ran.stderr
    else:
        assert ran.returncode != 0, "mutated installed-body control unexpectedly passed"


def current_rows():
    result = {}
    for relative in {owner for owner, _name, _mode in OWNERS.values()}:
        for row in read_file(ROOT / relative):
            if row.address in OWNERS:
                assert row.address not in result, f"duplicate live identity {row.address:08X}"
                result[row.address] = row
    return result


def test_all_eight_installed_mappings_preserve_reader_history():
    old = historical_rows()
    current = current_rows()
    assert set(old) == set(OWNERS)
    assert set(current) == set(OWNERS)
    for address, (relative, definition, mode) in OWNERS.items():
        before, after = old[address], current[address]
        assert after.path == ROOT / relative
        assert after.mode is mode
        assert not after.extract_error
        assert after.region
        assert definition in installed_definition(after.path, definition)
        for field in PRESERVED_FIELDS:
            if address == 0x005CED1D and field == "image_spans":
                # Reviewed extent repair 18f93db66 separates two independent
                # transforms. The original seed covered both; retain its
                # complete range through the two current owner identities.
                assert before.image_spans == ((0x005CED1D, 0x005CEF8A),)
                assert after.image_spans == ((0x005CED1D, 0x005CEE80),)
                assert current[0x005CEE80].image_spans == ((0x005CEE80, 0x005CEF8A),)
                continue
            assert getattr(after, field) == getattr(before, field), (
                f"0x{address:08X} changed historical {field}"
            )
        lesson_history = lambda row: tuple(
            (lesson.kind, lesson.key, lesson.prose) for lesson in (row.origin or ())
        )
        assert lesson_history(after) == lesson_history(before), (
            f"0x{address:08X} changed original lesson prose"
        )
        assert f"ORIGINAL: 0x{address:08X}" in after.region
    assert not (ROOT / "src/leaf_recoveries.cpp").exists(), "retired leaf owner source reappeared"


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_six_packet_controls_compile_bodies_from_installed_movie_media(compiler, tmp_path):
    path = ROOT / "src/movie_media.cpp"
    bodies = "\n\n".join(
        installed_definition(path, name)
        for name in ("MediaRuntime::sub_5ca230", "sub_5cd3b0", "sub_5cdad0")
    )
    unit = SIX_HARNESS + "\n" + bodies + "\n" + SIX_MAIN
    compile_and_run(compiler, unit, tmp_path, succeeds=True)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_two_packet_controls_compile_bodies_from_installed_movie_media(compiler, tmp_path):
    path = ROOT / "src/movie_media.cpp"
    runtime = installed_definition(path, "MediaRuntime::sub_5cbbc0")
    mask = installed_definition(path, "media_component_mask_bits_31")
    unit = TWO_HARNESS.replace("__RUNTIME_BODY__", runtime).replace(
        "__MASK_BODY__", mask
    )
    compile_and_run(compiler, unit, tmp_path, succeeds=True)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_source_mutations_break_the_respective_controls(compiler, tmp_path):
    path = ROOT / "src/movie_media.cpp"
    mask_body = installed_definition(path, "media_component_mask_bits_31")
    assert mask_body.count("bit < 0x1F") == 1
    mutated_mask = mask_body.replace("bit < 0x1F", "bit < 0x20", 1)
    mask_unit = TWO_HARNESS.replace(
        "__RUNTIME_BODY__", installed_definition(path, "MediaRuntime::sub_5cbbc0")
    ).replace("__MASK_BODY__", mutated_mask)
    compile_and_run(compiler, mask_unit, tmp_path, succeeds=False)

    runtime_body = installed_definition(path, "MediaRuntime::sub_5cbbc0")
    assert runtime_body.count("queue_record_kind == 1") == 1
    mutated_runtime = runtime_body.replace("queue_record_kind == 1",
                                           "queue_record_kind != 1", 1)
    runtime_unit = TWO_HARNESS.replace("__RUNTIME_BODY__", mutated_runtime).replace(
        "__MASK_BODY__", installed_definition(path, "media_component_mask_bits_31")
    )
    compile_and_run(compiler, runtime_unit, tmp_path, succeeds=False)


def test_native_lexer_ignores_fake_definitions_and_rejects_missing_body(tmp_path):
    fake = tmp_path / "lexer_fake.cpp"
    fake.write_text("/* void ghost_owner() { return; } */\nint real_owner() { return 7; }\n")
    assert function_line(fake, "real_owner") == 2
    with pytest.raises(ValueError, match="0 definitions"):
        function_line(fake, "ghost_owner")
    with pytest.raises(ValueError, match="0 definitions"):
        function_line(ROOT / "src/movie_media.cpp", "missing_movie_owner")


def test_idct_rows_and_definitions_are_installed_in_decoder_owner():
    decoder = ROOT / "src/movie_media_decoder.cpp"
    for address in (0x005CEE80, 0x005CEF8A, 0x005CED1D):
        row = next(item for item in read_file(decoder) if item.address == address)
        assert row.mode is Mode.BODY and not row.extract_error
        installed_definition(decoder, OWNERS[address][1])


def test_5cc940_calls_the_typed_directdraw_owner_without_integer_casts():
    caller = installed_definition(ROOT / "src/movie_media.cpp", "sub_5cc940")
    call = "sub_5cdad0(d.draw_device_10, &d.surface_1c,"
    assert " ".join(caller.split()).count(call) == 1
    assert "sub_5cdad0((int *)" not in caller
    assert "(int *)&d.surface_1c" not in caller
