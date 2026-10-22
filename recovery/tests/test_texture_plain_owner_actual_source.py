"""Exercise the literal mapped 0x620A10 body with controlled host callbacks.

The pin keeps two algorithms in this one entry: borrowed storage refills its
cached edges before painting a saved row; owned storage paints first and uses
Texture::setup_edge for uncached rollover. Host memory/callbacks are controlled
here; this is not a VC6/x87, invalid-pointer, full pixel-equivalence, or game
runtime proof.
"""
from pathlib import Path
import resource
import re
import shutil
import subprocess

import pytest

import test_texture_raster_actual_source as raster
from match_recovery.source import Mode, read_text
from texture_direct_storage_host import PRELUDE_MARKER, direct_storage_prelude


ROOT = raster.ROOT
TEXTURE = ROOT / "src/texture.cpp"
ENTRY = "void Texture::sub_620a10("
ORIGINAL = (0x00620A10, 0x006212A4)
RETAINED_HELPERS = tuple(signature for (signature,) in raster.HELPERS)


def _recorded_body(source):
    rows = [row for row in read_text(source, TEXTURE)
            if row.address == ORIGINAL[0]]
    assert len(rows) == 1
    row = rows[0]
    assert row.mode is Mode.FILE
    assert row.image_spans == (ORIGINAL,)
    assert "Texture::sub_620a10" in row.region
    body = raster._extract_definition(source, ENTRY)
    assert body in row.region
    return body


def _replace_once(source, old, new):
    assert source.count(old) == 1, (old, source.count(old))
    return source.replace(old, new, 1)


def _instrument_host_callbacks(prefix):
    prefix = _replace_once(
        prefix,
        "LIFE_SETUP_GOURAUD, LIFE_CONTROL87",
        "LIFE_SETUP_GOURAUD, LIFE_SETUP_PLAIN, LIFE_SETUP_CACHED, LIFE_CONTROL87",
    )
    store_tail = "    EdgeScan edges_[10];\n};\n"
    observers = """
static TextureStore *plain_observed_cache;
static int plain_next_record_at_get = -1;
static int plain_flags_at_free = -1;
static int plain_flags_at_reset[16];
static int plain_reset_count;
static int plain_edge_observe_call;
static int plain_edge_observe_offset;
static int plain_edge_observed_byte = -1;
static int plain_negative_u;
static int plain_setup_period = 2;
static int plain_setup_calls;
static int plain_setup_fail_at;
static int plain_setup_observe_call;
static int plain_setup_observe_offset;
static int plain_setup_observed_byte = -1;
static unsigned char *plain_setup_observed_base;
"""
    prefix = _replace_once(prefix, store_tail, store_tail + observers)
    prefix = _replace_once(
        prefix,
        "void *get_data() { lifecycle(LIFE_GET_DATA); return active_data_; }",
        "void *get_data() { lifecycle(LIFE_GET_DATA); "
        "plain_next_record_at_get = plain_observed_cache->next_record_; "
        "return active_data_; }",
    )
    prefix = _replace_once(
        prefix,
        "void free_data(int) { lifecycle(LIFE_FREE_DATA); ++frees_; }",
        "void free_data(int) { lifecycle(LIFE_FREE_DATA); ++frees_; "
        "plain_flags_at_free = plain_observed_cache->flags_; }",
    )
    prefix = _replace_once(
        prefix,
        "    ++edge_helper_calls;\n",
        "    ++edge_helper_calls;\n"
        "    lifecycle(LIFE_SETUP_CACHED);\n"
        "    if (edge_helper_calls == plain_edge_observe_call) {\n"
        "        plain_edge_observed_byte = Buffer::active_data_[plain_edge_observe_offset];\n"
        "    }\n",
    )
    prefix = _replace_once(
        prefix,
        "    edge->counter = edge_helper_period;\n"
        "    edge->id = top_index;\n"
        "    edge->accumA = edge->f0 < 0 ? 0 : 4 << 16;\n",
        "    edge->counter = edge_helper_period;\n"
        "    edge->id = top_index;\n"
        "    edge->accumA = plain_negative_u\n"
        "        ? (edge->f0 < 0 ? 3 << 16 : 0)\n"
        "        : (edge->f0 < 0 ? 0 : 4 << 16);\n",
    )
    prefix = _replace_once(
        prefix,
        "    edge->counter = 4;\n    edge->id = 0;\n",
        "    lifecycle(LIFE_SETUP_PLAIN);\n"
        "    ++plain_setup_calls;\n"
        "    if (plain_setup_calls == plain_setup_observe_call)\n"
        "        plain_setup_observed_byte = plain_setup_observed_base[plain_setup_observe_offset];\n"
        "    edge->counter = plain_setup_period;\n    edge->id = 0;\n",
    )
    prefix = _replace_once(
        prefix,
        "    return 1;\n}\n\n// ACTUAL_TEXTURE_DEFINITIONS_INSERTED_BY_TEST",
        "    return plain_setup_calls == plain_setup_fail_at ? 0 : 1;\n}\n\n"
        "// ACTUAL_TEXTURE_DEFINITIONS_INSERTED_BY_TEST",
    )
    prefix = _replace_once(
        prefix,
        "static void texture_reset_fpu() {\n"
        "    lifecycle(LIFE_RESET_FPU);\n    ++fpu_resets;\n}",
        "static void texture_reset_fpu() {\n"
        "    lifecycle(LIFE_RESET_FPU);\n"
        "    if (plain_observed_cache != 0 && plain_reset_count < 16)\n"
        "        plain_flags_at_reset[plain_reset_count++] = plain_observed_cache->flags_;\n"
        "    ++fpu_resets;\n}",
    )
    return prefix


