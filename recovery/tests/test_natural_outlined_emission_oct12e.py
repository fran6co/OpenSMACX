"""Baseline-backed source controls for ordinary out-of-line definitions."""

from __future__ import annotations

import re
import subprocess
from pathlib import Path

from match_recovery.source import read_file, read_text

ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())
BASELINE = "2560e3d174cf280271e39e86040e1a2ee14531ee"
FILES = (
    "src/buffer.h", "src/buffer.cpp", "src/jackal/palette.h",
    "src/jackal/palette.cpp", "src/dialog.h", "src/dialog.cpp",
)
ATTRIBUTES = {
    "src/buffer.h": ("MEASURED", "virtual", "~", "Buffer"),
    "src/buffer.cpp": ("MEASURED", "Buffer", "::", "~", "Buffer"),
    "src/jackal/palette.h": ("MEASURED", "~", "Palette"),
    "src/jackal/palette.cpp": ("MEASURED", "Palette", "::", "~", "Palette"),
    "src/dialog.h": ("extern", '"C"', "MEASURED", "void", "__cdecl",
                     "dialog_set_def_dialog_text_color"),
    "src/dialog.cpp": ("MEASURED", "void", "__cdecl",
                       "dialog_set_def_dialog_text_color"),
}
IDENTITIES = (
    ("src/buffer.cpp", 0x005D7210),  # Buffer constructor
    ("src/buffer.cpp", 0x005D7410),  # Buffer destructor, including cold span
    ("src/buffer.cpp", 0x005E3610),  # Buffer scalar deleting entry
    ("src/jackal/palette.cpp", 0x005FE2A0),
    ("src/jackal/palette.cpp", 0x005FE2E0),
    ("src/dialog.cpp", 0x00609D50),
    ("src/popup.cpp", 0x00588460),  # recovered direct caller
)
PRESERVED_FIELDS = (
    "address", "mode", "state", "name", "image_spans", "symbol", "kind",
    "exclusion", "byte_exact", "semantic", "recipe", "levers", "ruled_out",
    "unrecoverable", "deferred", "source_role", "source_role_reason",
)


def _baseline_text(rel: str) -> str:
    result = subprocess.run(
        ["git", "show", f"{BASELINE}:{rel}"], cwd=ROOT, text=True,
        capture_output=True, timeout=30,
    )
    assert result.returncode == 0, (
        f"cannot read immutable task baseline {BASELINE}:{rel}: {result.stderr}"
    )
    return result.stdout


def _code_tokens(text: str) -> list[str]:
    tokens = re.findall(
        r"//[^\n]*|/\*[\s\S]*?\*/|\"(?:\\.|[^\"\\])*\"|"
        r"'(?:\\.|[^'\\])*'|[A-Za-z_]\w*|0[xX][0-9A-Fa-f]+|\d+|"
        r"::|->|&&|\|\||==|!=|<=|>=|\+\+|--|[^\s]",
        text,
    )
    return [token for token in tokens if not token.startswith(("//", "/*"))]


def _without_scoped_attribute(rel: str, text: str, *, baseline: bool) -> list[str]:
    tokens = _code_tokens(text)
    prefix = ATTRIBUTES[rel]
    matches = [index for index in range(len(tokens) - len(prefix) + 1)
               if tuple(tokens[index:index + len(prefix)]) == prefix]
    if baseline:
        assert len(matches) == 1, (rel, matches)
        del tokens[matches[0] + prefix.index("MEASURED")]
    else:
        assert not matches, f"scoped MEASURED attribute remains in {rel}"
    return tokens


def _continuity(row):
    fields = tuple(getattr(row, name) for name in PRESERVED_FIELDS)
    origin = tuple(
        (item.kind, item.key, item.prose, tuple(text for _, text in item.lines))
        for item in row.origin or ()
    )
    return fields + (origin,)


def _body(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        if text[end] == "{":
            depth += 1
        elif text[end] == "}":
            depth -= 1
            if depth == 0:
                return text[start:end + 1]
    raise AssertionError(f"unclosed source body: {signature}")


def test_six_files_differ_only_by_the_six_scoped_measured_attributes():
    for rel in FILES:
        historical = _baseline_text(rel)
        current = (ROOT / rel).read_text(encoding="utf-8")
        assert _without_scoped_attribute(rel, current, baseline=False) == \
            _without_scoped_attribute(rel, historical, baseline=True), rel

    buffer_cpp = (ROOT / "src/buffer.cpp").read_text(encoding="utf-8")
    palette_cpp = (ROOT / "src/jackal/palette.cpp").read_text(encoding="utf-8")
    assert _body(buffer_cpp, "Buffer::~Buffer()") == "Buffer::~Buffer() { close(); }"
    assert _body(palette_cpp, "Palette::~Palette()") == "Palette::~Palette() { close(); }"


def test_original_phase_annotations_and_recovered_dialog_caller_are_unchanged():
    baseline_rows = {}
    current_rows = {}
    for rel, _ in IDENTITIES:
        if rel not in baseline_rows:
            baseline_rows[rel] = {row.address: row for row in read_text(
                _baseline_text(rel), f"{BASELINE}:{rel}")}
            current_rows[rel] = {row.address: row for row in read_file(ROOT / rel)}

    for rel, address in IDENTITIES:
        assert address in baseline_rows[rel], (BASELINE, rel, address)
        assert address in current_rows[rel], (rel, address)
        assert _continuity(current_rows[rel][address]) == \
            _continuity(baseline_rows[rel][address]), f"{rel} {address:#x}"

    popup = (ROOT / "src/popup.cpp").read_text(encoding="utf-8")
    popups_normal = _body(popup, "void __cdecl popups_normal()")
    assert "dialog_set_def_dialog_text_color(0xE0, -1, 1, 1, 0);" in popups_normal

    dialog = (ROOT / "src/dialog.cpp").read_text(encoding="utf-8")
    assert "MEASURED: the image's one caller is not recovered" not in dialog
