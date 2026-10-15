"""Actual-source controls for Buffer's shared inline Sprite setup family."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())
SOURCE = (ROOT / "src/buffer.cpp").read_text(encoding="utf-8")


def body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start + len(signature))
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError(f"unterminated source body: {signature}")


PREPARE = body(SOURCE, "inline void Buffer::prepare_shared_sprite(int colour)")
SETUP = body(SOURCE, "void Buffer::setup_buff_sprite(int colour)")
RELEASE = body(SOURCE, "void Buffer::release_buff_sprite()")
MONO = body(SOURCE, "int Buffer::draw_mono(Buffer *destination, int transparent, int x, int y,")
UNIT = body(SOURCE, "void Buffer::draw_dest_unk1(Buffer *destination, int x, int y,")


HARNESS = r'''#include <cassert>
#include <cstdint>

typedef void *LPVOID;
uint8_t WinFillColour=0xAB;
struct MockSurface { int unlocks; MockSurface():unlocks(0){} void Unlock(void*){++unlocks;} };
struct Header { int biWidth, biHeight; };
struct Dib { Header bmiHeader; };
struct MockSprite {
  unsigned char *pcBits_;
  int iSpriteWidth2_, iSpriteWidth_, iSpriteHeight_, iWidth_, iHeight_;
  int iLeftOffset_, iTopOffset_;
  uint8_t cTransparentIndex_;
  int fObj1Exists_;
  int calls, kind, args[8];
  unsigned char *observed_bits;
  void *target_seen;
  unsigned char *table_seen;
  MockSprite():pcBits_(0),iSpriteWidth2_(0),iSpriteWidth_(0),iSpriteHeight_(0),
    iWidth_(0),iHeight_(0),iLeftOffset_(0),iTopOffset_(0),
    cTransparentIndex_(0),fObj1Exists_(1),calls(0),kind(0),observed_bits(0),
    target_seen(0),table_seen(0) { for(int &a:args)a=0; }
  int draw_mono(void *target,int transparent,int x,int y,int colour,int n,int d) {
    ++calls; kind=1; target_seen=target; observed_bits=pcBits_;
    args[0]=(target!=0); args[1]=transparent; args[2]=x;
    args[3]=y; args[4]=colour; args[5]=n; args[6]=d; return 721;
  }
  int draw_mono(void *target,int transparent,int x,int y,int colour) {
    ++calls; kind=2; target_seen=target; observed_bits=pcBits_;
    args[0]=(target!=0); args[1]=transparent; args[2]=x;
    args[3]=y; args[4]=colour; return 722;
  }
  void draw_dest_unk1(void *target,int x,int y,unsigned char *table) {
    ++calls; kind=3; target_seen=target; table_seen=table; observed_bits=pcBits_;
    args[0]=(target!=0); args[1]=x; args[2]=y; args[3]=(table!=0);
  }
  void draw_dest_unk1(void *target,int x,int y,int n,int d,unsigned char *table) {
    ++calls; kind=4; target_seen=target; table_seen=table; observed_bits=pcBits_;
    args[0]=(target!=0); args[1]=x; args[2]=y;
    args[3]=n; args[4]=d; args[5]=(table!=0);
  }
};
struct BufferSpriteOwnerType { MockSprite value; MockSprite &sprite(){return value;} };
BufferSpriteOwnerType BufferSpriteOwner;

class BufferMock {
 public:
  MockSurface *surface_;
  LPVOID dib_bits_, locked_bits_;
  int surface_lock_count_;
  unsigned int stride_;
  Dib dib_;
  BufferMock():surface_(0),dib_bits_(0),locked_bits_(0),surface_lock_count_(0),stride_(0) {
    dib_.bmiHeader.biWidth=0; dib_.bmiHeader.biHeight=0;
  }
  void setup_buff_sprite(int colour);
  static void release_buff_sprite();
  int draw_mono(BufferMock *destination,int transparent,int x,int y,int colour,int n,int d);
  void draw_dest_unk1(BufferMock *destination,int x,int y,unsigned char *table);
 private:
  void prepare_shared_sprite(int colour);
};
// PREPARE
// SETUP
// RELEASE
// MONO
// UNIT

static void clean_sprite() { BufferSpriteOwner.value=MockSprite(); }
static void check_descriptor(const unsigned char *bits) {
  const MockSprite &s=BufferSpriteOwner.value;
  assert(s.pcBits_==bits && s.iSpriteWidth2_==29);
  assert(s.iSpriteWidth_==23 && s.iSpriteHeight_==17);
  assert(s.iWidth_==23 && s.iHeight_==17);
  assert(s.iLeftOffset_==0 && s.iTopOffset_==0);
  assert(s.cTransparentIndex_==0xAB && s.fObj1Exists_==0);
}
int main() {
  unsigned char pixels[64]={0};
  BufferMock b; b.dib_.bmiHeader.biWidth=23; b.dib_.bmiHeader.biHeight=-17;
  b.stride_=29; b.dib_bits_=pixels;

  b.setup_buff_sprite(-1);
  check_descriptor(pixels);
  assert(b.locked_bits_==0 && b.surface_lock_count_==0);
  BufferMock::release_buff_sprite();
  assert(BufferSpriteOwner.value.pcBits_==0);

  clean_sprite(); b.dib_bits_=0; b.locked_bits_=pixels; b.surface_lock_count_=0;
  b.setup_buff_sprite(0x123);
  assert(BufferSpriteOwner.value.pcBits_==0 && b.locked_bits_==0);
  assert(b.surface_lock_count_==0 && BufferSpriteOwner.value.cTransparentIndex_==0x23);

  clean_sprite(); b.dib_bits_=pixels; b.surface_lock_count_=4;
  b.setup_buff_sprite(0x123);
  assert(BufferSpriteOwner.value.pcBits_==pixels && b.locked_bits_==pixels);
  assert(b.surface_lock_count_==4 && BufferSpriteOwner.value.cTransparentIndex_==0x23);

  // The setup guard exits on a non-null surface before publication; reaching
  // its later surface arm would require alias mutation outside this model.
  clean_sprite(); MockSurface surface; b.surface_=&surface;
  b.locked_bits_=pixels; b.surface_lock_count_=7;
  BufferSpriteOwner.value.pcBits_=pixels;
  BufferSpriteOwner.value.cTransparentIndex_=0x44;
  b.setup_buff_sprite(-1);
  assert(BufferSpriteOwner.value.pcBits_==pixels);
  assert(BufferSpriteOwner.value.cTransparentIndex_==0x44);
  assert(b.locked_bits_==pixels && b.surface_lock_count_==7 && surface.unlocks==0);

  clean_sprite(); b.surface_=0; b.locked_bits_=0; b.surface_lock_count_=0;
  BufferMock target;
  assert(b.draw_mono(&target,-1,31,-42,9,3,5)==721);
  assert(BufferSpriteOwner.value.kind==1 && BufferSpriteOwner.value.calls==1);
  assert(BufferSpriteOwner.value.target_seen==&target);
  assert(BufferSpriteOwner.value.observed_bits==pixels);
  assert(BufferSpriteOwner.value.args[0]==1 && BufferSpriteOwner.value.args[1]==-1);
  assert(BufferSpriteOwner.value.args[2]==31 && BufferSpriteOwner.value.args[3]==-42);
  assert(BufferSpriteOwner.value.args[4]==9 && BufferSpriteOwner.value.args[5]==3 &&
         BufferSpriteOwner.value.args[6]==5);
  assert(BufferSpriteOwner.value.cTransparentIndex_==0xAB);
  assert(BufferSpriteOwner.value.pcBits_==0 && b.locked_bits_==0 && b.surface_lock_count_==0);

  clean_sprite();
  unsigned char table[4]={1,2,3,4};
  b.draw_dest_unk1(&target,-13,27,table);
  assert(BufferSpriteOwner.value.kind==3 && BufferSpriteOwner.value.calls==1);
  assert(BufferSpriteOwner.value.target_seen==&target && BufferSpriteOwner.value.table_seen==table);
  assert(BufferSpriteOwner.value.observed_bits==pixels);
  assert(BufferSpriteOwner.value.args[0]==1 && BufferSpriteOwner.value.args[1]==-13 &&
         BufferSpriteOwner.value.args[2]==27 && BufferSpriteOwner.value.args[3]==1);
  assert(BufferSpriteOwner.value.cTransparentIndex_==0xAB);
  assert(BufferSpriteOwner.value.pcBits_==0);

  clean_sprite(); b.surface_=&surface; b.locked_bits_=pixels;
  BufferSpriteOwner.value.pcBits_=pixels;
  assert(b.draw_mono(&target,-1,1,2,3,4,5)==721);
  assert(BufferSpriteOwner.value.target_seen==&target &&
         BufferSpriteOwner.value.observed_bits==pixels);
  assert(BufferSpriteOwner.value.args[1]==-1 && BufferSpriteOwner.value.pcBits_==0);
  assert(BufferSpriteOwner.value.cTransparentIndex_==0);
}
'''


def fixture(prepare: str = PREPARE, setup: str = SETUP,
            release: str = RELEASE,
            mono: str = MONO, unit: str = UNIT) -> str:
    replacements = {"PREPARE": prepare, "SETUP": setup, "RELEASE": release,
                    "MONO": mono, "UNIT": unit}
    result = HARNESS
    for marker, definition in replacements.items():
        mock_definition = definition.replace("Buffer::", "BufferMock::")
        mock_definition = mock_definition.replace("Buffer *", "BufferMock *")
        result = result.replace(f"// {marker}", mock_definition)
    return result


def compile_and_run(compiler: str, source: str, name: str, tmp_path: Path):
    cpp, binary = tmp_path / f"{name}.cpp", tmp_path / name
    cpp.write_text(source, encoding="utf-8")
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(binary)], check=True, timeout=30)
    return subprocess.run([str(binary)], capture_output=True, timeout=30)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_setup_and_draw_paths_preserve_state_and_arguments(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    result = compile_and_run(compiler, fixture(), "buffer_sprite", tmp_path)
    assert result.returncode == 0, result.stderr.decode(errors="replace")


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_setup_and_forwarding_mutants_compile_but_fail(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    bad_colour = PREPARE.replace("if (colour == -1)", "if (colour != -1)", 1)
    assert bad_colour != PREPARE
    result = compile_and_run(compiler, fixture(prepare=bad_colour), "bad_colour", tmp_path)
    assert result.returncode != 0

    bad_forward = MONO.replace("destination, transparent, x, y, colour, numerator, denominator",
                               "destination, WinFillColour, x, y, colour, numerator, denominator", 1)
    assert bad_forward != MONO
    result = compile_and_run(compiler, fixture(mono=bad_forward), "bad_forward", tmp_path)
    assert result.returncode != 0

    clear_before_draw = MONO.replace(
        "    int result = BufferSpriteOwner.sprite().draw_mono(",
        "    BufferSpriteOwner.sprite().pcBits_ = 0;\n"
        "    int result = BufferSpriteOwner.sprite().draw_mono(", 1)
    assert clear_before_draw != MONO
    result = compile_and_run(compiler, fixture(mono=clear_before_draw),
                             "clear_before_draw", tmp_path)
    assert result.returncode != 0

    wrong_target = MONO.replace("        destination, transparent, x, y, colour, numerator, denominator)",
                                "        destination + 1, transparent, x, y, colour, numerator, denominator)", 1)
    assert wrong_target != MONO
    result = compile_and_run(compiler, fixture(mono=wrong_target),
                             "wrong_target", tmp_path)
    assert result.returncode != 0
