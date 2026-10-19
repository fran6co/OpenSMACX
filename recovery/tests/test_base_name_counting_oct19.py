"""Behavioral and pinned-source controls for the original base-name scan."""
from __future__ import annotations

import hashlib
import os
import re
import shutil
import subprocess
from pathlib import Path

import pytest

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import read_file, read_text
from match_recovery.source_inventory import image_pin


ROOT = Path(__file__).resolve().parents[2]
PINNED_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
BASE_REVISION = "8077e14f9d5169c1b43e7314ba86193a78e57f39"
TARGET_ADDRESS = 0x004F7FE0
TARGET_SPAN = Span(0x004F7FE0, 0x004F8087)
IMAGE_DEFAULT = ROOT / ".opensmacx/game/terranx_original.exe"


def source_mapping():
    rows = [row for row in read_file(ROOT / "src/base.cpp")
            if row.address == TARGET_ADDRESS]
    assert len(rows) == 1
    row, = rows
    assert row.image_spans == ((TARGET_SPAN.start, TARGET_SPAN.end),)
    assert row.extract_error == ""
    return row


def current_region() -> str:
    row = source_mapping()
    assert "BODY GOES HERE." not in row.region
    return row.region


def baseline_body() -> str:
    completed = subprocess.run(
        ["git", "show", f"{BASE_REVISION}:src/base.cpp"],
        cwd=ROOT,
        capture_output=True,
        text=True,
        check=False,
        timeout=30,
    )
    assert completed.returncode == 0, completed.stderr
    rows = [row for row in read_text(completed.stdout, ROOT / "src/base.cpp")
            if row.address == TARGET_ADDRESS]
    assert len(rows) == 1
    row, = rows
    assert row.image_spans == ((TARGET_SPAN.start, TARGET_SPAN.end),)
    assert row.extract_error == ""
    return row.region


def actual_base_name_declaration() -> str:
    header = (ROOT / "src/base.h").read_text(encoding="utf-8")
    declaration = re.search(r"(?m)^\s*char\s+name_string\[25\];\s*$", header)
    assert declaration is not None
    return declaration.group(0).strip()


HARNESS_TEMPLATE = r"""
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define __cdecl

struct Base { NAME_DECLARATION };
Base Bases[8];
struct GameDataStateStorageType {
    struct Fields { int base_current_count_; } fields_;
} GameDataStateStorage;

char *_itoa(int value, char *buffer, int radix) {
    if (radix != 10) abort();
    snprintf(buffer, 0x50, "%d", value);
    return buffer;
}

// Match the product helper's leading/trailing ASCII-space trim for the
// baseline negative control. The corrected body must not call it here.
void purge_spaces(char *text) {
    size_t length = strlen(text);
    size_t first = 0;
    while (first < length && text[first] == ' ') ++first;
    while (length > first && text[length - 1] == ' ') --length;
    memmove(text, text + first, length - first);
    text[length - first] = 0;
}

void __cdecl make_base_unique(int base_id);

static void set_name(int index, const char *name) {
    strcpy(Bases[index].name_string, name);
}

int main(int argc, char **argv) {
    if (argc != 2) return 90;
    const char *scenario = argv[1];
    int target = 0;
    if (!strcmp(scenario, "trim_without_collision")) {
        GameDataStateStorage.fields_.base_current_count_ = 1;
        set_name(0, "  Alpha  ");
    } else if (!strcmp(scenario, "valid_24_character_name")) {
        GameDataStateStorage.fields_.base_current_count_ = 1;
        set_name(0, "abcdefghijklmnopqrstuvwx");
    } else if (!strcmp(scenario, "one_collision")) {
        GameDataStateStorage.fields_.base_current_count_ = 2;
        set_name(0, "Name"); set_name(1, "Name");
    } else if (!strcmp(scenario, "two_collisions")) {
        GameDataStateStorage.fields_.base_current_count_ = 3;
        set_name(0, "Name"); set_name(1, "Name"); set_name(2, "Name");
    } else if (!strcmp(scenario, "three_collisions")) {
        GameDataStateStorage.fields_.base_current_count_ = 4;
        set_name(0, "Name"); set_name(1, "Name");
        set_name(2, "Name"); set_name(3, "Name");
    } else if (!strcmp(scenario, "self_only")) {
        GameDataStateStorage.fields_.base_current_count_ = 1;
        set_name(0, "Name");
    } else if (!strcmp(scenario, "self_excluded")) {
        GameDataStateStorage.fields_.base_current_count_ = 2;
        target = 1;
        set_name(0, "Other"); set_name(1, "Name");
    } else if (!strcmp(scenario, "zero_count")) {
        GameDataStateStorage.fields_.base_current_count_ = 0;
        set_name(0, "Name");
    } else if (!strcmp(scenario, "negative_count")) {
        GameDataStateStorage.fields_.base_current_count_ = -2;
        set_name(0, "Name");
    } else if (!strcmp(scenario, "already_suffixed")) {
        GameDataStateStorage.fields_.base_current_count_ = 3;
        set_name(0, "Name-2"); set_name(1, "Name-2"); set_name(2, "Name-2");
    } else if (!strcmp(scenario, "truncate_after_scan")) {
        GameDataStateStorage.fields_.base_current_count_ = 3;
        const char *long_name = "abcdefghijklmnopqrstuvwx";
        set_name(0, long_name);
        set_name(1, long_name);
        // The original counts against the full unmodified target. Mutating
        // early creates this truncated-and-suffixed value, which is not equal
        // to the original long target and must not count as another collision.
        set_name(2, "abcdefghijklmnopqrstu-2");
    } else {
        return 91;
    }

    make_base_unique(target);
    puts(Bases[target].name_string);
    return 0;
}
"""


