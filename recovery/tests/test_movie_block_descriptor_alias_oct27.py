"""Actual block-loop behavior when caller pixel output aliases runtime storage.

The decoder double writes through its supplied byte destination with memcpy,
as the production packed-pixel helper does. It has no mutation callback into
the runtime. Host layouts are not target ABI witnesses.
"""
from pathlib import Path
import os
import shutil
import subprocess

import pytest
from match_recovery.local_variants import _function_body_bytes
from match_recovery.source import read_file

ROOT = Path(__file__).resolve().parents[2]

PRELUDE = r'''
#include <cassert>
#include <cstddef>
#include <cstring>
#include <vector>
typedef unsigned char BYTE;
typedef unsigned int DWORD;
#define __fastcall
struct MediaDescriptor { DWORD output_height_60, output_width_64; };
struct MediaDecoderContext {
    BYTE *compressed_input_200;
    int bit_position_204, predictor_500, predictor_4fc, predictor_4f8;
};
struct MediaRuntime {
    MediaDescriptor *descriptor;
    DWORD record_option_040;
    BYTE caller_output_backing[192];
    MediaDecoderContext decoder_context_3f8;
};
static MediaDescriptor *pixel_pointer_bits;
static BYTE *compressed_base;
static int wanted_pitch;
static std::vector<BYTE *> block_outputs;
static std::vector<int> block_kinds;
static bool clear_option_pixel;
void pixel_block(int kind, BYTE *output, int pitch, MediaDecoderContext *context) {
    assert(pitch == wanted_pitch);
    assert(context->compressed_input_200 == compressed_base);
    assert(context->bit_position_204 == (int)block_outputs.size());
    assert(context->predictor_500 == 0 && context->predictor_4fc == 0 &&
           context->predictor_4f8 == 0);
    if (pixel_pointer_bits && block_outputs.empty())
        std::memcpy(output, &pixel_pointer_bits, sizeof(pixel_pointer_bits));
    else {
        const DWORD pixel = clear_option_pixel ? 0 : 0x01234567;
        std::memcpy(output, &pixel, sizeof(pixel));
    }
    block_outputs.push_back(output);
    block_kinds.push_back(kind);
    ++context->bit_position_204;
}
void sub_5d1466(BYTE *p,int pitch,MediaDecoderContext *c) { pixel_block(1,p,pitch,c); }
void sub_5d1842(BYTE *p,int pitch,MediaDecoderContext *c) { pixel_block(2,p,pitch,c); }
void sub_5d1654(BYTE *p,int pitch,MediaDecoderContext *c) { pixel_block(3,p,pitch,c); }
void sub_5d1a30(BYTE *p,int pitch,MediaDecoderContext *c) { pixel_block(4,p,pitch,c); }
void sub_5d1278(BYTE *p,int pitch,MediaDecoderContext *c) { pixel_block(5,p,pitch,c); }
'''
MAIN = r'''
void reset_context(MediaRuntime &runtime, BYTE *input, int pitch) {
    runtime.decoder_context_3f8.bit_position_204 = 99;
    runtime.decoder_context_3f8.predictor_500 = 12;
    runtime.decoder_context_3f8.predictor_4fc = 13;
    runtime.decoder_context_3f8.predictor_4f8 = 14;
    compressed_base = input; wanted_pitch = pitch;
    block_outputs.clear(); block_kinds.clear();
}
typedef void (*BlockLoop)(MediaRuntime *,BYTE *,BYTE *,BYTE *,unsigned int);
void run_loop(BlockLoop loop,int block_step,int kind_true,int kind_false) {
    BYTE input[64] = {}, output[256] = {};
    MediaDescriptor first = {32, 32}, replacement = {16, 16};
    MediaRuntime runtime = {};
    runtime.descriptor = &first;
    pixel_pointer_bits = 0;
    clear_option_pixel = false;
    for (int option=0;option<2;++option) {
        runtime.record_option_040=option;
        reset_context(runtime,input,8);
        loop(&runtime,input,output,0,8);
        assert(block_outputs.size() == 4);
        assert(block_outputs[0] == output && block_outputs[1] == output+block_step);
        assert(block_outputs[2] == output+128 && block_outputs[3] == output+128+block_step);
        for (unsigned int i=0;i<block_kinds.size();++i)
            assert(block_kinds[i] == (option ? kind_true : kind_false));
    }

    // The first output pixel bytes replace the runtime's descriptor pointer.
    // A cached owner incorrectly decodes a second block from the old width.
    first.output_height_60 = 16;
    runtime.descriptor = &first;
    pixel_pointer_bits = &replacement;
    reset_context(runtime,input,0);
    loop(&runtime,input,reinterpret_cast<BYTE *>(&runtime),0,0);
    assert(runtime.descriptor == &replacement && block_outputs.size() == 1);

    // Old width already ends the row; the new height must also stop next row.
    first.output_height_60 = 32; first.output_width_64 = 16;
    runtime.descriptor = &first;
    reset_context(runtime,input,0);
    loop(&runtime,input,reinterpret_cast<BYTE *>(&runtime),0,0);
    assert(runtime.descriptor == &replacement && block_outputs.size() == 1);

    runtime.descriptor = &first;
    first.output_height_60 = 0;
    pixel_pointer_bits = 0;
    reset_context(runtime,input,0);
    loop(&runtime,input,output,0,0);
    assert(block_outputs.empty());
    assert(runtime.decoder_context_3f8.bit_position_204 == 0);

    if (kind_true != kind_false) {
        first.output_height_60=16; first.output_width_64=32;
        runtime.record_option_040=1;
        clear_option_pixel=true;
        reset_context(runtime,input,0);
        BYTE *option_output=reinterpret_cast<BYTE *>(&runtime)+
            offsetof(MediaRuntime,record_option_040);
        loop(&runtime,input,option_output,0,0);
        assert(block_outputs.size()==2);
        assert(block_kinds[0]==kind_true && block_kinds[1]==kind_false);
    }
}
int main() {
    run_loop(sub_5cd8d0,32,1,2);
    run_loop(sub_5cd980,64,3,4);
    run_loop(sub_5cda30,32,5,5);
}
'''


