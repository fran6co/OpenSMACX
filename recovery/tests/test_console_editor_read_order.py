"""Source-order controls for the pinned editor clear path.

The original body loads MapArea at 0x4E1231 and MapTiles at 0x4E1237 before
resetting MapRandSeed at 0x4E123E. Its initial guard uses that captured area,
while the loop tail reloads MapArea at 0x4E1271.
"""

from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[2]
from console_sources import console_method_source


def body(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for end in range(brace, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated source body")


def test_editor_clear_random_captures_initial_map_inputs_before_seed_reset():
    source = console_method_source("void Console::editor_clear_random()")
    method = body(source, "void Console::editor_clear_random() {")

    captures = re.search(
        r"const int initial_area\s*=\s*MapArea\s*;\s*"
        r"Map\s*\*\s*tile\s*=\s*MapTiles\s*;\s*"
        r"MapRandSeed\s*=\s*0\s*;",
        method,
    )
    assert captures is not None
    loop = method[captures.end():]
    assert re.search(r"if\s*\(\s*initial_area\s*>\s*0\s*\)", loop)
    assert re.search(r"while\s*\(\s*i\s*<\s*MapArea\s*\)", loop)