EXPECTED = {
    "trim_without_collision": "  Alpha  ",
    "valid_24_character_name": "abcdefghijklmnopqrstuvwx",
    "one_collision": "Name-2",
    "two_collisions": "Name-3",
    "three_collisions": "Name-4",
    "self_only": "Name",
    "self_excluded": "Name",
    "zero_count": "Name",
    "negative_count": "Name",
    "already_suffixed": "Name-2-3",
    "truncate_after_scan": "abcdefghijklmnopqrstu-2",
}


def compile_body(body: str, declaration: str, directory: Path) -> Path:
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.fail("host g++ is required for this bounded source test")
    generated = HARNESS_TEMPLATE.replace("NAME_DECLARATION", declaration)
    generated += "\n" + body + "\n"
    source_path = directory / "base_name_harness.cpp"
    executable = directory / "base_name_harness"
    source_path.write_text(generated, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++17", "-O0", "-Wall", "-Wextra", "-Werror",
         "-Wno-sign-compare",
         str(source_path), "-o", str(executable)],
        capture_output=True,
        text=True,
        timeout=30,
        check=False,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return executable


@pytest.fixture(scope="module")
def binaries(tmp_path_factory) -> dict[str, Path]:
    declaration = actual_base_name_declaration()
    product_dir = tmp_path_factory.mktemp("base-name-current")
    baseline_dir = tmp_path_factory.mktemp("base-name-baseline")
    return {
        "current": compile_body(current_region(), declaration, product_dir),
        "baseline": compile_body(baseline_body(), declaration, baseline_dir),
    }


def run_case(binary: Path, case: str) -> str:
    completed = subprocess.run(
        [str(binary), case], capture_output=True, text=True, timeout=10, check=False
    )
    assert completed.returncode == 0, completed.stdout + completed.stderr
    return completed.stdout.rstrip("\n")


def test_actual_source_mapping_and_original_claim_are_retained():
    row = source_mapping()
    assert "// ORIGINAL: 0x004F7FE0" in row.region
    assert "// LEVER: dropped std::string entirely" in row.region
    assert "function has been rewritten to work as intended" not in row.region
    assert "BUG IN THE ORIGINAL: only the FIRST collision truncates" not in row.region


@pytest.mark.parametrize("case, expected", EXPECTED.items())
def test_extracted_current_body_matches_semantic_cases(
    binaries: dict[str, Path], case: str, expected: str
) -> None:
    assert run_case(binaries["current"], case) == expected


@pytest.mark.parametrize(
    "case",
    ["trim_without_collision", "two_collisions", "three_collisions",
     "already_suffixed", "truncate_after_scan"],
)
def test_original_committed_wrong_body_is_a_negative_control(
    binaries: dict[str, Path], case: str
) -> None:
    assert run_case(binaries["baseline"], case) != EXPECTED[case]


def _load_pinned(path: Path) -> PeImage:
    if not path.is_file():
        raise ValueError(f"pinned image is missing: {path}")
    raw = path.read_bytes()
    actual = hashlib.sha256(raw).hexdigest()
    if actual != PINNED_SHA256:
        raise ValueError(f"pinned image digest mismatch: {actual}")
    return PeImage(raw, PINNED_SHA256)


