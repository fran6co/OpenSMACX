from __future__ import annotations

from pathlib import Path
from runpy import run_path
from types import SimpleNamespace

import pytest


ROOT = Path(__file__).resolve().parents[1]
PRODUCT = ROOT.parent


def test_existing_terrain_hook_retains_all_calls_and_project_hook_extends_it() -> None:
    namespace = run_path(str(ROOT / "terrain_data.py"))
    calls: list[str | tuple[str, Path]] = []
    functions = namespace["terrain_initializers"].__globals__
    for name in (
        "terrain_coast_initializer",
        "terrain_landmark_initializer",
        "terrain_altitude_initializer",
        "terrain_direction_initializer",
        "terrain_landmark_bonus_initializer",
    ):
        functions[name] = lambda *args, _name=name: calls.append(_name)

    namespace["terrain_initializers"](Path("/irrelevant"), Path("/irrelevant"),
                                      Path("/irrelevant"))
    assert calls == [
        "terrain_coast_initializer",
        "terrain_landmark_initializer", "terrain_landmark_initializer",
        "terrain_landmark_initializer", "terrain_altitude_initializer",
        "terrain_altitude_initializer", "terrain_altitude_initializer",
        "terrain_direction_initializer",
        "terrain_landmark_bonus_initializer", "terrain_landmark_bonus_initializer",
        "terrain_landmark_bonus_initializer", "terrain_landmark_bonus_initializer",
        "terrain_landmark_bonus_initializer",
    ]

    calls[:] = []
    functions["terrain_initializers"] = lambda *args: calls.append("terrain")
    functions["movie_decoder_initializers"] = lambda *args: calls.append("decoder")
    functions["graphicwin_palette_initializer"] = lambda *args: calls.append("graphicwin")
    functions["caviar_projection_rows_initializer"] = (
        lambda root, original, output: calls.append(("caviar", output)))
    namespace["project_initializers"](Path("/irrelevant"), Path("/irrelevant"),
                                       Path("/irrelevant"))
    assert calls == [
        "terrain", "decoder", "graphicwin",
        ("caviar", Path("/irrelevant") / "caviar_angle_rows.inc"),
    ]


def test_cmake_configure_uses_extended_hook_without_replacing_legacy_entry() -> None:
    cmake = (PRODUCT / "CMakeLists.txt").read_text()
    assert "['project_initializers']" in cmake
    assert "['terrain_initializers']" not in cmake
    assert '"${CMAKE_CURRENT_SOURCE_DIR}/recovery/terrain_data.py"' in cmake
    assert '"${CMAKE_CURRENT_SOURCE_DIR}/src/movie_media_decoder.cpp"' in cmake
    assert '"${CMAKE_CURRENT_SOURCE_DIR}/src/graphicwin.cpp"' in cmake


def test_decoder_initializers_require_v1_start_only_access_windows(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch,
) -> None:
    namespace = run_path(str(ROOT / "terrain_data.py"))
    functions = namespace["movie_decoder_initializers"].__globals__
    entries = namespace["_MOVIE_DECODER_DATA"]
    assert all(len(item) == 5 and item[4] == "accessed_window"
               for item in entries)
    coefficient_entry = next(item for item in entries
                             if item[0] == "movie_decoder_constants.inc")
    assert coefficient_entry == (
        "movie_decoder_constants.inc", 0x009C9594, 0x009C9AA8, "words",
        "accessed_window",
    )

    exact_span = functions["Span"]
    bindings = []
    for entry in entries:
        start, end = entry[1:3]
        bindings.append(SimpleNamespace(
            unit="src/movie_media_decoder.cpp",
            origin=SimpleNamespace(address=start),
            span=None,
        ))

    class Image:
        sections = []
        section_flags = []
        nonzero_bss = False
        read_spans: list[object] = []

        def __init__(self, _data: bytes, _pin: object) -> None:
            pass

        @classmethod
        def read(cls, span: object) -> bytes:
            if span.start == 0x009B2D64:
                raise ValueError("BSS window has no unique file backing")
            cls.read_spans.append(span)
            return bytes(span.end - span.start)

        @classmethod
        def read_initial(cls, span: object) -> bytes:
            if span.start == 0x009B2D64 and cls.nonzero_bss:
                return b"\x01" + bytes(span.end - span.start - 1)
            return bytes(span.end - span.start)

        @staticmethod
        def absolute_sites(_span: object) -> tuple[object, ...]:
            return ()

    pin = object()
    functions["image_pin"] = lambda _root: pin
    functions["read_objects"] = lambda _root, _pin: bindings
    functions["PeImage"] = Image
    functions["artifact_parent"] = lambda *_args, **_kwargs: None

    def run_at(root: Path) -> None:
        original = root / "original.exe"
        original.write_bytes(b"pinned test image")
        output = root / "build" / "recovery" / "terrain-data"
        output.mkdir(parents=True)
        namespace["movie_decoder_initializers"](root, original, output)

    accepted_root = tmp_path / "accepted"
    accepted_root.mkdir()
    run_at(accepted_root)
    generated = (accepted_root / "build" / "recovery" / "terrain-data"
                 / "movie_decoder_constants.inc")
    assert generated.is_file()
    assert generated.read_text().startswith(
        "// Generated from pinned, source-owned data; do not edit.\n")
    assert all(span.start != 0x009B2D64 for span in Image.read_spans)

    Image.nonzero_bss = True
    mutant_root = tmp_path / "nonzero-bss"
    mutant_root.mkdir()
    original = mutant_root / "original.exe"
    original.write_bytes(b"pinned test image")
    output = mutant_root / "build" / "recovery" / "terrain-data"
    output.mkdir(parents=True)
    with pytest.raises(ValueError, match="zero-initialized source storage"):
        namespace["movie_decoder_initializers"](mutant_root, original, output)
    Image.nonzero_bss = False

    coefficient_binding = next(
        item for item in bindings
        if item.origin.address == 0x009C9594
    )
    coefficient_binding.span = exact_span(0x009C9594, 0x009C9AA8)
    rejected_root = tmp_path / "rejected"
    rejected_root.mkdir()
    original = rejected_root / "original.exe"
    original.write_bytes(b"pinned test image")
    output = rejected_root / "build" / "recovery" / "terrain-data"
    output.mkdir(parents=True)
    with pytest.raises(ValueError, match="v1 start-only source binding"):
        namespace["movie_decoder_initializers"](rejected_root, original, output)
