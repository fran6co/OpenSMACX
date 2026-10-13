"""Actual-source vertex controls; host tests are not native ABI/raster proof.

The Vert declaration and the bounded vertex scan from Texture::draw at 61B1F0
are extracted unchanged. Local count/index slots explicitly double its global
dependencies. The remainder of that draw function is not executed by the scan
test: it still has separate depth, edge, buffer and raster obligations.

The color-key test executes the complete live 61A240 draw_trans and its local
helpers. Buffer lock/unlock and floating-point environment calls are explicit
doubles; native layout, actual x87 flags, shared scratch side effects and game
rendering remain outside this host evidence.

The packed 6198E0 test retains the actual body and typed edge helpers, with a
delegating call observer for its initial edge setups. Guarded host buffers test
inclusive opaque copying and independent packed coordinate arithmetic, not the
original allocator's 64-KiB alignment or shared-scratch side effects.

The keyed 61A760 and alternating 6239E0 tests execute those complete bodies
with caller-owned vertex counts and explicit UV arrays, including the packed
countdown's zero/inverted clipped spans and its high-word 0x8000 boundary,
absolute even/odd and negative-Y parity, low-byte alternate colors, and the
exactly-minus-one forward that precedes the lock. The countdown oracle and
wide fixtures stay inside allocated arrays so rejected mutants fail by
comparison rather than by writing out of bounds.

The 61AC60 cutoff controls extract its whole body and real Gouraud helpers.
Independent rectangular planes exercise every-row lighting, clipping and UV
carries. Lock/FPU calls remain doubles, not native lock or x87-status proof.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest
from texture_direct_storage_host import PRELUDE_MARKER, direct_storage_prelude


ROOT = Path(__file__).resolve().parents[2]


def braced(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    # A helper may be forward-declared before the mapped caller. Extract the
    # definition itself, never the declaration plus the next unrelated body.
    while ";" in source[start:brace]:
        start = source.index(signature, start + len(signature))
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
    source = (ROOT / "src/texture.cpp").read_text()
    harness = r'''
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
        // TEXTURE_DIRECT_STORAGE_PRELUDE
        struct Bounds { int left, top, right, bottom, first_top, first_bottom; };
        Bounds scan(Vert *a2, int requested_count) {
            TextureRasterVertexCount = requested_count;
            TextureRasterMaxYIndex = 91;
            ACTUAL_SCAN
            return {minX, minY, maxX, maxY, topVert, TextureRasterMaxYIndex};
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
    assert harness.count(PRELUDE_MARKER) == 1
    return harness.replace(PRELUDE_MARKER, direct_storage_prelude(source), 1)


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
    ("TextureRasterMaxYIndex = i;", "TextureRasterMaxYIndex = 0;"),
    ("int count = TextureRasterVertexCount;", "int count = 4;"),
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
    result = {name: braced(source, "static int " + name + "(")
              for name in ("texture_fixed", "texture_difference")}
    for coordinate_type in ("Vert",):
        for axis in ("u", "v"):
            signature = "static int texture_" + axis + "(const " + coordinate_type + " &uv)"
            result[axis + "_" + coordinate_type] = braced(source, signature)
    for name in ("texture_edge", "texture_advance_edge"):
        # Include the actual template prefix, not a fixture-created substitute.
        signature = re.search(r"template\s*<class Coordinates>\s*static int "
                              + name + r"\(", source).group()
        result[name] = braced(source, signature)
    result["texture_span_step"] = braced(source, "static int texture_span_step(")
    result["texture_span_pixels"] = braced(source, "static unsigned int texture_span_pixels(")
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
                        for name in ("EdgeScan",))
    texture_class = braced(header, "class Texture {")
    # Actual storage declarations, made public only in this compact fixture.
    fields = []
    for name in ("pixels_", "iWidth_", "iHeight_", "corners_", "cornersAndMids_", "borrowed_"):
        matches = re.findall(r"^\s*((?:unsigned\s+char\s*\*|uint32_t\s+|Vert\s+)"
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
    source = (ROOT / "src/texture.cpp").read_text()
    harness = r'''
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
        // TEXTURE_DIRECT_STORAGE_PRELUDE
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
        struct RECT { int32_t left, top, right, bottom; };
        static unsigned char output[16 * 14], expected[16 * 14], pixels[8 * 8];
        RECT lock_clip;
        unsigned lock_key, lock_stride;
        struct Buffer {
            RECT rect1_;
            uint32_t stride_;
            void *get_data() {
                require(events.empty()); events.push_back(1);
                // The lock publishes current clip, stride and key. All are
                // different from the stale values the fixture starts with.
                rect1_ = lock_clip; stride_ = lock_stride; TextureRasterKey = lock_key;
                return output + 16;
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

            // The span kernel's packed countdown writes one pixel for spans
            // that clipping leaves zero or inverted after admission.
            Vert wide_span[] = {{2,0},{6,0},{6,1},{2,1}};
            start(); lock_clip={5,0,3,2};
            for (auto &uv : texture.corners_) uv={1,2};
            sample(5,0,1,2);
            finish(wide_span, 0);
            start(); lock_clip={4,0,4,2};
            for (auto &uv : texture.corners_) uv={1,2};
            sample(4,0,1,2);
            finish(wide_span, 0);
            // end == clip.left rejects before clipping; only a strict
            // greater-than admits.
            Vert narrow[] = {{1,0},{4,0},{4,1},{1,1}};
            start(); lock_clip={4,0,5,2};
            for (auto &uv : texture.corners_) uv={1,2};
            finish(narrow, 0);
            // A span of 0x10005 runs only its low sixteen pixels.
            start(); lock_clip={0,0,0x10006,12};
            Vert truncated[] = {{1,1},{0x10006,1},{0x10006,2},{1,2}};
            for (auto &uv : texture.corners_) uv={1,2};
            for (int i=0; i<5; ++i) sample(1+i,1,1,2);
            finish(truncated, 0);
        }
    '''.replace("ACTUAL_VERTEX", vertex_declaration()).replace("ACTUAL_RECORDS", records).replace(
        "ACTUAL_FIELDS", "\n".join(fields)).replace("ACTUAL_BODIES", "\n".join(methods.values())).replace(
        "SLOPE_CASES", ",\n".join(cases))
    assert harness.count(PRELUDE_MARKER) == 1
    return harness.replace(PRELUDE_MARKER, direct_storage_prelude(source), 1)


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


def packed_bodies():
    actual = raster_bodies()
    actual["draw_trans"] = braced(
        (ROOT / "src/texture.cpp").read_text(),
        "void Texture::draw_trans(Buffer *buffer, Vert *vertices, int count, Vert *uv) {")
    return actual


def packed_harness(methods, observe_initial=True):
    methods = dict(methods)
    # Observe initial calls while delegating every calculation to the actual
    # helper. The macro is scoped to the unchanged draw body only; the already
    # defined advance helper retains its direct actual edge-helper calls.
    if observe_initial:
        methods["draw_trans"] = r'''
        int initial_edge_calls;
        template<class Coordinates>
        int observe_initial_edge(EdgeScan &edge, int index, const Vert *vertices,
                                 const Coordinates *uv, int count, int bottom) {
            require(events == std::vector<int>({1,2,3}));
            ++initial_edge_calls;
            require(initial_edge_calls <= 2);
            require(edge.f0 == (initial_edge_calls == 1 ? -1 : 1));
            return texture_edge(edge, index, vertices, uv, count, bottom);
        }
        #define texture_edge observe_initial_edge
        ''' + methods["draw_trans"] + "\n#undef texture_edge\n"
    else:
        methods["draw_trans"] = "int initial_edge_calls;\n" + methods["draw_trans"]
    # Reuse declaration extraction and the explicit Buffer/FPU doubles from
    # the color-key fixture. These replacements change fixture scaffolding
    # only, never any extracted product definition.
    common = raster_harness(methods).split("        void start() {", 1)[0]
    common = common.replace("void draw_trans(Buffer *, Vert *, int);",
                            "void draw_trans(Buffer *, Vert *, int, Vert *);")
    common = common.replace("static unsigned char output[16 * 14], expected[16 * 14], pixels[8 * 8];", r'''
        static unsigned char output[16 * 14], expected[16 * 14];
        static unsigned char pixel_storage[65536+64], preserved_pixels[65536+64];
        static unsigned char *pixels = pixel_storage + 32;
    ''')
    common = common.replace("struct Buffer {", "void publish_texture();\n        struct Buffer {")
    common = common.replace("TextureRasterKey = lock_key;",
                            "TextureRasterKey = lock_key; publish_texture();")
    common += "\nconst bool observe_initial = " + ("true" if observe_initial else "false") + ";\n"
    return common + r'''
        unsigned publish_borrowed;
        void publish_texture() {
            texture.pixels_ = pixels;
            texture.borrowed_ = publish_borrowed;
        }
        void start() {
            events.clear(); initial_edge_calls = 0;
            std::memset(output, 0xee, sizeof(output));
            std::memset(expected, 0xee, sizeof(expected));
            std::memset(pixel_storage, 0xa5, sizeof(pixel_storage));
            for (int v=0; v<256; ++v) for (int u=0; u<256; ++u)
                pixels[v*256+u] = static_cast<unsigned char>(u*37+v*71+13);
            std::memcpy(preserved_pixels, pixel_storage, sizeof(pixel_storage));
            texture.pixels_ = pixel_storage; // stale pointer before lock
            texture.borrowed_ = 0;
            texture.iWidth_ = texture.iHeight_ = 256;
            publish_borrowed = 1;
            buffer.rect1_={0,0,0,0}; buffer.stride_=8;
            lock_clip={0,0,12,12}; lock_stride=16;
            TextureRasterKey=255; lock_key=0x120000u | pixels[3*256+2];
        }
        void sample(int x, int y, int u, int v) {
            require(x >= 0 && x < 16 && y >= 0 && y < 12);
            require(u >= 0 && u < 256 && v >= 0 && v < 256);
            // No key filtering: every source palette value is copied.
            expected[16+y*lock_stride+x] = pixels[v*256+u];
        }
        void finish(Vert *vertices, int count, Vert *uv, bool raster=true, int setups=2) {
            std::vector<Vert> saved_vertices, saved_uv;
            if (count > 0) {
                saved_vertices.assign(vertices, vertices+count);
                saved_uv.assign(uv, uv+count);
            }
            texture.draw_trans(&buffer, vertices, count, uv);
            if (observe_initial) require(initial_edge_calls == setups);
            const std::vector<int> active = {1,2,3,4,5,2,3};
            const std::vector<int> inactive = {1,2,3,5,2,3};
            require(events == (raster ? active : inactive));
            require(std::memcmp(output, expected, sizeof(output)) == 0);
            require(std::memcmp(pixel_storage, preserved_pixels, sizeof(pixel_storage)) == 0);
            if (count > 0) {
                require(std::memcmp(vertices, saved_vertices.data(), count*sizeof(Vert)) == 0);
                require(std::memcmp(uv, saved_uv.data(), count*sizeof(Vert)) == 0);
            }
        }
        int main() {
            // Arbitrary signed counts, not the other overload's 4/8 selector.
            Vert triangle[]={{1,1},{5,1},{1,3}};
            Vert triangle_uv[]={{2,3},{2,3},{2,3}};
            start();
            for (int x=1; x<=5; ++x) sample(x,1,2,3);
            for (int x=1; x<=3; ++x) sample(x,2,2,3);
            finish(triangle,3,triangle_uv); // texel equals the current key

            Vert pentagon[]={{1,1},{3,1},{5,1},{5,3},{1,3}};
            Vert pentagon_uv[5];
            for (auto &uv : pentagon_uv) uv={6,7};
            start();
            for (int y=1; y<3; ++y) for (int x=1; x<=5; ++x) sample(x,y,6,7);
            finish(pentagon,5,pentagon_uv);

            Vert perimeter[]={{1,1},{2,1},{3,1},{5,1},{5,2},{5,3},
                              {4,3},{3,3},{2,3},{1,3},{1,2},{1,1}};
            Vert perimeter_uv[12];
            for (auto &uv : perimeter_uv) uv={6,7};
            start();
            for (int y=1; y<3; ++y) for (int x=1; x<=5; ++x) sample(x,y,6,7);
            finish(perimeter,12,perimeter_uv);

            // Opposite fractional X edge slopes: -2/3 delays its first
            // left correction while +2/3 advances the right edge immediately.
            Vert sloped[]={{2,0},{5,0},{7,3},{0,3}};
            Vert sloped_uv[]={{6,7},{6,7},{6,7},{6,7}};
            start();
            const int left[]={2,2,1}, right[]={5,6,7};
            for (int y=0; y<3; ++y) for (int x=left[y]; x<=right[y]; ++x) sample(x,y,6,7);
            finish(sloped,4,sloped_uv);

            // At row1, left=(255.75,3.25), du=.5,dv=.75. These explicit
            // original packed-ADC samples distinguish independent byte wraps
            // from a linear pointer increment carrying U into V.
            Vert rectangle[]={{1,0},{5,0},{5,4},{1,4}};
            Vert fractional_uv[]={{255,3},{257,6},{260,7},{258,4}};
            const int positive_uv[5][2]={{255,3},{0,4},{0,4},{1,5},{1,6}};
            start(); lock_clip={0,1,12,2};
            for (int i=0; i<5; ++i) sample(1+i,1,positive_uv[i][0],positive_uv[i][1]);
            finish(rectangle,4,fractional_uv);
            start(); lock_clip={3,1,4,2};
            sample(3,1,0,4); sample(4,1,1,5); // includes clip.right
            finish(rectangle,4,fractional_uv);

            // Independent fractional borrow in U and V, modulo256.
            Vert short_row[]={{1,1},{5,1},{5,2},{1,2}};
            Vert negative_uv[]={{0,0},{-2,-1},{-2,-1},{0,0}};
            const int negative_samples[5][2]={{0,0},{255,255},{255,255},{254,255},{254,255}};
            start();
            for (int i=0; i<5; ++i) sample(1+i,1,negative_samples[i][0],negative_samples[i][1]);
            finish(short_row,4,negative_uv);

            // Integer steps wrap each component too, including the V byte.
            Vert wrap_uv[]={{254,254},{258,258},{258,258},{254,254}};
            start();
            const int diagonal[]={254,255,0,1,2};
            for (int i=0; i<5; ++i) sample(1+i,1,diagonal[i],diagonal[i]);
            finish(short_row,4,wrap_uv);

            // PC24/chop rounds the width3 slope below65536; the inclusive
            // four samples are U=[0,0,1,2], not the exclusive kernel's three.
            Vert width_three[]={{1,1},{4,1},{4,2},{1,2}};
            Vert width_three_uv[]={{0,0},{3,0},{3,0},{0,0}};
            start();
            sample(1,1,0,0); sample(2,1,0,0); sample(3,1,1,0); sample(4,1,2,0);
            finish(width_three,4,width_three_uv);

            // Signed negative screen/UV coordinates, clipped on all four
            // sides. Constant UV=-1,-2 selects packed column255,row254.
            Vert clipped[]={{-2,-1},{4,-1},{4,3},{-2,3}};
            Vert clipped_uv[]={{-1,-2},{-1,-2},{-1,-2},{-1,-2}};
            start(); lock_clip={0,0,2,2};
            for (int y=0; y<2; ++y) for (int x=0; x<=2; ++x) sample(x,y,255,254);
            finish(clipped,4,clipped_uv);

            // A zero-width clip strictly inside a positive raw span writes
            // one pixel. Merely touching the raw endpoint still rejects.
            start(); lock_clip={3,1,3,2}; sample(3,1,255,255);
            finish(short_row,4,negative_uv);
            start(); lock_clip={5,1,7,2}; finish(short_row,4,negative_uv);
            start(); lock_clip={0,1,1,2}; finish(short_row,4,negative_uv);
            Vert reversed[]={{1,1},{1,3},{5,1}};
            start(); finish(reversed,3,triangle_uv);
            Vert vertical[]={{2,1},{2,3}};
            start(); finish(vertical,2,triangle_uv);

            // Edge setup still precedes the borrowed-zero guard, while the
            // active PC24 policy and pixel loop do not run in this case.
            start(); publish_borrowed=0;
            finish(short_row,4,negative_uv,false,2);
            start(); lock_clip={0,0,12,1}; finish(short_row,4,negative_uv);
            start(); finish(triangle,0,triangle_uv,false,0);
            start(); finish(triangle,-1,triangle_uv,false,0);
            Vert flat[]={{1,1},{3,1},{5,1},{1,1}};
            start(); finish(flat,4,negative_uv,false,0);
        }
    '''


@pytest.mark.parametrize("observe_initial", [False, True])
def test_actual_packed_opaque_raster(tmp_path, observe_initial):
    result = run_host(tmp_path, packed_harness(packed_bodies(), observe_initial))
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("before,after", [
    ("i < count", "i < (count > 4 ? 4 : count)"),
    ("+ 1u;", "+ 0u;"),
    ("*out = pixels[row * 256 + column];",
     "if (pixels[row * 256 + column] != static_cast<unsigned char>(TextureRasterKey)) *out = pixels[row * 256 + column];"),
    ("row * 256 + column", "row * 255 + column"),
    ("column = (column + column_step + (uf >> 16)) & 0xff;",
     "column = (column + column_step) & 0xff;"),
    ("column = (column + column_step + (uf >> 16)) & 0xff;",
     "row = (row + ((column + column_step + (uf >> 16)) >> 8)) & 0xff; column = (column + column_step + (uf >> 16)) & 0xff;"),
    ("row = (row + row_step + (vf >> 16)) & 0xff;",
     "row = (row + row_step) & 0xff;"),
    ("row = (row + row_step + (vf >> 16)) & 0xff;",
     "row = (row + row_step + (uf >> 16)) & 0xff;"),
    ("const unsigned int column_step = static_cast<unsigned char>(du >> 16);",
     "const unsigned int column_step = 0;"),
    ("if (y >= clip.top && end > clip.left && x < clip.right && end > x)",
     "if (!done && y >= clip.top && end > clip.left && x < clip.right && end > x)"),
    ("end > clip.left && x < clip.right", "end >= clip.left && x <= clip.right"),
    ("if (min_y < max_y && texture_edge", "if (borrowed_ != 0 && min_y < max_y && texture_edge"),
])
def test_compiling_packed_mutations_are_rejected(tmp_path, before, after):
    actual = packed_bodies()
    assert actual["draw_trans"].count(before) == 1
    actual["draw_trans"] = actual["draw_trans"].replace(before, after)
    result = run_host(tmp_path, packed_harness(actual))
    assert result.returncode == 42, result.stderr


@pytest.mark.parametrize("declaration", [
    "const unsigned char *pixels = pixels_;",
    "const int stride = static_cast<int>(buffer->stride_);",
    "const RECT clip = buffer->rect1_;",
])
def test_packed_lock_publication_cannot_be_captured_early(tmp_path, declaration):
    actual = packed_bodies()
    method = actual["draw_trans"]
    assert method.count(declaration) == 1
    method = method.replace(declaration, "")
    brace = method.index("{") + 1
    actual["draw_trans"] = method[:brace] + "\n" + declaration + method[brace:]
    result = run_host(tmp_path, packed_harness(actual))
    assert result.returncode == 42, result.stderr


def keyed_bodies():
    actual = raster_bodies()
    actual["draw_color_key"] = braced(
        (ROOT / "src/texture.cpp").read_text(),
        "void Texture::draw_color_key(Buffer *buffer, Vert *vertices, int count, Vert *uv) {")
    return actual


def alternating_bodies():
    actual = keyed_bodies()
    actual["draw_color_key5"] = braced(
        (ROOT / "src/texture.cpp").read_text(),
        "void Texture::draw_color_key(Buffer *buffer, Vert *vertices, int count, Vert *uv, int color) {")
    return actual


KEYED_MAIN = r'''
    int main() {
        // Arbitrary counts, not the other overload's 4/8 selector, with the
        // texture sampled through width-eight rows (v*8+u). The keyed span
        // is exclusive: end-left pixels starting at the left edge.
        Vert triangle[]={{1,1},{5,1},{1,3}};
        Vert constant_uv[]={{2,3},{2,3},{2,3}};
        start();
        for (int x=1; x<5; ++x) sample(x,1,2,3);
        for (int x=1; x<3; ++x) sample(x,2,2,3);
        finish(triangle,3,constant_uv); // texel equals the current key

        Vert pentagon[]={{1,1},{3,1},{5,1},{5,3},{1,3}};
        Vert pentagon_uv[5];
        for (auto &uv : pentagon_uv) uv={6,7};
        start();
        for (int y=1; y<3; ++y) for (int x=1; x<5; ++x) sample(x,y,6,7);
        finish(pentagon,5,pentagon_uv);

        Vert perimeter[]={{1,1},{2,1},{3,1},{5,1},{5,2},{5,3},
                          {4,3},{3,3},{2,3},{1,3},{1,2},{1,1}};
        Vert perimeter_uv[12];
        for (auto &uv : perimeter_uv) uv={6,7};
        start();
        for (int y=1; y<3; ++y) for (int x=1; x<5; ++x) sample(x,y,6,7);
        finish(perimeter,12,perimeter_uv);

        // Explicit UVs can walk backwards along a scanline.
        Vert reverse[]={{1,1},{5,1},{5,2},{1,2}};
        Vert reverse_uv[]={{4,4},{0,0},{0,0},{4,4}};
        start();
        for (int i=0; i<4; ++i) sample(1+i,1,4-i,4-i);
        finish(reverse,4,reverse_uv);

        // Clipping with negative coordinates: the skip advances U by two.
        Vert clipped[]={{-2,-1},{4,-1},{4,3},{-2,3}};
        Vert clipped_uv[]={{0,0},{6,0},{6,4},{0,4}};
        start(); lock_clip={0,0,2,2};
        sample(0,0,1,1); sample(1,0,2,1); sample(0,1,1,2); sample(1,1,2,2);
        finish(clipped,4,clipped_uv);

        // The final row is still drawn after the left edge runs out.
        Vert sloped[]={{2,0},{6,0},{4,4},{0,4}};
        start();
        const int left[]={2,2,1,1};
        for (int y=0; y<4; ++y) for (int x=0; x<4; ++x) sample(left[y]+x,y,6,7);
        finish(sloped,4,perimeter_uv);

        // A constant texel equal to the key leaves the span empty.
        Vert hole_uv[]={{1,0},{1,0},{1,0},{1,0}};
        start(); finish(reverse,4,hole_uv);

        // Admitted spans that clipping leaves zero, inverted or touching
        // still write exactly one pixel (or none before admission).
        Vert spanned[]={{2,0},{6,0},{6,1},{2,1}};
        Vert span_uv[]={{1,2},{1,2},{1,2},{1,2}};
        start(); lock_clip={5,0,3,2};
        sample(5,0,1,2);
        finish(spanned,4,span_uv);
        start(); lock_clip={4,0,4,2};
        sample(4,0,1,2);
        finish(spanned,4,span_uv);
        Vert narrow[]={{1,0},{4,0},{4,1},{1,1}};
        start(); lock_clip={4,0,5,2};
        finish(narrow,4,span_uv);

        // A raw span of 0x10005 runs only its low sixteen pixels.
        start(); lock_clip={0,0,0x10006,12};
        Vert truncated[]={{1,1},{0x10006,1},{0x10006,2},{1,2}};
        for (int i=0; i<5; ++i) sample(1+i,1,1,2);
        finish(truncated,4,span_uv);

        // The entry row test rejects a polygon already below the clip.
        start(); lock_clip={0,0,12,1}; finish(reverse,4,span_uv);
        // Counts at or below zero never reach the edge helpers.
        start(); finish(triangle,0,constant_uv,false);
        start(); finish(triangle,-1,constant_uv,false);
        start(); finish(pentagon,-7,pentagon_uv,false);
        Vert flat[]={{1,1},{3,1},{5,1},{1,1}};
        start(); finish(flat,4,span_uv,false);
        Vert point[]={{0,0},{0,0},{0,0},{0,0}};
        start(); finish(point,4,span_uv,false);
    }
'''

KEYED_WIDE_MAIN = r'''
    int main() {
        Vert span_uv[4]={{6,7},{6,7},{6,7},{6,7}};
        // 32769 pixels: the largest span the countdown reproduces whole.
        start(); lock_clip={0,0,74000,3}; lock_stride=74000;
        Vert full[]={{1,0},{32770,0},{32770,1},{1,1}};
        for (int x=1; x<=32769; ++x) sample(x,0,6,7);
        finish(full,4,span_uv);
        // One pixel more collapses the whole span to a single write.
        start(); lock_clip={0,0,74000,3}; lock_stride=74000;
        Vert collapsed[]={{1,0},{32771,0},{32771,1},{1,1}};
        sample(1,0,6,7);
        finish(collapsed,4,span_uv);
        start(); lock_clip={0,0,74000,3}; lock_stride=74000;
        Vert far_over[]={{1,0},{0x9000,0},{0x9000,1},{1,1}};
        sample(1,0,6,7);
        finish(far_over,4,span_uv);
        // 0x10005 wraps to five pixels again, 0x10000 to one.
        start(); lock_clip={0,0,74000,3}; lock_stride=74000;
        Vert wrapped[]={{1,0},{0x10006,0},{0x10006,1},{1,1}};
        for (int i=0; i<5; ++i) sample(1+i,0,6,7);
        finish(wrapped,4,span_uv);
        start(); lock_clip={0,0,74000,3}; lock_stride=74000;
        Vert wrapped_zero[]={{1,0},{0x10001,0},{0x10001,1},{1,1}};
        sample(1,0,6,7);
        finish(wrapped_zero,4,span_uv);
        // A clip that inverts by exactly 0x8000 still runs 0x8000 pixels.
        start(); lock_clip={40000,0,7232,2}; lock_stride=74000;
        Vert inverted[]={{1,0},{50000,0},{50000,1},{1,1}};
        for (int x=40000; x<=72767; ++x) sample(x,0,6,7);
        finish(inverted,4,span_uv);
    }
'''


def keyed_harness(methods, wide=False, main_body=KEYED_MAIN):
    """Call the explicit-UV color-key overload with caller-owned counts.

    The normal fixture reuses the color-key doubles and its width-eight
    texture; the wide variant hosts spans whose packed countdown's high word
    crosses 0x8000, with rows sized so even mutant over-writes stay inside
    the allocated arrays and fail by comparison, not by smashing memory.
    """
    methods = dict(methods)
    common = raster_harness(methods).split("        void start() {", 1)[0]
    common = common.replace(
        "void draw_trans(Buffer *, Vert *, int);",
        "void draw_trans(Buffer *, Vert *, int);\n"
        "            void draw_color_key(Buffer *, Vert *, int, Vert *);")
    if wide:
        common = common.replace(
            "static unsigned char output[16 * 14], expected[16 * 14], pixels[8 * 8];",
            "static unsigned char output[74000 * 3], expected[74000 * 3], pixels[8 * 8];")
        common = common.replace("return output + 16;", "return output + 74000;")
    return common + r'''
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
                expected[BASE+y*lock_stride+x] = value;
        }
        void finish(Vert *vertices, int count, Vert *uv, bool raster=true) {
            texture.draw_color_key(&buffer, vertices, count, uv);
            const std::vector<int> active = {1,2,3,4,5,2,3};
            const std::vector<int> inactive = {1,2,3,5,2,3};
            require(events == (raster ? active : inactive));
            require(std::memcmp(output, expected, sizeof(output)) == 0);
        }
    '''.replace("BASE", "74000" if wide else "16") + main_body


def test_actual_explicit_uv_color_key_raster(tmp_path):
    result = run_host(tmp_path, keyed_harness(keyed_bodies()))
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("mutation", [None, "missing", "before_lock", "wrong_width"])
@pytest.mark.parametrize("which", ["explicit", "embedded", "keyed", "alternating"])
def test_shared_pitch_publication_and_callback_epoch(tmp_path, mutation, which):
    """Four original entry publications, including no-pixel paths.

    A lock callback changes the receiver width. Release observes the published
    value and replaces it itself; publishing before lock, late after release,
    or deriving pitch from stride must not pass. Service callbacks are doubles.
    """
    source = (ROOT / "src/texture.cpp").read_text()
    signatures = {
        "explicit": "void Texture::draw_trans(Buffer *buffer, Vert *vertices, int count, Vert *uv)",
        "embedded": "void Texture::draw_trans(Buffer *buffer, Vert *vertices, int use_eight_vertices)",
        "keyed": "void Texture::draw_color_key(Buffer *buffer, Vert *vertices, int count, Vert *uv)",
        "alternating": "void Texture::draw_color_key(Buffer *buffer, Vert *vertices, int count, Vert *uv, int color)",
    }
    bodies = raster_bodies()
    del bodies["draw_trans"]
    publication = "TextureRowPitchScratch = static_cast<int>(iWidth_);"
    for name, signature in signatures.items():
        body = braced(source, signature + " {")
        assert body.count(publication) == 1
        if name == which and mutation:
            if mutation == "missing":
                body = body.replace(publication, "")
            elif mutation == "wrong_width":
                body = body.replace(publication, "TextureRowPitchScratch = static_cast<int>(buffer->stride_);")
            else:
                body = body.replace(publication, "")
                brace = body.index("{") + 1
                body = body[:brace] + "\n" + publication + body[brace:]
        bodies[name] = body
    fixture = raster_harness(bodies).split("        void start() {", 1)[0]
    fixture = fixture.replace("void draw_trans(Buffer *, Vert *, int);", """
            void draw_trans(Buffer *, Vert *, int);
            void draw_trans(Buffer *, Vert *, int, Vert *);
            void draw_color_key(Buffer *, Vert *, int, Vert *);
            void draw_color_key(Buffer *, Vert *, int, Vert *, int);
    """)
    fixture = fixture.replace("struct Buffer {", "void publish_width();\n        struct Buffer {")
    fixture = fixture.replace("require(events.empty()); events.push_back(1);",
                              "require(events.empty()); require(TextureRowPitchScratch == -19); publish_width(); events.push_back(1);")
    fixture = fixture.replace("events.push_back(5);",
                              "require(TextureRowPitchScratch == 37); TextureRowPitchScratch = -99; events.push_back(5);")
    calls = {
        "explicit": "texture.draw_trans(&buffer, vertices, 4, uv);",
        "embedded": "texture.draw_trans(&buffer, vertices, 0);",
        "keyed": "texture.draw_color_key(&buffer, vertices, 4, uv);",
        "alternating": "texture.draw_color_key(&buffer, vertices, 4, uv, 7);",
    }
    fixture += """
        void publish_width() { texture.iWidth_ = 37; }
        int main() {
            texture.iWidth_ = 11; texture.iHeight_ = 8; texture.borrowed_ = 1;
            texture.pixels_ = pixels; TextureRowPitchScratch = -19;
            buffer.stride_ = 6; lock_stride = 16; lock_clip = {0,0,12,12};
            Vert vertices[4] = {{1,1},{2,1},{3,1},{4,1}};
            Vert uv[4] = {};
            CALL
            require(TextureRowPitchScratch == -99);
            require(events == std::vector<int>({1,2,3,5,2,3}));
        }
    """.replace("CALL", calls[which])
    result = run_host(tmp_path, fixture)
    assert result.returncode == (42 if mutation else 0), result.stderr


def test_actual_explicit_uv_color_key_wide_boundaries(tmp_path):
    result = run_host(tmp_path, keyed_harness(keyed_bodies(), wide=True,
                                              main_body=KEYED_WIDE_MAIN))
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("method,before,after", [
    ("draw_color_key", "if (texel != key) *out = texel;", "*out = texel;"),
    ("draw_color_key", "texture_span_pixels(texture_difference(end, x))",
     "texture_span_pixels(texture_difference(x, end))"),
    ("draw_color_key", "if (min_y < max_y && texture_edge", "if (4 <= count && texture_edge"),
    ("draw_color_key", "if (vertices[i].y > max_y) { max_y = vertices[i].y; bottom = i; }",
     "if (vertices[i].y > max_y) { max_y = vertices[i].y; bottom = 0; }"),
    ("draw_color_key", "const unsigned char key = static_cast<unsigned char>(TextureRasterKey);",
     "const unsigned char key = 0;"),
])
def test_compiling_keyed_mutations_are_rejected(tmp_path, method, before, after):
    actual = keyed_bodies()
    assert actual[method].count(before) == 1
    actual[method] = actual[method].replace(before, after)
    result = run_host(tmp_path, keyed_harness(actual))
    assert result.returncode == 42, result.stderr


@pytest.mark.parametrize("before,after", [
    ("return pixels - 1u < 0x8001u ? pixels : 1u;", "return pixels ? pixels : 1u;"),
    ("return pixels - 1u < 0x8001u ? pixels : 1u;", "return pixels < 0x8001u ? pixels : 1u;"),
    ("return pixels - 1u < 0x8001u ? pixels : 1u;", "return 1u;"),
    ("static_cast<unsigned int>(difference) & 0xffff",
     "static_cast<unsigned int>(difference)"),
])
def test_compiling_countdown_mutations_are_rejected(tmp_path, before, after):
    actual = keyed_bodies()
    assert actual["texture_span_pixels"].count(before) == 1
    actual["texture_span_pixels"] = actual["texture_span_pixels"].replace(before, after)
    result = run_host(tmp_path, keyed_harness(actual, wide=True, main_body=KEYED_WIDE_MAIN))
    assert result.returncode == 42, result.stderr


def span_pixels_oracle(difference):
    n16 = difference & 0xFFFF
    return n16 if 1 <= n16 <= 0x8001 else 1


def test_actual_span_pixel_countdown_boundaries(tmp_path):
    body = keyed_bodies()["texture_span_pixels"]
    cases = [0, 1, 2, 3, 4, 5, 0x7ffe, 0x7fff, 0x8000, 0x8001, 0x8002, 0x8003,
             0xfffe, 0xffff, 0x10000, 0x10001, 0x10004, 0x10005, 0x10006,
             0x18000, 0x18001, 0x18002, 0x7fffffff,
             -1, -2, -3, -0x7fff, -0x8000, -0x8001, -0x8002, -0xffff,
             -0x10005, -0x7fffffff, -2147483647 - 1]
    table = ",\n".join("{%d, %du}" % (value, span_pixels_oracle(value)) for value in cases)
    source = r'''
        #include <cstdio>
        #include <cstdlib>
        #define require(c) do { if (!(c)) { \
            std::fprintf(stderr, "line %d: %s\n", __LINE__, #c); \
            std::exit(42); } } while (0)
        ACTUAL_BODY
        const int cases[][2] = { CASES };
        int main() {
            for (const auto &c : cases) require(texture_span_pixels(c[0]) == c[1]);
        }
    '''.replace("ACTUAL_BODY", body).replace("CASES", table)
    result = run_host(tmp_path, source)
    assert result.returncode == 0, result.stderr


ALTERNATING_MAIN = r'''
    int main() {
        const int color = 0x1F5;
        // Negative first row: parity comes from min_y, not from zero.
        Vert raised[]={{1,-3},{5,-3},{5,0},{1,0}};
        Vert constant_uv[4]={{2,4},{2,4},{2,4},{2,4}};
        start();
        for (int y=-3; y<0; ++y) for (int x=1; x<5; ++x) sample(x,y,2,4,color);
        finish(raised,4,constant_uv,color);

        // The same shape from row zero alternates the other way round.
        Vert ground[]={{1,0},{5,0},{5,3},{1,3}};
        start();
        for (int y=0; y<3; ++y) for (int x=1; x<5; ++x) sample(x,y,2,4,-2);
        finish(ground,4,constant_uv,-2);

        // Other negative colors recolor; only exactly -1 forwards.
        start();
        for (int y=0; y<3; ++y) for (int x=1; x<5; ++x) sample(x,y,2,4,(int)0xFFFFFF00u);
        finish(ground,4,constant_uv,(int)0xFFFFFF00u);

        // Key holes survive on copy rows and recolor rows alike.
        start(); lock_key = 0x12000Bu;
        Vert walk[]={{1,0},{5,0},{5,1},{1,1}};
        Vert walk_uv[]={{1,1},{5,1},{5,2},{1,2}};
        for (int x=1; x<5; ++x) sample(x,0,x,1,color);
        finish(walk,4,walk_uv,color);

        // Clipped rows keep the absolute parity of the screen row.
        start(); lock_clip={0,0,12,1};
        for (int x=1; x<5; ++x) sample(x,0,2,4,color);
        finish(ground,4,constant_uv,color);
        start(); lock_clip={0,1,12,2};
        for (int x=1; x<5; ++x) sample(x,1,2,4,color);
        finish(ground,4,constant_uv,color);

        // Degenerate inputs recolor nothing at all.
        Vert flat[]={{1,1},{3,1},{5,1},{1,1}};
        start(); finish(flat,4,constant_uv,color,false);
        start(); finish(ground,0,constant_uv,color,false);
        start(); finish(ground,-3,constant_uv,color,false);
    }
'''


def alternating_harness(methods, forward_observer=False, main_body=ALTERNATING_MAIN):
    methods = dict(methods)
    observer = ""
    if forward_observer:
        del methods["draw_color_key"]
        observer = r'''
        void Texture::draw_color_key(Buffer *b, Vert *v, int c, Vert *u) {
            ++forward_calls; forwarded_buffer = b; forwarded_verts = v;
            forwarded_count = c; forwarded_uv = u;
            require(events.empty());
        }
        '''
    common = raster_harness(methods).split("        void start() {", 1)[0]
    common = common.replace(
        "void draw_trans(Buffer *, Vert *, int);",
        "void draw_trans(Buffer *, Vert *, int);\n"
        "            void draw_color_key(Buffer *, Vert *, int, Vert *);\n"
        "            void draw_color_key(Buffer *, Vert *, int, Vert *, int);")
    common = common.replace(
        "static unsigned char output[16 * 14], expected[16 * 14], pixels[8 * 8];",
        "static unsigned char output[16 * 16], expected[16 * 16], pixels[8 * 8];")
    common = common.replace("return output + 16;", "return output + 8 * 16;")
    return common + r'''
        int forward_calls; Buffer *forwarded_buffer; Vert *forwarded_verts;
        int forwarded_count; Vert *forwarded_uv;
    ''' + observer + r'''
        void start() {
            events.clear(); forward_calls = 0; forwarded_buffer = 0;
            forwarded_verts = 0; forwarded_count = -9; forwarded_uv = 0;
            std::memset(output, 0xee, sizeof(output));
            std::memset(expected, 0xee, sizeof(expected));
            for (unsigned i=0; i<sizeof(pixels); ++i) pixels[i] = i + 1;
            texture.pixels_ = pixels; texture.iWidth_ = texture.iHeight_ = 8;
            texture.borrowed_ = 1;
            buffer.rect1_ = {0,0,0,0}; buffer.stride_ = 8;
            lock_clip = {0,-6,12,8}; lock_stride = 16;
            TextureRasterKey = 255; lock_key = 0x1200F5u;
            for (auto &uv : texture.corners_) uv = {0,0};
            for (auto &uv : texture.cornersAndMids_) uv = {7,7};
        }
        // Absolute screen row y is odd when it copies, even when recoloring.
        void sample(int x, int y, int u, int v, int color) {
            const unsigned char value = pixels[v*8+u];
            if (value == static_cast<unsigned char>(lock_key)) return;
            expected[128 + y*lock_stride + x] = ((y & 1) != 0) ? value : (unsigned char)color;
        }
        void finish(Vert *vertices, int count, Vert *uv, int color, bool raster=true) {
            texture.draw_color_key(&buffer, vertices, count, uv, color);
            const std::vector<int> active = {1,2,3,4,5,2,3};
            const std::vector<int> inactive = {1,2,3,5,2,3};
            require(events == (raster ? active : inactive));
            require(std::memcmp(output, expected, sizeof(output)) == 0);
        }
    ''' + main_body


ALTERNATING_FORWARD_MAIN = r'''
    int main() {
        Vert triangle[]={{1,1},{5,1},{1,3}};
        Vert triangle_uv[]={{2,3},{2,3},{2,3}};
        // Exactly -1 forwards before this overload locks or resets: no
        // buffer events, no pixels, and every argument passes through.
        start();
        texture.draw_color_key(&buffer, triangle, 3, triangle_uv, -1);
        require(forward_calls == 1 && events.empty());
        require(forwarded_buffer == &buffer && forwarded_verts == triangle);
        require(forwarded_count == 3 && forwarded_uv == triangle_uv);
        require(std::memcmp(output, expected, sizeof(output)) == 0);
    }
'''


@pytest.mark.parametrize("main_body,observer", [
    (ALTERNATING_MAIN, False),
    (ALTERNATING_FORWARD_MAIN, True),
])
def test_actual_alternating_row_color_key_raster(tmp_path, main_body, observer):
    result = run_host(tmp_path, alternating_harness(alternating_bodies(), observer, main_body))
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("before,after", [
    ("if (color == -1) {", "if (color != 0) {"),
    ("int parity = min_y & 1;", "int parity = 0;"),
    ("++parity;", ";"),
    ("*out = (parity & 1) != 0 ? texel : alternate;",
     "*out = texel;"),
    ("if (texel != key) *out = (parity & 1) != 0 ? texel : alternate;",
     "*out = (parity & 1) != 0 ? texel : alternate;"),
    ("const unsigned char alternate = static_cast<unsigned char>(color);",
     "const unsigned char alternate = static_cast<unsigned char>(color >> 8);"),
])
def test_compiling_alternating_mutations_are_rejected(tmp_path, before, after):
    actual = alternating_bodies()
    assert actual["draw_color_key5"].count(before) == 1
    actual["draw_color_key5"] = actual["draw_color_key5"].replace(before, after)
    result = run_host(tmp_path, alternating_harness(actual))
    assert result.returncode == 42, result.stderr


def test_alternating_forward_runs_before_lock(tmp_path):
    actual = alternating_bodies()
    real = actual["draw_color_key5"]
    brace = real.index("{") + 1
    # A body that locks first publishes an event before forwarding.
    actual["draw_color_key5"] = (real[:brace]
        + "\n            buffer->get_data();\n"
        + "            if (color == -1) { draw_color_key(buffer, vertices, count, uv); return; }"
        + real[brace:])
    result = run_host(tmp_path,
                      alternating_harness(actual, True, ALTERNATING_FORWARD_MAIN))
    assert result.returncode == 42, result.stderr


@pytest.mark.parametrize("declaration", [
    "const unsigned char key = static_cast<unsigned char>(TextureRasterKey);",
    "const int stride = static_cast<int>(buffer->stride_);",
    "const RECT clip = buffer->rect1_;",
])
def test_keyed_lock_publication_cannot_be_captured_early(tmp_path, declaration):
    actual = keyed_bodies()
    method = actual["draw_color_key"]
    assert method.count(declaration) == 1
    method = method.replace(declaration, "")
    brace = method.index("{") + 1
    actual["draw_color_key"] = method[:brace] + "\n" + declaration + method[brace:]
    result = run_host(tmp_path, keyed_harness(actual))
    assert result.returncode == 42, result.stderr


def shaded_rows_bodies():
    """Complete draw2 plus real helpers; its sentinel callee is isolated."""
    source = (ROOT / "src/texture.cpp").read_text()
    methods = raster_bodies()
    del methods["draw_trans"]
    for name in ("texture_gouraud_edge", "texture_advance_gouraud_edge"):
        methods[name] = braced(source, "static int " + name + "(")
    methods["texture_light_step"] = braced(source, "static unsigned short texture_light_step(")
    methods["draw2"] = braced(source, "void Texture::draw2(")
    return methods


def shaded_rows_harness(methods):
    common = raster_harness(methods).split("        void start() {", 1)[0]
    common = common.replace("struct Texture {", "int forward_calls;\nstruct Texture {")
    common = common.replace(
        "void draw_trans(Buffer *, Vert *, int);",
        "void draw2(Buffer *, Vert *, int *, int, Vert *, int, int);\n"
        "void draw_below_cutoff(Buffer *b, Vert *v, int *z, int n, Vert *uv, int cutoff) {\n"
        "    require(events.empty() && b == &buffer && v && z && uv);\n"
        "    require(n == 4 && cutoff == 17); ++forward_calls;\n}")
    common = common.replace("return output + 16;", "return output + 7 * 16;")
    cases = []
    for width in (1, 2, 3, 7, 31, 255, 32767):
        for delta in (0, 1, -1, 65536, -65536, 8388607, 8388608,
                      -8388608, -8388609, 2147483647, -2147483648):
            step = x87_span_step_expected(delta, width)
            magnitude = abs(step) // 256
            value = -magnitude if step < 0 else magnitude
            expected = value & 0xffff if -32768 <= value <= 32767 else 0x8000
            cases.append("{%d, %d, %d}" % (delta, width, expected))
    return common + r'''
        void start() {
            events.clear(); forward_calls = 0;
            std::memset(output, 0xee, sizeof(output));
            std::memset(expected, 0xee, sizeof(expected));
            for (unsigned i=0; i<sizeof(pixels); ++i) pixels[i] = 240 + i;
            texture.pixels_ = pixels; texture.iWidth_ = texture.iHeight_ = 8;
            texture.borrowed_ = 1;
            buffer.rect1_ = {0,0,0,0}; buffer.stride_ = 8;
            lock_clip = {0,-6,12,6}; lock_stride = 16;
        }
        void finish(Vert *vertices, int *z, int count, Vert *uv,
                    int cutoff, int color, bool active=true) {
            texture.draw2(&buffer, vertices, z, count, uv, cutoff, color);
            const std::vector<int> raster = {1,2,3,4,5,2,3};
            const std::vector<int> skipped = {1,2,3,5,2,3};
            require(events == (active ? raster : skipped));
            require(std::memcmp(output, expected, sizeof(output)) == 0);
        }
        int main() {
            const int slopes[][3] = { LIGHT_CASES };
            for (const auto &c : slopes)
                require(texture_light_step(c[0], c[1]) == c[2]);
            Vert vertices[] = {{1,1},{5,1},{5,5},{1,5}};
            Vert uv[] = {{0,0},{0,0},{0,0},{0,0}};
            int z[4] = {};
            start();
            texture.draw2(&buffer, vertices, z, 4, uv, 17, -1);
            require(forward_calls == 1 && events.empty());
            require(std::memcmp(output, expected, sizeof(output)) == 0);

            // Full-int polygon admission differs from the per-pixel signed
            // byte comparison. Odd rows shade; even rows fill regardless.
            const int depths[] = {-129,-128,-1,0,1,126,127,128,255,256};
            const int cutoffs[] = {-128,-1,0,1,127,128,129,256,257};
            for (int depth : depths) for (int cutoff : cutoffs)
            for (int top : {-3, 0, 1}) {
                start();
                vertices[0].y=vertices[1].y=top;
                vertices[2].y=vertices[3].y=top+4;
                for (int &value : z) value=depth;
                const bool active = depth < cutoff;
                if (active) for (int y=top; y<top+4; ++y)
                for (int x=1; x<5; ++x) {
                    if (!(y & 1)) expected[112+y*16+x]=0x43;
                    else if ((int8_t)depth < (int8_t)cutoff)
                        expected[112+y*16+x]=(unsigned char)(pixels[0]+(int8_t)depth);
                }
                finish(vertices,z,4,uv,cutoff,0x143,active);
            }

            // Four-row edge slopes: biased V=.5,.5+.875,...; horizontal
            // U and light advance by one. This is an independent rectangle
            // oracle, not a second implementation of the scan converter.
            vertices[0].y=vertices[1].y=1;
            vertices[2].y=vertices[3].y=5;
            Vert mapped[]={{0,0},{4,0},{4,4},{0,4}};
            int lit[]={0,4,8,4};
            for (int left : {0,3}) {
                start(); lock_clip.left=left;
                for (int y=1; y<5; ++y) for (int x=1; x<5; ++x) {
                    if (x < left) continue;
                    if (!(y&1)) expected[112+y*16+x]=0x91;
                    else if (y+x-2 < 3)
                        expected[112+y*16+x]=(unsigned char)(pixels[(y-1)*8+x-1]+y+x-2);
                }
                finish(vertices,lit,4,mapped,3,0x91);
            }

            // Real Gouraud setup keeps the half-texel at the start but
            // subtracts it when deriving the endpoint delta.
            EdgeScan edge={}; edge.f0=-1;
            require(texture_gouraud_edge(edge,0,vertices,mapped,lit,4,2)==1);
            require(edge.accumA==32768 && edge.accumB==32768);
            require(edge.deltaA==-8192 && edge.deltaB==57344);
            require(edge.accumD==0 && edge.deltaD==65536);
            require(texture_advance_gouraud_edge(edge,vertices,mapped,lit,4,2)==1);
            require(edge.accumA==24576 && edge.accumB==90112 && edge.accumD==65536);
            start();
            finish(vertices,lit,0,mapped,3,0x91,false);
            return 0;
        }
    '''.replace("LIGHT_CASES", ",".join(cases))


def test_actual_shaded_alternating_rows(tmp_path):
    result = run_host(tmp_path, shaded_rows_harness(shaded_rows_bodies()))
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("method,before,after", [
    ("draw2", "fillColor == -1", "cutoff == -1"),
    ("draw2", "zvals[in_front] < cutoff", "zvals[in_front] <= cutoff"),
    ("draw2", "(y & 1) == 0", "(y & 1) != 0"),
    ("draw2", "texel + shade", "texel"),
    ("draw2", "shade < static_cast<signed char>(cutoff)",
     "shade < cutoff"),
    ("texture_gouraud_edge", "+ 0x8000u", "+ 0u"),
    ("texture_advance_gouraud_edge", "edge.accumD) + edge.deltaD", "edge.accumD) + 0"),
    ("texture_light_step", "return 0x8000u", "return 0u"),
])
def test_shaded_row_damage_is_observed(tmp_path, method, before, after):
    methods = shaded_rows_bodies()
    assert before in methods[method]
    methods[method] = methods[method].replace(before, after)
    result = run_host(tmp_path, shaded_rows_harness(methods))
    assert result.returncode == 42, result.stderr


def cutoff_bodies():
    methods = shaded_rows_bodies()
    del methods["draw2"]
    methods["draw_below_cutoff"] = braced(
        (ROOT / "src/texture.cpp").read_text(), "void Texture::draw_below_cutoff(")
    return methods


def cutoff_harness(methods):
    common = raster_harness(methods).split("        void start() {", 1)[0]
    common = common.replace(
        "void draw_trans(Buffer *, Vert *, int);",
        "void draw_below_cutoff(Buffer *, Vert *, int *, int, Vert *, int);")
    common = common.replace("unsigned lock_key, lock_stride;",
                            "unsigned lock_key, lock_stride; bool null_lock;")
    common = common.replace("return output + 16;",
                            "return null_lock ? nullptr : output + 112;")
    return common + r'''
        void start() {
            events.clear(); null_lock=false;
            std::memset(output,0xee,sizeof(output));
            std::memset(expected,0xee,sizeof(expected));
            for (unsigned i=0; i<sizeof(pixels); ++i) pixels[i]=240+i;
            texture.pixels_=pixels; texture.iWidth_=texture.iHeight_=8;
            buffer.rect1_={0,0,0,0}; buffer.stride_=8;
            lock_clip={0,-6,12,6}; lock_stride=16;
            TextureRasterKey=240; lock_key=240;
        }
        void finish(Vert *v,int *z,int n,Vert *uv,int cutoff,bool raster=true) {
            texture.draw_below_cutoff(&buffer,v,z,n,uv,cutoff);
            const std::vector<int> active={1,2,3,4,5,2,3};
            const std::vector<int> inactive={1,2,3,5,2,3};
            require(events==(raster ? active : inactive));
            require(std::memcmp(output,expected,sizeof(output))==0);
        }
        int floor_div(int numerator,int denominator) {
            return numerator>=0 ? numerator/denominator
                                : -((-numerator+denominator-1)/denominator);
        }
        int main() {
            Vert v[]={{1,1},{5,1},{5,5},{1,5}};
            Vert uv[]={{0,0},{0,0},{0,0},{0,0}};
            int z[4]={};
            // Original 61ACC9 admits by signed full integers, independently
            // of the signed CH/AH comparison at 61B16C. No row parity, key,
            // fill color, light clamp or special cutoff sentinel is involved.
            for (int depth : {-129,-128,-1,0,1,126,127,128,255,256})
            for (int cutoff : {-128,-1,0,1,127,128,129,256,257})
            for (int top : {-3,0,1}) {
                start();
                v[0].y=v[1].y=top; v[2].y=v[3].y=top+4;
                for (int &value : z) value=depth;
                const bool admitted=depth<cutoff;
                if (admitted && (int8_t)depth<(int8_t)cutoff)
                    for (int y=top;y<top+4;++y) for (int x=1;x<5;++x)
                        expected[112+y*lock_stride+x]=(unsigned char)(pixels[0]+(int8_t)depth);
                finish(v,z,4,uv,cutoff,admitted);
            }
            v[0].y=v[1].y=1; v[2].y=v[3].y=5;
            // Independent rational planes, in eighth-texel/half-light units.
            // Both increasing and decreasing U,V and light have fractional
            // carries. Power-of-two spans make these exact PC24 anchors.
            // Biased edge starts approach the un-biased endpoint, hence
            // vertical eighth-texel slopes +3 and -5, not +4 and -4.
            // The original zero-extends the WORD light slope for clipping.
            // Signed extension differs by whole 65536-unit light cycles and
            // is pixel-equivalent after truncation here; these controls test
            // the resulting negative-slope pixels, not that representation.
            for (bool descending : {false,true})
            for (int pitch : {12,16}) for (int left : {0,3,6})
            for (int right : {4,12}) for (int top : {-6,2,5}) {
                start(); lock_stride=pitch; lock_clip={left,top,right,6};
                Vert rising[]={{1,1},{3,4},{5,6},{3,3}};
                Vert falling[]={{5,6},{3,3},{1,1},{3,4}};
                int bright[]={-3,3,7,1}; int dark[]={7,-3,-7,3};
                for (int row=0;row<4;++row) for (int col=0;col<4;++col) {
                    const int x=col+1,y=row+1;
                    if (x<left || x>=right || y<top) continue;
                    const int u=descending ? (44-5*row-4*col)/8 : (12+3*row+4*col)/8;
                    const int w=descending ? (52-5*row-6*col)/8 : (12+3*row+6*col)/8;
                    const int shade=floor_div(descending ? 14-2*row-5*col
                                                         : -6+2*row+3*col,2);
                    if (shade<4) expected[112+y*pitch+x]=(unsigned char)(pixels[w*8+u]+shade);
                }
                finish(v,descending ? dark : bright,4,descending ? falling : rising,4);
            }
            // Admission must inspect every vertex. Clipping away the polygon
            // isolates admission/FPU sequencing without using another raster
            // implementation to predict these nonplanar light values.
            for (int low=0;low<4;++low) {
                start(); lock_clip.bottom=1;
                for (int &value : z) value=1;
                z[low]=-1; finish(v,z,4,uv,0);
            }
            start(); finish(v,z,0,uv,0,false);
            // Original release is still reached after a null data result on
            // a safely rejected polygon. Active null-data drawing is outside
            // this fixture's valid allocation domain, not invented recovery.
            start(); null_lock=true; finish(v,z,0,uv,0,false);
            start(); for (int &value : z) value=-1;
            for (Vert &vertex : v) vertex.y=2;
            finish(v,z,4,uv,0,false);
            return 0;
        }
    '''


def test_actual_full_cutoff_raster(tmp_path):
    result = run_host(tmp_path, cutoff_harness(cutoff_bodies()))
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("before,after", [
    ("zvals[in_front] < cutoff", "zvals[in_front] <= cutoff"),
    ("zvals[in_front] < cutoff", "static_cast<signed char>(zvals[in_front]) < static_cast<signed char>(cutoff)"),
    ("shade < static_cast<signed char>(cutoff)", "shade < cutoff"),
    ("shade < static_cast<signed char>(cutoff)", "shade <= static_cast<signed char>(cutoff)"),
    ("if (shade <", "if ((y & 1) && shade <"),
    ("texel + shade", "texel"),
    ("if (shade < static_cast<signed char>(cutoff))\n                            *out = static_cast<unsigned char>(texel + shade);",
     "if (shade < static_cast<signed char>(cutoff) && texel != TextureRasterKey)\n                            *out = static_cast<unsigned char>(texel + shade);"),
    ("(static_cast<int64_t>(dz) * skip) >> 8", "(static_cast<int64_t>(dz) * skip) >> 16"),
    ("(uf >> 16)", "0"),
    ("static_cast<int64_t>(vf >> 16) * width", "0"),
    ("light + dz", "light"),
    ("buffer->stride_", "iWidth_"),
    ("buffer->free_data(1)", "buffer->free_data(2)"),
    ("texture_reset_fpu();", ";"),
])
def test_full_cutoff_damage_is_observed(tmp_path, before, after):
    methods = cutoff_bodies()
    assert before in methods["draw_below_cutoff"]
    methods["draw_below_cutoff"] = methods["draw_below_cutoff"].replace(before, after)
    result = run_host(tmp_path, cutoff_harness(methods))
    assert result.returncode == 42, result.stderr
