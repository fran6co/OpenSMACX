from pathlib import Path
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[2]


def body(signature):
    source = (ROOT / "src/datalink.cpp").read_text()
    start = source.index(signature + " {")
    brace = source.index("{", start)
    depth = 0
    for end in range(brace, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated function")


def test_help_wrapper_receiver_arguments_and_shared_callback():
    callback = body("void __cdecl help_info_pop(char *text)")
    wrapper = body("void __cdecl help_tech_info(Popup *popup, int technology, int faction)")
    program = r'''
#include <cassert>
#define __cdecl
struct Popup {
    int calls;
    char *last;
    int string(char *text) { ++calls; last = text; return -1; }
};
Popup *TechnologyHelpPopup;
Popup *expected_receiver;
int expected_tech, expected_faction;
char text[] = "technology";
int builder_calls;
void help_info_pop(char *);
void help_tech_info(void (*callback)(char *), int tech, int faction, int mode) {
    ++builder_calls;
    assert(TechnologyHelpPopup == expected_receiver);
    assert(callback == help_info_pop);
    assert(tech == expected_tech && faction == expected_faction && mode == -1);
    callback(text);
}
''' + callback + "\n" + wrapper + r'''
int main() {
    Popup first = {}, second = {};
    expected_receiver = &first;
    expected_tech = 17;
    expected_faction = 3;
    help_tech_info(&first, 17, 3);
    assert(first.calls == 1 && first.last == text);
    expected_receiver = &second;
    expected_tech = -2;
    expected_faction = 0;
    help_tech_info(&second, -2, 0);
    assert(second.calls == 1 && second.last == text && builder_calls == 2);
    // Original storage is shared and not restored by the wrapper.
    help_info_pop(text);
    assert(first.calls == 1 && second.calls == 2);
}
'''
    with tempfile.TemporaryDirectory() as directory:
        source = Path(directory) / "help.cpp"
        executable = Path(directory) / "help"
        source.write_text(program)
        subprocess.run(["g++", "-std=c++11", "-Wall", "-Wextra", "-Werror",
                        str(source), "-o", str(executable)], check=True)
        subprocess.run([str(executable)], check=True)
