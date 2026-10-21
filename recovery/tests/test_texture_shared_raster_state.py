"""Actual-source controls for the shared-state 0x6198E0 rasterizer path.

The full body and mapped `setup_edge` definition are extracted from
`src/texture.cpp`; host checks cover their typed-cell dataflow, not native ABI,
x87 status, callback observability, or full game behavior.
"""
from __future__ import annotations

from pathlib import Path

import pytest

from test_texture_vertices import packed_bodies, packed_harness, run_host


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/texture.cpp").is_file())
SOURCE = ROOT / "src/texture.cpp"


MAIN = r'''
int main() {
    Vert rectangle[] = {{1,1},{4,1},{4,2},{1,2}};
    Vert uv[] = {{1,0},{4,0},{4,0},{1,0}};
    start();
    sample(1,1,1,0); sample(2,1,1,0); sample(3,1,2,0); sample(4,1,3,0);
    finish(rectangle,4,uv);

    // The mapped helper used the published max index, count and both vertex
    // arrays, and the per-row cells retain the exact first-row edge values.
    require(TextureRasterMaxYIndex == 2);
    require(TextureRasterSpanRight == 1 && TextureRasterSpanLeft == 4);
    require(TextureRasterUAccumulator.dword == (1 << 16));
    require(TextureRasterVAccumulator.dword == 0);
    require(TextureRasterScratch4C4 == (4 << 16));
    require(TextureRasterScratch530 == 0);
    require(TextureRasterScratch4E4.dword == 65535);
    require(TextureRasterScratch4C8.dword == 0);

    // 9BB508 is a mixed-use target-width scratch word. For this one-row
    // raster, its value is the low 32 bits of base + minY*stride. Do not turn
    // it back into a host pointer.
    const uint32_t expected_row = static_cast<uint32_t>(
        reinterpret_cast<uintptr_t>(output + 16)) + 16u;
    require(static_cast<uint32_t>(TextureRasterRowOffset) == expected_row);

    // The second scanline is exactly the exclusive bottom row. The loop's
    // signed row-word comparison must reject it even though raw y is only 1.
    Vert bottom_clip[] = {{1,0},{4,0},{4,2},{1,2}};
    Vert bottom_clip_uv[] = {{2,3},{2,3},{2,3},{2,3}};
    start(); lock_clip={0,0,12,1};
    sample(1,0,2,3); sample(2,0,2,3); sample(3,0,2,3); sample(4,0,2,3);
    finish(bottom_clip,4,bottom_clip_uv);

    // Multiplying these signed-looking raw clip coordinates by the row
    // stride wraps in the target's 32-bit address word: top becomes base and
    // bottom becomes base+one stride, so the initial row is rejected.
    Vert wrapped_clip[] = {{1,1},{4,1},{4,2},{1,2}};
    Vert wrapped_uv[] = {{0,0},{3,0},{3,0},{0,0}};
    start(); lock_clip={0,0x40000000,12,0x40000001};
    finish(wrapped_clip,4,wrapped_uv,true,2);
    require(static_cast<uint32_t>(TextureRasterClipTop) ==
        static_cast<uint32_t>(reinterpret_cast<uintptr_t>(output + 16)));
    require(static_cast<uint32_t>(TextureRasterClipBottom) == expected_row);

    // A one-row span clipped at the left edge publishes the clipped X back
    // into the shared left-edge cell after its U/V skip.
    Vert clipped_x[] = {{1,0},{5,0},{5,1},{1,1}};
    Vert clipped_uv[] = {{0,0},{4,0},{4,0},{0,0}};
    start(); lock_clip={3,0,6,1};
    sample(3,0,2,0); sample(4,0,3,0); sample(5,0,4,0);
    finish(clipped_x,4,clipped_uv);
    require(TextureRasterSpanRight == 3);
    return 0;
}
'''


def harness_for(body_mutation: tuple[str, str] | None = None) -> str:
    methods = packed_bodies()
    if body_mutation is not None:
        before, after = body_mutation
        assert methods["draw_trans"].count(before) == 1, before
        methods["draw_trans"] = methods["draw_trans"].replace(before, after)
    full = packed_harness(methods, observe_initial=True, observe_rollover_state=True)
    marker = full.rfind("int main() {")
    assert marker >= 0
    return full[:marker] + MAIN


