"""Actual-source C++98 controls for the three provisional word-state methods."""
from pathlib import Path
import json
import shutil
import subprocess
import tempfile

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS

ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())
HEADER = ROOT / "src/word_state.h"
SOURCE = ROOT / "src/word_state.cpp"

HARNESS = r'''#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include "word_state.h"

int main() {
    // These extents belong only to the bounded source/test types. They do not
    // establish complete original allocations or relationships between types.
    assert(sizeof(PrefixFlagState) == 12);
    assert(offsetof(PrefixFlagState, word_04_) == 4);
    assert(offsetof(PrefixFlagState, word_08_) == 8);
    assert(sizeof(NineWordSentinelState) == 36);
    assert(offsetof(NineWordSentinelState, word_20_) == 32);
    assert(sizeof(PairWordState) == 12);
    assert(offsetof(PairWordState, word_08_) == 8);

    struct GuardedPrefix { uint32_t before; PrefixFlagState state; uint32_t after; };
    GuardedPrefix prefix = {0x11223344U, {0xABCDEF01U, 0x12345678U, 0xA5A50000U}, 0x55667788U};
    PrefixFlagState *prefix_result = prefix.state.initialize();
    assert(prefix_result == &prefix.state);
    assert(prefix.state.word_00_ == 0x24U);
    assert(prefix.state.word_04_ == 0U);
    assert(prefix.state.word_08_ == 0xA5A50001U);
    assert(prefix.before == 0x11223344U && prefix.after == 0x55667788U);

    struct GuardedNine { uint32_t before; NineWordSentinelState state; uint32_t after; };
    GuardedNine nine = {0x13579BDFU,
        {1U,2U,3U,4U,5U,6U,7U,8U,9U}, 0xFDB97531U};
    NineWordSentinelState *nine_result = nine.state.reset();
    assert(nine_result == &nine.state);
    assert(nine.state.word_00_ == 0U && nine.state.word_04_ == 0U);
    assert(nine.state.word_08_ == 0U && nine.state.word_0C_ == 0U);
    assert(nine.state.word_10_ == 0U && nine.state.word_14_ == 0U);
    assert(nine.state.word_18_ == 0U && nine.state.word_1C_ == 0U);
    assert(nine.state.word_20_ == 0xFFFFFFFFU);
    assert(nine.before == 0x13579BDFU && nine.after == 0xFDB97531U);

    struct GuardedPair { uint32_t before; PairWordState state; uint32_t after; };
    GuardedPair pair = {0xCAFEBABEU, {0x87654321U,0x11112222U,0x33334444U}, 0xDEADC0DEU};
    PairWordState *pair_result = pair.state.reset_tail();
    assert(pair_result == &pair.state);
    assert(pair.state.word_00_ == 0x87654321U);
    assert(pair.state.word_04_ == 0U && pair.state.word_08_ == 0U);
    assert(pair.before == 0xCAFEBABEU && pair.after == 0xDEADC0DEU);
    return 0;
}
'''


def _mask(source):
    return COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if c == "\n" else " " for c in match[0]), source)


def method_body(source, signature):
    masked = _mask(source)
    start = masked.find(signature)
    assert start >= 0, "missing actual method: " + signature
    opening = masked.find("{", start)
    assert opening >= 0
    depth = 0
    for i in range(opening, len(masked)):
        if masked[i] == "{":
            depth += 1
        elif masked[i] == "}":
            depth -= 1
            if depth == 0:
                return source[start:i + 1]
    raise AssertionError("unbalanced method: " + signature)


def assert_prefix_order(body):
    ops = [
        "uint32_t saved_word_08 = word_08_;",
        "word_04_ = 0;",
        "saved_word_08 |= 1U;",
        "word_00_ = 0x24U;",
        "word_08_ = saved_word_08;",
    ]
    positions = [body.index(op) for op in ops]
    assert positions == sorted(positions)


def assert_nine_store_order(body):
    ops = ["word_04_ = 0;", "word_08_ = 0;", "word_0C_ = 0;",
           "word_10_ = 0;", "word_00_ = 0;", "word_14_ = 0;",
           "word_18_ = 0;", "word_1C_ = 0;", "word_20_ = 0xFFFFFFFFU;"]
    positions = [body.index(op) for op in ops]
    assert positions == sorted(positions)


