"""Host controls for Sound's inherited slot-24 signed flag contract.

The fixture extracts and executes the product method rather than copying its
implementation as an oracle. Host execution checks ordinary 32-bit integer
semantics under the host compiler's signed-conversion and arithmetic-shift
rules; the optional leased VC6/Wine control separately exercises the configured
32-bit compiler/runtime. Neither is whole-game acceptance evidence.
"""
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/sound.cpp"
HEADER = ROOT / "src/sound.h"
SIGNATURE = "int Sound::unk_slot24()"
EXPRESSION = "static_cast<int>(flags_40_ << 31) >> 31"


def product_body() -> str:
    source = SOURCE.read_text(encoding="utf-8")
    from match_recovery.includes import COMMENTS_OR_STRINGS

    code = COMMENTS_OR_STRINGS.sub(lambda match: "\n" * match[0].count("\n") + " " *
                                  (len(match[0]) - match[0].count("\n")), source)
    assert code.count(SIGNATURE) == 1, f"expected exactly one {SIGNATURE!r} definition"
    start = code.index(SIGNATURE)
    opening = code.index("{", start)
    depth = 0
    for end in range(opening, len(code)):
        if code[end] == "{":
            depth += 1
        elif code[end] == "}":
            depth -= 1
            if depth == 0:
                body = source[start:end + 1]
                assert EXPRESSION in body, "product sign-extension expression changed"
                return body
    raise AssertionError(f"unterminated product definition: {SIGNATURE}")


def verify_identity_and_storage() -> None:
    source = SOURCE.read_text(encoding="utf-8")
    header = HEADER.read_text(encoding="utf-8")
    assert "// ORIGINAL: 0x00448380 sub_448380 0x00448380-0x0044838A BYTE_EXACT" in source
    assert "?unk_slot24@Sound@@UAEHXZ" in source
    assert "uint32_t flags_40_;" in header
    product_body()


HOST_FIXTURE = r"""
#include <limits.h>
#include <stdio.h>

typedef char sound_int_must_be_32_bits[(sizeof(int) == 4) ? 1 : -1];
typedef char sound_uint_must_be_32_bits[(sizeof(unsigned int) == 4) ? 1 : -1];
typedef char sound_int_range_must_be_32_bits[(INT_MAX == 2147483647) ? 1 : -1];

class Sound {
public:
    unsigned int flags_40_;
    int unk_slot24();
};

__PRODUCT_BODY__

static int check(unsigned int value) {
    Sound sound;
    sound.flags_40_ = value;
    const unsigned int before = sound.flags_40_;
    const int result = sound.unk_slot24();
    const int expected = (value & 1u) ? -1 : 0;
    if (result != expected || sound.flags_40_ != before) {
        fprintf(stderr, "flags=%08x result=%d expected=%d after=%08x\n",
                value, result, expected, sound.flags_40_);
        return 1;
    }
    return 0;
}

int main() {
    int failures = 0;
    const unsigned int edges[] = {
        0u, 1u, 2u, 3u, 0x7fffffffu, 0x80000000u,
        0x80000001u, 0xfffffffeu, 0xffffffffu
    };
    for (unsigned int i = 0; i < sizeof(edges) / sizeof(edges[0]); ++i)
        failures += check(edges[i]);
    for (unsigned int bit = 0; bit < 32; ++bit)
        failures += check(1u << bit);
    return failures ? 1 : 0;
}
"""


def fixture_source(body: str, mutant=None) -> str:
    if mutant is not None:
        before, after = mutant
        assert before in body, f"mutant anchor missing in product function: {before!r}"
        body = body.replace(before, after, 1)
    assert HOST_FIXTURE.count("__PRODUCT_BODY__") == 1
    return HOST_FIXTURE.replace("__PRODUCT_BODY__", body)


def build_host(body: str, tmp_path: Path, compiler: str, name: str, mutant=None) -> Path:
    source = tmp_path / (name + ".cpp")
    executable = tmp_path / name
    source.write_text(fixture_source(body, mutant), encoding="utf-8")
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(executable)], check=True,
                   capture_output=True, text=True, timeout=30)
    return executable


def run_host(executable: Path) -> subprocess.CompletedProcess:
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)


HOST_COMPILERS = tuple((name, shutil.which(name)) for name in ("g++", "clang++")
                       if shutil.which(name))