def test_original_shared_input_and_row_effects_follow_mapped_helper(tmp_path):
    source = SOURCE.read_text(encoding="utf-8")
    body = packed_bodies()["draw_trans"]
    ordered = [
        "TextureRasterPixelBase = buffer->get_data();",
        "texture_reset_fpu();",
        "TextureRasterEdgeVerticesB = vertices;",
        "TextureRasterScratch544 = pixels_;",
        "TextureRowPitchScratch = static_cast<int>(iWidth_);",
        "TextureRasterTextureHeight = static_cast<int>(iHeight_);",
        "TextureRasterVertexCount = count;",
        "TextureRasterEdgeVerticesA = uv;",
        "TextureRasterBufferStride = static_cast<int>(buffer->stride_);",
        "TextureRasterTextureWidth = static_cast<int>(iWidth_);",
        "TextureRasterClipRight = buffer->rect1_.right;",
        "TextureRasterClipLeft = buffer->rect1_.left;",
        "TextureRasterClipBottom = buffer->rect1_.bottom;",
        "TextureRasterClipTop = buffer->rect1_.top;",
    ]
    positions = [body.index(item) for item in ordered]
    assert positions == sorted(positions)
    row_offset = body.index("TextureRasterRowOffset = static_cast<int>(")
    clip_bottom = body.index("TextureRasterClipBottom = static_cast<int>(")
    clip_top = body.index("TextureRasterClipTop = static_cast<int>(")
    initial_edge = body.index("Texture::setup_edge(&left, top)")
    assert row_offset < clip_bottom < clip_top < initial_edge
    assert body.count("TextureRasterRowOffset >= TextureRasterClipBottom") == 2
    assert body.count("TextureRasterRowOffset >= TextureRasterClipTop") == 1
    row_store_order = [
        "TextureRasterScratch4C4 = right.accumA;",
        "TextureRasterScratch530 = right.accumB;",
        "TextureRasterSpanLeft = right.accumC;",
        "TextureRasterUAccumulator.dword = left.accumA;",
        "TextureRasterVAccumulator.dword = left.accumB;",
        "TextureRasterSpanRight = left.accumC;",
    ]
    row_positions = [body.index(item) for item in row_store_order]
    assert row_positions == sorted(row_positions)
    left_rollover = body.index("if (--left.counter == 0)")
    right_rollover = body.index("if (--right.counter == 0)")
    assert row_positions[-1] < left_rollover < right_rollover
    assert body.index("if (span > 0)") < body.index(
        "TextureRasterScratch4E4.dword = du;")
    assert body.index("if (span > 0)") < body.index(
        "TextureRasterScratch4C8.dword = dv;")
    assert left_rollover < right_rollover < body.index(
        "TextureRasterScratch4E4.dword = du;")
    clipped_store = body.index("TextureRasterSpanRight = TextureRasterClipLeft;")
    clip_branch = body.index("if (x < TextureRasterClipLeft)")
    assert body.index("TextureRasterVAccumulator.dword = static_cast<int>(", clip_branch) < clipped_store
    assert clipped_store < body.index("x = TextureRasterSpanRight;", clipped_store)
    assert source.count("int __cdecl Texture::setup_edge(EdgeScan *a1, int a2) {") == 1
    assert body.count("Texture::setup_edge(&left, top)") == 1
    assert body.count("Texture::setup_edge(&right, top)") == 1
    assert body.count("Texture::setup_edge(&left, left.id)") == 1
    assert body.count("Texture::setup_edge(&right, right.id)") == 1

    result = run_host(tmp_path, harness_for(), disable_shift_ubsan=True)
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("mutation", [
    ("TextureRasterEdgeVerticesA = uv;", "TextureRasterEdgeVerticesA = vertices;"),
    ("pixel_base + static_cast<uint32_t>(min_y) * stride", "pixel_base"),
    ("TextureRasterSpanRight = left.accumC;", "TextureRasterSpanRight = left.accumC + 1;"),
    ("TextureRasterScratch4E4.dword = du;", "TextureRasterScratch4E4.dword = dv;"),
    ("TextureRasterVertexCount = count;", "TextureRasterVertexCount = 3;"),
    ("pixel_base + stride * static_cast<uint32_t>(TextureRasterClipBottom)",
     "pixel_base + stride * static_cast<uint32_t>(TextureRasterClipTop)"),
    ("pixel_base + stride * static_cast<uint32_t>(TextureRasterClipTop)",
     "pixel_base + stride * static_cast<uint32_t>(TextureRasterClipBottom)"),
    ("if (TextureRasterRowOffset >= TextureRasterClipBottom) break;",
     "if (y >= TextureRasterClipBottom) break;"),
    ("TextureRasterRowOffset >= TextureRasterClipTop",
     "static_cast<uint32_t>(y) >= static_cast<uint32_t>(TextureRasterClipTop)"),
    ("TextureRasterSpanRight = TextureRasterClipLeft;",
     "TextureRasterSpanRight = x;"),
    ("int done = 0;", "if (span > 0) {\n"
     "                du = texture_span_step(texture_difference(TextureRasterScratch4C4, u), span >> 16);\n"
     "                dv = texture_span_step(texture_difference(TextureRasterScratch530, v), span >> 16);\n"
     "                TextureRasterScratch4E4.dword = du;\n"
     "                TextureRasterScratch4C8.dword = dv;\n            }\n            int done = 0;"),
])
def test_shared_state_damage_mutants_are_rejected(tmp_path, mutation):
    result = run_host(tmp_path, harness_for(mutation), disable_shift_ubsan=True)
    assert result.returncode == 42, result.stderr


def test_compiling_left_edge_snapshot_after_rollover_is_rejected(tmp_path):
    methods = packed_bodies()
    body = methods["draw_trans"]
    snapshot = "TextureRasterUAccumulator.dword = left.accumA;"
    rollover = "if (!Texture::setup_edge(&left, left.id)) done = 1;"
    assert body.count(snapshot) == body.count(rollover) == 1
    body = body.replace(snapshot, "", 1)
    body = body.replace(rollover, rollover + "\n                " + snapshot, 1)
    methods["draw_trans"] = body
    full = packed_harness(methods, observe_initial=True, observe_rollover_state=True)
    marker = full.rfind("int main() {")
    assert marker >= 0
    result = run_host(tmp_path, full[:marker] + MAIN, disable_shift_ubsan=True)
    assert result.returncode == 42, result.stderr
