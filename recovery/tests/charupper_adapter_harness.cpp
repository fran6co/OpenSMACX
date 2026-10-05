// Host-only projection. test_charupper_adapter.py injects the actual
// annotated owner definition; this does not emulate Windows locale/DBCS rules.
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#ifndef __cdecl
#define __cdecl
#endif

static int api_calls;
static char api_arguments[4][64];

static char *CharUpperA(char *text) {
    assert(api_calls < 4);
    const size_t length = strlen(text);
    assert(length < sizeof(api_arguments[0]));
    strcpy(api_arguments[api_calls], text);
    ++api_calls;
    for (size_t index = 0; index < length; ++index) {
        text[index] = static_cast<char>(
            toupper(static_cast<unsigned char>(text[index])));
    }
    return text;
}

/*EXTRACTED_UPPERCASE_TEXT*/

static void reset_api() {
    api_calls = 0;
    memset(api_arguments, 0, sizeof(api_arguments));
}

static void test_noop_preserves_pointer_and_does_not_touch_text() {
    char text[] = "mIxed";
    reset_api();
    assert(uppercase_text(text, 0) == text);
    assert(strcmp(text, "mIxed") == 0);
    assert(api_calls == 0);

    reset_api();
    assert(uppercase_text(0, 0) == 0);
    assert(api_calls == 0);
}

static void test_first_character_uses_nul_terminated_one_byte_input() {
    char text[] = "mIxed";
    reset_api();
    assert(uppercase_text(text, 1) == text);
    assert(strcmp(text, "MIxed") == 0);
    assert(api_calls == 1);
    assert(strcmp(api_arguments[0], "m") == 0);
}

static void test_other_nonzero_modes_uppercase_the_complete_string() {
    char text[] = "mIxed 42?";
    reset_api();
    assert(uppercase_text(text, 2) == text);
    assert(strcmp(text, "MIXED 42?") == 0);
    assert(api_calls == 1);
    assert(strcmp(api_arguments[0], "mIxed 42?") == 0);

    char negative_mode[] = "aZ";
    reset_api();
    assert(uppercase_text(negative_mode, -7) == negative_mode);
    assert(strcmp(negative_mode, "AZ") == 0);
    assert(api_calls == 1);
    assert(strcmp(api_arguments[0], "aZ") == 0);
}

int main() {
    test_noop_preserves_pointer_and_does_not_touch_text();
    test_first_character_uses_nul_terminated_one_byte_input();
    test_other_nonzero_modes_uppercase_the_complete_string();
    puts("PASS: extracted uppercase adapter host controls");
    return 0;
}