def test_product_identity_and_host_compiler_availability():
    verify_identity_and_storage()
    assert HOST_COMPILERS, "at least one of g++ or clang++ is required for this control"


@pytest.mark.parametrize("compiler_name,compiler", HOST_COMPILERS,
                         ids=[name for name, _ in HOST_COMPILERS])
def test_actual_product_method_sign_extends_only_flag_bit_zero(compiler_name, compiler, tmp_path):
    del compiler_name
    executable = build_host(product_body(), tmp_path, compiler, "sound-slot24-good")
    result = run_host(executable)
    assert result.returncode == 0, f"{result.stdout}{result.stderr} ({result.returncode})"


@pytest.mark.parametrize("compiler_name,compiler", HOST_COMPILERS,
                         ids=[name for name, _ in HOST_COMPILERS])
def test_host_control_rejects_wrong_bit_logical_shift_and_positive_boolean(
        compiler_name, compiler, tmp_path):
    del compiler_name
    body = product_body()
    mutants = (
        (EXPRESSION, "static_cast<int>(flags_40_ << 30) >> 31"),
        (EXPRESSION, "static_cast<unsigned int>(static_cast<int>(flags_40_ << 31)) >> 31"),
        (EXPRESSION, "(flags_40_ & 1u) != 0"),
    )
    for index, mutant in enumerate(mutants):
        executable = build_host(body, tmp_path, compiler, f"sound-slot24-mutant-{index}", mutant)
        result = run_host(executable)
        assert result.returncode != 0, (
            f"host control missed mutant {index}: {result.stdout}{result.stderr}")


def test_vc6_x86_actual_source_behavior_and_mutants(tmp_path):
    """Optional target-ABI control; skipped unless the coordinator supplied a lease."""
    if not os.environ.get("MATCH_RECOVERY_RESOURCE_LEASE"):
        pytest.skip("native Sound slot-24 controls require an existing coordinator compiler lease")
    from match_recovery.measurement import Measurement
    from match_recovery.processes import run_bounded
    from recovery.environment.compile import compile_object
    from recovery.environment.wine import VC6_ROOT, environment

    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None:
        pytest.skip("VC6 and Wine32 are required under the coordinator lease")
    settings = Measurement.read(ROOT)
    wrapper = ROOT / "recovery/environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")

    def build_native(name, flags, mutant=None):
        source = tmp_path / (name + ".cpp")
        obj = tmp_path / (name + ".obj")
        exe = tmp_path / (name + ".exe")
        source.write_text(fixture_source(product_body(), mutant), encoding="utf-8")
        obj.write_bytes(compile_object(source, command, tuple(flags) + settings.layout_flags))
        linked = run_bounded([sys.executable, str(wrapper), "--tool=link", "/nologo",
                              "/subsystem:console", "/incremental:no", "/out:" + str(exe),
                              str(obj), "kernel32.lib"], cwd=tmp_path, timeout=60)
        assert linked.returncode == 0, (linked.stdout + linked.stderr).decode(errors="replace")
        return exe

    def run_native(executable):
        with tempfile.TemporaryFile() as diagnostics:
            result = subprocess.run(["wine32", str(executable)], env=environment(),
                                    stdout=diagnostics, stderr=diagnostics, timeout=30)
            diagnostics.seek(0)
            output = diagnostics.read().decode(errors="replace")
        return result.returncode, output

    mutants = (
        (EXPRESSION, "static_cast<int>(flags_40_ << 30) >> 31"),
        (EXPRESSION, "static_cast<unsigned int>(static_cast<int>(flags_40_ << 31)) >> 31"),
        (EXPRESSION, "(flags_40_ & 1u) != 0"),
    )
    for profile_index, flags in enumerate(settings.flag_sets):
        good = build_native("sound-slot24-good-" + str(profile_index), flags)
        code, output = run_native(good)
        assert code == 0, f"profile {profile_index}: {output} ({code})"
        for mutant_index, mutant in enumerate(mutants):
            damaged = build_native("sound-slot24-bad-%d-%d" % (profile_index, mutant_index),
                                   flags, mutant)
            code, output = run_native(damaged)
            assert code != 0, (
                f"native control missed profile {profile_index} mutant {mutant_index}: {output}")
