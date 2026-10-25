"""Execute the actual movie LZ body against pinned-instruction command fixtures."""
from __future__ import annotations

import os
from pathlib import Path
import shutil
import signal
import subprocess
import sys

import pytest

ROOT = Path(os.environ.get("OPENSMACX_TEST_ROOT", Path(__file__).resolve().parents[2]))
sys.path.insert(0, str(ROOT / "recovery/tests"))
from test_movie_runtime_decoder_ownership_actual_source_oct9 import _definition

COMPILERS = [path for name in ("g++", "clang++") if (path := shutil.which(name))]
SIGNATURE = 'extern "C" int __cdecl sub_5d427c('
PRELUDE = r'''
#include <cassert>
#include <cstring>
#include <vector>
#define __cdecl
'''
FIXTURES = r'''
typedef unsigned char Byte;
static void check(const std::vector<Byte>& commands,
                  const std::vector<Byte>& history,
                  const std::vector<Byte>& expected, bool extended = false) {
    // Aligned allocation and readable trailing padding retain the original's
    // speculative four-byte loads. The zero declared size does not stop decoding.
    std::vector<unsigned short> storage((commands.size() + 32) / 2, 0);
    Byte *packet = reinterpret_cast<Byte *>(storage.data());
    packet[0] = extended ? 0x11 : 0x10;
    packet[1] = 0xfb;
    if (extended) { packet[2] = 0x71; packet[3] = 0x82; packet[4] = 0x93; }
    const unsigned int header = extended ? 8 : 5;
    std::memcpy(packet + header, commands.data(), commands.size());
    std::vector<Byte> output(history.size() + expected.size() + 16, 0xa5);
    std::memcpy(output.data(), history.data(), history.size());
    (void)sub_5d427c(storage.data(), output.data() + history.size(), 1);
    assert(std::memcmp(output.data(), history.data(), history.size()) == 0);
    assert(std::memcmp(output.data() + history.size(), expected.data(), expected.size()) == 0);
    for (unsigned int i = history.size() + expected.size(); i < output.size(); ++i)
        assert(output[i] == 0xa5);
}

static void discriminator() {
    std::vector<Byte> history(65000, 0x31);
    const Byte word[] = {'H','E','L','L','O'};
    std::memcpy(history.data() + 487, word, 5);
    // C0 FC 00 00: no literals; copy five from distance64513.
    // Correct cursor then reaches FD and its final Z. With the old +1 it
    // reaches the FC embedded in the header and exits safely before Z.
    check({0xc0,0xfc,0,0,0xfd,'Z'}, history, {'H','E','L','L','O','Z'});
}

int main() {
    discriminator();
#ifndef ONLY_DISCRIMINATOR
    const std::vector<Byte> history = {'A','B','C','D'};
    // Short commands: two-byte header, 0..3 literals, length3..10.
    check({0,0,0xfc}, history, {'D','D','D'});
    check({1,0,'x',0xfc}, history, {'x','x','x','x'});
    check({2,0,'x','y',0xfc}, history, {'x','y','y','y','y'});
    check({7,3,'x','y','z',0xfc}, history, {'x','y','z','D','x','y','z'});
    check({0x1c,0,0xfc}, history, std::vector<Byte>(10,'D'));
    // Medium commands: three-byte header; second byte's top two bits are literals.
    check({0x80,0,0,0xfc}, history, std::vector<Byte>(4,'D'));
    check({0x80,0x40,0,'x',0xfc}, history, std::vector<Byte>(5,'x'));
    check({0x80,0x80,0,'x','y',0xfc}, history, {'x','y','y','y','y','y'});
    check({0x80,0xc0,3,'x','y','z',0xfc}, history, {'x','y','z','D','x','y','z'});
    std::vector<Byte> medium_history(800, 0x31);
    const Byte digits[] = {'1','2','3','4','5','6','7','8','9'};
    std::memcpy(medium_history.data() + 29, digits, 9);
    check({0x85,0x83,4,'a','b',0xfc}, medium_history,
          {'a','b','1','2','3','4','5','6','7','8','9'});
    check({0xbf,0,0,0xfc}, history, std::vector<Byte>(67,'D'));
    // Long commands: four-byte header; low two token bits are literals.
    check({0xc0,0,0,0,0xfc}, history, std::vector<Byte>(5,'D'));
    check({0xc1,0,0,0,'x',0xfc}, history, std::vector<Byte>(6,'x'), true);
    check({0xc2,0,2,0,'x','y',0xfc}, history, {'x','y','D','x','y','D','x'});
    check({0xc3,0,3,0,'x','y','z',0xfc}, history, {'x','y','z','D','x','y','z','D'});
    check({0xcc,0,0,0xff,0xfc}, history, std::vector<Byte>(1028,'D'));
    std::vector<Byte> far_history(65540, 0x31);
    const Byte far_word[] = {'V','W','X','Y','Z'};
    std::memcpy(far_history.data() + 3, far_word, 5);
    check({0xd0,0,0,0,0xfc}, far_history, {'V','W','X','Y','Z'});
    // Literal-only commands retain their one-byte header and DWORD count.
    check({0xe0,'A','B','C','D',0xfc}, history, {'A','B','C','D'});
    std::vector<Byte> literal(112);
    for (unsigned int i = 0; i != 112; ++i) literal[i] = static_cast<Byte>(i + 1);
    std::vector<Byte> commands(1, 0xfb);
    commands.insert(commands.end(), literal.begin(), literal.end());
    commands.push_back(0xfc);
    check(commands, history, literal);
    check({0xfc}, history, {});
    check({0xfd,'Z'}, history, {'Z'});
    check({0xfe,'Y','Z'}, history, {'Y','Z'});
    check({0xff,'X','Y','Z'}, history, {'X','Y','Z'});
    unsigned short packet[8] = {0xfb11};
    Byte output[4] = {1,2,3,4};
    (void)sub_5d427c(packet, output, 0);
    (void)sub_5d427c(0, output, 1);
    assert(output[0] == 1 && output[1] == 2 && output[2] == 3 && output[3] == 4);
#endif
    // No return-value claim: original POPAD restores incoming EAX. These
    // bounded controls establish output effects, not stronger register ABI.
    return 0;
}
'''


@pytest.mark.parametrize("compiler", COMPILERS)
@pytest.mark.parametrize("damaged", [False, True], ids=["actual", "wrong-cursor-control"])
def test_actual_movie_lz_commands(tmp_path: Path, compiler: str, damaged: bool) -> None:
    body = _definition((ROOT / "src/general.cpp").read_text(), SIGNATURE)
    if damaged:
        anchor = "} else {\n              puVar8 = puVar8 + 4;"
        assert body.count(anchor) == 1
        body = body.replace(anchor, "} else {\n              puVar8 = puVar8 + 1;")
    source = tmp_path / "decoder.cpp"
    binary = tmp_path / "decoder"
    source.write_text(PRELUDE + body + FIXTURES)
    command = [compiler, "-std=c++17", "-fno-strict-aliasing", str(source), "-o", str(binary)]
    if damaged:
        command.insert(1, "-DONLY_DISCRIMINATOR")
    subprocess.run(command, check=True, timeout=60)
    result = subprocess.run([str(binary)], check=False, timeout=10, capture_output=True)
    if damaged:
        assert result.returncode == -signal.SIGABRT
        assert b"expected.data()" in result.stderr
    else:
        assert result.returncode == 0
