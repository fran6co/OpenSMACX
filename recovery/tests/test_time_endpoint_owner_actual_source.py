"""Host controls for the actual endpoint methods after coordinator integration."""
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path
import re
import hashlib

import pytest

from match_recovery.source import SourceRole, read_file
from match_recovery.binding_source import parse as parse_bindings
from match_recovery.source import Mode


ADDRESSES = (0x004C80C0, 0x004C80E0, 0x004C8350)


def find_checkout_root(start: Path) -> Path:
    for candidate in (start, *start.parents):
        if (candidate / "src/IMAGE_PIN").is_file():
            return candidate
    raise RuntimeError(f"could not locate checkout root from {start}")


ROOT = find_checkout_root(Path(__file__).resolve())
HISTORY_SHA256 = {
    0x004C80C0: "e0d6eb0a4d32e64334cc1f4cded43e7d701b510f32d7a1a14fb4d75f7ad5a11f",
    0x004C80E0: "8c15061494d8d9b690e02e41f5aa7054bafa4a713979ab8f6841a65deb438b95",
}
FIELD_SOURCE_SHA256 = "e7103f386f51cc4338c932611b975da2e3655a90005600cdac86173b07d8c0ee"


def function(text: str, signature: str) -> str:
    match = re.search(re.escape(signature), text)
    assert match, signature
    brace = text.find("{", match.end())
    depth = 0
    for i in range(brace, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return text[match.start():i + 1]
    raise AssertionError("unterminated source function")


def class_declaration(text: str, name: str) -> str:
    signature = f"class {name} {{"
    assert text.count(signature) == 1
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[start:text.index(";", index) + 1]
    raise AssertionError(f"unterminated class declaration: {name}")


def archived_history(text: str, address: int) -> str:
    token = f"RECOVERY_HISTORY: v1 address=0x{address:08X} "
    assert text.count(token) == 1
    line_at = text.index(token)
    start = text.rfind("/*", 0, line_at)
    close = text.find("\n */", line_at)
    assert start >= 0 and close >= 0
    archive = text[start:close + 4]
    header = text[line_at:text.find("\n", line_at)]
    assert f"source_sha256={FIELD_SOURCE_SHA256}" in header
    expected = HISTORY_SHA256[address]
    assert f"envelope_sha256={expected}" in header
    payload = []
    for line in archive.splitlines()[2:-1]:
        if line == " * [ARCHIVE]":
            payload.append("")
        else:
            assert line.startswith(" * [ARCHIVE] "), line
            escaped = line[len(" * [ARCHIVE] "):]
            assert "*/" not in escaped
            payload.append(escaped.replace("* /", "*/"))
    normalized = "\n".join(line.rstrip() for line in payload)
    assert hashlib.sha256(normalized.encode("utf-8", "surrogateescape")).hexdigest() == expected
    assert f"// original marker: 0x{address:08X} " in normalized
    assert f"// ORIGINAL: 0x{address:08X} " not in normalized
    return normalized


def actual_methods() -> tuple[str, str, str]:
    time_path = ROOT / "src/time.cpp"
    field_path = ROOT / "src/field_accessors.cpp"
    time = time_path.read_text()
    field = field_path.read_text()
    records = {r.address: r for r in read_file(time_path) if r.address in ADDRESSES}
    assert set(records) == set(ADDRESSES)
    assert not any(r.address in ADDRESSES[:2] for r in read_file(field_path))
    for address, span in ((ADDRESSES[0], (0x004C80C0, 0x004C80D7)),
                          (ADDRESSES[1], (0x004C80E0, 0x004C80F7)),
                          (ADDRESSES[2], (0x004C8350, 0x004C8396))):
        row = records[address]
        assert row.image_spans == (span,)
        assert row.source_role is SourceRole.SCAFFOLD
        assert row.source_role_reason and "lifetime remain unproved" in row.source_role_reason
        assert row.name == {ADDRESSES[0]: "sub_4c80c0",
                            ADDRESSES[1]: "sub_4c80e0",
                            ADDRESSES[2]: "sub_4c8350"}[address]
        assert row.mode is Mode.FILE
        assert row.state.value == "implemented"
    assert records[ADDRESSES[0]].byte_exact
    assert records[ADDRESSES[1]].byte_exact
    assert not records[ADDRESSES[2]].byte_exact
    assert all(not records[address].semantic for address in ADDRESSES)
    bindings = parse_bindings(time, read_file(time_path))
    for address, row in records.items():
        assert row.line in bindings
        rationale = bindings[row.line]
        assert rationale["evidence"]
        assert "lifetime" in rationale["reason"]
    assert "0x004C8350" in time and "reads byte[this+4]&1" in time
    history = {address: archived_history(time, address) for address in ADDRESSES[:2]}
    for address, envelope in history.items():
        assert "Emitted by tools/generate_field_accessors.py from" in envelope
        for retained_fact in (
                    "size      23 bytes", "prototype", "callers   0   call targets   0",
                "kind      game", "flags     frame;hidden;sp_ready;purged_ok",
                "calls     (none)", "Return Value: n/a", "Status: Complete"):
            assert retained_fact in envelope
    assert "jae 0x4c80d0" in time and "jbe 0x4c80f0" in time
    assert "mov dword ptr [ecx + 0xc], eax / mov dword ptr [ecx + 8], eax" in time
    assert "mov dword ptr [ecx + 8], eax / mov dword ptr [ecx + 0xc], eax" in time
    declaration = class_declaration(time, "TimeRangeReceiverView")
    assert "int lower_bound_8_;" in declaration
    assert "int upper_bound_C_;" in declaration
    lower = function(time, "void TimeRangeReceiverView::set_lower_endpoint(int value)")
    upper = function(time, "void TimeRangeReceiverView::set_upper_endpoint(int value)")
    return declaration, lower, upper


def harness(declaration: str, lower: str, upper: str) -> str:
    return r'''#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
typedef uint32_t DWORD;
#define private public
''' + declaration + r'''
#undef private
struct Guarded {
    uint32_t before;
    TimeRangeReceiverView value;
    uint32_t after;
};
typedef char receiver_size_must_be_16[(sizeof(TimeRangeReceiverView) == 16) ? 1 : -1];
typedef char receiver_flag_offset_must_be_4[(offsetof(TimeRangeReceiverView, enabled_flags_4_) == 4) ? 1 : -1];
typedef char receiver_lower_offset_must_be_8[(offsetof(TimeRangeReceiverView, lower_bound_8_) == 8) ? 1 : -1];
typedef char receiver_upper_offset_must_be_12[(offsetof(TimeRangeReceiverView, upper_bound_C_) == 12) ? 1 : -1];
''' + lower + "\n" + upper + r'''
int main() {
    Guarded storage;
    Guarded *g = &storage;
    TimeRangeReceiverView *value = &g->value;
    g->before = 0x13579BDFu;
    memset(value->unobserved_prefix_0_, 0xA5, sizeof(value->unobserved_prefix_0_));
    value->enabled_flags_4_ = 0xD3;
    memset(value->unobserved_bytes_5_7_, 0x5A, sizeof(value->unobserved_bytes_5_7_));
    value->lower_bound_8_ = 2;
    value->upper_bound_C_ = 9;
    g->after = 0x2468ACE0u;
    value->set_lower_endpoint(12);
    assert(value->lower_bound_8_ == 12 && value->upper_bound_C_ == 12);
    value->set_upper_endpoint(3);
    assert(value->lower_bound_8_ == 3 && value->upper_bound_C_ == 3);
    value->lower_bound_8_ = (int)0x80000000u;
    value->upper_bound_C_ = 0x7FFFFFFF;
    value->set_lower_endpoint(0x7FFFFFFF);
    assert((uint32_t)value->lower_bound_8_ == 0x7FFFFFFFu);
    assert((uint32_t)value->upper_bound_C_ == 0x7FFFFFFFu);
    value->lower_bound_8_ = 1;
    value->upper_bound_C_ = (int)0x80000000u;
    value->set_lower_endpoint(0x7FFFFFFF);
    assert((uint32_t)value->upper_bound_C_ == 0x80000000u);
    value->lower_bound_8_ = 0x7FFFFFFF;
    value->upper_bound_C_ = (int)0x80000000u;
    value->set_upper_endpoint((int)0x80000000u);
    assert((uint32_t)value->lower_bound_8_ == 0x7FFFFFFFu);
    assert((uint32_t)value->upper_bound_C_ == 0x80000000u);
    value->lower_bound_8_ = (int)0x80000000u;
    value->upper_bound_C_ = 1;
    value->set_upper_endpoint(0x7FFFFFFF);
    assert((uint32_t)value->lower_bound_8_ == 0x7FFFFFFFu);
    assert(g->before == 0x13579BDFu && g->after == 0x2468ACE0u);
    assert(value->enabled_flags_4_ == 0xD3);
    assert(value->unobserved_prefix_0_[0] == 0xA5 && value->unobserved_prefix_0_[3] == 0xA5);
    assert(value->unobserved_bytes_5_7_[0] == 0x5A && value->unobserved_bytes_5_7_[2] == 0x5A);
    return 0;
}
'''


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_endpoint_member_methods_and_mutation_controls(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} is not installed")
    declaration, lower, upper = actual_methods()
    # The pinned instructions require conditional counterpart update before
    # the unconditional requested-endpoint store; final-state-only checks do
    # not distinguish the order, so guard it structurally too.
    assert lower.index("upper_bound_C_ = value") < lower.index("lower_bound_8_ = value")
    assert upper.index("lower_bound_8_ = value") < upper.index("upper_bound_C_ = value")
    mutations = {
        "actual": (lower, upper, True),
        "signed-compare": (lower.replace(
            "static_cast<DWORD>(upper_bound_C_) < static_cast<DWORD>(value)",
            "upper_bound_C_ < value", 1), upper, False),
        "missing-clamp": (re.sub(r"if \([^\n]+\) \{\n        upper_bound_C_ = value;\n    \}", "if (false) {}", lower), upper, False),
    }
    assert mutations["actual"][0] != mutations["signed-compare"][0]
    assert mutations["actual"][0] != mutations["missing-clamp"][0]
    for label, (candidate_lower, candidate_upper, should_pass) in mutations.items():
        cpp = tmp_path / f"{label}.cpp"
        exe = tmp_path / label
        cpp.write_text(harness(declaration, candidate_lower, candidate_upper))
        built = subprocess.run([compiler, "-std=c++98", "-O2", str(cpp), "-o", str(exe)],
                               capture_output=True, text=True, timeout=30)
        assert built.returncode == 0, built.stdout + built.stderr
        ran = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
        if should_pass:
            assert ran.returncode == 0, ran.stdout + ran.stderr
        else:
            assert ran.returncode != 0, "mutated behavior escaped host oracle"
