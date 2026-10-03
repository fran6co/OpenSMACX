// C++98 actual-body control. GraphicWin models its true 0x444-byte Win
// primary prefix followed by the public Buffer secondary base.
#include <cassert>

#define __cdecl
struct Buffer {
    int remap_count;
    int remaps[2][2];
    Buffer() : remap_count(0) {}
    int change_color(int from, int to) {
        assert(remap_count < 2);
        remaps[remap_count][0] = from;
        remaps[remap_count][1] = to;
        ++remap_count;
        return 0;
    }
};
struct GraphicWinPrefix { unsigned char bytes[0x444]; };
struct GraphicWin : public GraphicWinPrefix, public Buffer {};
typedef char graphic_primary_prefix_size_is_444[
    sizeof(GraphicWinPrefix) == 0x444 ? 1 : -1];
struct Sprite {
    int alpha;
    Buffer *buffer;
    int args[6];
    int calls;
    int result;
    Sprite() : alpha(7), buffer(0), calls(0), result(91) {}
    unsigned char transparent_index() const {
        return static_cast<unsigned char>(alpha);
    }
    void draw(Buffer *b, int transparent, int x, int y, int n, int d) {
        buffer = b;
        args[0] = transparent; args[1] = x; args[2] = y;
        args[3] = n; args[4] = d; ++calls;
    }
    int extract(Buffer *b, int transparent, int x, int y,
                int width, int height, void *) {
        buffer = b;
        args[0] = transparent; args[1] = x; args[2] = y;
        args[3] = width; args[4] = height; ++calls;
        return result;
    }
};
struct Texture {
    Buffer *buffer;
    int args[5];
    int calls;
    int result;
    Texture() : buffer(0), calls(0), result(37) {}
    int extract(Buffer *b, int x, int y, int width, int height, void *) {
        buffer = b;
        args[0] = x; args[1] = y; args[2] = width; args[3] = height;
        ++calls;
        return result;
    }
};
Buffer g_PCX_PARSE_TEMP_BUFFER1;

// ACTUAL_SOURCE_BODIES

static void assert_remaps() {
    assert(g_PCX_PARSE_TEMP_BUFFER1.remap_count == 2);
    assert(g_PCX_PARSE_TEMP_BUFFER1.remaps[0][0] == 0x107);
    assert(g_PCX_PARSE_TEMP_BUFFER1.remaps[0][1] == 0x109);
    assert(g_PCX_PARSE_TEMP_BUFFER1.remaps[1][0] == 0x106);
    assert(g_PCX_PARSE_TEMP_BUFFER1.remaps[1][1] == 0x109);
}
static void clear_buffer() { g_PCX_PARSE_TEMP_BUFFER1.remap_count = 0; }

int main() {
    Sprite sprite;
    Texture texture;
    GraphicWin owner;
    Buffer *adjusted_owner = static_cast<Buffer *>(&owner);
    assert(reinterpret_cast<char *>(adjusted_owner)
           - reinterpret_cast<char *>(&owner) == 0x444);

    sprite_draw_func(&sprite, &owner, 123, 29, 41);
    assert(sprite.buffer == adjusted_owner);
    assert(sprite.args[0] == 7 && sprite.args[1] == 24 && sprite.args[2] == 35);
    assert(sprite.args[3] == 1 && sprite.args[4] == 1 && sprite.calls == 1);
    sprite_draw_func(&sprite, 0, 0, 5, 6);
    assert(sprite.buffer == 0 && sprite.args[1] == 0 && sprite.args[2] == 0);

    clear_buffer();
    assert(sub_450de0(&sprite, 11, 12) == 91);
    assert_remaps();
    assert(sprite.buffer == &g_PCX_PARSE_TEMP_BUFFER1 && sprite.args[0] == 0x109);
    assert(sprite.args[1] == 11 && sprite.args[2] == 12);
    assert(sprite.args[3] == 100 && sprite.args[4] == 50);

    clear_buffer();
    assert(sub_450e30(&sprite, -4, 19) == 91);
    assert_remaps();
    assert(sprite.args[3] == 100 && sprite.args[4] == 62);

    clear_buffer();
    assert(sub_450e80(&sprite, 21, -6) == 91);
    assert_remaps();
    assert(sprite.args[3] == 100 && sprite.args[4] == 75);

    clear_buffer();
    assert(sub_450ed0(&sprite, 1, 2, 33, 44) == 91);
    assert_remaps();
    assert(sprite.args[1] == 1 && sprite.args[2] == 2);
    assert(sprite.args[3] == 33 && sprite.args[4] == 44);

    clear_buffer();
    sub_450f30(&sprite, 3, 4, 55, 66);
    assert_remaps();
    assert(sprite.args[3] == 55 && sprite.args[4] == 66);

    assert(sub_450f90(&texture, 9, 10) == 37);
    assert(texture.calls == 1 && texture.buffer == &g_PCX_PARSE_TEMP_BUFFER1);
    assert(texture.args[0] == 9 && texture.args[1] == 10);
    assert(texture.args[2] == 56 && texture.args[3] == 56);
    return 0;
}
