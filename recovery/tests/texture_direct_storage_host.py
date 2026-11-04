"""Small literal-source helpers shared by Texture direct-storage fixtures.

These helpers splice current production declarations and bodies into host
fixtures; they do not rewrite production aliases back into the old source form.
Host execution checks storage/dataflow contracts only, not target ABI layout.
"""
from __future__ import annotations

import re
from pathlib import Path

from match_recovery.includes import COMMENTS_OR_STRINGS


DIRECT_STORAGE_START = "// Shared Texture-owned storage for the original raster cells."
DIRECT_STORAGE_END = "static int texture_coast_advance_edge"
KEY_OBJECT_MARKER = "// RECOVERY_OBJECT: v1 0x009BB548 ?TextureRasterKey@@3IA"
KEY_HELPER_SIGNATURE = "static int texture_raster_key_signed_bits()"
PRELUDE_MARKER = "// TEXTURE_DIRECT_STORAGE_PRELUDE"
SIGNED_KEY_NOTE = (
    "// Direct raster-global accesses remove private alias cells; preserve the key's\n"
    "// former signed-int view as representation bits here.\n"
)
SIGNED_KEY_HELPER = """static int texture_raster_key_signed_bits() {
    int value;
    memcpy(&value, &TextureRasterKey, sizeof(value));
    return value;
}"""


def _code_replace(source: str, pattern: re.Pattern[str], replacement) -> str:
    """Replace regex matches in C++ code, leaving comments/literals untouched."""
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source)
    matches = list(pattern.finditer(masked))
    for match in reversed(matches):
        value = replacement(match) if callable(replacement) else replacement
        source = source[:match.start()] + value + source[match.end():]
    return source


def _alias_block(source: str) -> tuple[int, int, str, dict[str, str]]:
    start_match = re.search(
        r"(?m)^static const float \*const blit_scale_65536f\s*=", source)
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source)
    assert start_match is not None
    start = start_match.start()
    end_match = re.search(r"(?m)^static int texture_coast_advance_edge\b", masked)
    assert end_match is not None and end_match.start() > start
    end = end_match.start()
    block = source[start:end]
    declaration = re.compile(
        r"(?ms)^static\s+[^;]*?\b((?:blit|g2)_[A-Za-z0-9_]+)\s*=\s*&(.+?);"
    )
    targets = {name: target.strip() for name, target in declaration.findall(block)}
    key_alias = re.search(
        r"(?ms)^static int \*const blit_light_threshold_2\s*=\s*"
        r"reinterpret_cast<int \*>\(&TextureRasterKey\);", block)
    assert key_alias is not None
    targets["blit_light_threshold_2"] = "TextureRasterKey"
    assert len(targets) == 53 and len(set(targets)) == 53, len(targets)
    return start, end, block, targets


def direct_storage_projection(source: str) -> str:
    """Project the historical alias source through the reviewed storage rewrites.

    This remains a source projection, not a body normalizer: it applies the 53
    declarations/use-site substitutions authorized by the Texture storage task,
    the fixed signed-key helper, and the separately authorized four-byte light
    accumulator union view. Callers retain their task-specific body/history
    checks around this projection.
    """
    start, end, block, targets = _alias_block(source)
    projected = source[:start] + source[end:]
    for alias, target in targets.items():
        if alias == "blit_light_threshold_2":
            projected = _code_replace(
                projected, re.compile(r"\*\s*" + re.escape(alias) + r"\b"),
                "texture_raster_key_signed_bits()")
            continue
        projected = _code_replace(
            projected, re.compile(r"\*\s*" + re.escape(alias) + r"\b"), target)
        projected = _code_replace(
            projected, re.compile(re.escape(alias) + r"\s*\[\s*0\s*\]"), target)
        projected = _code_replace(
            projected, re.compile(r"\b" + re.escape(alias) + r"\b"),
            "&(" + target + ")")

    insertion = SIGNED_KEY_NOTE + SIGNED_KEY_HELPER + "\n\n"
    projected = projected.replace("static int texture_coast_advance_edge", insertion +
                                  "static int texture_coast_advance_edge", 1)
    return _project_light_accumulator_union(projected)


def _project_light_accumulator_union(source: str) -> str:
    """Apply only the authorized 9BB518 int-to-existing-union representation."""
    old_declaration = "static int TextureRasterLightAccumB;"
    new_declaration = "static TextureRasterDwordHighWordView TextureRasterLightAccumB;"
    if source.count(new_declaration) == 1:
        return source
    assert source.count(old_declaration) == 1
    update = re.compile(
        r"\*reinterpret_cast<unsigned short \*>\s*\(&\(TextureRasterLightAccumB\)\)\s*"
        r"=\s*static_cast<unsigned short>\(\*reinterpret_cast<short \*>\s*"
        r"\(&\(TextureRasterLightAccumB\)\)\s*\+\s*"
        r"TextureRasterLightPackA\.low_word\);"
    )
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source)
    assert len(list(update.finditer(masked))) == 2
    source = _code_replace(source, update, "__LIGHT_ACCUM_LOW_UPDATE__")
    source = source.replace(old_declaration, "__LIGHT_ACCUM_DECLARATION__", 1)
    source = _code_replace(
        source, re.compile(r"\bTextureRasterLightAccumB\b"),
        "TextureRasterLightAccumB.dword")
    source = source.replace(
        "__LIGHT_ACCUM_DECLARATION__", new_declaration, 1)
    replacement = (
        "TextureRasterLightAccumB.words.low_word = static_cast<unsigned short>("
        "TextureRasterLightAccumB.words.low_word + TextureRasterLightPackA.low_word);"
    )
    source = source.replace("__LIGHT_ACCUM_LOW_UPDATE__", replacement)
    assert "__LIGHT_ACCUM_" not in source
    return source


