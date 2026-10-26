"""Exercise the product AlphaMovie::exec body with typed callback probes."""
from pathlib import Path
import os
import subprocess

HERE = Path(__file__).resolve().parent


def repo_root():
    configured = os.environ.get("OPENSMACX_TEST_SOURCE_ROOT")
    if configured:
        candidate = Path(configured).resolve()
        if (candidate / "src/alphamovie.cpp").exists():
            return candidate
        raise AssertionError("OPENSMACX_TEST_SOURCE_ROOT lacks src/alphamovie.cpp")
    for candidate in (HERE, *HERE.parents):
        if (candidate / "src/alphamovie.cpp").exists():
            return candidate
    raise AssertionError("cannot locate OpenSMACX source root")


def extract_definition(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    quote = None
    escaped = False
    line_comment = block_comment = False
    index = brace
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if line_comment:
            if char == "\n":
                line_comment = False
            index += 1
            continue
        if block_comment:
            if char == "*" and nxt == "/":
                block_comment = False
                index += 2
                continue
            index += 1
            continue
        if quote:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == quote:
                quote = None
            index += 1
            continue
        if char == "/" and nxt == "/":
            line_comment = True
            index += 2
            continue
        if char == "/" and nxt == "*":
            block_comment = True
            index += 2
            continue
        if char in ("'", '"'):
            quote = char
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
        index += 1
    raise AssertionError(f"unterminated product definition: {signature}")


def product_exec():
    source = (repo_root() / "src/alphamovie.cpp").read_text()
    return extract_definition(source, "int AlphaMovie::exec(char *movie_name)")


FIXTURE = r'''#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

typedef int HWND;
enum { FALSE = 0, SW_HIDE = 0 };
struct Win {};
static Win MainInterfaceGlobal;
static Win *active_movie_window;
static char StringTemp[128]; // bounded test fixture; product has no guard/reset
static int load_results[2];
static int load_calls;
static int screen_height_reads;
static int screen_width_reads;
static int screen_height_value = 600;
static int screen_width_value = 800;
static int init_width, init_height, init_calls, fill_calls;
static int move_calls, move_window, move_x, move_y, move_width, move_height;
static int show_window_calls, show_window_handle;
static int modal_calls, flush_calls, non_input_calls;
static std::vector<std::string> events;
static std::vector<std::string> load_paths;
static void event(const char *name) { events.push_back(name); }
static int read_height() { ++screen_height_reads; event("height"); return screen_height_value; }
static int read_width() { ++screen_width_reads; event("width"); return screen_width_value; }
static std::string event_trace() {
    std::string out;
    for (size_t i = 0; i < events.size(); ++i) {
        if (i != 0) out += ',';
        out += events[i];
    }
    return out;
}

struct AlphaMovie;
static AlphaMovie *active_movie;
struct Palette {
    int kind;
    explicit Palette(int value = 0) : kind(value) {}
    void set();
};
struct MCIVideo {
    int blit_height_;
    int blit_width_;
    HWND window_handle_;
    Palette palette_;
    MCIVideo() : blit_height_(200), blit_width_(320), window_handle_(10), palette_(0) {}
    int load(char *path, Win *target, int x, int y) {
        assert(target == active_movie_window && x == 0 && y == 0);
        assert(load_calls < 2);
        event(load_calls == 0 ? "load1" : "load2");
        load_paths.push_back(path == 0 ? std::string("<null>") : std::string(path));
        return load_results[load_calls++];
    }
    void play() { event("play"); }
    void stop() { event("stop"); window_handle_ = 23; }
};
struct GraphicWin : Win {
    int init(int x, int y, int width, int height, char *title, int flags,
             Win *parent, void *menu, void *border) {
        assert(x == 0 && y == 0 && title == 0 && flags == 0x404000);
        assert(parent == &MainInterfaceGlobal && menu == 0 && border == 0);
        event("init"); ++init_calls; init_width = width; init_height = height; return 0;
    }
    void fill(int color) { assert(color == 0); event("fill"); ++fill_calls; }
    virtual void show(int visible) { assert(visible == 0); event("show"); }
    virtual void hide() { event("hide"); }
    void set_modal(int a, int b, int c) {
        assert(a == 0 && b == 0 && c == 0); event("modal"); ++modal_calls;
    }
};
struct AlphaMovie : GraphicWin {
    int exec(char *movie_name);
    MCIVideo mciVideo_;
};
void Palette::set() {
    if (kind == 0) {
        event("device_palette");
        active_movie->mciVideo_.window_handle_ = 21;
    } else {
        event("active_palette");
        active_movie->mciVideo_.window_handle_ = 29;
    }
}
static Palette g_PALETTE1(1);
static void do_all_non_input() { event("pump"); ++non_input_calls; }
static void flush_input() { event("flush"); ++flush_calls; }
static int MoveWindow(HWND window, int x, int y, int width, int height, int repaint) {
    assert(repaint == FALSE);
    event("move"); ++move_calls; move_window = window; move_x = x; move_y = y;
    move_width = width; move_height = height; return 1;
}
static int ShowWindow(HWND window, int command) {
    assert(command == SW_HIDE);
    event("window_hide"); ++show_window_calls; show_window_handle = window; return 1;
}

/* PRODUCT AlphaMovie::exec BODY */
'''


def compile_fixture(tmp_path):
    source = product_exec()
    fixture = FIXTURE.replace(
        "/* PRODUCT AlphaMovie::exec BODY */",
        "#define WinScreenHeight read_height()\n"
        "#define WinScreenWidth read_width()\n" + source,
    )
    fixture += r'''
static void reset_state() {
    StringTemp[0] = 0;
    load_results[0] = load_results[1] = 0;
    load_calls = screen_height_reads = screen_width_reads = 0;
    init_width = init_height = init_calls = fill_calls = 0;
    move_calls = move_window = move_x = move_y = move_width = move_height = 0;
    show_window_calls = show_window_handle = modal_calls = flush_calls = non_input_calls = 0;
    events.clear(); load_paths.clear();
}
static void assert_playback_trace() {
    assert(event_trace() ==
        "width,height,init,fill,show,pump,load1,move,device_palette,play,flush,modal,stop,active_palette,window_hide,hide,pump");
}
static void assert_retry_playback_trace() {
    assert(event_trace() ==
        "width,height,init,fill,show,pump,load1,load2,move,device_palette,play,flush,modal,stop,active_palette,window_hide,hide,pump");
}
int main() {
    reset_state();
        AlphaMovie movie; active_movie = &movie;
        active_movie_window = static_cast<Win *>(&movie);
    strcpy(StringTemp, "stale");
    assert(movie.exec(0) == 0x10);
    assert(event_trace().empty() && load_calls == 0 && strcmp(StringTemp, "stale") == 0);

    reset_state();
    movie.mciVideo_.window_handle_ = 10;
    strcpy(StringTemp, "keep/");
    assert(movie.exec(const_cast<char *>("intro.smk")) == 0);
    assert_playback_trace();
    assert(screen_height_reads == 1 && screen_width_reads == 1);
    assert(init_width == 800 && init_height == 600);
    assert(load_calls == 1 && load_paths[0] == "intro.smk");
    assert(strcmp(StringTemp, "keep/") == 0);
    assert(move_calls == 1 && move_window == 10);
    assert(move_x == 240 && move_y == 200 && move_width == 320 && move_height == 200);
    assert(show_window_calls == 1 && show_window_handle == 29);
    assert(movie.mciVideo_.window_handle_ == 29);
    assert(fill_calls == 1 && modal_calls == 1 && flush_calls == 1 && non_input_calls == 2);

    reset_state();
    movie.mciVideo_.window_handle_ = 10;
    strcpy(StringTemp, "preexisting/");
    load_results[0] = 5;
    load_results[1] = 0;
    assert(movie.exec(const_cast<char *>("intro.smk")) == 0);
    assert_retry_playback_trace();
    assert(load_calls == 2 && load_paths[0] == "intro.smk" && load_paths[1].empty());
    assert(strcmp(StringTemp, "preexisting/k:\\game\\intro.smk") == 0);
    assert(move_window == 10 && show_window_handle == 29);

    reset_state();
    movie.mciVideo_.window_handle_ = 10;
    strcpy(StringTemp, "not-cleared/");
    load_results[0] = 5;
    load_results[1] = 7;
    assert(movie.exec(const_cast<char *>("retry.smk")) == 7);
    assert(event_trace() == "width,height,init,fill,show,pump,load1,load2");
    assert(load_calls == 2 && load_paths[0] == "retry.smk" && load_paths[1].empty());
    assert(strcmp(StringTemp, "not-cleared/k:\\game\\retry.smk") == 0);
    assert(move_calls == 0 && show_window_calls == 0 && modal_calls == 0 && flush_calls == 0);
    return 0;
}
'''
    cpp = tmp_path / "alphamovie_exec.cpp"
    exe = tmp_path / "alphamovie_exec"
    cpp.write_text(fixture)
    built = subprocess.run(
        ["c++", "-std=c++11", "-O2", "-Wall", "-Wextra", "-Werror=return-type",
         str(cpp), "-o", str(exe)], capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    return exe


def test_product_alphamovie_exec_native_retry_and_window_order(tmp_path):
    exe = compile_fixture(tmp_path)
    run = subprocess.run([str(exe)], capture_output=True, text=True)
    assert run.returncode == 0, run.stdout + run.stderr
