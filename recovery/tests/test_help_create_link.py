"""Host-check the product help-link formatter and its original delimiters."""

import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def test_help_link_format(tmp_path: Path) -> None:
    source = (ROOT / "src/datalink.cpp").read_text()
    start = source.index("void __cdecl help_create_link(")
    opening = source.index("{", start)
    depth = 0
    end = None
    for offset in range(opening, len(source)):
        if source[offset] == "{":
            depth += 1
        elif source[offset] == "}":
            depth -= 1
            if depth == 0:
                end = offset + 1
                break
    assert end is not None
    body = source[start:end]
    harness = (
        "#include <cassert>\n#include <cstring>\n"
        "#define __cdecl\n"
        + body
        + '\nint main() { char out[128] = "prefix "; char label[] = "Energy"; '
        'char id[] = "30042"; help_create_link(out, label, id); '
        'assert(strcmp(out, "prefix $LINK<Energy=30042>") == 0); }\n'
    )
    cpp = tmp_path / "link.cpp"
    binary = tmp_path / "link"
    cpp.write_text(harness)
    subprocess.run(["g++", "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