@pytest.mark.parametrize("mutation", (None, "cache-owner", "skip-context-reset", "cache-option"))
def test_original_live_descriptor_loop(tmp_path, mutation):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    rows = read_file(ROOT / "src/movie_media.cpp")
    definitions = []
    for address, name in ((0x5CD8D0, "sub_5cd8d0"), (0x5CD980, "sub_5cd980"),
                          (0x5CDA30, "sub_5cda30")):
        row, = (r for r in rows if r.address == address)
        lo, hi = _function_body_bytes(row.region, name)
        body = row.region.encode()[lo:hi].decode()
        if mutation == "cache-owner":
            assert "runtime->descriptor->output_width_64" in body
            body = body.replace("runtime->descriptor->output_", "cached->output_")
            body = "{\nMediaDescriptor *cached = runtime->descriptor;" + body[1:]
        elif mutation == "skip-context-reset" and address == 0x5CDA30:
            anchor = "runtime->decoder_context_3f8.bit_position_204 = 0;"
            assert body.count(anchor) == 1
            body = body.replace(anchor, "(void)0;")
        elif mutation == "cache-option" and address == 0x5CD8D0:
            anchor = "runtime->record_option_040"
            assert body.count(anchor) == 1
            body = body.replace(anchor, "cached_option")
            body = "{\nDWORD cached_option=runtime->record_option_040;" + body[1:]
        signature = ('extern "C" void __fastcall ' + name + '(MediaRuntime *runtime, '
                     'BYTE *compressed, BYTE *destination, BYTE *unused, '
                     'unsigned int pitch) ')
        definitions.append(signature + body)
    source = tmp_path / "block.cpp"
    source.write_text(PRELUDE + "\n".join(definitions) + MAIN)
    program = tmp_path / "block"
    subprocess.run([compiler, "-std=c++11", "-O1", "-fsanitize=address,undefined",
                    "-fno-sanitize-recover=all", str(source), "-o", str(program)],
                   check=True, capture_output=True)
    result = subprocess.run([str(program)], capture_output=True,
                            env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))
    assert (result.returncode == 0) == (mutation is None), result.stderr.decode()
    if mutation is not None:
        assert b"assert" in result.stderr.lower(), result.stderr.decode()
