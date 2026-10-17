"""Actual CRT wrappers and observed original wrap/call effects.

Host -fwrapv explicitly models the observed32-bit wrap; these doubles do not
prove portable C++ overflow behavior or identify the original runtime provider.
"""
from __future__ import annotations

import os
import re
import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/general.cpp").is_file())
SOURCE = ROOT / "src/crt_random.cpp"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
TOKEN = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|[{}]', re.S)
SIGNATURES = {
    "seed": "void __cdecl my_srand(int reseed)",
    "bounded": "uint32_t __cdecl rnd(int bounds, LPSTR UNUSED(input))",
}


def definition(text: str, signature: str) -> str:
    hits = [match for match in re.finditer(re.escape(signature), text)]
    assert len(hits) == 1, (signature, len(hits))
    start = hits[0].start()
    opened = False
    depth = 0
    for token in TOKEN.finditer(text, hits[0].end()):
        if token.group() == "{":
            opened = True
            depth += 1
        elif token.group() == "}":
            depth -= 1
            if opened and depth == 0:
                return text[start:token.end()]
    raise AssertionError("unterminated actual body: " + signature)


def bodies(source_text: str, mutant: str | None = None) -> str:
    extracted = {name: definition(source_text, signature)
                 for name, signature in SIGNATURES.items()}
    if mutant == "duplicate_srand":
        old = "    srand(reseed);"
        assert extracted["seed"].count(old) == 1
        extracted["seed"] = extracted["seed"].replace(old, old + "\n" + old, 1)
    elif mutant == "srand_before_log":
        old = '    log_say("Reseed to", reseed, 0, 0);\n    srand(reseed);'
        assert extracted["seed"].count(old) == 1
        extracted["seed"] = extracted["seed"].replace(
            old, '    srand(reseed);\n    log_say("Reseed to", reseed, 0, 0);', 1)
    elif mutant == "guard_bounds_le_one":
        old = "if (bounds - 1 <= 0)"
        assert extracted["bounded"].count(old) == 1
        extracted["bounded"] = extracted["bounded"].replace(old, "if (bounds <= 1)", 1)
    elif mutant == "guard_bounds_lt_one":
        old = "if (bounds - 1 <= 0)"
        assert extracted["bounded"].count(old) == 1
        extracted["bounded"] = extracted["bounded"].replace(old, "if (bounds < 1)", 1)
    elif mutant == "rand_mod_bounds_minus_one":
        old = "return rand() % bounds;"
        assert extracted["bounded"].count(old) == 1
        extracted["bounded"] = extracted["bounded"].replace(
            old, "return rand() % (bounds - 1);", 1)
    elif mutant == "duplicate_rand":
        old = "return rand() % bounds;"
        assert extracted["bounded"].count(old) == 1
        extracted["bounded"] = extracted["bounded"].replace(
            old, "const int value = rand();\n    (void)rand();\n    return value % bounds;", 1)
    elif mutant is not None:
        raise AssertionError("unknown mutation: " + mutant)
    return "\n".join(extracted.values())


HOST = r"""
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#define __cdecl
#define UNUSED(name) name __attribute__((unused))
using LPSTR = char *;
using uint32_t = unsigned int;
static std::vector<std::string> events;
static int crt_seed = -1;
static unsigned crt_draw_count = 0;
static int next_rand_value = 0;
static uint32_t random_lcg_state = 0;
static int last_log_seed = -1;

static void host_log_say(const char *label, int seed, int a, int b) {
    events.push_back(std::string("log:") + label + ":" + std::to_string(seed) + ":" +
                     std::to_string(a) + ":" + std::to_string(b));
    last_log_seed = seed;
}
static void host_srand(unsigned seed) {
    events.push_back("srand:" + std::to_string(seed));
    crt_seed = static_cast<int>(seed);
    crt_draw_count = 0;
}
static int host_rand() {
    events.push_back("rand");
    ++crt_draw_count;
    return next_rand_value;
}
#define log_say host_log_say
#define srand host_srand
#define rand host_rand

@@BODIES@@

#undef log_say
#undef srand
#undef rand

static void expect(const std::vector<std::string> &wanted) {
    assert(events == wanted);
}

int main() {
    events.clear(); crt_seed = -1; crt_draw_count = 0;
    random_lcg_state = 0x13579bdfu;
    my_srand(12345);
    expect({"log:Reseed to:12345:0:0", "srand:12345"});
    assert(crt_seed == 12345 && crt_draw_count == 0 && last_log_seed == 12345);
    assert(random_lcg_state == 0x13579bdfu);

    // Ordinary <=1 bounds return before touching the CRT stream, including
    // negative values. The input pointer is unused and must remain unchanged.
    char input[] = "untouched";
    for (int bound : {1, 0, -1, -23}) {
        events.clear();
        const unsigned before = crt_draw_count;
        assert(rnd(bound, input) == 0);
        expect({});
        assert(crt_draw_count == before);
        assert(std::string(input) == "untouched");
    }

    events.clear(); next_rand_value = 17;
    const unsigned before_positive = crt_draw_count;
    assert(rnd(6, input) == 5u);
    expect({"rand"});
    assert(crt_draw_count == before_positive + 1);
    assert(std::string(input) == "untouched");

    events.clear(); next_rand_value = 7;
    const unsigned before_min = crt_draw_count;
    // On x86, LEA eax,[esi-1] wraps INT_MIN-1 to INT_MAX, so JG takes
    // the rand/idiv path. -fwrapv models that observed 32-bit wrap here.
    assert(rnd((-2147483647 - 1), input) == 7u);
    expect({"rand"});
    assert(crt_draw_count == before_min + 1);
    assert(std::string(input) == "untouched");
    assert(random_lcg_state == 0x13579bdfu);
    return 0;
}
"""


