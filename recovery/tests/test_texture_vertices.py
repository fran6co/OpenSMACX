"""Actual-source vertex controls; host tests are not native ABI/raster proof.

The Vert declaration and the bounded vertex scan from Texture::draw at 61B1F0
are extracted unchanged. Local count/index slots explicitly double its global
dependencies. The remainder of that draw function is not executed by the scan
test: it still has separate depth, edge, buffer and raster obligations.

The color-key test executes the complete live 61A240 draw_trans and its local
helpers. Buffer lock/unlock and floating-point environment calls are explicit
doubles; native layout, actual x87 flags, shared scratch side effects and game
rendering remain outside this host evidence.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def braced(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for end in range(brace, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated product definition")


def vertex_declaration():
    return braced((ROOT / "src/vert.h").read_text(), "struct Vert {") + ";"


def vertex_scan():
    method = braced((ROOT / "src/texture.cpp").read_text(),
                    "void Texture::draw(Buffer *a1, Vert *a2, int *a3, int a4, Vert *a5) {")
    # minY's declaration precedes three unrelated clip-slot assignments.
    # Preserve its actual initializer, then the entire following range scan.
    min_y = re.search(r"int minY = [^;]+;", method).group()
    start = method.index("int maxY =")
    end = method.index("if (minY < maxY)", start)
    # The selected source ends inside `if (count > 0)`, after the complete
    # for-loop. Close that surrounding guard in the extraction wrapper only.
    return min_y + "\n" + method[start:end] + "}\n"


def scan_harness(declaration, scan):
    return r'''
        #include <cstddef>
        #include <cstdint>
        #include <cstdio>
        #include <cstdlib>
        #include <cstring>
        #include <initializer_list>
        #include <type_traits>
        #define require(c) do { if (!(c)) { \
            std::fprintf(stderr, "line %d: %s\n", __LINE__, #c); \
            std::exit(42); } } while (0)
        ACTUAL_VERTEX
        struct Bounds { int left, top, right, bottom, first_top, first_bottom; };
        Bounds scan(Vert *a2, int requested_count) {
            int count_slot = requested_count, bottom_slot = 91;
            int *blit_vertex_count = &count_slot;
            int *blit_max_y_index = &bottom_slot;
            ACTUAL_SCAN
            return {minX, minY, maxX, maxY, topVert, bottom_slot};
        }
        void check(Vert *vertices, int count, Bounds expected) {
            const Bounds actual = scan(vertices, count);
            require(actual.left == expected.left && actual.right == expected.right);
            require(actual.top == expected.top && actual.bottom == expected.bottom);
            require(actual.first_top == expected.first_top);
            require(actual.first_bottom == expected.first_bottom);
        }
        int main() {
            // Host layout observation only; native layout is checked elsewhere.
            require(sizeof(Vert) == 8 && offsetof(Vert, x) == 0 && offsetof(Vert, y) == 4);
            require((std::is_same<decltype(Vert::x), int>::value));
            require((std::is_same<decltype(Vert::y), int>::value));
            require(sizeof(int) == 4);
            Vert vertices[] = {{-4, 5}, {8, -9}, {-10, 12}, {2, -9},
                               {-20, 20}, {22, -30}, {1, 20}, {22, -30}};
            Vert preserved[8];
            std::memcpy(preserved, vertices, sizeof(vertices));
            check(vertices, 4, {-10, -9, 8, 12, 1, 2});
            check(vertices, 8, {-20, -30, 22, 20, 5, 4});
            require(std::memcmp(vertices, preserved, sizeof(vertices)) == 0);
            Vert tied[] = {{-13, -21}, {-13, -21}, {-13, -21}, {-13, -21},
                          {-13, -21}, {-13, -21}, {-13, -21}, {-13, -21}};
            for (int count : {4, 8}) check(tied, count, {-13, -21, -13, -21, 0, 0});
            // Four rows must not consume later vertices; equal extrema keep
            // the first index, and an empty input preserves the bottom slot.
            check(vertices, 0, {32767, 32767, -32765, -32765, 0, 91});
            check(vertices, -1, {32767, 32767, -32765, -32765, 0, 91});
        }
    '''.replace("ACTUAL_VERTEX", declaration).replace("ACTUAL_SCAN", scan)


def run_host(tmp_path, source):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host g++ required")
    cpp = tmp_path / "vertices.cpp"
    executable = tmp_path / "vertices"
    cpp.write_text(source)
    compiled = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
         "-fno-sanitize-recover=undefined", str(cpp), "-o", str(executable)],
        capture_output=True, text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)


def test_actual_vertex_declaration_and_draw_scan(tmp_path):
    result = run_host(tmp_path, scan_harness(vertex_declaration(), vertex_scan()))
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("before,after", [
    ("if (y < minY)", "if (y <= minY)"),
    ("if (maxY < y)", "if (maxY <= y)"),
    ("topVert = i;", "topVert = 0;"),
    ("*blit_max_y_index = i;", "*blit_max_y_index = 0;"),
    ("int count = *blit_vertex_count;", "int count = 4;"),
    ("int y = v->y;", "int y = v->x;"),
    ("int x = v->x;", "int x = v->y;"),
])
def test_compiling_vertex_scan_mutations_are_rejected(tmp_path, before, after):
    scan = vertex_scan()
    assert scan.count(before) == 1
    result = run_host(tmp_path, scan_harness(vertex_declaration(), scan.replace(before, after)))
    assert result.returncode == 42, result.stderr


def raster_bodies():
    source = (ROOT / "src/texture.cpp").read_text()
    names = ("texture_fixed", "texture_difference", "texture_edge",
             "texture_advance_edge", "texture_span_step")
    result = {name: braced(source, "static int " + name + "(") for name in names}
    result["texture_reset_fpu"] = braced(source, "static void texture_reset_fpu() {")
    result["draw_trans"] = braced(source, "void Texture::draw_trans(Buffer *buffer, Vert *vertices, int use_eight_vertices) {")
    return result


def x87_span_step_expected(delta, width):
    """Exact rational reciprocal/product rounded separately to 24 bits, chop.

    Original AND AX,FCFF / OR AX,0C00 selects PC24 and chop after FNINIT.
    FILD loads delta exactly before multiplication; it is not pre-rounded.
    This exponent/significand oracle models both operations with unbounded
    integers independently of the C++ helper. It does not execute a host FPU.
    """
    divisor = width << 16
    exponent = -(divisor - 1).bit_length() - 23
    significand = (1 << -exponent) // divisor
    product = significand * abs(delta)
    dropped = max(0, product.bit_length() - 24)
    product >>= dropped
    exponent += dropped + 16
    magnitude = product << exponent if exponent >= 0 else product >> -exponent
    return -magnitude if delta < 0 else magnitude


def raster_harness(methods):
    header = (ROOT / "src/texture.h").read_text()
    records = "\n".join(braced(header, "struct " + name + " {") + ";"
                        for name in ("EdgeScan", "EdgePoint"))
    texture_class = braced(header, "class Texture {")
    # Actual storage declarations, made public only in this compact fixture.
    fields = []
    for name in ("pixels_", "iWidth_", "iHeight_", "corners_", "cornersAndMids_", "borrowed_"):
        matches = re.findall(r"^\s*((?:void\s*\*|uint32_t\s+|EdgePoint\s+)"
                             + name + r"(?:\[[^\]]+\])?\s*;)", texture_class, re.M)
        assert len(matches) == 1, name
        fields.append(matches[0])
    cases = []
    for width in (1, 2, 3, 4, 5, 7, 8, 31, 127, 255, 256, 1023, 32767):
        multiple = width * 65536
        for delta in (0, 1, -1, 196608, -196608, 196609, -196609,
                      multiple - 1, multiple, multiple + 1,
                      -multiple - 1, -multiple, -multiple + 1,
                      2147483647, 2147483646, -2147483647, -2147483648):
            cases.append("{%d, %d, %d}" % (delta, width, x87_span_step_expected(delta, width)))
    return r'''
        #include <cstdint>
        #include <cstdio>
        #include <cstdlib>
        #include <cstring>
        #include <initializer_list>
        #include <vector>
        #define require(c) do { if (!(c)) { \
            std::fprintf(stderr, "line %d: %s\n", __LINE__, #c); \
            std::exit(42); } } while (0)
        ACTUAL_VERTEX
        ACTUAL_RECORDS
        // Distinct symbolic fields let the double check the requested policy;
        // these are not a replacement implementation of the floating-point API.
        const unsigned _MCW_EM=0x003f, _MCW_PC=0x0300, _MCW_RC=0x0c00, _MCW_IC=0x1000;
        const unsigned _PC_64=0x0300, _PC_24=0, _RC_NEAR=0, _IC_PROJECTIVE=0, _RC_CHOP=0x0c00;
        std::vector<int> events;
        void _fpreset() { events.push_back(2); }
        unsigned _control87(unsigned value, unsigned mask) {
            if (mask == (_MCW_PC | _MCW_RC)) {
                require(value == (_PC_24 | _RC_CHOP)); events.push_back(4);
            } else {
                require(mask == (_MCW_EM | _MCW_PC | _MCW_RC | _MCW_IC));
                require(value == (_MCW_EM | _PC_64 | _RC_NEAR | _IC_PROJECTIVE));
                events.push_back(3);
            }
            return 0;
        }
        unsigned int TextureRasterKey;
        struct RECT { int32_t left, top, right, bottom; };
        static unsigned char output[16 * 14], expected[16 * 14], pixels[8 * 8];
        RECT lock_clip;
        unsigned lock_key, lock_stride;
        struct Buffer {
            RECT rect1_;
            uint32_t stride_;
            intptr_t get_data() {
                require(events.empty()); events.push_back(1);
                // The lock publishes current clip, stride and key. All are
                // different from the stale values the fixture starts with.
                rect1_ = lock_clip; stride_ = lock_stride; TextureRasterKey = lock_key;
                return reinterpret_cast<intptr_t>(output + 16);
            }
            void free_data(int count) {
                require(count == 1 && events.size() >= 3);
                require(events.front() == 1 && events.back() != 5);
                events.push_back(5);
            }
        } buffer;
        struct Texture {
            ACTUAL_FIELDS
            void draw_trans(Buffer *, Vert *, int);
        } texture;
        ACTUAL_BODIES
        void start() {
            events.clear();
            std::memset(output, 0xee, sizeof(output));
            std::memset(expected, 0xee, sizeof(expected));
            for (unsigned i=0; i<sizeof(pixels); ++i) pixels[i] = i + 1;
            texture.pixels_ = pixels; texture.iWidth_ = texture.iHeight_ = 8;
            texture.borrowed_ = 1;
            buffer.rect1_ = {0,0,0,0}; buffer.stride_ = 8;
            lock_clip = {0,0,12,12}; lock_stride = 16;
            TextureRasterKey = 255; lock_key = 0x120002u;
            for (auto &uv : texture.corners_) uv = {0,0};
            for (auto &uv : texture.cornersAndMids_) uv = {7,7};
        }
        void sample(int x, int y, int u, int v) {
            const unsigned char value = pixels[v*8+u];
            if (value != static_cast<unsigned char>(lock_key))
                expected[16+y*lock_stride+x] = value;
        }
        void finish(Vert *vertices, int selector, bool edges=true) {
            texture.draw_trans(&buffer, vertices, selector);
            const std::vector<int> normal = {1,2,3,4,5,2,3};
            const std::vector<int> empty = {1,2,3,5,2,3};
            require(events == (edges ? normal : empty));
            require(std::memcmp(output, expected, sizeof(output)) == 0);
        }
        int main() {
            const int slope_cases[][3] = { SLOPE_CASES };
            for (const auto &c : slope_cases) require(texture_span_step(c[0], c[1]) == c[2]);
            // Independently reviewed PC24 anchors. PC64 gives a different
            // answer even for width1: FMUL still rounds the exact FILD value.
            require(texture_span_step(2147483647, 1) == 2147483520);
            require(texture_span_step(-2147483647, 1) == -2147483520);
            require(texture_span_step(2147483647, 3) == 715827776);
            require(texture_span_step(2147483647, 32767) == 65537);
            require(texture_span_step(196608, 3) == 65535);
            require(texture_span_step(2147418113, 32767) == 65535);
            require(texture_span_step(-2147418113, 32767) == -65535);
            require(texture_fixed(-1) == -65536);
            require(texture_difference(-2147483647-1, 1) == 2147483647);

            // Independent rectangle oracle: power-of-two horizontal width
            // and unit UV slope make sampled texels exactly the listed grid.
            Vert rectangle[] = {{1,1},{5,1},{5,4},{1,4}};
            start();
            texture.corners_[0]={0,0}; texture.corners_[1]={4,0};
            texture.corners_[2]={4,3}; texture.corners_[3]={0,3};
            for (int y=1; y<4; ++y) for (int x=1; x<5; ++x) sample(x,y,x-1,y-1);
            finish(rectangle, 0);

            // Non-power-of-two span: 3/3 rounded toward zero twice produces
            // step65535, hence texels [0,0,1], not [0,1,2].
            Vert short_row[] = {{1,1},{4,1},{4,2},{1,2}};
            start();
            texture.corners_[0]={0,0}; texture.corners_[1]={3,0};
            texture.corners_[2]={3,0}; texture.corners_[3]={0,0};
            sample(1,1,0,0); sample(2,1,0,0); sample(3,1,1,0);
            finish(short_row, 0);
            start();
            texture.corners_[0]={3,3}; texture.corners_[1]={0,3};
            texture.corners_[2]={0,3}; texture.corners_[3]={3,3};
            sample(1,1,3,3); sample(2,1,2,3); sample(3,1,1,3);
            finish(short_row, 0);

            // Clipping all four sides of a polygon with negative coordinates.
            // At original X=-2, U=0 and du=65535; skipping two gives U=1.ffffe.
            Vert clipped[] = {{-2,-1},{4,-1},{4,3},{-2,3}};
            start(); lock_clip={0,0,2,2};
            texture.corners_[0]={0,0}; texture.corners_[1]={6,0};
            texture.corners_[2]={6,4}; texture.corners_[3]={0,4};
            sample(0,0,1,1); sample(1,0,2,1); sample(0,1,1,2); sample(1,1,2,2);
            finish(clipped, 0);

            // Original negative-X edge DDA (-2/4) delays its first correction:
            // left rows [2,2,1,1], right [6,6,5,5], U=0..3 and V=row.
            Vert sloped[] = {{2,0},{6,0},{4,4},{0,4}};
            start();
            texture.corners_[0]={0,0}; texture.corners_[1]={4,0};
            texture.corners_[2]={4,4}; texture.corners_[3]={0,4};
            const int left[] = {2,2,1,1};
            for (int y=0; y<4; ++y) for (int u=0; u<4; ++u) sample(left[y]+u,y,u,y);
            finish(sloped, 0);

            // Eight-point perimeter: horizontal skips, both edge refreshes at
            // row2, and the last row still drawn after the left edge ends.
            for (int selector : {1, -7}) for (bool repeat_top : {false,true}) {
                Vert octagon[] = {{0,0},{2,0},{4,0},{4,2},{4,4},{2,4},{0,4},{0,2}};
                if (repeat_top) octagon[1]={0,0};
                start();
                for (auto &uv : texture.corners_) uv={7,7};
                for (int i=0; i<8; ++i) texture.cornersAndMids_[i]={octagon[i].x,octagon[i].y};
                for (int y=0; y<4; ++y) for (int x=0; x<4; ++x) sample(x,y,x,y);
                finish(octagon, selector);
            }

            // V as well as U can step backwards across a scanline.
            Vert reverse[] = {{1,1},{5,1},{5,2},{1,2}};
            start();
            texture.corners_[0]={4,4}; texture.corners_[1]={0,0};
            texture.corners_[2]={0,0}; texture.corners_[3]={4,4};
            for (int i=0; i<4; ++i) sample(1+i,1,4-i,4-i);
            finish(reverse, 0);

            Vert flat[] = {{0,2},{2,2},{4,2},{0,2}};
            start(); finish(flat, 0, false);
            Vert point[] = {{0,0},{0,0},{0,0},{0,0}};
            start(); finish(point, 0, false);
            start(); lock_clip={0,0,0,0}; finish(rectangle, 0);
        }
    '''.replace("ACTUAL_VERTEX", vertex_declaration()).replace("ACTUAL_RECORDS", records).replace(
        "ACTUAL_FIELDS", "\n".join(fields)).replace("ACTUAL_BODIES", "\n".join(methods.values())).replace(
        "SLOPE_CASES", ",\n".join(cases))


def test_actual_color_key_raster_and_helpers(tmp_path):
    result = run_host(tmp_path, raster_harness(raster_bodies()))
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("method,before,after", [
    ("draw_trans", "const int count = use_eight_vertices ? 8 : 4;", "const int count = 4;"),
    ("draw_trans", "if (texel != key) *out = texel;", "*out = texel;"),
    ("draw_trans", "const RECT clip = buffer->rect1_;", "RECT clip = buffer->rect1_; ++clip.bottom;"),
    ("draw_trans", "if (y >= clip.top && y < clip.bottom", "if (y > clip.top && y < clip.bottom"),
    ("draw_trans", "if (end > clip.right) end = clip.right;", "if (end > clip.right) end = clip.right + 1;"),
    ("draw_trans", "if (x < clip.left)", "if (false)"),
    ("draw_trans", "if (done) break;", "if (done) break; if (y == min_y) break;"),
    ("draw_trans", "if (y >= clip.top && y < clip.bottom", "if (!done && y >= clip.top && y < clip.bottom"),
    ("draw_trans", "buffer->free_data(1);\n    texture_reset_fpu();",
     "texture_reset_fpu();\n    buffer->free_data(1);"),
    ("texture_edge", "edge.fracAccum = dx < 0 ? 1 - edge.counter : 0;", "edge.fracAccum = 0;"),
    ("texture_edge", "index = next;", "return 0;"),
    ("texture_advance_edge", "if (--edge.counter == 0)", "if (--edge.counter <= 1)"),
    ("texture_span_step", "int reciprocal_shift = 23;", "int reciprocal_shift = 24;"),
    # The unrounded product is at most 55 bits. Keeping all 55 incorrectly gives
    # the multiply the same exactness it would have under PC64.
    ("texture_span_step", "while (product >= (static_cast<uint64_t>(1) << 24))",
     "while (product >= (static_cast<uint64_t>(1) << 55))"),
    ("draw_trans", "_control87(_PC_24 | _RC_CHOP, _MCW_PC | _MCW_RC);",
     "_control87(_PC_64 | _RC_CHOP, _MCW_PC | _MCW_RC);"),
    ("draw_trans", "_control87(_PC_24 | _RC_CHOP, _MCW_PC | _MCW_RC);",
     "_control87(_RC_CHOP, _MCW_RC);"),
    ("texture_reset_fpu", "_PC_64", "0"),
])
def test_compiling_raster_mutations_are_rejected(tmp_path, method, before, after):
    actual = raster_bodies()
    assert actual[method].count(before) == 1
    actual[method] = actual[method].replace(before, after)
    result = run_host(tmp_path, raster_harness(actual))
    assert result.returncode == 42, result.stderr


@pytest.mark.parametrize("declaration", [
    "const unsigned char key = static_cast<unsigned char>(TextureRasterKey);",
    "const int stride = static_cast<int>(buffer->stride_);",
    "const RECT clip = buffer->rect1_;",
])
def test_lock_callback_state_cannot_be_captured_early(tmp_path, declaration):
    actual = raster_bodies()
    method = actual["draw_trans"]
    assert method.count(declaration) == 1
    method = method.replace(declaration, "")
    brace = method.index("{") + 1
    actual["draw_trans"] = method[:brace] + "\n" + declaration + method[brace:]
    result = run_host(tmp_path, raster_harness(actual))
    assert result.returncode == 42, result.stderr