def _cpp_tokens(source: str) -> list[str]:
    token = re.compile(
        r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|'
        r'//[^\n]*|/\*.*?\*/|[A-Za-z_]\w*|0[xX][0-9A-Fa-f]+|\d+|'
        r'::|->|\+\+|--|&&|\|\||==|!=|<=|>=|<<|>>|'
        r'\+=|-=|\*=|/=|%=|&=|\|=|\^=|##|.', re.S)
    return [value for match in token.finditer(source)
            if not (value := match.group(0)).startswith(("//", "/*"))
            and not value.isspace()]


def texture_asset_segment(assets: str) -> str:
    """Extract and validate just the contiguous moved Texture storage segment."""
    definitions = list(re.finditer(r"(?m)^Texture g_\w+(?:\[\d+\])?;\s*$", assets))
    assert len(definitions) == 20
    start = assets.rfind("// RECOVERY_OBJECT:", 0, definitions[0].start())
    assert start >= 0
    end = definitions[-1].end()
    segment = assets[start:end]
    assert segment.count("// RECOVERY_OBJECT:") == 20
    assert len(re.findall(r"(?m)^Texture g_\w+(?:\[\d+\])?;\s*$", segment)) == 20
    return segment


def restore_asset_block_for_comparison(current: str, baseline: str, assets: str) -> str:
    """Reassemble only moved Texture globals for an old TU comparison.

    Other application assets share interface_assets.cpp and are deliberately
    outside this projection. This validates storage conservation, not startup order.
    """
    start = "// ===== MANAGED GLOBALS - real objects, homed to their domain ====="
    end = "// Shared Texture-owned storage"
    assert baseline.count(start) == baseline.count(end) == 1
    expected = baseline[baseline.index(start):baseline.index(end)]
    segment = texture_asset_segment(assets)
    assert current.count(start) == 0 and current.count(end) == 1
    assert _cpp_tokens(segment) == _cpp_tokens(expected)
    assert len(re.findall(r"(?m)^Texture g_\w+(?:\[\d+\])?;\s*$", current)) == 0
    return current.replace(end, segment + "\n" + end, 1)


def reverse_direct_storage_source(current: str, baseline: str) -> str:
    """Validate the exact task projection and return its historical source form.

    This is useful to a sibling test whose immutable baseline predates the
    alias removal.  It does not excuse sibling deltas: first reverse that
    sibling's narrowly authorized change, then call this helper.  Comments,
    annotations and task metadata remain under the caller's existing checks.
    """
    if "// ===== MANAGED GLOBALS" not in current:
        assets = (Path(__file__).resolve().parents[2] / "src/interface_assets.cpp").read_text()
        current = restore_asset_block_for_comparison(current, baseline, assets)
    expected = direct_storage_projection(baseline)
    assert current.count(SIGNED_KEY_NOTE) == 1
    assert current.count(SIGNED_KEY_HELPER) == 1
    assert _cpp_tokens(current) == _cpp_tokens(expected), (
        "Texture direct-storage changes differ from the exact 53-alias projection")
    return baseline


def extract_method(source: str, signature: str) -> str:
    """Return one complete literal C++ function definition by signature."""
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source)
    assert masked.count(signature) == 1, (signature, masked.count(signature))
    start = masked.index(signature)
    opening = masked.index("{", start + len(signature))
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated function definition: {signature}")


def extract_key_bit_helper(source: str) -> str:
    """Return the exact private helper that representation-copies the key bits."""
    return extract_method(source, KEY_HELPER_SIGNATURE)


def direct_storage_prelude(source: str) -> str:
    """Return real mapped storage declarations and private helpers for fixtures.

    The result includes the direct TextureRaster objects/types and pointer
    arithmetic helpers from their source block, the genuine TextureRasterKey
    definition, and the current signed-key bit-copy helper.  It deliberately
    refuses the old generated `blit_*`/`g2_*` alias cells, ensuring fixture
    bodies operate on the same target objects as production.
    """
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source)
    assert source.count(DIRECT_STORAGE_START) == 1
    start = source.index(DIRECT_STORAGE_START)
    assert masked.count(DIRECT_STORAGE_END) == 1
    end = masked.index(DIRECT_STORAGE_END, start)
    block = source[start:end]

    alias_decl = re.compile(
        r"(?m)^\s*(?:static\s+)?[^;\n]*\b(?:blit_[A-Za-z0-9_]+|g2_[A-Za-z0-9_]+)\b[^;\n]*;"
    )
    aliases = alias_decl.findall(block)
    assert not aliases, f"generated pointer aliases remain in direct-storage block: {aliases}"

    assert source.count(KEY_OBJECT_MARKER) == 1
    key_start = source.index(KEY_OBJECT_MARKER)
    key_end = source.find(";", key_start)
    assert key_end >= 0
    key_definition = source[key_start:key_end + 1]
    assert re.search(r"\bunsigned\s+int\s+TextureRasterKey\s*;", key_definition)

    helper = extract_key_bit_helper(source)
    if helper in block:
        block = block.replace(helper, "", 1)
    return block + "\n" + key_definition + "\n" + helper + "\n"