def compile_run(compiler, source_text, expected=True):
    with tempfile.TemporaryDirectory(prefix="word-state-source-") as temp_name:
        temp = Path(temp_name)
        header = temp / "word_state.h"
        stdafx = temp / "stdafx.h"
        compat = temp / "vc6_compat.h"
        cpp = temp / "word_state.cpp"
        harness = temp / "control.cpp"
        exe = temp / "control"
        header.write_text(HEADER.read_text(encoding="utf-8"), encoding="utf-8")
        # Host shim only: the component includes project stdafx first for VC6,
        # but the native Windows precompiled header is irrelevant to these
        # extracted method bodies.
        stdafx.write_text("/* host-only stdafx shim */\n", encoding="utf-8")
        compat.write_text((ROOT / "src/vc6_compat.h").read_text(encoding="utf-8"),
                          encoding="utf-8")
        cpp.write_text(source_text, encoding="utf-8")
        harness.write_text(HARNESS, encoding="utf-8")
        built = subprocess.run(
            [compiler, "-std=c++98", "-O0", "-Wall", "-Wextra",
             "-I", str(temp), str(cpp), str(harness), "-o", str(exe)],
            capture_output=True, text=True)
        assert built.returncode == 0, built.stdout + built.stderr
        ran = subprocess.run([str(exe)], capture_output=True, text=True)
        assert (ran.returncode == 0) == expected, ran.stdout + ran.stderr


def test_actual_component_methods_preserve_behavior_offsets_and_order():
    source = SOURCE.read_text(encoding="utf-8")
    header = HEADER.read_text(encoding="utf-8")
    assert "RECOVERY_ROLE: SCAFFOLD" in source
    roles = [line for line in source.splitlines()
             if line.startswith("// RECOVERY_ROLE: SCAFFOLD")]
    bindings = [line.partition("// RECOVERY_BINDING: v1 ")[2]
                for line in source.splitlines()
                if line.startswith("// RECOVERY_BINDING: v1 ")]
    assert len(roles) == 3 and len(bindings) == 3
    for binding in bindings:
        parsed = json.loads(binding)
        assert parsed["reason"]
        assert parsed["evidence"]
    for marker in (
        "// ORIGINAL: 0x004C8070 sub_4c8070 0x004C8070-0x004C8089 BYTE_EXACT",
        "// ORIGINAL: 0x004C8090 sub_4c8090 0x004C8090-0x004C80B3 BYTE_EXACT",
        "// ORIGINAL: 0x004C8100 sub_4c8100 0x004C8100-0x004C810B BYTE_EXACT",
    ):
        assert marker in source
    assert "PrefixFlagState::initialize()" in source
    assert "NineWordSentinelState::reset()" in source
    assert "PairWordState::reset_tail()" in source
    assert "leaf_004c8070_redirect" not in SOURCE.read_text(encoding="utf-8")
    assert "field_accessor_004c8090_redirect" not in source
    assert "field_accessor_004c8100_redirect" not in source
    assert "field_accessor_004c8090_redirect" not in (ROOT / "src/field_accessors.h").read_text()
    assert "field_accessor_004c8100_redirect" not in (ROOT / "src/field_accessors.h").read_text()
    assert "src/word_state.cpp" in (ROOT / "CMakeLists.txt").read_text()
    for name in ("PrefixFlagState", "NineWordSentinelState", "PairWordState"):
        assert "struct " + name in header

    prefix = method_body(source, "PrefixFlagState *PrefixFlagState::initialize()")
    assert_prefix_order(prefix)
    nine = method_body(source, "NineWordSentinelState *NineWordSentinelState::reset()")
    assert_nine_store_order(nine)
    pair = method_body(source, "PairWordState *PairWordState::reset_tail()")
    assert pair.index("word_04_ = 0;") < pair.index("word_08_ = 0;") < pair.index("return this;")

    compilers = (shutil.which("g++"), shutil.which("clang++"))
    assert all(compilers), "GCC and Clang are required"
    actual = SOURCE.read_text(encoding="utf-8")
    for compiler in compilers:
        compile_run(compiler, actual)


def test_source_order_mutants_are_rejected():
    source = SOURCE.read_text(encoding="utf-8")
    prefix = method_body(source, "PrefixFlagState *PrefixFlagState::initialize()")
    prefix_mutant = prefix.replace(
        "word_04_ = 0;\n    saved_word_08 |= 1U;",
        "saved_word_08 |= 1U;\n    word_04_ = 0;", 1)
    assert prefix_mutant != prefix
    with pytest.raises(AssertionError):
        assert_prefix_order(prefix_mutant)

    nine = method_body(source, "NineWordSentinelState *NineWordSentinelState::reset()")
    nine_mutant = nine.replace("word_04_ = 0;\n    word_08_ = 0;",
                              "word_08_ = 0;\n    word_04_ = 0;", 1)
    assert nine_mutant != nine
    with pytest.raises(AssertionError):
        assert_nine_store_order(nine_mutant)


def test_behavior_mutants_fail_gcc_and_clang():
    source = SOURCE.read_text(encoding="utf-8")
    mutations = (
        ("saved_word_08 |= 1U;", "saved_word_08 &= ~1U;"),
        ("word_20_ = 0xFFFFFFFFU;", "word_20_ = 0U;"),
        ("word_08_ = 0;\n    return this;", "word_08_ = 1;\n    return this;"),
    )
    compilers = (shutil.which("g++"), shutil.which("clang++"))
    assert all(compilers), "GCC and Clang are required"
    for before, after in mutations:
        assert source.count(before) == 1
        mutant = source.replace(before, after, 1)
        for compiler in compilers:
            compile_run(compiler, mutant, expected=False)