def _configured_image_path() -> Path:
    configured = os.environ.get("OPENSMACX_IMAGE")
    return Path(configured).expanduser().resolve() if configured else IMAGE_DEFAULT


def _decode_complete(image: PeImage, span: Span):
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32

    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    instructions = tuple(decoder.disasm(image.read(span), span.start))
    cursor = span.start
    for instruction in instructions:
        assert instruction.address == cursor
        cursor += instruction.size
    assert cursor == span.end
    return {instruction.address: instruction for instruction in instructions}


def _pair(instructions, address: int) -> tuple[str, str]:
    instruction = instructions[address]
    return instruction.mnemonic, instruction.op_str


def test_pinned_original_proves_count_scan_precedes_single_suffix():
    assert image_pin(ROOT) == PINNED_SHA256
    image_path = _configured_image_path()
    try:
        image = _load_pinned(image_path)
    except ValueError as error:
        pytest.fail(str(error))
    instructions = _decode_complete(image, TARGET_SPAN)

    assert _pair(instructions, 0x004F7FE6) == ("mov", "eax, dword ptr [0x9a64cc]")
    assert _pair(instructions, 0x004F8017) == ("push", "eax")
    assert _pair(instructions, 0x004F8018) == ("push", "ebx")
    assert _pair(instructions, 0x004F8019) == ("call", "0x645660")
    assert _pair(instructions, 0x004F8023) == ("jne", "0x4f8028")
    assert _pair(instructions, 0x004F8025) == ("inc", "dword ptr [ebp - 4]")
    assert _pair(instructions, 0x004F8036) == ("jl", "0x4f8003")
    assert _pair(instructions, 0x004F803E) == ("je", "0x4f8081")

    # Only after the full scan: [0x97D053 + target*0x134 + 0x15] is
    # name_string[21], followed by hyphen append, _itoa(count + 1), append.
    assert _pair(instructions, 0x004F8057) == (
        "mov", "byte ptr [eax + 0x97d068], 0"
    )
    assert 0x97D068 - 0x97D053 == 21
    assert _pair(instructions, 0x004F8043) == ("push", "0x689f08")
    assert _pair(instructions, 0x004F805E) == ("push", "esi")
    assert _pair(instructions, 0x004F805F) == ("call", "0x645470")
    assert _pair(instructions, 0x004F8064) == ("mov", "ecx, dword ptr [ebp - 4]")
    assert _pair(instructions, 0x004F8067) == ("lea", "eax, [ebp - 0x54]")
    assert _pair(instructions, 0x004F806A) == ("push", "0xa")
    assert _pair(instructions, 0x004F806C) == ("inc", "ecx")
    assert _pair(instructions, 0x004F806D) == ("push", "eax")
    assert _pair(instructions, 0x004F806E) == ("push", "ecx")
    assert _pair(instructions, 0x004F806F) == ("call", "0x64fc88")
    assert _pair(instructions, 0x004F8074) == ("lea", "edx, [ebp - 0x54]")
    assert _pair(instructions, 0x004F8077) == ("push", "edx")
    assert _pair(instructions, 0x004F8078) == ("push", "esi")
    assert _pair(instructions, 0x004F8079) == ("call", "0x645470")

    direct_calls = [
        (address, _pair(instructions, address)[1])
        for address in sorted(instructions)
        if instructions[address].mnemonic == "call"
    ]
    assert direct_calls == [
        (0x004F8019, "0x645660"),
        (0x004F805F, "0x645470"),
        (0x004F806F, "0x64fc88"),
        (0x004F8079, "0x645470"),
    ]


def test_pinned_reader_refuses_missing_or_corrupt_images(tmp_path: Path):
    with pytest.raises(ValueError, match="missing"):
        _load_pinned(tmp_path / "missing.exe")

    configured = _configured_image_path()
    try:
        original = _load_pinned(configured)
    except ValueError as error:
        pytest.fail(str(error))
    damaged_path = tmp_path / "damaged.exe"
    damaged = bytearray(configured.read_bytes())
    damaged[0x100] ^= 0x01
    damaged_path.write_bytes(damaged)
    with pytest.raises(ValueError, match="digest mismatch"):
        _load_pinned(damaged_path)
