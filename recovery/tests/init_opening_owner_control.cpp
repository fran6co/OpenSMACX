#define __cdecl

struct Win {
    virtual ~Win() = default;
    virtual void show(int visible) = 0;
};
struct Buffer {
    Buffer();
    ~Buffer();
    struct DIB { struct Header { int biWidth, biHeight; } bmiHeader; } dib_;
    // ACTUAL_BUFFER_GETTERS
    void load_pcx(const char *, void *, int, int);
    void copy(Buffer *, int, int, int, int, int, int, int, int);
};
struct GraphicWin : Win, Buffer {
    GraphicWin();
    void show(int) override;
    int init(int, int, int, int, char *, int, Win *, void *, void *);
    void fill(int);
    virtual void on_update();
};
struct Palette { int set(); };
struct Console : Win { void show(int) override; };
extern GraphicWin *OpeningWindow;
extern Win *BasePopDefaultField322C;
extern Console ConsoleGlobal;
extern Palette g_PALETTE1;
extern int WinScreenWidth, WinScreenHeight;
void close_opening();
void do_all_draws();
int __cdecl init_opening(const char *);

int init_opening(const char *filename);


#include <cassert>
#include <cstddef>
#include <new>

namespace {
alignas(GraphicWin) unsigned char storage[sizeof(GraphicWin)];
GraphicWin after_load, after_copy, after_update, after_draws;
bool allocation_fails, init_fails, mutate_load, mutate_copy, mutate_update,
    mutate_draws;
int events[64], count;
}

GraphicWin *OpeningWindow;
Win *BasePopDefaultField322C;
Console ConsoleGlobal;
Palette g_PALETTE1;
int WinScreenWidth = 800, WinScreenHeight = 600;

namespace {
void record(int n) { events[count++] = n; }

}

void *operator new(std::size_t n) {
    assert(n == sizeof(GraphicWin));
    record(20);
    return allocation_fails ? nullptr : storage;
}
void operator delete(void *) noexcept {}
void operator delete(void *, std::size_t) noexcept {}

void close_opening() {
    BasePopDefaultField322C = &ConsoleGlobal;
    delete OpeningWindow;
    OpeningWindow = nullptr;
    record(11);
}
void do_all_draws() {
    record(7);
    if (mutate_draws) OpeningWindow = &after_draws;
}
Buffer::Buffer() { dib_.bmiHeader.biWidth = 640; dib_.bmiHeader.biHeight = -400; record(13); }
GraphicWin::GraphicWin() { dib_.bmiHeader.biWidth = 320; dib_.bmiHeader.biHeight = -200; }
Buffer::~Buffer() { record(14); }
void Buffer::load_pcx(const char *f, void *p, int flags, int transparency) {
    assert(f && !p && flags == 10 && transparency == 0xEC);
    record(2);
    if (mutate_load) OpeningWindow = &after_load;
}
void Buffer::copy(Buffer *target, int sx, int sy, int sw, int sh, int dx,
                  int dy, int dw, int dh) {
    assert(sx == 0 && sy == 0 && sw == 640 && sh == 400);
    assert(dx == 0 && dy == 0 && dw == 320 && dh == 200);
    if (mutate_load) assert(target == static_cast<Buffer *>(&after_load));
    else assert(target == static_cast<Buffer *>(OpeningWindow));
    record(3);
    if (mutate_copy) OpeningWindow = &after_copy;
}
void GraphicWin::show(int visible) {
    assert(visible == 0);
    if (mutate_copy) assert(this == &after_update);
    else assert(this == OpeningWindow);
    record(61);
}
int GraphicWin::init(int x, int y, int w, int h, char *title, int flags,
                     Win *parent, void *menu, void *border) {
    assert(x == 0 && y == 0 && w == 800 && h == 600 && !title);
    assert(flags == 0x400000 && !parent && !menu && !border);
    record(10);
    return init_fails ? 0x10 : 0;
}
void GraphicWin::fill(int c) { assert(c == 0); record(1); }
void GraphicWin::on_update() {
    if (mutate_copy) assert(this == &after_copy);
    if (mutate_update) OpeningWindow = &after_update;
    record(4);
}
void Console::show(int) {}
int Palette::set() { record(5); return 0; }

// ACTUAL_SOURCE_BODY

int main() {
    allocation_fails = true;
    count = 0;
    assert(init_opening("logo.pcx") == 4);
    assert(OpeningWindow == nullptr && count == 1 && events[0] == 20);

    allocation_fails = false;
    init_fails = true;
    count = 0;
    assert(init_opening("logo.pcx") == 0x10);
    const int failed[] = {20, 13, 10, 14, 11};
    assert(count == 5);
    for (int i = 0; i < count; ++i) assert(events[i] == failed[i]);

    init_fails = false;
    count = 0;
    assert(init_opening("logo.pcx") == 0);
    const int normal[] = {20, 13, 10, 1, 13, 2, 3, 4, 14, 5, 61, 7};
    assert(count == 12);
    for (int i = 0; i < count; ++i) assert(events[i] == normal[i]);

    OpeningWindow = nullptr;
    count = 0;
    mutate_load = mutate_copy = mutate_update = mutate_draws = true;
    assert(init_opening("logo.pcx") == 0);
    assert(count == 12);
    const int independent_reloads[] = {20, 13, 10, 1, 13, 2, 3, 4, 14, 5,
                                       61, 7};
    for (int i = 0; i < count; ++i)
        assert(events[i] == independent_reloads[i]);
    assert(OpeningWindow == &after_draws && BasePopDefaultField322C == &after_draws);
}