def _scenario_main():
    return r"""
static void reset_plain_observers(TextureStore *cache) {
    reset_lifecycle();
    plain_observed_cache = cache;
    plain_next_record_at_get = -1;
    plain_flags_at_free = -1;
    plain_reset_count = 0;
    plain_edge_observe_call = 0;
    plain_edge_observe_offset = 0;
    plain_edge_observed_byte = -1;
    plain_negative_u = 0;
    plain_setup_calls = 0;
    plain_setup_fail_at = 0;
    plain_setup_observe_call = 0;
    plain_setup_observe_offset = 0;
    plain_setup_observed_byte = -1;
    edge_helper_calls = 0;
    edge_helper_fail_at = 0;
    edge_helper_period = 2;
    edge_directions = 0;
    control87_calls = 0;
    fpu_resets = 0;
}

static void initialize_scene(Texture &texture, Buffer &buffer,
                             TextureStore &cache, unsigned char *surface,
                             unsigned char *owned_target,
                             unsigned char *owned_pixels,
                             unsigned char *borrowed_pixels,
                             int borrowed, int top, int bottom, int right) {
    memset(surface, 77, 4096);
    memset(owned_target, 77, 4096);
    buffer.rect1_ = RECT{0, top, right, bottom};
    buffer.stride_ = 1;
    buffer.frees_ = 0;
    Buffer::active_data_ = surface;
    texture.pixels_ = borrowed ? borrowed_pixels : owned_pixels;
    texture.iWidth_ = 4;
    texture.iHeight_ = 4;
    texture.borrowed_ = borrowed;
    cache.flags_ = 0x55u;
    cache.next_record_ = 9;
    expected_edge_cache = &cache;
    reset_plain_observers(&cache);
    plain_setup_observed_base = borrowed ? surface : owned_target;
    TextureRasterKey = 0;
}

static int first_event(int event) {
    for (int i = 0; i < lifecycle_count; ++i)
        if (lifecycle_events[i] == event) return i;
    return -1;
}

static int last_event(int event) {
    for (int i = lifecycle_count - 1; i >= 0; --i)
        if (lifecycle_events[i] == event) return i;
    return -1;
}

int main() {
    // This is the installed precision-emulation helper, not the fixture's
    // ordinary-division stand-in used by the Gouraud suite.
    assert(texture_span_step(2147483647, 3) == 715827776);
    assert(texture_span_step(-2147483647, 1) == -2147483520);

    unsigned char *surface = static_cast<unsigned char *>(mmap(
        0, 4096, PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0));
    assert(surface != MAP_FAILED);
    const uintptr_t base = static_cast<uint32_t>(
        reinterpret_cast<uintptr_t>(surface));
    const uintptr_t owned_address = static_cast<uint32_t>(base * 2u);
    unsigned char *owned_target = static_cast<unsigned char *>(mmap(
        reinterpret_cast<void *>(owned_address), 4096,
        PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS |
        MAP_FIXED_NOREPLACE, -1, 0));
    assert(owned_target == reinterpret_cast<unsigned char *>(owned_address));

    unsigned char owned_pixels[16] = {
        0, 0, 10, 255, 0, 0, 10, 255,
        0, 0, 10, 255, 0, 0, 10, 255,
    };
    unsigned char *borrowed_mapping = static_cast<unsigned char *>(mmap(
        0, 0x30000, PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0));
    assert(borrowed_mapping != MAP_FAILED);
    const uintptr_t borrowed_base =
        (reinterpret_cast<uintptr_t>(borrowed_mapping) + 0xffffu) & ~uintptr_t(0xffffu);
    unsigned char *borrowed_pixels = reinterpret_cast<unsigned char *>(borrowed_base);
    assert((borrowed_base & 0xffffu) == 0);
    memset(borrowed_pixels, 0, 4096);
    for (int y = 0; y < 4; ++y) {
        borrowed_pixels[y * 256 + 1] = static_cast<unsigned char>(5 + y * 10);
        borrowed_pixels[y * 256 + 2] = static_cast<unsigned char>(10 + y * 10);
        borrowed_pixels[y * 256 + 3] = 255;
    }
    Buffer buffer;
    Texture texture;
    TextureStore cache = {};
    Vert geometry[4] = {{0, 0}, {4, 0}, {4, 3}, {0, 3}};
    Vert uv[4] = {{0, 0}, {4, 0}, {4, 3}, {0, 3}};

    // Borrowed path: initial cached pair, then left refill before the saved
    // second row is painted. Failure still paints that row and suppresses the
    // right refill. The keyed first texel does not stop the next source sample.
    initialize_scene(texture, buffer, cache, surface, owned_target,
                     owned_pixels, borrowed_pixels, 1, 0, 3, 4);
    edge_helper_period = 2;
    edge_helper_fail_at = 3;
    plain_edge_observe_call = 3;
    plain_edge_observe_offset = 1;
    texture.sub_620a10(&buffer, geometry, 4, uv, &cache);
    assert(plain_next_record_at_get == 0);
    assert(edge_helper_calls == 3 && plain_setup_calls == 0);
    assert(edge_helper_direction[0] == -1 && edge_helper_direction[1] == 1 &&
           edge_helper_direction[2] == -1);
    assert(plain_edge_observed_byte == 5);
    assert(surface[0] == 77 && surface[1] == 5);
    assert(surface[2] == 15); // failed left refill still emits the saved row
    assert(control87_calls == 1 && buffer.frees_ == 1 && fpu_resets == 2);
    assert(plain_flags_at_free == 0x55u && cache.flags_ == 1u);
    assert(plain_reset_count == 2 && plain_flags_at_reset[0] == 0x55u &&
           plain_flags_at_reset[1] == 1u);
    assert(first_event(LIFE_GET_DATA) < first_event(LIFE_RESET_FPU));
    assert(first_event(LIFE_CONTROL87) < first_event(LIFE_FREE_DATA));
    assert(first_event(LIFE_FREE_DATA) < last_event(LIFE_RESET_FPU));

    // A failed borrowed right refill also occurs before drawing the saved row;
    // the successful left event remains visible and the row is still painted.
    initialize_scene(texture, buffer, cache, surface, owned_target,
                     owned_pixels, borrowed_pixels, 1, 0, 3, 4);
    edge_helper_period = 2;
    edge_helper_fail_at = 4;
    plain_edge_observe_call = 4;
    plain_edge_observe_offset = 1;
    texture.sub_620a10(&buffer, geometry, 4, uv, &cache);
    assert(edge_helper_calls == 4 && plain_setup_calls == 0);
    assert(edge_helper_direction[2] == -1 && edge_helper_direction[3] == 1);
    assert(plain_edge_observed_byte == 5 && surface[2] == 15);

    // Owned path uses the same initial cached pair, then paints before the
    // uncached left refill. A failed left setup keeps the current row and
    // suppresses right setup; the target mapping models the pinned 32-bit
    // row*stride + clip-left + pixel-base expression without changing it.
    initialize_scene(texture, buffer, cache, surface, owned_target,
                     owned_pixels, borrowed_pixels, 0, 0, 3, 4);
    edge_helper_period = 2;
    plain_setup_period = 2;
    plain_setup_fail_at = 1;
    plain_setup_observe_call = 1;
    plain_setup_observe_offset = 2;
    texture.sub_620a10(&buffer, geometry, 4, uv, &cache);
    assert(edge_helper_calls == 2 && plain_setup_calls == 1);
    assert(plain_setup_observed_byte == 10);
    assert(owned_target[0] == 77 && owned_target[1] == 10 &&
           owned_target[2] == 10 && owned_target[3] == 255);
    assert(control87_calls == 0 && buffer.frees_ == 1);

    // Owned right failure is likewise post-draw; left setup succeeds first.
    initialize_scene(texture, buffer, cache, surface, owned_target,
                     owned_pixels, borrowed_pixels, 0, 0, 3, 4);
    edge_helper_period = 2;
    plain_setup_period = 2;
    plain_setup_fail_at = 2;
    plain_setup_observe_call = 2;
    plain_setup_observe_offset = 2;
    texture.sub_620a10(&buffer, geometry, 4, uv, &cache);
    assert(edge_helper_calls == 2 && plain_setup_calls == 2);
    assert(plain_setup_observed_byte == 10 && owned_target[2] == 10);

    // A controlled negative owned U slope with a nonzero fixed-point
    // remainder exercises the pinned IDIV/remainder-correction branch. The
    // source values distinguish reverse traversal without pretending to
    // model the original x87 packed flags.
    unsigned char negative_pixels[16] = {
        11, 22, 33, 44, 11, 22, 33, 44,
        11, 22, 33, 44, 11, 22, 33, 44,
    };
    initialize_scene(texture, buffer, cache, surface, owned_target,
                     owned_pixels, borrowed_pixels, 0, 0, 1, 4);
    texture.pixels_ = negative_pixels;
    TextureRasterKey = 255;
    plain_negative_u = 1;
    edge_helper_period = 1;
    plain_setup_period = 2;
    plain_setup_fail_at = 1;
    texture.sub_620a10(&buffer, geometry, 4, uv, &cache);
    assert(plain_setup_calls == 1 && edge_helper_calls == 2);
    assert(owned_target[0] == 44 && owned_target[1] == 33 &&
           owned_target[2] == 33 && owned_target[3] == 22);

    // Successful left/right rollovers use mode-specific providers. Top-clipped
    // rows still advance edges, but only row two writes; a right-clipped bbox
    // does not publish the cache-valid flag.
    initialize_scene(texture, buffer, cache, surface, owned_target,
                     owned_pixels, borrowed_pixels, 1, 2, 3, 4);
    edge_helper_period = 2;
    texture.sub_620a10(&buffer, geometry, 4, uv, &cache);
    assert(edge_helper_calls == 4 && plain_setup_calls == 0);
    assert(surface[0] == 77 && surface[1] == 77 && surface[2] == 77);
    assert(surface[3] == 5 && surface[4] == 10);
    assert(plain_flags_at_free == 0x55u && cache.flags_ == 0x55u);

    // The borrowed loop stops once its row cursor reaches the bottom clip;
    // it does not issue another pair of edge-refill callbacks past that row.
    initialize_scene(texture, buffer, cache, surface, owned_target,
                     owned_pixels, borrowed_pixels, 1, 0, 1, 4);
    edge_helper_period = 2;
    texture.sub_620a10(&buffer, geometry, 4, uv, &cache);
    assert(edge_helper_calls == 2 && plain_setup_calls == 0);
    assert(surface[0] == 77 && surface[1] == 5);

    initialize_scene(texture, buffer, cache, surface, owned_target,
                     owned_pixels, borrowed_pixels, 0, 0, 1, 3);
    edge_helper_period = 1;
    plain_setup_period = 1;
    texture.sub_620a10(&buffer, geometry, 4, uv, &cache);
    assert(edge_helper_calls == 2 && plain_setup_calls == 2);
    assert(owned_target[1] == 10 && owned_target[2] == 255 &&
           owned_target[3] == 77);
    assert(buffer.frees_ == 1 && cache.flags_ == 0x55u);
    return 0;
}
"""


