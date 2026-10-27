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
    BYTE caller_output_backing[192];
    MediaDecoderContext decoder_context_3f8;
};
static MediaDescriptor *pixel_pointer_bits;
static BYTE *compressed_base;
static int wanted_pitch;
static std::vector<BYTE *> block_outputs;
void sub_5d1278(BYTE *output, int pitch, MediaDecoderContext *context) {
    assert(pitch == wanted_pitch);
    assert(context->compressed_input_200 == compressed_base);
    assert(context->bit_position_204 == (int)block_outputs.size());
    assert(context->predictor_500 == 0 && context->predictor_4fc == 0 &&
           context->predictor_4f8 == 0);
    if (pixel_pointer_bits && block_outputs.empty())
        std::memcpy(output, &pixel_pointer_bits, sizeof(pixel_pointer_bits));
    else {
        const DWORD pixel = 0x01234567;
        std::memcpy(output, &pixel, sizeof(pixel));
    }
    block_outputs.push_back(output);
    ++context->bit_position_204;
}
'''
MAIN = r'''
void reset_context(MediaRuntime &runtime, BYTE *input, int pitch) {
    runtime.decoder_context_3f8.bit_position_204 = 99;
    runtime.decoder_context_3f8.predictor_500 = 12;
    runtime.decoder_context_3f8.predictor_4fc = 13;
    runtime.decoder_context_3f8.predictor_4f8 = 14;
    compressed_base = input; wanted_pitch = pitch;
    block_outputs.clear();
}
int main() {
    BYTE input[64] = {}, output[256] = {};
    MediaDescriptor first = {32, 32}, replacement = {16, 16};
    MediaRuntime runtime = {};
    runtime.descriptor = &first;
    pixel_pointer_bits = 0;
    reset_context(runtime,input,8);
    sub_5cda30(&runtime,input,output,0,8);
    assert(block_outputs.size() == 4);
    assert(block_outputs[0] == output && block_outputs[1] == output+32);
    assert(block_outputs[2] == output+128 && block_outputs[3] == output+160);

    // The first output pixel bytes replace the runtime's descriptor pointer.
    // A cached owner incorrectly decodes a second block from the old width.
    first.output_height_60 = 16;
    runtime.descriptor = &first;
    pixel_pointer_bits = &replacement;
    reset_context(runtime,input,0);
    sub_5cda30(&runtime,input,reinterpret_cast<BYTE *>(&runtime),0,0);
    assert(runtime.descriptor == &replacement && block_outputs.size() == 1);

    // Old width already ends the row; the new height must also stop next row.
    first.output_height_60 = 32; first.output_width_64 = 16;
    runtime.descriptor = &first;
    reset_context(runtime,input,0);
    sub_5cda30(&runtime,input,reinterpret_cast<BYTE *>(&runtime),0,0);
    assert(runtime.descriptor == &replacement && block_outputs.size() == 1);

    runtime.descriptor = &first;
    first.output_height_60 = 0;
    pixel_pointer_bits = 0;
    reset_context(runtime,input,0);
    sub_5cda30(&runtime,input,output,0,0);
    assert(block_outputs.empty());
    assert(runtime.decoder_context_3f8.bit_position_204 == 0);
}
'''


@pytest.mark.parametrize("mutation", (None, "cache-owner", "skip-context-reset"))
def test_original_live_descriptor_loop(tmp_path, mutation):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    row, = (r for r in read_file(ROOT / "src/movie_media.cpp")
            if r.address == 0x005CDA30)
    lo, hi = _function_body_bytes(row.region, "sub_5cda30")
    body = row.region.encode()[lo:hi].decode()
    if mutation == "cache-owner":
        assert "runtime->descriptor->output_width_64" in body
        body = body.replace("runtime->descriptor->output_", "cached->output_")
        body = "{\nMediaDescriptor *cached = runtime->descriptor;" + body[1:]
    elif mutation == "skip-context-reset":
        anchor = "runtime->decoder_context_3f8.bit_position_204 = 0;"
        assert body.count(anchor) == 1
        body = body.replace(anchor, "(void)0;")
    signature = ('extern "C" void __fastcall sub_5cda30(MediaRuntime *runtime, '
                 'BYTE *compressed, BYTE *destination, BYTE *unused, '
                 'unsigned int pitch) ')
    source = tmp_path / "block.cpp"
    source.write_text(PRELUDE + signature + body + MAIN)
    program = tmp_path / "block"
    subprocess.run([compiler, "-std=c++11", "-O1", "-fsanitize=address,undefined",
                    "-fno-sanitize-recover=all", str(source), "-o", str(program)],
                   check=True, capture_output=True)
    result = subprocess.run([str(program)], capture_output=True,
                            env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))
    assert (result.returncode == 0) == (mutation is None), result.stderr.decode()
    if mutation is not None:
        assert b"assert" in result.stderr.lower(), result.stderr.decode()