def pinned_evidence() -> None:
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source.asm import Listing

    image = Path(os.environ.get("OPENSMACX_IMAGE", str(ROOT / ".opensmacx/game/terranx_original.exe")))
    if not image.is_file():
        pytest.skip("pinned image unavailable; host controls do not replace original anchors")
    pe = PeImage(image.read_bytes(), PIN)

    def listing(start: int, end: int) -> list[str]:
        return Listing(pe.read(Span(start, end)), start).lines

    seed = listing(0x00538FB0, 0x00538FD2)
    seed_calls = [row.split("call ", 1)[1].strip() for row in seed if "call " in row]
    assert seed_calls == ["0x6262f0", "0x646010"]
    assert "push 0" in seed[4] and "push 0" in seed[5]
    assert "push esi" in seed[6]
    assert "push 0x68d430" in seed[7]  # "Reseed to"
    assert any("add esp, 0x14" in row for row in seed)

    bounded = listing(0x00579770, 0x00579790)
    assert any("lea eax, [esi - 1]" in row for row in bounded)
    assert any("jg 0x579783" in row for row in bounded)
    assert any("call 0x64601d" in row for row in bounded)
    assert any("idiv esi" in row for row in bounded)
    # INT_MIN-1 is the wrapping edge: LEA computes 0x7fffffff and signed JG
    # enters rand/idiv. This is original behavior, not an invitation to fix it.
    source = SOURCE.read_text(encoding="utf-8")
    assert "// ORIGINAL: 0x00538FB0 ?my_srand@@YAHH@Z 0x00538FB0-0x00538FD2 BYTE_EXACT" in source
    assert "// ORIGINAL: 0x00579770 ?rnd@@YAHHPAD@Z 0x00579770-0x00579790 BYTE_EXACT" in source


def compile_and_run(compiler: str, source_text: str, mutant: str | None) -> bool:
    generated = HOST.replace("@@BODIES@@", bodies(source_text, mutant))
    with tempfile.TemporaryDirectory(prefix="osmx-crt-rng-host-") as directory:
        cpp = Path(directory) / "crt_rng.cpp"
        executable = Path(directory) / "crt_rng"
        cpp.write_text(generated, encoding="utf-8")
        built = subprocess.run(
            [compiler, "-std=c++17", "-O2", "-fwrapv", "-Wall", "-Wextra", "-Werror",
             str(cpp), "-o", str(executable)],
            capture_output=True, text=True, timeout=60)
        assert built.returncode == 0, built.stdout + built.stderr
        result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
        return result.returncode == 0


def test_pinned_crt_wrapper_instruction_evidence():
    pinned_evidence()


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("mutant", (
    None, "duplicate_srand", "srand_before_log", "guard_bounds_le_one",
    "guard_bounds_lt_one", "rand_mod_bounds_minus_one", "duplicate_rand",
))
def test_actual_crt_call_effects_and_compiling_damage(compiler_name, mutant):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable: " + compiler_name)
    source_text = SOURCE.read_text(encoding="utf-8")
    passed = compile_and_run(compiler, source_text, mutant)
    assert passed == (mutant is None), (compiler_name, mutant, passed)
