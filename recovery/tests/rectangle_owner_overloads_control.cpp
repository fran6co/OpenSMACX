#include <cassert>
#include <cstring>
typedef char *LPSTR;
struct RECT { int left, top, right, bottom; };
struct Spot {
    int add(int, int, int, int, int, int);
    int add(int, int, RECT *);
};
struct Buffer {
    int write_cent_l(LPSTR, RECT *, int);
    int write_cent_l(LPSTR, RECT *);
};
static Spot *expected_spot;
static Buffer *expected_buffer;
static RECT *expected_rect;
static char *expected_text;
static int spot_calls, buffer_calls;
int Spot::add(int position, int type, int x, int y, int width, int height) {
    assert(this == expected_spot);
    assert(position == 3 && type == 7);
    assert(x == -5 && y == 4 && width == 17 && height == 9);
    ++spot_calls;
    return 19;
}
int Buffer::write_cent_l(LPSTR text, RECT *rect, int length) {
    assert(this == expected_buffer && rect == expected_rect);
    assert(text == expected_text && length == static_cast<int>(strlen(text)));
    ++buffer_calls;
    return 23;
}
// ACTUAL_SOURCE_BODIES
int main() {
    Spot spot;
    Buffer buffer;
    RECT area = {-5, 4, 12, 13};
    expected_spot = &spot;
    expected_buffer = &buffer;
    expected_rect = &area;
    assert(spot.add(3, 7, &area) == 19 && spot_calls == 1);
    assert(buffer.write_cent_l(0, 0) == 0 && buffer_calls == 0);
    char text[] = "rectangle";
    expected_text = text;
    assert(buffer.write_cent_l(text, &area) == 23 && buffer_calls == 1);
    char empty[] = "";
    expected_text = empty;
    assert(buffer.write_cent_l(empty, &area) == 23 && buffer_calls == 2);
}
