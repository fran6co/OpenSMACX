from __future__ import annotations

import re
import subprocess
from pathlib import Path


ROOT = next(
    parent for parent in Path(__file__).resolve().parents
    if (parent / "src" / "movie_media_decoder.cpp").is_file()
)
SRC = ROOT / "src"
TASK_BASE = "45686c6c02de5018cc73de5856c7064fba9ed9bc"


def _tokens(source: str) -> list[str]:
    without_comments = re.sub(r"/\*.*?\*/|//[^\n]*", "", source, flags=re.DOTALL)
    return re.findall(
        r"0[xX][0-9A-Fa-f]+|[A-Za-z_]\w*|\d+|::|->|==|!=|<=|>=|&&|\|\||[^\s]",
        without_comments,
    )


def _body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for pos in range(opening, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start : pos + 1]
    raise AssertionError(f"unterminated C++ body: {signature}")


def _replace_once(tokens: list[str], old: list[str], new: list[str]) -> list[str]:
    starts = [
        index for index in range(len(tokens) - len(old) + 1)
        if tokens[index : index + len(old)] == old
    ]
    assert len(starts) == 1
    index = starts[0]
    return tokens[:index] + new + tokens[index + len(old) :]


def _git_base(path: str) -> str:
    result = subprocess.run(
        ["git", "show", f"{TASK_BASE}:{path}"],
        cwd=ROOT,
        capture_output=True,
        text=True,
        check=True,
    )
    return result.stdout


def test_pointer_member_is_the_only_authorized_source_token_change() -> None:
    base_header = _git_base("src/movie_decoder_owner.h")
    current_header = (SRC / "movie_decoder_owner.h").read_text()
    expected_header = _tokens(base_header)
    expected_header = _replace_once(
        expected_header,
        ["DWORD", "current_output_pointer_bits", ";"],
        ["int", "*", "current_output_pointer", ";"],
    )
    idct_window_check = _tokens(
        "typedef char media_decoder_idct_access_window_size_check["
        "sizeof(MediaDecoderIdctStorage) == 0x14c ? 1 : -1];"
    )
    additions = _tokens(
        "typedef char media_decoder_idct_output_pointer_offset_check["
        "offsetof(MediaDecoderIdctStorage, current_output_pointer) == 4 ? 1 : -1];"
        "typedef char media_decoder_idct_butterfly_scratch_offset_check["
        "offsetof(MediaDecoderIdctStorage, butterfly_fp_scratch_words) == 8 ? 1 : -1];"
    )
    positions = [
        index for index in range(len(expected_header) - len(idct_window_check) + 1)
        if expected_header[index : index + len(idct_window_check)] == idct_window_check
    ]
    assert len(positions) == 1
    insert_at = positions[0] + len(idct_window_check)
    expected_header[insert_at:insert_at] = additions
    assert _tokens(current_header) == expected_header

    base_cpp = _git_base("src/movie_media_decoder.cpp")
    current_cpp = (SRC / "movie_media_decoder.cpp").read_text()
    expected_cpp = _tokens(base_cpp)
    old_getter = _body(base_cpp, "static int *movie_media_idct_output_pointer()")
    old_setter = _body(
        base_cpp, "static void movie_media_idct_set_output_pointer(int *value)"
    )
    expected_cpp = _replace_once(
        expected_cpp,
        _tokens(old_getter),
        _tokens(
            "static int *movie_media_idct_output_pointer() {"
            "return movie_media_idct_storage.current_output_pointer;"
            "}"
        ),
    )
    expected_cpp = _replace_once(
        expected_cpp,
        _tokens(old_setter),
        _tokens(
            "static void movie_media_idct_set_output_pointer(int *value) {"
            "movie_media_idct_storage.current_output_pointer = value;"
            "}"
        ),
    )
    assert _tokens(current_cpp) == expected_cpp

    # These are the complete source-owned mapping/history annotation lines;
    # the member/helper repair must not rewrite identities, spans, claims,
    # bindings, roles, or their TRIED/LEVER restoration obligations.
    history_prefixes = (
        "// ORIGINAL:", "// RECOVERY_BINDING:", "// RECOVERY_ROLE:",
        "// RECOVERY_HISTORY:", "// symbol", "// size", "// prototype",
        "// callers", "// call targets", "// kind", "// flags", "// calls",
        "// indirect", "// LEVER", "// TRIED", "// UNRECOVERABLE:",
        "// DEFERRED:",
    )

    def history(text: str) -> list[str]:
        return [
            line for line in text.splitlines()
            if line.lstrip().startswith(history_prefixes)
        ]

    assert history(current_cpp) == history(base_cpp)
    assert "BYTE_EXACT" in "\n".join(history(current_cpp))
    def comments(text: str) -> list[str]:
        return re.findall(r"/\*.*?\*/|//[^\n]*", text, re.DOTALL)

    assert comments(current_cpp) == comments(base_cpp)

