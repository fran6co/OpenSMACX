import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile


HERE = Path(__file__).resolve().parent
DEFAULT_SOURCE = next(parent / "src/caviar.cpp" for parent in HERE.parents
                      if (parent / "src/caviar.cpp").is_file())
FIXTURE = HERE / "fixtures/caviar_product_extraction.cpp"
FUNCTIONS = (
    "caviar_projection_floor",
    "caviar_projection_copy_float_bits",
    "vox_stage_scene",
    "caviar_clip_shape",
    "vox_load_scene_view",
    "caviar_direction_to_projection_index",
)


def extract_definition(source: str, name: str) -> str:
    for match in re.finditer(r"\b" + re.escape(name) + r"\s*\(", source):
        start = source.rfind("\n", 0, match.start()) + 1
        while start < match.start() and source[start].isspace() and source[start] != "\n":
            start += 1
        # The return type/calling convention begin on this line for the
        # selected product declarations; retain them with the actual body.
        open_paren = source.find("(", match.start())
        depth = 0
        close_paren = None
        for index in range(open_paren, len(source)):
            if source[index] == "(":
                depth += 1
            elif source[index] == ")":
                depth -= 1
                if depth == 0:
                    close_paren = index
                    break
        if close_paren is None:
            continue
        body_start = close_paren + 1
        while body_start < len(source) and source[body_start].isspace():
            body_start += 1
        if body_start >= len(source) or source[body_start] != "{":
            continue

        depth = 0
        state = "code"
        index = body_start
        while index < len(source):
            current = source[index]
            following = source[index + 1] if index + 1 < len(source) else ""
            if state == "code":
                if current == '"':
                    state = "string"
                elif current == "'":
                    state = "char"
                elif current == "/" and following == "/":
                    state = "line_comment"
                    index += 1
                elif current == "/" and following == "*":
                    state = "block_comment"
                    index += 1
                elif current == "{":
                    depth += 1
                elif current == "}":
                    depth -= 1
                    if depth == 0:
                        return source[start:index + 1]
            elif state == "string":
                if current == "\\":
                    index += 1
                elif current == '"':
                    state = "code"
            elif state == "char":
                if current == "\\":
                    index += 1
                elif current == "'":
                    state = "code"
            elif state == "line_comment" and current == "\n":
                state = "code"
            elif state == "block_comment" and current == "*" and following == "/":
                state = "code"
                index += 1
            index += 1
    raise AssertionError("no function definition found for " + name)


def test_extracted_product_bodies_gcc_and_clang():
    source_path = Path(os.environ.get("OPENSMACX_CAVIAR_SOURCE", DEFAULT_SOURCE))
    source = source_path.read_text()
    extracted = "\n\n".join(extract_definition(source, name) for name in FUNCTIONS)
    template = FIXTURE.read_text()
    assert template.count("/* PRODUCT_EXTRACTS */") == 1
    translation_unit = template.replace("/* PRODUCT_EXTRACTS */", extracted)

    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    available = [compiler for compiler in compilers if compiler]
    assert available, "GCC or Clang is required for extracted product-source controls"
    with tempfile.TemporaryDirectory(prefix="caviar-product-extract-") as directory:
        directory = Path(directory)
        source_file = directory / "extracted.cpp"
        source_file.write_text(translation_unit)
        for compiler in available:
            executable = directory / (Path(compiler).name.replace("+", "p"))
            subprocess.run(
                [compiler, "-std=c++98", "-Wall", "-Wextra", "-I", str(HERE),
                 str(source_file), "-o", str(executable)],
                check=True, timeout=60,
            )
            subprocess.run([str(executable)], check=True, timeout=30)