def _unit_for(source):
    fixture = raster.FIXTURE.read_text(encoding="utf-8")
    assert fixture.count(raster.INSERTION) == 1
    helper_stub = "static int texture_span_step(int delta, int width) { return delta / width; }"
    actual_slope = raster._extract_definition(source, "static int texture_span_step(")
    assert fixture.count(helper_stub) == 1
    fixture = fixture.replace(helper_stub, actual_slope, 1)
    prefix = fixture
    prefix = prefix.replace(PRELUDE_MARKER, direct_storage_prelude(source), 1)
    prefix = _instrument_host_callbacks(prefix)
    prefix = prefix.split(raster.INSERTION, 1)[0]
    body = _recorded_body(source)
    helpers = [raster._extract_definition(source, signature)
               for signature in RETAINED_HELPERS]
    return prefix + "\n\n".join(helpers + [body]) + _scenario_main()


def _compile_and_run(source, compiler, tmp_path, stem, success):
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(f"{compiler} is unavailable")
    cpp = tmp_path / f"{stem}.cpp"
    binary = tmp_path / stem
    cpp.write_text(_unit_for(source), encoding="utf-8")
    built = subprocess.run(
        [compiler_path, "-std=c++11", "-Wall", "-Wextra",
         "-Wno-unused-parameter", str(cpp), "-o", str(binary)],
        cwd=ROOT, check=False, capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], check=False, capture_output=True,
                         text=True, timeout=15,
                         preexec_fn=lambda: resource.setrlimit(resource.RLIMIT_CORE, (0, 0)))
    assert (ran.returncode == 0) == success, ran.stdout + ran.stderr


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_plain_owner_modes_callbacks_key_clip_and_cleanup(compiler, tmp_path):
    source = TEXTURE.read_text(encoding="utf-8")
    _compile_and_run(source, compiler, tmp_path, "plain_owner", success=True)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_plain_owner_key_mutant_fails_actual_pixel_controls(compiler, tmp_path):
    source = TEXTURE.read_text(encoding="utf-8")
    body = _recorded_body(source)
    key = r"if \(texel != static_cast<unsigned char>\(\s*TextureRasterLightThreshold\)\) \{"
    mutant, replaced = re.subn(key, "if (true) {", body)
    assert replaced == 2, replaced
    assert mutant != body
    changed = source.replace(body, mutant, 1)
    _compile_and_run(changed, compiler, tmp_path, "plain_key_mutant", success=False)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_plain_owner_mode_provider_mutant_fails_callback_controls(compiler, tmp_path):
    source = TEXTURE.read_text(encoding="utf-8")
    body = _recorded_body(source)
    mode_test = "const int borrowed = borrowed_ != 0;"
    assert body.count(mode_test) == 1
    mutant = body.replace(mode_test, "const int borrowed = borrowed_ == 0;", 1)
    changed = source.replace(body, mutant, 1)
    _compile_and_run(changed, compiler, tmp_path, "plain_mode_mutant", success=False)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_plain_owner_negative_remainder_mutant_fails_pixel_controls(compiler, tmp_path):
    source = TEXTURE.read_text(encoding="utf-8")
    body = _recorded_body(source)
    correction = "if (du < 0 &&"
    assert body.count(correction) == 1
    mutant = body.replace(correction, "if (false &&", 1)
    changed = source.replace(body, mutant, 1)
    _compile_and_run(changed, compiler, tmp_path,
                     "plain_negative_remainder_mutant", success=False)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_plain_owner_cache_publication_mutant_fails_cleanup_control(compiler, tmp_path):
    source = TEXTURE.read_text(encoding="utf-8")
    body = _recorded_body(source)
    publication = "if (v80 == 0) cache->flags_ = 1;"
    assert body.count(publication) == 1
    mutant = body.replace(publication,
                          "if (v80 != 0) cache->flags_ = 1;", 1)
    changed = source.replace(body, mutant, 1)
    _compile_and_run(changed, compiler, tmp_path, "plain_flags_mutant", success=False)
