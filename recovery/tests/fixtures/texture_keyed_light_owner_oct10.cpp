// Runtime tail for the actual keyed draw_trans method. The host declarations
// and setup mock are borrowed from texture_draw_trans_host.cpp by the test;
// this case configures two-row Gouraud edges and a clipped high-WORD light.
int main() {
    static unsigned char pixels[8 * 8];
    static unsigned char storage[8 * 4];
    memset(pixels, 10, sizeof(pixels));
    memset(storage, 77, sizeof(storage));
    memset(slots, 0, sizeof(slots));
    surface = storage;
    texture_raster_key = 99;
    setup_calls = fpu_resets = 0;
    keyed_owner_rows = 2;
    keyed_owner_delta_d = 1;

    Vert vertices[3] = {{0, 0}, {4, 2}, {0, 0}};
    Vert uv[3] = {};
    int zvals[3] = {};
    Buffer buffer = {};
    buffer.rect1_ = RECT{0, 0, 1, 2};
    buffer.stride_ = 8;
    Texture texture = {};
    texture.pixels_ = pixels;
    texture.iWidth_ = 8;
    texture.iHeight_ = 8;

    texture.draw_trans(&buffer, vertices, zvals, 3, uv);

    assert(buffer.frees == 1 && fpu_resets == 2);
    assert(setup_calls == 4);
    assert(keyed_owner_span_count == 2);
    // The second argument is the intermediate entering the real keyed span;
    // downstream 16-bit light truncation can hide the signed/unsigned delta.
    assert(keyed_owner_steps[0] == 0x8000);
    assert(keyed_owner_lights[0] == 0x807fffffu);
    assert(keyed_owner_lights[1] == 0x80800000u);
    assert(storage[0] == 137 && storage[8] == 138);
    assert(storage[1] == 77 && storage[7] == 77);
    assert(storage[8 + 1] == 77 && storage[8 + 7] == 77);
    return 0;
}
