// Actual-body behavioral witnesses; mock state does not prove original ABI/layout.
#include <assert.h>
#include <string.h>
#define __cdecl

struct DiplomacyWindow {
    int calls, first, second;
    void exec(int a, int b) { ++calls; first = a; second = b; }
} DiploWindow;
struct Vehicle { short next_veh_id_stack; } Vehs[3];
char text_storage[] = "source line";
char StringTemp[64];
char *TextBufferGetPtr;
int text_calls, parse_calls;
void text_get() {
    ++text_calls;
    TextBufferGetPtr = text_storage;
    strcpy(StringTemp, "old output");
}
void parse_string(char *input, char *output) {
    ++parse_calls;
    assert(text_calls == parse_calls);
    assert(input == text_storage);
    assert(output == StringTemp && output[0] == 0);
    strcpy(output, input);
}
struct Rect { int left, top, right, bottom; } WinBubbleRect;
int WinBubbleActive;
struct Win {
    static int calls;
    static void flip(Rect *area) {
        assert(area == &WinBubbleRect);
        ++calls;
    }
};
int Win::calls;

// ACTUAL_SOURCE_BODIES

int main() {
    diplo(3, 7);
    assert(DiploWindow.calls == 1);
    assert(DiploWindow.first == 3 && DiploWindow.second == 7);
    assert(veh_next_stack_id(-1) == -1);
    assert(veh_next_stack_id(-19) == -19);
    Vehs[0].next_veh_id_stack = -17;
    Vehs[1].next_veh_id_stack = 2;
    Vehs[2].next_veh_id_stack = -32768;
    assert(veh_next_stack_id(0) == -17);
    assert(veh_next_stack_id(1) == 2);
    assert(veh_next_stack_id(2) == -32768);
    read_and_parse_text_line();
    read_and_parse_text_line();
    assert(text_calls == 2 && parse_calls == 2);
    assert(strcmp(StringTemp, text_storage) == 0);
    WinBubbleActive = 0;
    flip_active_bubble_text();
    assert(Win::calls == 0);
    WinBubbleActive = 1;
    flip_active_bubble_text();
    WinBubbleActive = -3;
    flip_active_bubble_text();
    assert(Win::calls == 2);
}
